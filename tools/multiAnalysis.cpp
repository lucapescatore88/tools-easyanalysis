#include "multiAnalysis.hpp"


void MultiAnalysis::AddCategory(Analysis * _ana, TString nameCat)
{
    samples->defineType(nameCat);
    categories.push_back(nameCat);
    ana.push_back(_ana);
    constr->add(*(_ana->GetConstraints()));

    if (vars->getSize() == 0) vars->add(*_ana->GetVariable());
    else if (!vars->find(_ana->GetVariable()->GetName()))
        vars->add(*_ana->GetVariable());
}

void MultiAnalysis::AddCategory(TString nameCat, RooRealVar * var, string opt)
{
    samples->defineType(nameCat);
    categories.push_back(nameCat);

    if (vars->getSize() == 0) vars->add(*var);
    else if (!vars->find(var->GetName())) vars->add(*var);

    if (opt == "setana")
    {
        Analysis * newana = new Analysis(nameCat, var);
        ana.push_back(newana);
    }
}

void MultiAnalysis::PlotCategories()
{
    TCanvas * c = new TCanvas();
    for (unsigned i = 0; i < categories.size(); i++)
    {
        RooPlot* frame = ana[i]->GetVariable()->frame();
        TString cut = "samples==samples::" + categories[i];

        cout << "Category: " << categories[i] << endl;
        combData->Print();
        combModel->Print();

        combData->plotOn(frame, Cut(cut));
        RooDataSet::setDefaultStorageType(RooAbsData::Vector);
        RooAbsData * reddata = combData->reduce(cut);
        combModel->getPdf(categories[i])->plotOn(frame, Normalization(reddata->numEntries(), RooAbsReal::NumEvent));

        frame->SetTitle("");
        frame->SetXTitle(((TString)ana[i]->GetVariable()->GetName()).ReplaceAll("__var__", ""));
        frame->Draw();
        c->Print(name + "_" + categories[i] + ".pdf");

    }
}

bool MultiAnalysis::Initialize(string opt)
{
    bool nomodel = false, nodata = false;
    if (opt.find("-noinitmodel") != string::npos) nomodel = true;
    if (opt.find("-noinitdata") != string::npos) nodata = true;

    if ( !nomodel ) combModel = new RooSimultaneous("combModel", "", *samples);
    map<string, RooDataSet*> mymap;

    if (!nodata || !nomodel)
        for (unsigned i = 0; i < categories.size(); i++)
        {
            if (opt.find("-initialize") != string::npos) ana[i]->Initialize(opt);
            if ( !ana[i]->isValid() ) { cout << "*** WARNING!: Analysis " << ana[i]->GetName() << " is not initialized!" << endl; return false; }
            mymap[(string)categories[i]] = (RooDataSet*)(ana[i]->GetDataSet(opt));
            if ( !nomodel ) combModel->addPdf(*(ana[i]->GetModel()), categories[i]);
        }

    if ( !nodata ) combData = new RooDataSet("combData", "combined datas", *vars, Index(*samples), Import(mymap));

    if ((string)ana[0]->GetPrintLevel() == "v") if (combData)  combData->Print();
    if ((string)ana[0]->GetPrintLevel() == "v") if (combModel) combModel->Print();
    //if((string)ana[0]->GetPrintLevel() == "v") if(combModel) printParams(combModel, *vars);
    //if((string)ana[0]->GetPrintLevel() == "v") if(combModel) printParams(combModel);

    cout << "Object MultiAna " << name << " initialized correctly." << endl;

    init = true;
    return init;
}

map < string, RooPlot * > MultiAnalysis::SimultaneousFit(double min, double max, unsigned nbins, string opt)
{
    if (!init) Initialize(opt);
    if ((string)ana[0]->GetPrintLevel() == "v") cout << "--------------- FITTING --------------- " << endl;

    if (opt.find("-toy") != string::npos) isToy = true;
    RooCmdArg isExtended = Extended(kTRUE);
    if (opt.find("-noextended") != string::npos) isExtended = Extended(kFALSE);
    RooCmdArg isQuiet = PrintLevel(2);
    if (opt.find("-quiet") != string::npos) isQuiet = PrintLevel(-1);
    RooCmdArg useMinos = Minos(kFALSE);
    if (opt.find("-minos") != string::npos) useMinos = Minos(kTRUE);
    RooCmdArg initialHesse = InitialHesse(kFALSE);
    if (opt.find("-initialhesse") != string::npos) initialHesse = InitialHesse(kTRUE);
    RooCmdArg hesse = Hesse(kTRUE);
    if (opt.find("-nohesse") != string::npos) hesse = Hesse(kFALSE);

    if (!combModel || !combData) { cout << "Model or data not set" << endl; return map < string, RooPlot * >(); }

    if (min == max) fitResult = combModel->fitTo(*combData, Save(), isExtended, isQuiet, useMinos, ExternalConstraints(*constr), hesse, initialHesse);
    else
    {
        for (unsigned i = 0; i < categories.size(); i++)
            ana[i]->GetVariable()->setRange("FitRange", min, max);

        fitResult = combModel->fitTo(*combData, Save(), isExtended,
                                     isQuiet, useMinos, ExternalConstraints(*constr), hesse, initialHesse);
    }

    if (opt.find("-quiet") == string::npos)
        cout << name << " :  CovQual = " << fitResult->covQual() << ",   Status = " << fitResult->status() << ",   EDM = " << fitResult->edm() << endl;

    map < string, RooPlot * > plots;
    if ( opt.find("-noplot") == string::npos )
    {
        if (isToy) PlotCategories();
        else
        {
            for (unsigned i = 0; i < categories.size(); i++)
            {
                RooRealVar * var = ana[i]->GetVariable();
                if (opt.find("-noinitmodel") == string::npos && opt.find("-noinitdata") == string::npos)
                    plots[(string)categories[i]] = ana[i]->Print(opt + "-nochi2", nbins); //+"-t"+(string)categories[i]);
                else
                {
                    TCanvas * c = new TCanvas();
                    RooPlot * pl = new RooPlot(*var, var->getMin(), var->getMax(), nbins);
                    combData->plotOn(pl, Cut("samples==samples::" + categories[i]));

                    pl->SetTitle("");
                    pl->SetXTitle(((TString)var->GetName()).ReplaceAll("__var__", ""));
                    pl->Draw();
                    c->Print(name + "_" + categories[i] + ".pdf");
                }
            }
        }
    }

    for (auto a : ana) a->SetFitRes(fitResult);

    double logL = combModel->createNLL(*combData)->getVal();
    cout << "\n" << name << ": LogL = " << logL << endl;

    if (opt.find("-corr") != string::npos)
    {
        TCanvas * c = new TCanvas();
        TH2D * hCor = (TH2D*) fitResult->correlationHist();
        hCor->SetLabelSize(0.02, "xyz");
        hCor->Draw("colz");
        c->Print(name + "_corr.pdf");
        c->Print(name + "_corr.C");
        delete hCor;
    }

    return plots;
}


void MultiAnalysis::EnlargeYieldRanges(double factor)
{
    if (!combModel) return;

    RooArgSet * params = combModel->getParameters(RooDataSet());
    TIterator * it = params->createIterator();
    RooRealVar * arg;
    while ( (arg = (RooRealVar*)it->Next()) )
    {
        string varname = (string)arg->GetName();
        if (varname.find("nsig") != string::npos ||
                varname.find("nbkg") != string::npos )
        {
            cout << "Scaling" << endl;
            double val = ((RooRealVar*)arg)->getVal();
            if (val < 0) ((RooRealVar*)arg)->setVal(0);
            double min = ((RooRealVar*)arg)->getMin();
            double max = ((RooRealVar*)arg)->getMax();
            if ( varname.find("nbkg") != string::npos )
                ((RooRealVar*)arg)->setRange(0., max * factor);
            else ((RooRealVar*)arg)->setRange(min, max * factor);
            ((RooRealVar*)arg)->setVal(arg->getVal()*factor);
            arg->Print();
        }
    }
}

void MultiAnalysis::SetConstants(vector<RooDataSet *> input, int index)
{
    RooArgSet * params = combModel->getParameters(RooDataSet());
    TIterator * it = params->createIterator();
    RooRealVar * arg;
    while ( (arg = (RooRealVar*)it->Next()) )
    {
        if (((string)arg->GetName()).find("sample") != string::npos) continue;
        if (!(arg->getAttribute("Constant"))) continue;

        for (size_t ds = 0; ds < input.size(); ds++)
        {
            RooArgSet * set = (RooArgSet *)input[ds]->get(index);
            RooRealVar * var = (RooRealVar *)set->find(arg->GetName());
            if (var) { arg->setVal(var->getVal()); break; }
        }
    }
}

void MultiAnalysis::RandomizeInitialParams(string option)
{
    if (!combModel) return;

    TRandom3 rndm(0);
    size_t posseed = option.find("-seed");
    if (posseed != string::npos)
    {
        int seed = ((TString)(option.substr(posseed + 5))).Atof();
        rndm.SetSeed(seed);
    }

    cout << endl;
    cout << name << ": Randomize (" << option << ")" << endl;

    RooArgSet * params = combModel->getParameters(RooDataSet());
    TIterator * it = params->createIterator();
    RooRealVar * arg;
    while ( (arg = (RooRealVar*)it->Next()) )
    {
        if (((string)arg->GetName()).find("sample") != string::npos) continue;

        if (option.find("-free") != string::npos && arg->getAttribute("Constant") ) continue;
        if (option.find("-cost") != string::npos && !(arg->getAttribute("Constant")) ) continue;

        double val = 0;
        double oldv = arg->getVal();

        double sigma = arg->getError();
        double min = oldv - 3 * sigma;
        double max = oldv + 3 * sigma;

        if (option.find("-range") != string::npos)
        {
            min = arg->getMin();
            max = arg->getMax();
            sigma = (max - min) / 10;
        }

        if (option.find("-gauss") != string::npos) val = rndm.Gaus(oldv, sigma);
        else  val = min + (max - min) * rndm.Uniform();
        cout << "Randomize " << arg->GetName() << " ===>  Old value: " << oldv << "  new value: " << val << " error: " << sigma << endl;
        arg->setVal(val);
    }
    cout << endl;
}




RooWorkspace * MultiAnalysis::SaveToRooWorkspace()
{
    RooWorkspace * ws = new RooWorkspace("ws_" + name);
    cout << endl << name << ": SaveToRooWorkspace" << endl;
    /*
        for(size_t aa = 0; aa < ana.size(); aa++)
        {
        TIterator * it = ana[aa]->SaveToRooWorkspace()->componentIterator();
        TObject * arg;
        while( (arg=(TObject *)it->Next()) ) ws->import(*arg);
        }
        cout << endl;
        */
    if (combModel)
    {
        ws->import(*combModel);
        cout << "combModel: " << combModel->GetName() << endl;
    }
    if (combData)
    {
        ws->import(*combData);
        cout << "combData: " << combData->GetName() << endl;
    }

    return ws;
}

void MultiAnalysis::ImportModel(RooWorkspace * ws)
{
    cout << endl;
    cout << "ImportModel" << endl;
    cout << endl;
    TIterator * it = ws->componentIterator();
    TObject * arg;
    while ( (arg = (TObject *)it->Next()) )
    {
        string name = arg->GetName();
        if (name.find("combModel") != string::npos) {
            combModel = (RooSimultaneous*)arg;
            printParams(combModel);
        }
    }
    cout << endl;
}

void MultiAnalysis::ImportData(RooWorkspace * ws)
{
    cout << endl << name << ": ImportData" << endl;

    list<RooAbsData *> mylist = ws->allData();
    for (std::list<RooAbsData *>::iterator it = mylist.begin(); it != mylist.end(); ++it)
    {
        string name = (*it)->GetName();
        if (name.find("combData") != string::npos)
            combData = (RooDataSet*)(*it);
    }

    if (!combData) cout << "Data not found in work space" << endl;
    else init = true;
}


RooDataSet * MultiAnalysis::Generate(int nevts, string option)
{
    transform(option.begin(), option.end(), option.begin(), ::tolower);
    cout << endl << name << ": Generating " << nevts << " events (" << option << ")" << endl;

    if (!combModel) { cout << "No model set! Skipping generation..." << endl; return NULL; }

    cout << "Generating toys with roofit function" << endl;
    size_t posseed = option.find("-seed");
    if (posseed != string::npos)
    {
        int seed = ((TString)(option.substr(posseed + 5))).Atof();
        RooRandom::randomGenerator()->SetSeed(seed);
    }

    RooCmdArg ext = RooCmdArg::none();
    if (option.find("-genextended") != string::npos) ext = Extended();
    RooArgSet genvars = RooArgSet(*vars);
    genvars.add(*samples);
    combData = combModel->generate(genvars, nevts, ext);
    cout << name << ": " << " Generated events = " << combData->numEntries() << endl;
    combData->Print();
    isToy = true;
    return combData;
}

RooPlot * MultiAnalysis::PrintSum(string option, TString dovar, string printname, int nbins)
{
    cout << name << ": " << " PrintSum " << printname << " " << dovar << endl;

    TCanvas * c = new TCanvas();
    size_t posb = option.find("-nbins");
    if (posb != string::npos)
        nbins = ((TString) option.substr(posb + 6, string::npos)).Atof();

    if (!init && option.find("-noinit") == string::npos) Initialize();

    TLegend * leg = new TLegend(0.65, 0.7, 0.76, 0.9);
    if (option.find("-leg") != string::npos)
    {
        size_t pos = option.find("-leg") + 5;
        string ss = option.substr(pos, string::npos);
        double x1 = (TString(ss)).Atof();
        pos = ss.find(",") + 1;
        ss = ss.substr(pos, string::npos);
        double y1 = (TString(ss)).Atof();
        pos = ss.find(",") + 1;
        ss = ss.substr(pos, string::npos);
        double x2 = (TString(ss)).Atof();
        pos = ss.find(",") + 1;
        ss = ss.substr(pos, string::npos);
        double y2 = (TString(ss)).Atof();

        leg = new TLegend(x1, y1, x2, y2);
    }

    RooArgSet pdfs("pdfs");
    RooArgSet fracs("fracs");
    RooArgSet totpdfs("totpdfs");
    RooArgSet totfracs("totfracs");
    RooArgSet sigpdfs("sigpdfs");
    RooArgSet sigfracs("sigfracs");
    map< string, RooArgSet > bkgpdfs;
    map< string, RooArgSet > bkgfracs;
    map< string, double > sumNbkg;
    double sumAll = 0, sumNsig = 0, sumNtotbkg = 0;
    int k = 0;
    vector <string> bnames;
    vector <string> print_cats;
    bool first = true;

    for (unsigned i = 0; i < categories.size(); i++)
    {
        if (dovar != "") { if (ana[i]->GetVariable()->GetName() != dovar) continue; }
        else if (ana[i]->GetVariable()->GetName() != ana[0]->GetVariable()->GetName()) continue;
        if ( printname != "" && ((string)categories[i]).find(printname) == string::npos ) continue;
        k = i;

        print_cats.push_back((string)categories[i]);
        pdfs.add(*ana[i]->GetModel());
        RooRealVar * curfrac = new RooRealVar("frac" + categories[i], "", ana[i]->GetDataSet()->sumEntries());
        fracs.add(*curfrac);
        sumAll += curfrac->getVal();

        sigpdfs.add(*ana[i]->GetSig());
        sigfracs.add(*ana[i]->GetNSigPtr());
        sumNsig += ana[i]->GetNSigPtr()->getVal();

        vector <RooAbsPdf *> components = ana[i]->GetBkgComponents();
        for (unsigned bb = 0; bb < components.size(); bb++)
        {
            size_t pos1 = ((string)components[bb]->GetName()).find("_");
            size_t pos2 = ((string)components[bb]->GetName()).find("_", pos1 + 1);
            string bname = ((string)components[bb]->GetName()).substr(0, pos2);

            if (first)
            {
                bkgpdfs[bname] = RooArgSet("bkg" + (TString)bname);
                bkgfracs[bname] = RooArgSet("bkgfracs" + (TString)bname);
                sumNbkg[bname] = 0;
                bnames.push_back(bname);
            }

            bkgpdfs[bname].add(*components[bb]);
            bkgfracs[bname].add((*ana[i]->GetBkgFractions()[bb]));
            totpdfs.add(*components[bb]);
            totfracs.add((*ana[i]->GetBkgFractions()[bb]));
            sumNbkg[bname] += ana[i]->GetBkgFractions()[bb]->getVal();
            sumNtotbkg += ana[i]->GetBkgFractions()[bb]->getVal();
        }
        first = false;
    }

    RooPlot * pl = ana[k]->Print("-empty" + option, nbins);
    RooAddPdf * sumPdf = new RooAddPdf("sumPdf", "", pdfs, fracs);

    TString Xtitle = pl->GetXaxis()->GetTitle();

    TString legstyle = "l";
    if (option.find("-fillbkg") != string::npos) legstyle = "f";
    TString siglabel = getLegendLabel(ana[k]->GetSig()->GetTitle(), option);

    TString cut = "";
    for (size_t cc = 0; cc < print_cats.size(); cc++)
    {
        cut += "samples == samples::" + print_cats[cc];
        if (cc != (print_cats.size() - 1) ) cut += " || ";
    }

    //combData->plotOn(pl,MarkerSize(1),Cut(cut));
    RooAbsData * slice = combData->reduce(cut);
    slice->SetName("combData");
    slice->plotOn(pl, MarkerSize(1));

    string dataname = "h_";
    dataname += slice->GetName();
    double min = 1e9;
    RooHist *hist = pl->getHist(dataname.c_str());
    Double_t *cont = hist->GetY();
    for (int i = 0; i < hist->GetN(); i++)
    {
        if (cont[i] != 0) min = TMath::Min(min, cont[i]);
    }
    if (min == 1e9) min = 1;

    RooAddPdf * sumsigPdf = new RooAddPdf("sumsigPdf", "", sigpdfs, sigfracs);
    RooAddPdf * sumtotbkgPdf = new RooAddPdf("sumtotbkgPdf", "", totpdfs, totfracs);

    if (option.find("-nototsigplot") == string::npos)
    {
        sumsigPdf->plotOn(pl,
                          LineStyle(kDashed),
                          LineColor(kBlack),
                          Normalization(sumNsig, RooAbsReal::NumEvent),
                          Name("sumSigPdf"));
        leg->AddEntry(pl->findObject("sumSigPdf"), siglabel, "l");
    }

    vector <Color_t> colors = GetDefaultColors();
    if (ana[k]->GetColors().size() > 0) colors = ana[k]->GetColors();
    int styles[] = {3, 4, 5, 6, 7, 8, 9, 10, 3, 4, 5, 6, 7, 8, 9, 10};
    int counter = 0;
    //if(option.find("-stackbkg")==string::npos) counter++;
    if (option.find("-plottotbkg") != string::npos)
    {
        sumtotbkgPdf->plotOn(pl, LineStyle(styles[counter]), LineColor(colors[counter]), Normalization( sumNtotbkg , RooAbsReal::NumEvent));
        counter++;
    }
    if (option.find("-stack") != string::npos)
    {
        RooArgSet stackedFracs("stackedFracs"), stackedPdfs("stackedPdfs");
        double curSum = 0;
        for (unsigned bb = 0; bb < bnames.size(); bb++)
        {
            string bname = bnames[bb];
            stackedPdfs.add(bkgpdfs[bname]);
            stackedFracs.add(bkgfracs[bname]);
            curSum += sumNbkg[bname];
            RooAddPdf * curSumBkgPdf = new RooAddPdf("sumBkgPdf_" + (TString)bname, "", stackedPdfs, stackedFracs);
            TString leglabel = getLegendLabel(ana[k]->GetBkgComponents()[bb]->GetTitle(), option);

            if (option.find("-fillbkg") != string::npos)
            {
                curSumBkgPdf->plotOn(pl,
                                     FillColor(colors[counter]),
                                     FillStyle(1001),
                                     FillStyle(1001),
                                     LineWidth(0.),
                                     LineStyle(0),
                                     LineColor(colors[counter]),
                                     DrawOption("F"),
                                     Normalization(curSum, RooAbsReal::NumEvent),
                                     MoveToBack(),
                                     Name("sumBkgPdf_" + (TString)bname));
                leg->AddEntry(pl->findObject("sumBkgPdf_" + (TString)bname), leglabel, "f");
            }
            else
            {
                curSumBkgPdf->plotOn(pl,
                                     LineStyle(styles[counter]),
                                     LineColor(colors[counter]),
                                     Normalization(curSum, RooAbsReal::NumEvent),
                                     Name("sumBkgPdf_" + (TString)bname));
                leg->AddEntry(pl->findObject("sumBkgPdf_" + (TString)bname), leglabel, "l");
            }
            counter++;
        }
    }
    else {

        for (unsigned bb = 0; bb < bnames.size(); bb++)
        {
            string bname = bnames[bb];
            RooAddPdf * curSumBkgPdf = new RooAddPdf("sumBkgPdf_" + (TString)bname, "", bkgpdfs[bname], bkgfracs[bname]);
            curSumBkgPdf->plotOn(pl,
                                 LineStyle(styles[counter]),
                                 LineColor(colors[counter]),
                                 Normalization(sumNbkg[bname], RooAbsReal::NumEvent),
                                 Name("sumBkgPdf_" + (TString)bname));
            TString leglabel = getLegendLabel(ana[k]->GetBkgComponents()[bb]->GetTitle(), option);
            leg->AddEntry(pl->findObject("sumBkgPdf_" + (TString)bname), leglabel, "l");
            counter++;
        }
    }

    sumPdf->plotOn(pl, Normalization(sumAll, RooAbsReal::NumEvent));

    if (option.find("-lhcb") != string::npos)
    {
        double x1 = 0.05, x2 = 0.25, y1 = 0.80, y2 = 0.97;
        size_t pos = option.find("-lhcb[") + 6;
        if ( pos != string::npos )
        {
            string ss = option.substr(pos, string::npos);
            x1 = (TString(ss)).Atof();
            pos = ss.find(",") + 1;
            ss = ss.substr(pos, string::npos);
            y1 = (TString(ss)).Atof();
            pos = ss.find(",") + 1;
            ss = ss.substr(pos, string::npos);
            x2 = (TString(ss)).Atof();
            pos = ss.find(",") + 1;
            ss = ss.substr(pos, string::npos);
            y2 = (TString(ss)).Atof();
        }

        TPaveText * tbox = new TPaveText(gStyle->GetPadLeftMargin() + x1,
                                         y1 - gStyle->GetPadTopMargin(),
                                         gStyle->GetPadLeftMargin() + x2,
                                         y2 - gStyle->GetPadTopMargin(),
                                         "BRNDC");

        if (option.find("-lhcbdx") != string::npos)
        {
            tbox = new TPaveText(gStyle->GetPadRightMargin() + 0.63,
                                 0.85 - gStyle->GetPadTopMargin(),
                                 gStyle->GetPadRightMargin() + 0.83,
                                 0.97 - gStyle->GetPadTopMargin(),
                                 "BRNDC");
        }

        tbox->AddText("LHCb");
        tbox->SetFillStyle(0);
        tbox->SetTextAlign(12);
        tbox->SetBorderSize(0);
        pl->addObject(tbox);
    }

    pl->SetMinimum(0);
    pl->SetTitle("");
    if (option.find("-noborder") != string::npos )
        leg->SetBorderSize(0);
    pl->addObject(leg);

    pl->Draw();

    c->Print("fit_" + (TString)printname + ".pdf");
    if (option.find("-allformats") != string::npos)
    {
        c->Print("fit_" + (TString)printname + ".C");
        c->Print("fit_" + (TString)printname + ".eps");
        c->Print("fit_" + (TString)printname + ".png");
    }

    pl->SetMinimum(min);
    c->SetLogy();
    c->Print("fit_" + (TString)printname + "_log.pdf");
    if (option.find("-allformats") != string::npos)
    {
        c->Print("fit_" + (TString)printname + "_log.C");
        c->Print("fit_" + (TString)printname + "_log.eps");
        c->Print("fit_" + (TString)printname + "_log.png");
    }

    string data = "h_";
    data += slice->GetName();
    string model = sumPdf->GetName();
    model += "_Norm[" + dovar + "]";
    TH1 * residuals = getPulls(pl, NULL, "p", data, model);
    if (residuals && option.find("-andpulls") != string::npos)
    {
        TPad * plotPad = new TPad("plotPad", "", .005, .25, .995, .995);
        TPad * resPad = new TPad("resPad", "", .005, .015, .995, .248);
        if (option.find("-attach") != string::npos)
        {
            plotPad = new TPad("plotPad", "", .005, .35, .995, .995);
            resPad = new TPad("resPad", "", .005, .005, .995, .35);

            residuals->SetMinimum(-6.);
            residuals->SetMaximum(6.);
        }

        plotPad->Draw();
        resPad->Draw();

        resPad->cd();
        if (option.find("-attach") != string::npos)
        {
            gPad->SetTopMargin(1e-5);
            gPad->SetBottomMargin(0.4);
        }

        TAxis* yAxis = residuals->GetYaxis();
        TAxis* xAxis = residuals->GetXaxis();

        yAxis->SetNdivisions(505);
        double old_size = yAxis->GetLabelSize();
        yAxis->SetLabelSize( old_size * 2 );
        yAxis->SetTitleSize( old_size * 2.3 );
        yAxis->SetTitleOffset(0.5);
        yAxis->SetTitle("Pulls");
        xAxis->SetLabelSize( xAxis->GetLabelSize() / 0.33 );
        if (option.find("-attach") != string::npos)
        {
            xAxis->SetTitle(Xtitle);
            xAxis->SetTitleSize(old_size * 3);
            xAxis->SetTitleOffset(0.9);
        }

        resPad->cd();
        residuals->Draw();

        TLine *lc = new TLine(xAxis->GetXmin(),  0, xAxis->GetXmax(),  0);
        TLine *lu = new TLine(xAxis->GetXmin(),  3, xAxis->GetXmax(),  3);
        TLine *ld = new TLine(xAxis->GetXmin(), -3, xAxis->GetXmax(), -3);

        lc->SetLineColor(kGray + 2);
        lu->SetLineColor(kGray + 1);
        ld->SetLineColor(kGray + 1);

        lc->SetLineStyle(2);
        lu->SetLineStyle(2);
        ld->SetLineStyle(2);

        lc->Draw("same");
        lu->Draw("same");
        ld->Draw("same");

        residuals->Draw("same");
        plotPad->cd();

        pl->SetMinimum(0);
        if (option.find("-attach") != string::npos)
        {
            gPad->SetBottomMargin(1e-5);
            pl->SetMinimum(0.01);
        }

        pl->Draw();

        c->Print("fit_" + (TString)printname + "_fitAndRes.pdf");
        if (option.find("-allformats") != string::npos)
        {
            c->Print("fit_" + (TString)printname + "_fitAndRes.C");
            c->Print("fit_" + (TString)printname + "_fitAndRes.eps");
            c->Print("fit_" + (TString)printname + "_fitAndRes.png");
        }

        pl->SetMinimum(min);
        if (residuals && option.find("-andpulls") != string::npos)
            plotPad->SetLogy();
        else
            c->SetLogy();
        c->Print("fit_" + (TString)printname + "_log_fitAndRes.pdf");
        if (option.find("-allformats") != string::npos)
        {
            c->Print("fit_" + (TString)printname + "_log_fitAndRes.C");
            c->Print("fit_" + (TString)printname + "_log_fitAndRes.eps");
            c->Print("fit_" + (TString)printname + "_log_fitAndRes.png");
        }
    }

    return pl;

}







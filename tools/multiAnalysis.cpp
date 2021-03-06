#include "multiAnalysis.hpp"


string MultiAnalysis::m_pmode = "v";


void MultiAnalysis::AddCategory(Analysis * ana, TString nameCat)
{
    m_samples->defineType(nameCat);
    m_categories.push_back(nameCat);
    m_ana.push_back(ana);
    m_constr->add(*(ana->GetConstraints()));

    if (m_vars->getSize() == 0) m_vars->add(*ana->GetVariable());
    else if (!m_vars->find(ana->GetVariable()->GetName()))
        m_vars->add(*ana->GetVariable());
}

void MultiAnalysis::AddCategory(TString nameCat, RooRealVar * var, string opt)
{
    transform(opt.begin(), opt.end(), opt.begin(), ::tolower);

    m_samples->defineType(nameCat);
    m_categories.push_back(nameCat);

    if (m_vars->getSize() == 0) m_vars->add(*var);
    else if (!m_vars->find(var->GetName())) m_vars->add(*var);

    if (opt == "setana")
    {
        Analysis * newana = new Analysis(nameCat, var);
        m_ana.push_back(newana);
    }
}

void MultiAnalysis::PlotCategories()
{
    TCanvas * c = new TCanvas();
    for (unsigned i = 0; i < m_categories.size(); i++)
    {
        RooPlot* frame = m_ana[i]->GetVariable()->frame();
        TString cut = "samples==samples::" + m_categories[i];

        cout << m_name << ": Category: " << m_categories[i] << endl;
        m_combData->Print();
        m_combModel->Print();

        m_combData->plotOn(frame, Cut(cut));
        RooDataSet::setDefaultStorageType(RooAbsData::Vector);
        RooAbsData * reddata = m_combData->reduce(cut);
        m_combModel->getPdf(m_categories[i])->plotOn(frame, Normalization(reddata->numEntries(), RooAbsReal::NumEvent));

        frame->SetTitle("");
        frame->SetXTitle(((TString)m_ana[i]->GetVariable()->GetName()).ReplaceAll("__var__", ""));
        frame->Draw();
        c->Print(m_name + "_" + m_categories[i] + ".pdf");

    }
}

bool MultiAnalysis::Initialize(string opt)
{
    if (m_ana[0]) m_pmode = (string) m_ana[0]->GetPrintLevel();

    transform(opt.begin(), opt.end(), opt.begin(), ::tolower);
    if (m_pmode == "v") cout << endl << m_name << ": Initialize " << opt << endl;

    bool nomodel = false, nodata = false;
    if (opt.find("-noinitmodel") != string::npos) nomodel = true;
    if (opt.find("-noinitdata") != string::npos) nodata = true;

    map<string, RooDataSet*> mymap;
    map<string, TH1*> myhmap;
    if (!nomodel)
    {
        m_combModel = new RooSimultaneous("combModel", "", *m_samples);
        for (unsigned i = 0; i < m_categories.size(); i++)
        {
            if (opt.find("-initialize") != string::npos)
            {
                if (!m_unbinned) m_ana[i]->SetBinnedFit(m_nBins);
                m_ana[i]->Initialize(opt);
            }
            if (!m_ana[i]->isValid()) { cout << m_name << ": *** WARNING Initialize *** " << m_ana[i]->GetName() << " is not initialized!" << endl; return false; }
            m_combModel->addPdf(*(m_ana[i]->GetModel()), m_categories[i]);
            mymap[(string)m_categories[i]]  = (RooDataSet*)(m_ana[i]->GetDataSet(opt));
            myhmap[(string)m_categories[i]] = (TH1*)(m_ana[i]->GetDataHisto(opt));
        }
    }
    if (m_pmode == "v") if (m_combModel) m_combModel->Print();

    if (!nodata)
    {
        if (m_pmode == "v") cout << endl << m_name << ": CreateCombData" << endl;
        m_combData = new RooDataSet("combData", "combined datas", *m_vars, Index(*m_samples), Import(mymap));
        if (m_pmode == "v") if (m_combData) m_combData->Print();
        if (!m_unbinned)
        {
            if (m_pmode == "v") cout << endl << m_name << ": CreateCombHist" << endl;
            //m_combHist = new RooDataHist("combHisto", "combined histos", *m_vars, *m_combData);
            m_combHist = new RooDataHist("combHisto", "combined histos", *m_vars, Index(*m_samples), Import(myhmap));
            if (m_pmode == "v") if (m_combHist) m_combHist->Print();
        }
    }

    cout << endl << m_name << ": MultiAnalysis " << m_name << " initialized correctly" << endl;

    m_init = true;
    return m_init;
}

map < string, RooPlot * > MultiAnalysis::Fit(unsigned nbins, string opt)
{
    transform(opt.begin(), opt.end(), opt.begin(), ::tolower);
    if (!m_init) Initialize(opt);

    if (m_pmode == "v")
    {
        cout << endl << m_name << ": Fit ";
        (m_unbinned) ? cout << "unbinned " : cout << "binned " << nbins << " ";
        cout << opt << endl;
    }

    int ncpu = 1;
    size_t poscpu = opt.find("-ncpu");
    if (poscpu != string::npos) ncpu = atoi(opt.substr(poscpu + 5).c_str());
    if (m_pmode == "v") cout << m_name << ": Using " << ncpu << " CPU(s)" << endl << endl;

    RooCmdArg constraints     = ExternalConstraints(*m_constr);
    //RooCmdArg fitRange        = Range("","","");
    //for (unsigned i = 0; i < m_categories.size(); i++)
    //    m_ana[i]->GetVariable()->getRange("FitRange", min, max);
    RooCmdArg isExtended      = Extended(kTRUE);
    if (opt.find("-noextended") != string::npos) isExtended = Extended(kFALSE);
    RooCmdArg isQuiet         = PrintLevel(2);
    if (opt.find("-quiet") != string::npos) isQuiet = PrintLevel(-1);
    RooCmdArg offset          = Offset(kTRUE);
    RooCmdArg save            = Save(kTRUE);
    RooCmdArg splitRange      = SplitRange(kTRUE);
    RooCmdArg sumw2           = SumW2Error(kFALSE);
    if (opt.find("-sumw2err") != string::npos) sumw2 = SumW2Error(kTRUE);
    RooCmdArg useCPU          = NumCPU(ncpu);
    RooCmdArg useHesse        = Hesse(kTRUE);
    if (opt.find("-nohesse") != string::npos) useHesse = Hesse(kFALSE);
    RooCmdArg useInitialHesse = InitialHesse(kFALSE);
    if (opt.find("-initialhesse") != string::npos) useInitialHesse = InitialHesse(kTRUE);
    RooCmdArg useMinos        = Minos(kFALSE);
    if (opt.find("-minos") != string::npos) useMinos = Minos(kTRUE);
    RooCmdArg useTimer        = Timer(kTRUE);
    RooCmdArg warnings        = Warnings(kFALSE);

    RooLinkedList optList;
    optList.Add((TObject *) & constraints);
    //optList.Add((TObject *) & fitRange);
    optList.Add((TObject *) & isExtended);
    optList.Add((TObject *) & isQuiet);
    optList.Add((TObject *) & offset);
    optList.Add((TObject *) & save);
    optList.Add((TObject *) & splitRange);
    optList.Add((TObject *) & sumw2);
    optList.Add((TObject *) & useCPU);
    optList.Add((TObject *) & useHesse);
    optList.Add((TObject *) & useInitialHesse);
    optList.Add((TObject *) & useMinos);
    optList.Add((TObject *) & useTimer);
    //optList.Add((TObject *) & warnings);

    if (!m_combModel || (!m_combData && !m_combHist)) { cout << m_name << ": *** WARNING Fit *** Model or data not set!" << endl; return map < string, RooPlot * >(); }

    RooAbsData * mydata = m_combData;
    if (!m_unbinned) mydata = m_combHist;

    time_t _tstart = time(NULL);
    m_fitRes = m_combModel->fitTo(*mydata, optList);
    time_t _tstop = time(NULL);

    if (m_pmode == "v") {
        if (m_fitRes)
        {
            cout << endl << m_name;
            cout << ":   CovQual = " << m_fitRes->covQual();
            cout << ",   Status = "  << m_fitRes->status();
            cout << ",   EDM = "     << m_fitRes->edm();
            cout << ",   LogL = "    << m_fitRes->minNll() << endl;
        }
        cout << endl << m_name << ": Time = " << difftime(_tstop, _tstart) << "s" << endl << endl;
    }

    map < string, RooPlot * > plots;
    if (opt.find("-noplot") == string::npos)
    {
        if (m_isToy) PlotCategories();
        else
        {
            for (unsigned i = 0; i < m_categories.size(); i++)
            {
                RooRealVar * var = m_ana[i]->GetVariable();
                if (opt.find("-noinitmodel") == string::npos && opt.find("-noinitdata") == string::npos)
                    plots[(string)m_categories[i]] = m_ana[i]->Print(opt + "-nochi2", nbins); //+"-t"+(string)m_categories[i]);
                else
                {
                    TCanvas * c = new TCanvas();
                    RooPlot * pl = new RooPlot(*var, var->getMin(), var->getMax(), nbins);
                    m_combData->plotOn(pl, Cut("samples==samples::" + m_categories[i]));

                    pl->SetTitle("");
                    pl->SetXTitle(((TString)var->GetName()).ReplaceAll("__var__", ""));
                    pl->Draw();
                    c->Print(m_name + "_" + m_categories[i] + ".pdf");
                }
            }
        }
    }

    for (auto a : m_ana) a->SetFitRes(m_fitRes);

    double logL = m_combModel->createNLL(*m_combData)->getVal();
    cout << endl << m_name << ": LogL = " << logL << endl;

    if (opt.find("-corr") != string::npos)
    {
        TCanvas * c = new TCanvas();
        TH2D * hCor = (TH2D*) m_fitRes->correlationHist();
        hCor->SetLabelSize(0.02, "xyz");
        hCor->Draw("colz");
        c->Print(m_name + "_corr.pdf");
        c->Print(m_name + "_corr.C");
        delete hCor;
    }

    return plots;
}

void MultiAnalysis::EnlargeYieldRanges(double factor)
{
    if (!m_combModel) return;

    RooArgSet * params = m_combModel->getParameters(RooDataSet());
    TIterator * it = params->createIterator();
    RooRealVar * arg;
    while ((arg = (RooRealVar*)it->Next()))
    {
        string varname = (string)arg->GetName();
        if (varname.find("nsig") != string::npos ||
                varname.find("nbkg") != string::npos)
        {
            cout << "Scaling ..." << endl;
            double val = ((RooRealVar*)arg)->getVal();
            if (val < 0) ((RooRealVar*)arg)->setVal(0);
            double min = ((RooRealVar*)arg)->getMin();
            double max = ((RooRealVar*)arg)->getMax();
            if (varname.find("nbkg") != string::npos)
                ((RooRealVar*)arg)->setRange(0., max * factor);
            else ((RooRealVar*)arg)->setRange(min, max * factor);
            ((RooRealVar*)arg)->setVal(arg->getVal()*factor);
            arg->Print();
        }
    }
}

void MultiAnalysis::SetConstants(vector<RooDataSet *> input, int index)
{
    RooArgSet * params = m_combModel->getParameters(RooDataSet());
    TIterator * it = params->createIterator();
    RooRealVar * arg;
    while ((arg = (RooRealVar*)it->Next()))
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
    if (!m_combModel) return;

    TRandom3 rndm(0);
    size_t posseed = option.find("-seed");
    if (posseed != string::npos)
    {
        int seed = ((TString)(option.substr(posseed + 5))).Atof();
        rndm.SetSeed(seed);
    }

    if (m_pmode == "v") cout << endl << m_name << ": Randomize (" << option << ")" << endl;

    RooArgSet * params = m_combModel->getParameters(RooDataSet());
    TIterator * it = params->createIterator();
    RooRealVar * arg;
    while ((arg = (RooRealVar*)it->Next()))
    {
        if (((string)arg->GetName()).find("sample") != string::npos) continue;

        if (option.find("-free") != string::npos && arg->getAttribute("Constant")) continue;
        if (option.find("-cost") != string::npos && !(arg->getAttribute("Constant"))) continue;

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
        if (m_pmode == "v") cout << "Randomize " << arg->GetName() << " ===>  Old value: " << oldv << "  new value: " << val << " error: " << sigma << endl;
        arg->setVal(val);
    }
    cout << endl;
}

RooWorkspace * MultiAnalysis::SaveToRooWorkspace()
{
    if (m_pmode == "v") cout << endl << m_name << ": SaveToRooWorkspace" << endl;
    RooWorkspace * ws = new RooWorkspace("ws_" + m_name);
    /*
        for(size_t aa = 0; aa < m_ana.size(); aa++)
        {
        TIterator * it = m_ana[aa]->SaveToRooWorkspace()->componentIterator();
        TObject * arg;
        while((arg=(TObject *)it->Next())) ws->import(*arg);
        }
        cout << endl;
        */
    if (m_combModel)
    {
        ws->import(*m_combModel);
        if (m_pmode == "v") cout <<  m_name << ": combModel = " << m_combModel->GetName() << endl;
    }
    if (m_combData)
    {
        ws->import(*m_combData);
        if (m_pmode == "v") cout <<  m_name << ":  combData = " << m_combData->GetName() << endl;
    }

    return ws;
}

void MultiAnalysis::ImportModel(RooWorkspace * ws)
{
    if (m_pmode == "v") cout << endl << m_name << ": ImportModel" << endl << endl;
    TIterator * it = ws->componentIterator();
    TObject * arg;
    while ((arg = (TObject *)it->Next()))
    {
        string name = arg->GetName();
        if (name.find("combModel") != string::npos) {
            m_combModel = (RooSimultaneous*)arg;
            if (m_pmode == "v")
            {
                cout << m_name << ": combModel = " << name << endl;
                printParams(m_combModel);
                cout << endl;
            }
        }
    }
}

void MultiAnalysis::ImportData(RooWorkspace * ws)
{
    if (m_pmode == "v") cout << endl << m_name << ": ImportData" << endl;

    list<RooAbsData *> mylist = ws->allData();
    for (std::list<RooAbsData *>::iterator it = mylist.begin(); it != mylist.end(); ++it)
    {
        string name = (*it)->GetName();
        if (name.find("combData") != string::npos) {
            m_combData = (RooDataSet*)(*it);
            if (m_pmode == "v")
            {
                cout << m_name << ": combData = " << name << endl;
                m_combData->Print();
                cout << endl;
            }
        }
    }

    if (!m_combData) cout << m_name << ": *** WARNING ImportData *** Data not found in work space!" << endl;
    else m_init = true;
}

RooDataSet * MultiAnalysis::Generate(int nevts, string option)
{
    transform(option.begin(), option.end(), option.begin(), ::tolower);
    if (m_pmode == "v") cout << endl << m_name << ": Generate " << nevts << " events (" << option << ")" << endl;

    if (!m_combModel) { cout << m_name << ": *** WARNING Generate *** No model set!" << endl; return NULL; }

    //cout << "Generating toys with roofit function" << endl;
    size_t posseed = option.find("-seed");
    if (posseed != string::npos)
    {
        int seed = ((TString)(option.substr(posseed + 5))).Atof();
        RooRandom::randomGenerator()->SetSeed(seed);
    }

    RooCmdArg ext = RooCmdArg::none();
    if (option.find("-genextended") != string::npos) ext = Extended();
    RooArgSet genvars = RooArgSet(*m_vars);
    genvars.add(*m_samples);
    m_combData = m_combModel->generate(genvars, nevts, ext);
    cout << m_name << ": " << " Generated events = " << m_combData->numEntries() << endl;
    m_combData->Print();
    m_isToy = true;
    return m_combData;
}

RooPlot * MultiAnalysis::PrintSum(string option, TString dovar, string printname, int nbins)
{
    if (m_pmode == "v") cout << endl << m_name << ": PrintSum " << option << " " << dovar << " " << printname << " " << nbins << endl << endl;


    TCanvas * c = new TCanvas();

    if (!m_init && option.find("-noinit") == string::npos) Initialize();

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

    for (unsigned i = 0; i < m_categories.size(); i++)
    {
        if (dovar != "") { if (m_ana[i]->GetVariable()->GetName() != dovar) continue; }
        else if (m_ana[i]->GetVariable()->GetName() != m_ana[0]->GetVariable()->GetName()) continue;
        if (printname != "" && ((string)m_categories[i]).find(printname) == string::npos) continue;
        k = i;

        print_cats.push_back((string)m_categories[i]);
        pdfs.add(*m_ana[i]->GetModel());
        RooRealVar * curfrac = new RooRealVar("frac" + m_categories[i], "", m_ana[i]->GetDataSet()->sumEntries());
        fracs.add(*curfrac);
        sumAll += curfrac->getVal();

        sigpdfs.add(*m_ana[i]->GetSig());
        sigfracs.add(*m_ana[i]->GetNSigPtr());
        sumNsig += m_ana[i]->GetNSigPtr()->getVal();

        vector <RooAbsPdf *> components = m_ana[i]->GetBkgComponents();
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
            bkgfracs[bname].add((*m_ana[i]->GetNBkgPtrs()[bb]));
            totpdfs.add(*components[bb]);
            totfracs.add((*m_ana[i]->GetNBkgPtrs()[bb]));
            sumNbkg[bname] += m_ana[i]->GetNBkgPtrs()[bb]->getVal();
            sumNtotbkg += m_ana[i]->GetNBkgPtrs()[bb]->getVal();
        }
        first = false;
    }

    RooPlot * pl = m_ana[k]->Print("-empty" + option, nbins);
    RooAddPdf * sumPdf = new RooAddPdf("sumPdf", "", pdfs, fracs);

    TString Xtitle = pl->GetXaxis()->GetTitle();
    size_t posX = option.find("-x");
    if (posX != string::npos) Xtitle = option.substr(posX + 2, option.find("-", posX + 2) - posX - 2);
    //if (m_unit != "") Xtitle += " [" + m_unit + "]";
    if (Xtitle != "") pl->SetXTitle(((TString)Xtitle).ReplaceAll("__var__", ""));

    TString legstyle = "l";
    if (option.find("-fillbkg") != string::npos) legstyle = "f";
    TString siglabel = getLegendLabel(m_ana[k]->GetSig()->GetTitle(), option);

    TString cut = "";
    for (size_t cc = 0; cc < print_cats.size(); cc++)
    {
        cut += "samples == samples::" + print_cats[cc];
        if (cc != (print_cats.size() - 1)) cut += " || ";
    }

    //m_combData->plotOn(pl,MarkerSize(1),Cut(cut));
    RooAbsData * slice = m_combData->reduce(cut);
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

    vector <Color_t> colors = getDefaultColors();
    if (m_ana[k]->GetColors().size() > 0) colors = m_ana[k]->GetColors();
    int styles[] = {3, 4, 5, 6, 7, 8, 9, 10, 3, 4, 5, 6, 7, 8, 9, 10};
    int counter = 0;
    //if (option.find("-stackbkg")==string::npos) counter++;
    if (option.find("-plottotbkg") != string::npos)
    {
        sumtotbkgPdf->plotOn(pl, LineStyle(styles[counter]), LineColor(colors[counter]), Normalization(sumNtotbkg , RooAbsReal::NumEvent));
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
            TString leglabel = getLegendLabel(m_ana[k]->GetBkgComponents()[bb]->GetTitle(), option);

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
            TString leglabel = getLegendLabel(m_ana[k]->GetBkgComponents()[bb]->GetTitle(), option);
            leg->AddEntry(pl->findObject("sumBkgPdf_" + (TString)bname), leglabel, "l");
            counter++;
        }
    }

    sumPdf->plotOn(pl, Normalization(sumAll, RooAbsReal::NumEvent));

    if (option.find("-lhcb") != string::npos)
    {
        double x1 = 0.05, x2 = 0.25, y1 = 0.80, y2 = 0.97;
        size_t pos = option.find("-lhcb[") + 6;
        if (pos != string::npos)
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
    if (option.find("-noborder") != string::npos)
        leg->SetBorderSize(0);
    pl->addObject(leg);


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
        yAxis->SetLabelSize(old_size * 2);
        yAxis->SetTitleSize(old_size * 2.3);
        yAxis->SetTitleOffset(0.5);
        yAxis->SetTitle("Pulls");
        xAxis->SetLabelSize(xAxis->GetLabelSize() / 0.33);

        if (option.find("-attach") != string::npos)
        {
            if (Xtitle != "") xAxis->SetTitle(((TString)Xtitle).ReplaceAll("__var__", ""));
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
        c->Print((TString) printname + "_sum_fitAndRes.pdf");
        if (option.find("-allformats") != string::npos)
        {
            c->Print((TString) printname + "_sum_fitAndRes.C");
            c->Print((TString) printname + "_sum_fitAndRes.eps");
            c->Print((TString) printname + "_sum_fitAndRes.png");
        }

        if (option.find("-linlog") != string::npos)
        {
            pl->SetMinimum(min * 0.1);
            plotPad->SetLogy();
            c->Print((TString) printname + "_sum_log_fitAndRes.pdf");
            if (option.find("-allformats") != string::npos)
            {
                c->Print((TString) printname + "_sum_log_fitAndRes.C");
                c->Print((TString) printname + "_sum_log_fitAndRes.eps");
                c->Print((TString) printname + "_sum_log_fitAndRes.png");
            }
        }
    }
    else {
        pl->Draw();
        c->Print((TString) printname + "_sum.pdf");
        if (option.find("-allformats") != string::npos)
        {
            c->Print((TString) printname + "_sum.C");
            c->Print((TString) printname + "_sum.eps");
            c->Print((TString) printname + "_sum.png");
        }

        if (option.find("-linlog") != string::npos)
        {
            pl->SetMinimum(min * 0.1);
            c->SetLogy();
            c->Print((TString) printname + "_sum_log.pdf");
            if (option.find("-allformats") != string::npos)
            {
                c->Print((TString) printname + "_sum_log.C");
                c->Print((TString) printname + "_sum_log.eps");
                c->Print((TString) printname + "_sum_log.png");
            }
        }
    }

    return pl;

}

void MultiAnalysis::AddGaussConstraint(RooRealVar * par, double mean, double sigma)
{
    if (mean == -1e9) mean = par->getVal();
    if (sigma == -1e9) sigma = par->getError();
    TString name = par->GetName();
    RooRealVar *cm = new RooRealVar("mean_"  + name, "mean_"  + name, mean);
    RooRealVar *cs = new RooRealVar("sigma_" + name, "sigma_" + name, sigma);
    RooGaussian *constr = new RooGaussian("constr_" + name, "constr_" + name, *par, *cm, *cs);
    if (m_pmode == "v") {
        cout << endl << m_name << ": AddGaussConstraint " << name << " " << Form("gauss(%f,%f)", mean, sigma) << endl;
        constr->Print();
        cout << endl;
    }
    AddConstraint(constr);
}

void MultiAnalysis::AddMultiVarGaussConstraint(RooArgList & listOfVariables, TMatrixDSym covMatrix)
{
    TString name = TString("");
    RooArgList listOfMean;
    TIterator *it = listOfVariables.createIterator();
    RooRealVar *arg;
    while ((arg = (RooRealVar*) it->Next())) {
        name += (TString) "_" + arg->GetName();
        RooRealVar *cm = new RooRealVar((TString) "mean_" + arg->GetName(), (TString) "mean_" + arg->GetName(), arg->getVal());
        listOfMean.add(*cm);
    }
    RooMultiVarGaussian * constr = new RooMultiVarGaussian("constr_" + name, "constr_" + name, listOfVariables, listOfMean, covMatrix);
    if (m_pmode == "v") {
        cout << endl << m_name << ": AddMultiVarGaussConstraint " << name << " " << listOfMean.getSize() << endl;
        constr->Print();
        cout << endl;
    }
    AddConstraint(constr);
}

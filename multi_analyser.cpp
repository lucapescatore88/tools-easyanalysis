/*
 * Author : Luca Pescatore
 * Email  : luca.pescatore@cern.ch
 * Date   : 17/12/2015
 */



#include "multi_analyser.hpp"

using namespace RooFit;

void MultiAnalysis::AddCategory(Analysis * _ana, TString nameCat)
{
    samples->defineType(nameCat);
    categories.push_back(nameCat);
    ana.push_back(_ana);
    constr->add(*(_ana->GetConstraints()));

    if(vars->getSize()==0) vars->add(*_ana->GetVariable());
    else if(!vars->find(_ana->GetVariable()->GetName()))
        vars->add(*_ana->GetVariable());
}


bool MultiAnalysis::Initialize(string opt)
{
    bool nomodel = false, nodata = false;
    if(opt.find("-noInitModel")!=string::npos) nomodel = true;
    if(opt.find("-noInitData")!=string::npos) nodata = true;

    if( !nomodel ) combModel = new RooSimultaneous("combModel","",*samples);
    map<string,RooDataSet*> mymap;

    for(unsigned i = 0; i < categories.size(); i++)
    {
        if(opt.find("-I")!=string::npos) ana[i]->Initialize(opt);
        if( !ana[i]->isValid() ) { cout << "ATTENTION!: Analysis " << ana[i]->GetName() << " is not initialized!" << endl; return false; }
        mymap[(string)categories[i]] = (RooDataSet*)(ana[i]->GetDataSet(opt));
        if( !nomodel ) combModel->addPdf(*(ana[i]->GetModel()),categories[i]);
    }

    if( !nodata ) combData = new RooDataSet("combData","combined datas",*vars,Index(*samples),Import(mymap));

    if((string)ana[0]->GetPrintLevel() == "v") combData->Print();
    if((string)ana[0]->GetPrintLevel() == "v") combModel->Print();
    if((string)ana[0]->GetPrintLevel() == "v")
    {
        cout << "***" << endl;
        printParams(combModel, *vars);
        cout << "***" << endl;
        printParams(combModel);
        cout << "***" << endl;
    }

    cout << "Object MultiAna " << name << " initialized correctly." << endl;

    init = true;
    return init;
}

map < string, RooPlot * > MultiAnalysis::SimultaneousFit(double min, double max, unsigned nbins, string opt)
{
    if(!init) Initialize(opt);
    if((string)ana[0]->GetPrintLevel() == "v") cout << "--------------- FITTING --------------- " << endl;

    RooCmdArg isExtended = Extended(kTRUE);
    if (opt.find("-noextended") != string::npos) isExtended = Extended(kFALSE);
    RooCmdArg isQuiet = PrintLevel(2);
    if(opt.find("-quiet")!=string::npos) isQuiet = PrintLevel(-1);
    RooCmdArg useMinos = Minos(kFALSE);
    if(opt.find("-minos")!=string::npos) useMinos = Minos(kTRUE);

    if(!combModel || !combData) { cout << "Model or data not set" << endl; return map < string, RooPlot * >(); }

    if(min==max) fitResult = combModel->fitTo(*combData,Save(),isExtended,isQuiet,useMinos,ExternalConstraints(*constr));
    else
    {
        for(unsigned i = 0; i < categories.size(); i++)
            ana[i]->GetVariable()->setRange("FitRange",min,max);

        fitResult = combModel->fitTo(*combData,Save(),isExtended,
                isQuiet,useMinos,ExternalConstraints(*constr));
    }

    if (opt.find("-quiet") == string::npos)
        cout << name << " :  CovQual = " << fitResult->covQual() << ",   Status = " << fitResult->status() << ",   EDM = " << fitResult->edm() << endl;        

    map < string, RooPlot * > plots;
    if( opt.find("-noPlot")==string::npos )
        for(unsigned i = 0; i < categories.size(); i++)
        {
            RooRealVar * var = ana[i]->GetVariable();
            if(opt.find("-noInitModel")==string::npos && opt.find("-noInitData")==string::npos)
            {
                double range[2] = {var->getMin(),var->getMax()};
                plots[(string)categories[i]] = ana[i]->Print(opt+"-nochi2", nbins, range);//+"-t"+(string)categories[i]);
            }
            else
            {
                TCanvas * c = new TCanvas();
                RooPlot * pl = new RooPlot(*var,var->getMin(),var->getMax(),nbins);
                combData->plotOn(pl,Cut("samples==samples::"+categories[i]));
                combModel->getPdf(categories[i])->plotOn(pl,Normalization(ana[i]->GetDataSet("")->numEntries(),RooAbsReal::NumEvent));
                pl->SetTitle("");
                pl->SetXTitle(((TString)var->GetName()).ReplaceAll("__var__",""));
                pl->Draw();
                c->Print(name+"_"+categories[i]+".pdf");

            }
        }
    for(unsigned i = 0; i < categories.size(); i++) ana[i]->SetFitRes(fitResult);

    double logL = combModel->createNLL(*combData)->getVal();
    cout << "\n" << name << ": LogL = " << logL << endl;

    if(opt.find("-corr") != string::npos)
    {
        TCanvas * c = new TCanvas();
        TH2D * hCor = (TH2D*) fitResult->correlationHist();
        hCor->SetLabelSize(0.02, "xyz");
        hCor->Draw("colz");
        c->Print(name+"_corr.pdf");
        c->Print(name+"_corr.C");
        delete hCor;
    }

    return plots;
}



RooWorkspace * MultiAnalysis::SaveToRooWorkspace()
{
    RooWorkspace * ws = new RooWorkspace("ws_"+name);

    for(size_t aa = 0; aa < ana.size(); aa++)
    {
        TIterator * it = ana[aa]->SaveToRooWorkspace()->componentIterator();
        TObject * arg;
        while( (arg=(TObject *)it->Next()) ) ws->import(*arg);
    }
    ws->import(*combModel);
    ws->import(*combData);

    return ws;
}

void MultiAnalysis::ImportModel(RooWorkspace * ws)
{
    cout << endl;
    cout << "ImportModel" << endl;
    cout << endl;
    TIterator * it = ws->componentIterator();
    TObject * arg;
    while( (arg=(TObject *)it->Next()) )
    {
        string name = arg->GetName();
        cout << name << endl;
        if(name.find("combModel")!=string::npos) {
            combModel = (RooSimultaneous*)arg;
            cout << "*** IMPORTED " << name << endl;
            printParams(combModel);
            cout << "***" << endl;
        }
    }
    cout << endl;
}



RooPlot * MultiAnalysis::PrintSum(string option, TString dovar, string printname, int nbins)
{
    size_t posb = option.find("-nbins");
    if (posb != string::npos)
        nbins = ((TString) option.substr(posb+6, string::npos)).Atof();

    if(!init && option.find("-noInit")==string::npos) Initialize();

    TLegend * leg = getTLegend(option);
       
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

    for(unsigned i = 0; i < categories.size(); i++)
    {
        if(dovar != "") { if(ana[i]->GetVariable()->GetName() != dovar) continue; }
        else if(ana[i]->GetVariable()->GetName() != ana[0]->GetVariable()->GetName()) continue;
        if( printname != "" && ((string)categories[i]).find(printname)==string::npos ) continue;
        k = i;

        print_cats.push_back((string)categories[i]);
        pdfs.add(*ana[i]->GetModel());
        RooRealVar * curfrac = new RooRealVar("frac"+categories[i],"",ana[i]->GetDataSet()->sumEntries());
        fracs.add(*curfrac);
        sumAll += curfrac->getVal();

        sigpdfs.add(*ana[i]->GetSig());
        sigfracs.add(*ana[i]->GetNSigPtr());
        sumNsig += ana[i]->GetNSigPtr()->getVal();

        vector <RooAbsPdf *> components = ana[i]->GetBkgComponents();
        for(unsigned bb = 0; bb < components.size(); bb++)
        {
            size_t pos1 = ((string)components[bb]->GetName()).find("_");
            size_t pos2 = ((string)components[bb]->GetName()).find("_", pos1+1);
            string bname = ((string)components[bb]->GetName()).substr(0,pos2);

            if(first)
            {
                bkgpdfs[bname] = RooArgSet("bkg"+(TString)bname);
                bkgfracs[bname] = RooArgSet("bkgfracs"+(TString)bname);
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

    RooPlot * pl = ana[k]->Print("-empty"+option,nbins);
    RooAddPdf * sumPdf = new RooAddPdf("sumPdf","",pdfs,fracs);

    TString legstyle = "l";
    if(option.find("-fillbkg")!=string::npos) legstyle = "f";
    TString siglabel = getLegendLabel(ana[k]->GetSig()->GetTitle(),option);

    TString cut = "";
    for (size_t cc = 0; cc < print_cats.size(); cc++)
    {
        cut += "samples == samples::" + print_cats[cc];
        if(cc != (print_cats.size() - 1) ) cut += " || ";
    }

    combData->plotOn(pl,MarkerSize(1),Cut(cut));
    RooAddPdf * sumsigPdf = new RooAddPdf("sumsigPdf","",sigpdfs,sigfracs);
    RooAddPdf * sumtotbkgPdf = new RooAddPdf("sumtotbkgPdf","",totpdfs,totfracs);

    if(option.find("-nototsigplot")==string::npos)
    {
        sumsigPdf->plotOn(pl,
                LineStyle(kDashed),
                LineColor(kBlack),
                Normalization(sumNsig,RooAbsReal::NumEvent),
                Name("sumSigPdf"));
        leg->AddEntry(pl->findObject("sumSigPdf"),siglabel,"l");  
    }

    vector <Color_t> colors = GetDefaultColors();
    if(ana[k]->GetColors().size() > 0) colors = ana[k]->GetColors();
    int styles[] = {3,4,5,6,7,8,9,10,3,4,5,6,7,8,9,10};
    int counter = 0;
    //if(option.find("-stackbkg")==string::npos) counter++;
    if(option.find("-plottotbkg")!=string::npos)
    {
        sumtotbkgPdf->plotOn(pl,LineStyle(styles[counter]),LineColor(colors[counter]),Normalization( sumNtotbkg , RooAbsReal::NumEvent));
        counter++;
    }
    if(option.find("-stack")!=string::npos)
    {
        RooArgSet stackedFracs("stackedFracs"), stackedPdfs("stackedPdfs");
        double curSum = 0;
        for(unsigned bb = 0; bb < bnames.size(); bb++)
        {
            string bname = bnames[bb];
            stackedPdfs.add(bkgpdfs[bname]);
            stackedFracs.add(bkgfracs[bname]);
            curSum += sumNbkg[bname];
            RooAddPdf * curSumBkgPdf = new RooAddPdf("sumBkgPdf_"+(TString)bname,"",stackedPdfs,stackedFracs);
            TString leglabel = getLegendLabel(ana[k]->GetBkgComponents()[bb]->GetTitle(),option);

            if(option.find("-fillbkg")!=string::npos)
            {
                curSumBkgPdf->plotOn(pl,
                        FillColor(colors[counter]),
                        FillStyle(1001),
                        FillStyle(1001),
                        LineWidth(0.),
                        LineStyle(0),
                        LineColor(colors[counter]),
                        DrawOption("F"),
                        Normalization(curSum,RooAbsReal::NumEvent),
                        MoveToBack(), 
                        Name("sumBkgPdf_"+(TString)bname));  
                leg->AddEntry(pl->findObject("sumBkgPdf_"+(TString)bname),leglabel,"f");
            }
            else
            {
                curSumBkgPdf->plotOn(pl,
                        LineStyle(styles[counter]),
                        LineColor(colors[counter]),
                        Normalization(curSum,RooAbsReal::NumEvent),
                        Name("sumBkgPdf_"+(TString)bname));
                leg->AddEntry(pl->findObject("sumBkgPdf_"+(TString)bname),leglabel,"l");
            }
            counter++;
        }
    }
    else {

        for(unsigned bb = 0; bb < bnames.size(); bb++)
        {
            string bname = bnames[bb];
            RooAddPdf * curSumBkgPdf = new RooAddPdf("sumBkgPdf_"+(TString)bname,"",bkgpdfs[bname],bkgfracs[bname]);
            curSumBkgPdf->plotOn(pl,
                    LineStyle(styles[counter]),
                    LineColor(colors[counter]),
                    Normalization(sumNbkg[bname],RooAbsReal::NumEvent),
                    Name("sumBkgPdf_"+(TString)bname));
            TString leglabel = getLegendLabel(ana[k]->GetBkgComponents()[bb]->GetTitle(),option);
            leg->AddEntry(pl->findObject("sumBkgPdf_"+(TString)bname),leglabel,"l");
            counter++;
        }
    }

    sumPdf->plotOn(pl,Normalization(sumAll, RooAbsReal::NumEvent));

    return printFrame(pl,option,name,ana[k]->GetVariable(),"","",leg,fitResult);

    /*
    if(option.find("-LHCb")!=string::npos)
    {
        TPaveText * tbox = new TPaveText(gStyle->GetPadLeftMargin() + 0.05,
                0.85 - gStyle->GetPadTopMargin(),
                gStyle->GetPadLeftMargin() + 0.25,
                0.97 - gStyle->GetPadTopMargin(),
                "BRNDC");
        if(option.find("-LHCbDX")!=string::npos)
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

    pl->SetTitle("");
    pl->addObject(leg);
    pl->Draw();
    c->Print("fit_"+(TString)printname+".pdf");
    c->SetLogy();
    c->Print("fit_"+(TString)printname+"_log.pdf");
*/
    return pl;
}






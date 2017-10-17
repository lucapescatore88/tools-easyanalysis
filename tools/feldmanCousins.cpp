
#include "feldmanCousins.hpp"


RooFitResult * defFit(RooAbsPdf * pdf, RooDataSet * data, Str2VarMap p, ISVALIDF_PTR isValid, string opt, int nfree, RooArgSet * cons, RooAbsReal * mynll)
{
    return pdf->fitTo(*data, PrintLevel(-1), Save());
}

void myFeldmanCousins::DrawLogL()
{
    TCanvas * c = new TCanvas();
    RooAbsReal * nll = combPdf->createNLL(*combData, RooFit::CloneData(false));
    double nll_val = nll->getVal();
    RooPlot * nllPlot = PoI->frame();
    nll->plotOn(nllPlot, Name("LogL"));
    nllPlot->SetMinimum(nll_val * 0.95);
    nllPlot->Draw();
    c->Print("LogL_vs_" + (TString)PoI->GetName() + "_" + name + ".pdf");

    if (!PoI2) return;
    RooPlot * nllPlot2 = PoI2->frame();
    nll->plotOn(nllPlot2, Name("LogL"));
    nllPlot2->SetMinimum(nll_val * 0.95);
    nllPlot2->Draw();
    c->Print("LogL_vs_" + (TString)PoI2->GetName() + "_" + name + ".pdf");
}

void myFeldmanCousins::Initialize()
{
    if (categories.size() == 0) { combData = datas[0]; combPdf = pdfs[0]; }
    else
    {
        samples = new RooCategory("samples", "samples");
        combPdf = new RooSimultaneous("combModel", "", *samples);
        map<string, RooDataSet*> mymap;

        for (unsigned i = 0; i < categories.size(); i++)
        {
            samples->defineType(categories[i]);
            mymap[(string)categories[i]] = datas[i];
            ((RooSimultaneous *)combPdf)->addPdf(*(pdfs[i]), categories[i]);
        }

        if (weight == "") combData = new RooDataSet("combData", "combined datas", *obs, Index(*samples), Import(mymap));
        else
        {
            RooRealVar * wvar = new RooRealVar(weight, weight, 1.);
            combData = new RooDataSet("combData", "combined datas", RooArgSet(*obs, *wvar), Index(*samples), Import(mymap), WeightVar(weight));
        }
    }

    GetNFreePars();
    RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);
}


void myFeldmanCousins::InitializeAndFit()
{
    Initialize();

    origPars = copyFreePars(combPdf, *obs);
    PoI->setConstant(kFALSE);
    if (PoI2) PoI2->setConstant(kFALSE);

    nll = combPdf->createNLL(*combData, RooFit::CloneData(false));
    myfit(combPdf, combData, PoIs, NULL, "", 100, NULL, nll);
    printParams(combPdf, *obs); cout << endl;
}




double myFeldmanCousins::generateToys(double testVal, double testVal2, TH1 * h, string opt )
{
    double npass = 0, nfailed = 0;
    for (unsigned e = 0; e < nexp; e++)
    {
        showPercentage(e, nexp);

        RooDataSet * toyData = NULL;
        if (categories.size() == 0) { toyData = pdfs[0]->generate(*obs, datas[0]->numEntries()); }
        else
        {
            map<string, RooDataSet*> mymap;
            for (unsigned c = 0; c < categories.size(); c++)
                mymap[(string)categories[c]] = pdfs[c]->generate(*obs, datas[c]->numEntries());
            toyData = new RooDataSet("toyData", "", *obs, Index(*samples), Import(mymap));
        }

        fixParam(combPdf, obs, origPars);
        PoI->setConstant(kFALSE);
        if (PoI2) PoI2->setConstant(kFALSE);
        RooAbsPdf * copyPdf = (RooAbsPdf *)combPdf->Clone("toyPdf");
        RooAbsReal * toynll = copyPdf->createNLL(*toyData, RooFit::CloneData(false));
        RooFitResult * res = myfit(copyPdf, toyData, PoIs, isValid, opt, nfree, cons, toynll);
        double nll_val = toynll->getVal();

        fixParam(copyPdf, obs, origPars);
        PoI->setVal(testVal);
        PoI->setConstant(kTRUE);
        if (PoI2) PoI2->setVal(testVal2);
        if (PoI2) PoI2->setConstant(kTRUE);

        if (res && (res->covQual() != 3 || res->edm() > 0.1)) { nfailed++; npass++; }
        else
        {
            //if((nfree>1 && !PoI2) || nfree>2)
            myfit(copyPdf, toyData, PoIs, isValid, opt, nfree, cons, toynll);
            double nll_fix_val = toynll->getVal();

            double MCratio = nll_fix_val / nll_val;
            if (h) h->Fill(MCratio);
            if (dataratio < MCratio) npass++;
        }

        delete copyPdf;
        delete toyData;
        delete toynll;
    }

    if (nfailed > 0) cout << "************************************* Fits failed = " << nfailed / nexp << endl;
    return (double)npass / (double)nexp;
}


vector< vector < double > > myFeldmanCousins::ExtractLimits(vector< vector < double > > points, RooArgSet * _origPars, TGraph ** point, string opt, ISVALIDF_PTR g_isValid, double nll_val)
{
    isValid = g_isValid;
    if (_origPars) origPars = _origPars;
    if (!nll)
    {
        if (!origPars) origPars = copyFreePars(combPdf, *obs);
        else fixParam(combPdf, obs, origPars);
        PoI->setConstant(kFALSE);
        if (PoI2) PoI2->setConstant(kFALSE);
        nll = combPdf->createNLL(*combData, RooFit::CloneData(false));
        myfit(combPdf, combData, PoIs, isValid, opt, nfree, cons, nll);
        printParams(combPdf, *obs); cout << endl;
    }

    if (point && PoI2) (*point)->SetPoint(0, PoI->getVal(), PoI2->getVal());
    if (point && !PoI2) (*point)->SetPoint(0, PoI->getVal(), 0);
    nll_val = nll->getVal();

    vector< vector < double > > gr;
    if (points.empty())
    {
        cout << "Attention: No points set! Generating regular grid of " << npoints << " per dimension." << endl;
        if (PoI2)
        {
            double step1 = (PoI->getMax() - PoI->getMin()) / npoints;
            double step2 = (PoI2->getMax() - PoI2->getMin()) / npoints;
            for (int a = PoI->getMin(); a < PoI->getMax(); a += step1 )
            {
                for (int a2 = PoI2->getMin(); a2 < PoI2->getMax(); a2 += step2 )
                {
                    vector<double> curpoint(1, a);
                    curpoint.push_back(a2);
                    points.push_back(curpoint);
                }
            }
        }
        else
        {
            double step = (PoI->getMax() - PoI->getMin()) / npoints;
            for (int a = PoI->getMin(); a < PoI->getMax(); a += step ) points.push_back(vector<double>(1, a));
        }
    }
    if (points.size() < PoIs.size()) { cout << " NO POINTS SET FOR ALL POIs!!! " << endl; return vector< vector < double > >(); }
    npoints = points[0].size();
    for (unsigned i = 0; i < npoints; i++)
    {
        double curTestVal = points[0][i];
        double curTestVal2 = 0;
        if (PoI2) curTestVal2 = points[1][i];

        fixParam(combPdf, obs, origPars);
        PoI->setVal(curTestVal);
        PoI->setConstant(kTRUE);
        if (PoI2) PoI2->setVal(curTestVal2);
        if (PoI2) PoI2->setConstant(kTRUE);

        if (isValid && !isValid(combPdf))
        {
            cout << fixed << setprecision(2);
            if (PoI2) cout << "(" << PoI->GetName() << "," << PoI2->GetName() << ") = (" << curTestVal << "," << curTestVal2 << ")";
            else cout << PoI->GetName() << " = " << curTestVal;
            cout << " -> Invalid point: SKIPPING!!!" << endl;
            continue;
        }

        myfit(combPdf, combData, PoIs, isValid, opt, nfree, cons, nll);
        double nll_fix_val = nll->getVal();
        dataratio = nll_fix_val / nll_val;

        double pvalue = generateToys(curTestVal, curTestVal2, NULL, opt);
        vector < double > res(1, curTestVal);
        res.push_back(curTestVal2);
        res.push_back(pvalue);
        gr.push_back(res);

        if (PoI2)
        {
            cout << fixed << setprecision(3) << "(" << PoI->GetName() << "," << PoI2->GetName();
            cout << ") = (" << curTestVal << "," << curTestVal2 << ")";
        }
        else
        {
            cout << fixed << setprecision(3) << PoI->GetName();
            cout << " = " << curTestVal;
        }
        cout << " -> logL_fix / logL_free = " << fixed << setprecision(6) << nll_fix_val / nll_val;
        cout << " -> pvalue = " << pvalue << endl;
    }

    return gr;
}


/*********** Extra functions looping on all points and making post processing ********/
/*** Use only if NExp <= 500 and Npoint <= 10 or it takes too long ~10h *********/

/*
vector<double> extractLimits(TGraph * gr, double CL)
{
    double max = -1e6, pvmax = -1e6;
    double xmin = 0, xmax = 0, pvxmax = 0, pvxmin = 0;
    double pv = 0, x = 0;
    for(int i = 0; i < gr->GetN(); i++)
    {
        gr->GetPoint(i,x,pv);
        if(i==0) { xmin = x; pvxmin = pv; }
        if(i==(gr->GetN()-1)) { xmax = x; pvxmax = pv; }
        if(pv > pvmax) { pvmax = pv; max = x; }
    }

    int lp = -1, rp = -1;
    for(int i = 0; i < gr->GetN(); i++)
    {
        gr->GetPoint(i,x,pv);
        if(x < max && pv >= (1 - CL) && lp==-1.) lp = i;
        if(x > max && pv < (1 - CL) && rp==-1.) rp = i;
    }

    vector<double> res;
    double x1=0, p1=0, x2=0, p2=0;
    if(max != xmin && pvxmin < (1.-CL))
    {
        gr->GetPoint(lp-1,x1,p1);
        gr->GetPoint(lp,x2,p2);
        res.push_back(x1 + (x2-x1)*((1.-CL)-p1)/(p2-p1));
    }
    else res.push_back(xmin);
    if(max != xmax && pvxmax < (1.-CL) )
    {
        gr->GetPoint(rp-1,x1,p1);
        gr->GetPoint(rp,x2,p2);
        res.push_back(x1 + (x2-x1)*((1.-CL)-p1)/(p2-p1));
    }
    else res.push_back(xmax);

    return res;
}



TH2F * myFeldmanCousins::ExtractLimits(double min, double max, double min2, double max2, RooArgSet *origPars, FUNC_PTR isValid, TGraph * point, string opt )
{
    TCanvas * c = new TCanvas();

    if(!PoI2) { cout << "Second parameter missing!!!" << endl; return NULL; }
    if(!origPars) origPars = copyFreePars(combPdf,*obs);
    else fixParam(combPdf,obs,origPars);
    PoI->setConstant(kFALSE);
    PoI2->setConstant(kFALSE);

    RooAbsReal* nll = combPdf->createNLL(*combData,RooFit::CloneData(false));
    myfit(combPdf,combData,PoIs,isValid,opt,nfree,cons,nll);
    TGraph * grpoint = new TGraph();
    if(point) point = grpoint;
    grpoint->SetPoint(1,PoI->getVal(),PoI2->getVal());
    printParams(combPdf,*obs); cout << endl;

    double nll_val = nll->getVal();
    double central = PoI->getVal();
    double central2 = PoI2->getVal();

    DrawLogL();

    if(min == 0 && max == 0)
    {
        min = central - 2*PoI->getError();
        max = central + 2*PoI->getError();
    }
    if(min2 == 0 && max2 == 0)
    {
        min2 = central2 - 2*PoI2->getError();
        max2 = central2 + 2*PoI2->getError();
    }

    double step = (max-min)/(npoints-1);
    double step2 = (max2-min2)/(npoints-1);
    TH2F * gr = new TH2F("pvalues_"+name,"pvalues "+name,npoints,min-step/2.,max+step/2.,npoints,min2-step2/2.,max2+step2/2.);

    for(unsigned i = 0; i < npoints; i++)
    {
        for(unsigned j = 0; j < npoints; j++)
        {
            double curTestVal = min + i*step;
            double curTestVal2 = min2 + j*step2;

            vector < vector <double> > points(2,vector <double>());
            points[0].push_back(curTestVal);
            points[1].push_back(curTestVal2);
            PoI->setVal(curTestVal);
            PoI2->setVal(curTestVal2);

            vector < vector < double > > res = ExtractLimits(points, origPars, NULL, opt, isValid, nll_val);
            gr->Fill(curTestVal,curTestVal2,res[0][2]);
        }
    }

    PoI->setConstant(kFALSE);
    PoI2->setConstant(kFALSE);
    if(origPars) fixParam(combPdf,obs,origPars);

    double contours[2] = { 0.1, 0.32 };
    gr->SetContour(2, contours);
    gr->SetMinimum(0);
    gr->SetMaximum(1);
    gr->SetLineWidth(2);
    gStyle->SetOptStat(0);

    gr->Draw("CONT1 LIST");

    c->Update();

    TObjArray *conts = (TObjArray*)gROOT->GetListOfSpecials()->FindObject("contours");
    TList *lcontour1 = (TList*)conts->At(0);
    TGraph *gc1 = (TGraph*)lcontour1->First();

    for(int i = 0; i < gc1->GetN(); i++)
    {
        double a, f;
        gc1->GetPoint(i,a,f);
        if( (f-1)*3./4. > a )
        {
            double m = 4./3.;
            double q = 1.;
            gc1->SetPoint(i,(m*a+f-q)/(2*m),(m*a+f+q)/2.);
        }
        else if( a > -(f-1)*3./4. )
        {
            double m = -4./3.;
            double q = 1.;
            gc1->SetPoint(i,(m*a+f-q)/(2*m),(m*a+f+q)/2.);
        }
    }

    TH2F * htmp = new TH2F("tmp","",1e3,PoI->getMin()*1.05,PoI->getMax()*1.05,1e3,PoI2->getMin()*1.05,PoI2->getMax()*1.05);
    gr->GetXaxis()->SetTitle(PoI->GetName());
    gr->GetYaxis()->SetTitle(PoI2->GetName());
    htmp->Draw();

    Double_t xt[3] = {-0.75,0.,0.75};
    Double_t yt[3] = {0.,1.,0.};
    TPolyLine *pline = new TPolyLine(3,xt,yt);
    pline->SetLineColor(1);
    pline->SetLineWidth(1);
    pline->Draw("f");

    gc1->Draw("L SAME");

    grpoint->SetMarkerColor(4);
    grpoint->SetMarkerSize(1.1);
    grpoint->SetMarkerStyle(29);
    grpoint->Draw("P same");
    c->Print("contours_"+name+".pdf");

    return gr;
}



vector <double> myFeldmanCousins::ExtractLimits(double min, double max, RooArgSet *origPars, FUNC_PTR isValid, string opt)
{
    TCanvas * c = new TCanvas();
    if(!origPars) origPars = copyFreePars(combPdf,*obs);
    else fixParam(combPdf,obs,origPars);
    PoI->setConstant(kFALSE);

    RooAbsReal * nll = combPdf->createNLL(*combData,RooFit::CloneData(false));
    myfit(combPdf,combData,PoIs,isValid,opt,nfree,cons,nll);
    printParams(combPdf,*obs); cout << endl;
    double nll_val = nll->getVal();

    DrawLogL();

    double central = PoI->getVal();
    if(min == 0 && max == 0)
    {
        min = central - 2*PoI->getError();
        max = central + 2*PoI->getError();
    }

    TGraph * gr = new TGraph();
    double step = (max-min)/(npoints-1);
    for(unsigned i = 0; i < npoints; i++)
    {
        double curTestVal = min + i*step;

        vector < vector <double> > points(2,vector <double>());
        points[0].push_back(curTestVal);
        points[1].push_back(0);
        PoI->setVal(curTestVal);

        vector < vector < double > >  res = ExtractLimits(points, origPars, NULL, opt, isValid, nll_val);
        gr->SetPoint(i,curTestVal,res[0][2]);
    }

    PoI->setConstant(kFALSE);
    if(origPars) fixParam(combPdf,obs,origPars);

    gr->Draw("AP");
    gr->GetXaxis()->SetTitle(PoI->GetName());
    gr->GetYaxis()->SetTitle("pvalue");
    gr->Draw();
    TLine * line = new TLine(min,0.32,max*0.99,0.32);
    line->SetLineColor(kRed);
    line->Draw();

    vector<double> limits = extractLimits(gr,0.68);
    TLine * line2 = new TLine(limits[0],0,limits[0],1);
    line2->SetLineColor(kRed);
    line2->Draw();
    TLine * line3 = new TLine(limits[1],0.,limits[1],1);
    line3->SetLineColor(kRed);
    line3->Draw();
    c->Print("pvalue_"+(TString)PoI->GetName()+"_"+name+".pdf");

    return limits;
}


TH2F * myFeldmanCousins::ExtractLimits(Str2VarMap params, RooArgSet *origPars, double min, double max, double min2, double max2, FUNC_PTR isValid, TGraph * point, string opt )
{
    TCanvas * c = new TCanvas();

    if(!PoI2) { cout << "Second parameter missing!!!" << endl; return NULL; }
    if(origPars) fixParam(combPdf,obs,origPars);
    PoI->setConstant(kFALSE);
    PoI2->setConstant(kFALSE);

    RooAbsReal* nll = combPdf->createNLL(*combData,RooFit::CloneData(false));
    myfit(combPdf,combData,params,isValid,opt,nfree,cons,nll);
    TGraph * grpoint = new TGraph();
    if(point) point = grpoint;
    grpoint->SetPoint(1,PoI->getVal(),PoI2->getVal());
    printParams(combPdf,*obs); cout << endl;

    double nll_val = nll->getVal();
    double central = PoI->getVal();
    double central2 = PoI2->getVal();

    DrawLogL();

    if(min == 0 && max == 0)
    {
        min = central - 2*PoI->getError();
        max = central + 2*PoI->getError();
    }
    if(min2 == 0 && max2 == 0)
    {
        min2 = central2 - 2*PoI2->getError();
        max2 = central2 + 2*PoI2->getError();
    }

    double step = (max-min)/(npoints-1);
    double step2 = (max2-min2)/(npoints-1);
    TH2F * gr = new TH2F("pvalues_"+name,"pvalues "+name,npoints,min-step/2.,max+step/2.,npoints,min2-step2/2.,max2+step2/2.);

    for(unsigned i = 0; i < npoints; i++)
    {
        for(unsigned j = 0; j < npoints; j++)
        {
            double curTestVal = min + i*step;
            double curTestVal2 = min2 + j*step2;

            if(origPars) fixParam(combPdf,obs,origPars);
            PoI->setVal(curTestVal);
            PoI->setConstant(kTRUE);
            PoI2->setVal(curTestVal2);
            PoI2->setConstant(kTRUE);

            if(!isValid(params))
            {
                cout << fixed << setprecision(2) << "(" << PoI->GetName() << "," << PoI2->GetName() << ") = (" << curTestVal << "," << curTestVal2;
                cout << ") -> Invalid point: SKIPPING!!!" << endl;
                continue;
            }

            if(nfree>2) myfit(combPdf,combData,params,isValid,opt,nfree,cons,nll);
            double nll_fix_val = nll->getVal();

            double pvalue = generateToys(curTestVal,curTestVal2,params,nll_fix_val/nll_val,NULL,origPars,isValid,opt);
            gr->Fill(curTestVal,curTestVal2,pvalue);

            cout << fixed << setprecision(3) << "(" << PoI->GetName() << "," << PoI2->GetName() << ") = (" << curTestVal << "," << curTestVal2;
            cout << ") -> logL_fix / logL_free = " << fixed << setprecision(6) << nll_fix_val/nll_val;
            cout << " -> pvalue = " << pvalue << endl;
        }
    }

    PoI->setConstant(kFALSE);
    PoI2->setConstant(kFALSE);
    if(origPars) fixParam(combPdf,obs,origPars);

    TH2F * htmp = new TH2F("tmp","",1e3,PoI->getMin()*1.05,PoI->getMax()*1.05,1e3,PoI2->getMin()*1.05,PoI2->getMax()*1.05);
    gr->GetXaxis()->SetTitle(PoI->GetName());
    gr->GetYaxis()->SetTitle(PoI2->GetName());
    htmp->Draw();

    double contours[2] = { 0.1, 0.32 };
    gr->SetContour(2, contours);
    gr->SetMinimum(0);
    gr->SetMaximum(1);
    gr->SetLineWidth(2);
    gStyle->SetOptStat(0);

    Double_t xt[3] = {-0.75,0.,0.75};
    Double_t yt[3] = {0.,1.,0.};
    TPolyLine *pline = new TPolyLine(3,xt,yt);
    pline->SetLineColor(1);
    pline->SetLineWidth(1);
    pline->Draw("f");

    gr->Draw("CONT1 LIST SAME");

    grpoint->SetMarkerColor(4);
    grpoint->SetMarkerSize(1.1);
    grpoint->SetMarkerStyle(29);
    grpoint->Draw("P same");
    c->Print("contours_"+name+".pdf");

    return gr;
}



vector <double> myFeldmanCousins::ExtractLimits(Str2VarMap params, RooArgSet *origPars, double min, double max, FUNC_PTR isValid, string opt)
{
    TCanvas * c = new TCanvas();
    if(origPars) fixParam(combPdf,obs,origPars);
    PoI->setConstant(kFALSE);

    RooAbsReal * nll = combPdf->createNLL(*combData,RooFit::CloneData(false));
    myfit(combPdf,combData,params,isValid,opt,nfree,cons,nll);
    printParams(combPdf,*obs); cout << endl;
    double nll_val = nll->getVal();

    DrawLogL();

    double central = PoI->getVal();
    if(min == 0 && max == 0)
    {
        min = central - 2*PoI->getError();
        max = central + 2*PoI->getError();
    }

    TGraph * gr = new TGraph();
    double step = (max-min)/(npoints-1);
    for(unsigned i = 0; i < npoints; i++)
    {
        double curTestVal = min + i*step;
        if(origPars) fixParam(combPdf,obs,origPars);
        PoI->setVal(curTestVal);
        PoI->setConstant(kTRUE);

        if(nfree>1) myfit(combPdf,combData,params,isValid,opt,nfree,cons,nll);
        double nll_fix_val = nll->getVal();
        TH1 * toyDistrib = new TH1F("ratios_"+name,"",100,1.,1.05);
        double pvalue = generateToys(curTestVal,0,params,nll_fix_val/nll_val,toyDistrib,origPars,isValid,opt);
        gr->SetPoint(i,curTestVal,pvalue);
        if(opt.find("-printh")!=string::npos) { toyDistrib->Draw(); c->Print("toysDistrib_"+name+".pdf"); }
        delete toyDistrib;
        cout << fixed << setprecision(3) << PoI->GetName() << " = " << curTestVal;
        cout << " -> logL_fix / logL_free = " << fixed << setprecision(6) << nll_fix_val/nll_val;
        cout << " -> pvalue = " << pvalue << endl;
    }

    PoI->setConstant(kFALSE);
    if(origPars) fixParam(combPdf,obs,origPars);

    gr->Draw("AP");
    gr->GetXaxis()->SetTitle(PoI->GetName());
    gr->GetYaxis()->SetTitle("pvalue");
    gr->Draw();
    TLine * line = new TLine(min,0.32,max*0.99,0.32);
    line->SetLineColor(kRed);
    line->Draw();

    vector<double> limits = extractLimits(gr,0.68);
    TLine * line2 = new TLine(limits[0],0,limits[0],1);
    line2->SetLineColor(kRed);
    line2->Draw();
    TLine * line3 = new TLine(limits[1],0.,limits[1],1);
    line3->SetLineColor(kRed);
    line3->Draw();
    c->Print("pvalue_"+(TString)PoI->GetName()+"_"+name+".pdf");

    return limits;
}


*/

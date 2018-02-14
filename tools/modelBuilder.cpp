#include "modelBuilder.hpp"


/* ********************************************     ModelBuilder methods  *********************************************/

string ModelBuilder::m_pmode = "v";

void ModelBuilder::SetModel(RooAbsPdf * _model)
{
    m_model = _model;
    m_sig = m_model;
    Str2VarMap pars = GetParams();
    if (pars["nsig"]) m_nsig = pars["nsig"];
}

RooArgSet * ModelBuilder::GetParamsArgSet()
{
    return m_model->getParameters(RooDataSet("v", "", RooArgSet(*m_var)));
}

RooAbsPdf * ModelBuilder::GetParamsGaussian(RooFitResult * fitRes)
{
    return (RooAbsPdf *)(new RooMultiVarGaussian(
                             m_name + "_multivar_gauss", m_name + "_multivar_gauss",
                             *GetParamsArgSet(), fitRes->covarianceMatrix()));
}


RooDataSet * ModelBuilder::GetParamsVariations(int nvariations, RooFitResult * fitRes)
{
    RooArgSet * params = GetParamsArgSet();

    cout << "Making variations of " << endl;
    params->Print();
    cout << "Matrix - fit quality" << fitRes->covQual() << endl;
    fitRes->covarianceMatrix().Print();
    cout << "End matrix" << endl;

    RooMultiVarGaussian * gauss = new RooMultiVarGaussian(
        m_name + "_multivar_gauss", m_name + "_multivar_gauss",
        *params, fitRes->covarianceMatrix());
    gauss->Print();
    RooDataSet * variations = gauss->generate(*params, nvariations);
    return variations;
}


void ModelBuilder::AddGaussConstraint(TString name, double mean, double sigma)
{
    if(sigma <= 0) { cout << "Re sigma of the constraint must be >0, if you want it =0 the please set the parameter constant instead." << endl; }
    if (!m_sig) { cout << "You have to set the signal model before setting constraints to it." << endl; return; }
    RooRealVar * par = getParam(m_sig, (string)name, "-cut");
    if(!par) { cout << "Parameter " << name << " not found in signal pdf" << endl; return; }
    AddGaussConstraint(par, mean, sigma);
}

void ModelBuilder::AddGaussConstraint(TString pdf, TString name, double mean, double sigma)
{
    if(sigma <= 0) { cout << "Re sigma of the constraint must be >0, if you want it =0 the please set the parameter constant instead." << endl; }
    
    if(pdf == "sig") return AddGaussConstraint(name, mean, sigma);
    int id = GetBkgID((string)pdf);
    if (id < 0) { cout << "You have to '" << pdf << "' background component before setting constraints to it." << endl; return; }
    RooRealVar * par = getParam(m_bkg_components[id], (string)name, "-cut");
    if(!par) { cout << "Parameter " << name << " not found in " << pdf << " pdf" << endl; return; }
    AddGaussConstraint(par, mean, sigma);
}

void ModelBuilder::AddGaussConstraint(RooRealVar * par, double mean, double sigma)
{
    if (mean == -1e9) mean = par->getVal();
    if (sigma == -1e9) sigma = par->getError();
    TString name = par->GetName();

    RooRealVar  *cm = new RooRealVar("cm_" + name, "mean_" + name, mean);
    RooRealVar  *cs = new RooRealVar("cs_" + name, "error_" + name, sigma);
    RooGaussian *constr = new RooGaussian("constr_" + name, "constr_" + name, *par, *cm, *cs);
    if (m_pmode == "v") cout << Form("Constraint: " + name + "%s -> gauss(%f,%f)", mean, sigma) << endl;

    AddConstraint(constr);
}

/*
   Builds the model. And must be run before any fit.
   @param opt: options string. Options available are:
   "-exp" -> adds an exponential to the background you already set
   "-nobkg" -> ignores bkg
   */


RooAbsPdf * ModelBuilder::Initialize(string optstr)
{
    TString myname = "";
    myname = "_" + m_name;

    RooArgList * bkgList = new RooArgList("bkgList_" + m_name);
    RooArgList * fracList = new RooArgList("fracList_" + m_name);

    bool noBkg = (optstr.find("-nobkg") != string::npos);
    bool doExp = (optstr.find("-exp") != string::npos);
    if ( doExp ) AddBkgComponent("exp", "Exp", 1.e4, Str2VarMap(), "-ibegin");
    if ( m_totBkgMode ) m_bkg_fractions.clear();
    
    for (unsigned i = 0; i < m_bkg_components.size(); i++)
    {
        bkgList->add(*(m_bkg_components[i]));
        if (!m_totBkgMode) fracList->add(*(m_bkg_fractions[i]));
        else { if (i > 0) fracList->add(*(new RooRealVar(Form("frac_%i", i), "Frac", 0.5, 0., 1))); }
    }

    if (m_bkg_components.size() > 1)
    {
        m_bkg = new RooAddPdf("totbkg" + myname, "totbkg" + myname, *bkgList, *fracList);
    }
    else if (m_bkg_components.size() == 1)
    {
        m_bkg = m_bkg_components[0];
        m_bkg->SetName("totbkg" + myname);
    }

    GetTotNBkg();

    if (!m_sig) { cout << "WARNING: Signal not set!!" << endl; return NULL; }
    RooArgList pdfs(*m_sig), nevts(*m_nsig);

    if (!noBkg && !m_bkg_components.empty())
    {
        if (m_totBkgMode) m_model = new RooAddPdf("model" + myname, "model" + myname, RooArgSet(*m_sig, *m_bkg), RooArgSet(*m_nsig, *m_nbkg));
        else
        {
            pdfs.add(*bkgList);
            nevts.add(*fracList);
            m_model = new RooAddPdf("model" + myname, "model" + myname, pdfs, nevts);
        }
    }
    else m_model = m_sig;

    if (m_pmode == "v" && m_model)
    {
        cout << "\n" << m_name << ": Initialized Correctly! The model is:" << endl;
        m_model->Print();
    }
    m_isvalid = true;
    return m_model;
}




/*
   Allows to make nice plots of data and models including blinded plots
   @param title: titleyAxis->SetLabelSize  ( yAxis->GetLabelSize() / 0.33 );
   @param Xtitle: X axis label
   @param data: data to plot -> If no data use NULL
   @param opt: options string. Options available are:
   "-bw"                   -> prints in black&white
   "-fillSig"              -> signal is filled with color instead of dashed line
   "-fillBkg"              -> bkg is filled with color instead of dashed line
   "-log"                  -> logarithmic plot
   "-pulls" or "-ANDpulls" -> if data is inserted these add a pull histogram -pulls in other plot -ANDpulls under fit plot
   "-none"                 -> doesn't print and only returns the frame
   "-noleg"                -> doesn't draw legend
   "-leg[x1,y1,x2,y2]"     -> overrides the default layout of the legend
   @param bins: number of bins to use for data
   @param regions: number of regions to plot is blinded plot, if regions = 0 plots everything

   drawGaxis() is a service function to draw a scaled axis.
   */

/*
   void drawGaxis(TString Xtitle, double size)
   {
   gPad->Update();
   TGaxis * gaxis = new TGaxis(gPad->GetUxmin(),gPad->GetUymin(),gPad->GetUxmax(),gPad->GetUymin(),"ScaleFunc");
   gaxis->SetTitle(Xtitle);
   gaxis->SetLabelSize(size);
   gaxis->SetTitleSize(size*1.5);
   gaxis->Draw();
   }
   */

void ModelBuilder::Print(TString title, TString Xtitle, string opt, RooAbsData * data, int bins,
                         RooFitResult * fitRes, TString Ytitle, vector< RooRealVar *> myvars)
{
    for (auto v : myvars)
    {
        Print(title, Xtitle, opt, data, bins, vector<string>(), map<string, vector<double>>(), fitRes, Ytitle, v);
    }
}


RooPlot * ModelBuilder::Print(TString title, TString Xtitle, string opt, RooAbsData * data, int bins,
                              vector<string> regStr, map<string, vector<double>> reg, RooFitResult * fitRes, TString Ytitle, RooRealVar * myvar)
{
    transform(opt.begin(), opt.end(), opt.begin(), ::tolower);
    RooPlot* frame = NULL;
    TLegend * leg = new TLegend(0.65, 0.7, 0.76, 0.9);
    if (opt.find("-leg") != string::npos)
    {
        size_t pos = opt.find("-leg") + 5;
        string ss = opt.substr(pos, string::npos);
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

    if (!myvar) myvar = m_var;

    if (!isValid()) {
        cout << "**** WARNING: Model is not valid, probably not initialised. *****" << endl;
        return NULL;
    }

    // Create main frame

    frame = getFrame(myvar, data, m_model, opt, bins, regStr, reg, Xtitle, Ytitle, leg, m_colors);
    if (opt.find("-noplot") != string::npos) return frame;

    TString logstr = "";
    if (opt.find("-log") != string::npos) logstr = "_log";
    TCanvas * c = new TCanvas();
    if (opt.find("-bw") != string::npos) c->SetGrayscale();
    TH1 * residuals = NULL;
    string pullopt = "p";
    if (opt.find("resid") != string::npos) pullopt = "r";
    if (data && (opt.find("pulls") != string::npos || opt.find("resid") != string::npos)) residuals = getPulls(frame, NULL, pullopt);

    // If "-H" option draw pulls distribution too

    if (opt.find("-H") != string::npos)
    {
        TH1D * resH = NULL;
        if (pullopt == "p") resH = new TH1D( "rH" + m_name, "Pulls distribution", 15, -6, 6 );
        else resH = new TH1D( "rH", "", 15, -3, 3 );
        for (int i = 0; i < residuals->GetNbinsX(); i++) resH->Fill(residuals->GetBinContent(i));
        gStyle->SetOptStat(0);
        gStyle->SetOptFit(1011);
        resH->GetXaxis()->SetTitle("Pulls");
        resH->GetYaxis()->SetTitle("Bins");
        resH->Draw();
        resH->Fit("gaus");
        c->Print(m_name + "_pullsHist.pdf");
    }

    // "-andpulls" option draws pulls on the same canvas as the main frame otherwise 2 different pdfs are created

    if (residuals && opt.find("-andpulls") != string::npos)
    {
        TPad * plotPad = new TPad("plotPad", "", .005, .25, .995, .995);
        TPad * resPad = new TPad("resPad", "", .005, .015, .995, .248);
        if (opt.find("-attach") != string::npos)
        {
            plotPad = new TPad("plotPad", "", .005, .35, .995, .995);
            resPad = new TPad("resPad", "", .005, .005, .995, .35);

            residuals->SetMinimum(-6.);
            residuals->SetMaximum(6.);
        }

        plotPad->Draw();
        resPad->Draw();

        resPad->cd();
        if (opt.find("-attach") != string::npos)
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
        if (opt.find("-attach") != string::npos)
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
        if (opt.find("-attach") != string::npos)
        {
            gPad->SetBottomMargin(1e-5);
            frame->SetMinimum(0.01);
        }
        logstr += "_fitAndRes";
    }
    else if (residuals)
    {
        if (Xtitle != "") residuals->GetXaxis()->SetTitle(Xtitle);
        else residuals->GetXaxis()->SetTitle(m_var->GetName());
        residuals->GetYaxis()->SetTitle("pulls");
        residuals->Draw();
        if (opt.find("-eps") != string::npos) c->Print(m_name + logstr + "_residuals.eps");
        else c->Print(m_name + logstr + "_residuals.pdf");
    }

    // If set draw legend, box for fit failed warning, LHCb

    if (opt.find("-log") != string::npos) gPad->SetLogy();

    if (opt.find("-noleg") == string::npos)
    {
        if ( opt.find("-noborder") != string::npos )
            leg->SetBorderSize(0);
        leg->SetFillStyle(0);
        if (opt.find("-legf") != string::npos) {
            leg->SetFillStyle(1001);
            leg->SetFillColor(kWhite);
        }
        frame->addObject(leg);
        leg->Draw("same");
    }
    if (opt.find("-lhcb") != string::npos)
    {
        double x1 = 0.05, x2 = 0.25, y1 = 0.80, y2 = 0.97;
        size_t pos = opt.find("-lhcb[") + 6;
        if ( pos != string::npos )
        {
            string ss = opt.substr(pos, string::npos);
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

        if (opt.find("-lhcbdx") != string::npos)
        {
            tbox = new TPaveText(gStyle->GetPadRightMargin() + 0.63,
                                 0.80 - gStyle->GetPadTopMargin(),
                                 gStyle->GetPadRightMargin() + 0.83,
                                 0.97 - gStyle->GetPadTopMargin(),
                                 "BRNDC");
        }
        tbox->AddText("LHCb");
        tbox->SetFillStyle(0);
        tbox->SetTextAlign(12);
        tbox->SetBorderSize(0);
        frame->addObject(tbox);
    }
    if (data && fitRes && opt.find("-quality") != string::npos )
    {
        TPaveText * tbox = new TPaveText(0.4, 0.5, 0.9, 0.6, "BRNDC");
        tbox->SetFillStyle(0);
        tbox->AddText(Form("edm = %e, covQual = %i)", fitRes->edm(), fitRes->covQual()));
        tbox->SetBorderSize(0);
        frame->addObject(tbox);
    }

    frame->Draw();


    // Print

    if (opt.find("-none") == string::npos)
    {
        TString pname = m_name;
        if (!data) pname = "model_" + m_name;
        if (title != "") pname = title;
        pname = pname.ReplaceAll(" ", "").ReplaceAll("#rightarrow", "2");
        pname = pname.ReplaceAll("#", "").ReplaceAll("__", "_");
        pname = pname.ReplaceAll("{", "").ReplaceAll("}", "").ReplaceAll("(", "_");
        if (opt.find("-vname") != string::npos) pname += ("_" + (TString)myvar->GetName());

        if (opt.find("-eps") != string::npos) c->Print(pname + logstr + ".eps");
        if (opt.find("-animated") != string::npos) {
            size_t pos = opt.find("-animated") + 10;
            string ss = opt.substr(pos, string::npos);
            if ((TString(ss)).Atoi() <= 0)
            {   //if the given parameter is not an integer, we trow and error and ignore the parameter
                Error("Print", "In -animated, the given parameter should be a positive integer, expressing the delay after each image, in tens of ms.");
                ss = "";
            }
            c->Print(pname + logstr + ".gif++" + ss + "++");
        }
        else if (opt.find("-allformats") != string::npos)
        {
            c->Print(pname + logstr + ".eps");
            c->Print(pname + logstr + ".pdf");
            c->Print(pname + logstr + ".C");
            c->Print(pname + logstr + ".png");
        }
        else c->Print(pname + logstr + ".pdf");
    }

    if (residuals) delete residuals;
    delete c;
    return frame;
}




//Get all paramaters of only signal parameters

Str2VarMap ModelBuilder::GetSigParams(string opt)
{
    return getParamList(m_sig, m_vars, opt);
}


Str2VarMap ModelBuilder::GetParams(string opt)
{
    return getParamList(m_model, m_vars, opt);
}


//Returns the integral of the "total bkg" bunction in a range.
//Return all range (namely the value of nbkg) if no range is given.

double ModelBuilder::GetNBkgVal(double min, double max, double * valerr, RooFitResult * fitRes)
{
    GetTotNBkg();

    if (valerr && fitRes) *valerr = m_nbkg->getPropagatedError(*fitRes);
    if (min == max) return m_nbkg->getVal();

    m_var->setRange("myrange", min, max);
    RooAbsReal * integ = m_bkg->createIntegral(*m_var, NormSet(*m_var), Range("myrange"));
    RooFormulaVar * nbkgval = new RooFormulaVar("nbkgval", "nbkgval", (TString)m_nbkg->GetName() + " * " + (TString)integ->GetName(), RooArgSet(*m_nbkg, *integ));
    double integral = m_nbkg->getVal() * integ->getVal();
    if (valerr && fitRes) *valerr = nbkgval->getPropagatedError(*fitRes);
    m_var->setRange(m_tmpvar->getMin(), m_tmpvar->getMax());
    return integral;
}

double ModelBuilder::GetNSigVal(double min, double max, double * valerr, RooFitResult * fitRes)
{
    bool isRooAbs = ((string)typeid(m_nsig).name()).find("Abs") != string::npos;

    double sigval = m_nsig->getVal();
    if (valerr && !isRooAbs) *valerr = ((RooRealVar *)m_nsig)->getError();
    else if (valerr && fitRes) *valerr = m_nsig->getPropagatedError(*fitRes);
    if (min == max) return sigval;

    m_var->setRange("myrange", min, max);
    RooAbsReal * integ = m_sig->createIntegral(*m_var, NormSet(*m_var), Range("myrange"));
    double res = sigval * integ->getVal();

    RooFormulaVar * nsigval = new RooFormulaVar("nsigval",
            "nsigval", (TString)m_nsig->GetName() + " * " + (TString)integ->GetName(),
            RooArgSet(*m_nsig, *integ));

    /*
    m_var->setRange("myfitrange",min,max);
    fit_integ = m_sig->createIntegral(*m_var,NormSet(*m_var),Range("myfitrange"));
    double norm = fit_integ->getVal();
    res /= norm;
    nsigval = new RooFormulaVar("nsigval",
                "nsigval",(TString)m_nsig->GetName() + " * " + (TString)integ->GetName()  + " / " + (TString)fit_integ->GetName(),
                RooArgSet(*m_nsig,*integ,*fit_integ));
    */

    if (valerr && fitRes) *valerr = nsigval->getPropagatedError(*fitRes);
    m_var->setRange(m_tmpvar->getMin(), m_tmpvar->getMax());
    return res;
}

double ModelBuilder::GetSigVal(double * valerr, RooFitResult * fitRes)
{
    return GetNSigVal(0, 0, valerr, fitRes);
}

double ModelBuilder::GetSigVal(double * errHi, double * errLo)
{
    string tnsig = typeid(m_nsig).name();
    if (tnsig.find("Abs") != string::npos) cout << "WARNING: nsig is not a RooRealVar! Error will not make sense." << endl;
    *errHi = ((RooRealVar *)m_nsig)->getErrorHi();
    *errLo = ((RooRealVar *)m_nsig)->getErrorLo();
    return m_nsig->getVal();
}



//Creates a RooFormulaVar given by the sum of all single background yields  nbkg = nbkg1 + bkg1 + ...

RooAbsReal * ModelBuilder::GetTotNBkg()
{
    if (m_totBkgMode || m_bkg_fractions.size() == 0) return m_nbkg;
    if (m_bkg_fractions.size() == 1) { m_nbkg = m_bkg_fractions[0]; return m_nbkg; }

    stringstream formula;
    RooArgList * bkgList = new RooArgList("nbkgList_" + m_name);
    for (unsigned i = 0; i < m_bkg_fractions.size(); i++)
    {
        if (i > 0) formula << " + ";
        formula << m_bkg_fractions[i]->GetName();
        bkgList->add(*(m_bkg_fractions[i]));
    }
    m_nbkg = new RooFormulaVar("nbkg_tot", formula.str().c_str(), *bkgList);
    return m_nbkg;
}

RooAbsPdf * ModelBuilder::CalcTotBkg()
{
    if (m_totBkgMode || m_bkg_fractions.size() <= 1) return m_bkg;

    RooAbsReal * m_nbkg = GetTotNBkg();
    RooArgSet * pdfs = new RooArgSet("BkgPdfs");
    RooArgSet * fracs = new RooArgSet("BkgFracs");
    for (unsigned i = 0; i < m_bkg_fractions.size(); i++)
    {
        pdfs->add(*m_bkg_components[i]);
        if (i != m_bkg_fractions.size() - 1)
        {
            TString fname = m_bkg_fractions[i]->GetName() + (TString)"_frac";
            double fracv = m_bkg_fractions[i]->getVal() / ((double) m_nbkg->getVal());
            RooRealVar * frac = new RooRealVar(fname, fname, fracv, 0., 1.);
            fracs->add(*frac);
        }
    }

    m_bkg = new RooAddPdf("totbkg", "totbkg", *pdfs, *fracs);
    return m_bkg;
}




// Returns the signal fraction at a given value of the observable (s-Weight).

float ModelBuilder::GetReducedSWeight(float value)
{
    m_var->setVal(value);
    return (m_nsig->getVal() * m_sig->getVal(*m_var)) / ((m_nsig->getVal() + m_nbkg->getVal()) * m_model->getVal(*m_var));
}

void ModelBuilder::PrintComposition(float min, float max, RooFitResult * fitRes)
{
    GetTotNBkg();
    if (max > min) m_var->setRange("myrange", min, max);
    else { m_var->setRange("myrange", m_var->getMin(), m_var->getMax()); min = m_var->getMin(); max = m_var->getMax(); }

    cout << "Composition in [" << min << "," << max << "]" << endl;

    RooAbsReal * integ = m_model->createIntegral(*m_var, NormSet(*m_var), Range("myrange"));
    RooAbsReal * totalf = new RooFormulaVar("totalf", "", "( " + (TString)m_nsig->GetName() + " + " + (TString)m_nbkg->GetName() + ") * " + (TString)integ->GetName(), RooArgSet(*m_nsig, *m_nbkg, *integ));

    RooAbsReal * siginteg = m_sig->createIntegral(*m_var, NormSet(*m_var), Range("myrange"));
    RooAbsReal * sigf = new RooFormulaVar("sigf", "", (TString)m_nsig->GetName() + " * " + (TString)siginteg->GetName() + " / " + totalf->GetName(), RooArgSet(*m_nsig, *siginteg, *totalf));
    cout << "Signal : " << 100 * sigf->getVal();
    if (fitRes) cout << " +/- " << sigf->getPropagatedError(*fitRes);
    cout << "%" << endl;

    for (unsigned b = 0; b < m_bkg_components.size(); b++)
    {
        RooAbsReal * bkginteg = m_bkg_components[b]->createIntegral(*m_var, NormSet(*m_var), Range("myrange"));
        RooAbsReal * bkgf = new RooFormulaVar("bkgf", "", (TString)m_bkg_fractions[b]->GetName() + " * " + (TString)bkginteg->GetName() + " / " + totalf->GetName(), RooArgSet(*m_bkg_fractions[b], *bkginteg, *totalf));
        cout << m_bkg_fractions[b]->GetName() << " : " << 100 * bkgf->getVal();
        if (fitRes) cout << " +/- " << bkgf->getPropagatedError(*fitRes);
        cout << "%" << endl;
    }
}

double ModelBuilder::GetSigFraction(float min, float max, double * valerr, RooFitResult * fitRes)
{
    GetTotNBkg();
    double svalerr = 0, bvalerr = 0;
    double sval = GetNSigVal(min, max, &svalerr, fitRes);
    double bval = GetNBkgVal(min, max, &bvalerr, fitRes);
    (*valerr) = (sval / (bval + sval)) * (TMath::Sqrt( TMath::Power(svalerr / sval, 2) + TMath::Power(bvalerr / bval, 2) ));

    return sval / (bval + sval);
}


double ModelBuilder::GetSOverB(float min, float max, double * valerr, RooFitResult * fitRes)
{
    GetTotNBkg();
    double svalerr = 0, bvalerr = 0;
    double sval = GetNSigVal(min, max, &svalerr, fitRes);
    double bval = GetNBkgVal(min, max, &bvalerr, fitRes);
    (*valerr) = (sval / bval) * (TMath::Sqrt( TMath::Power(svalerr / sval, 2) + TMath::Power(bvalerr / bval, 2) ));

    return sval / bval;
}


RooWorkspace * ModelBuilder::SaveToRooWorkspace(string option)
{
    RooWorkspace * ws = new RooWorkspace("ws_" + m_name);
    if (option != "") ws->SetName("ws_" + m_name + "_" + option);
    if (m_pmode == "v") cout << endl << m_name << ": SaveToRooWorkspace" << endl;


    if (option == "")
        if (m_model)
        {
            ws->import(*m_model);
            if (m_pmode == "v") cout << "m_model: " << m_model->GetName() << endl;
        }

    if (option == "sig")
        if (m_sig)
        {
            ws->import(*m_sig);
            if (m_pmode == "v") cout << "signal: " << m_sig->GetName() << endl;
        }

    if (option == "bkg")
        if (m_bkg)
        {
            ws->import(*m_bkg);
            if (m_pmode == "v") cout << "background: " << m_bkg->GetName() << endl;
        }

    return ws;
}



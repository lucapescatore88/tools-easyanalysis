#include "ModelBuilder.hpp"
#include "RooMultiVarGaussian.h"
#include <algorithm>

using namespace RooFit;


/* ********************************************     ModelBuilder methods  *********************************************/



string ModelBuilder::pmode = "v";

void ModelBuilder::SetModel(RooAbsPdf * _model)
{ 
    model = _model;
    sig = model;
    Str2VarMap pars = GetParams();
    if(pars["nsig"]) nsig = pars["nsig"];
}


RooAbsPdf * ModelBuilder::GetParamsGaussian(RooFitResult * fitRes)
{
      RooArgSet * params = model->getParameters(RooDataSet("v","",RooArgSet(*var)));
      
      return (RooAbsPdf *)(new RooMultiVarGaussian(
		name+"_multivar_gauss",name+"_multivar_gauss",
		*params,fitRes->covarianceMatrix()));
}


RooDataSet * ModelBuilder::GetParamsVariations(int nvariations, RooFitResult * fitRes)
{
      RooArgSet * params = model->getParameters(RooDataSet("v","",RooArgSet(*var)));
      
	cout << "Making variations of " << endl;	
	params->Print();
	cout << "Matrix - fit quality"<< fitRes->covQual() << endl;
	fitRes->covarianceMatrix().Print();
	cout << "End matrix" << endl;
	
      RooMultiVarGaussian * gauss = new RooMultiVarGaussian(
		name+"_multivar_gauss",name+"_multivar_gauss",
		*params,fitRes->covarianceMatrix());
      gauss->Print();
	cout << "After" << endl;
      RooDataSet * variations = gauss->generate(*params,nvariations);
	cout << "Generated" << endl;
      return variations;

/*
    TRandom3  rndm(0);
    
    while ( out->numEntries() < nvariations )
    {
      RooArgSet * set = new RooArgSet("row");	

      RooArgSet * params = model->getParameters(RooDataSet("v","",RooArgSet(*var)));
      TIterator *it = params->createIterator();
      RooRealVar * arg;
      while( (arg=(RooRealVar*)it->Next()) )
      {
        if(arg->getAttribute("Constant")) continue;
        
	set->add(RooRealVar(arg->GetName(),
			    arg->GetTitle(),
			    rndm.Gaus(arg->getVal(),arg->getError()),
			    arg->getMin(),
	   		    arg->getMax()));
	}
	if(out) out->addFast(*set);
	else out = new RooDataSet(name+"_param_variations",name+"_param_variations",*set);
    }

    return out;
	*/
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
    myname = "_"+name;

    RooArgList * bkgList = new RooArgList("bkgList_"+name);
    RooArgList * fracList = new RooArgList("fracList_"+name);

    bool noBkg = (optstr.find("-nobkg") != string::npos);
    bool doExp = (optstr.find("-exp") != string::npos);
    if( doExp ) AddBkgComponent("exp","Exp",1.e4,"-ibegin");
    if( totBkgMode ) bkg_fractions.clear();

    for(unsigned i = 0; i < bkg_components.size(); i++)
    {
        bkgList->add(*(bkg_components[i]));
        if(!totBkgMode) fracList->add(*(bkg_fractions[i]));
        else { if(i>0) fracList->add(*(new RooRealVar(Form("frac_%i",i),"Frac",0.5,0.,1))); }
    }

    if(bkg_components.size()>1)
    { 
        bkg = new RooAddPdf("totbkg"+myname,"totbkg"+myname,*bkgList,*fracList);
    }
    else if(bkg_components.size()==1)
    {
	//bkg = (RooAbsPdf*)(bkg_components[0]->Clone("totbkg"+myname));
	    bkg = bkg_components[0];
        bkg->SetName("totbkg"+myname);
	//bkg = new RooAddPdf("totbkg"+myname,"totbkg"+myname,*bkg_components[0]);
    }
    
    GetTotNBkg();

    if(!sig) { cout << "WARNING: Signal not set!!" << endl; return NULL; }
    RooArgList pdfs(*sig), nevts(*nsig);

    if(!noBkg && !bkg_components.empty())
    {
        if(totBkgMode) model = new RooAddPdf("model"+myname,"model"+myname,RooArgSet(*sig,*bkg),RooArgSet(*nsig,*nbkg));
        else
        {
            pdfs.add(*bkgList);
            nevts.add(*fracList);
            model = new RooAddPdf("model"+myname,"model"+myname,pdfs,nevts);
        }
    }
    else model = sig; 
    
    if(pmode=="v")
    {
        cout << "\n" << name << ": Initialized Correctly! The model is:" << endl;
        model->Print();
    }
    isvalid = true;
    return model;
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
    for(size_t v = 0; v < myvars.size(); v++)
    {
        Print(title, Xtitle, opt, data, bins, vector<string>(), (double *)NULL, fitRes, Ytitle, myvars[v]);
    }
}


RooPlot * ModelBuilder::Print(TString title, TString Xtitle, string opt, RooAbsData * data, int bins,
        vector<string> regStr, double * range, RooFitResult * fitRes, TString Ytitle, RooRealVar * myvar)
{
    transform(opt.begin(), opt.end(), opt.begin(), ::tolower);
    RooPlot* frame = NULL;
    TLegend * leg = new TLegend(0.65,0.7,0.76,0.9);
    if(opt.find("-leg")!=string::npos)
    {
        size_t pos = opt.find("-leg")+5;
        string ss = opt.substr(pos,string::npos);
        double x1 = (TString(ss)).Atof();
        pos = ss.find(",")+1;
        ss = ss.substr(pos,string::npos);
        double y1 = (TString(ss)).Atof();
        pos = ss.find(",")+1;
        ss = ss.substr(pos,string::npos);
        double x2 = (TString(ss)).Atof();
        pos = ss.find(",")+1;
        ss = ss.substr(pos,string::npos);
        double y2 = (TString(ss)).Atof();

        leg = new TLegend(x1,y1,x2,y2);
    }

    Xtitle = Xtitle.ReplaceAll("__var__","");

    if(!myvar) myvar = var;

    if(isValid())
    {
        // Create main frame

        frame = GetFrame(myvar, data, model, opt, bins, range, regStr, Xtitle, Ytitle, leg, mycolors);
        if(opt.find("-noplot")!=string::npos) return frame;

        TString logstr = "";
        if(opt.find("-log")!=string::npos) logstr = "_log";
        TCanvas * c = new TCanvas();
        if(opt.find("-bw")!=string::npos) c->SetGrayscale();
        TH1 * residuals = NULL;
        string pullopt = "p";
        if(opt.find("resid")!=string::npos) pullopt = "r";
        if(data && (opt.find("pulls")!=string::npos || opt.find("resid")!=string::npos)) residuals = GetPulls(frame,NULL,pullopt);

        // If "-H" option draw pulls distribution too

        if(opt.find("-H")!=string::npos)
        {
            TH1D * resH = NULL;
            if(pullopt=="p") resH = new TH1D( "rH"+name, "Pulls distribution", 15, -5, 5 );
            else resH = new TH1D( "rH", "", 15, -3, 3 );
            for(int i = 0; i < residuals->GetNbinsX(); i++) resH->Fill(residuals->GetBinContent(i));
            gStyle->SetOptStat(0);
            gStyle->SetOptFit(1011);
            resH->GetXaxis()->SetTitle("Pulls");
            resH->GetYaxis()->SetTitle("Bins");
            resH->Draw();
            resH->Fit("gaus");
            c->Print(name+"_pullsHist.pdf");
        }

        // "-andpulls" option draws pulls on the same canvas as the main frame otherwise 2 different pdfs are created

        if(residuals && opt.find("-andpulls")!=string::npos)
        {
            TPad * plotPad = new TPad("plotPad", "", .005, .25, .995, .995);
            TPad * resPad = new TPad("resPad", "", .005, .015, .995, .248);
            plotPad->Draw();
            resPad->Draw();

            resPad->cd();

            TAxis* yAxis = residuals->GetYaxis();
            TAxis* xAxis = residuals->GetXaxis();
            yAxis->SetNdivisions(504);
            double old_size = yAxis->GetLabelSize();
            yAxis->SetLabelSize( old_size / 0.33 );
            yAxis->SetTitleSize( old_size * 3.6  );
            yAxis->SetTitleOffset(0.3);
            yAxis->SetTitle("Pulls");
            xAxis->SetLabelSize( xAxis->GetLabelSize() / 0.33 );

            resPad->cd();
            residuals->Draw();

            TLine *lc = new TLine(xAxis->GetXmin(),  0, xAxis->GetXmax(),  0);
            TLine *lu = new TLine(xAxis->GetXmin(),  3, xAxis->GetXmax(),  3);
            TLine *ld = new TLine(xAxis->GetXmin(), -3, xAxis->GetXmax(), -3);

            lc->SetLineColor(kGray+2);
            lu->SetLineColor(kGray+1);
            ld->SetLineColor(kGray+1);

            lc->SetLineStyle(2);
            lu->SetLineStyle(2);
            ld->SetLineStyle(2);

            lc->Draw("same");
            lu->Draw("same");
            ld->Draw("same");

            residuals->Draw("same");

            plotPad->cd();
            logstr+="_fitAndRes";
        }
        else if(residuals)
        {
            if(Xtitle!="") residuals->GetXaxis()->SetTitle(Xtitle);
            else residuals->GetXaxis()->SetTitle(var->GetName());
            residuals->GetYaxis()->SetTitle("pulls");
            residuals->Draw();
            if(opt.find("-eps")!=string::npos) c->Print(name+logstr+"_residuals.eps");
            else c->Print(name+logstr+"_residuals.pdf");
        }

        // If set draw legend, box for fit failed warning, LHCb

    if(opt.find("-log")!=string::npos) gPad->SetLogy(); //frame->SetMinimum(0.5); }
    
    if(opt.find("-noleg")==string::npos)
    {
        if( opt.find("-noborder")!=string::npos ) 
	    leg->SetBorderSize(0); 
        leg->SetFillStyle(0);
        if(opt.find("-legf")!=string::npos) {
            leg->SetFillStyle(1001);
            leg->SetFillColor(kWhite);
        }
        frame->addObject(leg);
        leg->Draw("same");
    }
    if(opt.find("-lhcb")!=string::npos)
    {
	double x1 = 0.05, x2 = 0.25, y1 = 0.80, y2 = 0.97;
        size_t pos = opt.find("-lhcb[")+6;
       	if( pos != string::npos )
	{
		string ss = opt.substr(pos,string::npos);
        	x1 = (TString(ss)).Atof();
       		pos = ss.find(",")+1;
       		ss = ss.substr(pos,string::npos);
        	y1 = (TString(ss)).Atof();
        	pos = ss.find(",")+1;
        	ss = ss.substr(pos,string::npos);
        	x2 = (TString(ss)).Atof();
        	pos = ss.find(",")+1;
        	ss = ss.substr(pos,string::npos);
        	y2 = (TString(ss)).Atof();
	}

        TPaveText * tbox = new TPaveText(gStyle->GetPadLeftMargin() + x1,
                y1 - gStyle->GetPadTopMargin(),
                gStyle->GetPadLeftMargin() + x2,
                y2 - gStyle->GetPadTopMargin(),
                "BRNDC");

        if(opt.find("-lhcbdx")!=string::npos)
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
    if(data && fitRes && opt.find("-quality")!=string::npos )
    {
        TPaveText * tbox = new TPaveText(0.4, 0.5, 0.9, 0.6, "BRNDC");
        tbox->SetFillStyle(0);
        tbox->AddText(Form("edm = %e, covQual = %i)",fitRes->edm(),fitRes->covQual()));
        tbox->SetBorderSize(0);
        frame->addObject(tbox);
    }

    frame->Draw();


    // Print

    if(opt.find("-none")==string::npos)
    {
        TString pname = name;
        if(!data) pname = "model_"+name;
        if(title!="") pname = title;
        pname = pname.ReplaceAll(" ","").ReplaceAll("#rightarrow","2").ReplaceAll("#","").ReplaceAll("__","_");
        if(opt.find("-vname")!=string::npos) pname+=("_"+(TString)myvar->GetName());

        if(opt.find("-eps")!=string::npos) c->Print(pname+logstr+".eps");
        else if(opt.find("-allformats")!=string::npos)
        {
            c->Print(pname+logstr+".eps");
            c->Print(pname+logstr+".pdf");
            c->Print(pname+logstr+".C");
            c->Print(pname+logstr+".png");
        }
        else c->Print(pname+logstr+".pdf");
    }

    if(residuals) delete residuals;
    delete c;
}
else cout << "**** WARNING: Model is not valid, probably not initialised. *****" << endl;

return frame;
}




//Get all paramaters of only signal parameters

Str2VarMap ModelBuilder::GetSigParams(string opt)
{
    return GetParamList(sig, var, opt);
}


Str2VarMap ModelBuilder::GetParams(string opt)
{
    return GetParamList(model, var, opt);
}


//Returns the integral of the "total bkg" bunction in a range.
//Return all range (namely the value of nbkg) if no range is given.

double ModelBuilder::GetNBkgVal(double min, double max, double * valerr, RooFitResult * fitRes)
{
    GetTotNBkg();

    if(valerr && fitRes) *valerr = nbkg->getPropagatedError(*fitRes);
    if(min == max) return nbkg->getVal();

    var->setRange("myrange",min,max);
    RooAbsReal * integ = bkg->createIntegral(*var,NormSet(*var),Range("myrange"));
    RooFormulaVar * nbkgval = new RooFormulaVar("nbkgval","nbkgval",(TString)nbkg->GetName() + " * " + (TString)integ->GetName(),RooArgSet(*nbkg,*integ));
    double integral = nbkg->getVal()*integ->getVal();
    if(valerr && fitRes) *valerr = nbkgval->getPropagatedError(*fitRes);
    var->setRange(tmpvar->getMin(),tmpvar->getMax());
    return integral;
}

double ModelBuilder::GetNSigVal(double min, double max, double * valerr, RooFitResult * fitRes, double fmin, double fmax)
{
    bool isRooAbs = ((string)typeid(nsig).name()).find("Abs")!=string::npos;

    double sigval = nsig->getVal();
    if(valerr && !isRooAbs) *valerr = ((RooRealVar *)nsig)->getError();
    else if(valerr && fitRes) *valerr = nsig->getPropagatedError(*fitRes); 
    if(min == max) return sigval;

    var->setRange("myrange",min,max);
    RooAbsReal * integ = sig->createIntegral(*var,NormSet(*var),Range("myrange"));
    double res = sigval*integ->getVal();
    RooAbsReal * fit_integ = NULL;
    
    RooFormulaVar * nsigval = new RooFormulaVar("nsigval",
            "nsigval",(TString)nsig->GetName() + " * " + (TString)integ->GetName(),
            RooArgSet(*nsig,*integ));
    if(fmin > fmax)
    {
        var->setRange("myfitrange",min,max);
        fit_integ = sig->createIntegral(*var,NormSet(*var),Range("myfitrange"));
        double norm = fit_integ->getVal();
        res /= norm;
        nsigval = new RooFormulaVar("nsigval",
                "nsigval",(TString)nsig->GetName() + " * " + (TString)integ->GetName()  + " / " + (TString)fit_integ->GetName(),
                RooArgSet(*nsig,*integ,*fit_integ));
    }
    if(valerr && fitRes) *valerr = nsigval->getPropagatedError(*fitRes);
    var->setRange(tmpvar->getMin(),tmpvar->getMax());
    return res;
}

double ModelBuilder::GetSigVal(double * valerr, RooFitResult * fitRes) 
{
    return GetNSigVal(0,0,valerr,fitRes);
}

double ModelBuilder::GetSigVal(double * errHi, double * errLo) 
{
    string tnsig = typeid(nsig).name();
    if(tnsig.find("Abs")!=string::npos) cout << "WARNING: nsig is not a RooRealVar! Error will not make sense." << endl;
    *errHi = ((RooRealVar *)nsig)->getErrorHi();
    *errLo = ((RooRealVar *)nsig)->getErrorLo();
    return nsig->getVal();
}



//Creates a RooFormulaVar given by the sum of all single background yields  nbkg = nbkg1 + bkg1 + ...

RooAbsReal * ModelBuilder::GetTotNBkg()
{
    if(totBkgMode || bkg_fractions.size()==0) return nbkg;
    if(bkg_fractions.size()==1) { nbkg = bkg_fractions[0]; return nbkg; }

    stringstream formula;
    RooArgList * bkgList = new RooArgList("nbkgList_"+name);
    for(unsigned i = 0; i < bkg_fractions.size(); i++)
    {
        if(i > 0) formula << " + ";
        formula << bkg_fractions[i]->GetName();
        bkgList->add(*(bkg_fractions[i]));
    }
    nbkg = new RooFormulaVar("nbkg_tot",formula.str().c_str(),*bkgList);
    return nbkg;
}

RooAbsPdf * ModelBuilder::CalcTotBkg()
{
	if(totBkgMode || bkg_fractions.size()<=1) return bkg;

	RooAbsReal * nbkg = GetTotNBkg();
	RooArgSet * pdfs = new RooArgSet("BkgPdfs");
	RooArgSet * fracs = new RooArgSet("BkgFracs");
	for(unsigned i = 0; i < bkg_fractions.size(); i++)
	{
		pdfs->add(*bkg_components[i]);
		if(i!=bkg_fractions.size()-1)
		{
			TString fname = bkg_fractions[i]->GetName()+(TString)"_frac";
			double fracv = bkg_fractions[i]->getVal() / ((double) nbkg->getVal());
			RooRealVar * frac = new RooRealVar(fname,fname,fracv,0.,1.);
			fracs->add(*frac);
		}
	}

	bkg = new RooAddPdf("totbkg","totbkg",*pdfs,*fracs);
	return bkg;
}




// Returns the signal fraction at a given value of the observable (s-Weight).

float ModelBuilder::GetReducedSWeight(float value)
{
    var->setVal(value);
    return (nsig->getVal() * sig->getVal(*var)) / ((nsig->getVal() + nbkg->getVal()) * model->getVal(*var));
}

void ModelBuilder::PrintComposition(float min, float max, RooFitResult * fitRes)
{
    GetTotNBkg();
    if(max > min) var->setRange("myrange",min,max);
    else { var->setRange("myrange",var->getMin(),var->getMax()); min = var->getMin(); max = var->getMax(); }

    cout << "Composition in [" << min << "," << max << "]" << endl;

    RooAbsReal * integ = model->createIntegral(*var,NormSet(*var),Range("myrange"));
    RooAbsReal * totalf = new RooFormulaVar("totalf","","( " +(TString)nsig->GetName() + " + " + (TString)nbkg->GetName() + ") * " + (TString)integ->GetName(),RooArgSet(*nsig,*nbkg,*integ));

    RooAbsReal * siginteg = sig->createIntegral(*var,NormSet(*var),Range("myrange"));
    RooAbsReal * sigf = new RooFormulaVar("sigf","",(TString)nsig->GetName() + " * " + (TString)siginteg->GetName() + " / " + totalf->GetName(),RooArgSet(*nsig,*siginteg,*totalf));
    cout << "Signal : " << 100 * sigf->getVal();
    if(fitRes) cout << " +/- " << sigf->getPropagatedError(*fitRes);
    cout << "%" << endl;

    for(unsigned b = 0; b < bkg_components.size(); b++)
    {
        RooAbsReal * bkginteg = bkg_components[b]->createIntegral(*var,NormSet(*var),Range("myrange"));
        RooAbsReal * bkgf = new RooFormulaVar("bkgf","",(TString)bkg_fractions[b]->GetName() + " * " + (TString)bkginteg->GetName() + " / " + totalf->GetName(),RooArgSet(*bkg_fractions[b],*bkginteg,*totalf));
        cout << bkg_fractions[b]->GetName() << " : " << 100 * bkgf->getVal();
        if(fitRes) cout << " +/- " << bkgf->getPropagatedError(*fitRes);
        cout << "%" << endl;
    }	
}

double ModelBuilder::GetSigFraction(float min, float max, double * valerr, RooFitResult * fitRes)
{
    GetTotNBkg();
    double svalerr = 0, bvalerr = 0;
    double sval = GetNSigVal(min, max, &svalerr, fitRes);
    double bval = GetNBkgVal(min, max, &bvalerr, fitRes);
    (*valerr) = (sval / (bval + sval)) * (TMath::Sqrt( TMath::Power(svalerr/sval,2) + TMath::Power(bvalerr/bval,2) ));

    return sval / (bval + sval);
}


double ModelBuilder::GetSOverB(float min, float max, double * valerr, RooFitResult * fitRes)
{
    GetTotNBkg();
    double svalerr = 0, bvalerr = 0;
    double sval = GetNSigVal(min, max, &svalerr, fitRes);
    double bval = GetNBkgVal(min, max, &bvalerr, fitRes);
    (*valerr) = (sval / bval) * (TMath::Sqrt( TMath::Power(svalerr/sval,2) + TMath::Power(bvalerr/bval,2) ));

    return sval / bval;
}





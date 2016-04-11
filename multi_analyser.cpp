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

	if((string)ana[0]->GetPrintLevel() == "v") if(combData)  combData->Print();
	if((string)ana[0]->GetPrintLevel() == "v") if(combModel) combModel->Print();
	//if((string)ana[0]->GetPrintLevel() == "v") if(combModel) printParams(combModel, *vars);
	//if((string)ana[0]->GetPrintLevel() == "v") if(combModel) printParams(combModel);

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
	RooCmdArg initialHesse = InitialHesse(kFALSE);
	if(opt.find("-initialHesse")!=string::npos) initialHesse = InitialHesse(kTRUE);
	RooCmdArg hesse = Hesse(kTRUE);
	if(opt.find("-nohesse")!=string::npos) hesse = Hesse(kFALSE);

    if(!combModel || !combData) { cout << "Model or data not set" << endl; return map < string, RooPlot * >(); }

	if(min==max) fitResult = combModel->fitTo(*combData,Save(),isExtended,isQuiet,useMinos,ExternalConstraints(*constr),hesse,initialHesse);
	else
	{
		for(unsigned i = 0; i < categories.size(); i++)
			ana[i]->GetVariable()->setRange("FitRange",min,max);

		fitResult = combModel->fitTo(*combData,Save(),isExtended,
                isQuiet,useMinos,ExternalConstraints(*constr), hesse, initialHesse);
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


void MultiAnalysis::EnlargeYieldRanges(double factor)
{
	if(!combModel) return;
	
	RooArgSet * params = combModel->getParameters(RooDataSet());
   	TIterator * it = params->createIterator();
    	RooRealVar * arg;
    	while( (arg=(RooRealVar*)it->Next()) )
    	{
        	string varname = (string)arg->GetName();
        	if(varname.find("nsig") != string::npos ||
			varname.find("nbkg") != string::npos )
        	{
			cout << "Scaling" << endl;
			double min = ((RooRealVar*)arg)->getMin();
			double max = ((RooRealVar*)arg)->getMax();
          		if( varname.find("nbkg") != string::npos )
				((RooRealVar*)arg)->setRange(0.,max*factor);
			else ((RooRealVar*)arg)->setRange(min,max*factor);
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
        while( (arg=(RooRealVar*)it->Next()) )
        {
                if(((string)arg->GetName()).find("sample")!=string::npos) continue;
                if(!(arg->getAttribute("Constant"))) continue;  
        
		for(size_t ds = 0; ds < input.size(); ds++)
		{
			RooArgSet * set = (RooArgSet *)input[ds]->get(index);
			RooRealVar * var = (RooRealVar *)set->find(arg->GetName());
			if(var) { arg->setVal(var->getVal()); break; }
		}
        }
}

void MultiAnalysis::RandomizeInitialParams(string option)
{
	if(!combModel) return;
	TRandom3 rndm(0);	

	RooArgSet * params = combModel->getParameters(RooDataSet());
   	TIterator * it = params->createIterator();
    	RooRealVar * arg;
    	while( (arg=(RooRealVar*)it->Next()) )
    	{
		cout << "Randomize" << endl;
		if(((string)arg->GetName()).find("sample")!=string::npos) continue;
		if(arg->getAttribute("Constant")) continue;	
		arg->Print();
        	double val = 0;
		double oldv = arg->getVal();
		double sigma = arg->getError();
		double min = oldv - 3*sigma;
		double max = oldv + 3*sigma;
		if(option=="Gauss") val = rndm.Gaus(oldv,sigma);
		else  val = min + (max - min)*rndm.Uniform();
		arg->setVal(val);
		arg->Print();
	}
}




RooWorkspace * MultiAnalysis::SaveToRooWorkspace()
{
    RooWorkspace * ws = new RooWorkspace("ws_"+name);
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
    if(combModel)
    {
	ws->import(*combModel);
	cout << "combModel: " << combModel->GetName() << endl;
    }
    if(combData)
    {
        ws->import(*combData);
	cout << "combData: " << combData->GetName() << endl;
    }
    /*
    cout << endl;
    ws->Print();
    cout << endl;
    */
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
        if(name.find("combModel")!=string::npos) {
            combModel = (RooSimultaneous*)arg;
	    printParams(combModel);
	}
    }
    cout << endl;
}



RooPlot * MultiAnalysis::PrintSum(string option, TString dovar, string printname, int nbins)
{
    TCanvas * c = new TCanvas();
	size_t posb = option.find("-nbins");
	if (posb != string::npos)
	nbins = ((TString) option.substr(posb+6, string::npos)).Atof();

	if(!init && option.find("-noInit")==string::npos) Initialize();

	TLegend * leg = new TLegend(0.65,0.7,0.76,0.9);
	if(option.find("-leg")!=string::npos)
	{
		size_t pos = option.find("-leg")+5;
		string ss = option.substr(pos,string::npos);
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
	
	if(option.find("-lhcb")!=string::npos)
	{
		double x1 = 0.05, x2 = 0.25, y1 = 0.80, y2 = 0.97;
        	size_t pos = option.find("-lhcb[")+6;
        	if( pos != string::npos )
        	{
                	string ss = option.substr(pos,string::npos);
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

		if(option.find("-lhcbdx")!=string::npos)
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
        if(option.find("-noborder")!=string::npos )
	  leg->SetBorderSize(0);
	pl->addObject(leg);
	pl->Draw();
	c->Print("fit_"+(TString)printname+".pdf");
	if(option.find("-allformats")!=string::npos)
	{
	    c->Print("fit_"+(TString)printname+".C");
	    c->Print("fit_"+(TString)printname+".eps");
	    c->Print("fit_"+(TString)printname+".png");
	}
	c->SetLogy();
	c->Print("fit_"+(TString)printname+"_log.pdf");
	if(option.find("-allformats")!=string::npos)
	{
	    c->Print("fit_"+(TString)printname+"_log.C");
	    c->Print("fit_"+(TString)printname+"_log.eps");
	    c->Print("fit_"+(TString)printname+"_log.png");
	}

	return pl;
}




/*
RooPlot * MultiAnalysis::PrintSum(string option, RooRealVar * dovar)
{
	int nbins = 50;
	size_t posb = option.find("-nbins");
	if (posb != string::npos)
		nbins = ((TString) option.substr(posb+6, string::npos)).Atof();
	
	//RooPlot * pl = ana[0]->Print("-empty"+option,nbins);

	RooArgSet pdfs("pdfs");
	RooArgSet fracs("fracs");
	RooArgSet sigpdfs("sigpdfs");
	RooArgSet sigfracs("sigfracs");
	map< string, RooArgSet > bkgpdfs;
	map< string, RooArgSet > bkgfracs;
	map< string, double > sumNbkg; 
	double sumAll = 0, sumNsig = 0;
	int k = 0;
	vector <string> bnames;

	for(unsigned i = 0; i < categories.size(); i++)
	{
		if(dovar) { if(ana[i]->GetVariable()->GetName() != dovar->GetName()) continue; }
		else if(ana[i]->GetVariable()->GetName() != ana[0]->GetVariable()->GetName()) continue;
		k = i;

		sumAll += ana[i]->GetDataSet()->sumEntries();
		sumNsig += ana[i]->GetNSigPtr()->getVal();

		vector <RooAbsPdf *> components = ana[i]->GetBkgComponents();
		for(unsigned bb = 0; bb < components.size(); bb++)
		{
			size_t pos1 = ((string)components[bb]->GetName()).find("_");
			size_t pos2 = ((string)components[bb]->GetName()).find("_", pos1+1);
			string bname = ((string)components[bb]->GetName()).substr(0,pos2);

			if(i==0)
			{
				bnames.push_back(bname);
				sumNbkg[bname] = ana[i]->GetBkgFractions()[bb]->getVal();
			}
			else if(bkgpdfs.find(bname) != bkgpdfs.end())
				sumNbkg[bname] += ana[i]->GetBkgFractions()[bb]->getVal();
		}
	}
	for(unsigned i = 0; i < categories.size(); i++)
	{
		if(dovar) { if(ana[i]->GetVariable()->GetName() != dovar->GetName()) continue; }
		else if(ana[i]->GetVariable()->GetName() != ana[0]->GetVariable()->GetName()) continue;
		k = i;

		pdfs.add(*ana[i]->GetModel());
		RooRealVar * curfrac = new RooRealVar("frac"+categories[i],"",ana[i]->GetDataSet()->sumEntries());
		fracs.add(*curfrac);
		//sumAll += curfrac->getVal();

		sigpdfs.add(*ana[i]->GetSig());
		RooAbsReal * frac = new RooFormulaVar("fractotsig"+categories[i],"",(TString)ana[i]->GetNSigPtr()->GetName()+Form(" / %e",sumNsig),*ana[i]->GetNSigPtr());
		sigfracs.add(*frac);
		//sumNsig += ana[i]->GetNSigPtr()->getVal();

		vector <RooAbsPdf *> components = ana[i]->GetBkgComponents();
		for(unsigned bb = 0; bb < components.size(); bb++)
		{
			size_t pos1 = ((string)components[bb]->GetName()).find("_");
			size_t pos2 = ((string)components[bb]->GetName()).find("_", pos1+1);
			string bname = ((string)components[bb]->GetName()).substr(0,pos2);
			
			if(i==0)
			{
				RooArgSet bkgc("bkg"+(TString)bname);
				RooArgSet bkgf("bkgfracs"+(TString)bname);
				bkgc.add(*components[bb]);
				RooAbsReal * bkgfrac = new RooFormulaVar("fractot"+bname+categories[i],"",(TString)ana[i]->GetBkgFractions()[bb]->GetName()+Form(" / %e",sumNbkg[bname]),*ana[i]->GetBkgFractions()[bb]);
				//bkgf.add(*(ana[i]->GetBkgFractions()[bb]));
				bkgf.add(* bkgfrac);
				bkgpdfs[bname] = bkgc;
				bkgfracs[bname] = bkgf;
				//sumNbkg[bname] = ana[i]->GetBkgFractions()[bb]->getVal();
			}
			else if(bkgpdfs.find(bname) != bkgpdfs.end())
			{
				bkgpdfs[bname].add(*components[bb]);
				bkgfracs[bname].add((*ana[i]->GetBkgFractions()[bb]));
				//sumNbkg[bname] += ana[i]->GetBkgFractions()[bb]->getVal();
			}
		}
	}

	Analysis * sumAna = new Analysis("SumAna_"+name,"",combData,ana[k]->GetVariable(),(RooRealVar*)NULL);
	sumAna->SetUnits(ana[k]->GetUnits());
	RooAbsPdf * sumsigPdf = new RooAddPdf("sig_sumPdf","",sigpdfs,sigfracs);
	sumAna->SetSignal(sumsigPdf,sumNsig);
	sumAna->GetSig()->SetName("totsig_sumPdf"+(TString)name+"_print");
	cout << sumNsig << endl;
	for(unsigned bb = 0; bb < bnames.size(); bb++)
	{
		string bname = bnames[bb];

		RooAbsPdf * curSumBkgPdf = new RooAddPdf("sumBkgPdf_"+(TString)bname,"",bkgpdfs[bname],bkgfracs[bname]);
		sumAna->addBkgComponent((bname+"tot_print").c_str(),curSumBkgPdf,sumNbkg[bname]);
		cout << bname << "   "  << sumNbkg[bname] << endl;
	}
	sumAna->Initialize("");
	RooPlot * pl = sumAna->Print(option+"-printonly", nbins);

	return pl;
}
*/


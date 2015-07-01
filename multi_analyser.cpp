/***
* class MultiAnalysis 
*
* author: Luca Pescatore
* email : luca.pescatore@cern.ch
* 
* date  : 01/07/2015
***/

#include "multi_analyser.hpp"

using namespace RooFit;

void MultiAnalysis::AddCategory(Analysis * _ana, TString nameCat)
{
	samples->defineType(nameCat);
	categories.push_back(nameCat);
	ana.push_back(_ana);

	if(vars->getSize()==0) vars->add(*_ana->GetVariable());
	else if(!vars->find(_ana->GetVariable()->GetName()))
		vars->add(*_ana->GetVariable());
}


bool MultiAnalysis::Initialize(string opt)
{
	combModel = new RooSimultaneous("combModel","",*samples);
	map<string,RooDataSet*> mymap;
	
	for(unsigned i = 0; i < categories.size(); i++)
	{
		if(opt.find("-I")!=string::npos) ana[i]->Initialize(opt);
		if( !ana[i]->isValid() ) { cout << "ATTENTION!: Analysis " << ana[i]->GetName() << " is not initialized!" << endl; return false; }
		mymap[(string)categories[i]] = (RooDataSet*)(ana[i]->GetDataSet(opt));
		combModel->addPdf(*(ana[i]->GetModel()),categories[i]);
	}

	combData = new RooDataSet("combData","combined datas",*vars,Index(*samples),Import(mymap));

	if((string)ana[0]->GetPrintLevel() == "v") combData->Print();
	if((string)ana[0]->GetPrintLevel() == "v") combModel->Print();
	if((string)ana[0]->GetPrintLevel() == "v") printParams(combModel, *vars);

	init = true;
	return init;
}

map < string, RooPlot * > MultiAnalysis::SimultaneousFit(double min, double max, unsigned nbins, string opt)
{
	if(!init) Initialize(opt);

	if((string)ana[0]->GetPrintLevel() == "v") cout << "--------------- FITTING --------------- " << endl;
		
	RooCmdArg isExtended = Extended(kFALSE);
	if(ana[0]->isExtended()) isExtended = Extended(kTRUE);
	RooCmdArg isQuiet = PrintLevel(2);
	if(opt.find("-quiet")!=string::npos) isQuiet = PrintLevel(-1);
	RooCmdArg useMinos = Minos(kFALSE);
	if(opt.find("-minos")!=string::npos) useMinos = Minos(kTRUE);

	if(min==max) fitResult = combModel->fitTo(*combData,Save(),isExtended,isQuiet,useMinos);
	else
	{
		for(unsigned i = 0; i < categories.size(); i++)
			ana[i]->GetVariable()->setRange("FitRange",min,max);

		fitResult = combModel->fitTo(*combData,Save(),isExtended,Range("FitRange"),isQuiet,useMinos);
	}
		
	map < string, RooPlot * > plots;
	if(opt.find("-noPlot")==string::npos)
		for(unsigned i = 0; i < categories.size(); i++)
		{
			double range[2] = {ana[i]->GetVariable()->getMin(),ana[i]->GetVariable()->getMax()};
			plots[(string)categories[i]] = ana[i]->Print(opt+"-nochi2", nbins, range);//+"-t"+(string)categories[i]);
		}

	double logL = combModel->createNLL(*combData)->getVal();
	cout << "\n" << name << ": LogL = " << logL << endl;

	return plots;
}



RooPlot * MultiAnalysis::PrintSum(string option, RooRealVar * dovar)
{
	int nbins = 50;
	size_t posb = option.find("-nbins");
	if (posb != string::npos)
	nbins = ((TString) option.substr(posb+6, string::npos)).Atof();

	if(!init && option.find("-noInit")==string::npos) Initialize();

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

	for(unsigned i = 0; i < categories.size(); i++)
	{
		if(dovar) { if(ana[i]->GetVariable()->GetName() != dovar->GetName()) continue; }
		else if(ana[i]->GetVariable()->GetName() != ana[0]->GetVariable()->GetName()) continue;
		k = i;
		
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
		
			if(i==0)
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
	}

	RooPlot * pl = ana[k]->Print("-empty"+option,nbins);
	RooAddPdf * sumPdf = new RooAddPdf("sumPdf","",pdfs,fracs);
	//pl->SetMarkerSize(0.7);
	combData->Draw();
	pl->Draw();
	combData->plotOn(pl,MarkerSize(0.7));
	RooAddPdf * sumsigPdf = new RooAddPdf("sumsigPdf","",sigpdfs,sigfracs);
	RooAddPdf * sumtotbkgPdf = new RooAddPdf("sumtotbkgPdf","",totpdfs,totfracs);

	if(option.find("-nototsigplot")==string::npos) sumsigPdf->plotOn(pl,LineStyle(kDashed),LineColor(kBlack),Normalization( sumNsig / (float)sumAll , RooAbsReal::Relative));

	Color_t colors[] = {kRed,46,kGreen,kCyan,kOrange,kMagenta,kGreen+4,kViolet+10,kGray+3};
	int styles[] = {3,4,5,6,7,8,9,10};
	int style = 0, color = 0;
	if(option.find("-plottotbkg")!=string::npos)
	{
		sumtotbkgPdf->plotOn(pl,LineStyle(styles[style++]),LineColor(colors[color++]),Normalization( sumNtotbkg / (float)sumAll , RooAbsReal::Relative));
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
			curSumBkgPdf->plotOn(pl,LineStyle(styles[style++]),LineColor(colors[color++]),Normalization( curSum / (float)sumAll , RooAbsReal::Relative));
		}
	}
	else {

		for(unsigned bb = 0; bb < bnames.size(); bb++)
		{
			string bname = bnames[bb];
			RooAddPdf * curSumBkgPdf = new RooAddPdf("sumBkgPdf_"+(TString)bname,"",bkgpdfs[bname],bkgfracs[bname]);
			curSumBkgPdf->plotOn(pl,LineStyle(styles[style++]),LineColor(colors[color++]),Normalization( sumNbkg[bname] / (float)sumAll , RooAbsReal::Relative));
		}
	}
	sumPdf->plotOn(pl);


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


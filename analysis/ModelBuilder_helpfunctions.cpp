#include "ModelBuilder_helpfunctions.hpp"

using namespace RooFit;


vector <Color_t> GetDefaultColors()
{
    vector <Color_t> default_colors;
    default_colors.push_back(kCyan+1);
    default_colors.push_back(kRed+1);
    default_colors.push_back(kGreen+1);
    default_colors.push_back(kOrange);
    default_colors.push_back(kMagenta+1);
    default_colors.push_back(kGray);
    default_colors.push_back(kGray+1);
    default_colors.push_back(kGray+2);
    default_colors.push_back(kGray+3);
    default_colors.push_back(kOrange+2);

    return default_colors;
}

// Gets the list of parameters of a RooAbsPdf in Str2VarMap form
// opt=="-cut" cuts the names and keeps the part before the underscore "(alwayskept)_(optional)"

RooRealVar * GetParam(RooAbsPdf * pdf, string name, string opt)
{
    RooArgSet * params = pdf->getParameters(RooDataSet());
    TIterator * it = params->createIterator();
    RooRealVar * arg;
    while( (arg=(RooRealVar*)it->Next()) )
    {
        string varname = (string)arg->GetName();
        if(opt == "-cut")
        {
            int _pos = ((string)arg->GetName()).find("_");
            varname = ((string)arg->GetName()).substr(0,_pos);
        }

        if( varname == name ) return arg;
    }

    return NULL;
}

Str2VarMap GetParams(RooAbsPdf * pdf, RooArgSet obs, vector < string > pnames, string opt)
{
    transform(opt.begin(), opt.end(), opt.begin(), ::tolower);

    Str2VarMap out;
    RooArgSet * params = pdf->getParameters(RooDataSet("v","",obs));
    TIterator * it = params->createIterator();
    RooRealVar * arg;
    while( (arg=(RooRealVar*)it->Next()) )
    {
        string complete_name = arg->GetName();
        int _pos = complete_name.find("_");
        string varname = complete_name.substr(0,_pos);
        vector<string>::iterator it = find (pnames.begin(), pnames.end(), varname);
        size_t posfor = complete_name.find("__for");
        if(opt.find("-orignames")!=string::npos) varname = (string)arg->GetName();
        else if(opt.find("-nameandvar")!=string::npos && posfor!=string::npos)
            varname+=("_"+complete_name.substr(posfor+5,string::npos));
        if(it != pnames.end() || pnames.empty()) out[varname] = arg;
    }

    return out;
}

Str2VarMap GetParamList(RooAbsPdf * pdf, RooAbsReal * var, string opt)
{
    return GetParamList(pdf, RooArgSet(*var), opt); 
}

Str2VarMap GetParamList(RooAbsPdf * pdf, RooArgSet obs, string opt)
{
    return GetParams( pdf, obs, vector < string >() , opt);
}


void GetParam(RooFitResult *fRes, string name, double &par, double &parE, string type) 
{
    RooArgList cPars = fRes->constPars();
    RooArgList fPars = fRes->floatParsFinal();

    TIterator *it = NULL;
    if (type == "c") it = cPars.createIterator();
    else it = fPars.createIterator();

    if(!it)
    {
        cout << "\nEmpty Pars\n" << endl;
        exit(EXIT_FAILURE);
    }

    par  = 0;
    parE = 0;

    RooRealVar *arg;
    while((arg = (RooRealVar*) it->Next())) {
        if (arg->GetName() == name) {
            par  = arg->getVal();
            parE = arg->getError();
            break;
        }
    }

    if ((par == 0) && (parE == 0)) {
        cout << "\nPar " << name << " not available\n" << endl;
        exit(EXIT_FAILURE);
    }

    return;
}

double GetParVal(RooFitResult *fRes, string name, string type) {

    double par, parE;
    GetParam(fRes, name, par, parE, type);

    return par;
}

double GetParErr(RooFitResult *fRes, string name, string type) {

    double par, parE;
    GetParam(fRes , name, par, parE, type);
    return parE;
}




//Returns the complete name of the parameter "par" is in the "myvars" object
//Return an empty string if it doesn't find it

string isParInMap( string par, Str2VarMap myvars, string option )
{
    string namepar = par.substr(0,par.find("_"));
    for(Str2VarMapItr iter = myvars.begin(); iter != myvars.end(); iter++)
    {
        size_t pos_ = iter->first.find("_");
        string namecurpar = iter->first.substr(0,pos_);
        if(namepar!=namecurpar) continue;
        if( option != "" && (iter->first).find(option)!=string::npos ) return iter->first;
        else if(option == "") return iter->first;
    }
    return (string)"";
}


//Allows to modify a parameter (or more) in a RooFormulaVar
//default       -> Scales the parameter by "c"
//opt=="-shift" -> Adds a shift by "c"

Str2VarMap ModifyPars(Str2VarMap * pars, vector<string> names, vector<RooRealVar *> c, string opt)
{
    for(unsigned i = 0; i < names.size(); i++)
    {
        string parMapName = isParInMap( names[i], *pars );
        RooRealVar * par = (RooRealVar*)((*pars)[parMapName]);
        if(!par) { cout << "Parameter " << names[i] << " not found!" << endl; continue; }
        TString fname = (TString)par->GetName()+"__"+c[i]->GetName();
        string fkey = parMapName;
        if(opt.find("-n")!=string::npos)
        {
            int posn = opt.find("-n");
            int posdash = opt.find("-",posn+2);
            fname += ("_" + opt.substr(posn+2,posdash));
        }
        RooFormulaVar * fpar;
        if(opt.find("-shift")!=string::npos) fpar = new RooFormulaVar(fname+"_shifted","@0+@1",RooArgSet(*c[i],*par));
        else fpar = new RooFormulaVar(fname+"_scaled","@0*@1",RooArgSet(*c[i],*par));
        (*pars)[fkey] = fpar;
    }
    return *pars;
}

Str2VarMap ModifyPars(Str2VarMap * pars, vector<string> names, RooRealVar * c, string opt)
{
    vector<RooRealVar *> vc(names.size(),c);
    return ModifyPars(pars,names,vc,opt);
}

Str2VarMap ModifyPars(Str2VarMap * pars, string name, RooRealVar * c, string opt)
{
    vector<string> names(1,name);
    return ModifyPars(pars,names,c,opt);
}


//Prints the parameters in a Str2VarMap object
//opt == "-nocost"  ->  doesn't print constants
//opt == "-latex"   ->  prints the Title instead of Name of variables assuming latex format

void PrintPars(Str2VarMap pars, string opt)
{
    for(Str2VarMapItr iter = pars.begin(); iter != pars.end(); iter++)
    {
        double prec = 4;
        size_t precpos = opt.find("-prec");
        if(precpos!=string::npos) prec = ((TString)opt.substr(precpos+5,string::npos)).Atof();
        if(opt.find("-nocost")!=string::npos && (bool)iter->second->getAttribute("Constant") ) continue;
        if(opt.find("-latex")!=string::npos)
        {
            cout << fixed << setprecision(prec) << "$" << iter->second->GetTitle() << "$ \t\t& $";
            cout << iter->second->getVal();
            try{
                if(TMath::Abs(((RooRealVar*)iter->second)->getErrorLo() + ((RooRealVar*)iter->second)->getErrorHi()) < TMath::Power(10,-prec))
                    cout << " \\pm " << ((RooRealVar*)iter->second)->getError();
                else cout << "^{" << ((RooRealVar*)iter->second)->getErrorHi() << "}_{" << ((RooRealVar*)iter->second)->getErrorLo() << "}";
                throw (string)"NOERROR";
            }
            catch(string err) {}
            cout << "$\t\t \\\\" << endl;
        }
        else {
            //cout << "Search name: " << iter->first << endl;
            iter->second->Print();
        }
    }	
}

//Prints all parameters of a RooAbsPdf

void printParams(RooAbsPdf * pdf, RooArgSet obs, string opt)
{
    Str2VarMap pars = GetParamList(pdf, obs, opt+"-orignames");
    PrintPars(pars, opt);
}



//Return the number of free patameters in a RooAbsPdf

int getNFreePars(RooAbsPdf * pdf, RooArgSet vars)
{
    int nfree = 0;
    RooArgSet * params = pdf->getParameters(RooDataSet("v","",vars));
    TIterator *it = params->createIterator();
    RooRealVar * arg;
    while( (arg=(RooRealVar*)it->Next()) )
        if(!arg->getAttribute("Constant") && ((string)arg->GetName())!="samples")
            nfree++;

    return nfree;
}



RooArgSet * gaussianConstraints(RooAbsPdf * pdf, RooArgSet vars)
{
    RooArgSet * res = new RooArgSet("gaussian_constraints");
    RooArgSet * params = pdf->getParameters(RooDataSet("v","",vars));
    TIterator *it = params->createIterator();
    RooRealVar * arg;
    while( (arg=(RooRealVar*)it->Next()) )
    {
        if(!arg->getAttribute("Constant") && ((string)arg->GetName())!="samples")
        {
            cout << arg->GetName() << "  " << arg->getVal() << "  " << arg->getError() << endl;
            RooGaussian * cost = new RooGaussian((TString)arg->GetName()+"_cost","gaussian constraint",*arg,RooConst(arg->getVal()),RooConst(arg->getError()));
            res->add(*cost);
        }
    }

    return res;
}

/*
RooAbsPdf * corrGaussConstraints(RooAbsPdf * pdf, RooArgSet vars)
{
    RooArgSet * params = pdf->getParameters(RooDataSet("v","",vars));
    
    return (RooAbsPdf *)(new RooMultiVarGaussian(
                name+"_multivar_gauss",name+"_multivar_gauss",
                *params,fitRes->covarianceMatrix()));
}
*/

bool checkModel(RooAbsPdf * model)
{
    cout << "====================== CHECKING MODEL ======================" << endl;

    bool model_ok = true;
    vector <string> names, pnames;

    RooArgSet * comps = model->getComponents();
    TIterator * it = comps->createIterator();
    RooAbsArg * arg;
    while( (arg=(RooAbsArg*)it->Next()) )
    {	
        TString legstyle = "l";
        string name = arg->GetName();
        if( find(names.begin(),names.end(),name) != names.end())
        {
            model_ok = false;
            cout << "ATTENTION: Component " << name << " repeated" << endl; 
        }
        names.push_back(name);
    }

    RooArgSet * params = model->getParameters(RooDataSet());
    TIterator * itp = params->createIterator();
    RooRealVar * argp;
    while( (argp=(RooRealVar*)itp->Next()) )
    {
        string pname = argp->GetName();
        if( find(pnames.begin(),pnames.end(),pname) != pnames.end())
        {
            model_ok = false;
            cout << "ATTENTION: Parameter " << pname << " repeated" << endl; 
        }
        pnames.push_back(pname);
    }

    if(model_ok)
        cout << "====================== MODEL OK!! HAVE A GOOD FIT ======================" << endl;
    else
        cout << "====================== MODEL NOT OK!! PLEASE CHECK ======================" << endl;

    return model_ok;
}



//Sets all parameters of a RooAbsPdf to the values of the
//parameters with the same name in the "set" list.
//Does nothing if the name is not found.
//fix == "fix"  -> sets also the parameters to constants
//fix == "free" -> sets also the parameters to free

void fixParam(RooAbsPdf * pdf, RooRealVar * obs, string fix)
{
    return fixParam( pdf, new RooArgSet(*obs), pdf->getParameters(RooDataSet("dataset","",*obs)), fix);
}

void fixParam(RooAbsPdf * pdf, RooRealVar * obs, RooArgSet * set, string fix)
{
    return fixParam( pdf, new RooArgSet(*obs), set, fix);
}

void fixParam(RooAbsPdf * pdf, RooArgSet * obs, RooArgSet * set, string fix)
{
    RooArgSet * params = pdf->getParameters(RooDataSet("dataset","",*obs));
    if(set->getSize()<1) set = params;
    TIterator *it = params->createIterator();
    RooRealVar * arg1, * arg2;
    while( (arg1=(RooRealVar*)it->Next()) ) 
    { 
        TIterator *setit = set->createIterator();
        if(set->getSize()==0) 
        {
            if(fix=="fix") arg1->setConstant(true);
            else if(fix=="free") arg1->setConstant(false);
            continue;
        }
        while( (arg2=(RooRealVar*)setit->Next()) ) 
        {
            if(((string)arg1->GetName())=="samples") continue;
            if((TString)arg1->GetName()==(TString)arg2->GetName())
            {
                double val = arg2->getVal();
                arg1->setVal(val);
                if(fix=="fix") arg1->setConstant(true);
                else if(fix=="free") arg1->setConstant(false);
            }
        }
        delete setit;
    }
    delete it;
}


TPaveText * createParamBox(RooAbsPdf * pdf, RooRealVar * obs, string opt, RooFitResult * fitRes)
{
    double x1 = 0.78, x2 = 1., y1 = 0.6, y2 = 1.;
    if(opt.find("-layout")!=string::npos)
    {
        size_t pos = opt.find("-layout")+8;
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
    TPaveText * box = new TPaveText(x1,y1,x2,y2,"BRNDC");
    box->SetFillColor(kWhite);
    box->SetBorderSize(1); 

    RooArgSet * params = pdf->getParameters(RooDataSet("dataset","",RooArgSet(*obs)));
    TIterator * it = params->createIterator();
    RooRealVar * arg;
    while( (arg=(RooRealVar*)it->Next()) ) 
    { 
        string oname = arg->GetName();
        string otitle = arg->GetTitle();
        if(opt.find("-nocost")!=string::npos 
                && arg->getAttribute("Constant")
                && oname.find("_print")==string::npos ) continue;
        if(opt.find("-vpar")!=string::npos 
                && oname.find(((TString)obs->GetName()).ReplaceAll("__var__",""))==string::npos
                && oname.find("__for")!=string::npos ) continue;
        if(otitle.find("_hide_")!=string::npos ) continue;
        if(otitle.find("var_")!=string::npos ) continue;

        size_t pos_  = oname.find("_");
        size_t pos__ = oname.find("__",pos_+1);
        oname = oname.substr(0,pos__);
        pos_  = oname.find("_",pos_+1);
        TString name = (TString)(oname.substr(pos_+1,string::npos));
        TString title = ((TString)arg->GetTitle()).ReplaceAll("\\","#");

        double error = arg->getError();
        if(fitRes) arg->getPropagatedError(*fitRes);
        int precision = 1;
        while( error > 1.e-6 && error < 0.1 )
        { 
            error *= 10.;
            precision++;
        }

        if(opt.find("-usenames")!=string::npos)
            box->AddText((TString)arg->GetName()+Form(" = %4.*f #pm %4.*f",precision,arg->getVal(),precision,arg->getError()));
        else box->AddText(title+Form(" = %4.*f #pm %4.*f",precision,arg->getVal(),precision,arg->getError()));
    }
    size_t pos_lab = opt.find("-label");
    if(pos_lab!=string::npos)
    {
        size_t pos_sq = opt.find("]",pos_lab);
        box->AddText((TString)opt.substr(pos_lab+7,pos_sq - (pos_lab+7)));
    }
    if(opt.find("-font")!=string::npos)
    {
        string str = opt.substr(opt.find("-font")+5,string::npos);
        box->SetTextSize(((TString)str).Atof());
    }
    else box->SetTextSize(0.03);
    box->SetTextAlign(12);
    return box;
}


//Returns a RooArgSet containing a copy of all the free parameters in a RooAbsPdf

RooArgSet * copyFreePars(RooAbsPdf * pdf, RooArgSet vars)
{
    RooArgSet * out = new RooArgSet();
    RooArgSet * params = pdf->getParameters(RooDataSet("v","",vars));
    TIterator *it = params->createIterator();
    RooRealVar * arg;
    while( (arg=(RooRealVar*)it->Next()) ) 
    {
        RooRealVar * vv = new RooRealVar(arg->GetName(),arg->GetName(),arg->getVal());
        if(!arg->getAttribute("Constant")) out->add(*vv);
    }
    return out;
}




//Set the paramteres with names in the "names" list to constats
// opt == "-except" -> set contrants all but the ones with name in the list

Str2VarMap setConstant(Str2VarMap * pars, string name, string opt)
{
    return setConstant(pars, vector<string>(1,name), opt);
}

Str2VarMap setConstant(Str2VarMap * pars, vector<string> names, string opt)
{
    for(Str2VarMapItr iter = (*pars).begin(); iter != (*pars).end(); iter++)
    {
	string cname = iter->first;
        bool found = false;
        for(unsigned i = 0; i < names.size(); i++)
	{
            if( ( names[i] == cname ) ||
		( cname.find(names[i]) != string::npos && opt.find("contains")!=string::npos ) ) 
		{ found = true; break; }
	}
        if(opt.find("except")!=string::npos) found = !found;
	bool setconst = kTRUE;
	if(opt.find("-free")!=string::npos) setconst = kFALSE;
	//cout << cname << "  " << found << "  " << setconst << endl;
	if( names.empty() || found ) ((RooRealVar*)iter->second)->setConstant(setconst);
    }
    return *pars;
}

Str2VarMap setConstant(RooAbsPdf * pdf, RooRealVar * var, vector<string> names, string opt)
{
    Str2VarMap pars = GetParamList(pdf,var,opt+"-orignames");
    return setConstant(&pars,names,opt);
}


Str2VarMap eraseParameter(Str2VarMap * parlist, vector<string> names)
{
    for(unsigned i = 0; i < names.size(); i++) (*parlist).erase(names[i].c_str());
    return *parlist;
}


TString getLegendLabel( TString title, string opt )
{
    TString leglabel = title.ReplaceAll("_print","");
    bool isbkg = leglabel.Contains("bkg_");
    leglabel = leglabel.ReplaceAll("bkg_","");
    if(isbkg) leglabel = "Bkg. " + leglabel;
    leglabel.ReplaceAll("totsig_","Sig. ");

    if(opt.find("-origlegnames")==string::npos)
    {
        size_t pos_ = ((string)leglabel).find("_");
        while(true) {
            if(((TString)leglabel)(pos_+1,1)=="{")
                pos_ = ((string)leglabel).find("_",pos_+1);
            else break;
        }
        leglabel = (TString)((string)leglabel).substr(0,pos_);
    }

    return leglabel;
}

/*
   Allows to make nice plots of data and models including blinded plots
   @param var: variable to plot
   @param data: data to plot -> If no data use NULL
   @param model: model to plot -> If no model use NULL
   @param opt: options string. Options available are:
   "-fillSig"    -> signal is filled with color instead of dashed line
   "-fillBkg"    -> bkg is filled with color instead of dashed line
   "-noParams"   -> no params box produced
   "-noCost"     -> no constant parameters shown in params box
   "-nochi2"     -> no chi2 in params box
   "-chi2ndf"    -> plots the result of chi2/ndf instead of the separate chi2 and ndf
   "-sumW2err"   -> if weighted data errors shown reflect statistics of initial sample
   "-plotSigComp"-> prints signal components and not only total signal function
   "-range"      -> plots only the fitted range, otherwise all available is plot
   "-min(num)"   -> sets the minimum to "num"
   "-max(num)"   -> sets the maximum to "num"
   @param bins: number of bins to use for data
   @param regions: number of regions to plot is blinded plot, if regions = 0 plots everything 
   @param Xtitle: X axis label
   @param title: title
   @param leg: A TLegend object to fill
 **/

RooPlot * GetFrame(RooRealVar * var, RooAbsData * data, RooAbsPdf * model, string opt, unsigned bins,
        double * range, vector<string> regStr, TString Xtitle, TString Ytitle, TLegend * leg, vector <Color_t> custom_colors)
{
    transform(opt.begin(), opt.end(), opt.begin(), ::tolower);
 	if (bins < 1) bins = 50;
    double tmp_range[] = {var->getMin(), var->getMax()};
    if(!range) range = &tmp_range[0];	
    var->setRange("PlotRange",range[0],range[1]);
    RooPlot * frame = new RooPlot(*var,range[0],range[1],bins);

    if(opt.find("-t")==string::npos) frame->SetTitle("");
    else
    {
        unsigned pos1 = opt.find("-t");
        unsigned pos2 = opt.find("-",pos1+1);
        if(pos2==string::npos) frame->SetTitle((opt.substr(pos1+2,string::npos)).c_str());
        else if((pos2-pos1)>1) frame->SetTitle((opt.substr(pos1+2,pos2-pos1-2)).c_str());
    }

    RooCmdArg drawOptSig(LineStyle(kDashed)), drawOptBkg(LineStyle(3)), moveBack(RooCmdArg::none());
    if(opt.find("-fillsig")!=string::npos) drawOptSig = DrawOption("C");
    if(opt.find("-fillbkg")!=string::npos) { drawOptBkg = DrawOption("F"); moveBack = MoveToBack(); }

    if(regStr.size()==0) regStr.push_back("PlotRange");
    bool noblind = (regStr.size()==1);
    RooCmdArg range_data(RooCmdArg::none()); 
    RooCmdArg range_model(RooCmdArg::none()); 
    RooCmdArg norm_range(RooCmdArg::none()); 
    RooCmdArg totMcolor = LineColor(4);
    RooCmdArg blindTotModel(RooCmdArg::none());
    TString dataname = "data", modelname = "model";
    double min = 1;

    RooLinkedList cmdList;

    for (unsigned i = 0; i < regStr.size(); ++i) 
    {
        string bandname = regStr[i];
        if(bandname.find("band")!=string::npos)
        {		
            totMcolor =  LineColor(4);
            blindTotModel = RooCmdArg::none();
        }
        else if(bandname.find("sig")!=string::npos) 
        {
            totMcolor =  LineColor(kWhite);
            blindTotModel = MoveToBack();
        }

        if(!noblind)
        {
            range_data = CutRange(bandname.c_str());
            range_model = Range(bandname.c_str());
        }

        if(i>0) dataname = Form("data_%i",i);
        if(i>0) modelname = Form("model_%i",i);

        //Plot data and total model


	TString category = "";
    RooCmdArg cut(RooCmdArg::none());
	if(opt.find("-category[")!=string::npos)
	{
	    size_t pos = opt.find("-category[");
            size_t posend = opt.find("]",pos);
            category = opt.substr(pos+10, posend - (pos+10) -1 );
            cout << "*************** category is" << category << endl;
            cut = Cut("samples==samples::"+category);	
	}

        if(data && ( bandname.find("band")!=string::npos || noblind ) )
        {
            frame->SetMarkerSize(1);
            if(opt.find("-sumw2err")!=string::npos) data->plotOn(frame, range_data, Name(dataname), DataError(RooAbsData::SumW2), cut);
            else data->plotOn(frame, range_data, Name(dataname), cut);

            min = 1e9;
            RooHist *hist = frame->getHist(dataname);
            Double_t *cont = hist->GetY();
            for(int i = 0; i < hist->GetN(); i++)
            {
                if(cont[i]!=0) min = TMath::Min(min, cont[i]);
            }
            if (min==1e9) min = 1;
        }

        vector <Color_t> colors;
        colors.push_back(kCyan+1);
        colors.push_back(kRed+1);
        colors.push_back(kGreen+1);
        colors.push_back(kOrange);
        colors.push_back(kMagenta+1);
        colors.push_back(kGray);
        colors.push_back(kGray+1);
        colors.push_back(kGray+2);
        colors.push_back(kGray+3);
        if(custom_colors.size() > 0) colors = custom_colors;
        int styles[] = {3,4,5,6,7,8,9,10,3,4,5,6,7,8,9,10};

        RooArgSet * stackedBkgs = NULL;
        if(opt.find("-stackbkg")!=string::npos) stackedBkgs = new RooArgSet("stackedBkgs");

        if(model && ( bandname.find("band")!=string::npos || noblind ) )
        {
            //Plot total model

            model->plotOn(frame, totMcolor, Name(modelname), blindTotModel, range_model, norm_range);

            //Plot signal and background components

            int counter = 0;
            if(opt.find("-stackbkg")==string::npos) counter++;
            RooArgSet * comps = model->getComponents();
            TIterator * it = comps->createIterator();
            RooAbsArg * arg;
            while( (arg=(RooAbsArg*)it->Next()) )
            {	
                TString legstyle = "l";
                string name = arg->GetName();
                bool isplot = false;

                if(opt.find("-printonly")==string::npos && name.find("_noprint")==string::npos)
                {
                    if(name.find("wrtsig")!=string::npos) continue;
                    if( noblind && name.find("totsig")!=string::npos && opt.find("-nototsigplot")==string::npos)
                    {
                        model->plotOn(frame, Components(*arg), drawOptSig, LineColor(1), Name(arg->GetName()), range_model, norm_range);
                        isplot = true;
                    }
                    else if( noblind && name.find("sig")!=string::npos && opt.find("-plotsigcomp")!=string::npos )
                    {
                        model->plotOn(frame, Components(*arg), drawOptSig, 
                                LineColor(colors[counter]), LineStyle(styles[counter]), 
                                Name(arg->GetName()), range_model, norm_range);
                        counter++;
                        isplot = true;
                    }
                    else if(name.find("bkg")!=string::npos && name.find("nbkg")==string::npos) 
                    {
                        int style = styles[counter];
                        if(opt.find("-fillbkg")!=string::npos)
                        {
                            legstyle = "f";
                            style = 0;
                        }
                        if(!stackedBkgs)
                        {
                            model->plotOn(frame, Components(*arg), drawOptBkg, 
                                    LineColor(colors[counter]), LineStyle(style), FillColor(colors[counter]), 
                                    Name(arg->GetName()), moveBack, range_model, norm_range);	
                            counter++;
                            isplot = true;
                        }
                        else
                        {
                            stackedBkgs->add(*arg);
                        }
                    }
                }
                if(name.find("_print")!=string::npos && !isplot)
                {
                    model->plotOn(frame, Components(*arg), drawOptSig, 
                            LineColor(colors[counter]), LineStyle(styles[counter]), FillColor(colors[counter]), 
                            Name(arg->GetName()), moveBack, range_model, norm_range);
                    counter++;
                    isplot = true;
                }

                TString leglabel = getLegendLabel(arg->GetTitle(), opt);
                if(i==0 && leg && isplot) leg->AddEntry(frame->findObject(arg->GetName()),leglabel,legstyle);
            }

            if(stackedBkgs)
            {
                int nbkgs = stackedBkgs->getSize();

                for(int bb = 0; bb < nbkgs; bb++)
                {
                    RooArgList curBkg;
                    TIterator * it = stackedBkgs->createIterator();
                    int count = 0;
                    RooAbsArg * arg;
                    TString myBkgName, myBkgTitle;
                    while((arg=(RooAbsArg*)it->Next()))
                    {
                        if( count > bb ) break;
                        count++;
                        curBkg.add(*arg);
                        myBkgName = arg->GetName();
                        myBkgTitle = arg->GetTitle();
                    }

                    if(opt.find("-fillbkg")!=string::npos)
                        model->plotOn(frame, Components(curBkg), DrawOption("F"),
                                FillColor(colors[counter]),
                                FillStyle(1001),
                                LineWidth(0.),
                                LineStyle(0),
                                LineColor(colors[counter]),
                                Name(myBkgName),
                                MoveToBack(), range_model //, norm_range
                                );
                    else
                        model->plotOn(frame, Components(curBkg), DrawOption("L"),
                                LineColor(colors[counter]),
                                LineStyle(styles[counter]),
                                Name(myBkgName),
                                MoveToBack(), range_model, norm_range
                                );

                    counter++;

                    TString leglabel = getLegendLabel(myBkgTitle,opt); 
                    TString legstyle = "l";
                    if(opt.find("-fillbkg")!=string::npos) legstyle = "f";
                    if(i==0 && leg) leg->AddEntry(frame->findObject(myBkgName),leglabel,legstyle);
                }
            }
        }
    }

    TString label = "";
    if(model && data && opt.find("-nochi2")==string::npos)
    {
        double * chi2ndf = calcChi2(frame, getNFreePars(model,RooArgSet(*var)));
        if(opt.find("-chi2ndf")!=string::npos) label = Form("Chi2/NDF =  %4.2f",chi2ndf[0]);
        else label = Form("Chi2/NDF =  %4.2f / %4.2f",chi2ndf[0]*chi2ndf[1],chi2ndf[1]);
    }
    if(model && noblind && opt.find("-noparams")==string::npos)
    {
        if(label!="") opt+=("-label["+label+"]");
        TPaveText * paramBox = createParamBox(model,var,opt);
        frame->addObject(paramBox);
    }
    
    if(opt.find("-min")!=string::npos)
    {
        size_t pos = opt.find("-min");
        if(opt.find("-minlog")!=string::npos) {
            TString strMin = opt.substr(pos+7,string::npos);
            frame->SetMinimum(strMin.Atof());
        }
        else {
            TString strMin = opt.substr(pos+4,string::npos);
            frame->SetMinimum(strMin.Atof());
        }
    }
    else if(opt.find("-log")!=string::npos) frame->SetMinimum(min * 0.1);
    else frame->SetMinimum(0.);
    if(opt.find("-max")!=string::npos)
    {
        size_t pos = opt.find("-max");
        TString strMin = opt.substr(pos+4,string::npos);
        frame->SetMaximum(strMin.Atof());
    }
    if(Xtitle!="") frame->SetXTitle(((TString)Xtitle).ReplaceAll("__var__",""));
    else frame->SetXTitle(((TString)var->GetName()).ReplaceAll("__var__",""));
    if(Ytitle!="") frame->SetYTitle(Ytitle);

    return frame;
}

RooPlot * GetFrame(RooRealVar * var, RooAbsPdf * model, RooAbsData * data, string opt, 
        unsigned bins, double * range, vector<string> regStr, TString Xtitle, TString Ytitle, 
        TLegend * leg, vector <Color_t>custom_colors)
{
    return GetFrame(var, data, model, opt, bins, range, regStr, Xtitle, Ytitle, leg, custom_colors);
}

RooPlot * GetFrame(RooRealVar * var, RooAbsPdf * model, RooAbsData * data, string opt,
        unsigned bins, TString Xtitle, TString Ytitle, TLegend * leg, vector <Color_t>custom_colors)
{
    return GetFrame(var, data, model, opt, bins, NULL, vector<string>(1,"PlotRange"), Xtitle, Ytitle, leg, custom_colors);
}

RooPlot * GetFrame(RooRealVar * var, RooAbsData * data, RooAbsPdf * model, string opt,
        unsigned bins, TString Xtitle, TString Ytitle, TLegend * leg, vector <Color_t>custom_colors)
{
    return GetFrame(var, data, model, opt, bins, NULL, vector<string>(1,"PlotRange"), Xtitle, Ytitle, leg, custom_colors);
}



/*
   This functions allows to choose a predefined PDF defining all its parameters together with it.
   N.B.: In order to add a PDF or modify one this is the only bit of code you should touch!
   */

RooRealVar * addPar(string par, string parstr, Str2VarMap stval_list, Str2VarMap myvars, string option)
{
    RooRealVar * curpar = (RooRealVar *)stval_list[par];
    size_t pos = parstr.find(par+"[");

    string dist_name = "";
    size_t posname = option.find("-n");
    if(posname!=string::npos)
        dist_name = option.substr(posname+2,option.find("-",posname+1) - posname + 2);

    string parMapName = "";
    if(myvars.size()>0) 
    {
        if( par=="a2os" ) parMapName = isParInMap( "a2", myvars, dist_name );
        else parMapName = isParInMap( par, myvars, dist_name );
        cout << par << " --> name in map "  << parMapName << "    --> parameter name ";
        if(parMapName!="") cout << myvars[parMapName]->GetName() << endl;
        else cout << " (WRONG!!)" << endl;
    }
    if(parMapName!="") curpar = (RooRealVar *)myvars[parMapName];
    else if(par=="a2os" && myvars.find("a2")!=myvars.end()) curpar = (RooRealVar *)myvars["a2"];
    else if(pos!=string::npos)
    {
        size_t endPar = parstr.find("]",pos);
        string s = parstr.substr(pos+par.length()+1,endPar-(pos+par.length()+1));
        double par_v = ((TString)s).Atof();
        size_t comma1 = s.find(',');
        if(comma1!=string::npos)
        {
            size_t comma2 = s.find(',',comma1+1);
            string smin = s.substr(comma1+1,comma2-comma1-1);
            double min = ((TString)smin).Atof();
            string smax = s.substr(comma2+1,endPar-comma2-1);
            double max = ((TString)smax).Atof();
            curpar->setRange(min,max);
        }
        curpar->setVal(par_v);
        if(par_v < curpar->getMin()) curpar->setMin(par_v/10.);
        if(par_v > curpar->getMax()) curpar->setMax(par_v*10.);
        
	    if(parstr.substr(pos-1,2).find("X")!=string::npos) curpar->setConstant(kTRUE);
    }

    return curpar;
}


TString getPrintParName(string typepdf_, TString namepdf_)
{
    namepdf_ = namepdf_.ReplaceAll("__noprint__",""); 
    size_t pos_ = ((string)namepdf_).find("_");
    TString namepar = (TString)(((string)namepdf_).substr(0,pos_));
    TString nameana = (TString)((string)namepdf_).substr(pos_+1,string::npos);
    size_t pos__ = ((string)nameana).find("__");
    nameana = (TString)((string)nameana).substr(0,pos__);
    TString pstrname = "_{"+namepar+"}^{"+nameana+"}";
    if(((string)namepdf_).find("sig")!=string::npos) pstrname = "^{"+nameana+"}";
    return pstrname;
}


Str2VarMap getPar(string typepdf_, TString namepdf_, RooRealVar * val, Str2VarMap myvars, string opt, TString title)
{
    Str2VarMap parout;
    if(typepdf_.find("Poly")!=string::npos || typepdf_.find("Cheb")!=string::npos) return parout;

    Str2VarMap stval_list;
    double sc = val->getVal()/5000.;
    namepdf_ = namepdf_.ReplaceAll("__noprint__","");
    if(title == "") title = namepdf_;
    TString pstrname = getPrintParName(typepdf_, title);
    
    stval_list["m"]     = new RooRealVar("m_"+namepdf_,  "m"+pstrname,        val->getVal(),val->getMin(),val->getMax());
    stval_list["mg"]    = new RooRealVar("mg_"+namepdf_, "m_{gauss}"+pstrname,val->getVal(),val->getVal()*0.5,val->getVal()*2.);
    stval_list["mcb"]   = new RooRealVar("mcb_"+namepdf_,"m_{cb}"+pstrname,   val->getVal(),val->getVal()*0.5,val->getVal()*2.);
    stval_list["m0"]    = new RooRealVar("m0_"+namepdf_, "m_{0}"+pstrname,    val->getVal(),val->getVal()*0.5,val->getVal()*2.);

    stval_list["s"]     = new RooRealVar("s_"+namepdf_,      "\\sigma"+pstrname,        10*sc,1*sc, 50*sc);
    stval_list["s2"]    = new RooRealVar("s2_"+namepdf_,     "\\sigma_{2}"+pstrname,    20*sc,2*sc,100*sc);
    stval_list["s3"]    = new RooRealVar("s3_"+namepdf_,     "\\sigma_{3}"+pstrname,    30*sc,3*sc,150*sc);
    stval_list["sg"]    = new RooRealVar("sg_"+namepdf_,     "\\sigma_{gauss}"+pstrname,1e2,1e-2,1e3);
    stval_list["scb"]   = new RooRealVar("scb_"+namepdf_,    "\\sigma_{cb}"+pstrname,   1e2,1e-2,1e3);
    stval_list["sconv"] = new RooRealVar("conv_sg_"+namepdf_,"\\sigma_{res}"+pstrname,  1,1e-2,1e2);

    stval_list["a"]     = new RooRealVar("a_"+namepdf_,  "\\alpha"+pstrname,     1, 1e-3, 1e2);
    stval_list["a2"]    = new RooRealVar("a2_"+namepdf_, "\\alpha_{2}"+pstrname, 1, 1e-3, 1e2);
    stval_list["a2os"]  = new RooRealVar("a2_"+namepdf_, "\\alpha_{2}"+pstrname,-1,-1e2, -1e-3);
    stval_list["a3"]    = new RooRealVar("a3_"+namepdf_, "\\alpha_{3}"+pstrname,-1,-1e2, -1e-3);
    stval_list["acb"]   = new RooRealVar("acb_"+namepdf_,"\\alpha_{cb}"+pstrname,1, 1e-3, 1e2);

    stval_list["n"]    = new RooRealVar("n_"+namepdf_,  "n"+pstrname,     2,0.2, 1e3);
    stval_list["n2"]   = new RooRealVar("n2_"+namepdf_, "n_{2}"+pstrname, 2,0.02,1e3);
    stval_list["n3"]   = new RooRealVar("n3_"+namepdf_, "n_{3}"+pstrname, 2,0.02,1e3);
    stval_list["ncb"]  = new RooRealVar("ncb_"+namepdf_,"n_{cb}"+pstrname,2,0.2, 1e3);

    stval_list["b"]    = new RooRealVar("b_"+namepdf_,"b"+pstrname,      -5e-4,-1.,0.);
    stval_list["c"]    = new RooRealVar("c_"+namepdf_,"c"+pstrname,      -1,-1e2,0.);
    stval_list["g"]    = new RooRealVar("g_"+namepdf_,"\\gamma"+pstrname,30,5.,1e2);
    stval_list["p"]    = new RooRealVar("p_"+namepdf_,"p"+pstrname,      1,0.,1e2);

    stval_list["f"]    = new RooRealVar("f_"+namepdf_,  "f"+pstrname,        0.6,0.,1.);
    stval_list["f2"]   = new RooRealVar("f2_"+namepdf_, "f_{2}"+pstrname,    0.3,0.,1.);
    stval_list["fg"]   = new RooRealVar("fg_"+namepdf_, "f_{gauss}"+pstrname,0.5,0.,1.);
    stval_list["fcb"]  = new RooRealVar("fcb_"+namepdf_,"f_{cb}"+pstrname,   0.5,0.,1.);

    stval_list["l"]    = new RooRealVar("l_"+namepdf_,"l"+pstrname, -5,-10.,-1.);
    stval_list["z"]    = new RooRealVar("z_"+namepdf_,"z"+pstrname, 0.005,0.,0.01);

    stval_list["nu"]   = new RooRealVar("nu_"+namepdf_,"\\nu"+pstrname, 0.,-100.,100.);
    stval_list["tau"]  = new RooRealVar("tau_"+namepdf_,"\\tau"+pstrname, 1.,0.,1000.);

    std::map <string, vector<string>> par_list;
    vector<string> ApolloniosPar    {"m", "s", "b", "a", "n"};
    vector<string> ArgusPar         {"m0", "p", "c"};
    vector<string> BreitWignerPar   {"m", "s", "g"};
    vector<string> CBPar            {"m", "s", "a", "n"};
    vector<string> CBGaussPar       {"m", "s", "a", "n", "sg", "fg"};
    vector<string> DCBPar           {"m", "s", "s2", "f", "a", "a2",   "n", "n2"};
    vector<string> DCBPar_Sn        {"m", "s", "s2", "f", "a", "a2",   "n"};
    vector<string> DCBPar_OST       {"m", "s", "s2", "f", "a", "a2os", "n", "n2"};
    vector<string> DCBGaussPar      {"m", "s", "s2", "s3", "f", "f2", "a", "a2os", "n", "n2"};
    vector<string> TCBPar           {"m", "s", "s2", "s3", "f", "f2", "a", "a2", "a3", "n", "n2", "n3"};
    vector<string> ExpPar           {"b"};
    vector<string> ExpAGaussPar     {"m", "s", "b"};
    vector<string> ExpCGaussPar     {"s", "b"};
    vector<string> GammaPar         {"g", "b", "m"};
    vector<string> GausPar          {"m", "s"};
    vector<string> DGausPar         {"m", "s", "s2", "f"};
    vector<string> TGausPar         {"m", "s", "s2", "s3", "f", "f2"};
    vector<string> IpatiaPar        {"m", "s", "b", "l", "z", "a", "n"};
    vector<string> Ipatia2Par       {"m", "s", "b", "l", "z", "a", "n", "a2", "n2"};
    vector<string> VoigtPar         {"m", "s", "g"};
    vector<string> JohnsonPar       {"m", "s", "nu", "tau"};

    par_list["Apollonios"]  = ApolloniosPar;
    par_list["Argus"]       = ArgusPar;
    par_list["BreitWigner"] = BreitWignerPar;
    par_list["CB"]          = CBPar;
    par_list["CBGauss"]     = CBGaussPar;
    par_list["DCB"]         = DCBPar;
    par_list["DCB_Sn"]      = DCBPar_Sn;
    par_list["DCB_OST"]     = DCBPar_OST;
    par_list["DCBGauss"]    = DCBGaussPar;
    par_list["TCB"]         = TCBPar;
    par_list["Exp"]         = ExpPar;
    par_list["ExpAGauss"]   = ExpAGaussPar;
    par_list["ExpCGauss"]   = ExpCGaussPar;
    par_list["Gamma"]       = GammaPar;
    par_list["Gauss"]       = GausPar;
    par_list["DGauss"]      = DGausPar;
    par_list["TGauss"]      = TGausPar;
    par_list["Ipatia"]      = IpatiaPar;
    par_list["Ipatia2"]     = Ipatia2Par;
    par_list["Voigt"]       = VoigtPar;
    par_list["Johnson"]     = JohnsonPar;

    size_t plusgaus = typepdf_.find("AndGauss");
    size_t pluscb   = typepdf_.find("AndCB");
    size_t convgaus = typepdf_.find("ConvGauss");
    if(plusgaus!=string::npos) typepdf_ = typepdf_.replace(plusgaus,8,"");
    if(pluscb!=string::npos)   typepdf_ = typepdf_.replace(pluscb,5,"");
    if(convgaus!=string::npos) typepdf_ = typepdf_.replace(convgaus,9,"");
    string parstr = "";
    size_t endtype = typepdf_.find("-");
    if(endtype!=string::npos) parstr = typepdf_.substr(endtype,string::npos);
    vector<string> pars = par_list[typepdf_.substr(0,endtype)];

    for( auto par : pars )
    {
        if(par == "a2os") parout["a2"] = addPar(par, parstr, stval_list, myvars, opt);
        else parout[par] = addPar(par, parstr, stval_list, myvars, opt);
    }

    if(plusgaus!=string::npos)
    {
        parout["mg"] = addPar("mg", parstr, stval_list, myvars, opt);
        parout["sg"] = addPar("sg", parstr, stval_list, myvars, opt);
        parout["fg"] = addPar("fg", parstr, stval_list, myvars, opt);
    }

    if(pluscb!=string::npos)
    {
        parout["mcb"] = addPar("mcb", parstr, stval_list, myvars, opt);
        parout["scb"] = addPar("scb", parstr, stval_list, myvars, opt);
        parout["acb"] = addPar("acb", parstr, stval_list, myvars, opt);
        parout["ncb"] = addPar("ncb", parstr, stval_list, myvars, opt);
        parout["fcb"] = addPar("fcb", parstr, stval_list, myvars, opt);
    }

    if(convgaus!=string::npos) parout["sconv"] = addPar("sconv", parstr, stval_list, myvars, opt);

    return parout;
}



RooAbsPdf * stringToPdf(const char * typepdf, const char * namepdf, RooRealVar * var, Str2VarMap myvars, string opt, TString title)
{
    RooAbsPdf * pdf = NULL;
    string typepdf_ = (string)typepdf;
    TString namepdf_ = ((TString)namepdf).ReplaceAll("bkg_","");
    namepdf_ = namepdf_.ReplaceAll("_print","");
    namepdf_ = namepdf_.ReplaceAll("__noprint__","");
    if(title == "") title = namepdf_;

    opt += "-n"+(string)var->GetName();

    Str2VarMap p = getPar(typepdf_,namepdf_,var,myvars,opt,title);

    if(typepdf_.substr(0,5).find("Gauss")!=string::npos)
    {
        pdf = new RooGaussian(namepdf,title,*var,*p["m"],*p["s"]);
    }
    else if(typepdf_.find("DGauss")!=string::npos)
    {
        RooGaussian * gauss1 = new RooGaussian("gauss1_"+namepdf_,"Gauss",*var,*p["m"],*p["s"]);
        RooGaussian * gauss2 = new RooGaussian("gauss2_"+namepdf_,"Gauss",*var,*p["m"],*p["s2"]);

        pdf = new RooAddPdf(namepdf,namepdf,RooArgList(*gauss1,*gauss2),*p["f"]);
    }
    else if(typepdf_.find("TGauss")!=string::npos)
    {
        RooGaussian * gauss1 = new RooGaussian("gauss1_"+namepdf_,"Gauss",*var,*p["m"],*p["s"]);
        RooGaussian * gauss2 = new RooGaussian("gauss2_"+namepdf_,"Gauss",*var,*p["m"],*p["s2"]);
        RooGaussian * gauss3 = new RooGaussian("gauss3_"+namepdf_,"Gauss",*var,*p["m"],*p["s3"]);

        pdf = new RooAddPdf(namepdf,namepdf,RooArgList(*gauss1,*gauss2,*gauss3),RooArgList(*p["f"],*p["f2"]));
    }
    else if(typepdf_.find("DCBGauss")!=string::npos)
    {
        RooCBShape * CB1 = new RooCBShape("CB1_"+namepdf_,"CB",*var,*p["m"],*p["s"], *p["a"], *p["n"]);
        RooCBShape * CB2 = new RooCBShape("CB2_"+namepdf_,"CB",*var,*p["m"],*p["s2"],*p["a2"],*p["n2"]);

        RooGaussian * gauss1 = new RooGaussian("gauss1_"+namepdf_,"Gauss",*var,*p["m"],*p["s3"]);

        pdf = new RooAddPdf(namepdf,namepdf,RooArgList(*CB1,*CB2,*gauss1),RooArgList(*p["f"],*p["f2"]));
    }
    else if(typepdf_.find("DCB")!=string::npos)
    {
        RooCBShape * CB1 = new RooCBShape("CB1_"+namepdf_,"CB",*var,*p["m"],*p["s"],*p["a"],*p["n"]);

        RooCBShape * CB2;
        if(typepdf_.find("_Sn")!=string::npos)
	     CB2 = new RooCBShape("CB2_"+namepdf_,"CB",*var,*p["m"],*p["s2"],*p["a2"],*p["n"]);
        else CB2 = new RooCBShape("CB2_"+namepdf_,"CB",*var,*p["m"],*p["s2"],*p["a2"],*p["n2"]);

        pdf = new RooAddPdf(namepdf,namepdf,RooArgList(*CB1,*CB2),*p["f"]);
    }
    else if(typepdf_.find("TCB")!=string::npos)
    {
        RooCBShape * CB1 = new RooCBShape("CB1_"+namepdf_,"CB",*var,*p["m"],*p["s"], *p["a"], *p["n"]);
        RooCBShape * CB2 = new RooCBShape("CB2_"+namepdf_,"CB",*var,*p["m"],*p["s2"],*p["a2"],*p["n2"]);
        RooCBShape * CB3 = new RooCBShape("CB3_"+namepdf_,"CB",*var,*p["m"],*p["s3"],*p["a3"],*p["n3"]);

        pdf = new RooAddPdf(namepdf,namepdf,RooArgList(*CB1,*CB2,*CB3),RooArgList(*p["f"],*p["f2"]));
    }
    else if(typepdf_.substr(0,2).find("CB")!=string::npos)
    {
        RooAbsPdf *CB = new RooCBShape(namepdf,namepdf,*var,*p["m"],*p["s"],*p["a"],*p["n"]);
        if(typepdf_.find("CBGauss")!=string::npos)
        {
            RooGaussian * gaussCB = new RooGaussian("gauss_"+namepdf_,"Gauss",*var,*p["m"],*p["sg"]);
            pdf = new RooAddPdf("CBGauss_"+namepdf_,"CBGauss",RooArgSet(*CB,*gaussCB),*p["fg"]);
        }
        else pdf = CB;
    }
    else if(typepdf_.find("Voigt")!=string::npos)
    {
        pdf = new RooVoigtian(namepdf,namepdf,*var,*p["m"],*p["s"],*p["g"]);
    }
    else if(typepdf_.find("BreitWigner")!=string::npos)
    {
        pdf = new RooBreitWigner(namepdf,namepdf,*var,*p["m"],*p["g"]);
    }
    else if(typepdf_.find("Poly")!=string::npos || typepdf_.find("Cheb")!=string::npos)
    {
        TString str_npar = (TString)(typepdf_.substr(4,string::npos));
        int npar = str_npar.Atof();

        vector < double > pvals;
        vector < double > mins;
        vector < double > maxs;

        for(int vv = 0; vv < npar; vv++)
        {
            double pval = 0, min = -1, max = 1;
            size_t posval = typepdf_.find(Form("-v%i[",vv+1));
            if(posval!=string::npos)
            {
                size_t endPar = typepdf_.find("]",posval);
                string s = typepdf_.substr(posval+3,string::npos);
                pval = ((TString)s).Atof();
                size_t comma1 = s.find(',');
                if(comma1!=string::npos)
                {
                    size_t comma2 = s.find(',',comma1+1);
                    string smin = s.substr(comma1+1,comma2-comma1-1);
                    min = ((TString)smin).Atof();
                    string smax = s.substr(comma2+1,endPar-comma2-1);
                    max = ((TString)smax).Atof();
                }
            }

            pvals.push_back(pval);
            mins.push_back(min);
            maxs.push_back(max);
        }
        RooArgList * parList = new RooArgList("parList");
        TString pstrname = getPrintParName(typepdf_, title);
        for(int i = 0; i < npar; i++)
        {
            RooRealVar * v = new RooRealVar(Form("c%i_",i)+namepdf_,Form("c_%i"+pstrname,i),pvals[i],mins[i],maxs[i]);
            parList->add(*v);
        }

        if(typepdf_.find("Poly")!=string::npos) pdf = new RooPolynomial(namepdf,namepdf, *var, *parList);
        else pdf = new RooChebychev(namepdf,namepdf, *var, *parList);
    }
//    else if(typepdf_.find("ExpAGauss")!=string::npos)
//    {
//        pdf = new RooExpAndGauss(namepdf,namepdf,*var,*p["m"],*p["s"],*p["b"]);
//    }
    else if(typepdf_.find("ExpCGauss")!=string::npos)
    {
        pdf = new RooGExpModel(namepdf,namepdf,*var,*p["s"],*p["b"]);
    }
    else if(typepdf_.find("Exp")!=string::npos)
    {
        pdf = new RooExponential(namepdf,namepdf,*var,*p["b"]);
    }
    else if(typepdf_.find("Argus")!=string::npos)
    {
        pdf = new RooArgusBG(namepdf,namepdf,*var,*p["m0"],*p["c"],*p["p"]);
    }
    else if(typepdf_.find("Gamma")!=string::npos)
    {
        pdf = new RooGamma(namepdf,namepdf,*var,*p["g"],*p["b"],*p["m"]);
    }
    else if(typepdf_.find("apollonios")!=string::npos)
    {
        pdf = new RooApollonios(namepdf,namepdf,*var,*p["m"],*p["s"],*p["b"],*p["a"],*p["n"]);
    }
    else if(typepdf_.find("Johnson")!=string::npos)
    {
        pdf = new RooJohnson(namepdf,namepdf,*var,*p["m"],*p["s"],*p["nu"],*p["tau"]);
    } 
    else if(typepdf_.find("Ipatia2")!=string::npos)
    {
        pdf = new RooIpatia2(namepdf,namepdf,*var,*p["l"],*p["z"],*p["b"],*p["s"],*p["m"],*p["a"],*p["n"],*p["a2"],*p["n2"]);
    }
    else if(typepdf_.find("Ipatia")!=string::npos)
    {
        pdf = new RooIpatia(namepdf,namepdf,*var,*p["l"],*p["z"],*p["b"],*p["s"],*p["m"],*p["a"],*p["n"]);
    }

    if(typepdf_.find("AndGauss")!=string::npos)
    {
        pdf->SetName(((TString)pdf->GetName()).ReplaceAll("_print",""));
        RooGaussian * gauss = new RooGaussian("gauss_"+namepdf_,"Gauss",*var,*p["mg"],*p["sg"]);
        pdf = new RooAddPdf((TString)namepdf+"_plus_Gauss",(TString)namepdf+"_plus_Gauss",RooArgList(*pdf,*gauss),*p["fg"]);
    }

    if(typepdf_.find("AndCB")!=string::npos)
    {
        pdf->SetName(((TString)pdf->GetName()).ReplaceAll("_print",""));
        RooCBShape * cb = new RooCBShape("extraCB_"+namepdf_,"CB",*var,*p["mcb"],*p["scb"],*p["acb"],*p["ncb"]);
        pdf = new RooAddPdf((TString)namepdf+"_plus_CB",(TString)namepdf+"_plus_CB",RooArgList(*pdf,*cb),*p["fcb"]);
    }

    if(typepdf_.find("ConvGauss")!=string::npos)
    {
        RooRealVar * mg = new RooRealVar("conv_mg_"+namepdf_,"Mean of resolution",0.);
        RooGaussian * resolution_gauss = new RooGaussian("convgauss_"+namepdf_,"",*var,*mg,*p["sconv"]);
        RooNumConvPdf * respdf = new RooNumConvPdf(namepdf,namepdf,*var,*pdf,*resolution_gauss);
        respdf->setConvolutionWindow(*mg,*p["sconv"],3);
        return (RooAbsPdf *)respdf;
    }
    else return pdf;
}

RooAbsPdf * stringToPdf(const char * typepdf1, const char * typepdf2, const char * namepdf, RooRealVar * var1, RooRealVar * var2, Str2VarMap myvars, string opt, TString title)
{
    RooAbsPdf * pdf1 = stringToPdf(typepdf1, ((string)namepdf + "_" + var1->GetName()).c_str(), var1, myvars, opt, title);
    RooAbsPdf * pdf2 = stringToPdf(typepdf2, ((string)namepdf + "_" + var2->GetName()).c_str(), var2, myvars, opt, title); 
    RooAbsPdf * totpdf = new RooProdPdf(namepdf,namepdf,*pdf1,*pdf2);
    return totpdf;
}

RooAbsPdf * stringToPdf(RooAbsPdf * pdf1, const char * typepdf, const char * namepdf, RooRealVar * var, Str2VarMap myvars, string opt, TString title)
{
    RooAbsPdf * pdf2 = stringToPdf(typepdf, ((string)namepdf + "_" + var->GetName()).c_str(), var, myvars, opt, title); 
    RooAbsPdf * totpdf = new RooProdPdf(namepdf,namepdf,*pdf1,*pdf2);
    return totpdf;
}

RooAbsPdf * get2DRooKeys(string name, TTree * tree, RooRealVar * v1, RooRealVar * v2, string opt)
{
    RooDataSet * data = new RooDataSet("DataSet","",tree,RooArgSet(*v1,*v2));
    if(opt.find("-rho")!=string::npos) 
    {
        int pos = opt.find("-rho");
        string rhostr = opt.substr(pos+4,20);
        double rho = ((TString)rhostr).Atof();
        return (RooAbsPdf *)(new RooNDKeysPdf((TString)name,(TString)name,RooArgSet(*v1,*v2),*data,"am",rho));
    }
    return (RooAbsPdf *)(new RooNDKeysPdf((TString)name,(TString)name,RooArgSet(*v1,*v2),*data,"am",2));
}

RooAbsPdf * get2DRooKeys(TTree * tree, RooRealVar * v1, RooRealVar * v2, string opt)
{
    return get2DRooKeys("keys2D", tree, v1, v2, opt);
}


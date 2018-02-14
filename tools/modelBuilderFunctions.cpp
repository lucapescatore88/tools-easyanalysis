#include "modelBuilderFunctions.hpp"


vector <Color_t> GetDefaultColors()
{
    vector <Color_t> default_colors;
    default_colors.push_back(kCyan + 1);
    default_colors.push_back(kRed + 1);
    default_colors.push_back(kGreen + 1);
    default_colors.push_back(kOrange);
    default_colors.push_back(kMagenta + 1);
    default_colors.push_back(kGray);
    default_colors.push_back(kGray + 1);
    default_colors.push_back(kGray + 2);
    default_colors.push_back(kGray + 3);
    default_colors.push_back(kOrange + 2);

    return default_colors;
}

// Gets the list of parameters of a RooAbsPdf in Str2VarMap form
// opt=="-cut" cuts the names and keeps the part before the underscore "(alwayskept)_(optional)"

RooRealVar * getParam(RooAbsPdf * pdf, string name, string opt)
{
    RooArgSet * params = pdf->getParameters(RooDataSet());
    TIterator * it = params->createIterator();
    RooRealVar * arg;
    while ( (arg = (RooRealVar*)it->Next()) )
    {
        string varname = (string)arg->GetName();
        if (opt == "-cut")
        {
            int _pos = ((string)arg->GetName()).find("_");
            varname = ((string)arg->GetName()).substr(0, _pos);
        }

        if ( varname == name ) return arg;
    }

    return NULL;
}

Str2VarMap getParams(RooAbsPdf * pdf, RooArgSet obs, vector < string > pnames, string opt)
{
    transform(opt.begin(), opt.end(), opt.begin(), ::tolower);

    Str2VarMap out;
    RooArgSet * params = pdf->getParameters(RooDataSet("v", "", obs));
    TIterator * it = params->createIterator();
    RooRealVar * arg;
    while ( (arg = (RooRealVar*)it->Next()) )
    {
        string complete_name = arg->GetName();
        int _pos = complete_name.find("_");
        string varname = complete_name.substr(0, _pos);
        vector<string>::iterator it = find (pnames.begin(), pnames.end(), varname);
        size_t posfor = complete_name.find("__for");
        if (opt.find("-orignames") != string::npos) varname = (string)arg->GetName();
        else if (opt.find("-nameandvar") != string::npos && posfor != string::npos)
            varname += ("_" + complete_name.substr(posfor + 5, string::npos));
        if (it != pnames.end() || pnames.empty()) out[varname] = arg;
    }

    return out;
}

Str2VarMap getParamList(RooAbsPdf * pdf, RooAbsReal * var, string opt)
{
    return getParamList(pdf, RooArgSet(*var), opt);
}

Str2VarMap getParamList(RooAbsPdf * pdf, RooArgSet obs, string opt)
{
    return getParams(pdf, obs, vector < string >() , opt);
}

Str2VarMap getParamList(RooAbsPdf * pdf, vector<RooRealVar *> vars, string opt)
{
    RooArgSet obs("Observables");
    for (auto v : vars) obs.add(*v);
    return getParams(pdf, obs, vector < string >() , opt);
}

void getParam(RooFitResult *fRes, string name, double &par, double &parE, string type)
{
    if (fRes)
    {
        RooArgList cPars = fRes->constPars();
        RooArgList fPars = fRes->floatParsFinal();

        TIterator *it = NULL;
        if (type == "c") it = cPars.createIterator();
        else it = fPars.createIterator();

        if (!it)
        {
            cout << "\nEmpty Pars\n" << endl;
            exit(EXIT_FAILURE);
        }

        par  = 0;
        parE = 0;

        RooRealVar *arg;
        while ((arg = (RooRealVar*) it->Next())) {
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
    }

    return;
}

double getParVal(RooFitResult *fRes, string name, string type) {

    double par, parE;
    getParam(fRes, name, par, parE, type);

    return par;
}

double getParErr(RooFitResult *fRes, string name, string type) {

    double par, parE;
    getParam(fRes , name, par, parE, type);
    return parE;
}




//Returns the complete name of the parameter "par" is in the "myvars" object
//Return an empty string if it doesn't find it

string isParInMap( string par, Str2VarMap myvars, string option )
{
    string namepar = par.substr(0, par.find("_"));
    for (Str2VarMapItr iter = myvars.begin(); iter != myvars.end(); iter++)
    {
        size_t pos_ = iter->first.find("_");
        string namecurpar = iter->first.substr(0, pos_);
        if (namepar != namecurpar) continue;
        if ( option != "" && (iter->first).find(option) != string::npos ) return iter->first;
        else if (option == "") return iter->first;
    }
    return (string)"";
}


//Allows to modify a parameter (or more) in a RooFormulaVar
//default       -> Scales the parameter by "c"
//opt=="-shift" -> Adds a shift by "c"

Str2VarMap ModifyPars(Str2VarMap * pars, vector<string> names, vector<RooRealVar *> c, string opt)
{
    for (unsigned i = 0; i < names.size(); i++)
    {
        string parMapName = isParInMap( names[i], *pars );
        RooRealVar * par = (RooRealVar*)((*pars)[parMapName]);
        if (!par) { cout << "Parameter " << names[i] << " not found!" << endl; continue; }
        TString fname = (TString)par->GetName() + "__" + c[i]->GetName();
        string fkey = parMapName;
        if (opt.find("-n") != string::npos)
        {
            int posn = opt.find("-n");
            int posdash = opt.find("-", posn + 2);
            fname += ("_" + opt.substr(posn + 2, posdash));
        }
        RooFormulaVar * fpar;
        if (opt.find("-shift") != string::npos) fpar = new RooFormulaVar(fname + "_shifted", "@0+@1", RooArgSet(*c[i], *par));
        else fpar = new RooFormulaVar(fname + "_scaled", "@0*@1", RooArgSet(*c[i], *par));
        (*pars)[fkey] = fpar;
    }
    return *pars;
}

Str2VarMap ModifyPars(Str2VarMap * pars, vector<string> names, RooRealVar * c, string opt)
{
    vector<RooRealVar *> vc(names.size(), c);
    return ModifyPars(pars, names, vc, opt);
}

Str2VarMap ModifyPars(Str2VarMap * pars, string name, RooRealVar * c, string opt)
{
    vector<string> names(1, name);
    return ModifyPars(pars, names, c, opt);
}


//Prints the parameters in a Str2VarMap object
//opt == "-nocost"  ->  doesn't print constants
//opt == "-latex"   ->  prints the Title instead of Name of variables assuming latex format

void PrintPars(Str2VarMap pars, string opt)
{
    for (Str2VarMapItr iter = pars.begin(); iter != pars.end(); iter++)
    {
        double prec = 4;
        size_t precpos = opt.find("-prec");
        if (precpos != string::npos) prec = ((TString)opt.substr(precpos + 5, string::npos)).Atof();
        if (opt.find("-nocost") != string::npos && (bool)iter->second->getAttribute("Constant") ) continue;
        if (opt.find("-latex") != string::npos)
        {
            cout << fixed << setprecision(prec) << "$" << iter->second->GetTitle() << "$ \t\t& $";
            cout << iter->second->getVal();
            try {
                if (TMath::Abs(((RooRealVar*)iter->second)->getErrorLo() + ((RooRealVar*)iter->second)->getErrorHi()) < TMath::Power(10, -prec))
                    cout << " \\pm " << ((RooRealVar*)iter->second)->getError();
                else cout << "^{" << ((RooRealVar*)iter->second)->getErrorHi() << "}_{" << ((RooRealVar*)iter->second)->getErrorLo() << "}";
                throw (string)"NOERROR";
            }
            catch (string err) {}
            cout << "$\t\t \\\\" << endl;
        }
        else { iter->second->Print(); }
    }
}

//Prints all parameters of a RooAbsPdf

void printParams(RooAbsPdf * pdf, RooArgSet obs, string opt)
{
    Str2VarMap pars = getParamList(pdf, obs, opt + "-orignames");
    PrintPars(pars, opt);
}



//Return the number of free patameters in a RooAbsPdf

int getNFreePars(RooAbsPdf * pdf, RooArgSet vars)
{
    int nfree = 0;
    RooArgSet * params = pdf->getParameters(RooDataSet("v", "", vars));
    TIterator *it = params->createIterator();
    RooRealVar * arg;
    while ( (arg = (RooRealVar*)it->Next()) )
        if (!arg->getAttribute("Constant") && ((string)arg->GetName()) != "samples")
            nfree++;

    return nfree;
}



RooArgSet * gaussianConstraints(RooAbsPdf * pdf, RooArgSet vars)
{
    RooArgSet * res = new RooArgSet("gaussian_constraints");
    RooArgSet * params = pdf->getParameters(RooDataSet("v", "", vars));
    TIterator *it = params->createIterator();
    RooRealVar * arg;
    while ( (arg = (RooRealVar*)it->Next()) )
    {
        if (!arg->getAttribute("Constant") && ((string)arg->GetName()) != "samples")
        {
            cout << arg->GetName() << "  " << arg->getVal() << "  " << arg->getError() << endl;
            RooGaussian * cost = new RooGaussian((TString)arg->GetName() + "_cost", "gaussian constraint", *arg, RooConst(arg->getVal()), RooConst(arg->getError()));
            res->add(*cost);
        }
    }

    return res;
}

bool checkModel(RooAbsPdf * model)
{
    cout << "====================== CHECKING MODEL ======================" << endl;

    bool model_ok = true;
    vector <string> names, pnames;

    RooArgSet * comps = model->getComponents();
    TIterator * it = comps->createIterator();
    RooAbsArg * arg;
    while ( (arg = (RooAbsArg*)it->Next()) )
    {
        TString legstyle = "l";
        string name = arg->GetName();
        if ( find(names.begin(), names.end(), name) != names.end())
        {
            model_ok = false;
            cout << "ATTENTION: Component " << name << " repeated" << endl;
        }
        names.push_back(name);
    }

    RooArgSet * params = model->getParameters(RooDataSet());
    TIterator * itp = params->createIterator();
    RooRealVar * argp;
    while ( (argp = (RooRealVar*)itp->Next()) )
    {
        string pname = argp->GetName();
        if ( find(pnames.begin(), pnames.end(), pname) != pnames.end())
        {
            model_ok = false;
            cout << "ATTENTION: Parameter " << pname << " repeated" << endl;
        }
        pnames.push_back(pname);
    }

    if (model_ok)
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
    return fixParam( pdf, new RooArgSet(*obs), pdf->getParameters(RooDataSet("dataset", "", *obs)), fix);
}

void fixParam(RooAbsPdf * pdf, RooRealVar * obs, RooArgSet * set, string fix)
{
    return fixParam( pdf, new RooArgSet(*obs), set, fix);
}

void fixParam(RooAbsPdf * pdf, RooArgSet * obs, RooArgSet * set, string fix)
{
    RooArgSet * params = pdf->getParameters(RooDataSet("dataset", "", *obs));
    if (set->getSize() < 1) set = params;
    TIterator *it = params->createIterator();
    RooRealVar * arg1, * arg2;
    while ( (arg1 = (RooRealVar*)it->Next()) )
    {
        TIterator *setit = set->createIterator();
        if (set->getSize() == 0)
        {
            if (fix == "fix") arg1->setConstant(true);
            else if (fix == "free") arg1->setConstant(false);
            continue;
        }
        while ( (arg2 = (RooRealVar*)setit->Next()) )
        {
            if (((string)arg1->GetName()) == "samples") continue;
            if ((TString)arg1->GetName() == (TString)arg2->GetName())
            {
                double val = arg2->getVal();
                arg1->setVal(val);
                if (fix == "fix") arg1->setConstant(true);
                else if (fix == "free") arg1->setConstant(false);
            }
        }
        delete setit;
    }
    delete it;
}


TPaveText * createParamBox(RooAbsPdf * pdf, RooRealVar * obs, string opt, RooFitResult * fitRes)
{
    double x1 = 0.78, x2 = 1., y1 = 0.6, y2 = 1.;
    if (opt.find("-layout") != string::npos)
    {
        size_t pos = opt.find("-layout") + 8;
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
    TPaveText * box = new TPaveText(x1, y1, x2, y2, "BRNDC");
    box->SetFillStyle(1001);
    box->SetFillColor(kWhite);
    box->SetBorderSize(1);

    RooArgSet * params = pdf->getParameters(RooDataSet("dataset", "", RooArgSet(*obs)));
    TIterator * it = params->createIterator();
    RooRealVar * arg;
    while ( (arg = (RooRealVar*)it->Next()) )
    {
        string oname = arg->GetName();
        string otitle = arg->GetTitle();
        if (opt.find("-nocost") != string::npos
                && arg->getAttribute("Constant")
                && oname.find("_print") == string::npos ) continue;
        if (opt.find("-vpar") != string::npos
                && oname.find(((TString)obs->GetName()).ReplaceAll("__var__", "")) == string::npos
                && oname.find("__for") != string::npos ) continue;
        if (otitle.find("_hide_") != string::npos ) continue;
        if (otitle.find("var_") != string::npos ) continue;

        size_t pos_  = oname.find("_");
        size_t pos__ = oname.find("__", pos_ + 1);
        oname = oname.substr(0, pos__);
        pos_  = oname.find("_", pos_ + 1);
        TString name = (TString)(oname.substr(pos_ + 1, string::npos));
        TString title = ((TString)arg->GetTitle()).ReplaceAll("\\", "#");

        double error = arg->getError();
        if (fitRes) arg->getPropagatedError(*fitRes);
        int precision = 1;
        while ( error > 1.e-6 && error < 0.1 )
        {
            error *= 10.;
            precision++;
        }

        if (opt.find("-usenames") != string::npos)
            box->AddText((TString)arg->GetName() + Form(" = %4.*f #pm %4.*f", precision, arg->getVal(), precision, arg->getError()));
        else box->AddText(title + Form(" = %4.*f #pm %4.*f", precision, arg->getVal(), precision, arg->getError()));
    }
    size_t pos_lab = opt.find("-label");
    if (pos_lab != string::npos)
    {
        size_t pos_sq = opt.find("]", pos_lab);
        box->AddText((TString)opt.substr(pos_lab + 7, pos_sq - (pos_lab + 7)));
    }
    if (opt.find("-font") != string::npos)
    {
        string str = opt.substr(opt.find("-font") + 5, string::npos);
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
    RooArgSet * params = pdf->getParameters(RooDataSet("v", "", vars));
    TIterator * it = params->createIterator();
    RooRealVar * arg;
    while ( (arg = (RooRealVar*)it->Next()) )
    {
        RooRealVar * vv = new RooRealVar(arg->GetName(), arg->GetName(), arg->getVal());
        if (!arg->getAttribute("Constant")) out->add(*vv);
    }
    return out;
}




//Set the paramteres with names in the "names" list to constats
// opt == "-except" -> set contrants all but the ones with name in the list

Str2VarMap setConstant(Str2VarMap * pars, string name, string opt)
{
    return setConstant(pars, vector<string>(1, name), opt);
}

Str2VarMap setConstant(Str2VarMap * pars, vector<string> names, string opt)
{
    for (Str2VarMapItr iter = (*pars).begin(); iter != (*pars).end(); iter++)
    {
        string cname = iter->first;
        bool found = false;
        for (unsigned i = 0; i < names.size(); i++)
        {
            if ( ( names[i] == cname ) ||
                    ( cname.find(names[i]) != string::npos && opt.find("contains") != string::npos ) )
            { found = true; break; }
        }
        if (opt.find("except") != string::npos) found = !found;
        bool setconst = kTRUE;
        if (opt.find("-free") != string::npos) setconst = kFALSE;
        if ( names.empty() || found ) ((RooRealVar*)iter->second)->setConstant(setconst);
    }
    return *pars;
}

Str2VarMap setConstant(RooAbsPdf * pdf, RooRealVar * var, vector<string> names, string opt)
{
    Str2VarMap pars = getParamList(pdf, var, opt + "-orignames");
    return setConstant(&pars, names, opt);
}


Str2VarMap eraseParameter(Str2VarMap * parlist, vector<string> names)
{
    for (unsigned i = 0; i < names.size(); i++) (*parlist).erase(names[i].c_str());
    return *parlist;
}


TString getLegendLabel( TString title, string opt )
{
    TString leglabel = title.ReplaceAll("_print", "");
    //bool isbkg = leglabel.Contains("bkg_");
    leglabel = leglabel.ReplaceAll("bkg_", "");
    //if (isbkg) leglabel = "Bkg. " + leglabel;
    //leglabel.ReplaceAll("totsig_", "Sig. ");
    leglabel.ReplaceAll("totsig_", "");

    if (opt.find("-origlegnames") == string::npos)
    {
        size_t pos_ = ((string)leglabel).find("_");
        while (true)
        {
            if (((TString)leglabel)(pos_ + 1, 1) == "{")
                pos_ = ((string)leglabel).find("_", pos_ + 1);
            else break;
        }
        leglabel = (TString)((string)leglabel).substr(0, pos_);
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
   @param bins: number of bins to use for data
   @param regions: number of regions to plot is blinded plot, if regions = 0 plots everything
   @param Xtitle: X axis label
   @param title: title
   @param leg: A TLegend object to fill
 **/

RooPlot * getFrame(RooRealVar * var, RooAbsData * data, RooAbsPdf * model,
                   string opt, unsigned bins, vector<string> regStr, map<string, vector<double>> reg,
                   TString Xtitle, TString Ytitle,
                   TLegend * leg, vector <Color_t> custom_colors)
{
    transform(opt.begin(), opt.end(), opt.begin(), ::tolower);
    if (bins < 1) bins = 50;
    var->setRange("PlotRange", var->getMin(), var->getMax());
    RooPlot * frame = new RooPlot(*var, var->getMin(), var->getMax(), bins);

    if (opt.find("-t") == string::npos) frame->SetTitle("");
    else
    {
        size_t pos1 = opt.find("-t");
        size_t pos2 = opt.find("-", pos1 + 1);
        if (pos2 == string::npos) frame->SetTitle((opt.substr(pos1 + 2, string::npos)).c_str());
        else if ((pos2 - pos1) > 1) frame->SetTitle((opt.substr(pos1 + 2, pos2 - pos1 - 2)).c_str());
    }

    RooCmdArg drawOptSig(LineStyle(kDashed)), drawOptBkg(LineStyle(3)), moveBack(RooCmdArg::none());
    if (opt.find("-fillsig") != string::npos) drawOptSig = DrawOption("C");
    if (opt.find("-fillbkg") != string::npos) { drawOptBkg = DrawOption("F"); moveBack = MoveToBack(); }

    if (regStr.size() == 0) regStr.push_back("PlotRange");
    reg["PlotRange"] = {var->getMin(), var->getMax()};
    bool noblind = (regStr.size() == 1);
    RooCmdArg range_data(RooCmdArg::none());
    RooCmdArg range_model(RooCmdArg::none());
    RooCmdArg norm_range(RooCmdArg::none());
    RooCmdArg norm(RooCmdArg::none());
    RooCmdArg totMcolor = LineColor(4);
    TString dataname = "data";
    TString modelname = "model";
    double min = 1e9;

    string rangename = "";
    for (string bandname : regStr)
    {
        if (bandname.find("band") != string::npos) rangename += bandname + ",";
    }
    if (rangename != "") rangename.pop_back();

    if (!noblind)
    {
        range_data  = CutRange(rangename.c_str());
        range_model = Range(rangename.c_str());
        norm_range  = NormRange(rangename.c_str());
    }

    //Plot data and total model

    TString category = "";
    RooCmdArg cut(RooCmdArg::none());
    if (opt.find("-category[") != string::npos)
    {
        size_t pos = opt.find("-category[");
        size_t posend = opt.find("]", pos);
        category = opt.substr(pos + 10, posend - (pos + 10) - 1 );
        cut = Cut("samples==samples::" + category);
    }

    if ( data )
    {
        frame->SetMarkerSize(1);

        RooAbsData * datacut = data;
        if (!noblind)
        {
            string blindcut = "";
            for (auto bandname : regStr)
            {
                if (bandname.find("band") == string::npos) continue;
                if (blindcut != "") blindcut += " || ";
                blindcut += "(" + (string)var->GetName() + " > " + to_string(reg[bandname][0]);
                blindcut += "&&" + (string)var->GetName() + " < " + to_string(reg[bandname][1]) + ")";
            }

            datacut = data->reduce(RooArgSet(*var), RooFormulaVar((TString)blindcut, (TString)blindcut, RooArgSet(*var)));
        }
        if (opt.find("-sumw2err") != string::npos) datacut->plotOn(frame, range_data, Name(dataname), DataError(RooAbsData::SumW2), cut);
        else datacut->plotOn(frame, range_data, Name(dataname), cut);

        RooHist * hist = frame->getHist(dataname);
        Double_t *cont = hist->GetY();
        for (int i = 0; i < hist->GetN(); i++)
            if (cont[i] != 0) min = TMath::Min(min, cont[i]);
        if (min == 1e9) min = 1;
    }

    vector <Color_t> colors = { kCyan + 1, kRed + 1, kGreen + 1, kOrange, kMagenta + 1, kGray, kGray + 1, kGray + 2, kGray + 3 };
    if (custom_colors.size() > 0) colors = custom_colors;
    int styles[] = {3, 4, 5, 6, 7, 8, 9, 10, 3, 4, 5, 6, 7, 8, 9, 10};

    RooArgSet * stackedBkgs = NULL;
    if (opt.find("-stackbkg") != string::npos) stackedBkgs = new RooArgSet("stackedBkgs");

    if ( model )
    {
        //Plot total model

        model->plotOn(frame, totMcolor, Name(modelname), range_model, norm_range, norm);

        //Plot signal and background components

        int counter = 0;
        if (opt.find("-stackbkg") == string::npos) counter++;
        RooArgSet * comps = model->getComponents();
        TIterator * it = comps->createIterator();
        RooAbsArg * arg;
        while ( (arg = (RooAbsArg*)it->Next()) )
        {
            TString legstyle = "l";
            string name = arg->GetName();
            string title = arg->GetName();
            bool isplot = false;

            if (title.find("power_law_approx") != string::npos) continue;
            if (opt.find("-printonly") == string::npos && name.find("_noprint") == string::npos)
            {
                if (name.find("wrtsig") != string::npos) continue;
                if ( noblind && name.find("totsig") != string::npos && opt.find("-nototsigplot") == string::npos)
                {
                    model->plotOn(frame, Components(*arg), drawOptSig, LineColor(1), Name(arg->GetName()), range_model, norm_range, norm);
                    isplot = true;
                }
                else if ( noblind && name.find("sig") != string::npos && opt.find("-plotsigcomp") != string::npos )
                {
                    model->plotOn(frame, Components(*arg), drawOptSig,
                                  LineColor(colors[counter]), LineStyle(styles[counter]),
                                  Name(arg->GetName()), range_model, norm_range, norm);
                    counter++;
                    isplot = true;
                }
                else if (name.find("bkg") != string::npos && name.find("nbkg") == string::npos)
                {
                    int style = styles[counter];
                    if (opt.find("-fillbkg") != string::npos)
                    {
                        legstyle = "f";
                        style = 0;
                    }
                    if (!stackedBkgs)
                    {
                        model->plotOn(frame, Components(*arg), drawOptBkg,
                                      LineColor(colors[counter]), LineStyle(style), FillColor(colors[counter]),
                                      Name(arg->GetName()), moveBack, Range("PlotRange"), norm_range, norm);
                        counter++;
                        isplot = true;
                    }
                    else
                    {
                        stackedBkgs->add(*arg);
                    }
                }
            }
            if (name.find("_print") != string::npos && !isplot)
            {
                model->plotOn(frame, Components(*arg), drawOptSig,
                              LineColor(colors[counter]), LineStyle(styles[counter]), FillColor(colors[counter]),
                              Name(arg->GetName()), moveBack, range_model, norm_range, norm);
                counter++;
                isplot = true;
            }

            TString leglabel = getLegendLabel(arg->GetTitle(), opt);
            if (leg && isplot) leg->AddEntry(frame->findObject(arg->GetName()), leglabel, legstyle);
        }

        if (stackedBkgs)
        {
            int nbkgs = stackedBkgs->getSize();

            for (int bb = 0; bb < nbkgs; bb++)
            {
                RooArgList curBkg;
                TIterator * it = stackedBkgs->createIterator();
                int count = 0;
                RooAbsArg * arg;
                TString myBkgName, myBkgTitle;
                while ((arg = (RooAbsArg*)it->Next()))
                {
                    if ( count > bb ) break;
                    count++;
                    curBkg.add(*arg);
                    myBkgName = arg->GetName();
                    myBkgTitle = arg->GetTitle();
                }

                if (opt.find("-fillbkg") != string::npos)
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

                TString leglabel = getLegendLabel(myBkgTitle, opt);
                TString legstyle = "l";
                if (opt.find("-fillbkg") != string::npos) legstyle = "f";
                if (leg) leg->AddEntry(frame->findObject(myBkgName), leglabel, legstyle);
            }
        }
    }

    TString label = "";
    if (model && data && opt.find("-nochi2") == string::npos)
    {
        double * chi2ndf = calcChi2(frame, getNFreePars(model, RooArgSet(*var)));
        if (opt.find("-chi2ndf") != string::npos) label = Form("#chi^{2}/ndf =  %4.2f", chi2ndf[0]);
        else label = Form("#chi^{2}/ndf =  %4.2f / %4.2f", chi2ndf[0] * chi2ndf[1], chi2ndf[1]);
    }
    if (model && noblind && opt.find("-noparams") == string::npos)
    {
        if (label != "") opt += ("-label[" + label + "]");
        TPaveText * paramBox = createParamBox(model, var, opt);
        frame->addObject(paramBox);
    }

    if (opt.find("-min") != string::npos)
    {
        size_t pos = opt.find("-min");
        TString strMin = opt.substr(pos + 4, string::npos);
        frame->SetMinimum(strMin.Atof());
    }
    else if (opt.find("-log") != string::npos) frame->SetMinimum(min * 0.1);
    else frame->SetMinimum(0.);
    if (opt.find("-max") != string::npos)
    {
        size_t pos = opt.find("-max");
        TString strMin = opt.substr(pos + 4, string::npos);
        frame->SetMaximum(strMin.Atof());
    }
    if (Xtitle != "") frame->SetXTitle(((TString)Xtitle).ReplaceAll("__var__", ""));
    else frame->SetXTitle(((TString)var->GetName()).ReplaceAll("__var__", ""));
    if (Ytitle != "") frame->SetYTitle(Ytitle);

    return frame;
}

RooPlot * getFrame(RooRealVar * var, RooAbsPdf * model, RooAbsData * data, string opt,
                   unsigned bins, vector<string> regStr,  map<string, vector<double>> reg, TString Xtitle, TString Ytitle,
                   TLegend * leg, vector <Color_t>custom_colors)
{
    return getFrame(var, data, model, opt, bins, regStr, reg, Xtitle, Ytitle, leg, custom_colors);
}

RooPlot * getFrame(RooRealVar * var, RooAbsPdf * model, RooAbsData * data, string opt,
                   unsigned bins, TString Xtitle, TString Ytitle, TLegend * leg, vector <Color_t>custom_colors)
{
    return getFrame(var, data, model, opt, bins, vector<string>(1, "PlotRange"), map<string, vector<double>>(), Xtitle, Ytitle, leg, custom_colors);
}

RooPlot * getFrame(RooRealVar * var, RooAbsData * data, RooAbsPdf * model, string opt,
                   unsigned bins, TString Xtitle, TString Ytitle, TLegend * leg, vector <Color_t>custom_colors)
{
    return getFrame(var, data, model, opt, bins, vector<string>(1, "PlotRange"), map<string, vector<double>>(), Xtitle, Ytitle, leg, custom_colors);
}





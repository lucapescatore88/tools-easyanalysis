#include "stringToPdf.hpp"


/*
   This functions allows to choose a predefined PDF defining all its parameters together with it.
   N.B.: In order to add a PDF or modify one this is the only bit of code you should touch!
   */

RooRealVar * addPar(string par, string parstr, Str2VarMap stval_list, Str2VarMap myvars, string option)
{
    RooRealVar * curpar = (RooRealVar *)stval_list[par];
    size_t pos = parstr.find(par + "[");
    if (par == "a2os") pos = parstr.find("a2[");
    
    string dist_name = "";
    size_t posname = option.find("-n");
    if (posname != string::npos)
        dist_name = option.substr(posname + 2, option.find("-", posname + 1) - posname + 2);

    string parMapName = "";
    if (myvars.size() > 0)
    {
        if (par == "a2os") parMapName = isParInMap( "a2", myvars, dist_name );
        else parMapName = isParInMap( par, myvars, dist_name );

        if (parMapName == "") cout << parMapName << " (WRONG!!)" << endl;
    }
    if (parMapName != "") curpar = (RooRealVar *)myvars[parMapName];
    else if (par == "a2os" && myvars.find("a2") != myvars.end()) curpar = (RooRealVar *)myvars["a2"];
    else if (pos != string::npos)
    {
        size_t endPar = parstr.find("]", pos);
        string s = parstr.substr(pos + par.length() + 1, endPar - (pos + par.length() + 1));
        double par_v = ((TString)s).Atof();
        size_t comma1 = s.find(',');
        if (comma1 != string::npos)
        {
            size_t comma2 = s.find(',', comma1 + 1);
            string smin = s.substr(comma1 + 1, comma2 - comma1 - 1);
            double min = ((TString)smin).Atof();
            string smax = s.substr(comma2 + 1, endPar - comma2 - 1);
            double max = ((TString)smax).Atof();
            curpar->setRange(min, max);
        }
        curpar->setVal(par_v);
        if (par_v < curpar->getMin()) curpar->setMin(par_v / 10.);
        if (par_v > curpar->getMax()) curpar->setMax(par_v * 10.);

        if (parstr.substr(pos - 1, 2).find("X") != string::npos) curpar->setConstant(kTRUE);
    }

    return curpar;
}


TString getPrintParName(TString namepdf_, string opt)
{
    namepdf_ = namepdf_.ReplaceAll("__noprint__", "");
    size_t pos_ = ((string)namepdf_).find("_");
    TString namepdf = (TString)(((string)namepdf_).substr(0, pos_));
    TString nameana = (TString)((string)namepdf_).substr(pos_ + 1, string::npos);
    size_t pos__ = ((string)nameana).find("__");
    nameana = (TString)((string)nameana).substr(0, pos__);

    if (((string)namepdf_).find("sig") != string::npos) return "^{" + nameana + "}";

    TString pstrname = "_{" + namepdf + "}";
    if (opt.find("-anaparlab") != string::npos) pstrname += "^{" + nameana + "}";
    return pstrname;
}


Str2VarMap getPar(string typepdf_, TString namepdf_, RooRealVar * val, Str2VarMap myvars, string opt, TString title)
{
    Str2VarMap parout;
    if (typepdf_.find("Poly") != string::npos || typepdf_.find("Cheb") != string::npos) return parout;

    Str2VarMap stval_list;
    double sc = val->getVal() / 5000.;
    namepdf_ = namepdf_.ReplaceAll("__noprint__", "");
    if (title == "") title = namepdf_;
    TString pstrname = getPrintParName(title, opt);

    stval_list["m"]     = new RooRealVar("m_" + namepdf_,  "m" + pstrname,        val->getVal(), val->getMin(), val->getMax());
    stval_list["mg"]    = new RooRealVar("mg_" + namepdf_, "m_{gauss}" + pstrname, val->getVal(), val->getVal() * 0.5, val->getVal() * 2.);
    stval_list["mcb"]   = new RooRealVar("mcb_" + namepdf_, "m_{cb}" + pstrname,   val->getVal(), val->getVal() * 0.5, val->getVal() * 2.);
    stval_list["m0"]    = new RooRealVar("m0_" + namepdf_, "m_{0}" + pstrname,    val->getVal(), val->getVal() * 0.5, val->getVal() * 2.);

    stval_list["s"]     = new RooRealVar("s_" + namepdf_,      "\\sigma" + pstrname,        10 * sc, 1 * sc, 50 * sc);
    stval_list["s2"]    = new RooRealVar("s2_" + namepdf_,     "\\sigma_{2}" + pstrname,    20 * sc, 2 * sc, 100 * sc);
    stval_list["s3"]    = new RooRealVar("s3_" + namepdf_,     "\\sigma_{3}" + pstrname,    30 * sc, 3 * sc, 150 * sc);
    stval_list["sg"]    = new RooRealVar("sg_" + namepdf_,     "\\sigma_{gauss}" + pstrname, 1e2, 1e-2, 1e3);
    stval_list["scb"]   = new RooRealVar("scb_" + namepdf_,    "\\sigma_{cb}" + pstrname,   1e2, 1e-2, 1e3);
    stval_list["sconv"] = new RooRealVar("conv_sg_" + namepdf_, "\\sigma_{res}" + pstrname,  1, 1e-2, 1e2);

    stval_list["a"]     = new RooRealVar("a_" + namepdf_,  "\\alpha" + pstrname,     1, 1e-3, 1e2);
    stval_list["a2"]    = new RooRealVar("a2_" + namepdf_, "\\alpha_{2}" + pstrname, 1, 1e-3, 1e2);
    stval_list["a2os"]  = new RooRealVar("a2_" + namepdf_, "\\alpha_{2}" + pstrname, -1, -1e2, -1e-3);
    stval_list["a3"]    = new RooRealVar("a3_" + namepdf_, "\\alpha_{3}" + pstrname, -1, -1e2, -1e-3);
    stval_list["acb"]   = new RooRealVar("acb_" + namepdf_, "\\alpha_{cb}" + pstrname, 1, 1e-3, 1e2);

    stval_list["n"]    = new RooRealVar("n_" + namepdf_,  "n" + pstrname,     2, 0.2, 1e3);
    stval_list["n2"]   = new RooRealVar("n2_" + namepdf_, "n_{2}" + pstrname, 2, 0.02, 1e3);
    stval_list["n3"]   = new RooRealVar("n3_" + namepdf_, "n_{3}" + pstrname, 2, 0.02, 1e3);
    stval_list["ncb"]  = new RooRealVar("ncb_" + namepdf_, "n_{cb}" + pstrname, 2, 0.2, 1e3);

    stval_list["b"]    = new RooRealVar("b_" + namepdf_, "b" + pstrname,      -5e-4, -1., 0.);
    stval_list["c"]    = new RooRealVar("c_" + namepdf_, "c" + pstrname,      -1, -1e2, 0.);
    stval_list["g"]    = new RooRealVar("g_" + namepdf_, "\\gamma" + pstrname, 30, 5., 1e2);
    stval_list["p"]    = new RooRealVar("p_" + namepdf_, "p" + pstrname,      1, 0., 1e2);

    stval_list["f"]    = new RooRealVar("f_" + namepdf_,  "f" + pstrname,        0.6, 0., 1.);
    stval_list["f2"]   = new RooRealVar("f2_" + namepdf_, "f_{2}" + pstrname,    0.3, 0., 1.);
    stval_list["fg"]   = new RooRealVar("fg_" + namepdf_, "f_{gauss}" + pstrname, 0.5, 0., 1.);
    stval_list["fcb"]  = new RooRealVar("fcb_" + namepdf_, "f_{cb}" + pstrname,   0.5, 0., 1.);

    stval_list["l"]    = new RooRealVar("l_" + namepdf_, "l" + pstrname, -5, -10., -1.);
    stval_list["z"]    = new RooRealVar("z_" + namepdf_, "z" + pstrname, 0.005, 0., 0.01);

    stval_list["nu"]   = new RooRealVar("nu_" + namepdf_, "\\nu" + pstrname, 0., -100., 100.);
    stval_list["tau"]  = new RooRealVar("tau_" + namepdf_, "\\tau" + pstrname, 1., 0., 1000.);

    stval_list["dm2"]  = new RooRealVar("dm2_" + namepdf_, "\\Delta(m)" + pstrname, 224715.0, -1000000.0, 1000000.0);
    stval_list["dx"]  = new RooRealVar("dx_" + namepdf_, "\\Delta(x)" + pstrname, 0.5, 0., 5.);
    stval_list["xmin"]  = new RooRealVar("xmin_" + namepdf_, "xmin" + pstrname, 1.);//, 0., 1000.);
    stval_list["xmax"]  = new RooRealVar("xmax_" + namepdf_, "xmax" + pstrname, 50.);//, 0., 1000.);
    stval_list["pow"]  = new RooRealVar("pow_" + namepdf_, "\\lambda" + pstrname, 2.0, 0.05, 20.);

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
    vector<string> MisIDGaussianPar {"m", "s", "dm2", "pow", "xmin", "xmax", "dx"};
    vector<string> MomFracPdfPar    {"xmin", "dx", "pow"};

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
    par_list["MisIDGauss"]  = MisIDGaussianPar;
    par_list["MomFracPdf"]  = MomFracPdfPar;

    size_t plusgaus = typepdf_.find("AndGauss");
    size_t pluscb   = typepdf_.find("AndCB");
    size_t convgaus = typepdf_.find("ConvGauss");
    if (plusgaus != string::npos) typepdf_ = typepdf_.replace(plusgaus, 8, "");
    if (pluscb != string::npos)   typepdf_ = typepdf_.replace(pluscb, 5, "");
    if (convgaus != string::npos) typepdf_ = typepdf_.replace(convgaus, 9, "");
    string parstr = "";
    size_t endtype = typepdf_.find("-");
    if (endtype != string::npos) parstr = typepdf_.substr(endtype, string::npos);
    vector<string> pars = par_list[typepdf_.substr(0, endtype)];

    for ( auto par : pars )
    {
        if (par == "a2os") parout["a2"] = addPar(par, parstr, stval_list, myvars, opt);
        else parout[par] = addPar(par, parstr, stval_list, myvars, opt);
    }

    if (plusgaus != string::npos)
    {
        parout["mg"] = addPar("mg", parstr, stval_list, myvars, opt);
        parout["sg"] = addPar("sg", parstr, stval_list, myvars, opt);
        parout["fg"] = addPar("fg", parstr, stval_list, myvars, opt);
    }

    if (pluscb != string::npos)
    {
        parout["mcb"] = addPar("mcb", parstr, stval_list, myvars, opt);
        parout["scb"] = addPar("scb", parstr, stval_list, myvars, opt);
        parout["acb"] = addPar("acb", parstr, stval_list, myvars, opt);
        parout["ncb"] = addPar("ncb", parstr, stval_list, myvars, opt);
        parout["fcb"] = addPar("fcb", parstr, stval_list, myvars, opt);
    }

    if (convgaus != string::npos) parout["sconv"] = addPar("sconv", parstr, stval_list, myvars, opt);

    return parout;
}



RooAbsPdf * stringToPdf(const char * typepdf, const char * namepdf, RooRealVar * var, Str2VarMap myvars, string opt, TString title)
{
    RooAbsPdf * pdf = NULL;
    string typepdf_ = (string)typepdf;
    TString namepdf_ = ((TString)namepdf).ReplaceAll("bkg_", "");
    namepdf_ = namepdf_.ReplaceAll("_print", "");
    namepdf_ = namepdf_.ReplaceAll("__noprint__", "");
    if (title == "") title = namepdf_;

    opt += "-n" + (string)var->GetName();

    Str2VarMap p = getPar(typepdf_, namepdf_, var, myvars, opt, title);

    if (typepdf_.find("MomFracPdf") != string::npos)
    {
        pdf = new RooMomentumFractionPdf(namepdf, namepdf, *var, *p["xmin"], *p["dx"], *p["pow"]);
    }
    if (typepdf_.find("MisIDGauss") != string::npos)
    {
        pdf = new RooMisIDGaussian(namepdf, namepdf, *var, *p["m"], *p["s"], *p["dm2"], *p["pow"], *p["xmin"], *p["xmax"], *p["dx"]);
    }
    else if (typepdf_.find("DGauss") != string::npos)
    {
        RooGaussian * gauss1 = new RooGaussian("gauss1_" + namepdf_, "Gauss", *var, *p["m"], *p["s"]);
        RooGaussian * gauss2 = new RooGaussian("gauss2_" + namepdf_, "Gauss", *var, *p["m"], *p["s2"]);

        pdf = new RooAddPdf(namepdf, namepdf, RooArgList(*gauss1, *gauss2), *p["f"]);
    }
    else if (typepdf_.find("TGauss") != string::npos)
    {
        RooGaussian * gauss1 = new RooGaussian("gauss1_" + namepdf_, "Gauss", *var, *p["m"], *p["s"]);
        RooGaussian * gauss2 = new RooGaussian("gauss2_" + namepdf_, "Gauss", *var, *p["m"], *p["s2"]);
        RooGaussian * gauss3 = new RooGaussian("gauss3_" + namepdf_, "Gauss", *var, *p["m"], *p["s3"]);

        pdf = new RooAddPdf(namepdf, namepdf, RooArgList(*gauss1, *gauss2, *gauss3), RooArgList(*p["f"], *p["f2"]));
    }
    else if (typepdf_.find("DCBGauss") != string::npos)
    {
        RooCBShape * CB1 = new RooCBShape("CB1_" + namepdf_, "CB", *var, *p["m"], *p["s"], *p["a"], *p["n"]);
        RooCBShape * CB2 = new RooCBShape("CB2_" + namepdf_, "CB", *var, *p["m"], *p["s2"], *p["a2"], *p["n2"]);

        RooGaussian * gauss1 = new RooGaussian("gauss1_" + namepdf_, "Gauss", *var, *p["m"], *p["s3"]);

        pdf = new RooAddPdf(namepdf, namepdf, RooArgList(*CB1, *CB2, *gauss1), RooArgList(*p["f"], *p["f2"]));
    }
    else if (typepdf_.substr(0, 5).find("Gauss") != string::npos)
    {
        pdf = new RooGaussian(namepdf, title, *var, *p["m"], *p["s"]);
    }
    else if (typepdf_.find("DCB") != string::npos)
    {
        RooCBShape * CB1 = new RooCBShape("CB1_" + namepdf_, "CB", *var, *p["m"], *p["s"], *p["a"], *p["n"]);

        RooCBShape * CB2;
        if (typepdf_.find("_Sn") != string::npos)
            CB2 = new RooCBShape("CB2_" + namepdf_, "CB", *var, *p["m"], *p["s2"], *p["a2"], *p["n"]);
        else CB2 = new RooCBShape("CB2_" + namepdf_, "CB", *var, *p["m"], *p["s2"], *p["a2"], *p["n2"]);

        pdf = new RooAddPdf(namepdf, namepdf, RooArgList(*CB1, *CB2), *p["f"]);
    }
    else if (typepdf_.find("TCB") != string::npos)
    {
        RooCBShape * CB1 = new RooCBShape("CB1_" + namepdf_, "CB", *var, *p["m"], *p["s"], *p["a"], *p["n"]);
        RooCBShape * CB2 = new RooCBShape("CB2_" + namepdf_, "CB", *var, *p["m"], *p["s2"], *p["a2"], *p["n2"]);
        RooCBShape * CB3 = new RooCBShape("CB3_" + namepdf_, "CB", *var, *p["m"], *p["s3"], *p["a3"], *p["n3"]);

        pdf = new RooAddPdf(namepdf, namepdf, RooArgList(*CB1, *CB2, *CB3), RooArgList(*p["f"], *p["f2"]));
    }
    else if (typepdf_.substr(0, 2).find("CB") != string::npos)
    {
        RooAbsPdf *CB = new RooCBShape(namepdf, namepdf, *var, *p["m"], *p["s"], *p["a"], *p["n"]);
        if (typepdf_.find("CBGauss") != string::npos)
        {
            RooGaussian * gaussCB = new RooGaussian("gauss_" + namepdf_, "Gauss", *var, *p["m"], *p["sg"]);
            pdf = new RooAddPdf("CBGauss_" + namepdf_, "CBGauss", RooArgSet(*CB, *gaussCB), *p["fg"]);
        }
        else pdf = CB;
    }
    else if (typepdf_.find("Voigt") != string::npos)
    {
        pdf = new RooVoigtian(namepdf, namepdf, *var, *p["m"], *p["s"], *p["g"]);
    }
    else if (typepdf_.find("BreitWigner") != string::npos)
    {
        pdf = new RooBreitWigner(namepdf, namepdf, *var, *p["m"], *p["g"]);
    }
    else if (typepdf_.find("Poly") != string::npos || typepdf_.find("Cheb") != string::npos)
    {
        TString str_npar = (TString)(typepdf_.substr(4, string::npos));
        int npar = str_npar.Atof();

        vector < double > pvals;
        vector < double > mins;
        vector < double > maxs;

        for (int vv = 0; vv < npar; vv++)
        {
            double pval = 0, min = -1, max = 1;
            size_t posval = typepdf_.find(Form("-v%i[", vv + 1));
            if (posval != string::npos)
            {
                size_t endPar = typepdf_.find("]", posval);
                string s = typepdf_.substr(posval + 3, string::npos);
                pval = ((TString)s).Atof();
                size_t comma1 = s.find(',');
                if (comma1 != string::npos)
                {
                    size_t comma2 = s.find(',', comma1 + 1);
                    string smin = s.substr(comma1 + 1, comma2 - comma1 - 1);
                    min = ((TString)smin).Atof();
                    string smax = s.substr(comma2 + 1, endPar - comma2 - 1);
                    max = ((TString)smax).Atof();
                }
            }

            pvals.push_back(pval);
            mins.push_back(min);
            maxs.push_back(max);
        }
        RooArgList * parList = new RooArgList("parList");
        TString pstrname = getPrintParName(title, opt);
        for (int i = 0; i < npar; i++)
        {
            RooRealVar * v = new RooRealVar(Form("c%i_", i) + namepdf_, Form("c_%i" + pstrname, i), pvals[i], mins[i], maxs[i]);
            parList->add(*v);
        }

        if (typepdf_.find("Poly") != string::npos) pdf = new RooPolynomial(namepdf, namepdf, *var, *parList);
        else pdf = new RooChebychev(namepdf, namepdf, *var, *parList);
    }
    else if (typepdf_.find("ExpAGauss") != string::npos)
    {
        pdf = new RooExpAndGauss(namepdf, namepdf, *var, *p["m"], *p["s"], *p["b"]);
    }
    else if (typepdf_.find("ExpCGauss") != string::npos)
    {
        pdf = new RooGExpModel(namepdf, namepdf, *var, *p["s"], *p["b"]);
    }
    else if (typepdf_.find("Exp") != string::npos)
    {
        pdf = new RooExponential(namepdf, namepdf, *var, *p["b"]);
    }
    else if (typepdf_.find("Argus") != string::npos)
    {
        pdf = new RooArgusBG(namepdf, namepdf, *var, *p["m0"], *p["c"], *p["p"]);
    }
    else if (typepdf_.find("Gamma") != string::npos)
    {
        pdf = new RooGamma(namepdf, namepdf, *var, *p["g"], *p["b"], *p["m"]);
    }
    else if (typepdf_.find("Apollonios") != string::npos)
    {
        pdf = new RooApollonios(namepdf, namepdf, *var, *p["m"], *p["s"], *p["b"], *p["a"], *p["n"]);
    }
    else if (typepdf_.find("Johnson") != string::npos)
    {
        pdf = new RooJohnson(namepdf, namepdf, *var, *p["m"], *p["s"], *p["nu"], *p["tau"]);
    }
    else if (typepdf_.find("Ipatia2") != string::npos)
    {
        pdf = new RooIpatia2(namepdf, namepdf, *var, *p["l"], *p["z"], *p["b"], *p["s"], *p["m"], *p["a"], *p["n"], *p["a2"], *p["n2"]);
    }
    else if (typepdf_.find("Ipatia") != string::npos)
    {
        pdf = new RooIpatia(namepdf, namepdf, *var, *p["l"], *p["z"], *p["b"], *p["s"], *p["m"], *p["a"], *p["n"]);
    }

    if (typepdf_.find("AndGauss") != string::npos)
    {
        pdf->SetName(((TString)pdf->GetName()).ReplaceAll("_print", ""));
        RooGaussian * gauss = new RooGaussian("gauss_" + namepdf_, "Gauss", *var, *p["mg"], *p["sg"]);
        pdf = new RooAddPdf((TString)namepdf + "_plus_Gauss", (TString)namepdf + "_plus_Gauss", RooArgList(*pdf, *gauss), *p["fg"]);
    }

    if (typepdf_.find("AndCB") != string::npos)
    {
        pdf->SetName(((TString)pdf->GetName()).ReplaceAll("_print", ""));
        RooCBShape * cb = new RooCBShape("extraCB_" + namepdf_, "CB", *var, *p["mcb"], *p["scb"], *p["acb"], *p["ncb"]);
        pdf = new RooAddPdf((TString)namepdf + "_plus_CB", (TString)namepdf + "_plus_CB", RooArgList(*pdf, *cb), *p["fcb"]);
    }

    if (typepdf_.find("ConvGauss") != string::npos)
    {
        RooRealVar * mg = new RooRealVar("conv_mg_" + namepdf_, "m_{res}", 0.);
        RooGaussian * resolution_gauss = new RooGaussian("convgauss_" + namepdf_, "", *var, *mg, *p["sconv"]);
        RooNumConvPdf * respdf = new RooNumConvPdf(namepdf, namepdf, *var, *pdf, *resolution_gauss);
        respdf->setConvolutionWindow(*mg, *p["sconv"], 3);
        return (RooAbsPdf *)respdf;
    }
    else return pdf;
}

RooAbsPdf * stringToPdf(const char * typepdf1, const char * typepdf2, const char * namepdf, RooRealVar * var1, RooRealVar * var2, Str2VarMap myvars, string opt, TString title)
{
    RooAbsPdf * pdf1 = stringToPdf(typepdf1, ((string)namepdf + "_" + var1->GetName()).c_str(), var1, myvars, opt, title);
    RooAbsPdf * pdf2 = stringToPdf(typepdf2, ((string)namepdf + "_" + var2->GetName()).c_str(), var2, myvars, opt, title);
    RooAbsPdf * totpdf = new RooProdPdf(namepdf, namepdf, *pdf1, *pdf2);
    return totpdf;
}

RooAbsPdf * stringToPdf(RooAbsPdf * pdf1, const char * typepdf, const char * namepdf, RooRealVar * var, Str2VarMap myvars, string opt, TString title)
{
    RooAbsPdf * pdf2 = stringToPdf(typepdf, ((string)namepdf + "_" + var->GetName()).c_str(), var, myvars, opt, title);
    RooAbsPdf * totpdf = new RooProdPdf(namepdf, namepdf, *pdf1, *pdf2);
    return totpdf;
}

RooAbsPdf * get2DRooKeys(string name, TTree * tree, RooRealVar * v1, RooRealVar * v2, string opt)
{
    RooDataSet * data = new RooDataSet("DataSet", "", tree, RooArgSet(*v1, *v2));
    if (opt.find("-rho") != string::npos)
    {
        int pos = opt.find("-rho");
        string rhostr = opt.substr(pos + 4, 20);
        double rho = ((TString)rhostr).Atof();
        return (RooAbsPdf *)(new RooNDKeysPdf((TString)name, (TString)name, RooArgSet(*v1, *v2), *data, "am", rho));
    }
    return (RooAbsPdf *)(new RooNDKeysPdf((TString)name, (TString)name, RooArgSet(*v1, *v2), *data, "am", 2));
}

RooAbsPdf * get2DRooKeys(TTree * tree, RooRealVar * v1, RooRealVar * v2, string opt)
{
    return get2DRooKeys("keys2D", tree, v1, v2, opt);
}


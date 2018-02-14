#ifndef MODEL_BUILDER_HPP
#define MODEL_BUILDER_HPP

#include <algorithm>

#include "RooMultiVarGaussian.h"

#include "treeReader.hpp"
#include "generalFunctions.hpp"
#include "modelBuilderFunctions.hpp"
#include "stringToPdf.hpp"

using namespace std;
using namespace RooFit;


/** \class ModelBuilder
 * \brief Implements a class which uses RooFit to handel the creation of fit models
 * <br> - some functions like CB, gaussian, Double CB ecc are already available and callable just by name
 * <br> - the model distringuish between signal and background
 * <br> - bkg components can be added ad libitum
 * <br> - trees and histograms can be used to extract signal shapes
 */

class ModelBuilder {

    bool m_isvalid;
    bool m_doNegSig, m_doNegBkg;

protected:

    static string m_pmode;
    TString m_name;
    TString m_title;

    RooRealVar * m_var;
    vector<RooRealVar *> m_vars;
    RooRealVar * m_tmpvar;
    vector<RooRealVar *> m_tmpvars;

    RooAbsPdf * m_model;
    RooAbsPdf * m_sig;
    RooAbsPdf * m_bkg;
    RooAbsReal * m_nsig;
    RooAbsReal * m_nbkg;
    vector <RooAbsPdf *> m_bkg_components;
    vector <RooAbsReal *> m_bkg_fractions;
    RooArgSet * m_constr;

    bool m_totBkgMode;
    vector<Color_t> m_colors;

    void ForceValid() { m_isvalid = true; }

    /// \brief Wrapper for the external function stringToPdf() which returns a model from a string

    RooAbsPdf * StringToPdf(const char * typepdf, const char * namepdf, Str2VarMap mypars, RooRealVar * myvar = NULL, TString _title = "")
    {
        if (!myvar) myvar = m_var;
        return stringToPdf(typepdf, namepdf, myvar, mypars, m_pmode, _title);
    }

    /** \brief Returns a PDF starting from different kinds of objects
     * This is one of the core function of the ModelBuilder.
     * The template type "T" can be:
     * <br> - RooAbsPdf -> in this case it simply returns itself
     * <br> - string    -> it used stringToPdf() to get a PDF from the string
     * <br> - TTree     -> it uses the RooKeysPdf class to extract a smooth pdf from the tree events distribution
     * <br> - TH1F      -> it used the RoohistPdf class to extract a PDF
     *
     * @param _base:  Object to start with to create the PDF
     * @param _name:  Name to give to the newly created PDF
     * @param myvars: This works only if the given type is a string. While creating the model it looks if its parameters are already in "myvars". And if it finds them it uses them to create the new PDF. Does nothing if myvars contains no parameters or wrong parameters or if the given type is not "string".
     * @param weight: In case the starting object is a TTree distributions are created considering the variable "weight" as an event-by-event weight.
     * @param opt:    Options:
     *                This is important because every parameter name must be unique in RooFit.
     *                <br> "-c[]-v[]" (TTree only) : Using TTrees you can apply cuts on them before extracting shapes.
     *                The cuts go inside the "c" braces and in the v braces you must list comma separated all variables on which you cut.
     *                "-c" and "-v" always go together!
     *                e.g.: "-c[B0_PT > 50 && Kst_P > 200]-v[B0_PT,Kst_P]"
     *                <br> "-s(n)" (TTree only): n = 1 or n = 2 (default) defines how fine is the resolution of the smoothing
     *  */
    template <class T> RooAbsPdf * getPdf(T * _base, const char * _name, Str2VarMap mypars = Str2VarMap(), string opt = "", RooRealVar * myvar = NULL, TString _title = "")
    {
        string t = typeid(T).name();
        RooAbsPdf * res = NULL;

        if (_title == "") _title = _name;

        if (!myvar) myvar = m_var;
        if ((t.find("c") != string::npos && t.length() == 1) || t.find("TString") != string::npos)
        {
            res = (RooAbsPdf*)StringToPdf((const char *)_base, _name, mypars, myvar, _title);
        }
        else if (t.find("RooAbsPdf") != string::npos)
        {
            res = (RooAbsPdf*)_base;

            if (t.find("vector") != string::npos)
            {
                RooArgList * pdfList = new RooArgList("pdfList_" + m_name);
                RooArgList * fracList = new RooArgList("fracPdfList_" + m_name);

                for (auto el : * (vector<RooAbsPdf *> *)_base)
                {
                    RooRealVar * f  = new RooRealVar(("f_" + (string)(el->GetName())).c_str(), "f", 100., 0., 1.e10);
                    pdfList->add(*el);
                    fracList->add(*f);
                }

                res = new RooAddPdf("totpdf_" + m_name, "totpdf_" + _title, *pdfList, *fracList);
            }

            res->SetName(_name);
        }
        else if (t.find("TTree") != string::npos)
        {
            RooArgSet * vars = new RooArgSet(*myvar);
            if (opt.find("-v[") != string::npos)
            {
                size_t pos = opt.find("-v[") + 2;
                size_t posend = opt.find("]", pos);

                while (true)
                {
                    size_t pos2 = opt.find(",", pos + 1);
                    TString vname = (TString)opt.substr(pos + 1, pos2 - pos - 1);
                    if (pos2 >= posend) vname.ReplaceAll("]", "");
                    vars->add(*(new RooRealVar(vname, vname, 0.)));
                    if (pos2 < posend) pos = pos2;
                    else break;
                }
            }

            RooDataSet * sigDataSet = NULL;
            if (opt.find("-w[") != string::npos)
            {
                size_t pos = opt.find("-w[") + 2;
                size_t posend = opt.find("]", pos);
                if (pos != posend - 1) {
		    TString wname = (TString)opt.substr(pos + 1, posend - pos - 1);
		    if (wname != "1")
		    {
			vars->add(*(new RooRealVar(wname, wname, 0.)));
			sigDataSet = new RooDataSet((TString)_name + "_DataSet_" + m_name, "", (TTree*)_base, *vars, 0, wname);
		    }
		}
            }
            else sigDataSet = new RooDataSet((TString)_name + "_DataSet_" + m_name, "", (TTree*)_base, *vars);

            if (opt.find("-c[") != string::npos)
            {
                size_t pos = opt.find("-c[") + 3;
                size_t posend = opt.find("]", pos);
                TString treecut = (TString)(opt.substr( pos, posend - pos));
                sigDataSet->Print();
                sigDataSet->reduce(treecut);
            }

            if (opt.find("-rho") != string::npos)
            {
                int pos = opt.find("-rho");
                string rhostr = opt.substr(pos + 4, 20);
                double rho = ((TString)rhostr).Atof();
                res = new RooKeysPdf((TString)_name, _title, *myvar, *sigDataSet, RooKeysPdf::MirrorBoth, rho);
            }
            else res = new RooKeysPdf((TString)_name, _title, *myvar, *sigDataSet, RooKeysPdf::MirrorBoth, 2);
            /*if(opt.find("-noshift") == string::npos)
            {
                RooRealVar * shift = new RooRealVar("shift_rookey_"+(TString)_name,"shift_rookey_"+(TString)_name,0.,-1000.,1000.);
                RooRealVar * dummy_sigma = new RooRealVar("dummysigma_rookey","dummysigma_rookey",0.);
                RooGaussian * resMod = new RooGaussian("resmod_"+(TString)_name,"resmod_"+(TString)_name,*myvar,*shift,*dummy_sigma);
                res = new RooFFTConvPdf(res->GetName()+(TString)"_conv",res->GetTitle(),*myvar,*res,*resMod);
                //keyvar = new RooFormulaVar("x_rookey_"+(TString)_name,"x_rokey_"+(TString)_name,"@0+@1",RooArgList(*myvar,*shift));
            }*/


            if (opt.find("-print") != string::npos)
            {
                TCanvas * c = new TCanvas();
                RooPlot * keysplot = myvar->frame();
                sigDataSet->plotOn(keysplot);
                res->plotOn(keysplot);
                keysplot->SetTitle(_name);
                keysplot->Draw();
                TString name_ = _name;
                name_.ReplaceAll("__noprint__", "");
                name_.ReplaceAll(" ", "_");
                name_.ReplaceAll("#", "_").ReplaceAll("^", "_");
                name_.ReplaceAll("{", "_").ReplaceAll("}", "_");
                name_.ReplaceAll("(", "_").ReplaceAll(")", "_");
                name_.ReplaceAll(":", "_");
                name_.ReplaceAll("/", "_").ReplaceAll("+", "_").ReplaceAll("-", "_").ReplaceAll("*", "_");
                name_.ReplaceAll(",", "_").ReplaceAll(".", "_");
                name_.ReplaceAll("__", "_").ReplaceAll("_for", "__for");
                c->Print("rooKeysModel_" + name_ + ".pdf");
                delete c;
                delete keysplot;
            }
        }
        else if (t.find("TH1") != string::npos)
        {
            RooDataHist * sigDataHist = new RooDataHist((TString)_name + "_DataHist" + m_name, "", *myvar, (TH1*)_base, 6);
            res = new RooHistPdf((TString)_name, _name, *myvar, *sigDataHist);
        }
        else
        {
            if (((string)_name).find("_noprint") != string::npos) cout << m_name << ": ATTANTION:_noprint option in background name: component won't be added to background list" << endl;
            else cout << m_name << ": ***** ATTANTION: Wrong type (" << t << ") given to getPdf. Only string, RooAbsPdf, vector<RooAbsPdf *>, TTree and TH1 are allowed! *****" << endl;
        }

        return res;
    }


    /** \brief Adds a bkg component.
     * Adds a bkg component named "_name" based on the object "_comp".
     * @param _frac: This can ve a RooRealVar/RooFormulaVar or a double. The Roo object are set as yields of the current component.
     * <br> In case it's a double a Roo object is created in the following way:
     *  <br> - _frac > 1    creates a RooRealVar with "_frac" value (in the fit the yield will start from "_frac" and float)
     *  <br> - |_frac| <= 1 created a RooFormulaVar connecting this background yield as a fraction "_frac" of the singla yield
     *  <br> - _frac < -1   same as _frac > 1 but the number is fixed. Meaning that the yield of this bacgkround will not float in the fit
     *
     * @param opt: Options:
     * -> It is passed as options to getPdf()
     * <br> - "-ibegin":  Adds the bkg component in front and not at the back of the backgrounds list
     *
     * @param myvars:
     * -> It is passed as argument of getPdf()
     *  */

    template <class T> RooAbsPdf * AddBkgComponentPvt(const char * _name, T * _comp, RooAbsReal * _frac, const char * _opt = "", Str2VarMap _myvars = Str2VarMap())
    {
        if (!m_sig) { cout << "*** WARNING: Signal not set! Set the signal before any background!" << endl; return NULL; }

        TString nstr = "bkg_" + (TString)_name;
        string lowopt = (string)_opt;
        transform(lowopt.begin(), lowopt.end(), lowopt.begin(), ::tolower);
        RooAbsReal * frac = NULL;

        nstr += ("_" + m_name);

        if (lowopt.find("-frac") != string::npos)
        {
            TString ss( (TString)m_nsig->GetName() + " * " + _frac->GetName() );
            frac = new RooFormulaVar("n" + nstr, "f_{" + (TString)_name + "}^{wrtsig}", ss, RooArgSet(*_frac, *m_nsig));
        }
        else { frac = _frac; }

        frac->SetName((TString)frac->GetName() + "_" + m_name);

        nstr += "__for_" + (TString)m_var->GetName();
        RooAbsPdf * comp = getPdf(_comp, nstr, _myvars, _opt, (RooRealVar *)NULL, nstr + "_" + m_title); // + "__print");

        if (comp != NULL && _frac != NULL && lowopt.find("-nofit") == string::npos)
        {
            if (lowopt.find("-ibegin") == string::npos)
            {
                m_bkg_components.push_back(comp);
                m_bkg_fractions.push_back(frac);
                SetLastBkgColor(GetDefaultColors()[m_bkg_components.size() - 1]);
            }
            else
            {
                m_bkg_components.insert(m_bkg_components.begin(), comp);
                m_bkg_fractions.insert(m_bkg_fractions.begin(), frac);
                m_colors.insert(m_colors.begin(), GetDefaultColors()[m_bkg_components.size() - 1]);
            }
        }

        return comp;
    }

    template <class T> RooAbsPdf * AddBkgComponentPvt(const char * _name, T * _comp, double _frac = 0, const char * _opt = "", Str2VarMap _myvars = Str2VarMap())
    {
        if (!m_sig) { cout << "*** WARNING: Signal not set! Set the signal before any background!" << endl; return NULL; }

        TString nstr = "bkg_" + (TString)_name;
        RooAbsReal * frac = NULL;
        double val = TMath::Abs(_frac);
        if (val == 0) val = 1e3;
        double min = 0;
        if (m_doNegBkg) min = -3 * TMath::Sqrt(val);
        double max = 1.e7;

        if (_frac < 0) { min = val; max = val; }

        if (m_totBkgMode)
        {
            if (val > 1) { cout << "Attention in 'm_totBkgMode' the nevt must be between 0 and 1" << endl; return NULL; }
            frac = new RooRealVar("f" + nstr, "f_{" + (TString)_name + "}", val, 0, 1);
        }
        else if ((TMath::Abs(_frac) > 0 && TMath::Abs(_frac) <= 1) || ((string)_opt).find("-frac") != string::npos)
        {
            TString ss( (TString)m_nsig->GetName() + Form(" * %e", val) );
            if (!m_nsig) { cout << "Attention if you use this option abs(nevt) < 1 you must set the signal first." << endl; return NULL; }
            frac = new RooFormulaVar("n" + nstr, "f_{" + (TString)_name + "}^{wrtsig}", ss, *m_nsig);
        }
        else frac = new RooRealVar("n" + nstr, "N_{" + (TString)_name + "}", val, min, max);

        return AddBkgComponentPvt(_name, _comp, frac, _opt, _myvars);
    }


    /** \brief Adds the signal component
     * The behaviour it's similar to AddBkgComponent().
     * The only difference is the nsig, is set as signal yield. If given as a double it can be only > 1 (starting value)
     * or < -1 (starting value fixed in the fit).
     * */

    template <class T> RooAbsPdf * SetSignalPvt(T * _sig, RooAbsReal * _nsig, const char * opt = "", Str2VarMap myvars = Str2VarMap())
    {
        string lowopt = (string)opt;
        transform(lowopt.begin(), lowopt.end(), lowopt.begin(), ::tolower);

        ResetVariable();
        TString myname = "_" + m_name;
        m_nsig = _nsig;
        m_nsig->SetName((TString)m_nsig->GetName() + myname);
        myname += "__for_" + (TString)m_var->GetName();
        if (_sig)
        {
            m_sig = getPdf(_sig, "sig" + myname, myvars, opt, (RooRealVar *)NULL, "sig_" + m_title);
            m_sig->SetName("totsig" + myname);
            m_sig->SetTitle("totsig_" + m_title); // + "__print");
        }
        return m_sig;
    }

    template <class T> RooAbsPdf * SetSignalPvt(T * _sig, double _nsig = 0., const char * opt = "", Str2VarMap myvars = Str2VarMap())
    {
        RooAbsReal * tmpnsig = NULL;
        double val = TMath::Abs(_nsig);
        if (val == 0) val = 1e3;
        double min = 0;
        if (m_doNegSig) min = -3 * TMath::Sqrt(val);
        double max = 1.e7;
        if (_nsig < 0) { min = val; max = val; }

        if (TMath::Abs(_nsig) > 0 && TMath::Abs(_nsig) <= 1) tmpnsig = new RooRealVar("nsig", "N_{" + m_title + "}", val * max, min, max);
        else tmpnsig = new RooRealVar("nsig", "N_{" + m_title + "}", val, min, max);

        return SetSignalPvt(_sig, tmpnsig, opt, myvars);
    }



public:

    //Constructors

    ModelBuilder(TString _name, RooRealVar * _var, TString _title = ""):
        m_isvalid(false), m_doNegSig(false), m_doNegBkg(false), m_name(_name), m_title(_title),
        m_sig(NULL), m_bkg(NULL), m_totBkgMode(false), m_colors(vector<Color_t>())
    {
        SetVariable(_var);
        m_vars.push_back(_var);
        m_nsig = new RooRealVar("nsig_" + m_name, "N_{sig}", 0., 0., 1.e8);
        m_nbkg = new RooRealVar("nbkg_" + m_name, "N_{bkg}", 0., 0., 1.e8);
        if (_title == "") m_title = m_name;
        m_constr = new RooArgSet("constraints_" + m_name);
    }

    ~ModelBuilder()
    {
        /*
        delete m_var;
        delete m_model;
        delete m_sig;
        delete m_bkg;
        delete m_nsig;
        delete m_nbkg;

        for(unsigned i = 0; i < m_bkg_components.size(); i++)
        {
            delete m_bkg_components[i];
            delete m_bkg_fractions[i];
        }
        */
    }


    //Methods

    /** \brief Builds the model:
     * Using RooAddPdf builds a model using all components in sig and background_compnents.
     * Options:
     * <br> "-exp":     Automatically adds an exponential coponent to the background.
     * <br> "-noBkg" -> ignores bkg
     * */
    RooAbsPdf * Initialize(string optstr = "");


    /* Public overloads of AddBkgComponentPvt, necessary for python interface */

    RooAbsPdf * AddBkgComponent(const char * _name, const char * _comp, double _frac = 0, Str2VarMap _myvars = Str2VarMap(), const char * _opt = "")
    {
        return AddBkgComponentPvt(_name, _comp, _frac, _opt, _myvars);
    }

    RooAbsPdf * AddBkgComponent(const char * _name, TTree * _comp, double _frac = 0, Str2VarMap _myvars = Str2VarMap(), const char * _opt = "")
    {
        return AddBkgComponentPvt(_name, _comp, _frac, _opt, _myvars);
    }

    RooAbsPdf * AddBkgComponent(const char * _name, RooAbsPdf * _comp, double _frac = 0, Str2VarMap _myvars = Str2VarMap(), const char * _opt = "")
    {
        return AddBkgComponentPvt(_name, _comp, _frac, _opt, _myvars);
    }

    RooAbsPdf * AddBkgComponent(const char * _name, const char * _comp, RooAbsReal * _frac, Str2VarMap _myvars = Str2VarMap(), const char * _opt = "")
    {
        return AddBkgComponentPvt(_name, _comp, _frac, _opt, _myvars);
    }

    RooAbsPdf * AddBkgComponent(const char * _name, TTree * _comp, RooAbsReal * _frac, Str2VarMap _myvars = Str2VarMap(), const char * _opt = "")
    {
        return AddBkgComponentPvt(_name, _comp, _frac, _opt, _myvars);
    }

    RooAbsPdf * AddBkgComponent(const char * _name, RooAbsPdf * _comp, RooAbsReal * _frac, Str2VarMap _myvars = Str2VarMap(), const char * _opt = "")
    {
        return AddBkgComponentPvt(_name, _comp, _frac, _opt, _myvars);
    }

    RooAbsPdf * AddBkgComponent(const char * _name, const char * _comp, Str2VarMap _myvars, double _frac = 0, const char * _opt = "")
    {
        return AddBkgComponentPvt(_name, _comp, _frac, _opt, _myvars);
    }

    RooAbsPdf * AddBkgComponent(const char * _name, TTree * _comp, Str2VarMap _myvars, double _frac = 0, const char * _opt = "")
    {
        return AddBkgComponentPvt(_name, _comp, _frac, _opt, _myvars);
    }

    RooAbsPdf * AddBkgComponent(const char * _name, RooAbsPdf * _comp, Str2VarMap _myvars, double _frac = 0, const char * _opt = "")
    {
        return AddBkgComponentPvt(_name, _comp, _frac, _opt, _myvars);
    }


    template <class T> RooAbsPdf * SetExtraBkgDimension(const char * _name, T * _pdf, RooRealVar * extravar, string opt = "", Str2VarMap myvars = Str2VarMap())
    {
        RooAbsPdf * bkg = NULL;
        for (auto bb : m_bkg_components)
        {
            if ( ((string)bb->GetName()).find(_name) != string::npos )
            {
                bkg = bb;
                break;
            }
        }

        if (!bkg) { cout << "You must set a background component with the specified name using AddBkgComponent() first" << endl; return NULL; }

        m_vars.push_back(extravar);

        RooAbsPdf * old_bkg = bkg;

        TString pdfname   = ((TString)old_bkg->GetName()).ReplaceAll("__noprint__", "");
        size_t posfor = ((string)pdfname).find("__for");
        TString name_comp = ((string)pdfname).substr(0, posfor) + "__for_" + (TString)extravar->GetName() + "__noprint__";
        TString name_tot  = pdfname + "__and_" + (TString)extravar->GetName();
        old_bkg->SetName(pdfname + "__noprint__");

        RooAbsPdf * new_comp = getPdf(_pdf, name_comp, myvars, opt, extravar);
        bkg = new RooProdPdf("prod", "", *old_bkg, *new_comp);

        bkg->SetName(name_tot);
        bkg->SetTitle(name_tot);

        return bkg;
    }

    /*template <class T> RooAbsPdf * SetExtraBkgDimension(const char * _name, T * _pdf, RooRealVar * extravar, string opt, Str2VarMap myvars = Str2VarMap())
    {
        return SetExtraBkgDimension(_name, _pdf, extravar, opt, myvars);
    }*/



    /* Public overloads of AddBkgComponentPvt, necessary for python interface */

    RooAbsPdf * SetSignal(RooAbsPdf * _sig, RooAbsReal * _nsig, Str2VarMap _myvars = Str2VarMap(), const char * _opt = "")
    {
        return SetSignalPvt(_sig, _nsig, _opt, _myvars);
    }

    RooAbsPdf * SetSignal(RooAbsPdf * _sig, double _nsig = 0., Str2VarMap _myvars = Str2VarMap(), const char * _opt = "")
    {
        return SetSignalPvt(_sig, _nsig, _opt, _myvars);
    }

    RooAbsPdf * SetSignal(TTree * _sig, RooAbsReal * _nsig, Str2VarMap _myvars = Str2VarMap(), const char * _opt = "")
    {
        return SetSignalPvt(_sig, _nsig, _opt, _myvars);
    }

    RooAbsPdf * SetSignal(TTree * _sig, double _nsig = 0., Str2VarMap _myvars = Str2VarMap(), const char * _opt = "")
    {
        return SetSignalPvt(_sig, _nsig, _opt, _myvars);
    }

    RooAbsPdf * SetSignal(const char * _sig, RooAbsReal * _nsig, Str2VarMap _myvars = Str2VarMap(), const char * _opt = "")
    {
        return SetSignalPvt(_sig, _nsig, _opt, _myvars);
    }

    RooAbsPdf * SetSignal(const char * _sig, double _nsig = 0., Str2VarMap _myvars = Str2VarMap(), const char * _opt = "")
    {
        return SetSignalPvt(_sig, _nsig, _opt, _myvars);
    }

    RooAbsPdf * SetSignal(const char * _sig, Str2VarMap _myvars, double _nsig = 0., const char * _opt = "")
    {
        return SetSignalPvt(_sig, 0., _opt, _myvars);
    }

    RooAbsPdf * SetSignal(RooAbsPdf * _sig, Str2VarMap _myvars, double _nsig = 0., const char * _opt = "")
    {
        return SetSignalPvt(_sig, 0., _opt, _myvars);
    }

    RooAbsPdf * SetSignal(TTree * _sig, Str2VarMap _myvars, double _nsig = 0., const char * _opt = "")
    {
        return SetSignalPvt(_sig, 0., _opt, _myvars);
    }



    template <class T> RooAbsPdf * SetExtraSignalDimension(T * _sig, RooRealVar * extravar, string opt = "", Str2VarMap myvars = Str2VarMap())
    {
        if (!m_sig) { cout << "You must set the signal using SetSignal() first" << endl; return NULL; }

        m_vars.push_back(extravar);

        RooAbsPdf * old_sig = m_sig;
        TString pdfname = ((TString)m_sig->GetName()).ReplaceAll("__noprint__", "").ReplaceAll("totsig", "sig");
        size_t posfor = ((string)pdfname).find("__for");
        TString name_comp = ((string)pdfname).substr(0, posfor) + "__for_" + (TString)extravar->GetName() + "__noprint__";
        TString name_tot = pdfname + "__and_" + (TString)extravar->GetName();
        m_sig->SetName(pdfname + "__noprint__");

        RooAbsPdf * new_comp = getPdf(_sig, name_comp, myvars, opt, extravar);

        m_sig = new RooProdPdf("prod", "", *old_sig, *new_comp);
        m_sig->SetName(name_tot);
        m_sig->SetTitle(name_tot);

        return m_sig;
    }

    template <class T> RooAbsPdf * SetExtraSignalDimension(T * _sig, RooRealVar * extravar, string opt, string weight, Str2VarMap myvars = Str2VarMap())
    {
        return SetExtraSignalDimension(_sig, extravar, opt, myvars, weight);
    }

    void AllowNegativeYield(string option = "", bool cond = true)
    {
        cout << endl << m_name << ": AllowNegativeYield";
        if (option.find("sig") != string::npos)
        {
            m_doNegSig = cond;
            if (m_doNegSig) cout << " - Signal";
        }
        if (option.find("bkg") != string::npos)
        {
            m_doNegBkg = cond;
            if (m_doNegBkg) cout << " - Backgrounds";
        }
        cout << endl << endl;
        return;
    }

    void AddConstraint(RooAbsReal * pdfconst) { m_constr->add(*pdfconst); }
    void AddGaussConstraint(TString pdf, TString name, double mean, double sigma);
    void AddGaussConstraint(TString name, double mean, double sigma);
    void AddGaussConstraint(RooRealVar * par, double mean = -1e9, double sigma = -1e9);
    RooArgSet * GetConstraints() { return m_constr; }

    void SetLastBkgColor(Color_t color)
    {
        if (m_colors.size() == m_bkg_components.size()) m_colors[m_colors.size() - 1] = color;
        else m_colors.push_back(color);
    }
    vector <Color_t> GetColors() { return m_colors; }

    void SetTitle(TString _title) { m_title = _title; }

    /** \brief Sets the variable to fit "var".
     * It also creates a copy of the initial variable "m_tmpvar". In fact when fitting the varible is modified and
     * if you want to fit again or change the model and refit you need to reset the variable as it was.
     * */
    void SetVariable(RooRealVar * _var)
    {
        _var->SetTitle( ((TString)_var->GetTitle()).ReplaceAll("__var__", "") + "__var__" );
        m_var = _var;
        m_tmpvar = new RooRealVar(*_var);
    }

    /** \brief Sets the background mode
     * Normally the model is built as model = nsig*sig + nbkg1*bkg1 + nbkg2*bkg2 * ...
     * If the m_totBkgModel is set to "tot" the model is built as model = nsig*sig + nTotbkg*(fracBkg1*bkg1 + fracBkg2*bkg2 + ...)
     * */
    void SetBkgMode( bool mode ) { m_totBkgMode = mode; }
    void SetSig(RooAbsPdf * _sig) { m_sig = _sig; }
    ///\brief Forces a model
    void SetModel(RooAbsPdf * _model);
    void SetNSig(RooAbsReal * _nsig) { m_nsig = _nsig; }
    void SetBkg(RooAbsPdf * _bkg) { m_bkg = _bkg; }
    void SetBkg(vector<RooAbsPdf *> _bkg_comp) { m_bkg_components = _bkg_comp; }
    void SetName(const char * newname) { m_name = newname; }
    void ClearBkgList() { m_bkg_components.clear(); m_bkg_fractions.clear(); }
    void ResetVariable() { m_var->setVal(m_tmpvar->getVal()); m_var->setRange(m_tmpvar->getMin(), m_tmpvar->getMax()); };

    RooAbsPdf * GetParamsGaussian(RooFitResult * fitRes);
    RooDataSet * GetParamsVariations(int nvariations = 10000, RooFitResult * fitRes = NULL);

    ////\brief Return true if the model was correctly built and initialized
    bool isValid() { return m_isvalid; }
    //bool isExtended() { return extended; }
    TString GetName() { return m_name; }
    ///\brief Return the variable to fit
    RooRealVar * GetVariable() { return m_var; }
    ///\brief Returns the model pdf
    RooAbsPdf * GetModel() { return m_model; }
    ///\brief Returns the signal pdf
    RooAbsPdf * GetSig() { return m_sig; }
    ///\brief Returns a pdf corresponding to the sum of all bkg PDFs
    RooAbsPdf * GetTotBkg() { return m_bkg; }
    ///\brief Returns the number of bkg events integrating the bkg pdf in [min,max] (Returns nbkg * (int [min,max] of bkg) )
    /// If "fitRes" is passed stores the error in "valerr"
    double GetNBkgVal(double min = 0, double max = 0, double * valerr = NULL, RooFitResult * fitRes = NULL);
    ///\brief Returns the number of sig events integrating the sig pdf in [min,max] (Returns nsig * (int [min,max] of sig) )
    /// If "fitRes" is passed stores the error in "valerr"
    double GetNSigVal(double min = 0, double max = 0, double * valerr = NULL, RooFitResult * fitRes = NULL);
    ///\brief Return S/B integrating sig and bkg in [min,max]
    double GetSOverB(float min, float max, double * valerr = NULL, RooFitResult * fitRes = NULL);
    ///\brief Return S/(S+B) integrating sig and bkg in [min,max]
    double GetSigFraction(float min, float max, double * valerr = NULL, RooFitResult * fitRes = NULL);
    ///\brief Returns the full list of bkg PDFs
    vector<RooAbsPdf *> GetBkgComponents() { return m_bkg_components; }
    ///\brief Prints to screen the composition e.g signal = NSIG / NTOT, bkg1 = NBKG1/NTOT, etc
    void PrintComposition(float min = 0., float max = 0., RooFitResult * fitRes = NULL);
    ///\brief Returns the full list of bkg yields variables
    vector<RooAbsReal *> GetBkgFractions() { return m_bkg_fractions; }

    int GetBkgID( string name )
    {
        for(size_t i = 0; i < m_bkg_components.size(); i++)
            if(((string)m_bkg_components[i]->GetTitle()).find("bkg_"+name+"_")!=string::npos) return i;
        return -1;
    }
    RooAbsReal * GetBkgFraction( string name ) 
    { 
        int id = GetBkgID( name );
        if(id<0) return NULL;
        return m_bkg_fractions[id];
    }
    Str2VarMap GetBkgParams( string name )
    {
        int id = GetBkgID( name );
        if(id < 0) return Str2VarMap();
        return getParams(m_bkg_components[id],RooArgSet(*m_var),vector<string>());
    }
    
    ///\brief Returns the number of sig events in the full range. Same as GetNSigVal(0,0)
    double GetSigVal(double * valerr = NULL, RooFitResult * fitRes = NULL);
    ///\brief Returns the number of sig events in the full range. And can also return its asymmetric error
    double GetSigVal(double * errHi, double * errLo);
    bool CheckModel() { return checkModel(m_model); };
    RooArgSet * GetParamsArgSet();

    /** \brief Returns the number of total bkg evens
     * Returns a RooAbsReal variable which is a RooFormulaVar built as the sum of all bkg yields.
     */
    RooAbsReal * GetTotNBkg();
    RooAbsPdf * CalcTotBkg();
    RooAbsReal * GetNSigPtr() { return m_nsig; }
    ///\brief Returns a Str2VarMap object containing all paramters only of the signal PDF
    Str2VarMap GetSigParams(string opt = "");
    ///\brief Returns a Str2VarMap object containing all paramters of the entire model PDF, inclusing yields
    Str2VarMap GetParams(string opt = "");

    static void SetPrintLevel(string mode) { m_pmode = mode; }

    /** \brief Prints a plots with the model on it using the GetFrame() function.
     * @param title:  Title for the plot
     * @param Xtitle: Title for the X axis (can be latex)
     * @param data:   In case you want to print some data on it (from some external source)
     * @param bins:   How many bins
     * @param regStr: This vector containg a list of ranges to print. See GetFrame()
     * @param range:  Range to print
     * @param opt:    All options available from GetFrame() plus:
     * <br> - "-LHCb"/"-LHCbDX" -> Draws the LHCb name on the plot by default on the top left corner (if DX top right)
     * <br> - "-noleg"          -> Doesn't draw the legend
     * <br> - "-log"            -> Sets Y axis to logarithmic scale
     * <br> - "-quality"        -> Draws a box containing fir quality parameters (EDM and covQual)
     * <br> - "-H"              -> If "data != NULL" produces a separate histogram with pulls distribution
     * <br> - "-pulls"          -> Produces a separate hitogram with pulls vs variable
     * <br> - "-andpulls"       -> Produced an histogram with pulls and puts it below the main plot
     * */
    RooPlot * Print(TString title = "", TString Xtitle = "", string opt = "", RooAbsData* data = NULL, int bins = 50,
                    vector<string> regStr = vector<string>(), map<string, vector<double>> reg = map<string, vector<double>>(),
                    RooFitResult * fitRes = NULL, TString Ytitle = "", RooRealVar * myvar = NULL);
    void Print(TString title = "", TString Xtitle = "", string opt = "", RooAbsData* data = NULL, int bins = 50,
               RooFitResult * fitRes = NULL, TString Ytitle = "", vector<RooRealVar *> myvar = vector<RooRealVar *>());


    ///\brief Prints all the paramters to screen in RooFit format (opt="-nocost" skips constants)
    void PrintParams(string opt = "") { PrintPars(GetParams("-orignames"), opt); }
    ///\brief Prints the sgnal pdf paramters to screen in RooFit format (opt="-nocost" skips constants)
    void PrintSigParams(string opt = "") { PrintPars(GetSigParams("-orignames"), opt); }
    ///\brief Prints all the paramters to screen in latex table format (opt="-nocost" skips constants)
    void PrintParamsTable(string opt = "") { PrintPars(GetParams("-orignames"), "-latex" + opt); }
    ///\brief Prints all sigal PDF paramters to screen in latex table format (opt="-nocost" skips constants)
    void PrintSigParamsTable(string opt = "") { PrintPars(GetSigParams("-orignames"), "-latex" + opt); }

    ///\brief Returns the value of S(x)/(S(x)+B(x)) for x = value. It corresponds to a naive S-weight.
    float GetReducedSWeight(float value);

    RooWorkspace * SaveToRooWorkspace(string option);

};


#endif

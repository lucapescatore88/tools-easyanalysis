/*
 * Author : Luca Pescatore, Simone Bifani
 * Email  : luca.pescatore@cern.ch
 * Date   : 17/12/2015
 */



#ifndef MODEL_BUILDER_HPP
#define MODEL_BUILDER_HPP

#include "ReadTree_comp.hpp"
#include "general_functions.hpp"
#include "ModelBuilder_helpfunctions.hpp"

using namespace std;

/** \class ModelBuilder
 * \brief Implements a class which uses RooFit to handel the creation of fit models
 * <br> - some functions like CB, gaussian, Double CB ecc are already available and callable just by name
 * <br> - the model distringuish between signal and background
 * <br> - bkg components can be added ad libitum
 * <br> - trees and histograms can be used to extract signal shapes
 */

class ModelBuilder {

    bool isvalid;

    protected:

    static string pmode;
    TString name;
    RooRealVar * var;
    vector<RooRealVar *> vars; 
    RooAbsPdf * model;
    RooAbsPdf * sig;
    RooAbsPdf * bkg;
    RooAbsReal * nsig;
    RooAbsReal * nbkg;
    vector <RooAbsPdf *> bkg_components;
    vector <RooAbsReal *> bkg_fractions;
    RooRealVar * tmpvar;
    bool totBkgMode;
    TString title;
    vector<Color_t> mycolors;

    void ForceValid() { isvalid = true; }

    /// \brief Wrapper for the external function stringToPdf() which returns a model from a string

    RooAbsPdf * StringToPdf(const char * typepdf, const char * namepdf, Str2VarMap mypars, RooRealVar * myvar = NULL)
    {
        if(!myvar) myvar = var; 
        return stringToPdf(typepdf, namepdf, myvar, mypars, pmode);
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
     *	*/
    template <class T> RooAbsPdf * getPdf(T * _base, const char * _name, Str2VarMap mypars = Str2VarMap(), string weight = "", string opt = "", RooRealVar * myvar = NULL)
    {
        string t = typeid(T).name();
        RooAbsPdf * res = NULL;

        if(!myvar) myvar = var;
        if((t.find("c")!=string::npos && t.length()==1) || t.find("TString")!=string::npos)
        {
            res = (RooAbsPdf*)StringToPdf((const char *)_base, _name, mypars, myvar);
        }
        else if(t.find("RooAbsPdf")!=string::npos)
        {
            res = (RooAbsPdf*)_base;

            if(t.find("vector")!=string::npos)
            {
                RooArgList * pdfList = new RooArgList("pdfList_"+name);
                RooArgList * fracList = new RooArgList("fracPdfList_"+name);

                vector<RooAbsPdf *> * myv = (vector<RooAbsPdf *> *)_base;
                for(unsigned e = 0; e < myv->size(); e++)
                {
                    RooRealVar * f  = new RooRealVar(("f_"+(string)((*myv)[e]->GetName())).c_str(),"f",0.1,0.,1.);
                    pdfList->add(*(*myv)[e]);
                    if(e>0)fracList->add(*f);
                }

                res = new RooAddPdf("totpdf_"+name,"totpdf_"+name,*pdfList,*fracList);
            }

            res->SetName(_name);
        }
        else if(t.find("TTree")!=string::npos)
        {	
            RooArgSet * vars = new RooArgSet(*myvar);
            if(opt.find("-v[")!=string::npos)
            {
                size_t pos = opt.find("-v[") + 2; 
                size_t posend = opt.find("]",pos);

                while(true)
                {
                    size_t pos2 = opt.find(",",pos+1);
                    TString vname = (TString)opt.substr(pos+1,pos2-pos-1);
                    if(pos2 >= posend) vname.ReplaceAll("]","");
                    vars->add(*(new RooRealVar(vname,vname,0.)));
                    if(pos2 < posend) pos = pos2;
                    else break;
                }
            }

            RooDataSet * sigDataSet = NULL;
            if(weight!="")
            {
                vars->add(*(new RooRealVar(weight.c_str(),weight.c_str(),0.)));
                sigDataSet = new RooDataSet((TString)_name+"_DataSet_"+name,"",(TTree*)_base,*vars,0,weight.c_str());
            }
            else sigDataSet = new RooDataSet((TString)_name+"_DataSet_"+name,"",(TTree*)_base,*vars);

            if(opt.find("-c[")!=string::npos)
            {
                size_t pos = opt.find("-c[") +3; 
                size_t posend = opt.find("]",pos);
                TString treecut = (TString)(opt.substr( pos, posend - pos));
                sigDataSet->Print();
                sigDataSet->reduce(treecut);
            }

            if(opt.find("-rho")!=string::npos) 
            {
                int pos = opt.find("-rho");
                string rhostr = opt.substr(pos+4,20);
                double rho = ((TString)rhostr).Atof();
                res = new RooKeysPdf((TString)_name,_name,*myvar,*sigDataSet,RooKeysPdf::MirrorBoth,rho);
            }
            else res = new RooKeysPdf((TString)_name,_name,*myvar,*sigDataSet,RooKeysPdf::MirrorBoth,2);

            if(opt.find("-print")!=string::npos)
            {
                TCanvas * c = new TCanvas();
                RooPlot * keysplot = myvar->frame();
                sigDataSet->plotOn(keysplot);
                res->plotOn(keysplot);
                keysplot->SetTitle(_name);
                keysplot->Draw();
                c->Print("rooKeysModel_"+((TString)_name).ReplaceAll("__noprint__","")+".pdf");
                delete c;
                delete keysplot;
            }
        }
        else if(t.find("TH1")!=string::npos)
        {
            RooDataHist * sigDataHist = new RooDataHist((TString)_name+"_DataHist"+name,"",*myvar,(TH1*)_base,6);
            res = new RooHistPdf((TString)_name,_name,*myvar,*sigDataHist);
        }
        else
        {
            if(((string)_name).find("_noprint")!=string::npos) cout << name << ": ATTANTION:_noprint option in background name: component won't be added to background list" << endl;
            else cout << name << ": ***** ATTANTION: Wrong type (" << t << ") given to getPdf. Only string, RooAbsPdf, vector<RooAbsPdf *>, TTree and TH1 are allowed! *****" << endl;
        }

        return res;
    }


    public:

    //Constructors

    ModelBuilder():
        isvalid(false), var(NULL), sig(NULL), bkg(NULL), totBkgMode(false), mycolors(vector<Color_t>())
    {
        nsig = new RooRealVar("nsig","nsig",0.,0.,1.e10);
        nbkg = new RooRealVar("nbkg","nbkg",0.,0.,1.e10);
        name = "model";
        title = name;
    }

    ModelBuilder(TString _name, RooRealVar * _var):
        isvalid(false), name(_name), var(_var), sig(NULL), bkg(NULL), totBkgMode(false), mycolors(vector<Color_t>())
    {
        if(var)
        {
            if( ((string)var->GetTitle()).find("__var__")==string::npos )
                var->SetTitle( (TString)var->GetTitle()+"__var__" );
            tmpvar = new RooRealVar(*var); vars.push_back(var); 
        }
        nsig = new RooRealVar("nsig"+name,"nsig",0.,0.,1.e10);
        nbkg = new RooRealVar("nbkg"+name,"nbkg",0.,0.,1.e10);
        title = name;
    }

    ~ModelBuilder()
    {
        delete var;
        delete model;
        delete sig;
        delete bkg;
        delete nsig;
        delete nbkg;

        for(unsigned i = 0; i < bkg_components.size(); i++)
        {
            delete bkg_components[i];
            delete bkg_fractions[i];
        }
    }



    //Methods

    /** \brief Builds the model:
     * Using RooAddPdf builds a model using all components in sig and background_compnents.
     * Options: 
     * <br> "-exp":     Automatically adds an exponential coponent to the background.
     * <br> "-noBkg" -> ignores bkg
     * */
    RooAbsPdf * Initialize(string optstr = "-exp");

    /** \brief Adds a bkg component.
     * Adds a bkg component named "_name" based on the object "_comp".
     * @param _frac: This can ve a RooRealVar/RooFormulaVar or a double. The Roo object are set as yields of the current component.
     * <br> In case it's a double a Roo object is created in the following way:
     * 	<br> - _frac > 1    creates a RooRealVar with "_frac" value (in the fit the yield will start from "_frac" and float)
     * 	<br> - |_frac| <= 1 created a RooFormulaVar connecting this background yield as a fraction "_frac" of the singla yield
     * 	<br> - _frac < -1   same as _frac > 1 but the number is fixed. Meaning that the yield of this bacgkround will not float in the fit
     *
     * @param opt: Options:
     * -> It is passed as options to getPdf()
     * <br> - "-ibegin":  Adds the bkg component in front and not at the back of the backgrounds list
     *
     * @param myvars:
     * -> It is passed as argument of getPdf()
     *  */
    template <class T> RooAbsPdf * AddBkgComponent(const char * _name, T * _comp, RooAbsReal * _frac, string opt = "", Str2VarMap myvars = Str2VarMap(), string weight = "")
    {
        TString nstr = "bkg_"+(TString)_name;
        string lowopt = opt;
        transform(lowopt.begin(), lowopt.end(), lowopt.begin(), ::tolower);
        RooAbsReal * frac = NULL;

        nstr+=("_"+name);

        if(opt.find("-frac")!=string::npos) {

            TString ss( (TString)nsig->GetName() + " * " + _frac->GetName() );
            frac = new RooFormulaVar("n"+nstr,"f_{"+(TString)_name+"}^{wrtsig}",ss,RooArgSet(*_frac,*nsig));
        }
        else { frac = _frac; }

        frac->SetName((TString)frac->GetName()+"_"+name);

        nstr += "__for_" + (TString)var->GetName();
        RooAbsPdf * comp = getPdf(_comp,nstr,myvars,weight,opt);	

        if(comp!=NULL && _frac != NULL && lowopt.find("-nofit")==string::npos)
        {
            if(lowopt.find("-ibegin")==string::npos)
            {
                bkg_components.push_back(comp);
                bkg_fractions.push_back(frac);
                SetLastBkgColor(GetDefaultColors()[bkg_components.size()-1]);
            }
            else
            {
                bkg_components.insert(bkg_components.begin(),comp);
                bkg_fractions.insert(bkg_fractions.begin(),frac);
                mycolors.insert(mycolors.begin(),GetDefaultColors()[bkg_components.size()-1]);
            }
        }

        return comp;
    }

    template <class T> RooAbsPdf * AddBkgComponent(const char * _name, T * _comp, double _frac = 0, string opt = "", Str2VarMap myvars = Str2VarMap(), string weight = "")
    {
        TString nstr = "bkg_"+(TString)_name;
        RooAbsReal * frac = NULL;
        double val = TMath::Abs(_frac);
        if(val == 0) val = 1e3;
        double min = 0;
        double max = 1.e7;

        if(_frac < 0) { min = val; max = val; }

        if(totBkgMode) 
        {
            if(val > 1) { cout << "Attention in 'totBkgMode' the nevt must be between 0 and 1" << endl; return NULL; }
            frac = new RooRealVar("f"+nstr,"f_{"+(TString)_name+"}",val,0,1);
        }
        else if((TMath::Abs(_frac) > 0 && TMath::Abs(_frac) <= 1) || opt.find("-frac")!=string::npos)
        {
            TString ss( (TString)nsig->GetName() + Form(" * %e", val) );
            if(!nsig) { cout << "Attention if you use this option abs(nevt) < 1 you must set the signal first." << endl; return NULL; }
            frac = new RooFormulaVar("n"+nstr,"f_{"+(TString)_name+"}^{wrtsig}",ss,*nsig);
        }
        else frac = new RooRealVar("n"+nstr,"N_{"+(TString)_name+"}",val,min,max);

        return AddBkgComponent(_name, _comp, frac, opt, myvars, weight);
    }

    template <class T> RooAbsPdf * AddBkgComponent(const char * _name, T * _comp, RooAbsReal * _frac, string opt, string weight, Str2VarMap myvars = Str2VarMap())
    {
        return AddBkgComponent(_name, _comp, _frac, opt, myvars, weight);
    }

    template <class T> RooAbsPdf * AddBkgComponent(const char * _name, T * _comp, double _frac, string opt, string weight, Str2VarMap myvars = Str2VarMap())
    {
        return AddBkgComponent(_name, _comp, _frac, opt, myvars, weight);
    }

    template <class T> RooAbsPdf * SetExtraBkgDimension(const char * _name, T * _pdf, RooRealVar * extravar, string opt = "", Str2VarMap myvars = Str2VarMap(), string weight = "")
    {
        int ind = -1;
        for (int bb = 0; bb < bkg_components.size(); bb++)
        {
            if( ((string)bkg_components[bb]->GetName()).find(_name) != string::npos )
            {
                ind = bb;
                break;
            }
        }

        if(ind==-1) { cout << "You must set a background component with the specified name using AddBkgComponent() first" << endl; return NULL; }

        RooAbsPdf * old_bkg = bkg_components[ind];

        TString pdfname   = ((TString)old_bkg->GetName()).ReplaceAll("__noprint__",""); 
        size_t posfor = ((string)pdfname).find("__for");
        TString name_comp = ((string)pdfname).substr(0,posfor)+"__for_"+(TString)extravar->GetName()+"__noprint__";
        TString name_tot  = pdfname+"__and_"+(TString)extravar->GetName();
        old_bkg->SetName(pdfname+"__noprint__");

        RooAbsPdf * new_comp = getPdf(_pdf, name_comp, myvars, weight, opt, extravar);
        bkg_components[ind] = new RooProdPdf("prod","",*old_bkg,*new_comp);

        bkg_components[ind]->SetName(name_tot);
        bkg_components[ind]->SetTitle(name_tot);

        return bkg_components[ind];
    }

    template <class T> RooAbsPdf * SetExtraBkgDimension(const char * _name, T * _pdf, RooRealVar * extravar, string opt, string weight, Str2VarMap myvars = Str2VarMap())
    {
        return SetExtraBkgDimension(_name, _pdf, extravar, opt, myvars, weight);
    }


    /** \brief Adds a blacgound component
     * The behaviour it's similar to AddBkgComponent().
     * The only difference is the nsig, is set as signal yield. If given as a double it can be only > 1 (starting value)
     * or < -1 (starting value fixed in the fit).
     * */

    template <class T> RooAbsPdf * SetSignal(T * _sig, RooAbsReal * _nsig, string opt = "", Str2VarMap myvars = Str2VarMap(), string weight = "")
    {
        string lowopt = opt;
        transform(lowopt.begin(), lowopt.end(), lowopt.begin(), ::tolower);

        ResetVariable();
        TString myname = "_" + name;
        nsig = _nsig;
        nsig->SetName((TString)nsig->GetName()+myname);
        myname += "__for_" + (TString)var->GetName();
        if(_sig) 
        {
            sig = getPdf(_sig,"sig"+myname, myvars, weight, opt);
            sig->SetName("totsig"+myname);
            sig->SetTitle("totsig"+myname);
        }
        return sig;
    }

    template <class T> RooAbsPdf * SetSignal(T * _sig, double _nsig = 0, string opt = "", Str2VarMap myvars = Str2VarMap(), string weight = "")
    {
        RooAbsReal * tmpnsig = NULL;
        double val = TMath::Abs(_nsig);
        if(val == 0) val = 1e3;
        double min = 0;
        double max = 1.e7;
        if(_nsig < 0) { min = val; max = val; }

        if(TMath::Abs(_nsig) > 0 && TMath::Abs(_nsig) <= 1) tmpnsig = new RooRealVar("nsig","N_{sig}",val*max,min,max);
        else tmpnsig = new RooRealVar("nsig","N_{sig}",val,min,max);

        return SetSignal(_sig,tmpnsig,opt,myvars, weight);
    }

    template <class T> RooAbsPdf * SetSignal(T * _sig, RooAbsReal * _nsig, string opt, string weight, Str2VarMap myvars = Str2VarMap())
    {
        return SetSignal(_sig,_nsig,opt,myvars, weight);
    }

    template <class T> RooAbsPdf * SetSignal(T * _sig, double _nsig, string opt, string weight, Str2VarMap myvars = Str2VarMap())
    {
        return SetSignal(_sig,_nsig,opt,myvars, weight);
    }

    template <class T> RooAbsPdf * SetExtraSignalDimension(T * _sig, RooRealVar * extravar, string opt = "", Str2VarMap myvars = Str2VarMap(), string weight = "")
    {
        if(!sig) { cout << "You must set the signal using SetSignal() first" << endl; return NULL; }

        RooAbsPdf * old_sig = sig;
        TString pdfname = ((TString)sig->GetName()).ReplaceAll("__noprint__","").ReplaceAll("totsig","sig");
        size_t posfor = ((string)pdfname).find("__for");
        TString name_comp = ((string)pdfname).substr(0,posfor)+"__for_"+(TString)extravar->GetName()+"__noprint__";
        TString name_tot = pdfname+"__and_"+(TString)extravar->GetName();
        sig->SetName(pdfname+"__noprint__");

        RooAbsPdf * new_comp = getPdf(_sig, name_comp, myvars, weight, opt, extravar);

        sig = new RooProdPdf("prod","",*old_sig,*new_comp);
        sig->SetName(name_tot);
        sig->SetTitle(name_tot);

        return sig;
    }

    template <class T> RooAbsPdf * SetExtraSignalDimension(T * _sig, RooRealVar * extravar, string opt, string weight, Str2VarMap myvars = Str2VarMap())
    {
        return SetExtraSignalDimension(_sig, extravar, opt, myvars, weight);
    }
 
    void SetLastBkgColor(Color_t color) 
    {
        if(mycolors.size() == bkg_components.size()) mycolors[mycolors.size()-1] = color;
        else mycolors.push_back(color); 
    }
    vector <Color_t> GetColors() { return mycolors; }

    void SetTitle(TString _title) { title = _title; }

    /** \brief Sets the variable to fit "var".
     * It also creates a copy of the initial variable "tmpvar". In fact when fitting the varible is modified and
     * if you want to fit again or change the model and refit you need to reset the variable as it was.
     * */
    void SetVariable(RooRealVar * _var) { var = _var; tmpvar = new RooRealVar(*var); }

    /** \brief Sets the background mode
     * Normally the model is built as model = nsig*sig + nbkg1*bkg1 + nbkg2*bkg2 * ...
     * If the totBkgModel is set to "tot" the model is built as model = nsig*sig + nTotbkg*(fracBkg1*bkg1 + fracBkg2*bkg2 + ...)
     * */
    void SetBkgMode( bool mode ) { totBkgMode = mode; }
    void SetSig(RooAbsPdf * _sig) { sig = _sig; }
    ///\brief Forces a model
    void SetModel(RooAbsPdf * _model);
    void SetNSig(RooAbsReal * _nsig) { nsig = _nsig; }
    void SetBkg(RooAbsPdf * _bkg) { bkg = _bkg; }
    void SetBkg(vector<RooAbsPdf *> _bkg_comp) { bkg_components = _bkg_comp; }
    void SetName(const char * newname) { name = newname; }
    void ClearBkgList() { bkg_components.clear(); bkg_fractions.clear(); }
    void ResetVariable() { var->setVal(tmpvar->getVal()); var->setRange(tmpvar->getMin(),tmpvar->getMax()); };

    ////\brief Return true if the model was correctly built and initialized
    bool isValid() { return isvalid; }
    //bool isExtended() { return extended; }
    TString GetName() { return name; }
    ///\brief Return the variable to fit
    RooRealVar * GetVariable() { return var; }
    ///\brief Returns the model pdf
    RooAbsPdf * GetModel() { return model; }
    ///\brief Returns the signal pdf
    RooAbsPdf * GetSig() { return sig; }
    ///\brief Returns a pdf corresponding to the sum of all bkg PDFs
    RooAbsPdf * GetTotBkg() { return bkg; }
    ///\brief Returns the number of bkg events integrating the bkg pdf in [min,max] (Returns nbkg * (int [min,max] of bkg) )
    /// If "fitRes" is passed stores the error in "valerr"
    double GetNBkgVal(double min = 0, double max = 0, double * valerr = NULL, RooFitResult * fitRes = NULL);
    ///\brief Returns the number of sig events integrating the sig pdf in [min,max] (Returns nsig * (int [min,max] of sig) )
    /// If "fitRes" is passed stores the error in "valerr"
    double GetNSigVal(double min = 0, double max = 0, double * valerr = NULL, RooFitResult * fitRes = NULL, double fmin = 0, double fmax = 0);
    ///\brief Return S/B integrating sig and bkg in [min,max]
    double GetSOverB(float min, float max, double * valerr = NULL, RooFitResult * fitRes = NULL);
    ///\brief Return S/(S+B) integrating sig and bkg in [min,max]
    double GetSigFraction(float min, float max, double * valerr = NULL, RooFitResult * fitRes = NULL);
    ///\brief Returns the full list of bkg PDFs
    vector<RooAbsPdf *> GetBkgComponents() { return bkg_components; }
    ///\brief Prints to screen the composition e.g signal = NSIG / NTOT, bkg1 = NBKG1/NTOT, etc
    void PrintComposition(float min = 0., float max = 0., RooFitResult * fitRes = NULL);
    ///\brief Returns the full list of bkg yields variables
    vector<RooAbsReal *> GetBkgFractions() { return bkg_fractions; }
    ///\brief Returns the number of sig events in the full range. Same as GetNSigVal(0,0)
    double GetSigVal(double * valerr = NULL, RooFitResult * fitRes = NULL);
    ///\brief Returns the number of sig events in the full range. And can also return its asymmetric error
    double GetSigVal(double * errHi, double * errLo); 
    bool CheckModel() { return checkModel(model); };

    /** \brief Returns the number of total bkg evens
     * Returns a RooAbsReal variable which is a RooFormulaVar built as the sum of all bkg yields.
     */
    RooAbsReal * GetTotNBkg();
    RooAbsReal * GetNSigPtr() { return nsig; }
    ///\brief Returns a Str2VarMap object containing all paramters only of the signal PDF 
    Str2VarMap GetSigParams(string opt = "");
    ///\brief Returns a Str2VarMap object containing all paramters of the entire model PDF, inclusing yields
    Str2VarMap GetParams(string opt = "");

    static void SetPrintLevel(string mode) { pmode = mode; }

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
            vector<string> regStr = vector<string>(), double * range = NULL, RooFitResult * fitRes = NULL, 
            TString Ytitle = "", RooRealVar * myvar = NULL);
    void Print(TString title = "", TString Xtitle = "", string opt = "", RooAbsData* data = NULL, int bins = 50, 
            RooFitResult * fitRes = NULL, TString Ytitle = "", vector<RooRealVar *> myvar = vector<RooRealVar *>());


    ///\brief Prints all the paramters to screen in RooFit format (opt="-nocost" skips constants)
    void PrintParams(string opt = "") { PrintPars(GetParams("-orignames"), opt); }
    ///\brief Prints the sgnal pdf paramters to screen in RooFit format (opt="-nocost" skips constants)
    void PrintSigParams(string opt = "") { PrintPars(GetSigParams("-orignames"), opt); }
    ///\brief Prints all the paramters to screen in latex table format (opt="-nocost" skips constants)
    void PrintParamsTable(string opt = "") { PrintPars(GetParams("-orignames"),"-latex"+opt); }
    ///\brief Prints all sigal PDF paramters to screen in latex table format (opt="-nocost" skips constants)
    void PrintSigParamsTable(string opt = "") { PrintPars(GetSigParams("-orignames"),"-latex"+opt); }

    ///\brief Returns the value of S(x)/(S(x)+B(x)) for x = value. It corresponds to a naive S-weight.
    float GetReducedSWeight(float value);

};


#endif

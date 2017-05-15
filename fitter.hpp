#ifndef FITTER_HPP
#define FITTER_HPP

#include "general_functions.hpp"
#include "analyser.hpp"
#include "multi_analyser.hpp"
#include <cstdlib>
#include <string>
#include <vector>
typedef vector<double>  binned; 

class Fitter {

    public:
    //Constructors and destructor    
            Fitter(const string& inf, const string& intr, const string& inMC, const string& inMCtr);
            Fitter(const string& inf, const string& intr);
            
            ~Fitter();
    //Inizializer for the fit, the constructor sets the files, initialize sets the fit options (could have been done at once but would be hard to read)        
            void Initialize(const string mod, const string mcCut, const string datCut , const string var,const binned bin,const string fName,const string label,const string cutID,const string PorSL ,string unit="MeV/#it{c}^{2}", int chebOrder=1 );
            void Initialize(const string mod,  const TCut datCut , const string var,const binned bin,const string fName,const string label,const string cutID,const string PorSL ,const double min_blin=0,const double max_blind=0,string unit="MeV/#it{c}^{2}", int chebOrder=1 );
            void Initialize(const string mod,  const string datCut , const string var,const binned bin,const string fName,const string label,const string cutID,const string PorSL,double min_blin=0, double max_blind=0 ,string unit="MeV/#it{c}^{2}", int chebOrder=1 );
            void Initialize(const string mod, const TCut mcCut, const TCut datCut , const string var,const binned bin,const string fName,const string label,const string cutID,const string PorSL,string unit="MeV/#it{c}^{2}", int chebOrder=1 );
            
    // Fit function, the one to use and pass the parameters in         
            void Fit(const string& option);
    // MC and data Analysis initialyzer        
            void initializeMC(const string& mcModel, const string& mcCut,const string& fitName, const string& label,string units="MeV/#it{c}^{2}");            
            void initializeMC(const string& mcModel, TCut& mcCut,const string& fitName, const string& label, string units="MeV/#it{c}^{2}");
            void initializeData(const string& dataModel,const string& bkgModel, TCut& dataCut,const string& fitName, const string& label, string units="MeV/#it{c}^{2}");
            void initializeData(const string& dataModel, TCut& dataCut,const string& fitName, const string& label, string units="MeV/#it{c}^{2}");
            void initializeData(const string& dataModel,const string& bkgModel, const string& dataCut,const string& fitName, const string& label, string units="MeV/#it{c}^{2}");
    // MC and Data Analysis fits        
            void fitMC(const string& fitopt,bool docuts=true, bool unbinned=false, int binning=90);
            void fitData(const string& fitopt,bool docuts=true, bool unbinned=false, int binning=90);
    //Fix the Str2VarMap variables for dataFit
            void fixMCparsVar(const string var); //one at a time
            void fixFitVars(); // all depending on model Johnson or Ipatia
    //Generate options
            string generateFitOpt(bool mc, bool quiet=true);            
            string generateModelParam(const string model, bool mc);
            string generateChebyshevBkg(int order);
    // create RooRealVar from var to fit and binning
            void initializeVar(const string& var,const binned& bn);
    //Writing variables (called in destructor if bool write =true        
            void write_variables();
            void write_Str2VarMap();
    //Read Str2VarMap 
            void read_Str2VarMap();
    //Setter        
            void setMCCut(const TCut& c); // useless
            void setFitMC(bool fit);
    
            void setBlindRegion(const double min,const double max);
    //Fits routine to be performed depending on options of Fit()
            void fit_pPID();
            void fit_raw();
            void fit_ROC();
            void fit_sideband();
            void fit_phi();
            void fit_Lc2pmm();
    private:
            Analysis* anaMC;
            Analysis* ana;
            TFile* tmpout;
            RooRealVar* vM;
            RooFitResult* fitResult;
            Str2VarMap MCpars;
            RooWorkspace* ws;
            TCut MCcut;
            TCut dataCut; 
            string model; // Johnson or Ipatia
            string infile;
            string inMCfile;
            string intree;
            string inMCtree; 
            string fitName; //name of the .pdf saved
            string labelName; // legend name
            string units;
            int orderCheb;
            bool write; // Write the result of the fit 
            string type;
            string id;  //To be added to the /dat/type/variable+id.dat 
            bool fitmc;
            string mode;
            bool blinded;
            double min_blind_;
            double max_blind_;
};



#endif

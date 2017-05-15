#include "fitter.hpp"
using namespace std;

//Constructor + Destructor
Fitter::Fitter(const string& inf, const string& intr, const string& inMCf, const string& inMCt)
    : infile(inf), intree(intr), inMCfile(inMCf), inMCtree(inMCt)  { fitmc=true; blinded=false;}
Fitter::Fitter(const string& inf, const string& intr)
    : infile(inf), intree(intr) { fitmc=true; blinded=false; }
Fitter::~Fitter(){
    if (write) {write_variables();}
 /*   delete anaMC;
    delete ana;
    delete vM;
    delete fitResult;*/
    tmpout->Write();
    tmpout->Close();
//    delete tmpout;
}
// Methods
void Fitter::initializeMC(const string& mcModel, TCut& mcCut,const string& fitName, const string& label, string units){
    anaMC = new Analysis(mode+type+model+fitName+"_MC", label, inMCtree, inMCfile, vM, &mcCut);
    anaMC->SetUnits(units);
    anaMC->SetSignal(mcModel.c_str());}

void Fitter::initializeMC(const std::string& mcModel, const std::string& mcCut,const std::string& fitName, const std::string& label,string units){
    TCut mc = TCut(mcCut.c_str());
    initializeMC(mcModel,mc,fitName,label,units);}

void Fitter::initializeData(const std::string& dataModel,const std::string& bkgModel, TCut& dataCut,const std::string& fitName, const std::string& label, string units){
    ana = new Analysis(mode+type+model+fitName+id, label, intree,infile,vM,&dataCut);
    ana->SetUnits(units);
    ana->AddAllVariables();
    ana->SetSignal(dataModel.c_str(),5.e4,"",MCpars);
    ana->AddBkgComponent("Bkg",bkgModel.c_str());
}
 void Fitter::initializeData(const std::string& dataModel,const std::string& bkgModel, const string& dataCut,const std::string& fitName, const std::string& label, string units){
    TCut data = TCut(dataCut.c_str());
    initializeData(dataModel,bkgModel,dataCut,fitName,label,units);}

void Fitter::initializeData(const string& dataModel, TCut& dataCut,const string& fitName, const string& label, string units) {
    ana = new Analysis (mode+type+model+fitName+id,label,intree,infile,vM,&dataCut);
    ana->SetUnits(units);
    ana->AddAllVariables();
    ana->SetSignal(dataModel.c_str());
}


void Fitter::fitMC(const string& fitopt,bool docuts ,bool unbinned, int binning){
    tmpout->cd();
    if (docuts) {anaMC->Initialize("-docuts");}
    else {anaMC->Initialize();}
    anaMC->Fit(binning,unbinned,fitopt);
    MCpars = anaMC->GetSigParams();}
void Fitter::fitData(const string& fitopt, bool docuts ,bool unbinned, int binning){
    tmpout->cd();
    if (docuts) {ana->Initialize("-docuts");}
    else { ana->Initialize();}
    ana->Fit(binning,unbinned,fitopt);
    fitResult = ana->GetFitRes();}

void Fitter::fixMCparsVar(const string var){
    ((RooRealVar*)MCpars[var])->setConstant();
}
void Fitter::fixFitVars(){
    if (model=="Johnson") { fixMCparsVar("nu"); 
                            fixMCparsVar("tau");}
    else if(model=="Ipatia") {
                            fixMCparsVar("n");
                            fixMCparsVar("a");
                            fixMCparsVar("z");
                            fixMCparsVar("b");
                            fixMCparsVar("l");}
    else {  cout << "Available models : Johnson / Ipatia, please choose between those models! " <<endl;
            exit(0);}}
string Fitter::generateFitOpt(bool mc, bool quiet){
    string fitopt="-chi2-lin-andpulls-nocost-xM(pK#pi)-attach-layout[0.6,0.6,0.9,0.9]";
    fitopt += "-font0.03-leg[0.15,0.7,0.35,0.9]";
    if (quiet) { fitopt+="-quiet"; }
    if (mc) {fitopt+"-noleg-noextended";}
    return fitopt;
}
string Fitter::generateModelParam(const string model, bool mc){
    string ret=model;
    cout << ret << endl;
    if (model=="Johnson"&& mc) {ret += "-tau[0.7,0.,5.]-s[6.,2.,20.]-nu[-4.,-10.,10.]-m[2285.,2270.,2310.]";}
    else if (model=="Ipatia" && mc) { ret += "-n[2,0,5]-a[1.5,-5.,5.]-z[1.,0.1,10]-l[-0.5,-10,.]-s[6.,2.,20.]-b[0.01,-10.,10.]-m[2285.,2270.,2310.]";}
//    else {  cout << "Available models : Johnson / Ipatia, please choose between those models! " <<endl;
       //     return 0;}
    return ret;
}

string Fitter::generateChebyshevBkg(int order){
    string ret = "Cheb1";
    switch (order){
        case 1:
        ret += "-v1[0.,-0.4,0.4]";
        break;
        case 2:
        ret +=  "-v1[0.,-0.4,0.4]-v2[0.,-0.4,0.4]";
        break;
        case 3:
        ret += "-v1[0.,-0.4,0.4]-v2[0.,-0.4,0.4]-v3[0.,-0.4,0.4]";
        default:
        cout << "Orders between 1-2-3, system exited!"<< endl;
        exit(0);}
    return ret;
}


void Fitter::fit_pPID(){
    type = "PID"; write=true;
    string modelMC=generateModelParam(model,true);
    string fitoptMC=generateFitOpt(true,false);
    string modelData=generateModelParam(model,false);
    string fitopt=generateFitOpt(false,true);
    string bkgModel=generateChebyshevBkg(orderCheb);

    initializeMC(modelMC, MCcut,fitName,labelName,units);
    
    if (fitmc) {
                fitMC(fitoptMC);
                write_Str2VarMap();}
    else        {read_Str2VarMap();}    
    
    fixFitVars();
    initializeData(modelData,bkgModel,dataCut,fitName,labelName,units);
    fitData(fitopt);
    fitResult = ana->GetFitRes();
}

void Fitter::initializeVar(const string& var, const binned& bn){ vM=new RooRealVar(var.c_str(),var.c_str(),bn[0],bn[1],bn[2]);}
    

void Fitter::Initialize(const string mod, const TCut mcCut, const TCut datCut , const string var, const binned bin,const string fName,const string label,const string cutID,const string PorSL ,string unit, int chebOrder ){
            model=mod;
            MCcut=mcCut;
            dataCut=datCut;
            initializeVar(var,bin);
            fitName=fName;
            labelName=label;
            id=cutID;
            units=unit;
            orderCheb=chebOrder;
            mode=PorSL;
            tmpout = new TFile(("/eos/lhcb/user/m/mstamenk/rootFiles/"+mode+type+model+id+"tmp.root").c_str(),"recreate");
}
void Fitter::Initialize(const string mod, const string mcCut, const string datCut , const string var,const binned bin,const string fName,const string label,const string cutID, const string PorSL,string unit, int chebOrder ){
    Initialize(mod,TCut(mcCut.c_str()),TCut(datCut.c_str()),var,bin,fName,label,cutID,PorSL,unit,chebOrder);}
 
void Fitter::write_variables(){
    string path = "dat/fitter/"+type+"/"+mode+"variables"+id+".dat";    
    //Compute lower and higher border for mu-2*sigma : mu+2*sigma
    cout << "Before MCpars acces " << endl;
    double mu; double sig; bool NBkg=true;
    if (blinded){ //No access to the data when blinded
            mu=(min_blind_+max_blind_)/2.;
            sig=(max_blind_-min_blind_)/4.;
            NBkg = false;
                
    }
    else{   mu=MCpars["m"]->getVal();
            sig=MCpars["s"]->getVal();
            NBkg = true;}
    double lower_limit  = mu-2*sig;
    double higher_limit = mu+2*sig;
    cout << "After MCpars access " << endl;
    // Write variables
    ofstream out;
    out.open(path.c_str());
    cout << path << endl;
    cout <<"Result Cut NSig NBkg low high mu sig covQual edm :"<< id << " " << ana->GetNSigVal(lower_limit,higher_limit)<<" ";
    if (NBkg) {cout<< ana->GetNBkgVal(lower_limit,higher_limit)<<" ";}
    cout<<lower_limit << " " <<higher_limit <<" "<<mu << " " << sig << " " <<fitResult->covQual() << " "<< fitResult->edm() <<endl ;
    
    if (type=="PID" or type=="PHI" or type=="SIDEBAND") { out << id << " " ;}
    
    out << ana->GetNSigVal(lower_limit,higher_limit)<<" ";
    if (NBkg){out<< ana->GetNBkgVal(lower_limit,higher_limit) <<" ";}
    out<<lower_limit << " " <<higher_limit <<" "<<mu << " " << sig << " " <<fitResult->covQual() << " "<< fitResult->edm() ;
;
    out.close();
     
}
void Fitter::fit_raw(){
    type = "RAW"; write=true;
    string modelMC=generateModelParam(model,true);
    string fitoptMC=generateFitOpt(true,false); // MC, No Quiet
    string modelData=generateModelParam(model,false);
    string fitopt=generateFitOpt(false,true); // No MC, Quiet
    string bkgModel=generateChebyshevBkg(orderCheb);
    initializeMC(modelMC, MCcut,fitName,labelName,units);
    fitMC(fitoptMC);    
    fixFitVars();
    initializeData(modelData,bkgModel,dataCut,fitName,labelName,units);
    fitData(fitopt);
    fitResult = ana->GetFitRes();
}

void Fitter::fit_sideband(){
    type = "SIDEBAND"; write = true;
    string modelData=generateChebyshevBkg(orderCheb);
    string fitopt = generateFitOpt(false,false); //No MC, No quiet
    initializeData(modelData,dataCut,fitName,labelName,units);
    if((min_blind_>0 || max_blind_>0) && min_blind_ < max_blind_) { ana->SetBlindRegion(min_blind_,max_blind_);}
    fitData(fitopt);
    MCpars = ana->GetSigParams();

}
void Fitter::fit_phi(){
    type = "PHI"; write=true;
    string modelMC=generateModelParam(model,true);
    string fitoptMC=generateFitOpt(true,false);
    string modelData=generateModelParam(model,false);
    string fitopt=generateFitOpt(false,true);
    string bkgModel=generateChebyshevBkg(orderCheb);

    initializeMC(modelMC, MCcut,fitName,labelName,units);
    
    if (fitmc) {
                fitMC(fitoptMC);
                write_Str2VarMap();}
    else        {read_Str2VarMap();}    
    
    fixFitVars();
    initializeData(modelData,bkgModel,dataCut,fitName,labelName,units);
    fitData(fitopt);
    fitResult = ana->GetFitRes();
}
void Fitter::fit_Lc2pmm(){
    type="RARE"; write=false;
    string modelMC=generateModelParam(model,true);
    string fitoptMC=generateFitOpt(true,false);
    string modelData=generateModelParam(model,false);
    string fitopt=generateFitOpt(false,true);
    string bkgModel=generateChebyshevBkg(orderCheb);
    initializeMC(modelMC,MCcut,fitName,labelName,units);
    fitMC(fitoptMC);
    fixFitVars();
    //Blinded analysis
    ana->SetBlindRegion(min_blind_, max_blind_);
    
    initializeData(modelData,bkgModel,dataCut,fitName,labelName,units);
    fitData(fitopt);
    fitResult = ana->GetFitRes();    

}

void Fitter::Fit(const string& option){
           
    if (option=="RAW" || option=="raw") fit_raw();
    else if (option=="p_PID" || option=="PID") fit_pPID();
//    else if ( (option=="sideband") || (option=="SIDEBAND") || (option=="side") ) fit_sideband();
    else if (option=="sideband" or option=="SIDEBAND" or option=="side") fit_sideband();
    else if (option=="phi" or option=="PHI") fit_phi();
    else if (option=="rare" or option=="RARE") fit_Lc2pmm();
    else cout << "Please use Fit() with the following options : RAW , p_PID , SIDEBAND, PHI " << endl;
    
 }

void Fitter::setMCCut(const TCut& c){ MCcut +=c;}

void Fitter::write_Str2VarMap() {
    string name = mode + type + model + "MCparsVariables";
    ws = new RooWorkspace(name.c_str());
    string dat ="ressource/"+name+".dat";
    string root = "ressource/"+name+".root";
    cout << "Writing to file : " << dat << endl;
    ofstream out;
    out.open(dat.c_str());
    for (auto el: MCpars) {ws->import(*(el.second));
                           out << el.second->GetName() << endl;}
    ws->writeToFile(root.c_str());
    cout << "Writing finished ! " << endl;
    out.close();
    

}
void Fitter::read_Str2VarMap(){
    MCpars = Str2VarMap();
    string name =mode+ type+model+"MCparsVariables";
    string dat = "ressource/" + name+".dat";
    string root = "ressource/" + name+".root";
    cout << "Reading from file : "<<name.c_str() <<" "<<dat << endl; 

    TFile f(root.c_str());
    
    ws = (RooWorkspace*)f.Get(name.c_str());
    
    ifstream in;
    in.open(dat.c_str());
    if(in.is_open()){
    string line;

    while(getline(in,line)){
        string str=line.substr(0,line.find("_"));
        cout<< "Reading Line : "  << line.c_str() << endl;
        RooAbsReal* roo_obj=(RooAbsReal*)ws->obj(line.c_str());
        MCpars.insert(pair<string, RooAbsReal*>(str,roo_obj));
        cout<<"val : "<< str<<" "<< MCpars[str]->getVal() << endl;
    }}
    else {cout << "Couldn't open file " << dat.c_str() <<endl;}
    
    in.close();
    cout << "Reading finished ! " << endl;
}
void Fitter::setFitMC(bool fit){ fitmc=fit;}

void Fitter::setBlindRegion(const double min,const double max) {
    blinded=true;
    min_blind_=min;
    max_blind_=max;
}
void Fitter::Initialize(const string mod,  const TCut datCut , const string var,const binned bin,const string fName,const string label,const string cutID,const string PorSL ,const double min_blind,const double max_blind,string unit, int chebOrder ) {
            model=mod;
            dataCut=datCut;
            initializeVar(var,bin);
            fitName=fName;
            labelName=label;
            id=cutID;
            units=unit;
            orderCheb=chebOrder;
            mode=PorSL;
            min_blind_=min_blind;
            max_blind_=max_blind;
            tmpout = new TFile(("/eos/lhcb/user/m/mstamenk/rootFiles/"+mode+type+model+id+"tmp.root").c_str(),"recreate");
}

void Fitter::Initialize(const string mod,  const string datCut , const string var,const binned bin,const string fName,const string label,const string cutID,const string PorSL,double min_blind, double max_blind ,string unit, int chebOrder ){ 
            Initialize(mod,TCut(datCut.c_str()),var,bin,fName,label,cutID,PorSL,min_blind,max_blind,unit,chebOrder);
}


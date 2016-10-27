#include "analyser.hpp"
#include <algorithm>

using namespace RooFit;
using namespace RooStats;


RooRealVar * Scaler::_var = NULL;
double Scaler::_scale = 1.;

string Analysis::pmode = "v";

void Analysis::AddAllVariables()
{
	vector<variable * > myvars;
	if(dataReader) myvars = dataReader->GetVarList();
	else if(reducedTree)
	{
		TreeReader * reader = new TreeReader(reducedTree);
		reader->Initialize();
		myvars = reader->GetVarList();
	}

	for(auto v : myvars)
	{
		if(v->GetArraySize() > 1) continue;
		if(v->name!=var->GetName()) AddVariable(v->name);
	}
}

/*
   Function to unitialize the Analysis object before fitting
   */


bool Analysis::Initialize(string option, double frac)
{
    transform(option.begin(), option.end(), option.begin(), ::tolower);
    if (pmode == "v") cout << endl << name << ": Initialize " << option << endl;

    chi2[0] = chi2[1] = -1;

    vector<double> allregions;
    allregions.push_back(var->getMin());
    for(unsigned r = 0; r < regions.size(); ++r)
        allregions.push_back(regions[r]);
    allregions.push_back(var->getMax());
    if(allregions.size()>2)
    {	
        for(unsigned r = 1; r < allregions.size(); ++r)
        {
            string bandname = Form("band_%i",r/2);
            if (r % 2 == 0) bandname = Form("sig_%i",r/2);
            var->setRange(bandname.c_str(), allregions[r-1], allregions[r]);
            regStr.push_back(bandname);
        }
    }

    if (dataReader)
    {
        CreateReducedTree(option, frac);
        CreateDataSet();
    }

    if (data) init = true;
    else if (!reducedTree) cout << "WARNING: No data available!!" << endl;

    bool result = ModelBuilder::Initialize(option);
    if(bkg_components.size()==0) ((RooRealVar*)nsig)->setVal(data->numEntries());
    if (pmode == "v")
    {
        cout << endl << name << ": PrintParams" << endl << endl;
	ModelBuilder::PrintParams(option);
    }
    return result;
}



void Analysis::CreateReducedTree(string option, double frac, TCut mycuts)
{
    transform(option.begin(), option.end(), option.begin(), ::tolower);
    if(pmode == "v") cout << endl << name << ": CreateReducedTree " << option << endl;

    TCut doCuts = "";
    if (cuts)
        doCuts = *cuts;
    if (mycuts != "")
        doCuts += mycuts;

    if (!dataReader->isValid())
        dataReader->Initialize();

    if ((doCuts != "") && (option.find("-docuts") != string::npos))
        reducedTree = (TTree*) dataReader->CopyTree(doCuts, frac, (string) ("reduced_" + name));
    else if (!reducedTree)
        reducedTree = (TTree*) dataReader->GetChain()->Clone("reduced_" + name);

    if( scale!=1 && !dataReader->HasVar( ((string)var->GetName()+"_unscaled").c_str() ) )
    {
        if(pmode == "v") cout << "Scaling variable... " << endl;
        string oldpmode = TreeReader::GetPrintLevel();
        TreeReader::SetPrintLevel("s");
        Scaler::Set(scale,var);
        applyFunc(&Scaler::Scale);
        TreeReader::SetPrintLevel(oldpmode);
    }

    return;
}


/*
   This function converts the information in the Analysis object in a RooDataHist which can be fitted
   */ 

RooDataSet * Analysis::CreateDataSet(string option, TCut mycuts)
{
    transform(option.begin(), option.end(), option.begin(), ::tolower);
    if (pmode == "v") cout << endl << name << ": CreateDataSet " << option << endl;

    if (reducedTree && (option.find("-forcehist") == string::npos))
    {
        if(mycuts!="" || option.find("-docuts")!=string::npos) CreateReducedTree("-docuts",-1,mycuts);

        RooArgList varList("varList_" + name);
        for (unsigned i = 0; i < vars.size(); ++i)
            varList.add(*(vars[i]));

        if (pmode == "v") cout << "Candidates: " << reducedTree->GetEntries() << endl;
        if (dataReader && !dataReader->HasVar(var->GetName()))
            cout << "WARNING: the tree does not contain the requested variable!" << endl;

        if(data) data->reset();
        if (weight)
        {
            varList.add(*weight);
            data = new RooDataSet("data_" + name, "data" + name, varList, Import(*reducedTree), WeightVar(weight->GetName()));
        }
        else data = new RooDataSet("data_" + name, "data" + name, reducedTree, varList);

        CreateHisto("-usedataset");

        if (pmode == "v") data->Print();
    }
    else if (dataHist)
    {
        RooDataHist *htmp   = new RooDataHist("data" + name, "", *var, dataHist);
        RooRealVar  *w      = new RooRealVar("w" + name, "", 1., 0., 1.e6);
        RooArgSet   *ArgSet = new RooArgSet("args");
        ArgSet->add(*var);
        ArgSet->add(*w);
        RooDataSet  *tmp    = new RooDataSet("data" + name, "", *ArgSet, "w" + name);

        for (int i = 0; i < htmp->numEntries(); ++i)
        {
            htmp->get(i);
            tmp->add(*htmp->get(i), htmp->weight());
        }

        data = tmp;
    }

    return data;
}



/*
   This function returns an histogram of the variable in the "reducedTree" dataset
   between min and max and with nbins and "cuts" applied.
   */

TH1 * Analysis::CreateHisto(string option)
{
    return CreateHisto(0, 0, 50, (TCut)"", "", option);
}

TH1 * Analysis::CreateHisto(double min, double max, int nbin, TCut _cuts, string _weight, string option, TH1 * htemplate)
{
    transform(option.begin(), option.end(), option.begin(), ::tolower);
    if(gDirectory->FindObject("hist_"+name))
        delete gDirectory->FindObject("hist_"+name);
    if(_weight=="" && weight) _weight = weight->GetName();
    if(data && option.find("-usedataset")!=string::npos)
    {
        if(gDirectory->FindObject("hist_"+name+"__"+(TString)var->GetName()))
            delete gDirectory->FindObject("hist_"+name+"__"+(TString)var->GetName());
        dataHist = data->createHistogram("hist_"+name,*var,Binning(nbin, min, max));
    }
    else if(reducedTree)
    {
        TString exp((TString)var->GetName()+">>hist_"+name);
        if(htemplate) exp = ((TString)var->GetName()+">>+"+htemplate->GetName());
        if(min!=max) exp.Append(Form("(%i,%e,%e)",nbin,min,max));
        if(cuts) reducedTree->Draw(exp,buildSelectStr(*cuts+_cuts, _weight),"E");
        else reducedTree->Draw(exp,buildSelectStr(_cuts, _weight),"E");
        dataHist = (TH1*)gPad->GetPrimitive("hist_"+name);
    }

    return dataHist;
}


void Analysis::AddGaussConstraint(TString name, double mean, double sigma)
{
    AddGaussConstraint(GetParam(model, (string)name), mean, sigma);
}

void Analysis::AddGaussConstraint(RooRealVar * par, double mean, double sigma)
{
    if(mean == -1e9) mean = par->getVal();
    if(sigma == -1e9) sigma = par->getError();
    TString name = par->GetName(); 

    RooRealVar  *cm = new RooRealVar("cm_"+name, "mean_"+name, mean);
    RooRealVar  *cs = new RooRealVar("cs_"+name, "error_"+name, sigma);
    RooGaussian *constr = new RooGaussian("constr_"+name, "constr_"+name, *par, *cm, *cs);
    if (pmode == "v") cout << "Constraining... " << endl;
    par->Print();
    constr->Print();
    if (pmode == "v") cout << Form("Constraint : "+name+"%s -> gauss(%f,%f)",mean,sigma) << endl;

    AddConstraint(constr);
}


/*
   Functions to set the units.
   "units" is a string with the unit label
   "outScale" is the scale factor from internal scale to output scale
   SetUnits(string inUnit, string outUnit) finds automatically the correct outScale knowing the input and output units.
   */

void Analysis::SetUnits(string inUnit, string outUnit)
{
    int nunits = 8;
    string units[] = {"eV","keV","MeV","GeV","TeV","PeV"};

    int iIn = 0, iOut = 0;
    while( inUnit != units[iIn] && iIn < nunits ) iIn++;
    while( outUnit != units[iOut] && iOut < nunits ) iOut++;

    if(iIn >= nunits || iOut >= nunits) { cout << "In or Out unit not found, units are not set" << endl; return; }
    scale = TMath::Power(1000,(iIn - iOut));
    unit = units[iOut] + "/c^{2}";
}

void Analysis::SetUnits(string outUnit, double scalefactor)
{
    unit = outUnit;
}



/*
   Fits the "reducedTree" with the "model" which has to be previourly set and initialized.
   One can set the fit range and number of bins for chi2. If not set the variable range is used.
   @param min, max: fitting interval, if min => max all available is used 
   @param nbins: n of bins to use (if unbinned this does nothing)
   @param unbinned: true for unbinned fit
   @param print: Print options
   "-fillSig"    -> signal is filled with color instead of dashed line
   "-fillBkg"    -> bkg is filled with color instead of dashed line
   "-noParams"   -> no params box produced
   "-noCost"     -> no constant parameters shown in params box
   "-nochi2"     -> no chi2 in params box
   "-quiet"      -> shell output minimized
   "-sumW2err"   -> if weighted data errors shown reflect statistics of initial sample
   "-plotSigComp"-> prints signal components and not only total signal function
   "-log"        -> logarithmic plot
   "-pulls" or "-ANDpulls" -> if data is inserted these add a pull histogram -pulls in other plot -ANDpulls under fit plot
   "-range"      -> plots only the fitted range, otherwise all available is plot
   "-noPlot"     -> doesn't print and only returns the frame
   @param cuts: cuts to make before fitting
   */

RooPlot * Analysis::Fit(string option, TCut extracuts)
{
    return Fit(var->getMin(), var->getMax(), 100, true, option, extracuts);
}

RooPlot * Analysis::Fit(double min, double max, unsigned nbins, bool unbinned, string option, TCut extracuts)
{
    string low_opt = option;
    transform(low_opt.begin(), low_opt.end(), low_opt.begin(), ::tolower);
    if (!ModelBuilder::isValid())
    {
        cout << "***** WARNING: No model is set! *****" << endl;
        return NULL;
    }

    RooCmdArg fitRange(RooCmdArg::none());
    double minr = var->getMin();
    double maxr = var->getMax();

    // Plot range
    if ((min < max) && (min > minr) && (max < maxr))
    {
        minr = min;
        maxr = max;
    }

    //Fit range
    if(option.find("-sidebandfit")!=string::npos)
    {
        string ranges = "";
        if(regStr.size()<2) cout << "WARNING: no regions set!" << endl;
        else
        {
            for(auto r : regStr) if(option.find("band")!=string::npos) ranges += r+",";
            ranges.pop_back();
            fitRange = Range(ranges.c_str());
        }
    }
    else if(option.find("-fitrange")!=string::npos)
    {
        size_t pos_fr = option.find("-fitrange[");
        size_t pos_efr = option.find("]",pos_fr);
        string myranges = option.substr(pos_fr+10,pos_efr-pos_fr-10);
        fitRange = Range(myranges.c_str());
    }

    if (!dataHist && !reducedTree && !data)
    {
        cout << "WARNING: No data to fit available!" << endl;
        return NULL;
    }

    if (pmode == "v") cout << endl << name << ": Fit " << var->getTitle() << " (" << nbins << "," << minr << "," << maxr << ") " << option << endl;

    RooAbsData * mydata = data;
    if(low_opt.find("-docuts")==string::npos || !data || extracuts != "" )
    {
        if( (extracuts!="" || !data) && low_opt.find("-forcehist")==string::npos )
            mydata = CreateDataSet(option,extracuts);
        if( (reducedTree && !unbinned) || (dataHist && low_opt.find("-forcehist")!=string::npos) )
        {
            CreateHisto(minr, maxr, nbins, (TCut)"", GetWeight(), option);
            mydata = new RooDataHist("data"+name,"",*var,dataHist);
        }
    }

    if ((pmode != "v") || (low_opt.find("-quiet") != string::npos))
        RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);

    if (mydata)
    {
        RooCmdArg constraints = ExternalConstraints(*constr);
        if (low_opt.find("-constrainall") != string::npos)
            constraints = ExternalConstraints(*gaussianConstraints(model,RooArgSet(*var)));

        RooCmdArg isExtended = Extended(kTRUE);
        if ( (low_opt.find("-noextended") != string::npos) || (bkg_components.size() < 1) )
            isExtended = Extended(kFALSE);

        RooCmdArg isQuiet = PrintLevel(2);
        if (low_opt.find("-quiet") != string::npos)
            isQuiet = PrintLevel(-1);

        RooCmdArg useMinos = Minos(kFALSE);
        if (low_opt.find("-minos") != string::npos)
            useMinos = Minos(kTRUE);

        m_fitRes = model->fitTo(*mydata, fitRange, isExtended,
                SumW2Error(true), isQuiet, Warnings(false), useMinos, Save(true), constraints);
	
	if (low_opt.find("-quiet") == string::npos)
		cout << name << " :  CovQual = " << m_fitRes->covQual() << ",   Status = " << m_fitRes->status() << ",   EDM = " << m_fitRes->edm() << endl;
         
    }
    else
    {
        cout << "NO DATA!!" << endl;
        return NULL;
    }

    if(option.find("-hidesig") != string::npos)
    {
        ((RooRealVar*) nsig)->setConstant();
        option += "-nocost";
    }

    var->setVal(tmpvar->getVal());
    double range[2] = {minr, maxr};
    RooPlot *plot = (RooPlot*) PrintAndCalcChi2(nbins, range, option, mydata);

    /*
    if(vars.size()>1)
    {
        for(size_t v = 1; v < vars.size(); v++)
            PrintVar(vars[v],nbins,option+"-vname");
    }
	*/

    if(option.find("-hidesig") != string::npos) ((RooRealVar*) nsig)->setConstant(0);

    return plot;
}


RooWorkspace * Analysis::SaveToRooWorkspace(string option)
{
    RooWorkspace * ws = new RooWorkspace("ws_"+name);
    if(option != "") ws->SetName("ws_"+name+"_"+option);
    if (pmode == "v") cout << endl << name << ": SaveToRooWorkspace" << endl;


    if(option == "")
    if(model)
    {
	ws->import(*model);
	if (pmode == "v") cout << "model: " << model->GetName() << endl;
    }

    if(option == "sig")
    if(sig)
    {
        ws->import(*sig);
	if (pmode == "v") cout << "signal: " << sig->GetName() << endl;
    }

    if(option == "bkg")
    if(bkg)
    {
        ws->import(*bkg);
	if (pmode == "v") cout << "background: " << bkg->GetName() << endl;
    }

    if(data)
    {
        ws->import(*data);
	if (pmode == "v") cout << "data: " << data->GetName() << endl;
    }
    /*
    if (pmode == "v")
    {
        cout << endl;
        ws->Print();
        cout << endl;
    }
    */    
    return ws;
}

void Analysis::ImportModel(RooWorkspace * ws)
{
    if (pmode == "v") cout << endl << name << ": ImportModel" << endl << endl;

    bkg_fractions.clear();
    bkg_components.clear();

    TIterator * it = ws->componentIterator();
    TObject * arg;
    while( (arg=(TObject *)it->Next()) )
    {
        string name = arg->GetName();
	//if (pmode == "v") cout << name << endl;
        if(name.find("model")!=string::npos)
            model = (RooAbsPdf*)arg;
	
        else if (name.find("totsig")!=string::npos)
            sig = (RooAbsPdf*)arg;
        else if (name.find("totbkg")!=string::npos)
            bkg = (RooAbsPdf*)arg;
	    else if (name.find("nsig")!=string::npos)
            nsig = (RooAbsReal*)arg;
        else if (name.find("nbkg")!=string::npos)
        {
            bkg_fractions.push_back((RooAbsReal*)arg);
            TIterator * it2 = ws->componentIterator();
            TObject * arg2;
            while( (arg2=(TObject *)it2->Next()) )
            {
                string compname = (string)((TString)name).ReplaceAll("nbkg","bkg");
                if( ((string)(arg2->GetName())).find(compname)!=string::npos )
                    bkg_components.push_back( (RooAbsPdf*)arg2 );       
            }
        }
        else if (name.find("var")!=string::npos)
            var = (RooRealVar*)arg;
    }

    init = true;
    ForceValid();
    if (pmode == "v")
    {
        cout << name << ": PrintParams" << endl << endl;
	ModelBuilder::PrintParams();
    }
}

void Analysis::ImportModel(RooWorkspace * wsSig, RooWorkspace * wsBkg)
{
    if (pmode == "v") cout << endl << name << ": ImportModel" << endl << endl;

    TIterator * itSig = wsSig->componentIterator();
    TObject * argSig;
    while( (argSig=(TObject *)itSig->Next()) )
    {
        string name = argSig->GetName();
	//if (pmode == "v") cout << name << endl;
	if (name.find("totsig")!=string::npos)
	    sig = (RooAbsPdf*)argSig;
    }

    TIterator * itBkg = wsBkg->componentIterator();
    TObject * argBkg;
    while( (argBkg=(TObject *)itBkg->Next()) )
    {
        string name = argBkg->GetName();
	//if (pmode == "v") cout << "Importing... " << name << endl;
        if (name.find("totbkg")!=string::npos)
	{
            bkg = (RooAbsPdf*)argBkg;
	//	cout << "Found " << endl;
	}
    } 

    init = true;
    ForceValid();
    /*
    if (pmode == "v")
    {
        cout << name << ": PrintParams" << endl << endl;
	ModelBuilder::PrintParams();
    }
    */
}

void Analysis::ImportData(RooWorkspace * ws)
{
    if (pmode == "v") cout << endl << name << ": ImportData" << endl;

    list<RooAbsData *> mylist = ws->allData();
    for (std::list<RooAbsData *>::iterator it=mylist.begin(); it != mylist.end(); ++it)
    {
        string name = (*it)->GetName();
	//if (pmode == "v") cout << name << endl;
        if(name.find("data_")!=string::npos)
            data = (RooDataSet*)(*it);
    }

    if(!data) cout << "Data not found in work space" << endl;
    else init = true;
}




/*
   This function allows to apply cuts on "dataReader" and returned a tree containing only events which pass the cut.
   @substtree = true if you want to set the three obtained as "reducedTree" (default = true)
   @addFunc = you may define a function getting a TreeReader and a TTree which is called in the loop and adds variables to the new tree combining information from the old tree
   @frac uses only the fraction "frac" of the available entries
   N.B.: addFunc is called once before the loop and here you should set static addresses.
   */

TTree * Analysis::applyFunc(void (*addFunc)(TreeReader *,  TTree *, bool), double frac)
{
    return applyCuts((TCut)"", true, addFunc, frac);
}

TTree * Analysis::applyCuts(TCut _cuts, bool substtree, void (*addFunc)(TreeReader *,  TTree *, bool),  double frac)
{
    if( !dataReader ) {cout << "WARNING: No tree available! Set one before applying cuts." << endl; return NULL;}
    if( pmode=="v" ) cout << endl << name << ": Creating new tree with candidates which passed all cuts" << endl;

    if(cuts) _cuts += *cuts;
    TTree * newTree = new TTree("cand"+name,"");
    dataReader->FillNewTree(newTree, _cuts, frac, addFunc);

    if(substtree) reducedTree = newTree;
    return newTree;
}


/*
   This functions checks for multiple candidate in the same event and creates a plot of number of candidates and a new tree with a variable "isChosenCand" added. This will be 1 for the best candidate and 0 for the others.
   @substRedTree = true to set the tree obtained as "reducedTree"
   @useCuts = true also applies cuts on the tree and checks only events which pass the cut
   N.B.: if you just need to apply cuts ad not to check for multiples USE "applyCuts" it's much more efficient.

   randomKill() is a standard function for random killing of multiple candidates.
   */


Long64_t bestPID(TreeReader * reader, vector< Long64_t > entry)
{
    int best = 0;
    double bestpid = -1e9;
    for (size_t e = 0; e < entry.size(); e++)
    {
        reader->GetEntry(entry[e]);
        double pid = reader->GetValue("Pi_ProbNNpi") * reader->GetValue("K_ProbNNk");
        if(pid > bestpid)
        {
            bestpid = pid;
            best = e;
        }
    }

    return entry[best];
}


Long64_t randomKill(TreeReader * reader, vector< Long64_t > entry)
{
    TRandom3 rdm(0);
    int nrdm = rdm.Rndm() * entry.size();
    return entry[nrdm];
}

TTree * Analysis::GetSingleTree(FUNC_PTR choose, TString namevar, bool reset)
{
    if (!reducedTree)
    {
        cout << endl << "WARNING: No reduced tree available!" << endl << endl;
        return NULL;
    }

    if (pmode == "v") cout << name << ": Checking multiple candidates" << endl;

    TTree *singleTree = (TTree*) reducedTree->CloneTree(0);
    int isSingle, multiplicity;
    TString namesingle = "isSingle";
    if(namevar!="") namesingle += "_"+namevar;
    singleTree->Branch(namesingle, &isSingle, namesingle+"/I");

    bool has_multi = false;
    TObjArray* branches = singleTree->GetListOfBranches();
    for (int i = 0; i < branches->GetEntries(); ++i)
        if( (string)((TBranch*)branches->At(i))->GetName() == "Multiplicity" )
            has_multi = true;

    if(!has_multi) singleTree->Branch("Multiplicity", &multiplicity, "Multiplicity/I");

    TreeReader *reducedReader = new TreeReader(reducedTree);
    int ntot = reducedReader->GetEntries();

    vector <Long64_t> entryn;
    reducedReader->GetEntry(0);
    entryn.push_back(0);

    ULong64_t lastEvtNumber = reducedReader->GetValue("eventNumber");
    UInt_t    lastRunNumber = reducedReader->GetValue("runNumber");	

    if (pmode == "v") cout << endl;

    //TH1I * ncandhisto = new TH1I("ncand"+name,"ncand",10,0,10);

    for (Long64_t i = 1; i < ntot; i++)
    {
        showPercentage(i, ntot);

        reducedReader->GetEntry(i);

        ULong64_t curEvtNumber = reducedReader->GetValue("eventNumber");
        UInt_t    curRunNumber = reducedReader->GetValue("runNumber");

        bool changed = (curEvtNumber != lastEvtNumber || curRunNumber != lastRunNumber || i == (ntot-1));

        if (changed)
        {
            if (i == (ntot-1)) entryn.push_back(i);
            //ncandhisto->Fill(entryn.size());
            multiplicity = entryn.size();

            Long64_t keep = 0;
            if(choose) keep = choose(reducedReader,entryn);
            else keep = randomKill(reducedReader, entryn);

            for (unsigned v = 0; v < entryn.size(); v++)
            {
                reducedReader->GetEntry(entryn[v]);
                isSingle = 0;

                if (entryn[v] == keep) isSingle = 1;
                singleTree->Fill();
            }

            entryn.clear();
            lastEvtNumber = curEvtNumber;
            lastRunNumber = curRunNumber;
        }

        entryn.push_back(i);
    }

    if (pmode == "v") cout << endl;
    if (reset) reducedTree = singleTree;

    return singleTree;
}



/*
   Functions for chi2 extraction
   */

double Analysis::GetChi2()
{
    if( chi2[0] < 0. ) cout << name << ": No valid chi2 was calculated yet!" << endl;
    return chi2[0];
}
double Analysis::GetNDF()
{
    if( chi2[1] < 0. ) cout << name << ": No valid chi2 was calculated yet!" << endl;
    return chi2[1];
}
void Analysis::PrintChi2()
{
    if( chi2[0] < 0. ) cout << name << ": No valid chi2 was calculated yet!" << endl;
    else cout << name << fixed << setprecision(1) << ": Chi2/NDF = " << chi2[0] << " NDF = " << chi2[1] << " - with probability " << fixed << setprecision(3) << TMath::Prob(chi2[0]*chi2[1],chi2[1]) << endl;
}
double Analysis::GetProb()
{
    if( chi2[1] < 0. ) cout << name << ": No valid chi2 was calculated yet!" << endl;
    return TMath::Prob(chi2[0]*chi2[1],chi2[1]);
}


/*
   Allows to make nice plots of data and models including blinded plots
   @param model: if true plots model on data
   @param opt: options string. Options available are:
   "-fillSig" -> signal is filled with color instead of dashed line
   "-fillBkg" -> bkg is filled with color instead of dashed line
   "-log" -> logarithmic plot
   "-pulls" or "-ANDpulls" -> if data is inserted these add a pull histogram -pulls in other plot -ANDpulls under fit plot
   "-none" -> doesn't print and only returns the frame
   @param data: data to plot -> If no data use NULL
   @param bins: number of bins to use for data
   @param Xtitle: X axis label
   @param title: title
   */

RooPlot* Analysis::Print(string option, unsigned bins, double * range, TString Xtitle, TString title, RooRealVar * myvar)
{
    bool domodel = true;
    RooDataSet * mydata = NULL;
    if(!myvar) myvar = var;
    if(option.find("-nomodel")!=string::npos) domodel = false;
    if(option.find("-nodata")==string::npos) mydata = GetDataSet(option);
    return Print(domodel, mydata, option, bins, range, Xtitle, title, myvar);
}

RooPlot* Analysis::Print(bool domodel, RooAbsData * data, string option, unsigned bins, double * range, TString Xtitle, TString title, RooRealVar * myvar)
{
    if(!data) { cout << "WARNING!: No data available." << endl; domodel = true; }

    unsigned posX = option.find("-xu");
    if( posX > 1e4 ) posX = option.find("-x");
    else if(unit!="") Xtitle = option.substr(posX+3,option.find("-",posX+3) - posX - 3) + " ["+ unit +"]";
    if( posX < 1e4 && Xtitle == "") Xtitle = option.substr(posX+2,option.find("-",posX+2) - posX - 2);
    Xtitle = (string)((TString)Xtitle).ReplaceAll("__var__","");
    if(!myvar) myvar = var;

    TString Ytitle = "";
    double width = 0, intpart = 0;
    if(range) width = (range[1]-range[0])/bins;
    else width = (myvar->getMax()-myvar->getMin())/bins;
    if(modf(width,&intpart) == 0.) Ytitle = Form("Candidates per %i",(int)width);
    else if( modf(width/0.1,&intpart) == 0. ) Ytitle = Form("Candidates per %.1f",width);
    else Ytitle = Form("Candidates per %.2f",width);
    if(unit != "") Ytitle += ( " " + unit );

    transform(option.begin(), option.end(), option.begin(), ::tolower);

    if(option.find("-empty")!=string::npos) 
    {
        RooPlot * ple = new RooPlot(*myvar,myvar->getMin(),myvar->getMax(),bins);
        ple->SetXTitle(Xtitle);
        ple->SetYTitle(Ytitle);
        return ple;
    }

    if(domodel)
    {
        if(option.find("-linlog")!=string::npos)
        {
            if(option.find("-andpulls")!=string::npos)
            {
                string optLin = option;
                optLin.replace(optLin.find("log"), 3, "");
                optLin.replace(optLin.find("-andpulls"), 9, "");
                ModelBuilder::Print(title, Xtitle, optLin, data, bins, regStr, range, m_fitRes, Ytitle, myvar);
                string optLog = option;
                optLog.replace(optLog.find("lin"), 3, "");
                optLog.replace(optLog.find("-andpulls"), 9, "");
                ModelBuilder::Print(title, Xtitle, optLog, data, bins, regStr, range, m_fitRes,Ytitle, myvar);
            }
            string optLin = option;
            optLin.replace(optLin.find("log"), 3, "");
            ModelBuilder::Print(title, Xtitle, optLin, data, bins, regStr, range, m_fitRes, Ytitle, myvar);
            string optLog = option;
            optLog.replace(optLog.find("lin"), 3, "");
            return ModelBuilder::Print(title, Xtitle, optLog, data, bins, regStr, range, m_fitRes, Ytitle, myvar);
        }
        else return ModelBuilder::Print(title, Xtitle, option, data, bins, regStr, range, m_fitRes, Ytitle, myvar);
    }
    else
    {
        TCanvas * myc = new TCanvas();	
        if(option.find("-log")!=string::npos)  myc->SetLogy();
        RooPlot * frame = GetFrame(myvar, data, NULL, option, bins, range, regStr, Xtitle, Ytitle, NULL, vector<Color_t>());
        if(option.find("-bw")!=string::npos) myc->SetGrayscale();
        frame->Draw();
        if(title=="") myc->Print("data_"+name+".pdf");
        else myc->Print(title);
        if(option.find("-linlog")!=string::npos)
        {
            GetFrame(myvar, data, NULL, option, bins, range, regStr, Xtitle, Ytitle, NULL, vector<Color_t>())->Draw();
            if(title=="") myc->Print("data_"+name+".pdf");
            else myc->Print(title);
        }

        delete myc;
        return frame;
    }
}


RooPlot * Analysis::PrintAndCalcChi2(int nbins, double * range, string option, RooAbsData * mydata)
{
    string low_opt = option;
    transform(low_opt.begin(), low_opt.end(), low_opt.begin(), ::tolower);

    RooPlot * f = NULL;
    double rangeplot_tmp[] = {var->getMin(),var->getMax()};
    double * rangeplot = &rangeplot_tmp[0];
    if(low_opt.find("-plotrange")!=string::npos) rangeplot = range;
    if(mydata) f = Print(true, mydata, option, nbins, rangeplot);
    else f = Print(option, nbins, rangeplot);

    double * chi2ndf = calcChi2(f, getNFreePars(model,RooArgSet(*var)), range);
    chi2[0] = chi2ndf[0];
    chi2[1] = chi2ndf[1];
    if(option=='v') PrintChi2();

    return f; 
}


RooPlot * Analysis::PrintVar(RooRealVar * myvar, int nbins, string option) 
{
    string low_opt = option;
    transform(low_opt.begin(), low_opt.end(), low_opt.begin(), ::tolower);

    if(!myvar) myvar = var;
    return Print(option, nbins, (double *)NULL, "", "", myvar); 
}


/*
   Adds a blinded region.
   */

void Analysis::SetBlindRegion(double min, double max)
{
    if(min >= max) { cout << "Min has to be less than max" << endl; return; }
    if(regions.size() > 0) if(regions[regions.size()] > min) { cout << "Blinded signal regions must not overlap and must be entered in order from low to high" << endl; return; }
    if(min < var->getMin() || max > var->getMax()) { cout << "Blinded region bust be in variable range" << endl; return; }
    regions.push_back(min);
    regions.push_back(max);
    if(init) cout << "Remember to reinitialzie!!!" << endl;
}



/*
   Functions to generate events using the model set
   */

TTree * Analysis::Generate(int nevt, string option)
{
    transform(option.begin(), option.end(), option.begin(), ::tolower);
    if(pmode=="v") cout << endl << name << ": Generating " << nevt << " events (" << option << ")" << endl;

    if(model)
    {
        RooArgSet varList("varList_"+name);
        for(unsigned i = 0; i < vars.size(); i++) varList.add(*(vars[i]));

        if(option.find("-useroofit")==string::npos)
        {   
            TTree * newTree = generate(&varList,model,nevt,option);
            newTree->SetName("gen_"+name);
            reducedTree = newTree;
            if(option.find("-nodataset")==string::npos) GetDataSet("-recalc");
            return reducedTree;
        }
        else
        {
            cout << "Generating toys with roofit function" << endl;
            size_t posseed = option.find("-seed");
            if(posseed!=string::npos)
            {
                int seed = ((TString)(option.substr(posseed+5))).Atof();
                RooRandom::randomGenerator()->SetSeed(seed);
            }
            RooCmdArg ext = RooCmdArg::none();
            if(option.find("-genextended")!=string::npos) ext = Extended(); 
            data = model->generate(varList,nevt,ext);
            cout << name << ": " << " Generated events = " << data->numEntries() << endl;
            return NULL;
        }
    }
    else return NULL;
}

TTree * Analysis::Generate(double nsigevt, double nbkgevt, string option)
{
    transform(option.begin(), option.end(), option.begin(), ::tolower);
    cout << fixed << setprecision(3);
    if(pmode=="v") cout << endl << name << ": Generating " << nsigevt << " signal events and " << nbkgevt << " bkg events (" << option << ")" << endl;
    
    if(sig && bkg)
    {
        RooArgSet varList("varList_"+name);
        for(unsigned i = 0; i < vars.size(); i++) varList.add(*(vars[i]));

        if(option.find("-useroofit")==string::npos) {

            TTree * newTree = generate(&varList,sig,nsigevt,bkg,nbkgevt,option);
            newTree->SetName("gen_"+name);
            reducedTree = newTree;
            if(option.find("-nodataset")==string::npos) GetDataSet("-recalc");
            return reducedTree;
        }
        else
        {
            cout << "Generating toys with roofit function" << endl;
            size_t posseed = option.find("-seed");
            if(posseed!=string::npos)
            {
                int seed = ((TString)(option.substr(posseed+5))).Atof();
                RooRandom::randomGenerator()->SetSeed(seed);
            }
            RooCmdArg ext = RooCmdArg::none();
            if(option.find("-genextended")!=string::npos) ext = Extended(); 
            TString dataname = "data_"+name;
            if (data) dataname = data->GetName();

	    double ntot = nsigevt+nbkgevt;
	    double frac = nsigevt/ntot;
	    RooRealVar * f_sig = new RooRealVar("f_sig","f_sig",frac);  
            RooAbsPdf * tot = new RooAddPdf("total_pdf","total_pdf",RooArgSet(*sig, *bkg), RooArgSet(*f_sig));
	    cout << "Generating nsig/ntot = " << f_sig->getVal() << endl;
	    data = tot->generate(varList,ntot,ext);
	    
            //model->Print();
            //data = model->generate(varList,ntot,ext);
            cout << name << ": " << " Generated events = " << data->numEntries() << endl;
            
            return NULL; 
        }
    }
    else return NULL;
}



/*
   Function to calculate S-weight for the data set using a model specified
   */




RooDataSet * Analysis::CalcSWeight(double min, double max, unsigned nbins, bool unbinned, string option)
{
    transform(option.begin(), option.end(), option.begin(), ::tolower);
    if (pmode == "v") cout << endl << name << ": CalcSWeight " << option << endl;

    TString oldname = name;
    if(option.find("-nofit")==string::npos)
    {
        name+=(TString)"_sW";
	Fit(min, max, nbins, unbinned, option);
    }
    name = oldname;

    cout << endl << endl;
    if(min == max) { min = var->getMin(); max = var->getMax(); }
    cout << "Sig yield = " << nsig->getVal() << endl;
    cout << "Bkg yield = " << nbkg->getVal() << endl;
    cout << endl;

    // Create new TTree with sWeights 
    if(!reducedTree && data) reducedTree = (TTree *)data->tree();
    TreeReader * reducedReader = new TreeReader(reducedTree);
    TTree * sTree = (TTree*) reducedReader->CloneTree("sWeight");

    float sW;
    sTree->Branch("sW", &sW, "sW/F");
    float sWR;
    sTree->Branch("sWR", &sWR, "sWR/F");

    RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);

    Long64_t nEntries = reducedReader->GetEntries();

    GetTotNBkg();
    CalcTotBkg();

    double Vss = 0;
    double Vbs = 0;
    double Vbb = 0;

    double syield = nsig->getVal();
    double byield = nbkg->getVal();

    for(Long64_t i = 0 ; i < nEntries ; ++i)
    {
        reducedReader->GetEntry(i);
        float value = reducedReader->GetValue(var->GetName());
        if (value < min || value > max) continue;
        var->setVal(value);

        double s_pdf = sig->getVal(*var);
        double b_pdf = bkg->getVal(*var);

        double denom = syield*s_pdf + byield*b_pdf;
        Vss += (s_pdf * s_pdf)/(denom*denom);
        Vbs += (s_pdf * b_pdf)/(denom*denom);
        Vbb += (b_pdf * b_pdf)/(denom*denom);
    }

    double det = TMath::Abs(Vss*Vbb-Vbs*Vbs);
    double invVss = Vbb/det;
    double invVbs = - Vbs/det;
    //double invVbb = Vss/det;

    GetTotNBkg();

    double sum_sW = 0;
    for(Long64_t i = 0 ; i < nEntries ; ++i)
    {
        showPercentage(i, nEntries);

        reducedReader->GetEntry(i);
        float value = reducedReader->GetValue(var->GetName());
        if (value < min || value > max) continue;
        var->setVal(value);

        double b_pdf = bkg->getVal(*var);
        double s_pdf = sig->getVal(*var);
        double denom = syield*s_pdf + byield*b_pdf;
        sW = (invVss * s_pdf + invVbs * b_pdf) / denom;

        sWR = GetReducedSWeight(value);

        sTree->Fill();

        //double sW_b = (invVbb * b_pdf + invVbs * s_pdf) / denom;

        sum_sW += sW;
    }

    cout << endl;
    cout << "sWeighted " << sTree->GetEntries() << " entries" << endl;
    cout << "Sum of weights = " << sum_sW << endl;
    cout << endl;

    reducedTree = sTree;
    
    TCanvas *c = new TCanvas();
    gStyle->SetOptStat(0);

    sTree->Draw("sW");

    c->Print((TString) name + "_sWeights.pdf");

    sTree->Draw(var->GetName()+(TString)">>hSWHisto","sW");
    TH1D * hSWHisto = (TH1D*)gPad->GetPrimitive("hSWHisto");
    sTree->Draw(var->GetName()+(TString)">>hHisto");
    TH1D * hHisto = (TH1D*)gPad->GetPrimitive("hHisto");

    hSWHisto->SetLineColor(1);
    hHisto->SetLineColor(4);
    hHisto->Draw("hist");
    hSWHisto->Draw("hist same");

    c->Print((TString) name + "_sWeighted.pdf");
    
    return data;
}

RooDataSet * Analysis::CalcSWeightRooFit(double min, double max, unsigned nbins, bool unbinned, string option)
{
  transform(option.begin(), option.end(), option.begin(), ::tolower);
  if (pmode == "v") cout << endl << name << ": CalcSWeightRooFit " << option << endl;

  RooDataSet::setDefaultStorageType(RooAbsData::Tree);

  if(option.find("-freeyields")!=string::npos)
    cout << endl;

  bool doInit = false;
  vector<string> comps = { nsig->GetName() };
  RooArgSet * yields = new RooArgSet(*nsig);
  for(size_t bb = 0; bb < bkg_fractions.size(); bb++)
  {
    if(option.find("-freeyields")!=string::npos)
    {
	if(((string)typeid(*bkg_fractions[bb]).name()).find("RooFormulaVar")!=string::npos)
	{
	  cout << "Free Formula: " << bkg_fractions[bb]->GetName() << endl;
	  bkg_fractions[bb] = new RooRealVar(bkg_fractions[bb]->GetName(), bkg_fractions[bb]->GetName(), ((RooRealVar*)bkg_fractions[bb])->getVal(), ((RooRealVar*)bkg_fractions[bb])->getVal() / 2., ((RooRealVar*)bkg_fractions[bb])->getVal() * 2.);
	  doInit = true;
	}
        else
	{
	  cout << "Free Var:     " << bkg_fractions[bb]->GetName() << endl;
	  ((RooRealVar*)bkg_fractions[bb])->setConstant(0);
	}
    }
    yields->add(*bkg_fractions[bb]);
    comps.push_back(bkg_fractions[bb]->GetName());
  }

  setConstant(model,var,comps,"except-contains");

  if(doInit)
    ModelBuilder::Initialize("");

  TString oldname = name;
  if(option.find("-nofit")==string::npos)
  {
    name+=(TString)"_sWRF";
    Fit(min,max,nbins,unbinned,option);
  }
  name = oldname;

  cout << endl;
  cout << "Calculating sWeights: " << model->GetName() << endl;
  cout << endl;

  yields->Print();

  SPlot *sPlot = new SPlot("splot","splot",*data,model,*yields);

  reducedTree = (TTree*) data->store()->tree();

  RooArgList sweights = (RooArgList) sPlot->GetSWeightVars();

  data->Print();
  sweights.Print();
  reducedTree->Print();

  TCanvas *c = new TCanvas();
  gStyle->SetOptStat(0);

  reducedTree->Draw(sweights[0].GetName()+(TString)">>hSWeights_"+sweights[0].GetName());
  //TH1D *hSWeights = (TH1D*)gPad->GetPrimitive((TString)"hSWeights_"+sweights[0].GetName());

  c->Print((TString) name + "_sWeights.pdf");

  reducedTree->Draw(var->GetName()+(TString)">>hSWHisto_"+sweights[0].GetName(),sweights[0].GetName());
  TH1D *hSWHisto = (TH1D*)gPad->GetPrimitive((TString)"hSWHisto_"+sweights[0].GetName());

  reducedTree->Draw(var->GetName()+(TString)">>hHisto_"+sweights[0].GetName());
  TH1D *hHisto = (TH1D*)gPad->GetPrimitive((TString)"hHisto_"+sweights[0].GetName());

  hSWHisto->SetLineColor(1);
  hHisto->SetLineColor(4);
  hHisto->Draw("hist");
  hSWHisto->Draw("hist same");

  c->Print((TString) name + "_sWeighted.pdf");

  cout << endl;
  cout << "sWeighted " << reducedTree->GetEntries() << " entries" << endl;
  cout << "Sum of weights = " << hSWHisto->Integral() << endl;
  cout << endl;

  return data;
}

#include "analysis.hpp"


RooRealVar * Scaler::_var = NULL;
double Scaler::_scale = 1.;

string Analysis::m_pmode = "v";

void Analysis::AddAllVariables()
{
    vector< variable * > myvars;
    if (m_dataReader) myvars = m_dataReader->GetVarList();
    else if (m_reducedTree)
    {
        TreeReader * reader = new TreeReader(m_reducedTree);
        reader->Initialize();
        myvars = reader->GetVarList();
    }

    for (auto v : myvars)
    {
        if (v->GetArraySize() > 1) continue;
        AddVariable(v->name);
    }
}

/*
   Function to unitialize the Analysis object before fitting
   */


bool Analysis::Initialize(string option, double frac)
{
    transform(option.begin(), option.end(), option.begin(), ::tolower);
    if (m_pmode == "v") cout << endl << m_name << ": Initialize " << option << endl;

    m_chi2[0] = m_chi2[1] = -1;
    
    vector<double> allregions;
    allregions.push_back(m_var->getMin());
    for (unsigned r = 0; r < m_regions.size(); ++r)
        allregions.push_back(m_regions[r]);
    allregions.push_back(m_var->getMax());
    if (allregions.size() > 2)
    {
        for (unsigned r = 1; r < allregions.size(); ++r)
        {
            string bandname = Form("band_%i", r / 2);
            if (r % 2 == 0) bandname = Form("sig_%i", r / 2);
            m_var->setRange(bandname.c_str(), allregions[r - 1], allregions[r]);

            m_regStr.push_back(bandname);
            m_reg[bandname] = {allregions[r - 1], allregions[r]};
        }
    }

    if (m_dataReader)
    {
        CreateReducedTree(option, frac);
        CreateDataSet();
    }

    if (m_data) m_init = true;
    else if (!m_reducedTree) cout << "WARNING: No data available!!" << endl;

    bool result = ModelBuilder::Initialize(option);
    if (m_bkg_components.empty() && m_data) ((RooRealVar*)m_nsig)->setVal(m_data->numEntries());
    if (m_pmode == "v")
    {
        cout << endl << m_name << ": PrintParams" << endl << endl;
        ModelBuilder::PrintParams(option);
    }

    return result;
}



void Analysis::CreateReducedTree(string option, double frac, TCut mycuts)
{
    transform(option.begin(), option.end(), option.begin(), ::tolower);
    if (m_pmode == "v") cout << endl << m_name << ": CreateReducedTree " << option << endl;

    TCut doCuts = "";
    if (m_cuts) doCuts = *m_cuts;
    if (mycuts != "") doCuts += mycuts;

    if (!m_dataReader->isValid()) m_dataReader->Initialize();

    if (option.find("-docuts") != string::npos)
        m_reducedTree = (TTree*) m_dataReader->CopyTree(doCuts, frac, (string) ("reduced_" + m_name));
    else if (!m_reducedTree)
        m_reducedTree = (TTree*) m_dataReader->GetChain();//->Clone("reduced_" + m_name);

    if ( scale != 1 && !m_dataReader->HasVar( ((string)m_var->GetName() + "_unscaled").c_str() ) )
    {
        if (m_pmode == "v") cout << "Scaling variable... " << endl;
        string oldm_pmode = TreeReader::GetPrintLevel();
        TreeReader::SetPrintLevel("s");
        Scaler::Set(scale, m_var);
        applyFunc(&Scaler::Scale);
        TreeReader::SetPrintLevel(oldm_pmode);
    }

    return;
}


/*
   This function converts the information in the Analysis object in a RooDataHist which can be fitted
   */

RooDataSet * Analysis::CreateDataSet(string option, TCut mycuts)
{
    transform(option.begin(), option.end(), option.begin(), ::tolower);
    if (m_pmode == "v") cout << endl << m_name << ": CreateDataSet " << option << endl;

    if (m_reducedTree && (option.find("-forcehist") == string::npos))
    {
        if (mycuts != "" || option.find("-docuts") != string::npos) CreateReducedTree("-docuts", -1, mycuts);

        RooArgList varList("varList_" + m_name);
        for (auto vv : m_datavars) varList.add(*vv);

        if (m_pmode == "v") cout << "Candidates: " << m_reducedTree->GetEntries() << endl;
        if (m_dataReader && !m_dataReader->HasVars(m_datavars))
            cout << "WARNING: the tree does not contain the requested variable!" << endl;

        if (m_data) m_data->reset();
        if (m_weight)
        {
            varList.add(*m_weight);
            m_data = new RooDataSet("data_" + m_name, "data" + m_name, varList, Import(*m_reducedTree), WeightVar(m_weight->GetName()));
        }
        else m_data = new RooDataSet("data_" + m_name, "data" + m_name, varList, Import(*m_reducedTree));

        CreateHisto("-usedataset");

        if (m_pmode == "v") m_data->Print();
    }
    else if (m_dataHist)
    {
        RooDataHist *htmp   = new RooDataHist("data" + m_name, "", *m_var, m_dataHist);
        RooRealVar  *w      = new RooRealVar("w" + m_name, "", 1., 0., 1.e6);
        RooArgSet   *ArgSet = new RooArgSet("args");
        ArgSet->add(*m_var);
        ArgSet->add(*w);
        RooDataSet  *tmp    = new RooDataSet("data" + m_name, "", *ArgSet, "w" + m_name);

        for (int i = 0; i < htmp->numEntries(); ++i)
        {
            htmp->get(i);
            tmp->add(*htmp->get(i), htmp->weight());
        }

        m_data = tmp;
    }

    return m_data;
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
    if (gDirectory->FindObject("hist_" + m_name))
        delete gDirectory->FindObject("hist_" + m_name);
    if (_weight == "" && m_weight) _weight = m_weight->GetName();
    if (m_data && option.find("-usedataset") != string::npos)
    {
        if (gDirectory->FindObject("hist_" + m_name + "__" + (TString)m_var->GetName()))
            delete gDirectory->FindObject("hist_" + m_name + "__" + (TString)m_var->GetName());
        m_dataHist = m_data->createHistogram("hist_" + m_name, *m_var, Binning(nbin, min, max));
    }
    else if (m_reducedTree)
    {
        TString exp((TString)m_var->GetName() + ">>hist_" + m_name);
        if (htemplate) exp = ((TString)m_var->GetName() + ">>+" + htemplate->GetName());
        if (min != max) exp.Append(Form("(%i,%e,%e)", nbin, min, max));
        if (m_cuts) m_reducedTree->Draw(exp, buildSelectStr(*m_cuts + _cuts, _weight), "E");
        else m_reducedTree->Draw(exp, buildSelectStr(_cuts, _weight), "E");
        m_dataHist = (TH1*)gPad->GetPrimitive("hist_" + m_name);
    }

    return m_dataHist;
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
    string units[] = {"eV", "keV", "MeV", "GeV", "TeV", "PeV"};

    int iIn = 0, iOut = 0;
    while ( inUnit != units[iIn] && iIn < nunits ) iIn++;
    while ( outUnit != units[iOut] && iOut < nunits ) iOut++;

    if (iIn >= nunits || iOut >= nunits) { cout << "In or Out unit not found, units are not set" << endl; return; }
    scale = TMath::Power(1000, (iIn - iOut));
    m_unit = units[iOut] + "/c^{2}";
}

void Analysis::SetUnits(string outUnit, double scalefactor)
{
    m_unit = outUnit;
}



/*
   Fits the "reducedTree" with the "m_model" which has to be previourly set and initialized.
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
    return Fit(100, true, option, extracuts);
}

RooPlot * Analysis::Fit(unsigned nbins, bool unbinned, string option, TCut extracuts)
{
    string low_opt = option;
    transform(low_opt.begin(), low_opt.end(), low_opt.begin(), ::tolower);
    if (!ModelBuilder::isValid())
    {
        cout << "***** WARNING: No m_model is set! *****" << endl;
        return NULL;
    }
    int ncpu = 1;
    size_t poscpu = option.find("-ncpu");
    if (poscpu != string::npos) ncpu = atoi(option.substr(poscpu + 5).c_str());
    cout << "Using " << ncpu << " CPUs" << endl;

    RooCmdArg fitRange(RooCmdArg::none());
    double minr = m_var->getMin();
    double maxr = m_var->getMax();

    //Fit range
    if (option.find("-sidebandfit") != string::npos)
    {
        string ranges = "";
        if (m_regStr.size() < 2) cout << "WARNING: no regions set!" << endl;
        else
        {
            for (auto r : m_regStr) if (r.find("band") != string::npos) ranges += r + ",";
            ranges.pop_back();
            fitRange = Range(ranges.c_str());
        }
    }
    else if (option.find("-fitrange") != string::npos)
    {
        size_t pos_fr = option.find("-fitrange[");
        size_t pos_efr = option.find("]", pos_fr);
        string myranges = option.substr(pos_fr + 10, pos_efr - pos_fr - 10);
        fitRange = Range(myranges.c_str());
    }

    if (!m_dataHist && !m_reducedTree && !m_data)
    {
        cout << "WARNING: No data to fit available!" << endl;
        return NULL;
    }

    if (m_pmode == "v") cout << endl << m_name << ": Fit " << m_var->getTitle() << " (" << nbins << "," << minr << "," << maxr << ") " << option << endl;

    RooAbsData * mydata = m_data;
    if (low_opt.find("-docuts") == string::npos || !m_data || extracuts != "" )
    {
        if ( (extracuts != "" || !m_data) && low_opt.find("-forcehist") == string::npos )
            mydata = CreateDataSet(option, extracuts);
        if ( (m_reducedTree && !unbinned) || low_opt.find("-forcehist") != string::npos )
        {
            CreateHisto(minr, maxr, nbins, (TCut)"", GetWeight(), option);
            mydata = new RooDataHist("data" + m_name, "", *m_var, m_dataHist);
        }
    }

    if ((m_pmode != "v") || (low_opt.find("-quiet") != string::npos))
        RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);

    if (mydata)
    {
        if (low_opt.find("-constrainall") != string::npos)
            m_constr = gaussianConstraints(m_model, RooArgSet(*m_var));

        RooCmdArg isExtended = Extended(kTRUE);
        if ( (low_opt.find("-noextended") != string::npos) || (m_bkg_components.size() < 1) )
            isExtended = Extended(kFALSE);

        if (low_opt.find("-fitto") != string::npos)
        {
            RooCmdArg constraints = ExternalConstraints(*m_constr);
            
            RooCmdArg isQuiet = PrintLevel(2);
            if (low_opt.find("-quiet") != string::npos)
                isQuiet = PrintLevel(-1);

            RooCmdArg useMinos = Minos(kFALSE);
            if (low_opt.find("-minos") != string::npos)
                useMinos = Minos(kTRUE);

            m_fitRes = m_model->fitTo(*mydata, fitRange, isExtended, SumW2Error(true), isQuiet, Warnings(false), useMinos, Save(true), constraints);

            if (m_fitRes)
            {
                if (m_pmode == "v") {
                    cout << endl << m_name << ":   CovQual = " << m_fitRes->covQual();
                    cout << ",   Status = " << m_fitRes->status() << ",   EDM = " << m_fitRes->edm();
                    cout << ",   LogL = " << m_fitRes->minNll() << endl;
                }
            }
        }
        else
        {
            // Prepare Likelihood (normalise and add constraints)

            RooAbsReal * nll = m_model->createNLL(*mydata, isExtended, fitRange, NumCPU(ncpu));

            double nll_init_val = nll->getVal(nll->getVariables());
            RooFormulaVar * nll_norm = new RooFormulaVar("nll_norm", ("@0-" + to_string(nll_init_val)).c_str(), *nll);
            RooAbsReal * nll_toFit = nll_norm;
            if (m_constr->getSize() > 0)
            {
                RooArgList list_for_product;
                list_for_product.add(*nll_norm);
                list_for_product.add(*m_constr);
                nll_toFit = new RooAddition("nll_constrained", "nll_constrained", list_for_product);
                //nll_toFit = new RooProduct("nll_constrained", "nll_constrained", list_for_product);
            }

            // Actual fit

            RooMinuit m(*nll_toFit);

            if (low_opt.find("-quiet") != string::npos)
            {
                m.setPrintLevel(-1);
                m.setWarnLevel(-1);
            }

            bool refit = false;
            if (option.find("-refit") != string::npos) refit = true;

            int i(1);
            double minNll(1);
            do
            {
                if (i > 5) break;

                m.migrad();
                m.hesse();
                if (low_opt.find("-minos") != string::npos) m.minos();
                m_fitRes = m.save();

                if (m_fitRes)
                {
                    if (m_pmode == "v") {
                        cout << endl << m_name << ": (" << i << ")   CovQual = " << m_fitRes->covQual();
                        cout << ",   Status = " << m_fitRes->status() << ",   EDM = " << m_fitRes->edm();
                        cout << ",   LogL = " << m_fitRes->minNll() << " (" << TMath::Abs((m_fitRes->minNll() - minNll) / minNll) << ")" << endl;
                    }
                    minNll = m_fitRes->minNll();
                }

                ++i;
            }
            while (refit && (m_fitRes == NULL || (m_fitRes->covQual() < 3 && TMath::Abs((m_fitRes->minNll() - minNll) / minNll) > 0.01)) ); // loop until converged or no improvement found
        }
    }
    else { cout << "NO DATA!!" << endl; return NULL; }

    if (option.find("-hidesig") != string::npos)
    {
        ((RooRealVar*) m_nsig)->setConstant();
        option += "-nocost";
    }

    //m_var->setVal(m_tmpvar->getVal());
    if (option.find("-hidesig") != string::npos) ((RooRealVar*) m_nsig)->setConstant(0);
    RooPlot *plot = (RooPlot*) PrintAndCalcChi2(nbins, option, mydata);

    return plot;
}


RooWorkspace * Analysis::SaveToRooWorkspace(string option)
{
    RooWorkspace * ws = ModelBuilder::SaveToRooWorkspace(option);

    if (m_data)
    {
        ws->import(*m_data);
        if (m_pmode == "v") cout << "m_data: " << m_data->GetName() << endl;
    }

    return ws;
}

void Analysis::ImportModel(RooWorkspace * ws)
{
    if (m_pmode == "v") cout << endl << m_name << ": ImportModel" << endl << endl;

    m_bkg_fractions.clear();
    m_bkg_components.clear();

    TIterator * it = ws->componentIterator();
    TObject * arg;
    while ( (arg = (TObject *)it->Next()) )
    {
        string name = arg->GetName();

        if (name.find("m_model") != string::npos)
            m_model = (RooAbsPdf*)arg;

        else if (name.find("totsig") != string::npos)
            m_sig = (RooAbsPdf*)arg;
        else if (name.find("totbkg") != string::npos)
            m_bkg = (RooAbsPdf*)arg;
        else if (name.find("nsig") != string::npos)
            m_nsig = (RooAbsReal*)arg;
        else if (name.find("nbkg") != string::npos)
        {
            m_bkg_fractions.push_back((RooAbsReal*)arg);
            TIterator * it2 = ws->componentIterator();
            TObject * arg2;
            while ( (arg2 = (TObject *)it2->Next()) )
            {
                string compname = (string)((TString)name).ReplaceAll("nbkg", "bkg");
                if ( ((string)(arg2->GetName())).find(compname) != string::npos )
                    m_bkg_components.push_back( (RooAbsPdf*)arg2 );
            }
        }
        else if (name.find("var") != string::npos)
            m_var = (RooRealVar*)arg;
    }

    m_init = true;
    ForceValid();
    if (m_pmode == "v")
    {
        cout << m_name << ": PrintParams" << endl << endl;
        ModelBuilder::PrintParams();
    }
}

void Analysis::ImportModel(RooWorkspace * wsSig, RooWorkspace * wsBkg)
{
    if (wsSig)
    {
        if (m_pmode == "v") cout << endl << m_name << ": ImportModel - Signal" << endl << endl;

        TIterator * itSig = wsSig->componentIterator();
        TObject * argSig;
        while ( (argSig = (TObject *)itSig->Next()) )
        {
            string name = argSig->GetName();
            if (name.find("totsig") != string::npos) m_sig = (RooAbsPdf*)argSig;
        }
    }

    if (wsBkg)
    {
        if (m_pmode == "v") cout << endl << m_name << ": ImportModel - Background" << endl << endl;

        TIterator * itBkg = wsBkg->componentIterator();
        TObject * argBkg;
        while ( (argBkg = (TObject *)itBkg->Next()) )
        {
            string name = argBkg->GetName();
            if (name.find("totbkg") != string::npos) m_bkg = (RooAbsPdf*)argBkg;
        }
    }
    /*
        if (wsSig && wsBkg)
        {
            m_init = true;
            ForceValid();
        }
    */
}

void Analysis::ImportData(RooWorkspace * ws)
{
    if (m_pmode == "v") cout << endl << m_name << ": ImportData" << endl;

    list<RooAbsData *> mylist = ws->allData();
    for (std::list<RooAbsData *>::iterator it = mylist.begin(); it != mylist.end(); ++it)
    {
        string name = (*it)->GetName();
        if (name.find("data_") != string::npos) m_data = (RooDataSet*)(*it);
    }

    if (!m_data) cout << "Data not found in work space" << endl;
    else m_init = true;
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
    if ( !m_dataReader ) {cout << "WARNING: No tree available! Set one before applying cuts." << endl; return NULL;}
    if ( m_pmode == "v" ) cout << endl << m_name << ": Creating new tree with candidates which passed all cuts" << endl;

    if (m_cuts) _cuts += *m_cuts;
    TTree * newTree = new TTree("cand" + m_name, "");
    m_dataReader->FillNewTree(newTree, _cuts, frac, addFunc);

    if (substtree) m_reducedTree = newTree;
    return newTree;
}


/*
   This functions checks for multiple candidate in the same event and creates a plot of number of candidates and a new tree with a variable "isChosenCand" added. This will be 1 for the best candidate and 0 for the others.
   @substRedTree = true to set the tree obtained as "reducedTree"
   @useCuts = true also applies cuts on the tree and checks only events which pass the cut
   N.B.: if you just need to apply cuts ad not to check for multiples USE "applyCuts" it's much more efficient.

   randomKill() is a standard function for random killing of multiple candidates.
   */


Long64_t randomKill(TreeReader * reader, vector< Long64_t > entry)
{
    TRandom3 rdm(0);
    int nrdm = rdm.Rndm() * entry.size();
    return entry[nrdm];
}

TTree * Analysis::GetSingleTree(FUNC_PTR choose, TString namevar, bool reset)
{
    if (!m_reducedTree)
    {
        cout << endl << "WARNING: No reduced tree available!" << endl << endl;
        return NULL;
    }

    if (m_pmode == "v") cout << m_name << ": Checking multiple candidates" << endl;

    TTree *singleTree = (TTree*) m_reducedTree->CloneTree(0);
    int isSingle, multiplicity;
    TString namesingle = "isSingle";
    if (namevar != "") namesingle += "_" + namevar;
    singleTree->Branch(namesingle, &isSingle, namesingle + "/I");

    bool has_multi = false;
    TObjArray* branches = singleTree->GetListOfBranches();
    for (int i = 0; i < branches->GetEntries(); ++i)
        if ( (string)((TBranch*)branches->At(i))->GetName() == "Multiplicity" )
            has_multi = true;

    if (!has_multi) singleTree->Branch("Multiplicity", &multiplicity, "Multiplicity/I");

    TreeReader * reducedReader = new TreeReader(m_reducedTree);
    int ntot = reducedReader->GetEntries();

    vector <Long64_t> entryn;
    reducedReader->GetEntry(0);
    entryn.push_back(0);

    ULong64_t lastEvtNumber = reducedReader->GetValue("eventNumber");
    UInt_t    lastRunNumber = reducedReader->GetValue("runNumber");

    if (m_pmode == "v") cout << endl;

    for (Long64_t i = 1; i < ntot; i++)
    {
        showPercentage(i, ntot);

        reducedReader->GetEntry(i);

        ULong64_t curEvtNumber = reducedReader->GetValue("eventNumber");
        UInt_t    curRunNumber = reducedReader->GetValue("runNumber");

        bool changed = (curEvtNumber != lastEvtNumber || curRunNumber != lastRunNumber || i == (ntot - 1));

        if (changed)
        {
            if (i == (ntot - 1)) entryn.push_back(i);
            multiplicity = entryn.size();

            Long64_t keep = 0;
            if (choose) keep = choose(reducedReader, entryn);
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

    if (m_pmode == "v") cout << endl;
    if (reset) m_reducedTree = singleTree;

    return singleTree;
}



/*
   Functions for chi2 extraction
   */

double Analysis::GetChi2()
{
    if ( m_chi2[0] < 0. ) cout << m_name << ": No valid chi2 was calculated yet!" << endl;
    return m_chi2[0];
}
double Analysis::GetNDF()
{
    if ( m_chi2[1] < 0. ) cout << m_name << ": No valid chi2 was calculated yet!" << endl;
    return m_chi2[1];
}
void Analysis::PrintChi2()
{
    if ( m_chi2[0] < 0. ) cout << m_name << ": No valid chi2 was calculated yet!" << endl;
    else cout << m_name << fixed << setprecision(1) << ": Chi2/NDF = " << m_chi2[0] << " NDF = " << m_chi2[1] << " - with probability " << fixed << setprecision(3) << TMath::Prob(m_chi2[0]*m_chi2[1], m_chi2[1]) << endl;
}
double Analysis::GetProb()
{
    if ( m_chi2[1] < 0. ) cout << m_name << ": No valid chi2 was calculated yet!" << endl;
    return TMath::Prob(m_chi2[0] * m_chi2[1], m_chi2[1]);
}


/*
   Allows to make nice plots of data and m_models including blinded plots
   @param m_model: if true plots m_model on data
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

RooPlot* Analysis::Print(string option, unsigned bins, TString Xtitle, TString title, RooRealVar * myvar)
{
    bool dom_model = true;
    RooDataSet * mydata = NULL;
    if (!myvar) myvar = m_var;
    if (option.find("-nomodel") != string::npos) dom_model = false;
    if (option.find("-nodata") == string::npos) mydata = GetDataSet(option);
    return Print(dom_model, mydata, option, bins, Xtitle, title, myvar);
}

RooPlot* Analysis::Print(RooRealVar * myvar, string option, unsigned bins, TString Xtitle, TString title)
{
    bool dom_model = true;
    RooDataSet * mydata = NULL;
    if (!myvar) myvar = m_var;
    if (option.find("-nomodel") != string::npos) dom_model = false;
    if (option.find("-nodata") == string::npos) mydata = GetDataSet(option);
    return Print(dom_model, mydata, option, bins, Xtitle, title, myvar);
}

RooPlot* Analysis::Print(bool domodel, RooAbsData * _data, string option, unsigned bins, TString Xtitle, TString title, RooRealVar * myvar)
{
    if (!_data) { cout << "WARNING!: No data available." << endl; domodel = true; }

    unsigned posX = option.find("-x");
    if (posX < 1e4) Xtitle = option.substr(posX + 2, option.find("-", posX + 2) - posX - 2);
    if (m_unit != "") Xtitle += " [" + m_unit + "]";

    if (!myvar) myvar = m_var;

    TString Ytitle = "";
    double intpart = 0;
    double width = (myvar->getMax() - myvar->getMin()) / bins;
    if (modf(width, &intpart) == 0.) Ytitle = Form("Candidates per %i", (int)width);
    else if ( modf(width / 0.1, &intpart) == 0. ) Ytitle = Form("Candidates per %.1f", width);
    else Ytitle = Form("Candidates per %.2f", width);
    if (m_unit != "") Ytitle += ( " " + m_unit );

    transform(option.begin(), option.end(), option.begin(), ::tolower);

    if (option.find("-empty") != string::npos)
    {
        RooPlot * ple = new RooPlot(*myvar, myvar->getMin(), myvar->getMax(), bins);
        ple->SetXTitle(Xtitle);
        ple->SetYTitle(Ytitle);
        return ple;
    }

    if (domodel)
    {
        if (option.find("-linlog") != string::npos)
        {
            string optLog = option;
            optLog.replace(option.find("lin"), 3, "");
            ModelBuilder::Print(title, Xtitle, optLog, _data, bins, m_regStr, m_reg, m_fitRes, Ytitle, myvar);
        }
        return ModelBuilder::Print(title, Xtitle, option, _data, bins, m_regStr, m_reg, m_fitRes, Ytitle, myvar);
    }
    else
    {
        TCanvas * myc = new TCanvas();
        if (option.find("-log") != string::npos || option.find("-linlog") != string::npos)  myc->SetLogy();
        RooPlot * frame = getFrame(myvar, _data, NULL, option, bins, m_regStr, m_reg, Xtitle, Ytitle, NULL, vector<Color_t>());
        if (option.find("-bw") != string::npos) myc->SetGrayscale();
        frame->Draw();
        if (title == "") myc->Print("data_" + m_name + ".pdf");
        else myc->Print(title);
        if (option.find("-linlog") != string::npos)
        {
            myc->SetLogy(0);
            getFrame(myvar, _data, NULL, option, bins, m_regStr, m_reg, Xtitle, Ytitle, NULL, vector<Color_t>())->Draw();
            if (title == "") myc->Print("data_" + m_name + ".pdf");
            else myc->Print(title);
        }

        delete myc;
        return frame;
    }
}


RooPlot * Analysis::PrintAndCalcChi2(int nbins, string option, RooAbsData * mydata)
{
    string low_opt = option;
    transform(low_opt.begin(), low_opt.end(), low_opt.begin(), ::tolower);

    RooPlot * f = NULL;
    double rangeplot[] = {m_var->getMin(), m_var->getMax()};
    f = Print(true, mydata, option, nbins);

    m_chi2 = calcChi2(f, getNFreePars(m_model, RooArgSet(*m_var)), rangeplot);
    if (option == 'v') PrintChi2();

    return f;
}


/*
   Adds a blinded region.
   */

void Analysis::SetBlindRegion(double min, double max)
{
    if (min >= max) { cout << "Min has to be less than max" << endl; return; }
    if (m_regions.size() > 0) if (m_regions[m_regions.size()] > min) { cout << "Blinded signal regions must not overlap and must be entered in order from low to high" << endl; return; }
    if (min < m_var->getMin() || max > m_var->getMax()) { cout << "Blinded region bust be in variable range" << endl; return; }
    m_regions.push_back(min);
    m_regions.push_back(max);
    if (m_init) cout << "Remember to reinitialzie!!!" << endl;
}



/*
   Functions to generate events using the m_model set
   */

TTree * Analysis::Generate(int nevt, string option)
{
    transform(option.begin(), option.end(), option.begin(), ::tolower);
    if (m_pmode == "v") cout << endl << m_name << ": Generating " << nevt << " events (" << option << ")" << endl;

    if (m_model)
    {
        RooArgSet varList("varList_" + m_name);
        for (auto vv : m_vars) varList.add(*vv);

        /*
        if(option.find("-useroofit")==string::npos)
        {
            TTree * newTree = generate(&varList,m_model,nevt,option);
            newTree->SetName("gen_"+m_name);
            m_reducedTree = newTree;
            if(option.find("-nodataset")==string::npos) GetDataSet("-recalc");
            return m_reducedTree;
        }
        else
        {
        */
        cout << "Generating toys with roofit function" << endl;
        size_t posseed = option.find("-seed");
        if (posseed != string::npos)
        {
            int seed = ((TString)(option.substr(posseed + 5))).Atof();
            RooRandom::randomGenerator()->SetSeed(seed);
        }
        RooCmdArg ext = RooCmdArg::none();
        if (option.find("-genextended") != string::npos) ext = Extended();
        m_data = m_model->generate(varList, nevt, ext);
        cout << m_name << ": " << " Generated events = " << m_data->numEntries() << endl;
        m_data->Print();
        return NULL;
        //}
    }
    else return NULL;
}

TTree * Analysis::Generate(double nsigevt, double nbkgevt, string option)
{
    transform(option.begin(), option.end(), option.begin(), ::tolower);
    cout << fixed << setprecision(3);
    if (m_pmode == "v") cout << endl << m_name << ": Generating " << nsigevt << " signal events and " << nbkgevt << " bkg events (" << option << ")" << endl;

    if (m_sig && m_bkg)
    {
        RooArgSet varList("varList_" + m_name);
        for (auto vv : m_vars) varList.add(*vv);

        /*
        if(option.find("-useroofit")==string::npos) {

            TTree * newTree = generate(&varList,m_sig,nsigevt,m_bkg,nbkgevt,option);
            newTree->SetName("gen_"+m_name);
            m_reducedTree = newTree;
            if(option.find("-nodataset")==string::npos) GetDataSet("-recalc");
            return m_reducedTree;
        }
        else
        {
        */
        cout << "Generating toys with roofit function" << endl;
        size_t posseed = option.find("-seed");
        if (posseed != string::npos)
        {
            int seed = ((TString)(option.substr(posseed + 5))).Atof();
            RooRandom::randomGenerator()->SetSeed(seed);
        }
        RooCmdArg ext = RooCmdArg::none();
        if (option.find("-genextended") != string::npos) ext = Extended();
        TString dataname = "data_" + m_name;
        if (m_data) dataname = m_data->GetName();

        double ntot = nsigevt + nbkgevt;
        double frac = nsigevt / ntot;
        RooRealVar * f_sig = new RooRealVar("f_sig", "f_sig", frac);
        RooAbsPdf * tot = new RooAddPdf("total_pdf", "total_pdf", RooArgSet(*m_sig, *m_bkg), RooArgSet(*f_sig));
        cout << "Generating nsig/ntot = " << f_sig->getVal() << endl;
        m_data = tot->generate(varList, ntot, ext);

        cout << m_name << ": " << " Generated events = " << m_data->numEntries() << endl;

        return NULL;
        //}
    }
    else return NULL;
}



/*
   Function to calculate S-weight for the data set using a m_model specified
   */


RooDataSet * Analysis::CalcSWeightRooFit(unsigned nbins, bool unbinned, string option)
{
    transform(option.begin(), option.end(), option.begin(), ::tolower);
    if (m_pmode == "v") cout << endl << m_name << ": CalcSWeightRooFit " << option << endl;

    RooDataSet::setDefaultStorageType(RooAbsData::Tree);

    bool doInit = false;
    vector<string> comps = { m_nsig->GetName() };
    RooArgSet * yields = new RooArgSet(*m_nsig);
    for (auto bfr : m_bkg_fractions)
    {
        RooRealVar * bfrVar = (RooRealVar*)bfr;
        if (option.find("-freeyields") != string::npos)
        {
            if (((string)typeid(*bfr).name()).find("RooFormulaVar") != string::npos)
            {
                cout << "Free Formula: " << bfr->GetName() << endl;
                bfr = new RooRealVar(bfr->GetName(), bfr->GetName(), bfrVar->getVal(), bfrVar->getVal() / 2., bfrVar->getVal() * 2.);
                doInit = true;
            }
            else
            {
                cout << "Free Var:     " << bfr->GetName() << endl;
                bfrVar->setConstant(0);
            }
        }
        yields->add(*bfrVar);
        comps.push_back(bfrVar->GetName());
    }

    setConstant(m_model, m_var, comps, "except-contains");

    if (doInit) ModelBuilder::Initialize("");

    TString oldname = m_name;
    if (option.find("-nofit") == string::npos)
    {
        m_name += (TString)"_sWRF";
        Fit(nbins, unbinned, option);
    }
    m_name = oldname;

    cout << endl;
    cout << "Calculating sWeights: " << m_model->GetName() << endl;
    cout << endl;

    yields->Print();

    SPlot *sPlot = new SPlot("splot", "splot", *m_data, m_model, *yields);

    m_reducedTree = (TTree*) m_data->store()->tree();

    RooArgList sweights = (RooArgList) sPlot->GetSWeightVars();

    m_data->Print();
    sweights.Print();

    TCanvas *c = new TCanvas();
    gStyle->SetOptStat(0);

    m_reducedTree->Draw(sweights[0].GetName() + (TString)">>hSWeights_" + sweights[0].GetName());

    c->Print((TString) m_name + "_sWeights.pdf");

    m_reducedTree->Draw(m_var->GetName() + (TString)">>hHisto_" + sweights[0].GetName());
    TH1D *hHisto = (TH1D*)gPad->GetPrimitive((TString)"hHisto_" + sweights[0].GetName());

    m_reducedTree->Draw(m_var->GetName() + (TString)">>hSWHisto_" + sweights[0].GetName(), sweights[0].GetName());
    TH1D *hSWHisto = (TH1D*)gPad->GetPrimitive((TString)"hSWHisto_" + sweights[0].GetName());

    hSWHisto->SetLineColor(1);
    hHisto->SetLineColor(4);
    hHisto->Draw("hist");
    hSWHisto->Draw("hist same");

    c->Print((TString) m_name + "_sWeighted.pdf");

    cout << endl;
    cout << "sWeighted " << m_reducedTree->GetEntries() << " entries" << endl;
    cout << "Sum of weights = " << hSWHisto->Integral() << endl;
    cout << endl;

    return m_data;
}



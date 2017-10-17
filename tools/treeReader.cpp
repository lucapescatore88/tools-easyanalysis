#include "treeReader.hpp"


vector<string> TypeDB::names = vector<string>();
vector<string> TypeDB::branchIDs = vector<string>();
bool TypeDB::init = false;

string TreeReader::pmode = "v";
string TreeReader::pfile = "v";


void TreeReader::AddChain(TChain *chain)
{
    if (fChain)
    {
        if (pmode == "v") cout << "TreeReader - Adding chain" << endl << endl;

        TObjArray *fileElements = chain->GetListOfFiles();
        TIter next(fileElements);
        TChainElement *chEl = 0;
        string fileName;
        while ((chEl = (TChainElement*) next()))
        {
            AddFile(chEl->GetTitle());
        }

        Initialize();
    }
}



void TreeReader::AddFile(const char *fileName, const char* treeName, Long64_t maxEntries)
{
    if (fChain)
    {
        if (!TFile::Open(fileName)) return;
        fChain->AddFile(fileName, maxEntries, treeName);
        if (pmode == "v") cout << "TreeReader - Adding file: " << fileName << " " << treeName << endl;
    }
}



void TreeReader::AddFriend(const char *fileName, const char* treeName)
{
    if (fChain)
    {
        if (!TFile::Open(fileName)) return;

        if (pmode == "v") cout << "TreeReader - Adding friend: " << treeName << endl << endl;

        fChain->AddFriend(treeName, fileName);
    }
}



void TreeReader::AddList(const char *fileName)
{
    if (fChain)
    {
        if (!ifstream(fileName).good())
        {
            if (pmode == "v") cout << "TreeReader - No such file or directory: " << fileName << endl;
            return;
        }

        ifstream listfile;
        listfile.open(fileName, ifstream::in);

        while (listfile.good())
        {
            char file[256];
            listfile.getline(file, 256);
            if ((TString) file == "") break;
            AddFile(file);
        }
        listfile.close();
    }
}



TTree * TreeReader::CopyTree(TCut cut, double frac, string name)
{
    if (!init)
    {
        cout << endl << "TreeReader - *** WARNING *** tree " << fChain->GetName() << " not initialized" << endl;
        return NULL;
    }

    if (cut == "1") cut = "";

    Long64_t nTot = fChain->GetEntries();

    if (pmode == "v") cout << "N Tot = " << nTot << endl;
    if ((nTot != 0) && (frac != 0.) && (cut != "") && pmode == "v") cut.Print();

    if (frac == -1 && cut == "")
    {
        cout << "Frac  = " << frac << endl;
        if (name != "") fChain->SetName(name.c_str());
        return (TTree*) fChain;
    }

    Long64_t nTot_ = nTot;
    if (frac == 0)
    {
        cut = "";
        nTot = 0;
    }
    if ((frac > 0) && (frac < 1))    nTot *= frac;
    if ((frac > 1) && (frac < nTot)) nTot = frac;

    if (pmode == "v")
    {
        if (frac != 0)
        {
            cout << "Frac  = " << frac << endl;
            if (frac >= 0) cout << "Copying entries..." << endl;
        }
        else cout << "Cloning tree structure..." << endl;
    }

    TTree *tTree = NULL;
    if (cut == "") tTree = (TTree*) fChain->CloneTree(nTot);
    else           tTree = (TTree*) fChain->CopyTree(cut, "", nTot);

    if (name != "") tTree->SetName(name.c_str());

    if (nTot != 0) if (pmode == "v")
        {
            Long64_t nPas = tTree->GetEntries();
            cout << "N Pas = " << fixed << setprecision(0) << nPas << " (" << fixed << setprecision(6) << (double) nPas / (double) nTot_ * 100 << "%)" << endl << endl;
        }

    return tTree;
}



void TreeReader::PrintListOfFiles()
{
    if (pfile == "") return;

    if (!init)
    {
        cout << endl << "TreeReader - *** WARNING *** tree " << fChain->GetName() << " not initialized" << endl;
        return;
    }

    if (pmode == "v")
    {
        TObjArray *fileElements = fChain->GetListOfFiles();

        if (fileElements) {
            TIter next(fileElements);
            TChainElement *chEl = 0;

            cout << endl << "List of files" << endl;
            while ((chEl = (TChainElement*) next()))
            {
                cout << chEl->GetTitle() << endl;
            }
            cout << endl;
        }
    }
}



void TreeReader::PrintListOfVariables()
{
    if (!init)
    {
        cout << endl << "TreeReader - *** WARNING *** tree " << fChain->GetName() << " not initialized" << endl;
        return;
    }

    if (pmode == "v")
    {
        vector<variable*> varList = GetVarList();

        cout << endl << endl << "Set up " << varList.size() << " branches" << endl << endl;

        for (unsigned i = 0; i < varList.size(); ++i)
        {
            cout << varList[i]->name << " " << varList[i]->bname << " " << varList[i]->title << endl;
        }
        cout << endl;
    }
}



void TreeReader::SetBranchStatus(vector<string> branches, bool status, string opt)
{
    if (!init)
    {
        cout << endl << "TreeReader - *** WARNING *** tree " << fChain->GetName() << " not initialized" << endl;
        return;
    }

    if (opt.find("all") != string::npos)
    {
        if (pmode == "v") cout << "TreeReader - SetBranchStatus " << status << ": ALL" << endl;
        fChain->SetBranchStatus("*", status);
    }

    if (branches.size() != 0) {
        if (pmode == "v") cout << "TreeReader - SetBranchStatus " << status << ": " << fChain->GetName() << " (" << branches.size() << ")" << endl;

        TObjArray *list = fChain->GetListOfBranches();

        int nbranches = 0;
        for (int i = 0; i < list->GetEntries(); ++i)
        {
            TBranch *branch = (TBranch*) list->At(i);
            for (unsigned b = 0; b < branches.size(); ++b)
            {
                if (branches[b] == branch->GetName())
                {
                    fChain->SetBranchStatus(branches[b].c_str(), status);
                    nbranches++;
                }
            }

            if (pmode == "v") cout << "TreeReader - SetBranchStatus " << status << ": " << fChain->GetName() << " (" << nbranches << ")" << endl << endl;
        }
    }
}



bool TreeReader::Initialize(vector <string> br, string opt)
{
    if (!init)
    {
        if ( !fChain )
        {
            cout << endl << "TreeReader - No tree to initialize" << endl << endl;
            return false;
        }

        TObjArray *fileElements = fChain->GetListOfFiles();
        if ( !fileElements || ( fileElements->GetEntries() == 0 ))
        {
            cout << endl << "TreeReader - No file(s) to initialize" << endl << endl;
            return false;
        }
    }

    varList.clear();

    TObjArray* branches = fChain->GetListOfBranches();
    int nBranches = branches->GetEntries();

    for (int i = 0; i < nBranches; ++i)
    {
        TBranch* branch = (TBranch*)branches->At(i);
        string brname = branch->GetName();
        TLeaf* leaf = branch->GetLeaf(branch->GetName());

        if ( leaf == 0 )  // leaf name is different from branch name
        {
            TObjArray* leafs = branch->GetListOfLeaves();
            leaf = (TLeaf*)leafs->At(0);
        }

        string curtype = leaf->GetTypeName();
        int id = TypeDB::getType(curtype.c_str());
        int arreysize = 1;
        string title = leaf->GetTitle();

        // Find out whether we have array by inspecting leaf title
        if ( title.find("[") != std::string::npos )
        {
            TLeaf * nelem = leaf->GetLeafCounter(arreysize);
            if (arreysize == 1 && nelem != NULL) arreysize = nelem->GetMaximum() + 1; //search for maximum value of the lenght
        }


        if (id >= 0)
        {
            bool addVar = true;
            if (curtype.find("vector") != std::string::npos) addVar = false;

            if (br.size() > 0)
            {
                addVar = false;
                for (unsigned b = 0; b < br.size(); b++)
                {
                    if (opt == "names" || opt == "except")
                    {
                        if (br[b] == brname) { addVar = true; break;}
                    }
                    else if (opt.find("contains") != string::npos)
                    {
                        if ((string(brname)).find(br[b]) != string::npos) { addVar = true; break;}
                    }
                    else if (opt.find("except") == string::npos) cout << "TreeReader - Option not found: " << opt << endl;
                }

                if (opt.find("except") != string::npos) addVar = !addVar;
            }

            if (addVar)
            {
                variable * tmpVar = new variable(id, arreysize);

                tmpVar->name = leaf->GetName();
                tmpVar->bname = branch->GetName();
                tmpVar->title = title;

                varList.push_back(tmpVar);
                fChain->SetBranchAddress(tmpVar->bname, tmpVar->value.address);
            }
        }
        else
        {
            cout << "TreeReader - type not found: " << curtype << endl;
            exit(1);
            return false;
        }
    }

    init = true;
    continueSorting = true;

    if ((init) && (pmode == "v"))
    {
        cout << endl << "Number of trees in chain: " << fChain->GetNtrees() << endl;
        cout << endl << "Set up " << varList.size() << " / " << nBranches << " branches" << endl;
        cout << endl << "Read in " << fChain->GetEntries() << " events" << endl << endl;
    }

    return true;
}



void TreeReader::BranchNewTree(TTree* tree)
{
    for (unsigned it = 0; it < varList.size(); ++it)
    {
        variable *var = varList[it];
        string branchID = TypeDB::branchID(var->GetType());
        tree->Branch(var->name, var->value.address, var->title + "/" + branchID);
    }
}



void TreeReader::FillNewTree(TTree* tree, TCut cuts, double frac, void (*addFunc)(TreeReader *, TTree *, bool))
{
    fChain->Draw(">>skim", cuts, "entrylist");
    TEntryList *skim = (TEntryList*)gDirectory->Get("skim");
    if (skim == NULL) return;
    int nEntries = skim->GetN();
    SetEntryList(skim);

    BranchNewTree(tree);
    if (addFunc) addFunc(this, tree, true);

    if (pmode == "v") cout << "N candidates = " << nEntries << endl;
    if (frac > 0 && frac < 1) { nEntries *= frac; if (pmode == "v") cout << "Using only " << 100 * frac << "% of the entries" << endl; }
    else if (frac > 1) { nEntries = frac; if (pmode == "v") cout << "Using only " << frac << " entries" << endl; }

    for (Long64_t i = 0 ; i < nEntries ; i++)
    {
        GetEntry(i, skim);
        if (addFunc) addFunc(this, tree, false);
        tree->Fill();
    }

    SetEntryList(0);
    delete skim;
}



/**
 * Keep statistics of which variables are accessed often
 * and once in a while swap those to early positions in list
 * to get faster finding performances.
 */

bool TreeReader::partialSort()
{
    bool didSwap = false;

    for (unsigned int i = 1; i < varList.size(); ++i)
    {
        int varGets = varList[i]->nGets;

        if ( varGets / nGets >= 0.01 ) // Care only about variables used more often
        {
            for (unsigned j = 0; j < i; ++j)  // Makes sense to swap only to earlier place
            {
                if ( varGets > varList[j]->nGets )
                {
                    variable* tmp = varList[i];
                    varList[i] = varList[j];
                    varList[j] = tmp;
                    didSwap = true;
                }
            }
        }
    }

    return didSwap;
}

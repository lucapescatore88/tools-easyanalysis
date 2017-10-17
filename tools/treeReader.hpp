#ifndef TREEREADER_HPP
#define TREEREADER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <functional>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "TROOT.h"
#include "TBranch.h"
#include "TObjArray.h"
#include "TLeaf.h"
#include "TChain.h"
#include "TTree.h"
#include "TFile.h"
#include "TString.h"
#include "TCut.h"
#include "TEntryList.h"
//#include "TIter.h"
#include "TChainElement.h"

#include "RooRealVar.h"

using namespace std;


typedef enum
{
    BOOL, INT, UINT, FLOAT, DOUBLE, LONG64, ULONG64, CHAR, UCHAR, SHORT, USHORT
} TypeVAR;



/** \class TypeDB
 *  \brief Database of types supported by Root with their branch representation
 *  */


class TypeDB {

    static vector<string> names;
    static vector<string> branchIDs;
    static bool init;

public:

    static void getInstance()
    {
        if (names.size() == 0)
        {
            names.push_back("Bool_t");
            names.push_back("Int_t");
            names.push_back("UInt_t");
            names.push_back("Float_t");
            names.push_back("Double_t");
            names.push_back("Long64_t");
            names.push_back("ULong64_t");
            names.push_back("Char_t");
            names.push_back("UChar_t");
            names.push_back("Short_t");
            names.push_back("UShort_t");
            names.push_back("vector<double>");

            branchIDs.push_back("O");
            branchIDs.push_back("I");
            branchIDs.push_back("i");
            branchIDs.push_back("F");
            branchIDs.push_back("D");
            branchIDs.push_back("L");
            branchIDs.push_back("l");
            branchIDs.push_back("B");
            branchIDs.push_back("b");
            branchIDs.push_back("S");
            branchIDs.push_back("s");
            branchIDs.push_back("");
        }

        init = true;
    }

    static inline string getTypeName( unsigned int id )
    {
        if (id < names.size()) return names[id];
        else return (string)"";
    }

    static inline int getType(const char *name)
    {
        if (!init) { getInstance(); }
        for (unsigned i = 0; i < names.size(); ++i)
        {
            if (names[i] == name) return i;
        }
        return -1;
    }

    static inline string branchID(unsigned id)
    {
        if (!init) { getInstance(); }
        if (id < names.size()) return branchIDs[id];
        return string("");
    }
};


/** \class variable
 *  \brief A class holding an array of values of any type supported by root
 *
 *  The class can hold a pointer to an array of values of any type supported by root
 *  It also contains information on the current type, name, branch name and title of the variable.
 */

class variable {

    unsigned type;
    unsigned arraySize;

    bool setVariable()
    {
        switch (type)
        {
        case BOOL:    { value.myBool   = new Bool_t[arraySize];       break; }
        case INT:     { value.myInt    = new Int_t[arraySize];        break; }
        case UINT:    { value.myUint   = new unsigned int[arraySize]; break; }
        case FLOAT:   { value.myFloat  = new Float_t[arraySize];      break; }
        case DOUBLE:  { value.myDouble = new Double_t[arraySize];     break; }
        case LONG64:  { value.myLong   = new Long64_t[arraySize];     break; }
        case ULONG64: { value.myULong  = new ULong64_t[arraySize];    break; }
        case CHAR:    { value.myChar   = new Char_t[arraySize];       break; }
        case UCHAR:   { value.myUChar  = new UChar_t[arraySize];      break; }
        case SHORT:   { value.myShort  = new Short_t[arraySize];      break; }
        case USHORT:  { value.myUShort = new UShort_t[arraySize];     break; }
        default: return false;
        }

        return true;
    }

public:

    TString name;  ///Variable name
    TString bname; ///Variable branch name
    TString title; ///Variable title

    int nGets;     ///Counter to keep track of how may times the variable is accessed

    union {
        Bool_t    *myBool;
        Int_t     *myInt;
        unsigned  *myUint;
        Float_t   *myFloat;
        Double_t  *myDouble;
        Long64_t  *myLong;
        ULong64_t *myULong;
        Char_t    *myChar;
        UChar_t   *myUChar;
        Short_t   *myShort;
        UShort_t  *myUShort;
        void      *address;
    } value;


    variable(const char *_type, int _arraysize = 1):
        arraySize(_arraysize), nGets(0)
    {
        type = TypeDB::getType(_type);
        setVariable();
    }
    variable( int _type, int _arraysize = 1):
        type(_type), arraySize(_arraysize), nGets(0)
    {
        setVariable();
    }


    void SetArraySize(unsigned size) { arraySize = size; }
    void SetType(unsigned _type) { type = _type; }
    unsigned GetArraySize() { return arraySize; }
    unsigned GetType() { return type; }
    string GetTypeName() { return TypeDB::getTypeName(type); }
    template <typename T = double> T GetValue(int iValue)
    {
        switch (type)
        {
        case BOOL:    { return value.myBool[iValue];   break; }
        case INT:     { return value.myInt[iValue];    break; }
        case UINT:    { return value.myUint[iValue];   break; }
        case FLOAT:   { return value.myFloat[iValue];  break; }
        case DOUBLE:  { return value.myDouble[iValue]; break; }
        case LONG64:  { return value.myLong[iValue];   break; }
        case ULONG64: { return value.myULong[iValue];  break; }
        case CHAR:    { return value.myChar[iValue];   break; }
        case UCHAR:   { return value.myUChar[iValue];  break; }
        case SHORT:   { return value.myShort[iValue];  break; }
        case USHORT:  { return value.myUShort[iValue]; break; }
        default: return -999;
        }
    }

    template < typename T > T *GetPtr()
    {
        switch (type)
        {
        case BOOL:    { return reinterpret_cast<T*>(value.myBool);   break; }
        case INT:     { return reinterpret_cast<T*>(value.myInt);    break; }
        case UINT:    { return reinterpret_cast<T*>(value.myUint);   break; }
        case FLOAT:   { return reinterpret_cast<T*>(value.myFloat);  break; }
        case DOUBLE:  { return reinterpret_cast<T*>(value.myDouble); break; }
        case LONG64:  { return reinterpret_cast<T*>(value.myLong);   break; }
        case ULONG64: { return reinterpret_cast<T*>(value.myULong);  break; }
        case CHAR:    { return reinterpret_cast<T*>(value.myChar);   break; }
        case UCHAR:   { return reinterpret_cast<T*>(value.myUChar);  break; }
        case SHORT:   { return reinterpret_cast<T*>(value.myShort);  break; }
        case USHORT:  { return reinterpret_cast<T*>(value.myUShort); break; }
        default: { cout << "*** WARNING: Type not available!" << endl; return NULL; }
        }
    }
};



class varEq : public std::unary_function<variable*, bool>
{
    string s;
public:
    explicit varEq(const char *&ss): s(ss) {}
    inline bool operator() (const variable *c) const
    { return s.compare(c->name) ?  false : true; }
};


/** \class TreeReader
 *  \brief The class allows to load a ROOT TTree object and easily manipulate information
*/

class TreeReader {

    TChain *fChain;
    ///List of variables in the tree with address pointers (filled by Initialize())
    vector < variable * > varList;

    ///Counter to keep trak of how many times the tree was accessed
    int nGets;
    bool continueSorting;

    bool partialSort();
    bool init;
    bool selected;

    static string pmode;
    static string pfile;

public:


    /// \brief Empty onstructor

    TreeReader():
        nGets(0), continueSorting(true), init(true), selected(false)
    {
        fChain = new TChain();
        return;
    }

    /// \brief Constructor with an existing TTree

    TreeReader(TTree *tree):
        nGets(0), continueSorting(true), init(true), selected(false)
    {
        if (!tree) { cout << "Attention your TTree is NULL" << endl; return; }
        fChain = (TChain*) tree;
        SetPrintFileLevel("");
        Initialize();
        return;
    }

    /// \brief Constructor with an existing TChain

    TreeReader(TChain *chain):
        nGets(0), continueSorting(true), init(true), selected(false)
    {
        if (!chain) { cout << "Attention your TChain is NULL" << endl; return; }
        fChain = chain;
        SetPrintFileLevel("");
        Initialize();
        return;
    }

    /// \brief Constructor with the name of a tree (then you can add many filesusing AddFile(namefile) )

    TreeReader(const char *treeName):
        fChain(new TChain()), nGets(0), continueSorting(true), init(false), selected(false)
    {
        if (treeName) fChain->SetName(treeName);
    }

    /// \brief Constructor with the name of a TTree "treeName" contained in one single file "fileName"

    TreeReader(const char *treeName, const char *fileName):
        fChain(new TChain()), nGets(0), continueSorting(true), init(false), selected(false)
    {
        fChain->SetName(treeName);
        AddFile(fileName);
        Initialize();
    }

    ~TreeReader()
    {
        //if(fChain) delete fChain;
        varList.clear();
    }

    ///Sets the flag to enable verbose mode "v" or silent mode "s"

    static void SetPrintLevel( string pm ) { pmode = pm; }
    static string GetPrintLevel( ) { return pmode; }
    static void SetPrintFileLevel( string pm ) { pfile = pm; }
    static string GetPrintFileLevel( ) { return pfile; }

    /** \brief Function to add files to the internal TChain
     *
     * Remember to initialize after adding new Files!
     *
     *  */

    void AddChain(TChain *chain);

    /** \brief Function to add a chain to the internal TChain
     *  */

    void AddFile(const char *fileName, const char *treeName = "", Long64_t maxEntries = -1);

    /** \brief Function to add a friend to the internal TChain
     *  */

    void AddFriend(const char *fileName, const char *treeName = "");

    /** \brief Function to add a list of files to the internal TChain
     *
     * Remember to initialize after adding new Files!
     *  */

    void AddList(const char *fileName);

    Long64_t GetEntries() { return fChain->GetEntries(); }
    inline int GetEntry(Long64_t ientry) { if (!selected) selected = true; return fChain->GetEntry(ientry); }

    /** \brief Selects a new Entry.
     * If "list" is passed the ith entry is intended in the list not in te full tree
     */

    inline int GetEntry(Long64_t ientry, TEntryList *list)
    {
        int treenum, iEntry, chainEntry;
        treenum = iEntry = chainEntry = -1;

        if (!list) return GetEntry(ientry);
        iEntry = list->GetEntryAndTree(ientry, treenum);
        chainEntry = iEntry + (fChain->GetTreeOffset())[treenum];
        return GetEntry(chainEntry);
    }

    /**  \brief Creates and fills a new tree
     *  @param cuts:    Fills only entries with pass "cuts"
     *  @param frac:    Keeps only a fraction "frac" of the total events
     *  @param name:    Name given to the new tree
     *  */

    TTree *CopyTree(TCut cuts = "", double frac = -1., string name = "");

    /** \brief Adds to "tree" all branches of the stored tree (and sets addresses) */

    TTree *CloneTree(string name = "") { return CopyTree("", 0., name); }

    /// \brief Print the list of files linked by the internal TChain object

    void PrintListOfFiles();

    /// \brief Print the list of variables in the internal TChain object

    void PrintListOfVariables();

    /// \brief Set the branch status to Process or DoNotProcess for the internal TChain object

    void SetBranchStatus(vector<string> branches, bool status = true, string opt = "");

    /** \brief Reads the tree and stores information on variables
     *
     * @param br:  Contains a list of branch name (also partial names)
     * @param opt: Defines what to do with the "br" names list
     *             <br> opt = "names"            -> keeps only branches with exact names in br
     *             <br> opt = "contains"         -> keeps branches with names or partial names in br
     *             <br> opt = "except"           -> keeps all branches whose exact names are not in br
     *             <br> opt = "exceptcontains"   -> keeps all branches whose names don't contain (even partially) names in br
     */

    bool Initialize(vector <string> br = vector <string>(), string opt = "names");

    /** \brief Adds to "tree" all branches of the stored tree (and sets addresses)
     *  Same as "CloneTree()" which is preferred. This is kept for backward compatibility.
     *  */

    void BranchNewTree(TTree *tree);

    /** \brief Fills a new tree
     * @param cuts:    Fills only entries with pass "cuts"
     * @param frac:    Keeps only a fraction "frac" of the total events
     * @param addFunc: This function can be defined outside and given to the loop to add new variables.
     * */
    void FillNewTree(TTree *tree, TCut cuts = "", double frac = -1, void (*addFunc)(TreeReader *, TTree *, bool) = NULL);


    /// \brief Returns the internal TChain object

    TChain *GetChain() { return fChain; }
    vector < variable * >  GetVarList() { return varList; }
    float GetNVars() { return varList.size(); }

    /** \brief Returns the value of the "iValue'th" (by default 0) component of the variable "name".
    * It requires that an entry is selected first using "GetEntry()"
    * */

    template <typename T = double> T GetValue(const char *name, int iValue = 0)
    {
        if (!init) { cout << "*** WARNING: tree " << fChain->GetName() << " not initialized" << endl; return 0; }
        if (!selected) { cout << "*** WARNING: no entry selected" << endl; return 0; }

        ++nGets;
        if ( continueSorting && (nGets % 1000 == 0) ) continueSorting = partialSort();

        variable *myVar = GetVariable(name);
        if (!myVar) return -999;
        ++(myVar->nGets);

        return (T)myVar->GetValue<T>(iValue);
    }

    /** \brief Sets the value of the zero component of the variable "name" to "value".
    * It requires that an entry is selected first using "GetEntry()"
    * N.B.: It's the user responsibility to give a value of type corresponding to the variable type.
    * e.g. If "var" is stored as a float and you give a double instead it will break.
    * */

    template < typename T > void SetValue(const char *name, T value)
    {
        T *ptr = GetVariable(name)->GetPtr<T>();
        if (ptr) ptr[0] = value;
        else cout << "No pointer available for variable " << name << endl;
    }

    /// \brief Checks is the variable "namevar" is stored in the reader

    inline bool HasVar(const char *namevar) { return (bool)GetVariable(namevar); }
    bool HasVars(vector<string>vars)
    {
        bool res = true;
        for (auto v : vars) res *= HasVar(v.c_str());
        return res;
    }
    bool HasVars(vector<RooRealVar*>vars)
    {
        bool res = true;
        for (auto v : vars) res *= HasVar(v->GetName());
        return res;
    }
    inline bool isValid() { return init; }

    /* \brief Returns the pointer to the variable object with name "name" for the current entry
    * Using the varable class interface you can read/write the name, type and value.
    */

    inline variable *GetVariable(const char *name)
    {
        vector<variable *>::iterator it = find_if(varList.begin(), varList.end(), varEq(name));
        if ( it == varList.end() )
        {
            if (pmode == "v") cout << "Trying to access non-existing variable with name " << name << endl;
            return NULL;
        }
        else return *it;
    }

    inline string GetVariableType(const char *name)
    {
        return GetVariable(name)->GetTypeName();
    }

    void ShowNGets() { for (unsigned i = 0; i < varList.size(); ++i) cout << varList[i]->nGets / (double)nGets << endl; }

    /// \brief Allows to set an entry list so that the reader will loop only on a subset of entries

    void SetEntryList(TEntryList *list) { fChain->SetEntryList(list); }
    void GetNfiles() { fChain->GetNtrees(); }
};

#endif

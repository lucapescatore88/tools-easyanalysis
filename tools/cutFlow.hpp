#ifndef CUTFLOW_HPP
#define CUTFLOW_HPP

#include <vector>
#include <map>

#include "TCut.h"
#include "TString.h"
#include "TEfficiency.h"

#include "treeReader.hpp"
#include "efficiencyFunctions.hpp"

using namespace std;


typedef  double (*FUNC_CALC_EFF)(TString, TString, TChain *, TCut, TChain *, TCut, TString, TString, double *, TEfficiency **);

double get_integral_after_cut(TString name, TString plot, TTree * tree, TCut cuts, TString weight, TH1 ** hist = NULL);
double get_efficiency_default(TString name, TString plot, TChain * tree, TCut cuts, TChain * downTree, TCut downCuts,
                              TString weight, TString downWeight = "", double *efferr = NULL, TEfficiency ** heff = NULL);
double get_eff_one(TString name, TString plot, TChain * tree, TCut cuts, TChain * downTree, TCut downCuts,
                   TString weight, TString downWeight = "", double *efferr = NULL, TEfficiency ** heff = NULL);
TEfficiency * get_TEfficiency(TH1 * hpass, TH1* htot);


class EffComp {

    static int mVerbose;

public :

    TString mName;                     /// Name of the cut
    TString mLabel;

    map<TString, double> mValues;      /// Value of the efficiency for each sample
    map<TString, double> mErrors;

    map<TString, TCut> mCuts;          /// Samples to which the cut is applied with a cut formula for each sample
    map<TString, TCut> mBaseCuts;
    map<TString, FUNC_CALC_EFF> mFuncs;
    map<TString, TChain*> mCut_tree;
    map<TString, TChain*> mTot_tree;
    map<TString, TString> mWeights;
    map<TString, TString> mTot_weights;

public:

    EffComp(TString name, vector<TCut> formula, vector <TString> samples,
            vector <TChain * > trees = vector <TChain * >(), vector <TChain * > tot_trees = vector <TChain * >(),
            vector <TCut> basecuts = vector <TCut >(),
            vector <TString > weights = vector <TString >(), vector <TString > tot_weights = vector <TString >()
           );

    EffComp(TString name, map<TString, TCut> samples, map<TString, TCut> basecuts,
            map<TString, TChain*> trees, map<TString, TChain*> tot_trees,
            map<TString, TString> weights, map<TString, TString> tot_weights = map<TString, TString>())
        : mName (name), mCuts (samples), mBaseCuts (basecuts),
          mCut_tree (trees), mTot_tree(tot_trees),
          mWeights (weights), mTot_weights(tot_weights)
    {
        for (auto s : samples)
        {
            if (s.second == "")
            { mValues[s.first] = mErrors[s.first] = 1.; mFuncs[s.first] = &get_eff_one; }
            else
            { mValues[s.first] = mErrors[s.first] = -1.; mFuncs[s.first] = &get_efficiency_default; }
        }
    }

    static void SetPrintLevel(int plevel) { mVerbose = plevel; }

    map<TString, double> CalcEfficiencies(TString plot, string option = "-join", TFile * mFile = NULL);
    map<TString, double> CalcEfficiencies(vector<TString> plot, string option, TFile * ofile);
    double CalcEffRatios(TString up, TString down);

    void Print();

};


class CutFlow {

    TString mName;
    vector<TString> mSamples;
    map<TString, TString> mLabels;
    vector<TString> mEffs_order;
    map<TString, EffComp*> mEffs;
    TFile * mFile;

    EffComp * GetComponent(TString cut)
    {
        if (cut == "Total" || cut == "total" || cut == "Tot" || cut == "tot") cut = "__Total";
        if (HasCut(cut)) return mEffs[cut];
        else { cout << "ATTENTION: No cut with name " << cut << " set! " << endl; return NULL; }
    }

public:

    CutFlow(TString name)
    {
        mName = name;
        mFile = new TFile(name.ReplaceAll(".root", "") + ".root", "recreate");
        mFile->mkdir("histos");
    }
    void AddSample(TString sample) { mSamples.push_back(sample); }

    void AddCut(TString name, TCut formula, vector <TString> samples = vector <TString>(), TChain * tree = NULL,
                TString weight = "", TChain * downTree = NULL, TCut baseCut = "");
    void AddCut(TString name, map<TString, TCut> formula, TCut baseCut = "", TString weight = "", TChain * tree = NULL, TChain * downTree = NULL);
    void AddCut(TString name, TCut formula, TCut baseCut, vector <TString> samples = vector <TString>(), TString weight = "")
    {
        AddCut(name, formula, samples, (TChain*)NULL, weight, (TChain*)NULL, baseCut);
    }
    void AddCut(TString name, map<TString, TCut> cuts, map<TString, TCut> basecuts,
                map<TString, TString> weights = map<TString, TString>(), map<TString, TString> tot_weights = map<TString, TString>(),
                map<TString, TChain * > trees = map<TString, TChain*>(), map<TString, TChain*> downtrees = map<TString, TChain*>());
    void AddCut(TString name, vector<TCut> formula, vector <TString> samples, vector <TChain * > trees = vector <TChain * >(),
                vector <TCut> basecuts = vector <TCut >(), vector <TChain * > tot_trees = vector <TChain * >(),
                vector <TString > weights = vector <TString >(), vector <TString > tot_weights = vector <TString >());


    void SetTrees(TString nameeff, TString fileup, TString nameup, TString filedown = "", TString namedown = "");
    void SetTrees(TString nameeff, vector <TString> fileup, TString nameup, vector <TString> filedown = vector <TString>(), TString namedown = "");

    void SetFunction(TString nameeff, FUNC_CALC_EFF myfunc);
    void SetFunction(TString nameeff, TString namesample, FUNC_CALC_EFF myfunc);
    void SetSampleLabel(TString sample, TString label) { mLabels[sample] = label; }
    void SetSampleLabels(vector <TString> labels)
    {
        if (labels.size() == mSamples.size())
            for (size_t s = 0; s < mSamples.size(); s++)
                mLabels[mSamples[s]] = labels[s];
        else cout << "ATTENTION: label list must have the same size as the samples one." << endl;
    }
    void SetEffLabel(TString comp, TString label) { mEffs[comp]->mLabel = label; }
    void SetEffLabels(vector <TString> labels)
    {
        if (labels.size() == mEffs.size())
            for (size_t s = 0; s < mEffs.size(); s++)
                mEffs[mEffs_order[s]]->mLabel = labels[s];
        else cout << "ATTENTION: label list must have the same size as the efficiencies one." << endl;
    }


    map<TString, TCut> * GetCuts(TString cut)           { EffComp * c = GetComponent(cut); return c ? &c->mCuts : NULL; }
    map<TString, TCut> * GetBaseCuts(TString cut)       { EffComp * c = GetComponent(cut); return c ? &c->mBaseCuts : NULL; }
    map<TString, TChain*> * GetTreeCuts(TString cut)    { EffComp * c = GetComponent(cut); return c ? &c->mCut_tree : NULL; }
    map<TString, TChain*> * GetBaseTreeCuts(TString cut) { EffComp * c = GetComponent(cut); return c ? &c->mTot_tree : NULL; }
    map<TString, TString> * GetWeights(TString cut)     { EffComp * c = GetComponent(cut); return c ? &c->mWeights : NULL; }
    map<TString, TString> * GetBaseWeights(TString cut) { EffComp * c = GetComponent(cut); return c ? &c->mTot_weights : NULL; }
    map<TString, double> * GetEffValues(TString cut)    { EffComp * c = GetComponent(cut); return c ? &c->mValues : NULL; }
    map<TString, double> * GetEffErrors(TString cut)    { EffComp * c = GetComponent(cut); return c ? &c->mErrors : NULL; }
    double GetEff(TString eff, TString sample, double * err = NULL);
    double GetEffRatio(TString eff, TString sup, TString sdown, double * err = NULL);

    bool HasCut(TString name);

    void CalcEfficiencies(TString plot, TString eff = "",  string opt = "-join")
    {
        cout << "Calculating efficiencies... [ " << plot << " ]" << endl;
        if (HasCut(eff)) { mEffs[eff]->CalcEfficiencies(plot, opt, mFile); return;  }
        for (auto e : mEffs) if (e.first != "__Total") e.second->CalcEfficiencies(plot, opt, mFile);
        CalcTotals();
    }
    void CalcEfficiencies(vector<TString> plot, TString eff = "",  string opt = "-join")
    {
        for (auto p : plot) CalcEfficiencies(p, eff, opt);
        CalcTotals();
    }
    void CalcEffRatios(TString up, TString down) { for (auto e : mEffs) e.second->CalcEffRatios(up, down); }
    void CalcTotals();

    void Print() { for (auto e : mEffs_order) mEffs[e]->Print(); if (mEffs.find("__Total") != mEffs.end()) mEffs["__Total"]->Print(); }
    void MakeTable(TString fname = "", string option = "");
    void SaveToFile(TString filename = "", string option = "");
    void LoadFromFile(TString filename, string option = "");
};

#endif

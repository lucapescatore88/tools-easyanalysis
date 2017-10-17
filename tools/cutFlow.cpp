#include "cutFlow.hpp"


int EffComp::mVerbose = 1;

double get_integral_after_cut(TString name, TString plot, TTree * tree, TCut cuts, TString weight, TH1 ** hist)
{
    static unsigned id = 0;

    if (!tree) return 2.;

    TString exp = plot + Form(">>h_%i", id);
    if (plot.Contains(">>")) exp = plot.ReplaceAll(">>", Form(">>h_%i", id));
    size_t start = ((string)exp).find(">>");
    size_t end = ((string)exp).find("(", start);
    TString hname = (TString)( ((string)exp).substr(start + 2, end - start - 2) );
    id++;

    TString selection = buildSelectStr(cuts, weight);
    tree->Draw(exp, selection);
    (*hist) = (TH1 *)gPad->GetPrimitive(hname);
    return (*hist)->Integral(0, (*hist)->GetNbinsX() + 1);
}

double get_eff_one(TString name, TString plot, TChain * tree, TCut cuts,
                   TChain * downTree, TCut downCuts, TString weight, TString downWeight, double *efferr, TEfficiency ** heff)
{
    (*efferr) = 0.;
    return 1.;
}


TEfficiency * get_TEfficiency(TH1 * hpass, TH1* htot)
{
    hpass->Scale(htot->GetEntries() / htot->Integral());
    htot->Scale(htot->GetEntries() / htot->Integral());
    int nbins = htot->GetNbinsX();
    if (hpass->GetNbinsX() != htot->GetNbinsX()) { cout << "Histograms have different numbers of bins" << endl; return NULL; }
    if (hpass->GetBinLowEdge(1) != htot->GetBinLowEdge(1) ||
            hpass->GetBinLowEdge(nbins + 1) != htot->GetBinLowEdge(nbins + 1)) { cout << "Histograms have different ranges" << endl; return NULL; }

    static unsigned id = 0;
    TH1I * myhpass = new TH1I(Form("myhpass_%i", id), "myhpass", nbins, hpass->GetBinLowEdge(1), hpass->GetBinLowEdge(nbins + 1));
    TH1I * myhtot = new TH1I(Form("myhtot_%i", id), "myhtot", nbins, hpass->GetBinLowEdge(1), hpass->GetBinLowEdge(nbins + 1));
    for (int b = 1; b <= nbins; b++)
    {
        myhpass->SetBinContent(b, hpass->GetBinContent((int)round(b)));
        myhtot->SetBinContent(b, htot->GetBinContent((int)round(b)));
        if (hpass->GetBinContent((int)round(b)) > htot->GetBinContent((int)round(b))) myhpass->SetBinContent(b, 0.);
    }
    id++;
    return (new TEfficiency(*myhpass, *myhtot));
}


double get_efficiency_default(TString name, TString plot, TChain * tree, TCut cuts,
                              TChain * downTree, TCut downCuts, TString weight, TString downWeight, double *efferr, TEfficiency ** heff)
{
    TH1 * hnom = NULL, * hdenom = NULL;
    double nNom = get_integral_after_cut(name + "nom", plot, tree, cuts, weight, &hnom);
    if (!downTree) downTree = tree;
    double nDenom = get_integral_after_cut(name + "denom", plot, downTree, downCuts, downWeight, &hdenom);
    double totentries = hdenom->GetEntries();

    if (hnom && hdenom) (*heff) = get_TEfficiency(hnom, hdenom);
    double eff = nNom / nDenom;
    if (efferr) (*efferr) = TMath::Sqrt(eff * (1. - eff) / totentries);
    return eff;
}


EffComp::EffComp(TString name, vector<TCut> cuts, vector <TString> samples,
                 vector <TChain * > trees, vector <TChain * > tot_trees,
                 vector <TCut> basecuts,
                 vector <TString > weights, vector <TString > tot_weights
                ) : mName (name)
{
    size_t size = samples.size();
    if (cuts.size() != size) { cout << "Number of formulas different than number of samples" << endl; }
    else
    {
        for (size_t pos = 0; pos < size; pos++)
        {
            TString s = samples[pos];

            mCuts[s]        = cuts[pos];
            mBaseCuts[s]    = (trees.size() == size) ? basecuts[pos] : (TCut)"";
            mFuncs[s]       = (cuts[pos] != "") ? &get_efficiency_default : &get_eff_one;
            mCut_tree[s]    = (trees.size() == size) ? trees[pos] : (TChain*)NULL;
            mTot_tree[s]    = (tot_trees.size() == size) ? tot_trees[pos] : (TChain*)NULL;
            mWeights[s]     = (weights.size() == size) ? weights[pos] : (TString)"";
            mTot_weights[s] = (tot_weights.size() == size) ? tot_weights[pos] : (TString)"";

            mValues[s]      = (cuts[pos] != "") ? -1. : 1.;
            mErrors[s]      = (cuts[pos] != "") ? -1. : 0.;
        }
    }
}

map<TString, double> EffComp::CalcEfficiencies(vector<TString> plot, string option, TFile * ofile)
{
    map<TString, double> res;
    for (auto p : plot) res = CalcEfficiencies(p, option, ofile);
    return res;
}

map<TString, double> EffComp::CalcEfficiencies(TString plot, string option, TFile * ofile)
{
    for (auto c : mCuts)
    {
        TString cursample = c.first;
        if (mVerbose)
        {
            cout << "***********************************************" << endl;
            cout << "Calculating efficiency for cut " << mName << " on sample " << cursample << endl;
            cout << "                  Cut --->  ";
            c.second.Print();
            cout << "  With respect of Cut --->  ";
            mBaseCuts[cursample].Print();
            if (mTot_tree[cursample] && mCut_tree[cursample]) {
                cout << "Nominator tree   : " << mCut_tree[cursample]->GetName() << endl;
                cout << "Denominator tree : " << mTot_tree[cursample]->GetName() << endl;
            }
            cout << "Weight : " << mWeights[cursample] << endl;
            cout << "Function : " << mFuncs[cursample] << endl;
            cout << "Working... " << endl;
        }

        if (option.find("-sameweight") != string::npos) mTot_weights[cursample] = mWeights[cursample];

        TEfficiency * heff = NULL;
        double err;
        TCut upcut = mCuts[cursample];
        if (option.find("-join") != string::npos) upcut += mBaseCuts[cursample];
        mValues[cursample] = mFuncs[cursample](mName + "_" + cursample, plot,
                                               mCut_tree[cursample], upcut,
                                               mTot_tree[cursample], mCuts[cursample],
                                               mWeights[cursample], mTot_weights[cursample], &err, &heff);
        mErrors[cursample] = err;

        if (heff && ofile)
        {
            ofile->cd("histos");
            heff->Draw("AL");
            TGraphAsymmErrors *gr = (TGraphAsymmErrors*) heff->CreateGraph();
            if (gr) gr->Write("eff_" + mName + "_vs_" + plot);
            heff->Write("hist_eff_" + mName + "_vs_" + plot);
            ofile->cd();
        }

        if (mVerbose) cout << "Efficiency = " << mValues[cursample] << " +/- " << mErrors[cursample] << endl;
    }

    return mValues;
}

double EffComp::CalcEffRatios(TString up, TString down)
{
    if (mCuts.find(up) != mCuts.end() && mCuts.find(down) != mCuts.end())
    {
        mValues["__ratio__" + up + "_over_" + down] = mValues[up] / mValues[down];
        mErrors["__ratio__" + up + "_over_" + down] = mValues[up] / mValues[down] * TMath::Sqrt( TMath::Power(mErrors[up] / mValues[up], 2) + TMath::Power(mErrors[down] / mValues[down], 2));

        return mValues["__ratio__" + up + "_over_" + down];
    }
    else { cout << "ATTENTION: One of the samples is non exsisting" << endl; return 0.; }
}



void EffComp::Print()
{
    cout << "\n\n\n-------------------------------------------------" << endl;
    cout << "Cut : " << mName << endl;
    cout << "-------------------------------------------------" << endl;

    for (auto c : mCuts)
    {
        TString cursample = c.first;
        cout << "-------------------------------------------------" << endl;
        cout << "Applied on sample : " << cursample << endl;
        cout << "Cut under test : ";
        mCuts[cursample].Print();
        cout << "Base cut : ";
        mBaseCuts[cursample].Print();
        if (mTot_tree[cursample] && mCut_tree[cursample])
        {
            cout << "Nominator tree : " << mCut_tree[cursample]->GetName() << endl;
            cout << "Denominator tree : " << mTot_tree[cursample]->GetName() << endl;
        }
        else cout << "No trees set for this cut and sample" << endl;
        cout << "Weight : " << mWeights[cursample] << endl;
        cout << "BaseWeight : " << mTot_weights[cursample] << endl;
        cout << "Efficiency : " << mValues[cursample] << " +/- " << mErrors[cursample] << endl;
    }
}






bool CutFlow::HasCut(TString name)
{
    if (mEffs.find(name) != mEffs.end()) return true;
    return false;
}

void CutFlow::AddCut(TString name, vector<TCut> cuts, vector <TString> samples,
                     vector <TChain * > trees, vector <TCut> basecuts, vector <TChain * > tot_trees,
                     vector <TString > weights, vector <TString > tot_weights)
{
    map<TString, double> myvalues, myerrors;
    map<TString, TCut> mycuts, mybasecuts;
    map<TString, TChain*> mytrees, mytot_trees;
    map<TString, TString> myweights, mytot_weights;
    size_t size = samples.size();
    EffComp * prev_cut = NULL;
    if (mEffs.size() > 0) prev_cut = mEffs[mEffs_order.back()];

    for (size_t ss = 0; ss < size; ss++)
    {
        TString s = samples[ss];

        mycuts[s] = (cuts.size() == size) ?  cuts[ss] : (TCut)"";

        if (basecuts.size() == size) mybasecuts[s] = basecuts[ss];
        else mybasecuts[s] = prev_cut ? (prev_cut->mCuts[s] + prev_cut->mBaseCuts[s]) : (TCut)"";

        if (trees.size() == size) mytrees[s] = trees[ss];
        else mytrees[s] = prev_cut ? prev_cut->mCut_tree[s] : (TChain*)NULL;

        if (tot_trees.size() == size) mytot_trees[s] = tot_trees[ss];
        else mytot_trees[s] = prev_cut ? prev_cut->mCut_tree[s] : (TChain*)NULL;

        if (weights.size() == size) myweights[s] = weights[ss];
        else myweights[s] = prev_cut ? prev_cut->mWeights[s] : "";

        if (tot_weights.size() == size) mytot_weights[s] = tot_weights[ss];
        else mytot_weights[s] = prev_cut ? prev_cut->mTot_weights[s] : "";
    }

    AddCut(name, mycuts, mybasecuts, myweights, mytot_weights, mytrees, mytot_trees);
}

void CutFlow::AddCut(TString name, map<TString, TCut> cuts, map<TString, TCut> basecuts,
                     map<TString, TString> weights,  map<TString, TString> tot_weights,
                     map<TString, TChain * > trees,  map<TString, TChain*> downtrees)
{
    for (auto s : mSamples)
    {
        if (cuts.find(s) == cuts.end()) cuts[s] = "";
        if (basecuts.find(s) == basecuts.end()) basecuts[s] = "";
        if (weights.find(s) == weights.end()) weights[s] = "";
        if (tot_weights.find(s) == tot_weights.end()) tot_weights[s] = "";
        if (trees.find(s) == trees.end()) trees[s] = NULL;
        if (downtrees.find(s) == downtrees.end()) downtrees[s] = NULL;
    }

    mEffs[name] = new EffComp(name, cuts, basecuts, trees, downtrees, weights, tot_weights);
    if (name != "__Total") mEffs_order.push_back(name);
}

void CutFlow::AddCut(TString name, map<TString, TCut> cuts, TCut baseCut, TString weight, TChain * tree, TChain * downTree)
{
    map<TString, TCut> prev_cut;
    map<TString, TChain*> trees, tot_trees;
    map<TString, TString> weights, tot_weights;

    if (mEffs.size())
    {
        for (auto c : cuts)
        {
            TString s = c.first;
            EffComp * prev_eff = mEffs[mEffs_order.back()];

            prev_cut[s] = (baseCut != "") ? baseCut : (TCut)(prev_eff->mCuts[s] + prev_eff->mBaseCuts[s]);
            trees[s] = (tree) ? tree : prev_eff->mCut_tree[s];
            tot_trees[s] = (downTree) ? downTree : prev_eff->mCut_tree[s];
            weights[s] = weight;
            tot_weights[s] = weight;
        }
    }
    else for (auto c : cuts) prev_cut[c.first] = baseCut;

    AddCut(name, cuts, prev_cut, weights, tot_weights, trees, tot_trees);
}


void CutFlow::AddCut(TString name, TCut formula, vector <TString> samples,
                     TChain * tree, TString weight, TChain * downTree, TCut baseCut)
{
    if (samples.size() == 0) samples = mSamples;

    map<TString, TCut> cuts;
    for (auto s : samples) cuts[s] = formula;

    AddCut(name, cuts, baseCut, weight, tree, downTree);
}

void CutFlow::SetTrees(TString nameeff, TString fileup, TString nameup, TString filedown, TString namedown)
{
    if (filedown == "") filedown = fileup;
    SetTrees(nameeff, vector<TString>(1, fileup), nameup, vector<TString>(1, filedown), namedown);
}

void CutFlow::SetTrees(TString nameeff, vector<TString> fileup, TString nameup, vector<TString> filedown, TString namedown)
{
    if (namedown == "") namedown = nameup;
    if (filedown.size() == 0) filedown = fileup;
    if (!HasCut(nameeff)) return;

    for (auto s : mEffs[nameeff]->mCuts)
    {
        mEffs[nameeff]->mCut_tree[s.first] = new TChain(nameup);
        for (auto f : fileup) mEffs[nameeff]->mCut_tree[s.first]->AddFile(f);

        mEffs[nameeff]->mTot_tree[s.first] = new TChain(namedown);
        for (auto f : filedown) mEffs[nameeff]->mTot_tree[s.first]->AddFile(f);
    }
}

void CutFlow::SetFunction(TString nameeff, FUNC_CALC_EFF myfunc)
{
    if (!HasCut(nameeff)) return;

    for (auto s : mEffs[nameeff]->mCuts)
        mEffs[nameeff]->mFuncs[s.first] = myfunc;
}


void CutFlow::SetFunction(TString nameeff, TString namesample, FUNC_CALC_EFF myfunc)
{
    if (!HasCut(nameeff)) return;

    for (auto s : mEffs[nameeff]->mCuts)
        if (s.first == namesample) mEffs[nameeff]->mFuncs[s.first] = myfunc;
}



void CutFlow::MakeTable(TString fname, string option)
{
    cout << "Making tables... " << fname << endl;

    if (fname == "") fname = "tables_" + mName;
    ofstream tab_file;
    tab_file.open(fname.ReplaceAll(".tex", "") + ".tex");

    tab_file << "$Eff.$ & ";
    for (auto s : mSamples)
        tab_file << " $ " << (string)((mLabels.find(s) == mLabels.end()) ? s : mLabels[s] ) << "$ & ";
    tab_file.seekp((long)tab_file.tellp() - 2);
    tab_file << " \\\\ \\hline \n ";

    for (auto e : mEffs_order)
    {
        tab_file << " $" << (string)((mEffs[e]->mLabel == "") ? mEffs[e]->mName : mEffs[e]->mLabel) << "$ & ";
        for (auto s : mSamples)
            tab_file << "$ " << mEffs[e]->mValues[s] << " \\pm " << mEffs[e]->mErrors[s] << "$ & ";
        tab_file.seekp((long)tab_file.tellp() - 2);
        tab_file << " \\\\\n ";
    }
    tab_file << " \\hline \n ";
    if (mEffs.find("__Total") != mEffs.end())
    {
        tab_file << "Tot & ";
        for (auto s : mSamples)
            tab_file << "$ " << mEffs["__Total"]->mValues[s] << " \\pm " << mEffs["__Total"]->mErrors[s] << "$ & " << endl;
        tab_file.seekp((long)tab_file.tellp() - 2);
        tab_file << " \\\\ \\hline\n ";
    }
}

void CutFlow::SaveToFile(TString filename, string option)
{
    if (filename == "") filename = "cutflow_" + mName;
    ofstream outfile;
    outfile.open(filename + ".eff");

    cout << "Saving CutFlow to file... " << filename << endl;

    for (auto e : mEffs)
    {
        EffComp * comp = e.second;
        outfile << "-----Eff " << comp->mName << " \n";
        for (auto s : mSamples)
        {
            outfile << "--Sample " + s << " \n";
            outfile << "Cut " << comp->mCuts[s] << " \n";
            if ( comp->mBaseCuts[s] != "" ) outfile << "BaseCut " << comp->mBaseCuts[s] << " \n";
            if ( comp->mValues[s] != -1 )   outfile << "Value " << comp->mValues[s] << " \n";
            if ( comp->mErrors[s] != -1 )   outfile << "Error " << comp->mErrors[s] << " \n";
            if ( comp->mWeights[s] != "" )  outfile << "Weight " << comp->mWeights[s] << " \n";
            if ( comp->mTot_weights[s] != "" )  outfile << "BaseWeight " << comp->mTot_weights[s] << " \n";
            if ( comp->mCut_tree[s] )     outfile << "Tree: " << comp->mCut_tree[s]->GetName() << " \n";
            if ( comp->mTot_tree[s] )     outfile << "BaseTree: " << comp->mTot_tree[s]->GetName() << " \n";
        }
        outfile << " ------------------------- \n ";
    }
    outfile.close();
}

void CutFlow::LoadFromFile(TString filename, string option)
{
    cout << "Loading CutFlow from file... " << filename << endl;

    mSamples.clear();
    mEffs.clear();
    mEffs_order.clear();

    ifstream input;
    input.open(filename);
    bool first = true;
    TString name;
    map<TString, double>values, errors;
    map<TString, TCut>cuts, basecuts;
    map<TString, TString>weights, tot_weights;
    string line;
    while (!input.eof())
    {
        getline(input, line);

        if (line.find("-----Eff") != string::npos) name = ((TString)line.substr(9, string::npos)).ReplaceAll(" ", "");
        if (line.find("--Sample") != string::npos && first) mSamples.push_back(((TString)line.substr(9, string::npos)).ReplaceAll(" ", ""));
        if (line.find("Value") != string::npos) values[mSamples.back()] = ((TString)line.substr(6, string::npos)).ReplaceAll(" ", "").Atof();
        if (line.find("Error") != string::npos) errors[mSamples.back()] = ((TString)line.substr(6, string::npos)).ReplaceAll(" ", "").Atof();
        if (line.find("Weight") != string::npos) weights[mSamples.back()] = (TString)line.substr(7, string::npos);
        if (line.find("TotWeight") != string::npos) tot_weights[mSamples.back()] = (TString)line.substr(10, string::npos);
        if (line.find("Cut") != string::npos) cuts[mSamples.back()] = (TCut)((TString)line.substr(4, string::npos));
        if (line.find("BaseCut") != string::npos) basecuts[mSamples.back()] = (TCut)((TString)line.substr(8, string::npos));

        if (line.find("----------") != string::npos)
        {
            first = false;
            AddCut(name, cuts, basecuts, weights, tot_weights);
            for (auto s : mSamples) (*GetEffValues(name))[s] = values[s];
            for (auto s : mSamples) (*GetEffErrors(name))[s] = errors[s];
        }
    }

    cout << "CutFlow fully loaded! Enjoy!" << endl;
}


void CutFlow::CalcTotals()
{
    map<TString, double> values, errors;
    for (auto s : mSamples)
    {
        double toteff = 1;
        double toterr = 0;
        for (auto e : mEffs)
        {
            if (e.first.Contains("__Total")) continue;
            toteff *= e.second->mValues[s];
            toterr += e.second->mErrors[s] / e.second->mValues[s];
        }

        values[s] = toteff;
        errors[s] = toteff * TMath::Sqrt(toterr);
    }

    mEffs["__Total"] = new EffComp("__Total", vector<TCut>(mSamples.size(), "Total efficiency"), mSamples);
    for (auto s : mSamples) mEffs["__Total"]->mValues[s] = values[s];
    for (auto s : mSamples) mEffs["__Total"]->mErrors[s] = errors[s];
}


double CutFlow::GetEff(TString eff, TString sample, double * err)
{
    if (find(mSamples.begin(), mSamples.end(), sample) == mSamples.end())
    {
        cout << "No sample " << sample << endl;
        return 0.;
    }
    if (err && GetEffErrors(eff)) (*err) = (*GetEffErrors(eff))[sample];
    if (GetEffValues(eff)) return (*GetEffValues(eff))[sample];
    return 0.;
}



double CutFlow::GetEffRatio(TString eff, TString sup, TString sdown, double * err)
{
    map<TString, double> * myvmap = GetEffValues(eff);
    map<TString, double> * myemap = GetEffErrors(eff);
    if (myvmap && myemap)
        for (auto v : (*myvmap))
        {
            if (v.first.Contains(sup + "_over_" + sdown))
            {
                if (err) (*err) = (*myemap)[v.first];
                return v.second;
            }
        }

    cout << "No ratio (" << sup << " / " << sdown << ") found.";
    cout << " Try CalcRatios(" << sup << "," << sdown << ") or check settings." << endl;
    return 0.;
}

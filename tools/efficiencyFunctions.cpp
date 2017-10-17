#include "efficiencyFunctions.hpp"


/** The following 4 functions allow to get efficiencies from TTrees **/


// This is a help rutine that builds a sting composed as weight*(cut1 && cut2 ...) or just "weight" if no cuts specified.
// Can be the used in TTree->Draw()
TString buildSelectStr(TCut cuts, TString weight)
{
    TCut mycut;
    TString selection;
    if (cuts != "") mycut = cuts;
    if (weight == "") selection = cuts;
    else if (mycut != "") selection = weight + "*(" + (TString)mycut + ")";
    else selection = weight;

    return selection;
}


/** This functions returns the integral of an histogram and entries
@param name: name in order to create unique named object
@param plot: variable to plot
@param tree: tree of data (which must contain the variable of the previous arg)
@param cuts: further cuts to apply before plotting
@param weight: weight to use (N.B.: if weight != 1 then integral is in principle different than number of entries)
@param entrires: variable to store the number of entries
**/
double getEvtN(TString name, TString plot, TTree * tree, TCut cuts, TString weight, double * entries)
{
    TString hname = "h" + name;
    TString selection = buildSelectStr(cuts, weight);
    tree->Draw(plot + ">>" + hname, selection);
    TH1* hist = (TH1 *)gPad->GetPrimitive(hname);
    if (entries) (*entries) = hist->GetEntries();
    return hist->Integral();
}


/** This functions returns a vector of integrals and n of enries after a list of cuts
@param name: name in order to create unique named object
@param plot: variable to plot
@param tree: tree of data (which must contain the variable of the previous arg)
@param genCut: further cuts to apply before plotting
@param cuts: list of cuts to apply. A cut genCut + sum cut[i] will be always applied. So the cuts in the list are applied in series with the addition of genCut
@param weight: weight to use (N.B.: if weight != 1 then integral is in principle different than number of entries)
 **/
vector< vector< double > > getEvtsAfterCuts(TString name, TString plot, TTree * treeMC, TCut genCut, vector<TCut> cuts, TString weight)
{
    vector< vector< double > > resv;
    vector<double> eff, entries;
    TCut curCut = genCut;
    double curentries;
    eff.push_back(getEvtN(name + "base", plot, treeMC, curCut, weight, &curentries));
    entries.push_back(curentries);

    for (unsigned i = 0; i < cuts.size(); i++)
    {
        curCut += cuts[i];
        eff.push_back(getEvtN(name + Form("%i", i), plot, treeMC, curCut, weight, &curentries));
        entries.push_back(curentries);
    }

    resv.push_back( eff );
    resv.push_back( entries );
    return resv;
}


/** This functions returns the efficiency from trees. Eventually relative to one other tree.
@param name: name in order to create unique named object
@param plot: variable to plot
@param tree and downTree: data trees for nominator and denominator
@param cuts and downCuts: cuts to apply at the nom and denom trees before plotting
@param weight: weight to use (N.B.: if weight != 1 then integral is in principle different than number of entries)
@param efferr: variable to store efficiency error (if relative it is error on relative efficiency)
@param relTree, relDownTree, relCuts and relDownCuts: nome and denom data trees and cuts to apply for the second channel. If this are NULL normal efficiency of the first channel is returned. If not the efficiency relative to this second channel is returned.
 **/
double getEff(TString name, TString plot, TTree * tree, TCut cuts, TTree * downTree, TCut downCuts, TString weight, double *efferr, TTree * relTree, TCut relCuts, TTree * relDownTree, TCut relDownCuts, TString relWeight)
{
    double nNom = getEvtN(name + "nom", plot, tree, cuts, weight);
    if (!downTree) downTree = tree;
    double entries, relentries;
    double nDenom = getEvtN(name + "denom", plot, downTree, downCuts, weight, &entries);
    double eff = nNom / nDenom;

    if (!relTree)
    {
        if (efferr) (*efferr) = TMath::Sqrt(eff * (1. - eff) / entries);
        return eff;
    }
    else
    {
        if (relWeight == "") relWeight = weight;
        double nrelNom = getEvtN(name + "nom", plot, relTree, relCuts, relWeight);
        if (!relDownTree) relDownTree = relTree;
        double nrelDenom = getEvtN(name + "denom", plot, relDownTree, relDownCuts, relWeight, &relentries);
        double denomEff = (nrelNom / nrelDenom);

        double releff = eff / denomEff;
        if (efferr) (*efferr) = releff * TMath::Sqrt( eff * (1. - eff) / entries + denomEff * (1. - denomEff) / relentries );
        return releff;
    }
}


/** This functions returns the efficiency from trees. Eventually relative to one other tree.
@param name: name in order to create unique named object
@param plot: variable to plot
@param tree: tree of data (which must contain the variable of the previous arg)
@param genCut: further cuts to apply before plotting
@param cuts: list of cuts to apply. A cut genCut + sum cut[i] will be always applied. So the cuts in the list are applied in series with the addition of genCut
@param weight: weight to use (N.B.: if weight != 1 then integral is in principle different than number of entries)
@param relTree and relCuts: same procedure is applied as for tree and efficiency reltive to this is final result. N.B.: the cuts applied on the second channel are relCut + cut[i] (not genCut)
 **/
vector< vector < double > > getEff(TString name, TString part, TTree * treeMC, TCut genCut, vector<TCut> cuts, TString weight, TTree * relTree, TCut relCuts, TString relWeight)
{
    vector< vector< double > > resv;
    vector< double > effres, errres;
    resv.push_back(effres);
    resv.push_back(errres);

    vector< vector< double > > nevt = getEvtsAfterCuts(name, part, treeMC, genCut, cuts, weight);

    vector< vector< double > > nrelevt;
    if (relWeight == "") relWeight = weight;
    if (relTree) nrelevt = getEvtsAfterCuts(name + "rel", part, relTree, relCuts, cuts, relWeight);

    if (nevt[0].size() < 2) { cout << "Only one cut included two required for an efficiency" << endl; return resv;}
    for (unsigned i = 1; i < nevt[0].size(); i++)
    {
        double eff = nevt[0][i] / nevt[0][i - 1];
        double err = eff * (1. - eff) / TMath::Sqrt(nevt[1][i - 1]);

        if (!relTree) { resv[0].push_back(eff); resv[1].push_back(err); }
        else
        {
            double downeff = nrelevt[0][i] / nrelevt[0][i - 1];
            double downerr = downeff * (1. - downeff) / TMath::Sqrt(nrelevt[1][i - 1]);
            double releff = eff / downeff;
            double relerr = releff * TMath::Sqrt( TMath::Power(err / eff, 2) + TMath::Power(downerr / downeff, 2) );
            resv[0].push_back(releff);
            resv[1].push_back(relerr);
        }
    }
    return resv;
}




TH2F * getEff(TString name, TString plot, TString xvar, TString yvar, int xnbins, double * xbins, int ynbins, double * ybins, TTree * tree, TCut cuts, TTree * downTree, TCut downCuts, TString weight, TTree * relTree, TCut relCuts, TTree * relDownTree, TCut relDownCuts, TString relWeight, string opt)
{
    TH1::SetDefaultSumw2();
    TH2F * res = NULL;

    double factor = 1.;
    size_t factorpos = opt.find("-f");
    if (factorpos != string::npos && relTree == NULL)
    {
        TString s = opt.substr(factorpos + 2, string::npos);
        factor = s.Atof();
    }

    TString upselect = buildSelectStr(cuts, weight);
    TString downselect = buildSelectStr(downCuts, weight);
    TString rupselect = buildSelectStr(relCuts, relWeight);
    TString rdownselect = buildSelectStr(relDownCuts, relWeight);

    int relXnbins = 1, relYnbins = 1;
    double relXbins_tmp[] = { xbins[0], xbins[xnbins] };
    double relYbins_tmp[] = { ybins[0], ybins[ynbins] };
    double * relXbins = &relXbins_tmp[0];
    double * relYbins = &relYbins_tmp[0];
    if (opt.find("-binXrel") != string::npos) { relXbins = xbins; relXnbins = xnbins; }
    if (opt.find("-binYrel") != string::npos) { relYbins = ybins; relYnbins = ynbins; }
    TH2F * relup = new TH2F("relup" + name, "relup" + name, relXnbins, relXbins, relYnbins, relYbins);
    TH2F * reldown = new TH2F("reldown" + name, "reldown" + name, relXnbins, relXbins, relYnbins, relYbins);
    TH2F * up = new TH2F("up" + name, "up" + name, xnbins, xbins, ynbins, ybins);
    TH2F * down = new TH2F("down" + name, "down" + name, xnbins, xbins, ynbins, ybins);
    tree->Draw(yvar + ":" + xvar + ">>+up" + name, upselect, "E");
    downTree->Draw(yvar + ":" + xvar + ">>+down" + name, downselect, "E");
    TH2F * eff = new TH2F("eff" + name, "eff" + name, xnbins, xbins, ynbins, ybins);
    eff->Divide(up, down, 1., 1., "b");
    eff->Scale(factor);

    if (relTree)
    {
        TH2F * effrel = new TH2F("effrel" + name, "effrel" + name, relXnbins, relXbins, relYnbins, relYbins);;
        effrel->Divide(relup, reldown, 1., 1., "b");
        effrel->Scale(factor);
        relTree->Draw(yvar + ":" + xvar + ">>+relup" + name, rupselect, "E");
        relDownTree->Draw(yvar + ":" + xvar + ">>+reldown" + name, rdownselect, "E");

        if (relXnbins != 1 && relYnbins != 1) res->Divide(eff, effrel);
        else
        {
            res = (TH2F *)eff->Clone("releff");
            double releff = effrel->GetBinContent(1, 1);
            double relerr = effrel->GetBinError(1, 1);
            for (int i = 1; i <= res->GetNbinsX(); i++)
                for (int j = 1; j <= res->GetNbinsY(); j++)
                {
                    double cureff = eff->GetBinContent(i, j);
                    double curerr = eff->GetBinError(i, j);
                    res->SetBinError(i, j, (cureff / releff) * TMath::Sqrt( TMath::Power(curerr / cureff, 2) + TMath::Power(relerr / releff, 2) ));
                    res->SetBinContent(i, j, cureff / releff);
                }
        }
        res->SetMaximum(2.);
    }
    else { res = eff; res->SetMaximum(1.); }
    res->SetMinimum(0.);
    res->GetXaxis()->SetTitle(xvar);
    res->GetYaxis()->SetTitle(yvar);

    return res;
}


TH2F * getEff(TString name, TString plot, TString xvar, TString yvar, int xnbins, double * xbins, int ynbins, double * ybins, TTree * tree, TCut cuts, TCut downCuts, TString weight, TTree * relTree, TCut relCuts, TCut relDownCuts, TString relWeight, string opt)
{
    return getEff(name, plot, xvar, yvar, xnbins, xbins, ynbins, ybins, tree, cuts, tree, downCuts, weight, relTree, relCuts, relTree, relDownCuts, relWeight, opt);
}



TH1F * getEff(TString name, TString xvar, int xnbins, double * xbins, TTree * tree, TCut cuts, TTree * downTree, TCut downCuts, TString weight, TTree * relTree, TCut relCuts, TTree * relDownTree, TCut relDownCuts, TString relWeight, string opt)
{
    TH1::SetDefaultSumw2();
    TH1F * res = NULL;

    double factor = 1.;
    size_t factorpos = opt.find("-f");
    if (factorpos != string::npos && relTree == NULL)
    {
        TString s = opt.substr(factorpos + 2, string::npos);
        factor = s.Atof();
    }

    int relXnbins = 1;
    double relXbins_tmp[] = { xbins[0], xbins[xnbins] };
    double * relXbins = &relXbins_tmp[0];
    if (opt.find("-binXrel") != string::npos) { relXbins = xbins; relXnbins = xnbins; }

    TString upselect = buildSelectStr(cuts, weight);
    TString downselect = buildSelectStr(downCuts, weight);
    TString rupselect = buildSelectStr(relCuts, relWeight);
    TString rdownselect = buildSelectStr(relDownCuts, relWeight);

    TH1F * relup = new TH1F("relup" + name, "relup" + name, relXnbins, relXbins);
    TH1F * reldown = new TH1F("reldown" + name, "reldown" + name, relXnbins, relXbins);
    TH1F * up = new TH1F("up" + name, "up" + name, xnbins, xbins);
    TH1F * down = new TH1F("down" + name, "down" + name, xnbins, xbins);
    tree->Draw(xvar + ">>+up" + name, upselect, "E");
    downTree->Draw(xvar + ">>+down" + name, downselect, "E");
    TH1F * eff = new TH1F("eff_" + name, name, xnbins, xbins);
    eff->Divide(up, down, 1., 1., "B");
    eff->Scale(factor);

    if (relTree)
    {
        TH1F * effrel = new TH1F("releff_" + name, "releff_" + name, relXnbins, relXbins);
        relTree->Draw(xvar + ">>+relup" + name, rupselect, "E");
        relDownTree->Draw(xvar + ">>+reldown" + name, rdownselect, "E");
        effrel->Divide(relup, reldown, 1., 1., "B");
        effrel->Scale(factor);

        res = (TH1F *)eff->Clone("releff");
        if (relXnbins != 1) res->Divide(eff, effrel);
        else
        {
            double releff = effrel->GetBinContent(1);
            double relerr = effrel->GetBinError(1);

            for (int i = 1; i <= res->GetNbinsX(); i++)
            {
                double cureff = eff->GetBinContent(i);
                double curerr = eff->GetBinError(i);
                res->SetBinContent(i, cureff / releff);
                res->SetBinError(i, (cureff / releff)*TMath::Sqrt( TMath::Power(curerr / cureff, 2) + TMath::Power(relerr / releff, 2) ));
            }
        }
        res->SetMaximum(2.);
    }
    else { res = eff; res->SetMaximum(1.); }
    res->SetMinimum(0.);
    res->GetXaxis()->SetTitle(xvar);

    return res;
}



TH1F * getEff(TString name, TString xvar, int xnbins, double * xbins, TTree * tree, TCut cuts, TCut downCuts, TString weight, TTree * relTree, TCut relCuts, TCut relDownCuts, TString relWeight, string opt)
{
    return getEff(name, xvar, xnbins, xbins, tree, cuts, tree, downCuts, weight, relTree, relCuts, relTree, relDownCuts, relWeight, opt);
}

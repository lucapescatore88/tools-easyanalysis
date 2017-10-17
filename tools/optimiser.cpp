#include "optimiser.hpp"


/**
  \class CutOptimiser.

  Allows to opptimise any cut in an n-dimensional space is required.

  */

TString subStrings(TString tmp, vector <TString> names, vector<double> values)
{
    if (names.size() != values.size())
    {
        cout << "Names and values have to have the same size" << endl;
        return tmp;
    }

    TString outstr = tmp;
    for (size_t n = 0; n < names.size(); n++)
        outstr = outstr.ReplaceAll(names[n], Form("%e", values[n]));

    return outstr;
}

bool compare_vectors_double( vector<double> v1, vector<double> v2 )
{
    if (v1.size() != v2.size()) return false;
    for (size_t i = 0; i < v1.size(); i++)
        if (v1[i] != v2[i]) return false;
    return true;
}


/**
 * Builds all the combinations of n vectors
 *
 * e.g. input : { [0,1] , [2,3] }
 *      output: { [0,2], [0,3], [1,2], [1,3] }
 *
 * Options: -norepetition: does not allow repetition of the same number e.g. [1,0,1] or [2,2,4,5] are not allowed
 *          -unordered   : discards vectors with same content but different order e.g. keeps only one on [1,2,3] and [3,1,2]
 *
 * If you're not interested in these options but in speed use buildComb_fast()
 */

void buildComb(const vector<vector<double> > &allVecs, vector<vector<double> > &result, size_t vecIndex, const vector <double> vSoFar, string opt)
{
    if (vecIndex >= allVecs.size())
    {
        if (vSoFar.size() > 1)
        {
            vector <double> v_check = vSoFar;
            sort(v_check.begin(), v_check.end());

            if (opt.find("-norepetition") != string::npos)
            {
                for (size_t vi = 1; vi < v_check.size(); vi++)
                    if (v_check[vi] == v_check[vi - 1]) return;
            }
            if (opt.find("-unordered") != string::npos)
            {
                for (size_t vi = 0; vi < result.size(); vi++)
                {
                    vector <double> cur_res = result[vi];
                    sort(cur_res.begin(), cur_res.end());
                    if (compare_vectors_double(cur_res, v_check)) return;
                }
            }
        }
        result.push_back(vSoFar);
        return;
    }
    for (size_t i = 0; i < allVecs[vecIndex].size(); i++)
    {
        vector<double> tmp_v = vSoFar;
        tmp_v.push_back(allVecs[vecIndex][i]);
        buildComb(allVecs, result, vecIndex + 1, (const vector<double>) tmp_v, opt);
    }
}

void buildComb_fast(const vector<vector<double> > &allVecs, vector<vector<double> > &result, size_t vecIndex, const vector <double> vSoFar)
{
    if (vecIndex >= allVecs.size())
    {
        result.push_back(vSoFar);
        return;
    }
    for (size_t i = 0; i < allVecs[vecIndex].size(); i++)
    {
        vector<double> tmp_v = vSoFar;
        tmp_v.push_back(allVecs[vecIndex][i]);
        buildComb(allVecs, result, vecIndex + 1, (const vector<double>) tmp_v);

    }
}

void buildComb(const vector<vector<TString> > &allVecs, vector<TString > &result, size_t vecIndex, TString strSoFar)
{
    if (vecIndex >= allVecs.size())
    {
        result.push_back(strSoFar);
        return;
    }
    for (size_t i = 0; i < allVecs[vecIndex].size(); i++)
        buildComb(allVecs, result, vecIndex + 1, strSoFar + allVecs[vecIndex][i]);
}

void buildComb(const vector< double > &elms, vector<vector<double> > &result, string opt)
{
    vector < vector < double > > allVecs;
    for (auto e : elms) allVecs.push_back(vector<double>(1, e));
    buildComb(allVecs, result, 0, vector<double>(), opt);
}

/**
 * Given an interval between min and max build all combinations of integers in groups of size k.
 *
 * e.g. buildComb(0, 3, 2) = { [0,1], [0,2], [0,3], [0,4], [1,0], [1,2], [1,3], ...  }
 *
 * Option -unordered can be used. Option -norepetition exists but is automatically fulfilled in this case.
 */

void buildComb(int min, int max, int k, vector<vector<double> > &result, string opt)
{
    vector < vector < double > > allVecs;
    for (int kk = 0; kk < k; kk++)
    {
        vector < double > myv;
        for (int i = min; i <= max; i++) myv.push_back((double)i);
        allVecs.push_back(myv);
    }
    buildComb(allVecs, result, 0, vector<double>(), opt);
}

/**
 * Given an interval and a number of steps returns the value at each step:
 *
 * default: the full range is divided in N intervals and the function returns the centers (N points)
 * -edges: the full range is divided in N intervals and the function returns the edges (N+1 points)
 *
 */

vector <double> getSteps(double min, double max, int nsteps, string option)
{
    vector <double> steps;
    double vstep = (max - min) / nsteps;
    if (option.find("-edges") != string::npos)
    {
        for (int i = 0; i <= nsteps; i++)
            steps.push_back(min + vstep * i);
    }
    else
    {
        for (int i = 0; i < nsteps; i++)
            steps.push_back(min + vstep * (i + 0.5));
    }
    return steps;
}

/**
 * Creates a new TGraph with the entries put in an ascending x order.
 * */

TGraph * reorderTGraph(TGraph * gr)
{
    vector< vector < double > > pts;
    double x, y;
    for (int p = 0; p < gr->GetN(); p++)
    {
        gr->GetPoint(p, x, y);
        vector < double > pt;
        pt.push_back(p);
        pt.push_back(x);
        pt.push_back(y);
        pts.push_back(pt);
    }

    sort(pts.begin(), pts.end(),
    [](const vector<double>& a, const vector<double>& b) {
        return a[1] < b[1];
    });

    TGraph * grout = new TGraph();
    for (size_t p = 0; p < pts.size(); p++)
        grout->SetPoint(p, pts[p][1], pts[p][2]);

    return grout;
}



/**
 * Default functions to extract the number of signal and background events in the optimizer
 *
 * These can be customized by definiting function with template double f(TTree *, TString, TString, TString, double , TH1F *)
 * and then given to the optimizer using SetBkgFunctions(&f) or SetSigFunctions(&f).
 */

double get_signal(TTree * tree, TString plot, TString cut, TString weight, double norm)
{
    static int iS = 0;
    if (weight != "") tree->Draw(plot + ">>" + Form("hS_%d", iS), weight + " * (" + cut + ")", "e");
    else tree->Draw(plot + ">>" + Form("hS_%d", iS), cut, "e");
    TH1F * hh = (TH1F*) gPad->GetPrimitive(Form("hS_%d", iS));
    iS++;
    return norm * hh->Integral();
}

double get_background(TTree * tree, TString plot, TString cut, TString weight, double norm)
{
    static int iB = 0;
    tree->Draw(plot + ">>" + Form("hB_%d", iB), cut , "e");
    TH1F *hh = (TH1F*) gPad->GetPrimitive(Form("hB_%d", iB));
    iB++;
    return norm * hh->Integral();
}
/**
 * \brief CutOptimier constructor

  @param _analysis: just a name for the object
  @param _treeSig: is the signal sample of the signal
  @param _treeBkg: is the bkg sample
  @param _vars: a vector <RooRealVar*> containing the list of variables to scan
  @param _baseCut: is a basic cut which will be applied to both trees
  @param _sigCut and _sideBandCut: a cut which will be applied on treeSig/Bkg only
  @param _mycut: this is the cut to optimize
  @param _norm and _normBkg: normalization factors for sig and bkg to calculate pproper significnce. If not set they are 1.
  @param _nSteps: now many steps to do (default 100)
  @param _MCweight: weight to apply to the signal MC, can be a complex expression
  @param _fmerit: "significance" (default) or "punziN" where N is the number of sigmas to use
  @param _print: creates performace plots
**/


CutOptimizer::CutOptimizer(TString _analysis, TTree *_treeSig, TTree *_treeBkg,
                           vector<RooRealVar *> _vars, TString _mycut, TCut _sigCut, TCut _sideBandCut, TCut _baseCut,
                           double _sigNorm, double _bkgNorm,
                           TString _MCweight, int _nSteps,
                           string _fmerit, bool _print):
    analysis(_analysis),
    treeSig(_treeSig),
    treeBkg(_treeBkg),
    baseCut(_baseCut),
    sigCut(_sigCut),
    sideBandCut(_sideBandCut),
    sigNorm(_sigNorm),
    bkgNorm(_bkgNorm),
    MCweight(_MCweight),
    nSteps(_nSteps),
    vars(_vars),
    cut_to_optimize(_mycut),
    fmerit(_fmerit),
    print(_print)
{
    cout << endl;
    cout << analysis << ": Optimize " << cut_to_optimize << " (FoM = " << fmerit << ", steps = " << nSteps << ")" << endl;
    cout << endl;

    ofile = new TFile(analysis + "_optimize.root", "recreate");

    maxEff = maxBkgRej = pasB = pasS = maxSig = -1;

    if (nSteps < 3) nSteps = 3;

    get_sig = &get_signal;
    get_bkg = &get_background;

    points = new TTree("Optimisation_data", "Optimisation_data");
    double holder;
    points->Branch("FoM", &holder, "FoM/D");
    points->Branch("sig", &holder, "sig/D");
    points->Branch("bkg", &holder, "bkg/D");
    points->Branch("rej", &holder, "rej/D");
    points->Branch("eff", &holder, "eff/D");
    points->Branch("purity", &holder, "purity/D");

    vector < vector <double>> steps1D;
    for (auto v : vars)
    {
        orig_vars.push_back(new RooRealVar(v->GetName(), v->GetName(), v->getMin(), v->getMax()));
        points->Branch(v->GetName(), &holder, (TString)v->GetName() + "/D");
        vector <double> steps = getSteps(v->getMin(), v->getMax(), nSteps);
        steps1D.push_back(steps);
        step_sizes.push_back(TMath::Abs(steps[1] - steps[0]));
    }
    buildComb(steps1D, pts_to_scan);

    reader = new TreeReader(points);
    reader->Initialize();

    baseSigCut = baseCut;
    if (sigCut != "") baseSigCut += sigCut;
    baseBkgCut = baseCut;
    if (sideBandCut != "") baseBkgCut += sideBandCut;

    cout << endl;
    cout << "Signal Cut" << endl;
    baseSigCut.Print();
    cout << endl;
    cout << "Background Cut" << endl;
    baseBkgCut.Print();
    cout << endl;

    TObjArray* branches = treeSig->GetListOfBranches();
    TBranch* branch = (TBranch*)branches->At(0);
    vplot = branch->GetName();

    totS = get_sig(treeSig, vplot, (TString) (baseSigCut), MCweight, sigNorm);
    totB = get_bkg(treeBkg, vplot, (TString) (baseBkgCut), "", bkgNorm);

    cout << "S Tot = " << totS << endl;
    cout << "B Tot = " << totB << endl;
    cout << endl;
}


void CutOptimizer::ResetSteps(unsigned nsteps)
{
    if (nsteps < 3) nsteps = 3;
    nSteps = nsteps;
    pts_to_scan.clear();
    step_sizes.clear();
    vector < vector <double>> steps1D;
    for (auto v : vars)
    {
        vector <double> steps = getSteps(v->getMin(), v->getMax(), nSteps);
        steps1D.push_back(steps);
        step_sizes.push_back(TMath::Abs(steps[1] - steps[0]));
    }
    buildComb(steps1D, pts_to_scan);
}



void CutOptimizer::scan_points(string option)
{
    vector <TString> varnames;
    for (auto v : vars) varnames.push_back(v->GetName());

    size_t n_pts = pts_to_scan.size();
    for (size_t i = 0; i < n_pts; i++)
    {
        if (option.find("-noperc") == string::npos) showPercentage(i, n_pts, 0, n_pts);
        TString select = subStrings(cut_to_optimize, varnames, pts_to_scan[i]);

        double S = get_sig(treeSig, vplot, (TString) (baseSigCut + (TCut) select), MCweight, sigNorm);
        double B = get_bkg(treeBkg, vplot, (TString) (baseBkgCut + (TCut) select), "", bkgNorm);

        double eff    = S / totS;
        double P      = S / (S + B);
        double bkgRej = (1. - B / totB);
        double signif = get_significance(S, B);

        reader->SetValue<double>("purity", P);
        reader->SetValue<double>("sig", S);
        reader->SetValue<double>("bkg", B);
        reader->SetValue<double>("eff", eff);
        reader->SetValue<double>("rej", bkgRej);
        reader->SetValue<double>("FoM", signif);
        for (size_t v = 0; v < vars.size(); v++)
            reader->SetValue<double>(vars[v]->GetName(), pts_to_scan[i][v]);

        reader->GetChain()->Fill();

        if (signif > maxSig && option.find("-optimise") != string::npos)
        {
            pasS       = S;
            pasB       = B;
            maxSig     = signif;
            maxEff     = eff;
            maxBkgRej  = bkgRej;
            optimalW   = pts_to_scan[i];
        }
    }
}



double CutOptimizer::get_significance(double S, double B)
{
    double signif = 0;
    if (fmerit == "significance") signif = S / TMath::Sqrt(B + S);
    else if (fmerit.find("punzi") != string::npos)
    {
        TString str_nsigma = fmerit.substr(5, string::npos);
        int nsigma = str_nsigma.Atof();
        if (fmerit == "punzi") nsigma = 5;
        signif = S / (nsigma / 2. + TMath::Sqrt(B));
    }
    return signif;
}

/**
 * Scans a discrete grid of points (once) and finds the point with higher FoM
 */

vector <double>  CutOptimizer::optimise(string option)
{
    vector <TString> varnames;
    for (auto v : vars) varnames.push_back(v->GetName());

    cout << "Optimizing..." << endl;

    scan_points("-optimise" + option);

    cout << endl;
    cout << "Optimal Cut    = " << subStrings(cut_to_optimize, varnames, optimalW) << endl;
    cout << "N Sig          = " << fixed << setprecision(1) << pasS << endl;
    cout << "N Bkg          = " << fixed << setprecision(1) << pasB << endl;
    cout << "Sig Efficiency = " << fixed << setprecision(1) << maxEff * 100 << endl;
    cout << "Bkg Rejection  = " << fixed << setprecision(1) << maxBkgRej * 100 << endl;
    cout << endl;

    return optimalW;
}

/**
 * Makes a corse grid and scans it then it makes finer and finer grids around the best points found.
 *  - Grids are deterministic
 *  - Algorithm stops when sqrt{ sum [ (p-p0) / p0 ]} < prec
 *    where p is the best value found in he current iteration and p0 in the precedent
 *    the sum is on the values of all the variables in "vars".
 */

vector <double>  CutOptimizer::iterative_optimise(double precision, string option)
{
    cout << endl;
    cout << analysis << ": Iterative optimization (precision = " << precision << ")" << endl;

    vector <double> prec_opt(vars.size(), 1e9);
    double prec = 1e6;
    while (prec > precision)
    {
        if (prec != 1e6)
        {
            double min, max;
            for (size_t k = 0; k < vars.size(); k++) {
                min = prec_opt[k] - step_sizes[k];
                if (min < orig_vars[k]->getMin())
                    min = orig_vars[k]->getMin();

                max = prec_opt[k] + step_sizes[k];
                if (max > orig_vars[k]->getMax())
                    max = orig_vars[k]->getMax();

                vars[k]->setRange(min, max);
            }
            ResetSteps(nSteps);
        }

        optimise(option);

        double diff = 0;
        for (size_t vv = 0; vv < vars.size(); vv++)
        {
            diff += TMath::Power((optimalW[vv] - prec_opt[vv]) / optimalW[vv], 2);
        }
        prec = TMath::Sqrt(diff);
        if (prec < 10) cout << fixed << setprecision(4) << "Current precision: " << prec << endl << endl;

        prec_opt = optimalW;
    }

    return optimalW;
}


/**
 * Makes nice plots and saves them in a file
 *
 */

void CutOptimizer::ClosePrintAndSave(string option)
{
    ofile->cd();
    points->Write();

    if (option.find("-recalc") != string::npos)
    {
        for ( auto v : orig_vars )
        {
            cout << endl << "Scanning " << v->GetName() << "..." << endl;

            vector <double> steps1D = getSteps(v->getMin(), v->getMax(), 100, "-edges");
            vector < vector <double> > steps;
            for (size_t iv = 0; iv < orig_vars.size(); iv++)
            {
                if (orig_vars[iv]->GetName() != v->GetName()) steps.push_back(vector<double>(1, optimalW[iv]));
                else steps.push_back(steps1D);
            }

            pts_to_scan.clear();
            buildComb(steps, pts_to_scan);
            scan_points("-plotonly" + option);
        }
    }

    for (auto v : orig_vars)
    {
        TString select = "";
        int added = 0;
        TString vname = v->GetName();
        for (size_t vv = 0; vv < vars.size(); vv++)
        {
            if ((TString)vars[vv]->GetName() != vname)
            {
                if ( TMath::Abs( optimalW[vv] ) < 1e-6 ) select += "TMath::Abs(" + (TString)vars[vv]->GetName() + " ) < 1e-6";
                else select += "TMath::Abs(" + (TString)vars[vv]->GetName() + Form(" - %e)/TMath::Abs(%e) < 1e-5", optimalW[vv], optimalW[vv]);
                added++;
                if (added < ((int)optimalW.size() - 1)) select += " && ";
            }
        }

        points->Draw("rej:eff>>" + analysis + "_ROC" + vname, select);
        TGraph * gROC = reorderTGraph(new TGraph(points->GetSelectedRows(), points->GetV2(), points->GetV1()));
        gROC->Draw("apl");
        points->Draw("eff:" + vname + ">>" + analysis + "_eff_" + vname, select);
        TGraph * gEff = reorderTGraph(new TGraph(points->GetSelectedRows(), points->GetV2(), points->GetV1()));
        gEff->Draw("apl");
        points->Draw("purity:" + vname + ">>" + analysis + "_purity_" + vname, select);
        TGraph * gPur = reorderTGraph(new TGraph(points->GetSelectedRows(), points->GetV2(), points->GetV1()));
        gPur->Draw("apl");
        points->Draw("FoM:" + vname + ">>" + analysis + "_FoM_" + vname, select);
        TGraph * gFOM = reorderTGraph(new TGraph(points->GetSelectedRows(), points->GetV2(), points->GetV1()));
        gFOM->Draw("apl");
        points->Draw("sig:" + vname + ">>" + analysis + "_sig_" + vname, select);
        TGraph * gSig = reorderTGraph(new TGraph(points->GetSelectedRows(), points->GetV2(), points->GetV1()));
        gSig->Draw("apl");
        points->Draw("bkg:" + vname + ">>" + analysis + "_bkg_" + vname, select);
        TGraph * gBkg = reorderTGraph(new TGraph(points->GetSelectedRows(), points->GetV2(), points->GetV1()));
        gBkg->Draw("apl");
        points->Draw("rej:" + vname + ">>" + analysis + "_rej_" + vname, select);
        TGraph * gRej = reorderTGraph(new TGraph(points->GetSelectedRows(), points->GetV2(), points->GetV1()));
        gRej->Draw("apl");
        points->Draw("eff*purity:" + vname + ">>" + analysis + "_EP_" + vname, select);
        TGraph * gEP = reorderTGraph(new TGraph(points->GetSelectedRows(), points->GetV2(), points->GetV1()));
        gEP->Draw("apl");

        gSig->GetXaxis()->SetTitle(vname);
        gBkg->GetXaxis()->SetTitle(vname);
        gEff->GetXaxis()->SetTitle(vname);
        gRej->GetXaxis()->SetTitle(vname);
        gPur->GetXaxis()->SetTitle(vname);
        gEP->GetXaxis()->SetTitle(vname);
        gFOM->GetXaxis()->SetTitle(vname);
        gROC->GetXaxis()->SetTitle("#varepsilon_{S}");

        gSig->GetYaxis()->SetTitle("N_{S}");
        gBkg->GetYaxis()->SetTitle("N_{B}");
        gEff->GetYaxis()->SetTitle("#varepsilon_{S}");
        gRej->GetYaxis()->SetTitle("1 - #varepsilon_{B}");
        gPur->GetYaxis()->SetTitle("N_{S}/(N_{S}+N_{B})");
        gEP->GetYaxis()->SetTitle("#varepsilon_{S} #times N_{S}/(N_{S}+N_{B})");
        if (fmerit == "significance") gFOM->GetYaxis()->SetTitle("N_{S}/#sqrt{N_{S}+N_{B}}");
        else if (fmerit == "effpur") gFOM->GetYaxis()->SetTitle("#varepsilon_{S} #times N_{S}/(N_{S}+N_{B})");
        else if (fmerit.find("punzi") != string::npos) gFOM->GetYaxis()->SetTitle("S/(n_{#sigma}/2 + #sqrt{B})");
        else gFOM->GetYaxis()->SetTitle("FoM");
        gROC->GetYaxis()->SetTitle("1 - #varepsilon_{B}");

        gSig->GetXaxis()->SetRangeUser(v->getMin(), v->getMax());
        gBkg->GetXaxis()->SetRangeUser(v->getMin(), v->getMax());
        gEff->GetXaxis()->SetRangeUser(v->getMin(), v->getMax());
        gRej->GetXaxis()->SetRangeUser(v->getMin(), v->getMax());
        gPur->GetXaxis()->SetRangeUser(v->getMin(), v->getMax());
        gEP->GetXaxis()->SetRangeUser(v->getMin(), v->getMax());
        gFOM->GetXaxis()->SetRangeUser(v->getMin(), v->getMax());
        gEff->GetXaxis()->SetRangeUser(v->getMin(), v->getMax());
        gROC->GetXaxis()->SetRangeUser(0, 1.1);

        gEff->GetYaxis()->SetRangeUser(0, 1.1);
        gRej->GetYaxis()->SetRangeUser(0, 1.1);
        gPur->GetYaxis()->SetRangeUser(0, 1.1);
        gEP->GetYaxis()->SetRangeUser(0, 1.1);
        gROC->GetYaxis()->SetRangeUser(0, 1.1);

        gSig->SetLineColor(kBlue);
        gBkg->SetLineColor(kRed);
        gEff->SetLineColor(kBlue);
        gRej->SetLineColor(kRed);
        gPur->SetLineColor(kBlack);
        gEP->SetLineColor(kBlack);
        gFOM->SetLineColor(kGreen);

        gPur->SetLineStyle(9);
        gEP->SetLineStyle(7);

        ofile->cd();
        gSig->Write("Signal_" + vname);
        gBkg->Write("Background_" + vname);
        gEff->Write("Efficiency_" + vname);
        gRej->Write("Rejection_" + vname);
        gPur->Write("Purity_" + vname);
        gEP->Write("EfficiencyPurity" + vname);
        gFOM->Write("FoM_" + vname);
        gROC->Write("ROC_" + vname);

        TCanvas *c0 = new TCanvas();

        c0->SetLogy();
        gBkg->GetYaxis()->SetTitle("Candidates");
        gBkg->Draw("al");
        gSig->Draw("l same");
        c0->Print(analysis + "_Candidates_" + vname + ".pdf");

        c0->SetLogy(0);
        c0->Clear();

        gEff->Draw("al");
        c0->Print(analysis + "_Efficiency_" + vname + ".pdf");

        gRej->Draw("al");
        c0->Print(analysis + "_Rejection_" + vname + ".pdf");

        gPur->Draw("al");
        c0->Print(analysis + "_Purity_" + vname + ".pdf");

        gROC->Draw("al");
        c0->Print(analysis + "_ROC_" + vname + ".pdf");
        c0->Print(analysis + "_ROC_" + vname + ".C");

        gFOM->Draw("al");
        c0->Print(analysis + "_FoM_" + vname + ".pdf");

        delete c0;

        gROOT->ForceStyle(0);
        gStyle->SetPadTickY(0);

        c0 = new TCanvas();

        gEff->GetYaxis()->SetTitle("Efficiency, Purity (Significance)");
        gEff->GetYaxis()->SetRangeUser(0, 1.2);

        TLegend * legend = new TLegend(.16, .85, .92, .9);
        legend->SetNColumns(5);
        legend->AddEntry(gEff, "#varepsilon_{S}", "l");
        legend->AddEntry(gRej, "1 - #varepsilon_{B}", "l");
        legend->AddEntry(gPur, "N_{S}/(N_{tot})", "l");
        legend->AddEntry(gEP,  "#varepsilon_{S} #times N_{S}/(N_{tot})", "l");
        legend->AddEntry(gFOM, "N_{S}/#sqrt{N_{S}+N_{B}}", "l");

        TPad *pad = new TPad("pad", "", 0, 0, 1, 1);
        pad->SetFillColor(0);
        pad->SetGrid();
        pad->Draw();
        pad->cd();

        gEff->Draw("al");
        gRej->Draw("l");
        gPur->Draw("l");
        gEP->Draw("l");

        c0->cd();
        TPad *overlay = new TPad("overlay", "", 0, 0, 1, 1);
        overlay->SetFillStyle(4000);
        overlay->SetFillColor(0);
        overlay->SetFrameFillStyle(4000);
        overlay->Draw();
        overlay->cd();

        double xmin = v->getMin();
        double xmax = v->getMax();
        //double xmin = pad->GetUxmin();
        //double xmax = pad->GetUxmax();
        double ymin = 0;
        double ymax = 1.5 * maxSig;
        TH1F *oFrame = overlay->DrawFrame(xmin, ymin, xmax, ymax);
        oFrame->GetXaxis()->SetLabelOffset(99);
        oFrame->GetYaxis()->SetLabelOffset(99);
        oFrame->GetXaxis()->SetNdivisions(0);
        oFrame->GetYaxis()->SetNdivisions(0);

        gFOM->GetYaxis()->SetRangeUser(0, ymax);
        gFOM->Draw("l");

        // Label
        //double lOffset = .010;

        // Line
        //double lWidth  = 2;

        // Markers
        //double mSize   = 1;
        //double mStyle  = 20;

        // Text
        double tFont   = 132;
        //double tOffset = .10;
        //double tSize   = .06;

        TGaxis *axis = new TGaxis(xmax, ymin, xmax,  ymax, ymin, ymax, 510, "+L");
        //axis->SetTitle("Significance");
        axis->SetLabelFont(tFont);
        //axis->SetLabelSize(tSize);
        //axis->SetLabelOffset(lOffset);
        axis->SetTitleFont(tFont);
        //axis->SetTitleSize(tSize);
        //axis->SetTitleOffset(lOffset);

        axis->Draw();
        legend->Draw();

        c0->Print(analysis + "_Optimize_" + vname + ".pdf");
        c0->Print(analysis + "_Optimize_" + vname + ".C");

        delete gROC;
        delete gEff;
        delete gPur;
        delete gFOM;
        delete gSig;
        delete gBkg;
        delete gRej;
        delete gEP;

        delete oFrame;
        delete pad;
        delete overlay;
        delete axis;
        delete c0;
    }

    /*
       if(vars.size()==2 || vars.size()==3)
       {
       TCanvas * c0 = new TCanvas();
       vector< vector < double > > combs;
       buildComb(0, vars.size()-1, 2, combs,"-norepetitions-unordered");
       for(auto c : combs)
       {
       TString select = "";
       int added = 0;
       for(size_t vv = 0; vv < vars.size(); vv++)
       {
       if(vv!=c[0] && vv!=c[1])
       {
       select += "TMath::Abs("+(TString)vars[vv]->GetName()+Form(" - %e)/TMath::Abs(%e) < 1e-5", optimalW[vv], optimalW[vv]);
       added++;
       if(added < ((int)optimalW.size()-2)) select += " && ";
       }
       }
       points->Draw((TString)vars[c[0]]->GetName()+":"+vars[c[1]]->GetName()+":FoM>>hh2D",select);

       TGraph2D * gr = new TGraph2D(points->GetSelectedRows(),points->GetV2(), points->GetV1(), points->GetV3());
       gr->Draw("TRI1");
       gr->GetZaxis()->SetRangeUser(0,maxSig*1.5);
       gr->GetXaxis()->SetTitle(vars[c[0]]->GetName());
       gr->GetYaxis()->SetTitle(vars[c[1]]->GetName());
       gr->GetZaxis()->SetTitle("FoM");
       c0->Print((TString)"FoM_2D_"+vars[c[0]]->GetName()+"_"+vars[c[1]]->GetName()+".pdf");
       }
       }
       */

    ofile->Close();
}

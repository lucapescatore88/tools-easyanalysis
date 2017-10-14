


RooDataSet * Analysis::CalcSWeight(double min, double max, unsigned nbins, bool unbinned, string option)
{
    transform(option.begin(), option.end(), option.begin(), ::tolower);
    if (m_pmode == "v") cout << endl << m_name << ": CalcSWeight " << option << endl;

    TString oldname = m_name;
    if (option.find("-nofit") == string::npos)
    {
        m_name += (TString)"_sW";
        Fit(min, max, nbins, unbinned, option);
    }
    m_name = oldname;

    RooRealVar * myvar = m_vars[0]

                         cout << endl << endl;
    if (min == max) { min = myvar->getMin(); max = myvar->getMax(); }
    cout << "Sig yield = " << m_nsig->getVal() << endl;
    cout << "Bkg yield = " << m_nbkg->getVal() << endl;
    cout << endl;

    // Create new TTree with sWeights
    if (!m_reducedTree && m_data) m_reducedTree = (TTree *)m_data->tree();
    TreeReader * reducedReader = new TreeReader(m_reducedTree);
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

    double syield = m_nsig->getVal();
    double byield = m_nbkg->getVal();

    for (Long64_t i = 0 ; i < nEntries ; ++i)
    {
        reducedReader->GetEntry(i);
        float value = reducedReader->GetValue(myvar->GetName());
        if (value < min || value > max) continue;
        myvar->setVal(value);

        double s_pdf = m_sig->getVal(*myvar);
        double b_pdf = m_bkg->getVal(*myvar);

        double denom = syield * s_pdf + byield * b_pdf;
        Vss += (s_pdf * s_pdf) / (denom * denom);
        Vbs += (s_pdf * b_pdf) / (denom * denom);
        Vbb += (b_pdf * b_pdf) / (denom * denom);
    }

    double det = TMath::Abs(Vss * Vbb - Vbs * Vbs);
    double invVss = Vbb / det;
    double invVbs = - Vbs / det;

    GetTotNBkg();

    double sum_sW = 0;
    for (Long64_t i = 0 ; i < nEntries ; ++i)
    {
        showPercentage(i, nEntries);

        reducedReader->GetEntry(i);
        float value = reducedReader->GetValue(m_vars->GetName());
        if (value < min || value > max) continue;
        myvar->setVal(value);

        double b_pdf = m_bkg->getVal(*myvar);
        double s_pdf = m_sig->getVal(*myvar);
        double denom = syield * s_pdf + byield * b_pdf;
        sW = (invVss * s_pdf + invVbs * b_pdf) / denom;

        sWR = GetReducedSWeight(value);

        sTree->Fill();

        sum_sW += sW;
    }

    cout << endl;
    cout << "sWeighted " << sTree->GetEntries() << " entries" << endl;
    cout << "Sum of weights = " << sum_sW << endl;
    cout << endl;

    m_reducedTree = sTree;

    TCanvas *c = new TCanvas();
    gStyle->SetOptStat(0);

    sTree->Draw("sW");

    c->Print((TString) m_name + "_sWeights.pdf");

    sTree->Draw(myvar->GetName() + (TString)">>hSWHisto", "sW");
    TH1D * hSWHisto = (TH1D*)gPad->GetPrimitive("hSWHisto");
    sTree->Draw(myvar->GetName() + (TString)">>hHisto");
    TH1D * hHisto = (TH1D*)gPad->GetPrimitive("hHisto");

    hSWHisto->SetLineColor(1);
    hHisto->SetLineColor(4);
    hHisto->Draw("hist");
    hSWHisto->Draw("hist same");

    c->Print((TString) m_name + "_sWeighted.pdf");

    return m_data;
}
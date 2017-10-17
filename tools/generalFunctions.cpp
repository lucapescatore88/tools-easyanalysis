#include "generalFunctions.hpp"


/**
  \brief generates random events following a distribution
  @param nevt: number of events to generate
  @param var: variable
  @param pdf: distribution of the events
  @param opt: "-showperc" to view progress bar
  @param tree: add events to a pre-existing tree.
  */

bool genRndm(RooRealVar * var, RooCurve * curve, double * xval, double ymax, TRandom3 * rdm_x, double smear)
{
    (*xval) = rdm_x->Rndm() * (var->getMax() - var->getMin()) + var->getMin();
    if (smear > 0) (*xval) = rdm_x->Gaus(*xval, smear);

    double yval = rdm_x->Rndm() * ymax * 1.01;
    double yf   = curve->Eval( *xval );

    return (bool)(yval <= yf);
}


TTree * generate(RooArgSet * set, RooAbsPdf * pdf, int nevt, string opt)
{
    TRandom3 rdm_x(0);
    size_t posseed = opt.find("-seed");
    if (posseed != string::npos)
    {
        int seed = ((TString)(opt.substr(posseed + 5))).Atof();
        rdm_x.SetSeed(seed);
    }

    double smear = -1;
    size_t possmear = opt.find("-smear");
    if (possmear != string::npos) smear = ((TString)(opt.substr(possmear + 6))).Atof();

    // Setup tree

    TTree * newTree = new TTree("gentree", "");

    if (opt.find("-genextended") != string::npos) nevt = rdm_x.Poisson(nevt);
    if (nevt <= 0) return newTree;

    vector < RooCurve * >  curve;
    vector < double > ymax;
    double xval[set->getSize()];
    TIterator  * it = set->createIterator();
    RooRealVar * arg;
    unsigned vv = 0;
    while ( (arg = (RooRealVar*)it->Next()) )
    {
        TString argname = (TString)arg->GetName();
        newTree->Branch((TString)arg->GetName(), &(xval[vv]), argname + "/D");
        RooPlot * frame = arg->frame(Name("genplot_" + argname));
        pdf->plotOn(frame, Name("pdf_" + argname));
        curve.push_back(frame->getCurve("pdf_" + argname));
        ymax.push_back(curve.back()->getYAxisMax());
        vv++;
    }

    // Generate

    while ( newTree->GetEntries() < nevt )
    {
        if (opt.find("-perc") != string::npos) showPercentage(newTree->GetEntries(), nevt);

        bool passed = true;
        unsigned vv = 0;
        TIterator * it2 = set->createIterator();
        RooRealVar * arg2;
        while ( (arg2 = (RooRealVar*)it2->Next()) )
        {
            passed *= genRndm(arg2, curve[vv], &(xval[vv]), ymax[vv], &rdm_x, smear);
            vv++;
            if (!passed) break;
        }
        if (passed) newTree->Fill();
    }

    return newTree;
}



TTree * generate(RooArgSet * set, RooAbsPdf * pdfSig, float nsig, RooAbsPdf * pdfBkg, float nbkg, string opt)
{
    double ntot = nsig + nbkg;
    double frac = nsig / ntot;
    RooRealVar * f_sig = new RooRealVar("f_sig", "f_sig", frac);
    RooAbsPdf * tot = new RooAddPdf("total_pdf", "total_pdf", RooArgSet(*pdfSig, *pdfBkg), RooArgSet(*f_sig));
    cout << "Generating nsig/ntot = " << f_sig->getVal() << endl;
    return generate(set, tot, ntot, opt);
}


RooDataSet * generateDataSet(TString name, RooArgSet * set, RooAbsPdf * pdfSig, int nsig, RooAbsPdf * pdfBkg, int nbkg, string opt)
{
    TTree * genTree = generate(set, pdfSig, nsig, pdfBkg, nbkg, opt);
    return (new RooDataSet("genDataSet" + name, "", genTree, *set));
}

RooDataSet * generateDataSet(TString name, RooArgSet * set, RooAbsPdf * pdf, int nevt, string opt)
{
    TTree * genTree = generate(set, pdf, nevt, opt);
    return (new RooDataSet("genDataSet" + name, "", genTree, *set));
}








/**
  \brief Returns luminosity and its error from an LHCb data file
  */

double luminosity( vector<TString > namefile, string doerr )
{
    double totlumi = 0, totlumi_err = 0;

    TreeReader * reader = new TreeReader("GetIntegratedLuminosity/LumiTuple");
    for ( unsigned k = 0; k < namefile.size(); ++k ) reader->AddFile(namefile[k]);
    reader->Initialize();

    int ntot = reader->GetEntries();

    for ( int i = 0; i < ntot; ++i )
    {
        reader->GetEntry(i);
        totlumi += reader->GetValue("IntegratedLuminosity");
        totlumi_err += reader->GetValue("IntegratedLuminosityErr");
    }

    if (doerr != "err") cout << "Total luminosity = " << totlumi / 1000. << " +/- " << totlumi_err / 1000. << " fb-1" << endl;

    if (doerr == "err") return totlumi_err / 1000.;
    else return totlumi / 1000.;
}


double luminosity( TString namefile, string doerr )
{
    vector < TString > v;
    v.push_back(namefile);

    return luminosity(v, doerr);
}



/**
  \biref Calculates chi2 and ndf from a RooAbsPdf and an histogram
  Returnt a pointer with two components [chi2/ndf, ndf]
  */

double * calcChi2(RooPlot * frame, unsigned npar, double * range, bool extended)
{
    RooHist * hist = frame->getHist("data");
    RooCurve * curve = frame->getCurve("model");
    if (!(hist && curve)) return NULL;

    double ndf = 0, mychi2 = 0;
    double * chi2 = new double[2];
    chi2[0] = chi2[1] = -1;

    int     n = hist->GetN();
    double* x = hist->GetX();
    double* y = hist->GetY();
    float binWidth = (x[n - 1] - x[0]) / n;

    for (int i = 0; i < n; i++)
    {
        float lowedge = x[ i ] - binWidth / 2.;
        float upedge = x[ i ] + binWidth / 2.;
        if (range) if (lowedge < range[0] && upedge > range[1]) continue;

        double cont = y[ i ];
        double err = hist->GetErrorY(i);
        double fval = curve->Eval( x[ i ] );

        if (TMath::Abs(cont) > 0 && TMath::Abs(err) > 0 && TMath::Abs(fval) > 0)
        {
            mychi2 += TMath::Power((cont - fval) / err, 2);
            ndf++;
        }
    }

    ndf -= npar;
    if (extended) ndf ++;

    chi2[0] = mychi2 / ndf;
    chi2[1] = ndf;
    return chi2;
}



/*
   \brief Functions to extract a pull histogram from a RooPlot
   */

// Evaluates the residual:
double pull( double datum, double pdf )
{
    double chi2 = 0.;

    if ( pdf > 0 )
        chi2 += 2. * ( pdf - datum );

    if ( datum > 0 && pdf > 0 )
        chi2 += 2. * datum * log( datum / pdf );

    return ( ( datum >= pdf ) ? sqrt( chi2 ) : -sqrt( chi2 ) );
}

double residual( double datum, double pdf )
{
    return ( (datum - pdf) / datum );
}

// Makes the RooHist of the residual.
TH1D* residualHist( const RooHist* rhist, const RooCurve* curve, float * range, string opt )
{
    int      n = rhist->GetN();
    double * x = rhist->GetX();
    double * y = rhist->GetY();
    float binWidth = (x[n - 1] - x[0]) / n;

    double xMin = x[ 0     ] - binWidth;
    double xMax = x[ n - 1 ] + binWidth;
    TH1D* residuals_temp = new TH1D( "r", "", n, xMin, xMax );
    double datum = 0.;
    double pdf   = 0.;
    for (int bin = 0; bin < n; bin++)
    {
        if (range)
        {
            float lowedge = x[ bin ] - binWidth / 2.;
            float upedge  = x[ bin ] + binWidth / 2.;
            if (lowedge < range[0] && upedge > range[1]) continue;
        }
        datum = y[ bin ];
        pdf   = curve->Eval( x[ bin ] );
        if (datum <= 0) continue;
        double stat = 0;
        if ( opt.find("r") != string::npos ) stat = residual( datum, pdf );
        else stat = pull( datum, pdf );
        residuals_temp->SetBinContent( bin + 1, stat );
        residuals_temp->SetBinError  ( bin + 1, 1. );
    }

    residuals_temp->SetMinimum    ( -5.   );
    residuals_temp->SetMaximum    (  5.   );
    residuals_temp->SetStats      ( false );
    residuals_temp->SetMarkerStyle(  8    );
    residuals_temp->SetMarkerSize ( .8    );

    return residuals_temp;
}


TH1 * getPulls(RooPlot * pl, float * range, string opt, string data, string model)
{
    RooHist* histogram = pl->getHist(data.c_str());
    RooCurve* curve = pl->getCurve(model.c_str());
    if (!(histogram && curve)) return NULL;
    return residualHist(histogram, curve, range, opt);
}


/**
  \brief Computes the average of the bin contents of a TH1 histogram.
  It is a weighted average using the error of the bin as sigma.
  */

vector<float> computeAverage(TH1* hist)
{
    float sumweight = 0;
    float avg = 0;

    vector<float> result;

    for (int i = 1; i <= hist->GetNbinsX(); i++)
    {
        float cont = hist->GetBinContent(i);
        float err = hist->GetBinError(i);

        if ( TMath::Abs(err) > 0. )
        {
            avg += (TMath::Abs(cont) / (err * err));
            sumweight += 1. / (err * err);
        }
    }


    if (!sumweight)
    {
        cout << "Sum of weights == 0" << endl;
        result.push_back(0.);
        result.push_back(0.);
    }
    else
    {
        result.push_back(avg / sumweight);
        result.push_back(TMath::Sqrt(1. / sumweight));
    }

    return result;
}



/**
  \brief Computes the average of the bin contents of a TH2 histogram.
  It is a weighted average using the error of the bin as sigma.
  */

vector<float> computeAverage2D(TH2* hist2D)
{
    vector<float> res;
    vector<float> tmpAvg;

    float avg2D = 0;
    float sumweight2D = 0;

    for (int i = 1; i <= hist2D->GetNbinsX(); i++)
    {
        TH1F * proj = (TH1F *)hist2D->ProjectionY(Form("proj_%i", i), i, i, "e");
        tmpAvg = computeAverage(proj);
        if ( TMath::Abs(tmpAvg[1]) > 0. )
        {
            avg2D += tmpAvg[0] / (tmpAvg[1] * tmpAvg[1]);
            sumweight2D += 1. / (tmpAvg[1] * tmpAvg[1]);
        }
    }

    if (!sumweight2D)
    {
        cout << "Sum 2D of weights == 0" << endl;
        res.push_back(0.);
        res.push_back(0.);
    }
    else
    {
        res.push_back(avg2D / sumweight2D);
        res.push_back(TMath::Sqrt(1. / sumweight2D));
    }

    return res;
}



/**
  \brief Extracts a list of strings from a file
  More in general I use it to convert a file with a list of file names to a vector of strings
  */

vector<string> getFilesNames(string filename)
{
    ifstream f(filename.c_str());
    vector<string> name_list;

    if (f.is_open())
    {
        string fname;

        while (f.good())
        {
            getline(f, fname);
            if (fname.find("root") != string::npos) name_list.push_back(fname);
        }
        f.close();

        cout << "added " << name_list.size() << " files" << endl;
    }
    else cout << "unable to open file" << endl;

    return name_list;
}



/**
  \brief This function takes a TH2D histogram and returns a vector of nRandom TH2F
  Each TH2F in the list is a gaussian fluctuation around the initial one.
  Parameter "limit" limits the fluctuations between 0 and 1 (usefull for efficiencies).
  */

vector<TH1*> createRandomFluctuations(TH2D * input, int nRandom, char limit)
{
    vector<TH1*> res;

    for (int ihist = 0; ihist < nRandom; ihist++)
    {
        TString name(Form("h_%i", ihist));
        TH2D * h = (TH2D *)input->Clone(name);
        h->Reset();
        h->SetName(name);

        for (int ibin = 1; ibin <= h->GetNbinsX(); ibin++)
        {
            for (int jbin = 1; jbin <= h->GetNbinsY(); )
            {
                float cont = input->GetBinContent(ibin, jbin);
                float err = input->GetBinError(ibin, jbin);

                TRandom3 rdm(0);
                float fluct = rdm.Gaus(cont, err);

                if (limit != 'E') { h->SetBinContent(ibin, jbin, fluct); jbin++; }
                else
                {
                    if (cont >= 0. && cont <= 1. && fluct >= 0. && fluct <= 1.)
                    {
                        h->SetBinContent(ibin, jbin, fluct); jbin++;
                    }
                    else
                    {
                        TF1 *pdf = new TF1("pdf", "gausn(0)", -10., 10.);
                        pdf->SetParameter(0, 1.);
                        pdf->SetParameter(1, cont);
                        pdf->SetParameter(2, err);
                        double norm = pdf->Integral(0., 1.);

                        if (norm < 1.e-3 || norm > 1.)
                        {
                            if ( cont > 1.) { h->SetBinContent(ibin, jbin, 1.); jbin++; }
                            else if ( cont < 0. ) { h->SetBinContent(ibin, jbin, 0.); jbin++; }
                        }
                        else if (fluct >= 0. && fluct <= 1.) { h->SetBinContent(ibin, jbin, fluct); jbin++; }

                        delete pdf;
                    }
                }
            }
        }

        res.push_back(h);
    }

    return res;
}


/** \brief Extracts a TH1 from a TH2
 * By extracting the row with Y = slice
 * */

TH1 * getSliceX(TH2 *hHisto, double slice)
{
    int bin = hHisto->GetXaxis()->FindBin(slice);
    return getSliceX(hHisto, bin);
}

/** \brief Extracts a TH1 from a TH2
 * By extracting the Y row, where "bin" is the bin number (remember starts from 1)
 * */

TH1 * getSliceX(TH2 *hHisto, int bin)
{
    TH1 *hSlice = new TH1D("", "", hHisto->GetNbinsY(), hHisto->GetYaxis()->GetXmin(), hHisto->GetYaxis()->GetXmax());
    hSlice->SetXTitle(hHisto->GetYaxis()->GetTitle());

    for (int i = 1; i <= hHisto->GetNbinsY(); i++)
    {
        hSlice->SetBinContent(i, hHisto->GetBinContent(bin, i));
        hSlice->SetBinError(i, hHisto->GetBinError(bin, i));
    }

    return hSlice;
}



/**
  \brief Sets to zero bins with percent error grater than cut and outside the interval [min,max]
  */

void cleanHistos(TH1 &histo, float cut, float min, float max)
{
    for (int j = 0; j != histo.GetNbinsX(); ++j)
    {
        if (histo.GetBinContent(j) != 0)
        {
            if ( (histo.GetBinError(j) / histo.GetBinContent(j)) > cut || histo.GetBinCenter(j) < min || histo.GetBinCenter(j) > max)
            {
                histo.SetBinContent(j, 0);
                histo.SetBinError(j, 0);
            }
        }
    }
}



/**
  \brief Shifts all bins of the histogram adding "shift" to all bins
  */
void ShiftHistos(TH1 &histo, float shift)
{
    for (int j = 1; j != histo.GetNbinsX() + 1; ++j)
    {
        float cont = histo.GetBinContent(j);
        if (cont != 0.) histo.SetBinContent(j, cont + shift);
    }
}



/**
 * \brief Put labels to histogram
 *
 *  @param histo:  Histo to modify
 *  @param labels: Labels to set
 *  @param nDiv:   puts only one label every nDiv bins
 *  @param dNmax:  after N = dNmax put one label in every bin anyway
 */


TH1* setLabels(TH1* histo, vector<float > &labels, int nDiv, float dNmax)
{
    for (int j = 1; j != histo->GetNbinsX() + 1; ++j)
    {
        if (labels[j - 1] == 0) break;
        else if ((j - 1) % nDiv == 0 || nDiv == -1 || labels[j - 1] > dNmax)
        {
            stringstream lab;
            lab << fixed << setprecision(1) << setw(3) << labels[j - 1];
            histo->GetXaxis()->SetBinLabel(j, lab.str().c_str());
        }
        else histo->GetXaxis()->SetBinLabel(j, "");
    }

    return histo;
}



/**
  \brief Divides each bin by its width
  */

void uniformBins(TH1 *histo)
{
    for (int j = 1; j != histo->GetNbinsX() + 1; ++j)
    {
        float cont = histo->GetBinContent(j);
        float err = histo->GetBinError(j);
        histo->SetBinContent(j, cont / histo->GetBinWidth(j));
        histo->SetBinError(j, err / histo->GetBinWidth(j));
    }
}



/** \brief Computes error with MC technique over mean and integral
 *
 * @param mean: if mean = true computes error over mean, if false over integral
 * @param formula: the formula of the fit function
 * @param best: value of the mean/integral obtained from the fit
 * @param param: parameters obtained from the fit
 * @param err: errors on the parameters obtained from the fit
 * @param ntry: number of random values generated
 * @param gauss: if true generates random values gaussian distributed, if false unifomely
 * @param valAss: if true deviation is computed ad the absolute value of the difference bertween random value and fit value, if false deviation in computed as normal standard vediation
 *
 */

float computeMCError(bool mean, char* formula, float best, Double_t *param, Double_t *err, int ntry, bool gauss, bool valAss)
{

    TF1* funz = new TF1("funz", formula, 0, 100);

    vector<float> cand;
    float error = 0;
    float rms = 0;

    for (int j = 0; j != ntry; ++j)
    {
        TRandom3 rand(j);

        float par1 = 0, par2 = 0, par3 = 0;
        if (gauss)
        {
            par1 = rand.Gaus(param[0], err[0]);
            par2 = rand.Gaus(param[1], err[1]);
            par3 = rand.Gaus(param[2], err[2]);
        }
        else
        {
            par1 = rand.Uniform(param[0] - err[0], param[0] + err[0]);
            par2 = rand.Uniform(param[1] - err[1], param[1] + err[1]);
            par3 = rand.Uniform(param[2] - err[2], param[2] + err[2]);
        }


        funz->SetParameters(par1, par2, par3);
        float rndm = 0;
        if (mean) rndm = funz->Mean(0., 20.);
        else rndm = funz->Integral(0., 20.);


        float absdiff = rndm - best;
        if ((rndm - best) < 0) absdiff = -(rndm - best);

        rms += pow(absdiff, 2);

        error += absdiff;
        cand.push_back(absdiff);
    }

    float sigma = sqrt(rms / (ntry - 1));

    float in = 0;
    if (valAss) { for (size_t w = 0; w != cand.size(); ++w) { if (cand[w] <= 2 * error / ntry ) in++;}}
    else { for (size_t w = 0; w != cand.size(); ++w) { if (cand[w] <= 2 * sigma ) in++;}}

    cout << "******* sigma = " << sigma << " ********* " << " whithin_2sigma = " << in / ntry * 100. << "% *************" << endl;

    if (valAss) return error / ntry;
    else return sigma;
}



/**
  \brief Divides each bin for it's center plus a constant
  */

void divideForBinCenter(TH1 &histo, float cost)
{
    for (int j = 1; j != histo.GetNbinsX() + 1; ++j )
    {
        float cont = histo.GetBinContent(j);
        if (cont != 0)
        {
            float err = histo.GetBinError(j);
            float center = histo.GetBinCenter(j);

            histo.SetBinContent(j, cont / (center + cost));
            histo.SetBinError(j, err / (center + cost));
        }
    }
}



/** \brief Rebins "histo" in bins wide as described by the vector "sizes"
 *
 * @param init: start value of the binning
 * @param sizes: sizes of the bins
 * @param media: if media = true joins bins by averaging contents, if false by summing contents
 *
 */


TH1F * rebinHisto(TH1 &histo, float init, vector<float> sizes, char* title, bool media)
{
    vector <float> Bin;
    float bin = init;
    Bin.push_back(bin);
    for (size_t w = 0; w < sizes.size(); ++w)
    {
        bin += sizes[w];
        Bin.push_back(bin);
    }

    stringstream name;
    if (title == 0) name << histo.GetName() << "_Rebinned";
    else name << title;

    TH1F* RebinnedHisto = new TH1F(name.str().c_str(), "", Bin.size() - 1, &Bin[0]);
    cout << "ciao4  " << Bin.size() << endl;
    for (size_t ibin = 1; ibin < Bin.size(); ++ibin)
    {
        float val1 = Bin[ibin - 1];
        float val2 = Bin[ibin];
        cout << val1 << "   " << val2 << endl;
        float start = histo.GetXaxis()->FindBin(val1);
        float stop = histo.GetXaxis()->FindBin(val2);

        float integ = histo.Integral(start, stop - 1);
        if (media) RebinnedHisto->SetBinContent(ibin, integ / (stop - start));
        else RebinnedHisto->SetBinContent(ibin, integ);

        if (media) RebinnedHisto->SetBinError(ibin, sqrt(integ) / (stop - start)); //N.B.: NON CORRETTO!!!!
        else RebinnedHisto->SetBinError(ibin, sqrt(integ));
    }

    return RebinnedHisto;
}



/** \brief Rebins "histo" using an other histogram as template
 *
 * @param media: if media = true joins bins by averaging contents, if false by summing contents
 *
 */

TH1F *rebinHisto(TH1 &histo, TH1 &tamplateHisto, char* title, bool media)
{
    vector<float> sizes;
    float init = tamplateHisto.GetBinLowEdge(1);
    for (int tmpBin = 1; tmpBin < tamplateHisto.GetNbinsX() + 1; tmpBin++) sizes.push_back(tamplateHisto.GetBinWidth(tmpBin));

    TH1F* RebinnedHisto = rebinHisto(histo, init, sizes, title, media);

    return RebinnedHisto;
}



/** \brief Rebins "histo" by joining bins with an error greater than "error"
 *
 * @param media: if media = true joins bins by averaging contents, if false by summing contents
 *
 */

TH1F *rebinHisto(TH1 &histo, float error, char* title, bool media)
{
    vector<float> edges;
    vector<float> sizes;
    float init = histo.GetBinLowEdge(1);
    float err = 0;
    float cont = 0;
    for (int tmpBin = 1; tmpBin != histo.GetNbinsX(); tmpBin++)
    {
        err += histo.GetBinError(tmpBin);
        cont += histo.GetBinContent(tmpBin);

        if (cont != 0)
        {
            if (err / cont <= error)
            {
                edges.push_back(histo.GetBinLowEdge(tmpBin + 1));
                err = 0;
                cont = 0;
            }
        }
    }

    edges.push_back(histo.GetBinLowEdge(histo.GetNbinsX() + 1));

    for (size_t j = 1; j != edges.size(); j++) sizes.push_back(edges[j] - edges[j - 1]);

    TH1F* RebinnedHisto = rebinHisto(histo, init, sizes, title, media);

    return RebinnedHisto;
}



/**
  \brief Computes sqrt(statErr^2 + sysErr^2) and sets it to the histogram
  */

void addSystematicError(TH1 &histo, float sysError)
{
    for (int tmpBin = 1; tmpBin != histo.GetNbinsX() + 1; tmpBin++)
    {
        float err = histo.GetBinError(tmpBin);
        float cont = histo.GetBinContent(tmpBin);
        histo.SetBinError(tmpBin, sqrt( pow(err, 2) + pow(cont * sysError, 2)));
    }
}


/**
  \brief Computes the scalar product of vector "v" with scalar "c"
  */

vector <double> scalarProd(vector<double> v, double c)
{
    vector<double> res = v;
    for (unsigned n = 0; n < v.size(); n++) res[n] *= c;

    return res;
}



/**
  \brief This function takes in a string and converts it to a binning scheme.
  Returns an array of doubles with bin boundaries and _nbins if specified will contain the number of bins.
  <br>2 possible options:
  <br>- opt = "unif" (default) converts a string in the form [nbins,min,max] in a uniform binning
  <br>- opt = "custom" converts a string containing bin bundaries. Of the form [x1,x2....xn]
  */

double * decodeBinning(string str, int * _nbins, string opt)
{
    double * res = NULL;
    if (opt == "unif")
    {
        str.erase(0, 1);
        int pos = str.find(",");
        int pos2 = str.find(",", pos + 1);
        double max = ((TString)str.substr(pos2 + 1, string::npos)).Atof();
        double min = ((TString)str.substr(pos + 1, pos2 - pos)).Atof();
        str.erase(pos, string::npos);
        int nbins = ((TString)str).Atof();

        if (_nbins) *_nbins = nbins;
        res = new double[nbins + 1];
        for (int i = 0; i <= nbins; i++) res[i] = min + i * (max - min) / (double)nbins;
    }
    else
    {
        vector < string > v;
        unsigned pos = 0;
        while (true)
        {
            unsigned pos2 = str.find(",", pos + 1);
            v.push_back(str.substr(pos + 1, pos2 - pos));
            if (pos2 > 1.e9) break;
            pos = pos2;
        }

        if (_nbins) *_nbins = v.size() - 1;
        res = new double[v.size()];
        for (unsigned i = 0; i < v.size(); i++) res[i] = ((TString)v[i]).Atof();
    }

    return res;
}

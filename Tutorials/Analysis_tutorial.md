# Tutorial to use the Analysis class

This class is intended to be a RooFit wrapper to simplify its use at least is some situations.
It can do three things:
    - manipulate and hold data in the form of TTree, allows to do cuts, add variables, etc
    - build models and fit data
    - use the information from a fit (e.g. to calculate the sWeight)

A full doxygen for the class can be found at
http://pluca.web.cern.ch/pluca/doxygen/annotated.html

This tutorial is in C++ but all classes can also be used in python.
Open the Analysis_tutorial_python tutorial to get the same instructions as here but in python. 

## PART 1 - Constructors and handling data

The base constructor for the class is:

```
Analysis (TString _name, TString _title, TreeReader *reader, TCut *_cuts, RooRealVar *_var=NULL, string _w="")
```

Where:
- "_name"  it's the name for the object and must be unique.
- "_title" it's just a title.
- "reader" it's a TreeReader object which contains a TChain and holds pointers to all its branches.
- "_cuts"  are cuts that you want to be applied no matter what
- "_var"   it's the observable to be fitted, it's not compulsory to have one if you just want to apply cuts.
- "_w"     it's the name of a branch of the TChain to be interpreted as weight

In case your tree is in a single file you can also use the simplified constructor

```
Analysis (TString _name, TString _title, string treename, string filename, RooRealVar *_var=NULL, TCut *_cuts=NULL, string _w="")
```

- "treename" it's the name of a tree in a file
- "filename" it's the file containing that tree

Other two constructors for particular situations are available (see the doxygen)

### Initialising: (VERY IMPORTANT)

Before using the object. After filling information on data but also on the model.
You need to initialise it by using ```Analysis::Initialize()```.
e.g. This is necessary before applyCuts() or before Fit().

Options: 
    By default "-exp-namepar" (Always divided by dashes!)
    
- "-exp"     : adds automatically and exponential bkg component
- "-namepar" : appends the object name to the model name
- "-nobkg"   : ignores the bkg information and runs as signal only 


---------------------------------------------

## PART 1 - Building models and fitting

You can add a model using the SetSignal() and addBkgComponent() method.

### Create PDFs: SetSignal()

```
ana->SetSignal (T *_sig, double _nsig=0, string opt="-namepar", Str2VarMap myvars=Str2VarMap(), string weight="")
```

Arguments:
- The first argument, "T*_sig", is the source of the signal shape.
      "T" can be any of these types: string, a TTree, a TH1, or a RooAbsPdf
      
   * string :  This is a string that describes a model.
           Available models are:

		**Gauss, DGauss, CB, DCB, DCB_OST, DCB_Sn, Poly, Cheb, Argus, CBGauss, Exp**
           
           To any model a gaussian can be added by adding "AndGauss" to its name e.g. CBAndGauss returns the sum of a gaussian and a CB
           N.B.: CBAndGauss and CBGauss are different because the latter had the "m" parameter in common between Gauss and CB
           DCB\_OST means DCB with Opposite Side Tails, and DCB_Sn means Same "n" (the tail slope if the 2 CB is kept in common)
           You can also convolute any model with a gaussian adding ConvGauss to its name.
           
           Then you can (it's not compulsory) specify parameters in the following way:
           
           *"model-Xparam1[val,min,max]-param2[val,min,max]"*
           
           Notice that an X in front of the name fixes the parameter.
           The min and max are not compulsory (there are defaults which may be good or not for you)
           
           e.g. "DCB_Sn-Xm[5382,5000,5500]-s[10,5,20]-s2[20,10,50]-n[1,0,10]"
           
           If you want to check which models are available and their parameters names they are listed in the last 3 functions in ModelBuilder.cpp

  *  TTree : In this case the Analysis object used the RooKeysPdf class to extract a smooth shape from the TTree
           N.B.: The observable you used in the constructor MUST be in the tree with the same name.
           For more fancy things see "opt".
           
  *  TH1   : In this case the RooHistPdf class is used instead
  *  RooAbsPdf : If nothing above satisfies you you can buil a model of your own and give it to the fitter
    
     
- _nsig  This is the yield connected to the signal. You can give it a double or a RooRealVar.
			If you give it a double it will build a RooRealVar following this rules
			\_nsig > 0 -> is a  RooRealVar with initial value "_nsig"
			\_nsig < 0 -> is a RooRealVar with value "\_nsig" and fixed in the fit (the signal yield will not float)
			
- opt  : This can be used for extra options:
  			Using a TTree as source you can specify in the options
  			"-s1" or "-s2" to change the resolution of the smoothing
  			-v[var1,var2,...]-c[cuts] to apply a cut to the tree before extracting the distribution
  			N.B.: In this case you must speficy in -v[] all variables needed for the cut
  
- myvars  : This is a Str2VarMap object (a map connecting RooRealVar's and their names)
               This can be used to use parameters of previous fits with all their properties.
               N.B.: It will override any parameter set in the name description
			 
		e.g. Let's imagine you need to fit your rare signal but the statistics is not enough to constrain a fully-free DCB
		Then you want to fit J/psi and use its parameters for the rare fit.
			   
			   ```
			   Analysis * anaJpsi = new Analysis(...)
			   Analysis * anaRare = new Analysis(...)
			   anaJpsi->SetSignal(...);
			   anaJpsi->addBkgCompnent(...);
			   anaJpsi->Initialize("");
			   anaJpsi->Fit();                                 //Fit Jpsi
			   Str2VarMap jpsiPars = anaJpsi->GetSigParams();  //Retrieve parameters
			   anaRare->SetSignal("...",50,"",jpsiPars);       //Use parameters.
			   //This fits a resonant samples and used the parameters found for the rare fit
			   ```
	
	
#### Backgrounds

```
ana->addBkgComponent (const char *_name, T *_comp, double _frac=0, string opt="-namepar", Str2VarMap myvars=Str2VarMap(), string weight="")
```

To add a bkg component. As many as you like. There it is required to give the component an unique name "_name".
Works very similar to SetSignal() so I won't repeat. One difference is the following:

"_frac" it's the current bkg component yield can be again a RooRealVar or a RooFormulaVar.
Passing a RooFormulaVar can be useful if you want to bind 2 bkg yield together.

e.g. Let's say you know the ratio between Bd and Bs you expect.
Then you can define the Bd and Bs yields as

```
RooRealVar * NBd = new RooRealVar("NBd",...)
RooFormulaVar * NBs = new RooFormulaVar("NBs","somefactor * NBd",*NBd);
```

Then giving NBd and NBs to addBkgComponent() will happen that NBd is free in the fit and NBs linked to it.

As for the signal _frac > 0 means free and _frac < 0 means fixed but in addition
if |_frac| < 1. It is not interpreted as an absolute yield but as a fraction wrt of the signal yield.


### Fit 

```
ana->Fit(double min=0, double max=0., unsigned nbins=50, bool unbinned=false, string print="-range-log", TCut mycuts="")
```

After setting the model and initializing you can fit.

min,max:	fitting interval, if min => max all available is used
nbins:	n of bins to use (if unbinned this is only for display)
unbinned:	true for unbinned fit
print:	Options (includes also all options of ModelBuilder::Print() and GetFrame()) 
Here I report just the ones particular to Fit and a few others.
-  "-quiet" -> shell output minimized 
-  "-sumW2err" -> if weighted data errors shown reflect statistics of initial sample 
-  "-log" -> logarithmic plot 
-  "-pulls" or 
-  "-ANDpulls" -> if data is inserted these add a pull histogram -pulls in other plot -ANDpulls under fit plot 
-  "-range" -> plots only the fitted range, otherwise all available is plot 
-  "-noPlot" -> doesn't print and only returns the frame 
-  "-minos" -> Enables MINOS for asymmetric errors
cuts:	cuts to make just before fitting

After the fit a RooFitResult object is stored inside the analysis object and can be retrieved using GetFitRsult().

Notice that is not compulsory to use the standard function for fit.
If you want to build you own fancy fitter you can use it on the data and models built like this:

```
myfancyfitter(ana->getModel(),ana->GetDataSet())
```


##   Just one example 

```
RooRealVar * MM = new RooRealVar("MM","MM",5300,5000,6000);
Analysis * anaRare = new Analysis("my_ana","B0","myB0tree","myfile",MM)
TCut cuts = "somecuts";
 
anaRare->SetSignal("DCB-Xm[3283]");       // Double Crystal Ball with mass parameter fixed to 3283
anaRare->SetSignal("Combinatorial","Exp-b[-0.005,-0.03,0.01]"); // Exponential with slope variable between -0.03 and 0.01
anaRare->Initialize("");  // Never forget this!
anaRare->applyCuts(cuts);  // Apply the cuts "cuts" (cuts is a (TCut *))
anaRare->Fit(5300,5800,50,true,"-minos-quiet") // Fit unbinned in the given interval using minos
// The output will be fancy nice plots with legends and parameter boxes
```


---------------------------------------------

## PART 2 :Handling data:

### Apply cuts

```
TTree * applyCuts (TCut _cuts=NULL, bool substtree=true, void(*addFunc)(TreeReader *, TTree *, bool)=NULL, double frac=1)
```

Applies the logical and between the cuts added in the constructor and the ones passed to the method.
It returns a reduced TTree and by default it stores it in the object too.
N.B.: The initial dataset will not be modified so you can reapply different cuts.

- substtree :  The reduced tree is returned and if substtree=true if also stored in the object (for a future fit for example)
- frac      :  must be 0 < frac <= 1. It selects only the first frac*100% of the events in the initial dataset.
- addFunc(TreeReader *, TTree *, bool)

You may define a function to add variables to the tree too. This has to follow the template below.
The addFunc is called ones at the beginning of the event loop with 3rd argument = true.
And then once per event with 3rd argument = false. The idea is to let you initialise branches and then fill them.

```
void addFunc(TreeReader *reader, TTree *tree, bool reset)
{
	static var1, var2, ...;
	
	if(reset)
	{
		// Set branches and initialize stuff
	}
	else
	{
		//Do stuff 
	}
}
```

### Check for multiple cadidates

```
TTree * CheckMultiple(FUNC_PTR choose=NULL)
```

This goes through the dataset and checks if there are multiple candidates in a single event 
(looking at the Event and Run numbers). It produces an histogram of #cand per event 
and returns a tree with a variable "isSingle" added.

==> isSingle is "true" only for 1 candidate per event
Normally that candidate is chosen at random (using the already implemented randomKill() function)
But you may want to use a different method e.g. the event with best PID.
In this case you can pass to the method a function following this template:

```
Long64_t choose	(TreeReader * reader, vector< Long64_t > entry )
{
	//Typycally loop on entry and do something with reader
	return chosen_entry; // return the number of the selected candidate
}	
```

### RooDataSet * CreateDataSet()

This function simply used the information in the Analysis object to produce a RooDataSet which then you can use.
N.B.: By default the RooDataSet will contain only the observable you specified in the constructor.
If you want it to include more variables you can do it by using

```
Analysis::addVariable(name)
```

Example:

```
RooRealVar * MM = new RooRealVar("MM","MM",5300,5000,6000);
Analysis * ana = new Analysis("my_ana","B0","myB0tree","myfile",MM);
ana->addVariable("Jpsi_MM");
ana->Initialize("");
RooDataSet * ana->GetDataSet();
```

Notice that for fitting it is not necessary to get the dataset. All is done automatically.
This is just in case you want to do something with the dataset by yourself.




#### Scaling and units

Two functions are provided to set units. One is just SetUnits(string,factor)

e.g. ```ana->SetUnits("MeV")``` ---> A label [MeV] will appear on the axis
If factor is not specified then factor = 1 and nothing happens
If factor != 1 (```ana->SetUnits("MeV",0.01)```) then the first time that 
one creates the DataSet (initializes) a new observable 
is created by rescaling each entry by the factor. This may take some time but just once.

An other function is also provided which finds the factor for you.

e.g. ```ana->SetUnits("MeV","GeV")``` means that the input data is in MeV and you want the output plots in GeV
So the dataset will be scaled by 1000 and "GeV/#it{c}^{2}" sill be used as label


### Blinding

You can perform blind analysis by using 

```
ana->SetBlindRegion(min,max).
```

The plots will be blinded in that region (can also be more than one blind interval).
Be aware that data is not removed and does contribute in the fit. It's just not shown.
Since data contributes parameters of the fit and yields will be hidden and the chi2 as well.
If you need more than one region be careful to add them in sequence from low
to high and also to check that they don't overlap.


### Generate events

You can also creare an analysis with an empty constructor and then set a model, initialise, 
and generate a MC dataset.
Two functions are provided Generate(...). One produces a definite total number of events 
(sig + bkg). The other instead produced a specific number of sig and total bkg.
If you just need to generate signal (n background) a quick constructor is provided.
     
     ```
Analysis *toy = Analysis("toy", MM);
toy->SetSignal(...);
toy->addBkgComponent(...)
toy->Initialize("");
toy->Generate(50,100); //50 signal and 100 bkg
     ```
     
If you do not have background but one single PDF
(e.g. I want to generate a gaussian distribution)
then you can use directly a dedicated contructor.

```
Analysis (TString _name, RooRealVar *_var, RooAbsPdf *pdf=NULL, int ngen=1000, string opt="-subtree")
```
Example

```
Analysis *toy = Analysis("toy", MM, "Gauss-m[5000]-s[20]", 1000);```
toy->Fit();
```

Not even need to be intialized in this case: all is automatic!


Using Generate() you can use a couple of useful options:
- seed(n)  -> were n will be used as seed (by default TRndom3(0) is used but this makes things not reproducible)
- smear(r) -> "r" is a double interpreted as resolution. The generator will perform a gaussian smearing.
   Namely for each number generated following the given PDF this will not be directly stored.
   But a second random number will be generated normally distributed with sigma = r and mean at the first value.
     


###   Just one example 
```
TreeReader * reader = new TreeReader("mytree");
reader->AddFile("file1.root");
reader->AddFile("file2.root"); 
// I had here to add 2 files. It it's only one you can give directly name 
// of tree and file to the Analysis constructor

RooRealVar * MM = new RooRealVar("MM","MM",5300,5000,6000);
Analysis * ana = new Analysis("Bs2MM", "B_{s}#rightarrow#mu#mu", reader, "", MM);
TCut cuts = "somecuts";
 
ana->SetSignal("DCB-Xm[3283]");
ana->SetSignal("Combinatorial","Exp-b[-0.005,-0.03,0.01]");
ana->Initialize("");
ana->applyCuts(cuts);
ana->Fit(5300,5800,50,true,"-minos-quiet")
```
----------------------------------------------------------

## PART 3 - Simultaneous fits

The MultiAnalysis class is available to make simultaneous fits.
Almost all you need to know you learnt in the previous parts.
In fact to use the MultiAnalysis you need to create you Analysis
objects first and then give them to the MultiAnalysis object
using the AddCategory() function. Finally, you can use the
SimultaousFit() function to fit.

e.g.

```
Analysis * ana1 = new Analysis(...);
ana1->SetSignal(...);
...
Analysis * ana2 = new Analysis(...);
ana2->SetSignal(...);
...

MultiAnalysis * multiana = new MultiAnalysis("somename");
multiana->AddCategory(ana1,"name_for_ana1");
multiana->AddCategory(ana2,"name_for_ana2");
multiana->SimultaneousFit();
```

Notice that:
1) The two analysis object do not have to have the same variable
2) The two analysis objects do not need to have the same sample

For example you can take 2 variables from the same sample to make
a multudimensional fit or 2 separate samples to fit
different tipes of events at the same time.
While you specify the models you can assign shared parameters.
Custom functions such ad SetUniqueSignal() are available
which create automatically the Analysis object for the special
cases in which for example all samples are supposed to have the same
signal function.


#### Examples with shared parameters

In this specific example the same variable is fit from 2
independent samples and the same function is fit on both with
all same shape parameters but separate yields.

```
RooRealVar * MM = new RooRealVar("MM","mass",min,max)
Analysis * ana1 = new Analysis("MyCoolAnalyser1","B0","mytree","myfile");
ana1->SetSignal("DCB");
ana1->addBkgComponent("Combinatorial","Exp");
ana1->Initialize("");

// Str2VarMap is a c++ map object that maps a function parameters to their names
// You can use it for examples as pars["m"]->getVal() or ((RooRealVar *)pars["m"])->setConstant().
//Notice that the elements of the map are RooAbsReal namely RooRealVar or RooFormulaVar.
//Therefore for some purposes you need to cast.
Str2VarMap pars = ana1->GetSigParams();

Analysis * ana2 = new Analysis("MyCoolAnalyser2","B0","mytree2","myfile2");
ana2->SetSignal("DCB",0,"",pars); // Here is the key point for sharing. 
                                  // This function get as arguments: model, numer of events to begin
								  // with (0 is default), options and parameters to use. In this case
								  // all the shape parameters of the first Analsys object.

MultiAnalysis * multiana = new MultiAnalysis("somename");
multiana->AddCategory(ana1,"name_for_ana1");
multiana->AddCategory(ana2,"name_for_ana2");
multiana->SimultaneousFit();
```

In this second example the shape parameters are independent but the yields of the two samples are linked.
Say that you what to fit 2 samples of Downstream and Long candidates. But the decay is the same so
its branching ratio should be common.

```
RooRealVar * BR = new RooRealVar("BR","BR(B^0#rightarrow\mu\mu)",min,max)
RooAbsReal * NLong = new RooFormulaVar("NLong",Form("BR * %f",my_long_efficiency),RooArgSet( * BR));
RooAbsReal * NDown = new RooFormulaVar("NDown",Form("BR * %f",my_down_efficiency),RooArgSet( * BR));
// Two yields are defined as a function of a common BR

RooRealVar * MM = new RooRealVar("MM","mass",min,max)
Analysis * ana1 = new Analysis("Down","B0","mytree","myfile");
ana1->SetSignal("DCB",NDown); // I explicitely tell the object to you my RooFormulaVar for the yield
ana1->addBkgComponent("Combinatorial","Exp");

Analysis * ana2 = new Analysis("Long","B0","mytree2","myfile2");
ana2->SetSignal("DCB",NLong); // Long is connected to Down though the BR!
ana2->addBkgComponent("Combinatorial","Exp");

MultiAnalysis * multiana = new MultiAnalysis("somename");
multiana->AddCategory(ana1,"name_for_ana1");
multiana->AddCategory(ana2,"name_for_ana2");
multiana->SimultaneousFit();
```



----------------------------------------------------------

## PART 4 - After fitting: other useful methods

After fitting you can extract information!!

- GetNSig()            -> sig yield in a given interval
- GetNBkgVal()         -> bkg yield in a given interval
- PrintComposition()   -> Prints the fraction N_i/N_tot of each component in any given intervals
- GetSOverB()          -> returns S/B in a given interval
- GetSigFraction()          -> returns S/(S+B) in a given interval
- GetReducedSWeight(x) -> returns S(x)/(S(x) + B(x)) at a point
- GetParams()          -> Returns a Str2VarMap containing all parmeters of the model including yield
- GetSigparams()       -> Returns a Str2VarMap containing all parmeters of the signal PDF (excluding yield)
- PrintChi2()          -> Prints chi2 you can use GetChi2() to have chi2/NDF and GetNDF to have NDF
- CalcSWeight()        -> Will return a tree same as the stored tree with 2 variables added sW and sWR
	                          sW is the proper sWeight and sWR is the reduced SWeight (simpy S(x) / (S(x) + B(x)))
- and much else (see doxygen)	

import easyanalysis as ea
import ROOT as r
import yaml

def keys(d) :
    return d.keys()

def create_analysis(config) :

    ## Decode yaml file
    cont = None
    with open(config, 'r') as stream:
        
        try:    cont = yaml.load(stream)
        except yaml.YAMLError as exc: print(exc)

    print '----- CONFIGURATION -----'
    print cont
    
    ## Check all needed keys are present
    if set(['datafile','datatree','name','title','sig','var']) > set(keys(cont)) :
        print "Some necessary keys are missing... You have to specify at least: datafile, datatree, name, title, sigpdf, var"
        return

    cut = r.TCut("")
    if 'cut' in keys(cont) : cut = r.TCut(cont['cut'])

    if set(['name','min','max']) > set(keys(cont['var'])) :
        print "For the variable you have to specify: name, min, max"
        return 

    v = r.RooRealVar(cont['var']['name'],cont['var']['name'],cont['var']['min'],cont['var']['max'])
    a = ea.Analysis(cont['name'],cont['title'],cont['datatree'],cont['datafile'],v,cut)

    if 'units' in cont.keys() : a.SetUnits(cont['units'])

    #signorm = None
    if 'RooKeysPdf' in cont['sig']['pdf'] :
        ff = r.TFile.Open(cont['sig']['file'])
        tt = ff.Get(cont['sig']['tree'])
        a.SetSignal(tt) 
    else :
        a.SetSignal(cont['sig']['pdf'])

    if 'bkgs' not in keys(cont) :
        a.Initialize("")
        return a

    for bkg in cont['bkgs'] :

        ame = bkg.keys()[0]
        if 'RooKeysPdf' in bkg[name]['pdf'] :
            ff = r.TFile.Open(bkg[name]['file'])
            tt = ff.Get(bkg[name]['tree'])
            a.AddBkgComponent(name,tt)
        else :
            a.AddBkgComponent(name,bkg[name]['pdf'])

    a.Initialize("")
    print "----- CONFIGURED -----"

    return a









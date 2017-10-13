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
    if set(['datafile','datatree','name','title','sigpdf','var']) > set(keys(cont)) :
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
    a.SetSignal(cont['sigpdf'])

    if 'bkgs' not in keys(cont) :
        a.Initialize("")
        return a

    for bkg in cont['bkgs'] :

        print " Adding bkg"
        name = bkg.keys()[0]
        a.AddBkgComponent(name,bkg[name])

    a.Initialize("")
    print "----- CONFIGURED -----"

    return a









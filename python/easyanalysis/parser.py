import easyanalysis as ea
import ROOT as r
import yaml, json

analyses = {}

def keys(l) :

    keys = []
    for el in l :
        keys.append(el.keys()[0])
    return keys

def set_component(name,comp,an) :

    params = None
    if 'params' in comp.keys() :
        if comp['import'] not in analyses.keys() :
            print "Cannot find analysis", comp['import'], "to get parameters"
            return
        else :
            toks = comp['import'].split(':')
            imp = toks[0]
            if len(toks) == 1 : 
                print imp
                params = analyses[imp].GetSigParams()
            elif len(toks) > 1 : 
                if analyses[imp].GetBkgID(toks[1]) < 0 : 
                    print "No component", toks[1], "in", toks[0]
                    return
                params = analyses[imp].GetBkgParams(toks[1])

            plist = comp['params']
            if len(plist) > 0 :
                for p in params :

                    pname = p.first
                    if pname not in plist.keys() :
                        params.erase(params.find(pname))
                        continue

                    if plist[pname] == 'const' :
                        params[pname].setConstant()
                    elif plist[pname] == 'gauss' :
                        an.AddGaussConstraint(params[pname])
                    elif plist[pname] == 'free' :
                        params[pname].setConstant(0)

    norm = 0.
    if 'norm' in comp.keys() :
        if comp['norm'].isdigit() :
            norm = float(comp['norm'])
        #elif 'Roo' in comp['norm'] :
        #    norm = exec(comp['norm'])
        elif ':' in comp['norm'] :
            toks = comp['norm'].split(':')
            if toks[0] not in analyses.keys() :
                print "Cannot find analysis", toks[0], "to get parameters"
                return

            can = analyses[toks[0]]
            cname = 'sig'
            if len(toks) > 1 : 
                cname = toks[1]
                if can.GetBkgID(cname) < 0 :
                    print "No component", cname, "in", toks[0]
                    return

            if cname == 'sig' : norm = can.GetNSigPtr()
            else : norm = can.GetBkgFraction(cname)
            
    if 'RooKeysPdf' in comp['pdf'] :
        ff = r.TFile.Open(comp['file'])
        tt = ff.Get(comp['tree'])
        if name == 'sig' : an.SetSignal(tt)
        else : an.AddBkgComponent(name,tt)
    else :
        if params is None : 
            if name == 'sig' : an.SetSignal(comp['pdf'],norm)
            else : an.AddBkgComponent(name,comp['pdf'],norm)
        else :
            if name == 'sig' : an.SetSignal(comp['pdf'],norm,params)
            else : an.AddBkgComponent(name,comp['pdf'],norm,params)

    return an


def create_analysis(config) :

    ## Decode yaml file
    cont = None
    with open(config, 'r') as stream:
        
        reader = json
        if 'yaml' in config : reader = yaml

        try:    cont = reader.load(stream)
        except yaml.YAMLError as exc: print(exc)


    print '----- CONFIGURATION -----'
    print cont
    
    ## Check all needed keys are present
    if set(['datafile','datatree','name','title','sig','var']) > set(cont.keys()) :
        print "Some necessary keys are missing... You have to specify at least: datafile, datatree, name, title, sigpdf, var"
        return

    if 'preamble' in cont.keys() : exec(cont['preamble'])

    cut = r.TCut("")
    if 'cut' in cont.keys() : cut = r.TCut(cont['cut'])

    if set(['name','min','max']) > set(cont['var'].keys()) :
        print "For the variable you have to specify: name, min, max"
        return 

    v = r.RooRealVar(cont['var']['name'],cont['var']['name'],cont['var']['min'],cont['var']['max'])
    a = ea.Analysis(cont['name'],cont['title'],cont['datatree'],cont['datafile'],v,cut)
    analyses[cont['name']] = a

    if 'units' in cont.keys() : a.SetUnits(cont['units'])

    set_component('sig',cont['sig'],a)  

    if 'bkgs' in cont.keys() :
        for bkg in cont['bkgs'] :
            name = bkg.keys()[0]
            set_component(name,bkg[name],a)

    a.Initialize("")
    print "----- CONFIGURED -----"

    return a

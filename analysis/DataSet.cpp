#include "DataSet.hpp"

RooDataSet * convertTreeToData(TString name, TreeReader * reader, TString cuts, TString weight)
{
	RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);
	RooArgSet * varArgSet = new RooArgSet(name+"argSet");
	vector < variable * > varList = reader->GetVarList();
	for(unsigned i = 0; i < varList.size(); i++)
	{
		if(varList[i]->GetArraySize()>1 || varList[i]->GetType() == ULONG64 || varList[i]->GetType() == SHORT) continue;
		RooRealVar * v = new RooRealVar(varList[i]->name,varList[i]->title,0.);
		varArgSet->add(*v);
	}

	if(weight=="") return new RooDataSet("dataSet","",reader->GetChain(),*varArgSet,cuts);
	else return new RooDataSet(name,"",reader->GetChain(),*varArgSet,cuts,weight);
}


DataSet::DataSet(TString name, TString treename, TString filename, TString weight, TString cuts, vector <string> br, string opt)
{
	TreeReader * reader = new TreeReader(treename,filename);
	reader->Initialize(br,opt);
	data = convertTreeToData(name, reader, cuts, weight);
}

DataSet::DataSet(TString name, TreeReader * reader, TString weight, TString cuts, vector <string> br, string opt)
{
	if(br.size()>0) reader->Initialize(br,opt);
	data = convertTreeToData(name, reader, cuts, weight);
}

DataSet::DataSet(TString name, TTree * tree, TString weight, TString cuts, vector <string> br, string opt)
{
	TreeReader * reader = new TreeReader(tree);
	reader->Initialize(br,opt);
	data = convertTreeToData(name, reader, cuts, weight);
}

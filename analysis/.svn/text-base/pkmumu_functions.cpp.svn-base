#include "pkmumu_functions.hpp"
using namespace std;
using namespace RooFit;
//////////////////////////////////////////
//			Pete's Functions			//
//////////////////////////////////////////

void infoprint(string message, bool sep, string type)
{
	if (sep) cout << endl<<"==============================="<<endl;
	cout << type<<": "<<message<<endl;
	if (sep) cout <<"===============================\n";
}

TCut Str_conv(string input)
{
	TString tempStr(input);
	TCut cut(tempStr);
	return cut;
}

	

TCut Anglecut(string particle)
{
	string thetaMax = "0.4";
	string thetaMin = "0.01";
	string cutStr = (particle+"_theta >"+thetaMin+"&&"+particle+"_theta <"+thetaMax).c_str();
	TCut cut = Str_conv(cutStr);
	return cut;
}


TCut Triggers(int level,string mother)
{

	vector<string> l0;
	vector<string> h1;
	vector<string> h2;
	vector<vector<string> > triggerlines;
	string line, fullline;
	//define trigger scheme here:
	/////////////////////////////
	l0.push_back((mother+"_L0MuonDecision_TOS"));
	l0.push_back((mother+"_L0DiMuonDecision_TOS").c_str());
	
	h1.push_back((mother+"_Hlt1TrackAllL0Decision_TOS").c_str());
	h1.push_back((mother+"_Hlt1TrackMuonDecision_TOS").c_str());
	h1.push_back((mother+"_Hlt1DiMuonHighMassDecision_TOS").c_str());

	h2.push_back((mother+"_Hlt2TopoMu2BodyBBDTDecision_TOS").c_str());
	h2.push_back((mother+"_Hlt2TopoMu3BodyBBDTDecision_TOS").c_str());
	h2.push_back((mother+"_Hlt2TopoMu4BodyBBDTDecision_TOS").c_str());
	h2.push_back((mother+"_Hlt2Topo2BodyBBDTDecision_TOS").c_str());
	h2.push_back((mother+"_Hlt2Topo3BodyBBDTDecision_TOS").c_str());
	h2.push_back((mother+"_Hlt2Topo4BodyBBDTDecision_TOS").c_str());
	h2.push_back((mother+"_Hlt2SingleMuonDecision_TOS").c_str());
	h2.push_back((mother+"_Hlt2DiMuonDetachedDecision_TOS").c_str());
	
	triggerlines.push_back(l0);
	triggerlines.push_back(h1);
	triggerlines.push_back(h2);

	for (int i=0;i<level;i++)
	{

		vector<string> vec = triggerlines[i];

		line = "";
		for(unsigned j=0;j<vec.size();j++)
		{
			if (j < vec.size()-1)
			{
				line  = line + vec[j]+" || ";
			}
			else if (j== vec.size()-1)
			{
				line = line + vec[j];
			}
		}
		if (i==0) fullline = fullline+"("+line+")";
		else if (i==1||i==2) fullline = fullline+"&&("+line+")";
	}
	TCut trigcut = Str_conv(fullline.c_str());
	return trigcut;
}


bool kinvar_nanchecker(vector<double> v_anglemass, int entry, TLorentzVector proton,TLorentzVector kaon,TLorentzVector mupp,TLorentzVector mump,TLorentzVector JPsip)
{
	bool nancheck = false;	
	for (unsigned j =0;j<v_anglemass.size();j++)
	{
		if (std::isnan(v_anglemass[j]))
		{
			nancheck = true;
			cout <<"NAN detected in column "<<j<<". Skipping event "<<entry<<endl;
			cout <<"=================="<<endl;
			cout<<proton.Mag()<<endl;
			cout<<kaon.Mag()<<endl;
			cout<<mupp.Mag()<<endl;
			cout<<mump.Mag()<<endl;
			cout<<JPsip.Mag()<<endl;
			cout <<"=================="<<endl;
		}
	}
	return nancheck;
}



TCut pKmmSelectionCut(int diMuOpts,int PIDOpts,string bStr)
{


	TCut diMuCut;
	TCut pidCut;
	TCut preselCut;

	//preselection
	string psel_lbvtx = "5";
	string psel_p_pt = "500";

	//JpsipK selection
	double jpsiMax = 11; 
	double jpsiMin = 8; 
	
	//psi2S selection
	double psi2SMax = 15; 
	double psi2SMin = 12.5;
	string charmStr;

	if(bStr == "B") charmStr = "Psi_M";	
	else charmStr = "J_psi_1S_M";

	string gt = ">";
	string lt = "<";
	string am = "&&";
	string pp = "||";

	//converting q2 into dimuon mass
	std::vector<double> mucut_d;
	std::vector<string> mucuts(4);
	
	mucut_d.push_back(TMath::Sqrt(jpsiMin)*1000);
	mucut_d.push_back(TMath::Sqrt(jpsiMax)*1000);
	mucut_d.push_back(TMath::Sqrt(psi2SMin)*1000);
	mucut_d.push_back(TMath::Sqrt(psi2SMax)*1000);
	
	for (int i = 0;i<4;i++)
	{
		stringstream cutstream;
		cutstream << mucut_d[i];
		string cut = cutstream.str();
		mucuts[i] = cut;
	}

	string lbvtx = (bStr+"_ENDVERTEX_CHI2"+"/"+bStr+"_ENDVERTEX_NDOF").c_str();
	string p_pt = "pplus_PT";
	
	TString preselCuttmp((lbvtx+lt+psel_lbvtx+am+p_pt+gt+psel_p_pt).c_str());	
	preselCut = preselCuttmp;

	if (diMuOpts == 0)
	{
		diMuCut = "";
	}
	else if (diMuOpts == 1)
	{
		diMuCut = (charmStr+gt+mucuts[0]+am+charmStr+lt+mucuts[1]).c_str();
	}
	else if (diMuOpts == 2)
	{
		TString diMutmp((charmStr+lt+mucuts[0]+pp+"("+charmStr+gt+mucuts[1]+am+charmStr+lt+mucuts[2]+")"+pp+charmStr+gt+mucuts[3]).c_str());
		TCut diMutmp2(diMutmp);
		diMuCut= diMutmp2;
	}

	if (PIDOpts == 0||PIDOpts == 1) //no 2nd choice yet
	{
		pidCut = "";
	}

	else if (PIDOpts == 2)
	{
		TString pidtmp("pplus_ProbNNp>0.2&&Kminus_ProbNNk>0.2&&pplus_ProbNNpi<0.7&&Kminus_ProbNNp<0.8&&pplus_ProbNNk<0.8");
		TCut pidtmp2(pidtmp);
			pidCut = pidtmp;
	}


	TCut select = diMuCut+pidCut+preselCut;
	return select;

}
void binPrint(vector<double> bins)
{
	cout <<"\n|";
	for (unsigned m =0;m<bins.size();m++)
	{
		cout <<"|"<<bins[m]<<"|";
	}
	cout <<"|\n";
}

//Calculate a particle's pseudorapidity. Supply nTuple TreeReader and particle prefix.
double EtaCalc (TreeReader* reader, string pStr)
{
	double momMag = TMath::Abs(reader->GetValue((pStr+"_P").c_str()));
	double PT = TMath::Abs(reader->GetValue((pStr+"_PT").c_str()));
	double eta = TMath::ACosH(momMag/PT);

	return eta;
}

// Choose between different PID and q2 selection for Lb->pKmumu"TMath::Power(J_psi_1S_MM/1000,2)
// diMuOpts = 0: No q2 Selection applied"TMath::Power(J_psi_1S_MM/1000,2)
// diMuOpts = 1: Select J/Psi only
// diMuOpts = 2: Veto J/Psi and Psi(2S)
//
// PIDOpts = 0: No PID selection applied
// PIDOpts = 1: Use old DLL cuts designed for Lb->J/PsipK analysis
// PIDOpts = 2: Use new ProbNN variable cuts for Lb->pKmumu analysis

bool pKmmPreSelection(TreeReader* reader, int diMuOpts, int PIDOpts,string bStr)
{
	bool diMuCheck = false;
	bool PIDCheck = false;
	bool preSelection = false;
	bool selection = false;

	//preselection
	double psel_lbvtx = 5.;
	double psel_p_pt = 500.;

	//JpsipK selection
	double jpsiMax = 11; 
	double jpsiMin = 8; 
	
	//psi2S selection
	double psi2SMax = 15; 
	double psi2SMin = 12.5;
	string charmStr;
	if(bStr == "B") charmStr = "Psi_";	
	else charmStr = "J_psi_1S_";	
	double diMuMass = reader->GetValue((charmStr+"M").c_str());
	double q2 = diMuMass*diMuMass/1E6;

	double lbvtx = reader->GetValue((bStr+"_ENDVERTEX_CHI2").c_str())/reader->GetValue((bStr+"_ENDVERTEX_NDOF").c_str());
	double p_pt = reader->GetValue("pplus_PT");
	
	if(lbvtx < psel_lbvtx && p_pt > psel_p_pt) preSelection = true;
	else preSelection = false;
	
	if (diMuOpts == 0) diMuCheck = true;

	else if (diMuOpts == 1)
	{
		if(q2 > jpsiMin && q2 < jpsiMax)
		{
			diMuCheck = true;
		}
		else diMuCheck = false;
	}
	else if (diMuOpts == 2)
	{

		if(q2 < jpsiMin || (q2 > jpsiMax && q2 < psi2SMin) || q2 > psi2SMax)
		{
			diMuCheck = true;
		}
		else diMuCheck = false;
	}
	else cout <<"Di-muon selection option not recognised"<<endl;
	
	if (PIDOpts == 0) PIDCheck = true;

	else if (PIDOpts == 1)
	{
		if (reader->GetValue("pplus_PIDp") > 0 && reader->GetValue("Kminus_PIDK") > 0 &&(reader->GetValue("pplus_PIDp") - reader->GetValue("pplus_PIDK"))> 8 &&(reader->GetValue("Kminus_PIDp") - reader->GetValue("Kminus_PIDK")) < 8)
		{
			PIDCheck = true;
		}
		else PIDCheck = false;
	}

	else if (PIDOpts == 2)
	{
		if (reader->GetValue("pplus_ProbNNp") > 0.2 && reader->GetValue("Kminus_ProbNNk") > 0.2 && reader->GetValue("pplus_ProbNNpi")< 0.7 && reader->GetValue("Kminus_ProbNNp") < 0.8 && reader->GetValue("pplus_ProbNNk") < 0.8)
		{
			PIDCheck = true;
		}
		else PIDCheck = false;
	}

	else cout <<"PID selection option not recognised"<<endl;

	if (PIDCheck && diMuCheck && preSelection) selection = true;
	else selection = false;
	
	return selection;
}

//Check for L0 trigger pass for pKmumu (also applicable to Lmumu and jpsipK). Mother branch name currently "B"
bool L0triggers(TreeReader* reader, string mother)
{
	if (reader->GetValue((mother+"_L0MuonDecision_TOS").c_str()) || reader->GetValue((mother+"_L0DiMuonDecision_TOS").c_str()))
	{
		return true;
	}
	else return false;
}

//Check for Hlt1 trigger pass for pKmumu (also applicable to Lmumu and jpsipK). Mother branch name currently "B"
bool Hlt1triggers(TreeReader* reader, string mother)
{
	if (
			reader->GetValue((mother+"_Hlt1TrackAllL0Decision_TOS").c_str())		|| 
			reader->GetValue((mother+"_Hlt1TrackMuonDecision_TOS").c_str())			||
			reader->GetValue((mother+"_Hlt1DiMuonHighMassDecision_TOS").c_str())
//			||	reader->GetValue((mother+"_Hlt1MuTrackDecision_TOS").c_str())
			)
	{
		return true;
	}
	else return false;
}

//Check for Hlt2 trigger pass for pKmumu (also applicable to Lmumu and jpsipK). Mother branch name currently "B"
bool Hlt2triggers(TreeReader* reader, string mother)
{
	if (
			reader->GetValue((mother+"_Hlt2TopoMu2BodyBBDTDecision_TOS").c_str())		|| 
			reader->GetValue((mother+"_Hlt2TopoMu3BodyBBDTDecision_TOS").c_str())		||
			reader->GetValue((mother+"_Hlt2TopoMu4BodyBBDTDecision_TOS").c_str())		||
			reader->GetValue((mother+"_Hlt2Topo2BodyBBDTDecision_TOS").c_str())			||
			reader->GetValue((mother+"_Hlt2Topo3BodyBBDTDecision_TOS").c_str())			||
			reader->GetValue((mother+"_Hlt2Topo4BodyBBDTDecision_TOS").c_str())			||
			reader->GetValue((mother+"_Hlt2SingleMuonDecision_TOS").c_str())			||
			reader->GetValue((mother+"_Hlt2DiMuonDetachedDecision_TOS").c_str())
			)
	{
		return true;
	}
	else return false;
}

//Calculate the invariant mass of a four body particle system. Used primarily with particleBuilder() for manual reconstruction
double fourBodyMass(TLorentzVector part1,TLorentzVector part2,TLorentzVector part3,TLorentzVector part4)
{
	TLorentzVector mother_4mom = part1+part2+part3+part4;
	double inv_mass = mother_4mom.M();
	return inv_mass;
}
//Boolean check for specified q2 bin. Currently pKmumu binning
bool qbinCheck(double q2,int qbin)
{
	double q2min = 0;
	double q2max = 0;
	//0.1,2,4,6,8,11,12.5,15,17.5
	
	if(qbin == 1){q2min =0.1;q2max = 2;}
	else if	(qbin == 2){q2min =2;q2max = 4;}
	else if	(qbin == 3){q2min =4;q2max = 6;} 
	else if	(qbin == 4){q2min =6;q2max = 8;}
	else if	(qbin == 5){q2min =8;q2max = 11;} 
	else if	(qbin == 6){q2min =11;q2max = 12.5;} 
	else if	(qbin == 7){q2min =12.5;q2max = 15;} 
	else if	(qbin == 8){q2min =15;q2max = 17.5;}
	
	if (q2>=q2min&&q2<q2max) return true;
	else return false;
}

bool rangeCheck(double val,double min,double max)
{
	//0.1,2,4,6,8,11,12.5,15,17.5
	
	if (val>=min&&val<max) return true; //<= < construction designed for sequential binning checks
	else return false;
}

int binCheck(vector<double>bins,double val)
{
	for (unsigned i = 0; i < bins.size()-1; i++)
	{
		if (val>=bins[i]&&val<bins[i+1])
		{
			return i;
		}
		else continue;
	}
	return -1;	
}

//Checks that particle is within LHCb geometric acceptance using the angle to beam, Theta.
bool accCheck(double theta)
{
	double thetaMax = 0.4;
	double thetaMin = 0.01;
	if (theta <=thetaMax && theta >= thetaMin)
		return true;
	else return false;
}
bool motherIDmuCheck (TreeReader* reader,string mother)
{
	int mID = 0;
	if (mother == "Lb") mID = 5122;
	else if (mother == "B0") mID = 511;
	else if (mother == "Bs") mID = 531;

	if (TMath::Abs(reader->GetValue("muplus_MC_MOTHER_ID")) == mID) return true;
	else return false;
}

//Used to create samples containing only true Lb->pKmumu events. Useful for PID efficiency evaliation		
bool pKPIDTrue(TreeReader* reader, string bStr)
{
	int protonID = TMath::Abs(reader->GetValue("pplus_TRUEID"));
	int kaonID = TMath::Abs(reader->GetValue("Kminus_TRUEID"));
	int muonmID = TMath::Abs(reader->GetValue("muminus_TRUEID"));
	int muonpID = TMath::Abs(reader->GetValue("muplus_TRUEID"));
	int motherID = TMath::Abs(reader->GetValue((bStr+"_TRUEID").c_str()));
	
	bool cascade = false;
	bool anticascade = false;
	
	if (reader->GetValue("pplus_TRUEID") == 2212 && reader->GetValue((bStr+"_TRUEID").c_str()) == 5122)
	{
		cascade = true;
	}
	else if (reader->GetValue("pplus_TRUEID") == -2212 && reader->GetValue((bStr+"_TRUEID").c_str()) == -5122)
	{
		anticascade = true;
	}
	else
	{
		cascade = false;
		anticascade = false;
	}

	if (
			protonID == 2212
			&& kaonID == 321
			&& muonmID == 13
			&& muonpID == 13
			&& motherID == 5122
			&& (cascade || anticascade)
			
			)
	{
//		cout<< "True"<<endl;
		return true;
	}
	else 
	{
//		cout<< "False"<<endl;
		return false;
	}
}

bool bkgPIDTrue(TreeReader* reader,string bStr,bool recotrue)
{
	string had1 = "";
	string had2 = "";
	string mother = "";
	int had1_pdg = 0;
	int had2_pdg = 0;
	int mother_pdg = 0;
	
	if (recotrue && bStr == "B0")
	{
		mother = bStr;
		had1 = "piminus";
		had2 = "Kplus";
		
	}
	
	else if (recotrue && bStr == "Bs")
	{
		mother = bStr;
		had1 = "Kminus";
		had2 = "Kplus";
	}
	else
	{
		mother = "Lb";
		had1 = "Kminus";
		had2 = "pplus";
	}
	if (bStr == "B0")
	{
		had1_pdg = 211;
		had2_pdg = 321;
		mother_pdg = 511;
		
	}
	else if (bStr == "Bs")
	{
		had1_pdg = 321;
		had2_pdg = 321;
		mother_pdg = 531;
	}	
	int had1ID = TMath::Abs(reader->GetValue((had1+"_TRUEID").c_str()));
	int had2ID = TMath::Abs(reader->GetValue((had2+"_TRUEID").c_str()));
	int muonmID = TMath::Abs(reader->GetValue("muminus_TRUEID"));
	int muonpID = TMath::Abs(reader->GetValue("muplus_TRUEID"));
	int motherID = TMath::Abs(reader->GetValue((mother+"_TRUEID").c_str()));

	if (
	    had1ID == had1_pdg
	    && had2ID == had2_pdg
	    && muonmID == 13
	    && muonpID == 13
	    && motherID == mother_pdg
	    )
	{
//		cout<< "True"<<endl;
		return true;
	}
	else 
	{
//		cout<< "False"<<endl;
		return false;
	}
}

//RKst true PID checker. type == 0 for Muons, 1 for electrons
bool RKstPIDTrue(TreeReader* reader, int type)
{
	int lepton1ID = 0;
	int lepton2ID = 0;
	int leptonPDG = 0;



	if (type == 0)
	{
		lepton1ID = TMath::Abs(reader->GetValue("Mu1_TRUEID"));
		lepton2ID = TMath::Abs(reader->GetValue("Mu2_TRUEID"));
		leptonPDG = 13;
	}
	else if (type == 1)
	{
		lepton1ID = TMath::Abs(reader->GetValue("E1_TRUEID"));
		lepton2ID = TMath::Abs(reader->GetValue("E2_TRUEID"));
		leptonPDG = 11;
	}

	int pionID = TMath::Abs(reader->GetValue("Pi_TRUEID"));
	int kaonID = TMath::Abs(reader->GetValue("K_TRUEID"));
	int motherID = TMath::Abs(reader->GetValue("B0_TRUEID"));


	if (
			pionID == 211
			&& kaonID == 321
			&& lepton1ID == leptonPDG
			&& lepton2ID == leptonPDG
			&& motherID == 511
			
			)
	{
//		cout<< "True"<<endl;
		return true;
	}
	else 
	{
//		cout<< "False"<<endl;
		return false;
	}
}

//Build a particle's 4 momenta from 3 momenta values and energy. Supply with the nTuple reader and particle prefix. eg Kminus, pplus etc.
TLorentzVector particleBuilder(TreeReader* reader, string part,bool truth)
{
	string _PX,_PY,_PZ,_PE;
	if (truth)
	{
	_PX = "_TRUEP_X";
	_PY = "_TRUEP_Y";
	_PZ = "_TRUEP_Z";
	_PE = "_TRUEP_E";
	}
	else
	{
	_PX = "_PX";
	_PY = "_PY";
	_PZ = "_PZ";
	_PE = "_PE";
	}

	TLorentzVector particle_P;
	particle_P.SetPxPyPzE(
			reader->GetValue((part+_PX).c_str()),
			reader->GetValue((part+_PY).c_str()),
			reader->GetValue((part+_PZ).c_str()),
			reader->GetValue((part+_PE).c_str())
			);
//	cout <<particle_P.E()<<endl;
	return particle_P;
}

//Same as particleBuilder but 3-momenta only.
TVector3 pVecBuilder(TreeReader* reader, string part, bool truth)
{
	string _PX,_PY,_PZ;
	if (truth)
	{
		_PX = "_TRUEP_X";
		_PY = "_TRUEP_Y";
		_PZ = "_TRUEP_Z";
	}
	else
	{
		_PX = "_PX";
		_PY = "_PY";
		_PZ = "_PZ";
	}

	TVector3 particle_P;
	particle_P.SetXYZ(
			reader->GetValue((part+_PX).c_str()),
			reader->GetValue((part+_PY).c_str()),
			reader->GetValue((part+_PZ).c_str())
			);
	return particle_P;
}

//Like particleBuilder() but for misreconstructing particles. For this use particle id's such as kaon, proton, muon pion.
//recoID is what the particle has been reconstructed as. Hence this will choose the branch name in the TreeReader fr momentum information.
//swapID is the particle to be swapped to. PDG mass value will be inserted into 4-vector
TLorentzVector partSwap(TreeReader* reader, string recoID, string swapID)
{
	string Kstr = "Kminus";
	string pstr = "pplus";
	string mmstr = "muminus";
	string mpstr = "muplus";
	string _PX = "_PX";
	string _PY = "_PY";
	string _PZ = "_PZ";
	string _P = "_P";
	string part = "";
	double mass = -1;
	TLorentzVector particle_P;
	
	if (recoID == "kaon")
		part = Kstr;
	else if (recoID == "proton")
		part = pstr;
	
	else if (recoID == "muon")
	{
		if(swapID == "proton")
			part = mpstr;
		else if (swapID == "kaon")
			part = mmstr;
		else cout <<"something went wrong. - Love from TLorentzVector function partSwap"<<endl;
	}


	else cout <<"Reconstructed particle identifier, '"<<recoID<<"' not recognised"<<endl;

	if (swapID == "proton")
		mass = 938;
	else if (swapID == "kaon")
		mass = 493.7;
	else if (swapID == "pion")
		mass =138.6;
	else if (swapID == "muon")
		mass = 105.7;
	else cout <<"Swapped particle identifier, '"<<swapID<<"' not recognised"<<endl;

	double part_mom = reader->GetValue((part+_P).c_str());

	double part_E = sqrt(((part_mom)*(part_mom))  + ((mass)*(mass)));

	particle_P.SetPxPyPzE(

			reader->GetValue((part+_PX).c_str()),
			reader->GetValue((part+_PY).c_str()),
			reader->GetValue((part+_PZ).c_str()),
			part_E
			);
	return particle_P;
}

//Bin sample statistics in q^2
//bin vector: [0]:q2,[1]:cosTheta,[2]:cosThetaL,[3]:cosThetaB,[4]:phiL,[5]:phiB 
vector <vector<int> > q2BinStat(TreeReader* reader,vector<vector<double> > bins,int checkType, bool recoMu,string mother,string bStr,string weightType)
{
	//information printing
	//////////////////////////////////////////////////////////////////////////////////////////
	cout << "Running q2BinStat()"<<endl;
	cout << "|-------------------------------------------|"<<endl;
	cout << "|Initialising with the following parameters:|"<<endl;
	cout << "|-------------------------------------------|"<<endl;
	cout<<endl;
	cout << "|---------------------------------------------------------------------|"<<endl;
	cout << "|---------------------------------------------------------------------|"<<endl;
	if (checkType == 0)cout <<"Performing no PID selection criteria"<<endl;
	else if (checkType == 1) cout <<"Performing PID selection criteria for "<<endl;
	else if (checkType == 2) cout<<"Requiring valid decay hierarchy for ";		
	if (checkType == 1 && mother == "Lb")cout << "final state [p K mu mu]cc"<<endl;
	else if (checkType == 1 && mother == "B0")cout << "final state [K pi mu mu]cc"<<endl;
	else if (checkType == 1 && mother == "Bs")cout << "final state [K K mu mu]cc"<<endl;
	if (checkType == 2 && mother == "Lb")cout << "Lb->p K X"<<endl;
	if (checkType == 2 && mother == "B0")cout << "B0->K pi X"<<endl;
	if (checkType == 2 && mother == "Bs")cout << "Bs->K pi X"<<endl;
	cout << "Looking for mother particle branch with name prefix: "<<bStr<<endl;
	if (recoMu) cout<<"Reconstructing q^2 mass from muon vectors"<<endl;
	else cout<<"Using reconstructed muon mass in nTuples for q^2"<<endl;
	cout<<"\n q^2[GeV^2] bin boundaries:"<<endl;
	binPrint(bins[0]);
	cout<<"\n costheta bin boundaries:"<<endl;
	binPrint(bins[1]);
	cout<<"\n costhetaL bin boundaries:"<<endl;
	binPrint(bins[2]);
	cout<<"\n costhetaB bin boundaries:"<<endl;
	binPrint(bins[3]);
	cout<<"\n phiL bin boundaries:"<<endl;
	binPrint(bins[4]);
	cout<<"\n phiB bin boundaries:"<<endl;
	binPrint(bins[5]);
	cout << "|---------------------------------------------------------------------|"<<endl;
	cout << "|---------------------------------------------------------------------|"<<endl;
	//////////////////////////////////////////////////////////////////////////////////////////

	
//	double lhcbTau = 1.482;
//	double lhcbTauSigma = 0.021;
//	double genTau = 1.4247;
//	double lfweight =1;
//	double lfweightMax =1;
//	double lfweightMin =1;
//	double physW = 1;
//	double physWMin = 1;
//	double physWMax = 1;
//	double weight =1;
//	double weightMin =1;
//	double weightMax =1;
	

	int nevents = reader->GetEntries();
//	int nevents =1000;
	
	vector<int> q2array;
	vector<int> cTarray;
	vector<int> cTLarray;
	vector<int> cTBarray;
	vector<int> pLarray;
	vector<int> pBarray;
	
	for (unsigned ini = 0; ini <bins[0].size();ini++)
	{
		q2array.push_back(0);
	}
	for (unsigned ini = 0; ini <bins[1].size();ini++)
	{
		cTarray.push_back(0);
	}
	for (unsigned ini = 0; ini <bins[2].size();ini++)
	{
		cTLarray.push_back(0);
	}
	for (unsigned ini = 0; ini <bins[3].size();ini++)
	{
		cTBarray.push_back(0);
	}
	for (unsigned ini = 0; ini <bins[4].size();ini++)
	{
		pLarray.push_back(0);
	}
	for (unsigned ini = 0; ini <bins[5].size();ini++)
	{
		pBarray.push_back(0);
	}

	for (int event =0;event < nevents; event++)
	{
		
		reader->GetEntry(event);
//////////////////////////put all all code in loop below this line//////////////////////////////////
		float q2 = -9;
		float cosTheta = -9;
		float cosThetaL = -9;
		float cosThetaB = -9;
		float phiL = -9;
		float phiB = -9;
		//float dphi = -9;
		//int pcharge = -9;

		TLorentzVector Lst, kaon, proton, Lbp, JPsip, mupp, mump;
		TLorentzVector initialProton(0.,0.,4000000.,TMath::Sqrt(4000000.*4000000.+938.*938.));

		Lbp.SetPxPyPzE(reader->GetValue((bStr+"_TRUEP_X").c_str()),
			       reader->GetValue((bStr+"_TRUEP_Y").c_str()),
			       reader->GetValue((bStr+"_TRUEP_Z").c_str()),
			       reader->GetValue((bStr+"_TRUEP_E").c_str()));

		if (!recoMu)
		{
			double diMuMass = reader->GetValue("J_psi_1S_M");
			q2 = diMuMass*diMuMass/1E6;
			JPsip.SetPxPyPzE(reader->GetValue("J_psi_1S_TRUEP_X"),
					reader->GetValue("J_psi_1S_TRUEP_Y"),
					reader->GetValue("J_psi_1S_TRUEP_Z"),
					reader->GetValue("J_psi_1S_TRUEP_E"));
			//pcharge = reader->GetValue("pplus_TRUEID")/TMath::Abs(reader->GetValue("pplus_TRUEID"));
		}
		else
		{
			TLorentzVector muplus = particleBuilder(reader,"muplus");
			TLorentzVector muminus = particleBuilder(reader,"muminus");
			TLorentzVector m34 = muplus+muminus;
			q2 = m34.M2()/1E6;
			JPsip = m34;
			//if (reader->GetValue("pplus_MC_MOTHER_ID")>0) pcharge = 1;
			//else pcharge = -1;
		}

	proton.SetPxPyPzE(reader->GetValue("pplus_TRUEP_X"),
			reader->GetValue("pplus_TRUEP_Y"),
			reader->GetValue("pplus_TRUEP_Z"),
			reader->GetValue("pplus_TRUEP_E"));
	kaon.SetPxPyPzE(reader->GetValue("Kminus_TRUEP_X"),
			reader->GetValue("Kminus_TRUEP_Y"),
			reader->GetValue("Kminus_TRUEP_Z"),
			reader->GetValue("Kminus_TRUEP_E"));
	mupp.SetPxPyPzE(reader->GetValue("muplus_TRUEP_X"),
			reader->GetValue("muplus_TRUEP_Y"),
			reader->GetValue("muplus_TRUEP_Z"),
			reader->GetValue("muplus_TRUEP_E"));
	mump.SetPxPyPzE(reader->GetValue("muminus_TRUEP_X"),
			reader->GetValue("muminus_TRUEP_Y"),
			reader->GetValue("muminus_TRUEP_Z"),
			reader->GetValue("muminus_TRUEP_E"));
	Lst = proton + kaon;


			//DPHelpers::LbPsiRAngles(initialProton,Lbp,JPsip,Lst,mupp,mump,proton,kaon,pcharge,
		//			(double)cosTheta,(double)cosThetaL,(double)cosThetaB,(double)phiL,(double)phiB,(double)dphi);

		if (weightType == "lifetime"|| weightType == "all")
		{

//			double evtTau = reader->GetValue("Lb_TRUETAU");
//			lfweight = TMath::Exp(evtTau/lhcbTau)/TMath::Exp(lhcbTau/genTau);
//			lfweightMin = TMath::Exp(evtTau/(lhcbTau - lhcbTauSigma))/TMath::Exp((lhcbTau - lhcbTauSigma)/genTau);
//			lfweightMax = TMath::Exp(evtTau/(lhcbTau + lhcbTauSigma))/TMath::Exp((lhcbTau + lhcbTauSigma)/genTau);
		}
		else
		{
//			lfweight = 1;
//			lfweightMin = 1;
//			lfweightMax = 1;
		}

		//		cout << q2<<endl;
		showPercentage(event,nevents,0,2000,true,true,"Catagorizing events in q^2.");
		bool check = false;
		//add more check functions here as needed.
		if (checkType == 0) check = true;
		else if (checkType == 1&& mother == "Lb") check = pKPIDTrue(reader,bStr);
		else if (checkType == 1&& mother == "B0") check = bkgPIDTrue(reader,bStr="B0");
		else if (checkType == 2) check = motherIDmuCheck(reader,mother);	

		if (check)
		{
			for (unsigned j=0;j<bins[0].size()-1;j++)
			{
				if (rangeCheck(q2,bins[0][j],bins[0][j+1])) ++q2array[j];
			}
			for (unsigned j=0;j<bins[1].size()-1;j++)
			{
				if (rangeCheck(cosTheta,bins[1][j],bins[1][j+1])) ++cTarray[j];
			}
			for (unsigned j=0;j<bins[2].size()-1;j++)
			{
				if (rangeCheck(cosThetaL,bins[2][j],bins[2][j+1])) ++cTLarray[j];
			}
			for (unsigned j=0;j<bins[3].size()-1;j++)
			{
				if (rangeCheck(cosThetaB,bins[3][j],bins[3][j+1])) ++cTBarray[j];
			}
			for (unsigned j=0;j<bins[4].size()-1;j++)
			{
				if (rangeCheck(phiL,bins[4][j],bins[4][j+1])) ++pLarray[j];
			}
			for (unsigned j=0;j<bins[5].size()-1;j++)
			{
				if (rangeCheck(phiB,bins[5][j],bins[5][j+1])) ++pBarray[j];
			}
		}
	}

	vector<vector<int> > statArray;

	statArray.push_back(q2array);
	statArray.push_back(cTarray);
	statArray.push_back(cTLarray);
	statArray.push_back(cTBarray);
	statArray.push_back(pLarray);
	statArray.push_back(pBarray);

	return statArray;
}

//bin vector: [0]:q2, [1]:cosTheta, [2]:cosThetaL, [3]:cosThetaB, [4]:phiL, [5]:phiB 
vector < vector<vector<int> > > q2EffStats(TreeReader* reader,vector <vector<double> > bins,string mother,string bStr)
{
	//information printing
	//////////////////////////////////////////////////////////////////////////////////////////
	cout << "Running q2EffStats"<<endl;
	cout << "|-------------------------------------------|"<<endl;
	cout << "|Initialising with the following parameters:|"<<endl;
	cout << "|-------------------------------------------|"<<endl;
	cout<<endl;
	cout << "|---------------------------------------------------------------------|"<<endl;
	cout << "|---------------------------------------------------------------------|"<<endl;
	cout<< "Looking for ";	
	if (mother == "Lb")cout << "Lb->p K X"<<endl;
	else if (mother == "B0")cout << "B0->K pi X"<<endl;
	else if (mother == "Bs")cout << "Bs->K pi X"<<endl;
	cout << "Looking for mother particle branch with name prefix: "<<bStr<<endl;
	cout<<"\n q^2[GeV^2] bin boundaries:"<<endl;
	binPrint(bins[0]);
	cout<<"\n costheta bin boundaries:"<<endl;
	binPrint(bins[1]);
	cout<<"\n costhetaL bin boundaries:"<<endl;
	binPrint(bins[2]);
	cout<<"\n costhetaB bin boundaries:"<<endl;
	binPrint(bins[3]);
	cout<<"\n phiL bin boundaries:"<<endl;
	binPrint(bins[4]);
	cout<<"\n phiB bin boundaries:"<<endl;
	binPrint(bins[5]);
	cout << "|---------------------------------------------------------------------|"<<endl;
	cout << "|---------------------------------------------------------------------|"<<endl;
	//////////////////////////////////////////////////////////////////////////////////////////

	int nevents = reader->GetEntries();
//	int nevents = 1000;
	vector<vector<int> > q2array;
	vector<vector<int> > cTarray;
	vector<vector<int> > cTLarray;
	vector<vector<int> > cTBarray;
	vector<vector<int> > pLarray;
	vector<vector<int> > pBarray;

	vector<int> q2array_reco;
	vector<int> q2array_trigger;
	vector<int> q2array_PID;

	vector<int> cTarray_reco;
	vector<int> cTarray_trigger;
	vector<int> cTarray_PID;

	vector<int> cTLarray_reco;
	vector<int> cTLarray_trigger;
	vector<int> cTLarray_PID;

	vector<int> cTBarray_reco;
	vector<int> cTBarray_trigger;
	vector<int> cTBarray_PID;

	vector<int> pLarray_reco;
	vector<int> pLarray_trigger;
	vector<int> pLarray_PID;

	vector<int> pBarray_reco;
	vector<int> pBarray_trigger;
	vector<int> pBarray_PID;


	string charmStr;
	if (mother == "Lb" && bStr == "B")
	{
		charmStr = "Psi_";
	}
	else charmStr = "J_psi_1S_";

	for (unsigned ini = 0; ini <bins[0].size();ini++)
	{
		q2array_reco.push_back(0);
		q2array_trigger.push_back(0);
		q2array_PID.push_back(0);
	}
	for (unsigned ini = 0; ini <bins[1].size();ini++)
	{
		cTarray_reco.push_back(0);
		cTarray_trigger.push_back(0);
		cTarray_PID.push_back(0);
	}
	for (unsigned ini = 0; ini <bins[2].size();ini++)
	{
		cTLarray_reco.push_back(0);
		cTLarray_trigger.push_back(0);
		cTLarray_PID.push_back(0);
	}
	for (unsigned ini = 0; ini <bins[3].size();ini++)
	{
		cTBarray_reco.push_back(0);
		cTBarray_trigger.push_back(0);
		cTBarray_PID.push_back(0);
	}
	for (unsigned ini = 0; ini <bins[4].size();ini++)
	{
		pLarray_reco.push_back(0);
		pLarray_trigger.push_back(0);
		pLarray_PID.push_back(0);
	}
	for (unsigned ini = 0; ini <bins[5].size();ini++)
	{
		pBarray_reco.push_back(0);
		pBarray_trigger.push_back(0);
		pBarray_PID.push_back(0);
	}

	for (int event =0;event < nevents; event++)
	{
		reader->GetEntry(event);

		TLorentzVector Lst, kaon, proton, Lbp, JPsip, mupp, mump;
		TLorentzVector initialProton(0.,0.,4000000.,TMath::Sqrt(4000000.*4000000.+938.*938.));

		Lbp.SetPxPyPzE(reader->GetValue((bStr+"_TRUEP_X").c_str()),
				reader->GetValue((bStr+"_TRUEP_Y").c_str()),
				reader->GetValue((bStr+"_TRUEP_Z").c_str()),
				reader->GetValue((bStr+"_TRUEP_E").c_str()));
		JPsip.SetPxPyPzE(reader->GetValue("J_psi_1S_TRUEP_X"),
				reader->GetValue("J_psi_1S_TRUEP_Y"),
				reader->GetValue("J_psi_1S_TRUEP_Z"),
				reader->GetValue("J_psi_1S_TRUEP_E"));
		proton.SetPxPyPzE(reader->GetValue("pplus_TRUEP_X"),
				reader->GetValue("pplus_TRUEP_Y"),
				reader->GetValue("pplus_TRUEP_Z"),
				reader->GetValue("pplus_TRUEP_E"));
		kaon.SetPxPyPzE(reader->GetValue("Kminus_TRUEP_X"),
				reader->GetValue("Kminus_TRUEP_Y"),
				reader->GetValue("Kminus_TRUEP_Z"),
				reader->GetValue("Kminus_TRUEP_E"));
		mupp.SetPxPyPzE(reader->GetValue("muplus_TRUEP_X"),
				reader->GetValue("muplus_TRUEP_Y"),
				reader->GetValue("muplus_TRUEP_Z"),
				reader->GetValue("muplus_TRUEP_E"));
		mump.SetPxPyPzE(reader->GetValue("muminus_TRUEP_X"),
				reader->GetValue("muminus_TRUEP_Y"),
				reader->GetValue("muminus_TRUEP_Z"),
				reader->GetValue("muminus_TRUEP_E"));
		Lst = proton + kaon;
		float cosTheta = -9;
		float cosThetaL = -9;
		float cosThetaB = -9;
		float phiL = -9;
		float phiB = -9;
		//float dphi = -9;
		//int pcharge = reader->GetValue("pplus_TRUEID")/TMath::Abs(reader->GetValue("pplus_TRUEID"));

		//	DPHelpers::LbPsiRAngles(initialProton,Lbp,JPsip,Lst,mupp,mump,proton,kaon,pcharge,cosTheta,cosThetaL,cosThetaB,phiL,phiB,dphi);

		bool check_reco = false;
		bool check_trigger = false;
		bool check_PID = false;
		double q2 = -9;
		double diMuMass = reader->GetValue((charmStr+"M").c_str());
		q2 = diMuMass*diMuMass/1E6;
		
		showPercentage(event,nevents,0,2000,true,true,"Catagorizing events.");

		//Currently not general for B0 and Bs
		if(pKPIDTrue(reader,bStr))
		{
			check_reco = true;
			
			if(L0triggers(reader,"Lb") && Hlt1triggers(reader,"Lb") && Hlt2triggers(reader,"Lb"))
			{
				check_trigger = true;

				if (pKmmPreSelection(reader,0,2,"Lb"))
				{
					check_PID = true;
				}

			}
		}


		if (check_reco)
		{
			for (unsigned j=0;j<bins[0].size()-1;j++)
			{
				if (rangeCheck(q2,bins[0][j],bins[0][j+1])) 						++q2array_reco[j];
				if (rangeCheck(q2,bins[0][j],bins[0][j+1]) && check_trigger) 		++q2array_trigger[j];
				if (rangeCheck(q2,bins[0][j],bins[0][j+1]) && check_PID) 			++q2array_PID[j];
			}
			for (unsigned j=0;j<bins[1].size()-1;j++)
			{
				if (rangeCheck(cosTheta,bins[1][j],bins[1][j+1])) 					++cTarray_reco[j];
				if (rangeCheck(cosTheta,bins[1][j],bins[1][j+1]) && check_trigger) 	++cTarray_trigger[j];
				if (rangeCheck(cosTheta,bins[1][j],bins[1][j+1]) && check_PID) 		++cTarray_PID[j];
			}
			for (unsigned j=0;j<bins[2].size()-1;j++)
			{
				if (rangeCheck(cosThetaL,bins[2][j],bins[2][j+1])) 					++cTLarray_reco[j];
				if (rangeCheck(cosThetaL,bins[2][j],bins[2][j+1]) && check_trigger) ++cTLarray_trigger[j];
				if (rangeCheck(cosThetaL,bins[2][j],bins[2][j+1]) && check_PID) 	++cTLarray_PID[j];
			}
			for (unsigned j=0;j<bins[3].size()-1;j++)
			{
				if (rangeCheck(cosThetaB,bins[3][j],bins[3][j+1])) 					++cTBarray_reco[j];
				if (rangeCheck(cosThetaB,bins[3][j],bins[3][j+1]) && check_trigger) ++cTBarray_trigger[j];
				if (rangeCheck(cosThetaB,bins[3][j],bins[3][j+1]) && check_PID) 	++cTBarray_PID[j];
			}
			for (unsigned j=0;j<bins[4].size()-1;j++)
			{
				if (rangeCheck(phiL,bins[4][j],bins[4][j+1])) 						++pLarray_reco[j];
				if (rangeCheck(phiL,bins[4][j],bins[4][j+1]) && check_trigger) 		++pLarray_trigger[j];
				if (rangeCheck(phiL,bins[4][j],bins[4][j+1]) && check_PID) 			++pLarray_PID[j];
			}
			for (unsigned j=0;j<bins[5].size()-1;j++)
			{
				if (rangeCheck(phiB,bins[5][j],bins[5][j+1])) 						++pBarray_reco[j];
				if (rangeCheck(phiB,bins[5][j],bins[5][j+1]) && check_trigger) 		++pBarray_trigger[j];
				if (rangeCheck(phiB,bins[5][j],bins[5][j+1]) && check_PID) 			++pBarray_PID[j];
			}

		
		
		}
	}


	q2array.push_back(q2array_reco);
	q2array.push_back(q2array_trigger);
	q2array.push_back(q2array_PID);

	cTarray.push_back(cTarray_reco);
	cTarray.push_back(cTarray_trigger);
	cTarray.push_back(cTarray_PID);
	
	cTLarray.push_back(cTLarray_reco);
	cTLarray.push_back(cTLarray_trigger);
	cTLarray.push_back(cTLarray_PID);
	
	cTBarray.push_back(cTBarray_reco);
	cTBarray.push_back(cTBarray_trigger);
	cTBarray.push_back(cTBarray_PID);
	
	pLarray.push_back(pLarray_reco);
	pLarray.push_back(pLarray_trigger);
	pLarray.push_back(pLarray_PID);
	
	pBarray.push_back(pBarray_reco);
	pBarray.push_back(pBarray_trigger);
	pBarray.push_back(pBarray_PID);
	
	vector<vector<vector<int> > > array;

	array.push_back(q2array);
	array.push_back(cTarray);
	array.push_back(cTLarray);
	array.push_back(cTBarray);
	array.push_back(pLarray);
	array.push_back(pBarray);
	
	return array;

}
//takes in two vectors of statistics in q^2 bins and modifies efficiency string stream and a histogram with efficiency and error values. pass stringstream address and TH1F pointer as is.
void effCalc(vector<int>q2Stats1,vector<int>q2Stats2,string qbinStr[],stringstream *stream, TH1F* hist, double scaleFactor)
{
	for (unsigned i = 0;i<q2Stats1.size()-1;i++)
	{	
//		cout <<endl<< q2Stats1.size()<<"\t"<<q2Stats2.size()<<endl;
//		cout << q2Stats1[i]<<"\t"<<q2Stats2[i]<<endl;
		double q2_1N = (double)q2Stats1[i];
		double q2_2N = (double)q2Stats2[i];
		double eff = (q2_2N/q2_1N)*scaleFactor;
		double err = TMath::Sqrt(eff*(1 - eff)/q2_1N);
		(*stream) << qbinStr[i] << "\t\t"<<setprecision(6) << eff <<" +/- "<< err <<endl;
		hist->SetBinContent(i+1,eff);
		hist->SetBinError(i+1,err);
	}
}

void asciiPrint(string prog,string auth)
{
	stringstream title,author;

	if (prog == "geostrip")
	{
		title<<"        ________________  _______________  ________	"<<endl;
		title<<"       / ____/ ____/ __ \\/ ___/_  __/ __ \\/  _/ __ \\"<<endl;
		title<<"      / / __/ __/ / / / /\\__ \\ / / / /_/ // // /_/ /	"<<endl;
		title<<"     / /_/ / /___/ /_/ /___/ // / / _, _// // ____/ 	"<<endl;
		title<<"     \\____/_____/\\____//____//_/ /_/ |_/___/_/   	"<<endl;
	}
	else if(prog == "hadswap")
	{
	title<<"		_|    _|    _|_|    _|_|_|      _|_|_|  _|          _|    _|_|    _|_|_|    "<<endl;
	title<<"		_|    _|  _|    _|  _|    _|  _|        _|          _|  _|    _|  _|    _|  "<<endl;
	title<<"		_|_|_|_|  _|_|_|_|  _|    _|    _|_|    _|    _|    _|  _|_|_|_|  _|_|_|    "<<endl;
	title<<"		_|    _|  _|    _|  _|    _|        _|    _|  _|  _|    _|    _|  _|        "<<endl;
	title<<"		_|    _|  _|    _|  _|_|_|    _|_|_|        _|  _|      _|    _|  _|        "<<endl;
	}
	cout<<endl;
	if (auth=="PG")
	{
		author<<"        ___ ___ _____ ___ ___    ___ ___ ___ ___ ___ ___ _____ _  _   "<<endl;
		author<<"       | _ \\ __|_   _| __| _ \\  / __| _ \\_ _| __| __|_ _|_   _| || |  "<<endl;
		author<<"       |  _/ _|  | | | _||   / | (_ |   /| || _|| _| | |  | | | __ |  "<<endl;
		author<<"       |_| |___| |_| |___|_|_\\  \\___|_|_\\___|_| |_| |___| |_| |_||_|  "<<endl;
	
	}
	else if (auth =="LP")
	{
		author<<"       _   _   _  ___   _     ___ ___ ___  ___   _ _____ ___  ___ ___   "<<endl;
		author<<"      | | | | | |/ __| /_\\   | _ \\ __/ __|/ __| /_\\_   _/ _ \\| _ \\ __|  "<<endl;
		author<<"      | |_| |_| | (__ / _ \\  |  _/ _|\\__ \\ (__ / _ \\| || (_) |   / _|   "<<endl;
		author<<"      |____\\___/ \\___/_/ \\_\\ |_| |___|___/\\___/_/ \\_\\_| \\___/|_|_\\___|  "<<endl;
	}
	cout<<endl<<endl<<title.str()<<endl<<author.str()<<endl<<endl;
}

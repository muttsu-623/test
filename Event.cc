#include "Event.h"

//STL
#include <iostream>

#include "TGFileDialog.h"
#include "lcio.h"
#include "IMPL/LCTOOLS.h"
#include "EVENT/MCParticle.h"
#include "EVENT/ReconstructedParticle.h"
#include "EVENT/LCCollection.h"
#include "EVENT/LCEvent.h"
#include "UTIL/LCTime.h"
#include "UTIL/PIDHandler.h"
#include "UTIL/Operators.h"
#include "EVENT/SimTrackerHit.h"
#include "EVENT/SimCalorimeterHit.h"
#include "TEveElement.h"
#include "TEveManager.h"
#include "TEveTrack.h"
#include "TEveArrow.h"
#include "TEveTrackPropagator.h"
#include "TDatabasePDG.h"
#include "TEveVSDStructs.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::map;

ClassImp(Event);

extern TEveManager* gEve;

// constractor
Event::Event() : // メンバ変数の初期化．この場合はコンストラクタ内でも良いが、constの場合はここで記述する必要がある。()内は初期値。
                 _ev(0),
                 _ev_max(0),
                 _trklist(0),
                 _fdialog(0),
                 _fileinfo(0),
                 _lcReader(0),
                 _evt(0),
				 _col(0) {}

void Event::OpenFile() {
	if (_fileinfo) 
	{
		cout << "A file is already opened!!" << endl;
		return; 
	}
    _ev = 0;

    _fileinfo = new TGFileInfo();
    _fileinfo->fIniDir=(char*)".";
    _fdialog = new TGFileDialog(gClient->GetDefaultRoot(),0,kFDOpen,_fileinfo);
    cout << "Selected file name = " << _fileinfo->fFilename << endl;

    _lcReader = lcio::LCFactory::getInstance()->createLCReader(IO::LCReader::directAccess);
    _lcReader->open(_fileinfo->fFilename);

	_numberOfEvents = _lcReader->getNumberOfEvents();
	if (_numberOfEvents == 0) 
	{
		cout << "This slcio file don't include events!!" << endl;
		return;
	} 
	else 
	{
		cout << endl <<  "     [ number of runs: "    <<  _lcReader->getNumberOfRuns() 
    	<<       ", number of events: "  <<  _lcReader->getNumberOfEvents() << " ] "   
    	<< endl
    	<< endl; 
		loadEvent();
	}
}

void Event::Next() {
	if (_eventNumber >= _firstEventNum + _numberOfEvents) 
	{
		cout << "Already displayed last event!!" << endl;
		return;
	}
	
	++_eventNumber;
	loadEvent();
}

void Event::Prev() {
	if (_eventNumber == _firstEventNum)
	{
		cout << "Already displayed first event!!" << endl;
		return;
	}

	--_eventNumber;
	loadEvent();
}

void Event::loadEvent() {
	if (_runNumber == -1) {
		_evt = _lcReader->readNextEvent();
		_runNumber = _evt->getRunNumber();
		_firstEventNum = _evt->getEventNumber();
		_eventNumber = _firstEventNum;
	}
	else _evt = _lcReader->readEvent(_runNumber, _eventNumber);

	if (_evt != 0) 
	{
        printEventInfo(_evt);
        const vector< string >* strVec = _evt->getCollectionNames() ;
        vector< string >::const_iterator name ;
		int i = 0;
		for(name = strVec->begin() ; name != strVec->end() ; name++)
		{
            _col = _evt->getCollection( *name ) ;
            if(_evt->getCollection( *name )->getTypeName() == "MCParticle") { 
                // printMCParticlesInfo(_col);
                loadMCparticlesEvent();
            }
			// else 
			// if (*name == "PandoraPFOs") {
			// 	// printReconstructedParticles(_col);
			// 	loadReconstractedEvent(_col, *name);
			// }
			else if (*name == "RefinedJets") {
				loadReconstractedEvent(_col, *name);
			}
        }
    }
	gEve->Redraw3D(kFALSE, kTRUE);
}

void Event::printEventInfo(LCEvent* evt) {
    cout << endl 
        << "============================================================================" << endl ;
	cout << " Event  : " << evt->getEventNumber() << endl;
	cout << " run:  "         << evt->getRunNumber() << endl;
	cout << " timestamp "     << evt->getTimeStamp() << endl;
	cout << " weight "        << evt->getWeight() << endl;
        cout << "============================================================================" << endl ;    

        // UTIL::LCTime evtTime( evt->getTimeStamp() ) ;
        // cout << " date:      "      << evtTime.getDateString() << endl ;     
        cout << " detector : "      << evt->getDetectorName() << endl ;
}

void Event::printMCParticlesInfo(const EVENT::LCCollection* col) {
    cout << endl 
    << "--------------- " << "print out of "  << EVENT::LCIO::MCPARTICLE << " collection "
    << "--------------- " << endl ;

    int nParticles =  col->getNumberOfElements() ;

    // fill map with particle pointers and collection indices
    typedef map< EVENT::MCParticle*, int > PointerToIndexMap ;
    PointerToIndexMap p2i_map ;
    vector<EVENT::MCParticle*> moms ;

    for(int k=0; k<nParticles; k++){
        EVENT::MCParticle* part =  dynamic_cast<EVENT::MCParticle*>(col->getElementAt(k)) ;
        p2i_map[ part ] = k ; 
        moms.push_back( part ) ;
    }

    cout << endl
    <<  "[   id   ]index|       PDG|"
    <<  "    px   ,    py   ,    pz   |  px_ep  ,  py_ep  ,  pz_ep  | energy  |gen| vertex x,    y    ,    z    | endpoint x,     y   ,    z    |    mass |  charge |            spin             | colorflow | [parents] - [daughters]"    
    << endl 
    << endl ;

    // loop over collection - preserve order
    for(int index = 0 ; index < nParticles ; index++){

        EVENT::MCParticle* part = dynamic_cast<EVENT::MCParticle*>(col->getElementAt(index)) ;

        printf("[%8.8d]", part->id() );
        printf("%5d|"   , index );
        printf("%10d|" , part->getPDG() );
        printf("% 1.2e,% 1.2e,% 1.2e|" , 
        part->getMomentum()[0] ,
        part->getMomentum()[1] , 
        part->getMomentum()[2] );
        printf("% 1.2e,% 1.2e,% 1.2e|" , 
        part->getMomentumAtEndpoint()[0] ,
        part->getMomentumAtEndpoint()[1] , 
        part->getMomentumAtEndpoint()[2] );
        printf("% 1.2e|" , part->getEnergy() ) ; 
        printf(" %1d |" , part->getGeneratorStatus()  );
        printf("% 1.2e,% 1.2e,% 1.2e|" , 
        part->getVertex()[0] , 
        part->getVertex()[1] , 
        part->getVertex()[2] );
        printf("  % 1.2e,% 1.2e,% 1.2e|" , 
        part->getEndpoint()[0] , 
        part->getEndpoint()[1] ,  
        part->getEndpoint()[2] );
        printf("% 1.2e|" , part->getMass() ) ; 
        printf("% 1.2e|" , part->getCharge() ) ; 

        printf("% 1.2e,% 1.2e,% 1.2e|" , 
        part->getSpin()[0] ,
        part->getSpin()[1] , 
        part->getSpin()[2] );

        printf("  (%d, %d)   |" , 
        part->getColorFlow()[0] ,
        part->getColorFlow()[1] );

        cout << " [" ;

        for(unsigned int k=0;k<part->getParents().size();k++) {
            if(k>0) cout << "," ;
            cout << p2i_map[ part->getParents()[k] ]  ;
        }
        cout << "] - [" ;
        for(unsigned int k=0;k<part->getDaughters().size();k++) {
            if(k>0) cout << "," ;
            cout << p2i_map[ part->getDaughters()[k] ]  ;
        }
        cout << "] " << endl ;
    }
    cout << endl 
    << "-------------------------------------------------------------------------------- " 
    << endl ;
}

void Event::printReconstructedParticles( const EVENT::LCCollection* col ){

	cout << endl 
		<< "--------------- " << "print out of "  << EVENT::LCIO::RECONSTRUCTEDPARTICLE << " collection "
		<< "--------------- " << endl ;

	cout << endl 
		<< "  flag:  0x" << std::hex  << col->getFlag() << std::dec << endl ;

	int MAX_HITS = 1000;
	int nReconstructedParticles =  col->getNumberOfElements() ;
	int nPrint = nReconstructedParticles > MAX_HITS ? MAX_HITS : nReconstructedParticles ;

	cout << endl;
	EVENT::ReconstructedParticle* recP=NULL;

	for( int i=0 ; i< nPrint ; i++ ){
		cout << " [   id   ] |com|type|     momentum( px,py,pz)       | energy | mass   | charge |        position ( x,y,z)      |pidUsed|GoodnessOfPID|\n" << endl;
		recP = dynamic_cast<EVENT::ReconstructedParticle*>( col->getElementAt( i ));
		cout << UTIL::lcio_short<EVENT::ReconstructedParticle>(recP, col);
		cout << endl;
		cout << "--------------------------------------------------------------------------------------------------------------------------------------|\n" << endl;
	}
	

	// --- detailed PID info:
	cout <<  endl 
		<< "  ------------ detailed PID info: --- " <<   endl  <<   endl 
		<< "   algorithms : " 
		<<   endl ;

	UTIL::PIDHandler pidH( col )  ;

	try{  
		const EVENT::IntVec& ids =  pidH.getAlgorithmIDs() ;

		for(unsigned i=0; i<ids.size() ; ++i){

			cout << "   [id: " << ids[i] << "]   " 
				<<  pidH.getAlgorithmName( ids[i] ) 
				<< " - params: " << endl;

			const EVENT::StringVec& pNames = pidH.getParameterNames( ids[i] ) ;

			for( EVENT::StringVec::const_iterator it = pNames.begin() ; it != pNames.end() ; ++it ) cout << "   " << *it << endl;
			cout << endl << endl;
		}
		cout << endl ;
	}
	catch( UTIL::UnknownAlgorithm &e ){
	cout << "- NA - " << std::endl ;
	}

	std::cout << endl
		<< "   [particle] |  PDG   | likelihood |  type  |  algoId  | parameters : " << endl
		<< "              |        |            |        |          |              "
		<< endl ;


	for( int i=0 ; i< nPrint ; i++ ){

		EVENT::ReconstructedParticle* recP1 = 
			dynamic_cast<EVENT::ReconstructedParticle*>( col->getElementAt( i ) ) ;

		printf("   [%8.8x] " , recP1->id() ) ;


		for(unsigned int l=0;l<recP1->getParticleIDs().size();l++){

			if( l!=0)
				printf("              " ) ;

			EVENT::ParticleID* pid = recP1->getParticleIDs()[l] ;
			try{	
				printf("| %6d | %6.4e | %6.6d | %8d | [",  
						pid->getPDG() , 
						pid->getLikelihood()  ,
						pid->getType() ,
						pid->getAlgorithmType() 
						) ;

				const EVENT::StringVec& pNames = pidH.getParameterNames(  pid->getAlgorithmType() ) ;

				// if(  pNames.size() == pid->getParameters().size() ) {
				// 	for(unsigned j=0;j< pNames.size() ;++j){

				// 			cout << " " <<  pNames[j]
				// 		<< " : " <<  pid->getParameters()[j] << "," ; 
				// 	}
				// }

				cout << "]"<< endl ;

			}
			catch( UTIL::UnknownAlgorithm &e ){
		cout << "- NA - " << std::endl ;
			}

		}
		cout << endl ;

	} 

	cout << endl 
		<< "-------------------------------------------------------------------------------- " 
		<< endl ;

}

void Event::loadMCparticlesEvent() {
	// すでにeventがDisplayされていたら読み込まれる
    if(_pMCParticle)
    {
        _MCPDraw=_pMCParticle->GetRnrSelf();
        _MCPChildDraw=_pMCParticle->GetRnrChildren();

		// MCTrack elements
        for (TEveElement::List_i itt=_pMCParticle->BeginChildren(); itt!=_pMCParticle->EndChildren(); itt++) 
        {
            std::string colname = (*itt)->GetElementName();
			// MCParticleDisplayFlagマップ内にcolnameがなかったら、マップにselfのrendering値を代入する。
			// NOTE: たぶんだが、renderingするか否かをスイッチする場合があるので、必要となりそう。
            if(_MCParticleDisplayFlag.find(colname)==_MCParticleDisplayFlag.end()) _MCParticleDisplayFlag[colname]=(*itt)->GetRnrSelf();
        }

        _pMCParticle->DestroyElements();
        _pMCParticle->Destroy();
    }
    _pMCParticle = BuildMCParticles( _evt );
    _pMCParticle -> SetRnrSelfChildren(false, false);
    _pMCParticle->SetName("MCPARTICLE");
    gEve->AddElement(_pMCParticle);
}

void Event::loadReconstractedEvent(EVENT::LCCollection* col, std::string name) {
	if(_pPFOs)
    {
		_PFODraw = _pPFOs->GetRnrSelf();
		_PFOChildDraw = _pPFOs->GetRnrChildren();
		_pPFOs->DestroyElements();
        _pPFOs->Destroy();
    }
	_pPFOs = BuildPFOs( col, name );
	_pPFOs->SetRnrSelfChildren(_PFODraw, _PFOChildDraw);
	_pPFOs->SetName("ReconstractedParticles");
	gEve->AddElement(_pPFOs);
}

void Event::loadJets(EVENT::LCCollection* col, std::string name) {
	if(_jets)
    {
		_JetsDraw = _jets->GetRnrSelf();
		_JetsChildDraw = _jets->GetRnrChildren();
		_jets->DestroyElements();
        _jets->Destroy();
    }
	_jets = RecoJets( col, name );
	_jets->SetRnrSelfChildren(_JetsDraw, _JetsChildDraw);
	_jets->SetName("RefinedJets");
	gEve->AddElement(_jets);
}

using namespace lcio;
using namespace std;

float PTCut = 1.5; //GeV; Tracks with PT less than this threshold will not be displayed;

TEvePathMark * PathMarkEndTrackDecay(TEveVector &/*Vtx*/, TEveVector &End){
	TEveVector Mark = End;
	TEvePathMark* pm = new TEvePathMark(TEvePathMark::kDecay);
	pm->fV.Set(Mark);
	return pm;
}

bool IsNeutrino(int PID){
	int pid = abs(PID);
	if( pid==12 || pid==14 || pid==16 ) return true;
	return false;
}


TEveElementList* Event::BuildMCParticles( LCEvent *evt ){

	std::cout<<"  Start to build MC Tracks collection: "<<endl;

	TEveElementList  *MCTracks = new TEveElementList();

	MCTracks->SetMainColor(kRed);
	MCTracks->SetName("MC Particles");

	// 電荷があるものとないもののTrackPropagatorを作成している。
	TEveTrackPropagator* propsetNeutral = new TEveTrackPropagator();
	TEveTrackPropagator* propsetCharged = new TEveTrackPropagator();
	//  TEveTrackPropagator* propsetLowE = new TEveTrackPropagator();

	// 磁場の設定
	propsetCharged->SetMagField(-3.5);
	propsetCharged->SetName("Track propagator for charged particles");
	propsetCharged->SetMaxR(1000);
	propsetCharged->SetMaxZ(1000);
	propsetCharged->SetMaxOrbs(10.0);
	propsetCharged->SetDelta(0.01);
	//	propsetCharged->SetStepper(TEveTrackPropagator::kRungeKutta);

	propsetNeutral->SetMagField(0);
	propsetNeutral->SetName("Track propagator for neutral particles");
	propsetNeutral->SetMaxR(1000);
	propsetNeutral->SetMaxZ(1000);
	propsetNeutral->SetMaxOrbs(1.0);

	//Hand Put ini
	float MCPartUnit = 0.1;
	double MCTracksMinLength = 0.5; //cm
	double MCTracksLowEThresh = 0.1;

	// 粒子の種類
	enum ETrType { 
		kAucune=0, 
		kElectron, kPositron, kMuonP, kMuonN, kPionP, kPionN, kKaonP, kKaonN, kProton, kNeutron, kKlong, kGamma, kIon, kNeutralHad, kNeutrino, kLowE, 
		kBottomP, kBottomN, kDownP, kDownN, kUpP, kUpN, kLast
	};

	// Displayのときのtrackのcolor, width, styleの設定
	struct MCTrackDisplay {
		const char * Name;
		int Color;
		int Width;
		int Style;
	};

	MCTrackDisplay MCTParams[kLast] = {
		{"None       ",   0, 0, 0},
		{"Electron   ",   98, 2, 1},
		{"Positron   ",   53, 2, 1},
		{"Muon+      ",   2, 2, 1},
		{"Muon-      ",   4, 2, 1},
		{"Pion+      ",   96, 2, 1},
		{"Pion-      ",   66, 2, 1},
		{"Kaon+      ",   6, 2, 1},
		{"Kaon-      ",   7, 2, 1},
		{"Proton     ",   6, 2, 1},
		{"Neutron    ",   7, 1, 1},
		{"Klong     ",   3, 1, 1},
		{"Gamma     ",   5, 1, 1},
		{"Ion       ",   15, 1, 1},
		{"NeutralHad",   5, 1, 1},
		{"Neutrino  ",   7, 1, 1},
		{"LowE      ",   15, 1, 1},
		{"Bottom+      ",   kBlue, 10, 1},
		{"Bottom-      ",   kBlue, 10, 1},
		{"Down+        ",   kRed, 10, 1},
		{"Down-        ",   kRed, 10, 1},
		{"Up+          ",   kGreen, 10, 1},
		{"Up-          ",   kGreen, 10, 1}
	};


	std::string MCTrackName;
	MCTrackName="MCParticle";
	int PID, ParentNum, DaughterNum, EventNr, MotherPID, OriginPID;
	int displayedMCParticle = 0;
	int skippedMCParticle = 0;
	float energy, px, py, pz, mass, MotherEnergy, OriginEnergy, PT;
	float Vx, Vy, Vz;       //vertex position
	float Ex, Ey, Ez;       //endpoint position
	float charge;
	float KineticE, GenRadius, EndRadius;

	//Arrow to show the initial Mother Particle
	TEveTrackList* cpdLowE = new TEveTrackList("LowE");
	cpdLowE->SetMainColor(15);

	TEveTrackList* cpdMuons = new TEveTrackList("Muons");
	cpdMuons->SetMainColor(MCTParams[kMuonP].Color);

	TEveTrackList* cpdPions = new TEveTrackList("Pions");
	cpdPions->SetMainColor(MCTParams[kKaonP].Color);

	TEveTrackList* cpdElectrons = new TEveTrackList("Electrons");
	cpdElectrons->SetMainColor(MCTParams[kElectron].Color);

	TEveTrackList* cpdChargedKaons = new TEveTrackList("Charged Kaons");
	cpdChargedKaons->SetMainColor(MCTParams[kKaonP].Color);

	TEveTrackList* cpdProtons = new TEveTrackList("Protons");
	cpdProtons->SetMainColor(MCTParams[kProton].Color);

	TEveTrackList* cpdNeutrons = new TEveTrackList("Neutrons");
	cpdNeutrons->SetMainColor(MCTParams[kNeutron].Color);

	TEveTrackList* cpdKlongs = new TEveTrackList("Klong");
	cpdKlongs->SetMainColor(MCTParams[kKlong].Color);

	TEveTrackList* cpdGamma = new TEveTrackList("Gamma");
	cpdGamma->SetMainColor(MCTParams[kGamma].Color);

	TEveTrackList* cpdIon = new TEveTrackList("Ion");       //default: charged tracks besides the defined ones
	cpdIon->SetMainColor(MCTParams[kIon].Color);

	TEveTrackList* cpdNeutralHad = new TEveTrackList("NeutralHad");
	cpdNeutralHad->SetMainColor(MCTParams[kNeutralHad].Color);

	TEveTrackList* cpdNeutrinos = new TEveTrackList(MCTParams[kNeutrino].Name);
	cpdNeutrinos->SetMainColor(MCTParams[kNeutrino].Color);

	TEveTrackList* cpdBottoms = new TEveTrackList("Bottom");
	cpdBottoms->SetMainColor(MCTParams[kBottomN].Color);

	TEveTrackList* cpdDowns = new TEveTrackList("Down");
	cpdDowns->SetMainColor(MCTParams[kDownN].Color);

	TEveTrackList* cpdUps = new TEveTrackList("Up");
	cpdUps->SetMainColor(MCTParams[kUpP].Color);

	//Fix missing PIDs.
	TDatabasePDG *pdgDB = TDatabasePDG::Instance();
	Int_t ionCode = 1000010020;
	if(!pdgDB->GetParticle(ionCode)){
		pdgDB->AddParticle("Deuteron","Deuteron",2+8.071e-3,kTRUE,0,1,"Ion",ionCode);
	}

	std::vector<std::string>::const_iterator name;

	const std::vector< std::string >* strVec = evt->getCollectionNames() ;

	for( name = strVec->begin() ; name != strVec->end() ; name++){
		EVENT::LCCollection* col = evt->getCollection( *name ) ;
		EventNr = evt->getEventNumber();
		if(*name == MCTrackName)
		{
			// あるイベントにおける粒子数
			int nMCParticle =  col->getNumberOfElements();
			cout<<"  Number of MCParticle: "<<nMCParticle<<endl;
			cout<<endl;
			TEveTrackList * currCompound = 0;
			float EMCMax = -0.1;
			int countMother = 0; // used to identify Whizard event & Pythia event...
			for(int i=0; i<nMCParticle; i++)
			{
				//  MCParticleのリストの中にある粒子を取り出す
				MCParticle* partile =  dynamic_cast<MCParticle*>( col->getElementAt( i ) ) ;
				// 親がいなくて、EMCMaxよりエネルギーが大きいものの
				if(partile->getParents().size()==0 && partile->getEnergy()>EMCMax)
				{
					if(partile->getDaughters().size()==0) { countMother++; }
					EMCMax = partile->getEnergy();
				}
			}

			for(int i=0; i<nMCParticle; i++)
			{
				//  MCParticleのリストの中にある粒子を取り出す
				MCParticle* part =  dynamic_cast<MCParticle*>( col->getElementAt( i ) ) ;

				// this is initialize.
				PID=0; ParentNum=0; DaughterNum=0;
				charge=0; mass=0; energy=0;
				px=0; py=0; pz=0; Vx=0; Vy=0; Vz=0; Ex=0; Ey=0; Ez=0;
				KineticE = 0; GenRadius = 0; EndRadius = 0;
				MotherPID = 0; OriginPID = 0;
				MotherEnergy = 0; OriginEnergy = 0;
				PT = 0;

				// set values to particle properties.
				px=part->getMomentum()[0];
				py=part->getMomentum()[1];
				pz=part->getMomentum()[2];
				PID=part->getPDG();
				Vx=part->getVertex()[0];
				Vy=part->getVertex()[1];
				Vz=part->getVertex()[2];
				Ex=part->getEndpoint()[0];
				Ey=part->getEndpoint()[1];
				Ez=part->getEndpoint()[2];
				charge=part->getCharge();
				mass=part->getMass();
				energy=part->getEnergy();	
				ParentNum=part->getParents().size();
				DaughterNum=part->getDaughters().size();
				EndRadius = sqrt(Ex*Ex+Ey*Ey);
				GenRadius = sqrt(Vx*Vx+Vy*Vy);
				TEveVector Vtx(Vx, Vy, Vz);
				TEveVector End(Ex, Ey, Ez);
				PT = sqrt(px*px+py*py);
				//	PT = energy;					//tmplate usage...

				if (ParentNum > 0)
				{
					MCParticleVec mother = part->getParents();
					MotherPID = mother[0]->getPDG();
					MotherEnergy = mother[0]->getEnergy();
					if (MotherPID == 6 || MotherPID == -6) {
						cout << "The mother is t quark" << endl;
						cout << PID << endl;
						cout << endl;
					} 
					else if (MotherPID == 24 || MotherPID == -24) {
						cout << "The mother is W boson" << endl;
						cout << PID << endl;
						cout << endl;
					}
					else continue;
				}
				else continue;

				switch (PID) { 
					case 94:
					case 6:
					case 21:	
					continue;
				}

				if(PID == 22 && energy > 0.5)	//Only show the information for particle/gamma with energy > 0.5GeV
				{	
					MCParticleVec mother = part->getParents();
					if (mother.size() > 0)
					{
						MotherPID = mother[0]->getPDG();
						MotherEnergy = mother[0]->getEnergy();
					}
				}

				Vtx *= MCPartUnit;
				End *= MCPartUnit;
				KineticE = sqrt(px*px+py*py+pz*pz);

				TEveTrack* track = 0;
				ETrType TrType = kAucune;
				TEveArrow* a1 = 0;

				float Length = Vtx.Distance(End);

				// PT = energy.
				if( PT < PTCut || Length<MCTracksMinLength) skippedMCParticle++;
				else displayedMCParticle++;


				// if( Length<MCTracksMinLength ) continue; // Skip small tracks
				// if( Length<=0) continue; // Protectin against bad parameters = 0      ??
				if( PID>=1000010020 ) continue;  //Mute the heavy hygen nuclea and so on.
				// if( PT < PTCut) continue;


				if(charge!=0 && KineticE >= MCTracksLowEThresh){

					switch(PID){
						case 1:
							TrType = kDownN;
							currCompound = cpdDowns;
							break;

						case -1:
							TrType = kDownP;
							currCompound = cpdDowns;
							break;

						case 2:
							TrType = kUpP;
							currCompound = cpdUps;
							break;
						case -2:
							TrType = kUpN;
							currCompound = cpdUps;
							break;

						case 5:
							TrType = kBottomN;
							currCompound = cpdBottoms;
							break;
						case -5:
							TrType = kBottomP;
							currCompound = cpdBottoms;
							break;

						case 11:
							TrType = kElectron;
							currCompound = cpdElectrons;
							break;

						case -11:
							TrType = kPositron;
							currCompound = cpdElectrons;
							break;

						case 13:
							TrType = kMuonN;
							currCompound = cpdMuons;
							break;

						case -13:
							TrType = kMuonP;
							currCompound = cpdMuons;
							break;

						case 211:
							TrType = kPionP;
							currCompound = cpdPions;
							break;

						case -211:
							TrType = kPionN;
							currCompound = cpdPions;
							break;

						case 321:
							TrType = kKaonP;
							currCompound = cpdChargedKaons;
							break;

						case -321:
							TrType = kKaonN;
							currCompound = cpdChargedKaons;
							break;

						case 2212:
							TrType = kProton;
							currCompound = cpdProtons;
							break;

						default:
							// TrType = kIon; 
							// currCompound = cpdIon;
							break;
					}

					propsetCharged->RefPMAtt().SetMarkerColor(kYellow);
					propsetCharged->RefPMAtt().SetMarkerStyle(kCircle);
					propsetCharged->RefPMAtt().SetMarkerSize(1.0);

					TEveRecTrack* ChargedTrack = new TEveRecTrack();
					ChargedTrack->fV.Set(Vtx);
					ChargedTrack->fP.Set(px, py, pz);
					ChargedTrack->fSign = charge;

					track = new TEveTrack(ChargedTrack, propsetCharged);

					// TEvePathMark* pm3 = new TEvePathMark(TEvePathMark::kDecay);
					// pm3->fV.Set(End);
					// track->AddPathMark(*pm3);


				} 
				else  // Non charged particles
				{
					/*
					   propsetNeutral->SetMagFieldObj(new TEveMagFieldConst(0., 0., -3.5));
					   propsetNeutral->SetName("Track propagator for neutral particles");
					   propsetNeutral->SetMaxR(1000);
					   propsetNeutral->SetMaxZ(1000);
					   propsetNeutral->SetMaxOrbs(1.0);
					   */
					
					// Non charged and low energy particle. But it's not neutrino.
					// if( KineticE < MCTracksLowEThresh && ! IsNeutrino(PID) )
					// {
					// 	TrType = kLowE;
					// 	currCompound = cpdLowE;
					// }else{

					// 	switch( abs(PID) ){
					// 		case  12:; case  14:; case  16:;    //Neutrinos
					// 				 TrType = kNeutrino;
					// 				 currCompound = cpdNeutrinos;
					// 				 break;

					// 		case 22:    //Gammas
					// 				 TrType = kGamma;
					// 				 currCompound = cpdGamma;
					// 				 break;

					// 		case 2112:  // 中性子
					// 				 TrType = kNeutron;
					// 				 currCompound = cpdNeutrons;
					// 				 break;

					// 		case 130:
					// 				 TrType = kKlong;
					// 				 currCompound = cpdKlongs;
					// 				 break;

					// 		default:    //All neutral hadrons 中性ハドロン
					// 				 TrType = kNeutralHad;
					// 				 currCompound = cpdNeutralHad;
					// 				 break;
					// 	}
					// }

					if( TrType != kAucune )
					{

						TEveRecTrack* NeutralTrack = new TEveRecTrack();
						NeutralTrack->fV.Set(Vtx);
						NeutralTrack->fP.Set(px, py, pz);
						NeutralTrack->fSign = charge;

						track = new TEveTrack(NeutralTrack, propsetNeutral);

						TEvePathMark *pm = PathMarkEndTrackDecay(Vtx, End);
						track->AddPathMark(*pm);
					}

				}

				// Track settings.
				if(track){
					track->SetName(Form("Track %d", i));    // i = tracknum
					track->SetLineWidth(MCTParams[TrType].Width);
					track->SetLineColor(MCTParams[TrType].Color);
					track->SetLineStyle(MCTParams[TrType].Style);
					track->SetSmooth(kTRUE);
					if(PID == 22){ // Gamma
						track->SetTitle(Form("MCParticles: \n"
									"EventNr=%d, Track No.=%d\n""Charge=%.3f, PID=%d, Energy=%.3f\n"
									"(Vx, Vy, Vz) = (%.3f, %.3f, %.3f)\n"
									"(Ex, Ey, Ez) = (%.3f, %.3f, %.3f)\n"
									"(Px, Py, Pz) = (%.3f, %.3f, %.3f)\n"
									"MotherPID = %d, MotherEnergy = %.3f",
									EventNr, i, charge, PID, energy,
									Vx, Vy, Vz, Ex, Ey, Ez, px, py, pz, MotherPID, MotherEnergy));
					}else{
						track->SetTitle(Form("MCParticles: \n"
									"EventNr=%d, Track No.=%d\n""Charge=%.3f, PID=%d, Energy=%.3f\n"
									"(Vx, Vy, Vz) = (%.3f, %.3f, %.3f)\n"
									"(Ex, Ey, Ez) = (%.3f, %.3f, %.3f)\n"
									"(Px, Py, Pz) = (%.3f, %.3f, %.3f)\n",
									EventNr, i, charge, PID, energy,
									Vx, Vy, Vz, Ex, Ey, Ez, px, py, pz));
					}
					if ( currCompound ) {
						currCompound->AddElement(track);
					}
				}

				currCompound->MakeTracks();

			}

			//        currCompound->CloseCompound();

			// MCTracks->AddElement(cpdMother);
			// MCTracks->AddElement(cpdLowE);
			// MCTracks->AddElement(cpdNeutrinos);
			// MCTracks->AddElement(cpdGamma);
			// MCTracks->AddElement(cpdNeutralHad);
			// MCTracks->AddElement(cpdMuons);
			// MCTracks->AddElement(cpdPions);
			// MCTracks->AddElement(cpdElectrons);
			// MCTracks->AddElement(cpdChargedKaons);
			// MCTracks->AddElement(cpdProtons);
			// MCTracks->AddElement(cpdNeutrons);
			// MCTracks->AddElement(cpdKlongs);
			// MCTracks->AddElement(cpdIon);

			MCTracks->AddElement(cpdBottoms);
			MCTracks->AddElement(cpdUps);
			MCTracks->AddElement(cpdDowns);		

			cpdLowE->SetRnrSelfChildren(true, true);
			// cpdNeutralHad->SetRnrSelfChildren(false, false);
			// cpdNeutrons->SetRnrSelfChildren(false, false);

		} // if MCTrack collection

	} // loop over collections

	bool FlagDraw;
	for (TEveElement::List_i itt=MCTracks->BeginChildren(); itt!=MCTracks->EndChildren(); itt++){
		std::string colname = (*itt)->GetElementName();
		if(colname!= "LowE" && colname!="NeutralHad" && colname!="Neutrons"){
			// MCParticleDisplayFlagマップの中にcolnameがあった場合、そのDraw値をFlagDrawに代入する。
			if(_MCParticleDisplayFlag.find(colname)!=_MCParticleDisplayFlag.end()) FlagDraw=_MCParticleDisplayFlag[colname];
			else FlagDraw = true;
			(*itt)->SetRnrSelfChildren(FlagDraw, FlagDraw);
		}
	}

	std::cout<<"  With current PTCut "<<PTCut<<" GeV, "<<displayedMCParticle<<" MCparticle has been displayed, and "<<skippedMCParticle<<" particles has been skipped"<<std::endl<<std::endl<<std::endl;
	return MCTracks; // It's just TEveElement, not TEveTrackList.

}

TEveElementList* Event::BuildPFOs(EVENT::LCCollection* col, std::string name )
{

	if (name == "PandoraPFOs") {
		std::cout<<"  Reconstructed particle collection: "<<name.c_str()<<std::endl;
		std::cout<<"  Number of PFO: "<<col->getNumberOfElements()<<std::endl;
		std::cout<<std::endl;
	} else if (name == "RefinedJets") {
		std::cout<<"  Refined jets collection: "<<name.c_str()<<std::endl;
		std::cout<<"  Number of jet: "<<col->getNumberOfElements()<<std::endl;
		std::cout<<std::endl;
	}
	//	cout<<"HCALBarrelLength: "<<HCALBarrelLength<<endl;

	TEveElementList  *RecoTracks = new TEveElementList();
	RecoTracks->SetMainColor(kRed);
	RecoTracks->SetName(name.c_str());
	//    	RecoTracks->OpenCompound();

	TEveTrackPropagator* propsetNeutral = new TEveTrackPropagator();
	TEveTrackPropagator* propsetCharged = new TEveTrackPropagator();
	//	TEveTrackPropagator* propsetLowE = new TEveTrackPropagator();

	propsetCharged->SetMagFieldObj(new TEveMagFieldDuo(350, -3.5, 2.0));
	propsetCharged->SetName("Track propagator for charged particles");
	propsetCharged->SetMaxR(1000);
	propsetCharged->SetMaxZ(1000);
	propsetCharged->SetMaxOrbs(1.0);
	propsetCharged->SetDelta(0.01); // Step

	propsetCharged->RefPMAtt().SetMarkerColor(kYellow);
	propsetCharged->RefPMAtt().SetMarkerStyle(kCircle);
	propsetCharged->RefPMAtt().SetMarkerSize(1.0);

	propsetNeutral->SetMagFieldObj(new TEveMagFieldConst(0., 0., -3.5));
	propsetNeutral->SetName("Track propagator for neutral particles");
	propsetNeutral->SetMaxR(1000);
	propsetNeutral->SetMaxZ(1000);
	propsetNeutral->SetMaxOrbs(1.0);

	float MCPartUnit = 0.1;
	double MCTracksMinLength = 0.5; //cm
	double MCTracksLowEThresh = 0.1;

	enum ERecType { kRecAucune=0, kElectron, kPositron, kMuonP, kMuonN, kPionP, kPionN, kKaonP, kKaonN, kProton, kNeutron, kKlong, kGamma, kIonP, kIonN, kNeutralHad, kLowE, kRecLast};

	struct PFODisplay {
		const char * Name;
		int Color;
		int Width;
		int Style;
		float CaloHitColor;
	};

	PFODisplay PFOParams[kRecLast] = {
		{"None       ", 0, 0, 0, 0},
		{"Electron   ", 0, 10, 9, 10},
		{"Positron   ", 0, 10, 9, 90},
		{"Muon+      ", 0, 10, 9, 95},
		{"Muon-      ", 0, 10, 9, 5},
		{"Pion+      ", 0, 10, 9, 85},
		{"Pion-      ", 0, 10, 9, 15},
		{"Kaon+      ", 0, 10, 9, 75},
		{"Kaon-      ", 0, 10, 9, 25},
		{"Proton     ", 0, 10, 9, 80},
		{"Neutron    ", 0, 10, 2, 30},
		{"Klong     ", 0, 10, 2, 40},
		{"Gamma     ", 0, 10, 2, 70},
		{"Ion+      ", kRed, 10, 2, 75},
		{"Ion-		", kRed, 10, 2, 75},
		{"NeutralHad", kBlue, 10, 2, 35},
		{"LowE      ", 0, 10, 2, 33}
	};

	int PID, ParentNum, DaughterNum;
	float energy, px, py, pz, mass;
	float charge;
	float KineticE, GenRadius, EndRadius;

	//	TEveCompound* cpdLowE = new TEveCompound(PFOParams[kLowE].Name, "All low E tracks");
	TEveTrackList* cpdLowE = new TEveTrackList(PFOParams[kLowE].Name);
	cpdLowE->SetMainColor(PFOParams[kLowE].Color);

	TEveTrackList* cpdMuons = new TEveTrackList("Muons");
	cpdMuons->SetMainColor(PFOParams[kMuonP].Color);

	TEveTrackList* cpdPions = new TEveTrackList("Pions");
	cpdPions->SetMainColor(PFOParams[kKaonP].Color);

	TEveTrackList* cpdElectrons = new TEveTrackList("Electrons");
	cpdElectrons->SetMainColor(PFOParams[kElectron].Color);

	TEveTrackList* cpdChargedKaons = new TEveTrackList("Charged Kaons");
	cpdChargedKaons->SetMainColor(PFOParams[kKaonP].Color);

	TEveTrackList* cpdProtons = new TEveTrackList("Protons");
	cpdProtons->SetMainColor(PFOParams[kProton].Color);

	TEveTrackList* cpdNeutrons = new TEveTrackList("Neutrons");
	cpdNeutrons->SetMainColor(PFOParams[kNeutron].Color);

	TEveTrackList* cpdKlongs = new TEveTrackList("Klong");
	cpdKlongs->SetMainColor(PFOParams[kKlong].Color);

	TEveTrackList* cpdRecGamma = new TEveTrackList("Gamma");
	cpdRecGamma->SetMainColor(PFOParams[kGamma].Color);

	TEveTrackList* cpdIonP = new TEveTrackList("kIonP");	//default: charged tracks besides the defined ones
	cpdIonP->SetMainColor(PFOParams[kIonP].Color);

	TEveTrackList* cpdIonN = new TEveTrackList("kIonN");    
	cpdIonN->SetMainColor(PFOParams[kIonN].Color);

	TEveTrackList* cpdNeutralHad = new TEveTrackList("NeutralHad");
	cpdNeutralHad->SetMainColor(PFOParams[kNeutralHad].Color);

	TEveVector End(0.0, 0.0, 0.0);
	TEveVector Vtx(0.0, 0.0, 0.0);

	try{

		TEveTrackList * currCompound = 0;

		// if (name == "PandoraPFOs") {

			int nReconstructedParticle =  col->getNumberOfElements();

			for(int i=0; i<nReconstructedParticle; i++)
			{
				ReconstructedParticle* part =  dynamic_cast<ReconstructedParticle*>( col->getElementAt( i ) ) ;

				PID=0; ParentNum=0; DaughterNum=0;
				charge=0; mass=0; energy=0;
				px=0; py=0; pz=0; 
				KineticE = 0; GenRadius = 0; EndRadius = 0;

				px=part->getMomentum()[0];
				py=part->getMomentum()[1];
				pz=part->getMomentum()[2];
				KineticE = sqrt(px*px+py*py+pz*pz);

				if(part->getParticleIDs().size()>0)
				{
					PID = part->getParticleIDs()[0]->getPDG(); //First one of the PID lists
				}
				else
				{
					PID = -99; 
					PID = part->getType();
					//cout<<"PID not identified. Please check your data file."<<endl;	
				}

				charge=part->getCharge();
				mass=part->getMass();
				energy=part->getEnergy();
				Vtx = part->getReferencePoint();

				if(part->getClusters().size()>0)
				{
					End = part->getClusters()[0]->getPosition();	//Test 
					/*
					CalorimeterHitVec ClusterHits = part->getClusters()[0]->getCalorimeterHits();
					if(ClusterHits.size() > 0) End = ClusterHits[0]->getPosition();
					*/
				}
				else		// reserved for where cluster is dropped
				{
					End = part->getMomentum() ;
					End *= 3000.0/KineticE ;
				}

				int PFOshowhit = 1;

				Vtx *= MCPartUnit;
				End *= MCPartUnit;

				TEveTrack* track = 0;

				ERecType TrType = kRecAucune;
				
				if(charge!=0 && KineticE >= MCTracksLowEThresh){

					switch(PID){
						case 11:
							TrType = kElectron;
							currCompound = cpdElectrons;
							break;

						case -11:
							TrType = kPositron;
							currCompound = cpdElectrons;
							break;

						case 13:
							TrType = kMuonN;
							currCompound = cpdMuons;
							break;	

						case -13:
							TrType = kMuonP;
							currCompound = cpdMuons;
							break;

						case 211:
							TrType = kPionP;
							currCompound = cpdPions;
							break;	

						case -211:
							TrType = kPionN;
							currCompound = cpdPions;
							break;

						case 321:
							TrType = kKaonP;
							currCompound = cpdChargedKaons;
							break;

						case -321:
							TrType = kKaonN;
							currCompound = cpdChargedKaons;
							break;

						case 2212:
							TrType = kProton;
							currCompound = cpdProtons;
							break; 

						default:
							TrType = kIonP;
							if(charge > 0 ) currCompound = cpdIonP;
							else if(charge < 0) currCompound = cpdIonN;
							break;

					}

					TEveRecTrack* ChargedTrack = new TEveRecTrack();
					ChargedTrack->fV.Set(Vtx);
					ChargedTrack->fP.Set(px, py, pz);
					ChargedTrack->fSign = charge;

					track = new TEveTrack(ChargedTrack, propsetCharged);

					if(currCompound != cpdMuons)	//Any track besides Muon track will be end at the first calorimeter hit it corresponding to 
					{
						TEvePathMark* pm = new TEvePathMark(TEvePathMark::kDecay);
						pm->fV.Set(End);
						track->AddPathMark( *pm );
					}
				} 
				else 
				{

					if( KineticE < MCTracksLowEThresh )
					{
						TrType = kLowE;
						currCompound = cpdLowE;
					}
					else
					{

						switch( abs(PID) )
						{
							case  12:; case  14:; case  16:;    //Neutrinos	actually never be reconstructed in ILD I guess
									TrType = kRecAucune;
									currCompound = cpdNeutralHad;
									break;

							case 22:    
									TrType = kGamma;
									currCompound = cpdRecGamma;
									break;

							case 2112:
									TrType = kNeutron;
									currCompound = cpdNeutrons;		
									break;

							case 130:
									TrType = kKlong;
									currCompound = cpdKlongs;
									break;

							default:    
									TrType = kNeutralHad;	
									currCompound = cpdNeutralHad;
									break;
						}
					}
					if( TrType != kRecAucune )
					{

						TEveRecTrack* NeutralTrack = new TEveRecTrack();
						NeutralTrack->fV.Set(Vtx);
						NeutralTrack->fP.Set(px, py, pz);
						NeutralTrack->fSign = charge;

						track = new TEveTrack(NeutralTrack, propsetNeutral);

						// TEvePathMark *pm = PathMarkEndTrackDecay(Vtx, End);
						// track->AddPathMark(*pm);
					}

				}

				if(track && currCompound)
				{
					track->SetName(Form("Track %d", i));    // i = tracknum
					track->SetLineWidth(PFOParams[TrType].Width);
					track->SetLineColor(PFOParams[TrType].Color);
					track->SetLineStyle(PFOParams[TrType].Style);
					track->SetSmooth(kTRUE);
					track->SetTitle(Form("Reconstructed PFOs: %s\n"
								"Track No.=%d\n""Charge=%f, PID=%d\n"
								"Energy=%f\n"
								"Vtx position= (%.3f, %.3f, %.3f)\n"
								"Cluster pos = (%.3f, %.3f, %.3f)\n"
								"3-momentum = (%.3f, %.3f, %.3f)",
								name.c_str(), i, charge, PID, energy,
								10*Vtx[0], 10*Vtx[1], 10*Vtx[2], 10*End[0], 10*End[1], 10*End[2], px, py, pz));

					currCompound->AddElement(track);
				}

				currCompound->IncDenyDestroy();
				currCompound->MakeTracks();

			}

		// } else if (name == "RefinedJets") {

		// 	int NJets = col->getNumberOfElements();
		// 	int colorindex = 0;

		// 	for (int i=0; i < NJets; i++) { 
			
		// 		ReconstructedParticle * currJet = dynamic_cast<ReconstructedParticle*>( col->getElementAt(i) );
		// 		int NPart = currJet->getParticles().size();

		// 		colorindex = i + 2; 
		// 		if(colorindex == 5) colorindex = 94;
		

		// 		for(int j=0; j < NPart; j++) 
		// 		{
		// 			ReconstructedParticle * part = currJet->getParticles()[j];

		// 			PID=0; ParentNum=0; DaughterNum=0;
		// 			charge=0; mass=0; energy=0;
		// 			px=0; py=0; pz=0; 
		// 			KineticE = 0; GenRadius = 0; EndRadius = 0;

		// 			px=part->getMomentum()[0];
		// 			py=part->getMomentum()[1];
		// 			pz=part->getMomentum()[2];
		// 			KineticE = sqrt(px*px+py*py+pz*pz);

		// 			if(part->getParticleIDs().size()>0)
		// 			{
		// 				PID = part->getParticleIDs()[0]->getPDG(); //First one of the PID lists
		// 			}
		// 			else
		// 			{
		// 				PID = -99; 
		// 				PID = part->getType();
		// 				//cout<<"PID not identified. Please check your data file."<<endl;	
		// 			}

		// 			charge=part->getCharge();
		// 			mass=part->getMass();
		// 			energy=part->getEnergy();
		// 			Vtx = part->getReferencePoint();

		// 			if(part->getClusters().size()>0)
		// 			{
		// 				End = part->getClusters()[0]->getPosition();	//Test 
		// 				/*
		// 				CalorimeterHitVec ClusterHits = part->getClusters()[0]->getCalorimeterHits();
		// 				if(ClusterHits.size() > 0) End = ClusterHits[0]->getPosition();
		// 				*/
		// 			}
		// 			else		// reserved for where cluster is dropped
		// 			{
		// 				End = part->getMomentum() ;
		// 				End *= 3000.0/KineticE ;
		// 			}

		// 			int PFOshowhit = 1;

		// 			Vtx *= MCPartUnit;
		// 			End *= MCPartUnit;

		// 			TEveTrack* track = 0;

		// 			ERecType TrType = kRecAucune;
					
		// 			if(charge!=0 && KineticE >= MCTracksLowEThresh){

		// 				switch(PID){
		// 					case 11:
		// 						TrType = kElectron;
		// 						currCompound = cpdElectrons;
		// 						break;

		// 					case -11:
		// 						TrType = kPositron;
		// 						currCompound = cpdElectrons;
		// 						break;

		// 					case 13:
		// 						TrType = kMuonN;
		// 						currCompound = cpdMuons;
		// 						break;	

		// 					case -13:
		// 						TrType = kMuonP;
		// 						currCompound = cpdMuons;
		// 						break;

		// 					case 211:
		// 						TrType = kPionP;
		// 						currCompound = cpdPions;
		// 						break;	

		// 					case -211:
		// 						TrType = kPionN;
		// 						currCompound = cpdPions;
		// 						break;

		// 					case 321:
		// 						TrType = kKaonP;
		// 						currCompound = cpdChargedKaons;
		// 						break;

		// 					case -321:
		// 						TrType = kKaonN;
		// 						currCompound = cpdChargedKaons;
		// 						break;

		// 					case 2212:
		// 						TrType = kProton;
		// 						currCompound = cpdProtons;
		// 						break; 

		// 					default:
		// 						TrType = kIonP;
		// 						if(charge > 0 ) currCompound = cpdIonP;
		// 						else if(charge < 0) currCompound = cpdIonN;
		// 						break;

		// 				}

		// 				TEveRecTrack* ChargedTrack = new TEveRecTrack();
		// 				ChargedTrack->fV.Set(Vtx);
		// 				ChargedTrack->fP.Set(px, py, pz);
		// 				ChargedTrack->fSign = charge;

		// 				track = new TEveTrack(ChargedTrack, propsetCharged);

		// 				if(currCompound != cpdMuons)	//Any track besides Muon track will be end at the first calorimeter hit it corresponding to 
		// 				{
		// 					TEvePathMark* pm = new TEvePathMark(TEvePathMark::kDecay);
		// 					pm->fV.Set(End);
		// 					track->AddPathMark( *pm );
		// 				}

		// 			} 
		// 			else 
		// 			{

		// 				if( KineticE < MCTracksLowEThresh )
		// 				{
		// 					TrType = kLowE;
		// 					currCompound = cpdLowE;
		// 				}
		// 				else
		// 				{

		// 					switch( abs(PID) )
		// 					{
		// 						case  12:; case  14:; case  16:;    //Neutrinos	actually never be reconstructed in ILD I guess
		// 								TrType = kRecAucune;
		// 								currCompound = cpdNeutralHad;
		// 								break;

		// 						case 22:    
		// 								TrType = kGamma;
		// 								currCompound = cpdRecGamma;
		// 								break;

		// 						case 2112:
		// 								TrType = kNeutron;
		// 								currCompound = cpdNeutrons;		
		// 								break;

		// 						case 130:
		// 								TrType = kKlong;
		// 								currCompound = cpdKlongs;
		// 								break;

		// 						default:    
		// 								TrType = kNeutralHad;	
		// 								currCompound = cpdNeutralHad;
		// 								break;
		// 					}
		// 				}
		// 				if( TrType != kRecAucune )
		// 				{

		// 					TEveRecTrack* NeutralTrack = new TEveRecTrack();
		// 					NeutralTrack->fV.Set(Vtx);
		// 					NeutralTrack->fP.Set(px, py, pz);
		// 					NeutralTrack->fSign = charge;

		// 					track = new TEveTrack(NeutralTrack, propsetNeutral);

		// 					TEvePathMark *pm = PathMarkEndTrackDecay(Vtx, End);
		// 					track->AddPathMark(*pm);
		// 				}

			// 		}

			// 		if(track && currCompound)
			// 		{
			// 			track->SetName(Form("Track %d", i));    // i = tracknum
			// 			track->SetLineWidth(PFOParams[TrType].Width);
			// 			track->SetLineColor(colorindex);
			// 			track->SetLineStyle(PFOParams[TrType].Style);
			// 			track->SetSmooth(kTRUE);
			// 			track->SetTitle(Form("Reconstructed PFOs: %s\n"
			// 						"Track No.=%d\n""Charge=%f, PID=%d\n"
			// 						"Energy=%f\n"
			// 						"Vtx position= (%.3f, %.3f, %.3f)\n"
			// 						"Cluster pos = (%.3f, %.3f, %.3f)\n"
			// 						"3-momentum = (%.3f, %.3f, %.3f)",
			// 						name.c_str(), i, charge, PID, energy,
			// 						10*Vtx[0], 10*Vtx[1], 10*Vtx[2], 10*End[0], 10*End[1], 10*End[2], px, py, pz));

			// 			currCompound->AddElement(track);
			// 		}
			// 		currCompound->IncDenyDestroy();
			// 		currCompound->MakeTracks();

			// 	}
		// 	}
		// }

		RecoTracks->AddElement(cpdLowE);
		RecoTracks->AddElement(cpdRecGamma);
		RecoTracks->AddElement(cpdNeutralHad);
		RecoTracks->AddElement(cpdMuons);
		RecoTracks->AddElement(cpdPions);
		RecoTracks->AddElement(cpdElectrons);
		RecoTracks->AddElement(cpdChargedKaons);
		RecoTracks->AddElement(cpdProtons);
		RecoTracks->AddElement(cpdKlongs);
		RecoTracks->AddElement(cpdNeutrons);
		RecoTracks->AddElement(cpdIonP);
		RecoTracks->AddElement(cpdIonN);

		return RecoTracks;
	}
	catch(lcio::DataNotAvailableException zero) { }
}

#include "TEveCompound.h"
#include "TEveBox.h"

TEveElementList* Event::RecoJets(EVENT::LCCollection* col, std::string name ) {
	TEveCompound *Jets = new TEveCompound();
	Jets -> SetName(name.c_str());
	int NJets = col->getNumberOfElements();
	int NPart = 0;
	int NTrk = 0; 
	int NClu = 0; 
	float HitX = 0;
	float HitY = 0;
	float HitZ = 0;
	int colorindex = 0;
	float ClusterHitSize = 1.0;

	TVector3 HitScale(0.5*ClusterHitSize, 0.5*ClusterHitSize, 0.5*ClusterHitSize);

	for(int i = 0; i < NJets; i++)
	{
		ReconstructedParticle * currJet = dynamic_cast<ReconstructedParticle*>( col->getElementAt(i) );
		NPart = currJet->getParticles().size();
//		colorindex = ((i%2)*50+5*i+GlobalRandomColorIndex*31)%105;
//		colorindex = 51 + (i*9 + GlobalRandomColorIndex*15)%50 ;
//		colorindex = i*9 + 51;
		colorindex = i + 2; 
		if(colorindex == 5) colorindex = 94;
		TEveCompound *a_Jet = new TEveCompound();
		a_Jet->SetName("aJet");
		a_Jet->SetMainColor(colorindex);

		for(int j = 0; j < NPart; j++)
		{
			ReconstructedParticle * currRecoP = currJet->getParticles()[j];
			NTrk = currRecoP -> getTracks().size();
			NClu = currRecoP -> getClusters().size();
			TEveElementList* TrackAsshits = new TEveElementList;
			TrackAsshits->SetName("PFOTrk");

			TEveElementList* RecoPCluster = new TEveElementList;
			RecoPCluster->SetName("PFOClu");


			for(int k = 0; k < NTrk; k++)
			{
				Track* atrack = currRecoP -> getTracks()[k];
				int AssoHitNum = atrack->getTrackerHits().size();
				float Omega = atrack->getOmega();
				float TanL = atrack->getTanLambda();
				cout<<"Omega "<<Omega<<endl;
				cout << "i = " << i << ", j = " << j << ", k = " << k << endl;
				if(fabs(Omega) < 1e-3 || fabs(TanL) > 1)
				{
					cout << AssoHitNum << endl;
					for(int l(0); l<AssoHitNum; l++)
					{
						TrackerHit* hit = dynamic_cast<TrackerHit*>( atrack->getTrackerHits()[l] );
						HitX=hit->getPosition()[0];
						HitY=hit->getPosition()[1];
						HitZ=hit->getPosition()[2];

						cout << "HitX = " << HitX << ", HitY = " << HitY << ", HitZ = " << HitZ << endl;

						TEvePointSet* q1 = new TEvePointSet(1);
						q1->SetName("Calorimeter Hit ");
						q1->SetMarkerStyle(3);
						q1->SetPoint(0, 0.1*HitX, 0.1*HitY, 0.1*HitZ);
						q1->SetMarkerColor(colorindex);

						q1->SetMarkerSize(0.1);
						TrackAsshits->AddElement(q1);
					}
				}
			}

			// for(int k2 = 0; k2 < NClu; k2++)
			// {
			// 	Cluster* aclu = currRecoP -> getClusters()[k2];
			// 	CalorimeterHitVec Hits = aclu->getCalorimeterHits();
			// 	for(int j2 = 0; j2<Hits.size(); j2++)
			// 	{
			// 		TVector3 HitPosition = Hits[j2]->getPosition();
			// 		float HitEn = Hits[j2]->getEnergy();
			// 		HitPosition *= 0.1;
			// 		TEveBox* q = new TEveBox();
			// 		q = BoxPhi(HitPosition, HitScale, -1, 0, HitEn );
			// 		q->SetMainColor(colorindex);
			// 		q->SetLineColor(colorindex);
			// 		RecoPCluster->AddElement(q);
			// 	}
			// }

			a_Jet->AddElement(TrackAsshits);
			// a_Jet->AddElement(RecoPCluster);
		}

		Jets->AddElement(a_Jet);
	}
	return Jets;
}

#include "TStyle.h"
#include "TEveBox.h"
#include "TVector3.h"

TEveBox* Event::BoxPhi( TVector3 &HitPos, TVector3 &Scale, int Type, int SegOrStaveNumber, float HitEnergy ){

	//Type = -1; EndCap
	//Type = 0; Barrel, Based On Segment Number, ultilized for a la Videau Model HCAL
	//Type = 1; Barrel, Based On Phi Angle, ultilized for TESLA Model HCAL
	//Type = 2; Barrel, Based On Segment Number, ultilized for SID ECAL

	TEveBox * q = new TEveBox();
	q->SetName(Form("HitE = %.3e MeV", HitEnergy*1000));

	TStyle *gStyle;
	gStyle->SetPalette(1,0);
	q->SetMainTransparency(0);

	const float DegToRad = 1.74532925199432781e-02;
	const float Pi = 3.1415926535;

	float HitX=HitPos(0);
	float HitY=HitPos(1);
	float HitZ=HitPos(2);

	float s1X = 0;
	float s1Y = 0;
	float s1Z = 0;
	float s2X = 0;
	float s2Y = 0;
	float s2Z = 0;


	float phiAngle = 0;
	float SX = 0;
	float SY = 0;
	float SZ = 0;
	float SX1 = 0;
	float SY1 = 0;
	float SZ1 = 0;


	if(Type==-1)	//Based on EndCap
	{

		s1X = -1*Scale(0)/2.0;
		s2X = -1*s1X;
		s1Y = -1*Scale(1)/2.0;
		s2Y = s1Y;
		s1Z = Scale(2)/2.0;
		s2Z = s1Z;

		SX = fabs(0.5*Scale(0));
		SY = fabs(0.5*Scale(1));
		SZ = fabs(0.5*Scale(2));
		SX1 = SX;
		SY1 = SY;
		SZ1 = SZ;
	}
	else {
		if(Type==1)	//Based on Phi & SegmentNumbers(>=4 at least)
		{
			if(HitPos.Phi()>0)
			{
				phiAngle = 2*Pi/SegOrStaveNumber * int(HitPos.Phi()*SegOrStaveNumber/2/Pi+0.5);
			}else if(HitPos.Phi()<=0)
			{
				phiAngle = 2*Pi/SegOrStaveNumber * int(HitPos.Phi()*SegOrStaveNumber/2/Pi-0.5);
			}
		}
		if(Type==0)	//Based on StaveNumber		Currently Only used for ILD00 a la Videau model
		{
			if(SegOrStaveNumber==2 || SegOrStaveNumber==6)
			{phiAngle = 0;}
			if(SegOrStaveNumber==0 || SegOrStaveNumber==4)
			{phiAngle = 90*DegToRad;}
			if(SegOrStaveNumber==3 || SegOrStaveNumber==7)
			{phiAngle = 45*DegToRad;}
			if(SegOrStaveNumber==1 || SegOrStaveNumber==5)
			{phiAngle = 135*DegToRad;}
		}
		if(Type==2) //Based on StaveNumber		Used for SID ECAL
		{
			if(SegOrStaveNumber == 3 || SegOrStaveNumber == 9)
			{phiAngle = 0;}
			if(SegOrStaveNumber == 2 || SegOrStaveNumber == 8)
			{phiAngle = 30*DegToRad;}
			if(SegOrStaveNumber == 1 || SegOrStaveNumber == 7)
			{phiAngle = 60*DegToRad;}
			if(SegOrStaveNumber == 0 || SegOrStaveNumber == 6)
			{phiAngle = 90*DegToRad;}
			if(SegOrStaveNumber == 5 || SegOrStaveNumber == 11)
			{phiAngle = 120*DegToRad;}
			if(SegOrStaveNumber == 4 || SegOrStaveNumber == 10)
			{phiAngle = 150*DegToRad;}
		}	


		s1X = -0.5*(Scale(0)*sin(phiAngle)+Scale(2)*cos(phiAngle));
		s1Y = 0.5*(Scale(0)*cos(phiAngle)-Scale(2)*sin(phiAngle));
		s2X = -0.5*(Scale(0)*sin(phiAngle)-Scale(2)*cos(phiAngle));
		s2Y = 0.5*(Scale(0)*cos(phiAngle)+Scale(2)*sin(phiAngle));
		s1Z = Scale(1)/2.0;
		s2Z = s1Z;

		SX = fabs(0.5*(Scale(0)*sin(phiAngle)+Scale(2)*cos(phiAngle)));
		SX1 = fabs(0.5*(Scale(0)*sin(phiAngle)-Scale(2)*cos(phiAngle)));
		SY = fabs(0.5*(Scale(0)*cos(phiAngle)-Scale(2)*sin(phiAngle)));
		SY1 = fabs(0.5*(Scale(0)*cos(phiAngle)+Scale(2)*sin(phiAngle)));
		SZ = fabs(0.5*Scale(1));
		SZ1 = SZ;

	}

	q->SetVertex(5, HitX+s2X, HitY+s2Y, HitZ-s1Z);
	q->SetVertex(6, HitX+s2X, HitY+s2Y, HitZ+s1Z);

	q->SetVertex(4, HitX+s1X, HitY+s1Y, HitZ-s1Z);
	q->SetVertex(7, HitX+s1X, HitY+s1Y, HitZ+s1Z);

	q->SetVertex(3, HitX-s2X, HitY-s2Y, HitZ+s1Z);
	q->SetVertex(0, HitX-s2X, HitY-s2Y, HitZ-s1Z);

	q->SetVertex(2, HitX-s1X, HitY-s1Y, HitZ+s1Z);
	q->SetVertex(1, HitX-s1X, HitY-s1Y, HitZ-s1Z);

	return q;
}

void Event::DisplayMCParticles() {
	// MCTrack elements
	Bool_t selfRnrState = !(_pMCParticle->GetRnrSelf());
	Bool_t childRnrState = !(_pMCParticle->GetRnrChildren());

	_pMCParticle -> SetRnrSelfChildren(selfRnrState, childRnrState);
	gEve->Redraw3D(kFALSE, kTRUE);
}
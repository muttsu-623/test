#include "Event.h"

//STL
#include <iostream>

#include "TGFileDialog.h"
#include "lcio.h"
#include "IMPL/LCTOOLS.h"
#include "EVENT/MCParticle.h"
#include "EVENT/LCCollection.h"
#include "EVENT/LCEvent.h"
#include "UTIL/LCTime.h"
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
                 _evt(0) {}

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
	if (_eventNumber >= _lcReader->getNumberOfEvents()) 
	{
		cout << "Already displayed last event!!" << endl;
		return;
	}
	
	++_eventNumber;
	loadEvent();
}

void Event::Prev() {
	if (_eventNumber == 0) 
	{
		cout << "Already displayed first event!!" << endl;
		return;
	}

	--_eventNumber;
	loadEvent();
}

void Event::loadEvent() {
	if (_runNumber == -1) 
	{
    _evt = _lcReader->readNextEvent();
		_runNumber = _evt->getRunNumber();
		_eventNumber = _evt->getEventNumber();
	} 
	else 
	{
		_evt = _lcReader->readEvent(_runNumber, _eventNumber);
	}

	if (_evt != 0) 
	{
        printEventInfo(_evt);
        const vector< string >* strVec = _evt->getCollectionNames() ;
        vector< string >::const_iterator name ;
		for(name = strVec->begin() ; name != strVec->end() ; name++)
		{
            EVENT::LCCollection* col = _evt->getCollection( *name ) ;
            if(_evt->getCollection( *name )->getTypeName() == "MCParticle") { 
                printMCParticlesInfo(col);
                loadMCparticlesEvent();
            }
        }
    }
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
    _pMCParticle -> SetRnrSelfChildren(_MCPDraw, _MCPChildDraw);
    _pMCParticle->SetName("MCPARTICLE");
    gEve->AddElement(_pMCParticle);
    gEve->Redraw3D(kFALSE, kTRUE);
}

using namespace lcio;
using namespace EVENT;
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
	enum ETrType { kAucune=0, kElectron, kPositron, kMuonP, kMuonN, kPionP, kPionN, kKaonP, kKaonN, kProton, kNeutron, kKlong, kGamma, kIon, kNeutralHad, kNeutrino, kLowE, kLast};

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
		{"LowE      ",   15, 1, 1}
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

	//Fix missing PIDs.
	TDatabasePDG *pdgDB = TDatabasePDG::Instance();
	Int_t ionCode = 1000010020;
	if(!pdgDB->GetParticle(ionCode)){
		pdgDB->AddParticle("Deuteron","Deuteron",2+8.071e-3,kTRUE,0,1,"Ion",ionCode);
	}

	std::vector<std::string>::const_iterator name;

	const std::vector< std::string >* strVec = evt->getCollectionNames() ;

	for( name = strVec->begin() ; name != strVec->end() ; name++){
		LCCollection* col = evt->getCollection( *name ) ;
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


				if( Length<MCTracksMinLength ) continue; // Skip small tracks
				if( Length<=0) continue; // Protectin against bad parameters = 0      ??
				if( PID>=1000010020 ) continue;  //Mute the heavy hygen nuclea and so on.
				if( PT < PTCut) continue;

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
							TrType = kIon; 
							currCompound = cpdIon;
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

					// TEvePathMark* pm1 = new TEvePathMark(TEvePathMark::kDaughter);
					// TEvePathMark* pm2 = new TEvePathMark(TEvePathMark::kDaughter);

					TEvePathMark* pm3 = new TEvePathMark(TEvePathMark::kDecay);


					// if( (Vz<2350 && Vz>-2350 && GenRadius<1810) && (Ez>2350 || Ez<-2350 || EndRadius>1810) )   // if cross the board of TPC
					// {
					// 	std::string SETHitCollection = "SETCollection";
					// 	try{
					// 		LCCollection* col = evt->getCollection( SETHitCollection ) ;
					// 		int nHits = col->getNumberOfElements();
					// 		cout << nHits << endl;
					// 		int count = 0;
					// 		for(int j=0; j<nHits; j++)
					// 		{
					// 			SimTrackerHit* hit = dynamic_cast<SimTrackerHit*>( col->getElementAt(j) );
					// 			MCParticle* hitMCPart = dynamic_cast<MCParticle*>( hit->getMCParticle());
					// 			if(hitMCPart==part && count==0)
					// 			{
					// 				TEveVector SetHit(hit->getPosition()[0]/10.0, hit->getPosition()[1]/10.0, hit->getPosition()[2]/10.0);
					// 				pm1->fV.Set(SetHit);
					// 				track->AddPathMark(*pm1);
					// 				count=1;
					// 			}
					// 		}
					// 	}
					// 	catch (lcio::DataNotAvailableException zero) { }
					// }


					// if ( (Vz<3381.6 && Vz>-3381.6 && GenRadius<3973.6) && (Ez>3381.6 || Ez<-3381.6 || EndRadius > 3973.6) )   // if end outside the Calo
					// {
					// 	float MuonCaloHitDis = 0;
					// 	float EndPointDisMax = 0;
					// 	float Xmax = 0;
					// 	float Ymax = 0;
					// 	float Zmax = 0;

					// 	const std::vector< std::string >* strVec = evt->getCollectionNames() ;

					// 	for( std::vector<std::string>::const_iterator  name2 = strVec->begin() ; name2 != strVec->end() ; name2++){
					// 		try{
					// 			LCCollection* col = evt->getCollection( *name2 ) ;
					// 			string SubD (*name2, 0, 4);
					// 			if ( SubD=="Muon" )
					// 			{
					// 				int nMuonHits = col->getNumberOfElements();
					// 				for(int i = 0; i<nMuonHits; i++)
					// 				{
					// 					SimCalorimeterHit* hit11 = dynamic_cast<SimCalorimeterHit*>( col->getElementAt(i) );
					// 					MCParticle* hitMCPart11 = dynamic_cast<MCParticle*>( hit11->getParticleCont(0));
					// 					if( hitMCPart11==part )
					// 					{ 
					// 						MuonCaloHitDis = sqrt(hit11->getPosition()[0]*hit11->getPosition()[0]+hit11->getPosition()[1]*hit11->getPosition()[1]+hit11->getPosition()[2]*hit11->getPosition()[2]); 
					// 						if(MuonCaloHitDis>EndPointDisMax)
					// 						{
					// 							EndPointDisMax = MuonCaloHitDis;
					// 							Xmax = hit11->getPosition()[0]/10.0;
					// 							Ymax = hit11->getPosition()[1]/10.0;
					// 							Zmax = hit11->getPosition()[2]/10.0;
					// 						}
					// 					}
					// 				}
					// 			}
					// 		}catch (lcio::DataNotAvailableException zero) { }
					// 	}

					// 	TEveVector MuonFarHit(Xmax, Ymax, Zmax);
					// 	if(EndPointDisMax>10)
					// 	{
					// 		pm2->fV.Set(MuonFarHit);
					// 		track->AddPathMark(*pm2);
					// 	}
					// }

					pm3->fV.Set(End);
					track->AddPathMark(*pm3);


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
					if( KineticE < MCTracksLowEThresh && ! IsNeutrino(PID) )
					{
						TrType = kLowE;
						currCompound = cpdLowE;
					}else{

						switch( abs(PID) ){
							case  12:; case  14:; case  16:;    //Neutrinos
									 TrType = kNeutrino;
									 currCompound = cpdNeutrinos;
									 break;

							case 22:    //Gammas
									 TrType = kGamma;
									 currCompound = cpdGamma;
									 break;

							case 2112:  // 中性子
									 TrType = kNeutron;
									 currCompound = cpdNeutrons;
									 break;

							case 130:
									 TrType = kKlong;
									 currCompound = cpdKlongs;
									 break;

							default:    //All neutral hadrons 中性ハドロン
									 TrType = kNeutralHad;
									 currCompound = cpdNeutralHad;
									 break;
						}
					}
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

			//        MCTracks->AddElement(cpdMother);
			MCTracks->AddElement(cpdLowE);
			MCTracks->AddElement(cpdNeutrinos);
			MCTracks->AddElement(cpdGamma);
			MCTracks->AddElement(cpdNeutralHad);
			MCTracks->AddElement(cpdMuons);
			MCTracks->AddElement(cpdPions);
			MCTracks->AddElement(cpdElectrons);
			MCTracks->AddElement(cpdChargedKaons);
			MCTracks->AddElement(cpdProtons);
			MCTracks->AddElement(cpdNeutrons);
			MCTracks->AddElement(cpdKlongs);
			MCTracks->AddElement(cpdIon);

			cpdLowE->SetRnrSelfChildren(false, false);
			cpdNeutralHad->SetRnrSelfChildren(false, false);
			cpdNeutrons->SetRnrSelfChildren(false, false);

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
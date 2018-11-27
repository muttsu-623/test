#include "Event.h"

//STL
#include <iostream>

#include "TGFileDialog.h"
#include "EVENT/MCParticle.h"
#include "EVENT/LCCollection.h"
#include "EVENT/LCEvent.h"
#include "UTIL/LCTime.h"

ClassImp(Event);

// constractor
Event::Event() : // メンバ変数の初期化．この場合はコンストラクタ内でも良いが、constの場合はここで記述する必要がある。()内は初期値。
                 _ev(0),
                 _ev_max(0),
                 _trklist(0),
                 _fdialog(0),
                 _fileinfo(0),
                 _lcReader(0) {}

void Event::OpenFile() {
    _ev = 0;

    _fileinfo = new TGFileInfo();
    _fileinfo->fIniDir=(char*)".";
    _fdialog = new TGFileDialog(gClient->GetDefaultRoot(),0,kFDOpen,_fileinfo);
    std::cout << "Selected file name = " << _fileinfo->fFilename << std::endl;

    _lcReader = lcio::LCFactory::getInstance()->createLCReader(IO::LCReader::directAccess);
    _lcReader->open(_fileinfo->fFilename);

    std::cout  << std::endl <<  "     "  << _fileinfo->fFilename
    <<  "     [ number of runs: "    <<  _lcReader->getNumberOfRuns() 
    <<       ", number of events: "  <<  _lcReader->getNumberOfEvents() << " ] "   
    << std::endl
    << std::endl; 

    EVENT::LCEvent* evt ;
    int nEvents = 0 ;

    //----------- the event loop -----------
    while( (evt = _lcReader->readNextEvent()) != 0 ) {

        std::cout << std::endl 
        << "============================================================================" << std::endl ;
        std::cout << "        Event  : " << evt->getEventNumber() 
        << " - run:  "         << evt->getRunNumber()
        << " - timestamp "     << evt->getTimeStamp()   
        << " - weight "        << evt->getWeight()   
        << std::endl ;
        std::cout << "============================================================================" << std::endl ;    

        UTIL::LCTime evtTime( evt->getTimeStamp() ) ;
        std::cout << " date:      "      << evtTime.getDateString() << std::endl ;     
        std::cout << " detector : "      << evt->getDetectorName() << std::endl ;

        const std::vector< std::string >* strVec = evt->getCollectionNames() ;

        // loop over all collections:
        std::vector< std::string >::const_iterator name ;

        for( name = strVec->begin() ; name != strVec->end() ; name++){
            EVENT::LCCollection* col = evt->getCollection( *name ) ;

            // call the detailed print functions depending on type name
            if( evt->getCollection( *name )->getTypeName() == "MCParticle" ){

                printMCParticles( col ) ;

            }
        }

        nEvents ++ ;
    } 
  // -------- end of event loop -----------
}

void Event::printMCParticles(const EVENT::LCCollection* col ) {

    if( col->getTypeName() != EVENT::LCIO::MCPARTICLE ){

        std::cout << " collection not of type " << EVENT::LCIO::MCPARTICLE << std::endl ;
        return ;
    }

    std::cout << std::endl 
    << "--------------- " << "print out of "  << EVENT::LCIO::MCPARTICLE << " collection "
    << "--------------- " << std::endl ;

    int nParticles =  col->getNumberOfElements() ;

    // fill map with particle pointers and collection indices
    typedef std::map< EVENT::MCParticle*, int > PointerToIndexMap ;
    PointerToIndexMap p2i_map ;
    std::vector<EVENT::MCParticle*> moms ;

    for( int k=0; k<nParticles; k++){
        EVENT::MCParticle* part =  dynamic_cast<EVENT::MCParticle*>( col->getElementAt( k ) ) ;
        p2i_map[ part ] = k ; 

        moms.push_back( part ) ;
    }

    std::cout << std::endl
    <<  "[   id   ]index|      PDG |    px,     py,        pz    | px_ep,   py_ep , pz_ep      | energy  |gen|[simstat ]| vertex x,     y   ,   z     | endpoint x,    y  ,   z     |    mass |  charge |            spin             | colorflow | [parents] - [daughters]"    
    << std::endl 
    << std::endl ;

    // loop over collection - preserve order
    for(  int index = 0 ; index < nParticles ; index++){

        EVENT::MCParticle* part =  dynamic_cast<EVENT::MCParticle*>( col->getElementAt( index ) ) ;

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
        printf("% 1.2e,% 1.2e,% 1.2e|" , 
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

        std::cout << " [" ;

        for(unsigned int k=0;k<part->getParents().size();k++){
            if(k>0) std::cout << "," ;
            std::cout << p2i_map[ part->getParents()[k] ]  ;
        }
        std::cout << "] - [" ;
        for(unsigned int k=0;k<part->getDaughters().size();k++){
            if(k>0) std::cout << "," ;
            std::cout << p2i_map[ part->getDaughters()[k] ]  ;
        }
        std::cout << "] " << std::endl ;
    }

    std::cout << std::endl 
    << "-------------------------------------------------------------------------------- " 
    << std::endl ;
}

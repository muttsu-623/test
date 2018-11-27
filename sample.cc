#include <stdio.h>
#include <iostream>
#include "TGFileDialog.h"
#include "TGNumberEntry.h"
#include "TLegend.h"
#include "TNtupleD.h"
#include "TObject.h"
#include "TSystem.h"
#include "TEveManager.h"
#include "TGLEmbeddedViewer.h"
#include "TEveViewer.h"
#include "TEveGeoShape.h"
#include "TGeoTube.h"
#include "TEveTrans.h"
#include "TEveText.h"
#include "TEveArrow.h"
#include "TEvePad.h"
#include "TEveTrack.h"
#include "TEveTrackPropagator.h"
#include "lcio.h"
#include "EVENT/LCEvent.h"
#include "UTIL/LCTime.h"
#include "EVENT/LCCollection.h"
#include "EVENT/MCParticle.h"

#ifndef __EVENT_H__
#define __EVENT_H__

class TNtupleD;
class TEvePad;
class TGFileInfo;
class TGFileDialog;
class TFile;
class TGNumberEntryField;
class TPolyLine3D;
class TLegend;
class Particle;

class Event : public TObject {
  public :
    // Event(); // デフォルトコンストラクタは必須で、メンバ変数を初期化しなければいけない。
    // ~Event() {};
    void OpenFile();
    void printMCParticles(const EVENT::LCCollection* col );

  private :
    int _ev;
    int _ev_max;
    TEveTrackList *_trklist;
    TGFileDialog* _fdialog; 
    TGFileInfo* _fileinfo;
    lcio::LCReader* _lcReader;

  ClassDef(Event,1) // とりあえず書いておいたほうがいいらしい。
};

// constractor
// Event::Event() : // メンバ変数の初期化．この場合はコンストラクタ内でも良いが、constの場合はここで記述する必要がある。()内は初期値。
//                  _ev(0),
//                  _ev_max(0),
//                  _trklist(0),
//                  _fdialog(0),
//                  _fileinfo(0),
//                  _lcReader(0) {}

ClassImp(Event)

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

#endif

// main
int main() {    
    const float MAINFRAME_WIDTH  = 800.;
    const float MAINFRAME_HEIGHT = 800.;
    const float CANVAS_WIDTH     = 600.;
    const float CANVAS_HEIGHT    = 560.;

    TEveManager::Create(kFALSE); // this create a global pointer, gEve. kFALSE: not map widow yet.
    
    TGMainFrame* frm = new TGMainFrame(gClient->GetRoot());
      
    // create eve canvas
    TEvePad* pad = new TEvePad();
    pad->SetFillColor(kWhite);

    // create an embed GL viewer 
    TGLEmbeddedViewer* embviewer = new TGLEmbeddedViewer(frm, pad);
    frm->AddFrame(embviewer->GetFrame(), new TGLayoutHints(kLHintsNormal | kLHintsExpandX | kLHintsExpandY));

    // create a GL viewer 
    TEveViewer* viewer = new TEveViewer("GLViewer");
    viewer->SetGLViewer(embviewer, embviewer->GetFrame());
    viewer->AddScene(gEve->GetGlobalScene());
    viewer->AddScene(gEve->GetEventScene());
    gEve->GetViewers()->AddElement(viewer);

    // define a tube (cylinder)
    TEveGeoShape* tcp = new TEveGeoShape;
    tcp->SetShape(new TGeoTube(3.290000000e+02, 1.808000000e+03, 2.350000000e+03)); // rmin, rmax, dz
    tcp->SetNSegments(100); // number of vertices
    tcp->SetMainColor(kYellow);
    tcp->SetMainAlpha(0.5);
    tcp->RefMainTrans().SetPos(0, 0, 0); // set position
    gEve->AddElement(tcp);

    TEveGeoShape* eCal = new TEveGeoShape;
    eCal->SetShape(new TGeoTube(1.847415655e+03, 2.028e+03, 2.350000000e+03)); // rmin, rmax, dz
    eCal->SetNSegments(100); // number of vertices
    eCal->SetMainColor(kGreen);
    eCal->SetMainAlpha(0.5);
    eCal->RefMainTrans().SetPos(0, 0, 0); // set position
    gEve->AddElement(eCal);

    TEveGeoShape* hCal = new TEveGeoShape;
    hCal->SetShape(new TGeoTube(2.058000000e+03, 3.39546e+03, 2.350000000e+03)); // rmin, rmax, dz
    hCal->SetNSegments(100); // number of vertices
    hCal->SetMainColor(kBlue);
    hCal->SetMainAlpha(0.5);
    hCal->RefMainTrans().SetPos(0, 0, 0); // set position
    gEve->AddElement(hCal);

    // xAxis, yAxis, zAxis
    TEveArrow* xAxis = new TEveArrow(6000., 0., 0., 0., 0., 0.);
    xAxis->SetMainColor(kGreen);
    xAxis->SetPickable(kFALSE);
    xAxis->SetTubeR(0.1E-02);
    gEve->AddElement(xAxis);

    TEveText* xLabel = new TEveText("x");
    xLabel->SetFontSize(40);
    xLabel->SetMainColor(kGreen);
    xLabel->RefMainTrans().SetPos(6100, 0, 0);
    gEve->AddElement(xLabel);

    TEveArrow* yAxis = new TEveArrow(0., 6000., 0., 0., 0., 0.);
    yAxis->SetMainColor(kBlue);
    yAxis->SetPickable(kFALSE);
    yAxis->SetTubeR(0.1E-02);
    gEve->AddElement(yAxis);

    TEveText* yLabel = new TEveText("y");
    yLabel->SetFontSize(40);
    yLabel->SetMainColor(kBlue);
    yLabel->RefMainTrans().SetPos(0, 6100, 0);
    gEve->AddElement(yLabel);

    TEveArrow* zAxis = new TEveArrow(0., 0., 6000., 0., 0., 0.);
    zAxis->SetMainColor(kRed);
    zAxis->SetPickable(kFALSE);
    zAxis->SetTubeR(0.1E-02);
    gEve->AddElement(zAxis);

    TEveText* zLabel = new TEveText("z");
    zLabel->SetFontSize(40);
    zLabel->SetMainColor(kRed);
    zLabel->RefMainTrans().SetPos(0, 0, 6100);
    gEve->AddElement(zLabel);

    gEve->Redraw3D(kTRUE);

    // Frame1の作成。
    TGHorizontalFrame* bframe1 = new TGHorizontalFrame(frm, CANVAS_WIDTH, CANVAS_HEIGHT, kFixedWidth);
    frm->AddFrame(bframe1);

    // exitボタン。
    TGTextButton* b_exit = new TGTextButton(bframe1,"Exit","gApplication->Terminate(0)");
    bframe1->AddFrame(b_exit,new TGLayoutHints(kLHintsLeft));

    Event *ev = new Event();
    // display preparation done.
    frm->MapSubwindows();
    frm->Layout();
    frm->Resize(MAINFRAME_WIDTH,MAINFRAME_HEIGHT);
    frm->MapWindow();
}
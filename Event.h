#ifndef __EVENT_H__
#define __EVENT_H__

#include "TObject.h"

// iLCSoft
#include "lcio.h"
#include "IO/LCReader.h"
#include "EVENT/LCEvent.h"
#include "EVENT/LCCollection.h"

class TEveElementList;
class TEveTrackList;
class TGFileDialog;
class TGFileInfo;

using lcio::LCReader;
using lcio::LCEvent;

class Event : public TObject {
  public :
    Event(); // デフォルトコンストラクタは必須で、メンバ変数を初期化しなければいけない。
    // ~Event() {};
    void OpenFile();
    void Next();
    void Prev();
    void loadEvent();
    void printEventInfo(LCEvent* evt);
    void printMCParticlesInfo(const EVENT::LCCollection* col);
    void loadMCparticlesEvent();
    TEveElementList* BuildMCParticles( LCEvent *evt );
    void printReconstructedParticles( const EVENT::LCCollection* col );
    void loadReconstractedEvent(EVENT::LCCollection* col, std::string name);
    TEveElementList* BuildPFOs(EVENT::LCCollection* col, std::string name );
  private :
    int _ev;
    int _ev_max;
    TEveTrackList *_trklist;
    TGFileDialog* _fdialog; 
    TGFileInfo* _fileinfo;
    LCReader* _lcReader;
    int _runNumber = -1;
    int _eventNumber = 0;
    int _numberOfEvents;
    LCEvent* _evt;
    EVENT::LCCollection* _col;

    bool _MCPDraw = true;
    bool _MCPChildDraw = true;
    bool _PFODraw = true;
    bool _PFOChildDraw = true;
    std::map <std::string, int> _MCParticleDisplayFlag;
    TEveElementList* _pMCParticle = 0;
    TEveElementList* _pPFOs = 0;

  ClassDef(Event,1) // とりあえず書いておいたほうがいいらしい。
};
#endif
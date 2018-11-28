#ifndef __EVENT_H__
#define __EVENT_H__

#include "TObject.h"

// iLCSoft
#include "lcio.h"
#include "IO/LCReader.h"
#include "EVENT/LCEvent.h"

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
    void printEventInfo(LCEvent* evt);
    void printMCParticlesInfo(const EVENT::LCCollection* col);
    void loadMCparticlesEvent();
    TEveElementList* BuildMCParticles( LCEvent *evt );

  private :
    int _ev;
    int _ev_max;
    TEveTrackList *_trklist;
    TGFileDialog* _fdialog; 
    TGFileInfo* _fileinfo;
    LCReader* _lcReader;
    LCEvent* _evt;

    bool _MCPDraw = true;
    bool _MCPChildDraw = true;
    std::map <std::string, int> _MCParticleDisplayFlag;

  ClassDef(Event,1) // とりあえず書いておいたほうがいいらしい。
};
#endif
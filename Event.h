#ifndef __EVENT_H__
#define __EVENT_H__

#include "TObject.h"

// iLCSoft
#include "lcio.h"
#include "IO/LCReader.h"

class TEveTrackList;
class TGFileDialog;
class TGFileInfo;

using lcio::LCReader;

class Event : public TObject {
  public :
    Event(); // デフォルトコンストラクタは必須で、メンバ変数を初期化しなければいけない。
    // ~Event() {};
    void OpenFile();
    void printMCParticles(const EVENT::LCCollection* col );

  private :
    int _ev;
    int _ev_max;
    TEveTrackList *_trklist;
    TGFileDialog* _fdialog; 
    TGFileInfo* _fileinfo;
    LCReader* _lcReader;

  ClassDef(Event,1) // とりあえず書いておいたほうがいいらしい。
};
#endif
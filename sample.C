#include <stdio.h>
#include <iostream>
#include "TCanvas.h"
#include "TFile.h"
#include "TGFileDialog.h"
#include "TGNumberEntry.h"
#include "TLegend.h"
#include "TNtupleD.h"
#include "TObject.h"
#include "TPolyLine3D.h"
#include "TSystem.h"

#ifndef __PARTICLE_H__ // 擬似命令を取り入れてParticleの重複定義を避ける。
#define __PARTICLE_H__

#include "TObject.h"

class Particle : public TEveLine
{
  public :
    Particle();
    void SetMomentum();

    double _px;
    double _py;
    double _pz;

  ClassDef(Particle,1) // 自分で定義したクラスを利用する時に必要。
};

// constractor
Particle::Particle() :  TEveLine(), 
                        _px(0),
                        _py(0), 
                        _pz(0)
{
  SetNextPoint(0,0,0);
}

void Particle::SetMomentum()
{
   SetNextPoint(_px, _py, _pz);
}

#endif

#ifndef __EVENT_H__
#define __EVENT_H__

class TNtupleD;
class TCanvas;
class TGFileInfo;
class TGFileDialog;
class TFile;
class TGNumberEntryField;
class TPolyLine3D;
class TLegend;
class Particle;
class TEveManager;

class Event : public TObject
{
  public :
    Event(TEveManager *in);
    // void SetCanvas(TCanvas* in);
    // void SetEveManager(TEveManager* in);
    void OpenFile(); 
    void SetNField(TGNumberEntryField* in);
    void Update();
    void Next();
    void Prev();
    void Goto();

    const float x_min = -100.;
    const float x_max = 100.;
    const float y_min = -100.;
    const float y_max = 100.;
    const float z_min = -100.;
    const float z_max = 100.;

  private :
    int _ev;
    int _ev_max;

    // TCanvas* _canvas;
    TEveManager* _manager;
    TNtupleD* _tup; // double型のみ扱えるTTree

    TGFileDialog* _fdialog; 
    TGFileInfo* _fileinfo;
    TFile* _file;

    TGNumberEntryField* _ev_field; // 何番目の数字か表示されているUI部品

    Particle* _mc_b;    // MC(Monte Carlo) info b 
    Particle* _rc_b;    // Reco info b
    Particle* _mc_bbar; // MC info bbar    
    Particle* _rc_bbar; // Reco info bbar
    Particle* _mc_wp;   // MC info W+  
    Particle* _rc_wp;   // Reco info W+
    Particle* _mc_wm;   // MC info W- 
    Particle* _rc_wm;   // Reco info W-

    TPolyLine3D *_beampipe;
    TPolyLine3D *_yline;
    TPolyLine3D *_zline;

    TLegend *_leg;

  ClassDef(Event,1) // とりあえず書いておいたほうがいいらしい。
};

// constractor
Event::Event(TEveManager *in) : // メンバ変数の初期化．この場合はコンストラクタ内でも良いが、constの場合はここで記述する必要がある。()内は初期値。
                 _ev(0),
                 _ev_max(0),
                 _manager(in),
                 // ファイルに関するもの
                 _fileinfo(0),
                 _fdialog(0),
                 // 2変数のTTree
                 _tup(0),
                 // 粒子に関して
                 _mc_b(0),
                 _rc_b(0),
                 _mc_bbar(0),
                 _rc_bbar(0),
                 _mc_wp(0),
                 _rc_wp(0),
                 _mc_wm(0),
                 _rc_wm(0),
                 // 3D Polyline
                 _beampipe(0),
                 _yline(0),
                 _zline(0),
                 // ファイル
                 _file(0) {}

void Event::OpenFile()
{
  _ev = 0;
  _ev_field->SetNumber(_ev); // _ev_fieldに_evの値をセット。

  /*
  if (_mc_b) delete _mc_b;
  if (_rc_b) delete _rc_b;
  if (_mc_bbar) delete _mc_bbar;
  if (_rc_bbar) delete _rc_bbar;
  if (_mc_wp) delete _mc_wp;
  if (_rc_wp) delete _rc_wp;
  if (_mc_wm) delete _mc_wm;
  if (_rc_wm) delete _rc_wm;
  if (_beampipe) delete _beampipe;
  if (_yline) delete _yline;
  if (_zline) delete _zline;
  if (_file) delete _file;
  if (_fileinfo) delete _fileinfo;
  if (_fdialog) delete _fdialog;
  */

  _fileinfo = new TGFileInfo();
  _fileinfo->fIniDir=(char*)".";
  _fdialog = new TGFileDialog(gClient->GetDefaultRoot(),0,kFDOpen,_fileinfo);

  _file = new TFile(_fileinfo->fFilename);

  _tup = static_cast<TNtupleD*>(_file->Get("hAnl")); 
  _ev_max = _tup->GetEntries(); // TTreeのEntryの数を _ev_maxに入れている
  std::cerr << "Total # of events = " << _ev_max << std::endl;

  // b quark
  _mc_b = new Particle;
    _mc_b->SetMainColor(kRed);
    _mc_b->SetLineStyle(2);
  // _rc_b = new Particle;
    // _rc_b->SetLineColor(kRed);
    // _rc_b->SetLineStyle(1);
  // bbar quark
  // _mc_bbar = new Particle();
    // _mc_bbar->SetLineColor(kGreen);
    // _mc_bbar->SetLineStyle(2);
  // _rc_bbar = new Particle();
    // _rc_bbar->SetLineColor(kGreen);
    // _rc_bbar->SetLineStyle(1);
  // W+ boson 
  // _mc_wp = new Particle();
    // _mc_wp->SetLineColor(kBlue);
    // _mc_wp->SetLineStyle(2);
  // _rc_wp = new Particle();
    // _rc_wp->SetLineColor(kBlue);
    // _rc_wp->SetLineStyle(1);
  // W- boson 
  // _mc_wm = new Particle();
    // _mc_wm->SetLineColor(kYellow);
    // _mc_wm->SetLineStyle(2);
  // _rc_wm = new Particle();
    // _rc_wm->SetLineColor(kYellow);
    // _rc_wm->SetLineStyle(1);

  // TTreeの値をそれぞれ代入している。
  _tup->SetBranchAddress("b_px", &_mc_b->_px); // TTreeから"b_px"を取り出して、_mc_bのpxに入れている。
  _tup->SetBranchAddress("b_py", &_mc_b->_py); // TTreeから"b_py"を取り出して、_mc_bのpyに入れている。
  _tup->SetBranchAddress("b_pz", &_mc_b->_pz); // TTreeから"b_pz"を取り出して、_mc_bのpzに入れている。
  _tup->GetEntry(_ev);
  // _tup->SetBranchAddress("b_px_rec", &_rc_b->_px);
  // _tup->SetBranchAddress("b_py_rec", &_rc_b->_py);
  // _tup->SetBranchAddress("b_pz_rec", &_rc_b->_pz);
  // _tup->SetBranchAddress("bbar_px", &_mc_bbar->_px);
  // _tup->SetBranchAddress("bbar_py", &_mc_bbar->_py);
  // _tup->SetBranchAddress("bbar_pz", &_mc_bbar->_pz);
  // _tup->SetBranchAddress("bbar_px_rec", &_rc_bbar->_px);
  // _tup->SetBranchAddress("bbar_py_rec", &_rc_bbar->_py);
  // _tup->SetBranchAddress("bbar_pz_rec", &_rc_bbar->_pz);
  // _tup->SetBranchAddress("wp_px", &_mc_wp->_px);
  // _tup->SetBranchAddress("wp_py", &_mc_wp->_py);
  // _tup->SetBranchAddress("wp_pz", &_mc_wp->_pz);
  // _tup->SetBranchAddress("wp_px_rec", &_rc_wp->_px);
  // _tup->SetBranchAddress("wp_py_rec", &_rc_wp->_py);
  // _tup->SetBranchAddress("wp_pz_rec", &_rc_wp->_pz);
  // _tup->SetBranchAddress("wm_px", &_mc_wm->_px);
  // _tup->SetBranchAddress("wm_py", &_mc_wm->_py);
  // _tup->SetBranchAddress("wm_pz", &_mc_wm->_pz);
  // _tup->SetBranchAddress("wm_px_rec", &_rc_wm->_px);
  // _tup->SetBranchAddress("wm_py_rec", &_rc_wm->_py);
  // _tup->SetBranchAddress("wm_pz_rec", &_rc_wm->_pz);

  _mc_b->SetMomentum();
  // _rc_b->SetMomentum();
  // _mc_bbar->SetMomentum();
  // _rc_bbar->SetMomentum();
  // _mc_wp->SetMomentum();
  // _rc_wp->SetMomentum();
  // _mc_wm->SetMomentum();
  // _rc_wm->SetMomentum();

  _manager->AddElement((TEveLine*) _mc_b);
  _manager->Redraw3D(kTRUE);

  // _beampipe = new TPolyLine3D(2);
  // _beampipe->SetLineStyle(7);
  // _beampipe->SetLineColor(5);
  // _beampipe->SetPoint(1, x_min, 0, 0); // (何本目の線か, 終点のx座標, 終点のy座標, 終点のz座標)
  // _beampipe->SetPoint(2, x_max, 0, 0);
  // _beampipe->SetLineWidth(10);
  // _beampipe->Draw();

  // _yline = new TPolyLine3D(2);
  // _yline->SetLineStyle(7);
  // _yline->SetLineColor(2);
  // _yline->SetPoint(1, 0, y_min, 0);
  // _yline->SetPoint(2, 0, y_max, 0);
  // _yline->SetLineWidth(10);
  // _yline->Draw();

  // _zline = new TPolyLine3D(2);
  // _zline->SetLineStyle(7);
  // _zline->SetLineColor(4);
  // _zline->SetPoint(1, 0, 0, z_min);
  // _zline->SetPoint(2, 0, 0, z_max);
  // _zline->SetLineWidth(10);
  // _zline->Draw();

  // _leg = new TLegend(0.1,0.7,0.25,0.9);
  // _leg->AddEntry(_beampipe,"beam axis","l");
  // _leg->AddEntry(_yline, "yline", "l");
  // _leg->AddEntry(_zline, "zline", "l");
  // _leg->AddEntry(_mc_b,"b (MC)","l");
  // _leg->AddEntry(_rc_b,"b (REC)","l");
  // _leg->AddEntry(_mc_bbar,"bbar (MC)","l");
  // _leg->AddEntry(_rc_bbar,"bbar (REC)","l");
  // _leg->AddEntry(_mc_wp,"W+ (MC)","l");
  // _leg->AddEntry(_rc_wp,"W+ (REC)","l");
  // _leg->AddEntry(_mc_wm,"W- (MC)","l");
  // _leg->AddEntry(_rc_wm,"W- (REC)","l");

  // this->Update();
}

// void Event::SetCanvas(TCanvas* in) 
// {
//   _canvas = in;     // embcのアドレスが入っている。
//   _canvas->cd();    // Set current canvas & pad.
// //   _world->Draw();   // ヒストグラムを描く.
// }

// void SetEveManager(TEveManager* in) {
//   _manager = in;
// }

void Event::Next() 
{
  _ev++;
  this->Update();
}

void Event::Prev() 
{
  _ev--;
  this->Update();
}

void Event::SetNField(TGNumberEntryField* in) 
{
  _ev_field = in; // _ev_fieldのアドレスがはいる変数。
}

void Event::Goto()
{
  _ev = _ev_field->GetNumber(); 
  this->Update();
}

void Event::Update() 
{
  if (!_tup) OpenFile();

  if (_ev>=0 && _ev<_ev_max) {
    _tup->GetEntry(_ev); // TTreeのエントリーをevの数に指定。
    std::cerr << "Get Entry is called" << std::endl;
    std::cerr << "_mc_b's _px" << _mc_b->_px << std::endl;
  } else if (_ev<0){
    std::cerr << "### ERROR ### Event id must be more than 0." << std::endl;
    _ev=0;
  } else if (_ev>=_ev_max) {
    std::cerr << "### ERROR ### Event id must be less than " << _ev_max << "." << std::endl;
    _ev=_ev_max-1;
  }

  std::cerr << "_ev = " << _ev << std::endl;
  _ev_field->SetNumber(_ev);
 
  // _canvas->cd();                  // canvasを分割している場合、どの区画のcanvasに入るかのために重要。
  // _mc_b->SetMomentum();    // optionを代入し、Drawしている。
  // _rc_b->SetMomentum();
  // _mc_bbar->SetMomentum();
  // _rc_bbar->SetMomentum();
  // _mc_wp->SetMomentum();
  // _rc_wp->SetMomentum();
  // _mc_wm->SetMomentum();
  // _rc_wm->SetMomentum();
  // _leg->Draw();                   // 箱のDraw
  // _canvas->Update();
  // _manager->AddElement((TEveLine*) _mc_b);
  // _manager->Redraw3D(kTRUE);
  _manager->DoRedraw3D();
}

#endif

// main
void sample() {
  const float MAINFRAME_WIDTH  = 600.;
  const float MAINFRAME_HEIGHT = 600.;
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
  viewer->AddScene(gEve->GetEventScene());
  gEve->GetViewers()->AddElement(viewer);

  // display elements settings

      // // define a tube (cylinder)
      // TEveGeoShape* mytube = new TEveGeoShape;
      // mytube->SetShape(new TGeoTube(10, 10.1, 10)); // rmin, rmax, dz
      // mytube->SetNSegments(100); // number of vertices
      // mytube->RefMainTrans().SetPos(-10, -10, 10); // set position
      // gEve->AddElement(mytube);

      // TEveLine* myline = new TEveLine;
      // myline->SetMainColor(kGreen);
      // myline->SetLineStyle(1);
      // myline->SetLineWidth(5);
      // myline->SetNextPoint(0,0,0);
      // myline->SetNextPoint(-10,-10, 0);
      // gEve->AddElement(myline);

  // rendering.
  gEve->Redraw3D(kTRUE);

  // イベントインスタンスの生成。
  Event *ev = new Event(gEve);

  // ev->SetCanvas(gEve->AddCanvasTab("canvas"));
  // ev->SetEveManager(gEve);

  // Frame1の作成。
  TGHorizontalFrame* bframe1 = new TGHorizontalFrame(frm,CANVAS_WIDTH,(MAINFRAME_HEIGHT-CANVAS_HEIGHT)/2.,kFixedWidth);
  frm->AddFrame(bframe1);

  // exitボタン。
  TGTextButton* b_exit = new TGTextButton(bframe1,"Exit","gApplication->Terminate(0)");
  bframe1->AddFrame(b_exit,new TGLayoutHints(kLHintsLeft));

  // Open Fileボタン。
  TGTextButton* b_open = new TGTextButton(bframe1,"Open File");
  b_open->Connect("Clicked()", "Event", ev, "OpenFile()");
  bframe1->AddFrame(b_open,new TGLayoutHints(kLHintsLeft));

  // Frame2の作成。
  TGHorizontalFrame* bframe2 = new TGHorizontalFrame(frm,CANVAS_WIDTH,(MAINFRAME_HEIGHT-CANVAS_HEIGHT)/2.,kFixedWidth);
  frm->AddFrame(bframe2);

  // Prevボタン。
  TGTextButton* b_prev = new TGTextButton(bframe2,"Prev");
  b_prev->Connect("Clicked()","Event",ev,"Prev()");
  bframe2->AddFrame(b_prev,new TGLayoutHints(kLHintsCenterX));

  // 何番目のイベントかを表示するGUIパーツ。
  TGNumberEntry* ne = new TGNumberEntry(bframe2);
  ne->Connect("ValueSet(Long_t)","Event",ev,"Goto()");
  bframe2->AddFrame(ne,new TGLayoutHints(kLHintsCenterX));
  ev->SetNField(ne->GetNumberEntry());

  // Nextボタン
  TGTextButton* b_next = new TGTextButton(bframe2,"Next");
  b_next->Connect("Clicked()","Event",ev,"Next()");
  bframe2->AddFrame(b_next,new TGLayoutHints(kLHintsCenterX));

  // display preparation done.
  frm->MapSubwindows();
  frm->Layout();
  frm->Resize(MAINFRAME_WIDTH,MAINFRAME_HEIGHT);
  frm->MapWindow();

  ev->OpenFile();
}


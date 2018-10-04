#include <stdio.h>
#include <iostream>
#include "TFile.h"
#include "TGFileDialog.h"
#include "TGNumberEntry.h"
#include "TLegend.h"
#include "TNtupleD.h"
#include "TObject.h"
#include "TSystem.h"
#include "TEveText.h"
#include "TEveArrow.h"

#ifndef __PARTICLE_H__ // 擬似命令を取り入れてParticleの重複定義を避ける。
#define __PARTICLE_H__

#include "TObject.h"
#include "TEveTrack.h"
#include "TEveTrackPropagator.h"
#include "TEvePad.h"
#include "TEveManager.h"
#include "TGLEmbeddedViewer.h"
#include "TEveViewer.h"
#include "TEveGeoShape.h"
#include "TGeoTube.h"
#include "TEveTrans.h"

class Particle {
  public :
    Particle();

    TEveTrack* GetTrack(TEveTrackPropagator* prop);
    void SetLineColor(int in) { _lineColor = in; }
    void SetLineWidth(int in) { _lineWidth = in; }
    void SetLineStyle(int in) { _lineStyle = in; }

    double _px; 
    double _py; 
    double _pz; 
    double _e; 
    double _vx; 
    double _vy; 
    double _vz; 
    double _t0; 
    int    _pdg;

  private :
    int _lineColor;
    int _lineWidth;
    int _lineStyle;
    
    TParticle* _tpart; // This is necessary to create TEveTrack.
    TEveTrack* _evtrk; // This will be passed to gEve.

  ClassDef(Particle,1) // 自分で定義したクラスを利用する時に必要。
};

// constractor
Particle::Particle() : 
                        _px(0),
                        _py(0), 
                        _pz(0),
                        _e(0),
                        _vx(0),
                        _vy(0),
                        _vz(0),
                        _t0(0),
                        _pdg(11),
                        _lineColor(1),
                        _lineWidth(1),
                        _lineStyle(1),
	                      _tpart(0),
                        _evtrk(0) {}

// create TEveTrack from Particle instance.
TEveTrack* Particle::GetTrack(TEveTrackPropagator* prop) {
  if (_tpart) delete _tpart;
  if (_evtrk) delete _evtrk;
  // step2 define a particle
  _tpart = new TParticle;
  _tpart->SetProductionVertex(_vx,_vy,_vz,_t0); // vertex position and time (x,t)
  _tpart->SetMomentum(_px,_py,_pz,_e); // 4 momentum (p,e)
  _tpart->SetPdgCode(_pdg); // pid (used for charge definition) 
  // step3 assign the particle to track object 
  _evtrk = new TEveTrack(_tpart,0,prop);
  _evtrk->MakeTrack();
  _evtrk->SetMainColor(_lineColor);
  _evtrk->SetLineWidth(_lineWidth);
  _evtrk->SetLineStyle(_lineStyle);
  return _evtrk;
}

#endif

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
    Event();
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
    TEveTrackList *_trklist;
    TEveTrackPropagator* _prop;
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

    std::vector<Particle*> particles;
    void DrawTracks();

  ClassDef(Event,1) // とりあえず書いておいたほうがいいらしい。
};

// constractor
Event::Event() : // メンバ変数の初期化．この場合はコンストラクタ内でも良いが、constの場合はここで記述する必要がある。()内は初期値。
                 _ev(0),
                 _ev_max(0),
                 // ファイルに関するもの
                 _fileinfo(0),
                 _fdialog(0),
                 // 2変数のTTree
                 _tup(0),
                 // 粒子に関して
                 _trklist(0),
                 _mc_b(0),
                 _rc_b(0),
                 _mc_bbar(0),
                 _rc_bbar(0),
                 _mc_wp(0),
                 _rc_wp(0),
                 _mc_wm(0),
                 _rc_wm(0),
                 // ファイル
                 _file(0),
		            _ev_field(0) {}

void Event::OpenFile() {
  _ev = 0;
  if (!_ev_field) _ev_field = new TGNumberEntryField(); 
  _ev_field->SetNumber(_ev); // _ev_fieldに_evの値をセット。

  if (_mc_b) delete _mc_b;
  if (_rc_b) delete _rc_b;
  if (_mc_bbar) delete _mc_bbar;
  if (_rc_bbar) delete _rc_bbar;
  if (_mc_wp) delete _mc_wp;
  if (_rc_wp) delete _rc_wp;
  if (_mc_wm) delete _mc_wm;
  if (_rc_wm) delete _rc_wm;

  _fileinfo = new TGFileInfo();
  _fileinfo->fIniDir=(char*)".";
  _fdialog = new TGFileDialog(gClient->GetDefaultRoot(),0,kFDOpen,_fileinfo);

  _file = new TFile(_fileinfo->fFilename);
  _tup = static_cast<TNtupleD*>(_file->Get("hAnl")); 
  _ev_max = _tup->GetEntries(); // TTreeのEntryの数を _ev_maxに入れている
  std::cerr << "Total # of events = " << _ev_max << std::endl;

  if (!_trklist) {
	  cerr << "_trklist has been set." << endl;
    _trklist = new TEveTrackList();
    _prop = _trklist->GetPropagator();
    _prop->SetMaxR(1.797693e+308);
    _prop->SetMaxZ(1.797693e+308);
    _prop->SetMagField(3.); //FIXME
  }

  // b quark
  _mc_b = new Particle();
    _mc_b->SetLineColor(kRed);
    _mc_b->SetLineWidth(2);
    _mc_b->SetLineStyle(1);
  _rc_b = new Particle();
    _rc_b->SetLineColor(kRed);
    _rc_b->SetLineWidth(2);
    _rc_b->SetLineStyle(2);
  // bbar quark
  _mc_bbar = new Particle();
    _mc_bbar->SetLineColor(kBlue);
    _mc_bbar->SetLineWidth(2);
    _mc_bbar->SetLineStyle(1);
  _rc_bbar = new Particle();
    _rc_bbar->SetLineColor(kBlue);
    _rc_bbar->SetLineWidth(2);
    _rc_bbar->SetLineStyle(2);
  // W+ boson 
  _mc_wp = new Particle();
    _mc_wp->SetLineColor(kGreen);
    _mc_wp->SetLineWidth(2);
    _mc_wp->SetLineStyle(1);
  _rc_wp = new Particle();
    _rc_wp->SetLineColor(kGreen);
    _rc_wp->SetLineWidth(1);
    _rc_wp->SetLineStyle(2);
  // W- boson 
  _mc_wm = new Particle();
    _mc_wm->SetLineColor(kBlack);
    _mc_wm->SetLineWidth(2);
    _mc_wm->SetLineStyle(1);
  _rc_wm = new Particle();
    _rc_wm->SetLineColor(kBlack);
    _rc_wm->SetLineWidth(1);
    _rc_wm->SetLineStyle(2);

  particles.push_back(_mc_b);
  particles.push_back(_rc_b);
  particles.push_back(_mc_bbar);
  particles.push_back(_rc_bbar);
  particles.push_back(_mc_wp);
  particles.push_back(_rc_wp);
  particles.push_back(_mc_wm);
  particles.push_back(_rc_wm);

  // TTreeの値をそれぞれ代入している。
  _tup->SetBranchAddress("b_px", &_mc_b->_px); // TTreeから"b_px"を取り出して、_mc_bのpxに入れている。
  _tup->SetBranchAddress("b_py", &_mc_b->_py); // TTreeから"b_py"を取り出して、_mc_bのpyに入れている。
  _tup->SetBranchAddress("b_pz", &_mc_b->_pz); // TTreeから"b_pz"を取り出して、_mc_bのpzに入れている。
  _tup->SetBranchAddress("b_px_rec", &_rc_b->_px);
  _tup->SetBranchAddress("b_py_rec", &_rc_b->_py);
  _tup->SetBranchAddress("b_pz_rec", &_rc_b->_pz);
  _tup->SetBranchAddress("bbar_px", &_mc_bbar->_px);
  _tup->SetBranchAddress("bbar_py", &_mc_bbar->_py);
  _tup->SetBranchAddress("bbar_pz", &_mc_bbar->_pz);
  _tup->SetBranchAddress("bbar_px_rec", &_rc_bbar->_px);
  _tup->SetBranchAddress("bbar_py_rec", &_rc_bbar->_py);
  _tup->SetBranchAddress("bbar_pz_rec", &_rc_bbar->_pz);
  _tup->SetBranchAddress("wp_px", &_mc_wp->_px);
  _tup->SetBranchAddress("wp_py", &_mc_wp->_py);
  _tup->SetBranchAddress("wp_pz", &_mc_wp->_pz);
  _tup->SetBranchAddress("wp_px_rec", &_rc_wp->_px);
  _tup->SetBranchAddress("wp_py_rec", &_rc_wp->_py);
  _tup->SetBranchAddress("wp_pz_rec", &_rc_wp->_pz);
  _tup->SetBranchAddress("wm_px", &_mc_wm->_px);
  _tup->SetBranchAddress("wm_py", &_mc_wm->_py);
  _tup->SetBranchAddress("wm_pz", &_mc_wm->_pz);
  _tup->SetBranchAddress("wm_px_rec", &_rc_wm->_px);
  _tup->SetBranchAddress("wm_py_rec", &_rc_wm->_py);
  _tup->SetBranchAddress("wm_pz_rec", &_rc_wm->_pz);
}

void Event::Next() {
  _ev++;
  this->Update();
}

void Event::Prev() {
  _ev--;
  this->Update();
}

void Event::SetNField(TGNumberEntryField* in) {
  _ev_field = in; // _ev_fieldのアドレスがはいる変数。
}

void Event::Goto() {
  _ev = _ev_field->GetNumber(); 
  this->Update();
}

void Event::Update() {
  if (!_tup) OpenFile();

  if (_ev>=0 && _ev<_ev_max) {
    _tup->GetEntry(_ev); // TTreeのエントリーをevの数に指定。
  } else if (_ev<0){
    std::cerr << "### ERROR ### Event id must be more than 0." << std::endl;
    _ev=0;
  } else if (_ev>=_ev_max) {
    std::cerr << "### ERROR ### Event id must be less than " << _ev_max << "." << std::endl;
    _ev=_ev_max-1;
  }

  std::cerr << "_ev = " << _ev << std::endl;
  _ev_field->SetNumber(_ev);
  
  DrawTracks();
}

void Event::DrawTracks() {
  for (int i = 0; i < static_cast<int>(particles.size()); i++) {
    // step4 register the track to the track list.
    _trklist->AddElement(particles[i]->GetTrack(_prop));
  }
  gEve->AddElement(_trklist);
  gEve->Redraw3D(kFALSE, kTRUE);
}

#endif

// main
void sample() {    
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
    TEveGeoShape* tcpDetector = new TEveGeoShape;
    tcpDetector->SetShape(new TGeoTube(3.290000000e+02, 1.808000000e+03, 4.600000000e+03)); // rmin, rmax, dz
    tcpDetector->SetNSegments(100); // number of vertices
    tcpDetector->SetMainColor(kYellow);
    tcpDetector->SetMainAlpha(0.5);
    tcpDetector->RefMainTrans().SetPos(0, 0, 0); // set position
    gEve->AddElement(tcpDetector);

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
    TGTextButton* b_next = new TGTextButton(bframe1,"Next");
    b_next->Connect("Clicked()","Event",ev,"Next()");
    bframe1->AddFrame(b_next,new TGLayoutHints(kLHintsCenterX));

    // display preparation done.
    frm->MapSubwindows();
    frm->Layout();
    frm->Resize(MAINFRAME_WIDTH,MAINFRAME_HEIGHT);
    frm->MapWindow();
}
#include <stdio.h>
#include <iostream>
//#include "TCanvas.h"
#include "TFile.h"
#include "TGFileDialog.h"
#include "TGNumberEntry.h"
#include "TH3I.h"
#include "TLegend.h"
#include "TNtupleD.h"
#include "TObject.h"
//#include "TPolyLine3D.h"
#include "TSystem.h"

#ifndef __PARTICLE_H__ // 擬似命令を取り入れてParticleの重複定義を避ける。
#define __PARTICLE_H__

#include "TObject.h"
//#include "TPolyLine3D.h"
#include "TEveTrack.h"
#include "TEveTrackPropagator.h"
#include "TEvePad.h"
#include "TEveManager.h"
#include "TGLEmbeddedViewer.h"
#include "TEveViewer.h"
#include "TEveGeoShape.h"
#include "TGeoTube.h"
#include "TEveTrans.h"
                    //
//class Particle : p//ublic TPolyLine3D
class Particle // fo//r TEve drawing
{                   //
  public :          //
    Particle();
#if 0
    void DrawMomentum(Option_t *option="");
#else
    TEveTrack* GetTrack(TEveTrackPropagator* prop);
    void SetLineColor(int in) { _linecolor = in; }
    void SetLineWidth(int in) { _linewidth = in; }
#endif

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
    int    _linecolor;
    int    _linewidth;
    
    TParticle* _tpart; // This is necessary to create TEveTrack.
    TEveTrack* _evtrk; // This will be passed to gEve.


  ClassDef(Particle,1) // 自分で定義したクラスを利用する時に必要。
};

// constractor
Particle::Particle() : 
			// TPolyLine3D()の引数は
                        //TPolyLine3D(100), 
                        _px(0),
                        _py(0), 
                        _pz(0),
                        _e(0),
                        _vx(0),
                        _vy(0),
                        _vz(0),
                        _t0(0),
                        _pdg(11),
                        _linecolor(1),
                        _linewidth(1),
	                _tpart(0),
	                _evtrk(0)
{
#if 0
  SetPoint(1,0,0,0);
#endif
}

#if 0
void Particle::DrawMomentum(Option_t *option)
{
   SetPoint(2,_px,_py,_pz);
   Draw(option);
}
#else
TEveTrack* Particle::GetTrack(TEveTrackPropagator* prop)
{
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
  _evtrk->SetMainColor(_linecolor);
  _evtrk->SetLineWidth(_linewidth);
  return _evtrk;
}
#endif

#endif

#ifndef __EVENT_H__
#define __EVENT_H__

class TNtupleD;
#if 0
class TCanvas;
#else
class TEvePad;
#endif
class TGFileInfo;
class TGFileDialog;
class TFile;
class TGNumberEntryField;
class TPolyLine3D;
class TLegend;
class TH3I;
class Particle;

class Event : public TObject
{
  public :
    Event();
#if 0
    void SetCanvas(TCanvas* in);
#endif
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

#if 0
    TCanvas* _canvas;
#else
    //TEvePad* _canvas;
    TEveTrackList *_trklist;
    TEveTrackPropagator* _prop;
#endif
    TNtupleD* _tup; // double型のみ扱えるTTree

    TGFileDialog* _fdialog; 
    TGFileInfo* _fileinfo;
    TFile* _file;

    TGNumberEntryField* _ev_field; // 何番目の数字か表示されているUI部品

#if 0
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
    
    TH3I *_world;
#else
    std::vector<Particle*> particles;
    void DrawTracks();
#endif

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
#if 1
		 _trklist(0),
#endif
#if 0
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
#endif
                 // ファイル
                 _file(0),
		 _ev_field(0)
{

  // 3Dヒストグラムの作成。
//   _world = new TH3I("world","",
//     1, x_min, x_max, // Int_t nbinsx, Double_t xlow, Double_t xup
//     1, y_min, y_max, // Int_t nbinsy, Double_t ylow, Double_t yup
//     1, z_min, z_max); // Int_t nbinsz, Double_t zlow, Double_t zup
//   _world->SetStats(0);                    // 統計ボックス非表示
//   _world->GetXaxis()->SetNdivisions(0);   // X軸にアクセスし、目盛り分割数を0にする
//   _world->GetXaxis()->SetTickLength(0);   // X軸にアクセスし、目盛りの長さを0にする
//   _world->GetYaxis()->SetNdivisions(0);
//   _world->GetYaxis()->SetTickLength(0);
//   _world->GetZaxis()->SetNdivisions(0);
//   _world->GetZaxis()->SetTickLength(0);
}

void Event::OpenFile()
{
  _ev = 0;
  if (!_ev_field) _ev_field = new TGNumberEntryField(); 
  _ev_field->SetNumber(_ev); // _ev_fieldに_evの値をセット。

#if 0
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
#endif
  /*
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

#if 0
  // b quark
  _mc_b = new Particle();
    _mc_b->SetLineColor(2);
    _mc_b->SetLineStyle(2);
  _rc_b = new Particle();
    _rc_b->SetLineColor(2);
    _rc_b->SetLineStyle(1);
  // bbar quark
  _mc_bbar = new Particle();
    _mc_bbar->SetLineColor(4);
    _mc_bbar->SetLineStyle(2);
  _rc_bbar = new Particle();
    _rc_bbar->SetLineColor(4);
    _rc_bbar->SetLineStyle(1);
  // W+ boson 
  _mc_wp = new Particle();
    _mc_wp->SetLineColor(6);
    _mc_wp->SetLineStyle(2);
  _rc_wp = new Particle();
    _rc_wp->SetLineColor(6);
    _rc_wp->SetLineStyle(1);
  // W- boson 
  _mc_wm = new Particle();
    _mc_wm->SetLineColor(7);
    _mc_wm->SetLineStyle(2);
  _rc_wm = new Particle();
    _rc_wm->SetLineColor(7);
    _rc_wm->SetLineStyle(1);
#else
  if (!_trklist) {
	  cerr << "_trklist has been set." << endl;
    _trklist = new TEveTrackList();
    _prop = _trklist->GetPropagator();
    _prop->SetMagField(3.); //FIXME
    _prop->SetMaxZ(30);     //FIXME
  }

  // b quark
  Particle* _mc_b = new Particle();
    _mc_b->SetLineColor(2);
    _mc_b->SetLineWidth(2);
  // bbar quark
  Particle* _mc_bbar = new Particle();
    _mc_bbar->SetLineColor(4);
    _mc_bbar->SetLineWidth(2);

  particles.push_back(_mc_b);
  particles.push_back(_mc_bbar);
#endif

  // TTreeの値をそれぞれ代入している。
  _tup->SetBranchAddress("b_px", &_mc_b->_px); // TTreeから"b_px"を取り出して、_mc_bのpxに入れている。
  _tup->SetBranchAddress("b_py", &_mc_b->_py); // TTreeから"b_py"を取り出して、_mc_bのpyに入れている。
  _tup->SetBranchAddress("b_pz", &_mc_b->_pz); // TTreeから"b_pz"を取り出して、_mc_bのpzに入れている。
#if 0
  _tup->SetBranchAddress("b_px_rec", &_rc_b->_px);
  _tup->SetBranchAddress("b_py_rec", &_rc_b->_py);
  _tup->SetBranchAddress("b_pz_rec", &_rc_b->_pz);
#endif
  _tup->SetBranchAddress("bbar_px", &_mc_bbar->_px);
  _tup->SetBranchAddress("bbar_py", &_mc_bbar->_py);
  _tup->SetBranchAddress("bbar_pz", &_mc_bbar->_pz);
#if 0
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
#endif

#if 0  // TPolyLine3D can't be used in TEve. TEveLine can be used instead.
  _beampipe = new TPolyLine3D(2);
  _beampipe->SetLineStyle(7);
  _beampipe->SetLineColor(5);
  _beampipe->SetPoint(1, x_min, 0, 0); // (何本目の線か, 終点のx座標, 終点のy座標, 終点のz座標)
  _beampipe->SetPoint(2, x_max, 0, 0);
  _beampipe->SetLineWidth(10);
  _beampipe->Draw();

  _yline = new TPolyLine3D(2);
  _yline->SetLineStyle(7);
  _yline->SetLineColor(2);
  _yline->SetPoint(1, 0, y_min, 0);
  _yline->SetPoint(2, 0, y_max, 0);
  _yline->SetLineWidth(10);
  _yline->Draw();

  _zline = new TPolyLine3D(2);
  _zline->SetLineStyle(7);
  _zline->SetLineColor(4);
  _zline->SetPoint(1, 0, 0, z_min);
  _zline->SetPoint(2, 0, 0, z_max);
  _zline->SetLineWidth(10);
  _zline->Draw();

  _leg = new TLegend(0.1,0.7,0.25,0.9);
  _leg->AddEntry(_beampipe,"beam axis","l");
  _leg->AddEntry(_yline, "yline", "l");
  _leg->AddEntry(_zline, "zline", "l");
  _leg->AddEntry(_mc_b,"b (MC)","l");
  _leg->AddEntry(_rc_b,"b (REC)","l");
  _leg->AddEntry(_mc_bbar,"bbar (MC)","l");
  _leg->AddEntry(_rc_bbar,"bbar (REC)","l");
  _leg->AddEntry(_mc_wp,"W+ (MC)","l");
  _leg->AddEntry(_rc_wp,"W+ (REC)","l");
  _leg->AddEntry(_mc_wm,"W- (MC)","l");
  _leg->AddEntry(_rc_wm,"W- (REC)","l");

  this->Update();
#endif
}

#if 0
void Event::SetCanvas(TCanvas* in) 
{
  _canvas = in;     // embcのアドレスが入っている。
  _canvas->cd();    // Set current canvas & pad.
//   _world->Draw();   // ヒストグラムを描く.
}
#endif

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
  } else if (_ev<0){
    std::cerr << "### ERROR ### Event id must be more than 0." << std::endl;
    _ev=0;
  } else if (_ev>=_ev_max) {
    std::cerr << "### ERROR ### Event id must be less than " << _ev_max << "." << std::endl;
    _ev=_ev_max-1;
  }

  std::cerr << "_ev = " << _ev << std::endl;
  _ev_field->SetNumber(_ev);
#if 0
  _canvas->cd();                  // canvasを分割している場合、どの区画のcanvasに入るかのために重要。
  _mc_b->DrawMomentum("same");    // optionを代入し、Drawしている。
  _rc_b->DrawMomentum("same");
  _mc_bbar->DrawMomentum("same");
  _rc_bbar->DrawMomentum("same");
  _mc_wp->DrawMomentum("same");
  _rc_wp->DrawMomentum("same");
  _mc_wm->DrawMomentum("same");
  _rc_wm->DrawMomentum("same");
  _leg->Draw();                   // 箱のDraw
  _canvas->Update();
#else
  DrawTracks();
#endif
}

#endif

void Event::DrawTracks() {
  for (int i = 0; i < particles.size(); i++) {
    // step4 register the track to the track list.
    _trklist->AddElement(particles[i]->GetTrack(_prop));
  }
  gEve->AddElement(_trklist);
  gEve->Redraw3D(kTRUE);
}

// main
void sample() 
{    
    const float MAINFRAME_WIDTH  = 600.;
    const float MAINFRAME_HEIGHT = 600.;
    const float CANVAS_WIDTH     = 600.;
    const float CANVAS_HEIGHT    = 560.;

    TEveManager::Create(kFALSE); // this create a global pointer, gEve. kFALSE: not map widow yet.
    
    TGMainFrame* frm = new TGMainFrame(gClient->GetRoot());
      
    // create eve canvas
    TEvePad* pad = new TEvePad();
    pad->SetFillColor(kGray);

    // create an embed GL viewer 
    TGLEmbeddedViewer* embviewer = new TGLEmbeddedViewer(frm, pad);
    frm->AddFrame(embviewer->GetFrame(), new TGLayoutHints(kLHintsNormal | kLHintsExpandX | kLHintsExpandY));

    // create a GL viewer 
    TEveViewer* viewer = new TEveViewer("GLViewer");
    viewer->SetGLViewer(embviewer, embviewer->GetFrame());
    viewer->AddScene(gEve->GetGlobalScene());
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



#if 0 // This must be done after making tracks.
    // rendering.
    gEve->Redraw3D(kTRUE);
#endif

    // Frame1の作成。
    TGHorizontalFrame* bframe1 = new TGHorizontalFrame(frm, CANVAS_WIDTH, CANVAS_HEIGHT, kFixedWidth);
    frm->AddFrame(bframe1);

    // exitボタン。
    TGTextButton* b_exit = new TGTextButton(bframe1,"Exit","gApplication->Terminate(0)");
    bframe1->AddFrame(b_exit,new TGLayoutHints(kLHintsLeft));

#if 1 // test purpose only
    Event *ev = new Event();
    TGTextButton* b_next = new TGTextButton(bframe1,"Next");
    b_next->Connect("Clicked()","Event",ev,"Next()");
    bframe1->AddFrame(b_next,new TGLayoutHints(kLHintsCenterX));
#endif

    // display preparation done.
    frm->MapSubwindows();
    frm->Layout();
    frm->Resize(MAINFRAME_WIDTH,MAINFRAME_HEIGHT);
    frm->MapWindow();
}


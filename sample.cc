#include <stdio.h>
#include <iostream>
#include "TEveManager.h"
#include "TGLEmbeddedViewer.h"
#include "TEveViewer.h"
#include "TGButton.h"
#include "TEveGeoShape.h"
#include "TGeoTube.h"
#include "TEveTrans.h"
#include "TEveText.h"
#include "TEveArrow.h"
#include "TEvePad.h"
#include "Event.h"
#include "TRint.h"

// mainじゃないとダメっぽい?
int main(int argc, char** argv) {   

    TRint* app = new TRint("App",&argc,argv);

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

    Event *ev = new Event();

    // Frame1の作成。
    TGHorizontalFrame* bframe1 = new TGHorizontalFrame(frm, CANVAS_WIDTH, CANVAS_HEIGHT, kFixedWidth);
    frm->AddFrame(bframe1);

    TGTextButton* b_open = new TGTextButton(bframe1,"Open File");
    b_open->Connect("Clicked()","Event",ev,"OpenFile()");
    bframe1->AddFrame(b_open,new TGLayoutHints(kLHintsLeft));

    // exitボタン。
    TGTextButton* b_exit = new TGTextButton(bframe1,"Exit","gApplication->Terminate(0)");
    bframe1->AddFrame(b_exit,new TGLayoutHints(kLHintsLeft));

    // display preparation done.
    frm->MapSubwindows();
    frm->Layout();
    frm->Resize(MAINFRAME_WIDTH,MAINFRAME_HEIGHT);
    frm->MapWindow();

    app->Run(kTRUE);

    return 0;
}
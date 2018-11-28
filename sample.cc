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

    TEveGeoShape* vtx = new TEveGeoShape;
    vtx->SetShape(new TGeoTube(6.500000000, 6.549392000, 14.50000000)); // rmin, rmax, dz
    vtx->SetNSegments(16); // number of vertices
    vtx->SetMainColor(kYellow);
    vtx->SetMainAlpha(0.5);
    vtx->RefMainTrans().SetPos(0, 0, 0); // set position
    gEve->AddElement(vtx);

    TEveGeoShape* sit1 = new TEveGeoShape;
    sit1->SetShape(new TGeoTube(15.29, 15.44, 36.8)); // rmin, rmax, dz
    sit1->SetNSegments(10); // number of vertices
    sit1->SetMainColor(kYellow);
    sit1->SetMainAlpha(0.5);
    sit1->RefMainTrans().SetPos(0, 0, 0); // set position
    gEve->AddElement(sit1);

    TEveGeoShape* sit2 = new TEveGeoShape;
    sit2->SetShape(new TGeoTube(29.99, 30.14, 64.4)); // rmin, rmax, dz
    sit2->SetNSegments(19); // number of vertices
    sit2->SetMainColor(kYellow);
    sit2->SetMainAlpha(0.5);
    sit2->RefMainTrans().SetPos(0, 0, 0); // set position
    gEve->AddElement(sit2);

    // define a tube (cylinder)
    TEveGeoShape* tcp = new TEveGeoShape;
    tcp->SetShape(new TGeoTube(3.290000000e+01, 1.808000000e+02, 2.350000000e+02)); // rmin, rmax, dz
    tcp->SetNSegments(100); // number of vertices
    tcp->SetMainColor(kYellow);
    tcp->SetMainAlpha(0.5);
    tcp->RefMainTrans().SetPos(0, 0, 0); // set position
    gEve->AddElement(tcp);

    TEveGeoShape* eCal = new TEveGeoShape;
    eCal->SetShape(new TGeoTube(1.847415655e+02, 2.028e+02, 2.350000000e+02)); // rmin, rmax, dz
    eCal->SetNSegments(100); // number of vertices
    eCal->SetMainColor(kGreen);
    eCal->SetMainAlpha(0.5);
    eCal->RefMainTrans().SetPos(0, 0, 0); // set position
    gEve->AddElement(eCal);

    TEveGeoShape* hCal = new TEveGeoShape;
    hCal->SetShape(new TGeoTube(2.058000000e+02, 3.39546e+02, 2.350000000e+02)); // rmin, rmax, dz
    hCal->SetNSegments(100); // number of vertices
    hCal->SetMainColor(kBlue);
    hCal->SetMainAlpha(0.5);
    hCal->RefMainTrans().SetPos(0, 0, 0); // set position
    gEve->AddElement(hCal);

    // xAxis, yAxis, zAxis
    TEveArrow* xAxis = new TEveArrow(6000., 0., 0., 0., 0., 0.);
    xAxis->SetMainColor(kGreen);
    xAxis->SetPickable(kFALSE);
    xAxis->SetTubeR(0.1E-03);
    gEve->AddElement(xAxis);

    TEveText* xLabel = new TEveText("x");
    xLabel->SetFontSize(40);
    xLabel->SetMainColor(kGreen);
    xLabel->RefMainTrans().SetPos(6100, 0, 0);
    gEve->AddElement(xLabel);

    TEveArrow* yAxis = new TEveArrow(0., 6000., 0., 0., 0., 0.);
    yAxis->SetMainColor(kBlue);
    yAxis->SetPickable(kFALSE);
    yAxis->SetTubeR(0.1E-03);
    gEve->AddElement(yAxis);

    TEveText* yLabel = new TEveText("y");
    yLabel->SetFontSize(40);
    yLabel->SetMainColor(kBlue);
    yLabel->RefMainTrans().SetPos(0, 6100, 0);
    gEve->AddElement(yLabel);

    TEveArrow* zAxis = new TEveArrow(0., 0., 6000., 0., 0., 0.);
    zAxis->SetMainColor(kRed);
    zAxis->SetPickable(kFALSE);
    zAxis->SetTubeR(0.1E-03);
    gEve->AddElement(zAxis);

    TEveText* zLabel = new TEveText("z");
    zLabel->SetFontSize(40);
    zLabel->SetMainColor(kRed);
    zLabel->RefMainTrans().SetPos(0, 0, 6100);
    gEve->AddElement(zLabel);

    gEve->Redraw3D(kFALSE, kTRUE);

    Event *ev = new Event();

    // Frame1の作成。
    TGHorizontalFrame* bframe1 = new TGHorizontalFrame(frm, CANVAS_WIDTH, CANVAS_HEIGHT, kFixedWidth);
    frm->AddFrame(bframe1);

    TGTextButton* b_open = new TGTextButton(bframe1,"Open File");
    b_open->Connect("Clicked()","Event",ev,"OpenFile()");
    bframe1->AddFrame(b_open,new TGLayoutHints(kLHintsLeft));

    TGTextButton* b_next = new TGTextButton(bframe1,"Next");
    b_next->Connect("Clicked()","Event",ev,"Next()");
    bframe1->AddFrame(b_next,new TGLayoutHints(kLHintsCenterX));

    TGTextButton* b_prev = new TGTextButton(bframe1,"Prev");
    b_prev->Connect("Clicked()","Event",ev,"Prev()");
    bframe1->AddFrame(b_prev,new TGLayoutHints(kLHintsCenterX));

    // exitボタン。
    TGTextButton* b_exit = new TGTextButton(bframe1,"Exit","gApplication->Terminate(0)");
    bframe1->AddFrame(b_exit,new TGLayoutHints(kLHintsRight));

    // display preparation done.
    frm->MapSubwindows();
    frm->Layout();
    frm->Resize(MAINFRAME_WIDTH,MAINFRAME_HEIGHT);
    frm->MapWindow();

    ev->OpenFile();

    app->Run(kTRUE);

    return 0;
}
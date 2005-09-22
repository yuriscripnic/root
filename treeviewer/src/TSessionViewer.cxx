// @(#)root/treeviewer:$Name:  $:$Id: TSessionViewer.cxx
// Author: Marek Biskup, Jakub Madejczyk, Bertrand Bellenot 10/08/2005

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TSessionViewer                                                       //
//                                                                      //
// Widget used to manage Proof or local sessions, proof connections,    //
// queries construction and results handling.                           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TApplication.h"
#include "TGFileDialog.h"
#include "TBrowser.h"
#include "TGButton.h"
#include "TGLayout.h"
#include "TGListTree.h"
#include "TGCanvas.h"
#include "TGLabel.h"
#include "TGTextEntry.h"
#include "TGNumberEntry.h"
#include "TGTableLayout.h"
#include "TGComboBox.h"
#include "TGSplitter.h"
#include "TGProgressBar.h"
#include "TGListView.h"
#include "TGMsgBox.h"
#include "TGMenu.h"
#include "TGStatusBar.h"
#include "TGIcon.h"
#include "TChain.h"
#include "TDSet.h"
#include "TProof.h"
#include "TRandom.h"
#include "TSessionViewer.h"
#include "TSessionLogView.h"
#include "TQueryResult.h"
#include "TGTextView.h"
#include "TGMenu.h"
#include "TGToolBar.h"
#include "TGTab.h"
#include "TRootEmbeddedCanvas.h"
#include "TCanvas.h"
#include "TGMimeTypes.h"
#include "TInterpreter.h"
#include "TContextMenu.h"
#include "TG3DLine.h"
#include "TSessionDialogs.h"
#ifdef WIN32
#include "TWin32SplashThread.h"
#endif

ClassImp(TQueryDescription)
ClassImp(TSessionDescription)
ClassImp(TSessionServerFrame)
ClassImp(TSessionFrame)
ClassImp(TSessionQueryFrame)
ClassImp(TSessionFeedbackFrame)
ClassImp(TSessionOutputFrame)
ClassImp(TSessionInputFrame)
ClassImp(TSessionViewer)

const char *xpm_names[] = {
    "monitor01.xpm",
    "monitor02.xpm",
    "monitor03.xpm",
    "monitor04.xpm",
    0
};

const char *ofiletypes[] = {
   "C files",       "*.C",
   "ROOT files",    "*.root",
   "All files",     "*",
    0,               0
};

const char* const kPROOF_GuiConfFile = ".proofservers.conf";
char const kPROOF_GuiConfFileSeparator = '\t';

// Menu command id's
enum ESessionViewerCommands {
   kFileLoadLibrary,
   kFileCloseViewer,
   kFileQuit,

   kSessionConnect,
   kSessionDisconnect,
   kSessionCleanup,
   kSessionBrowse,

   kQueryNew,
   kQueryEdit,
   kQueryDelete,
   kQueryRemove,
   kQuerySubmit,
   kQueryStartViewer,

   kHelpAbout
};


////////////////////////////////////////////////////////////////////////////////
// Server Frame

//______________________________________________________________________________
TSessionServerFrame::TSessionServerFrame(TGWindow* p, Int_t w, Int_t h) :
   TGCompositeFrame(p, w, h), fFrmNewServer(0), fTxtName(0), fTxtAddress(0),
      fTxtConfig(0), fTxtUsrName(0), fViewer(0)
{
   // Constructor
}

//______________________________________________________________________________
TSessionServerFrame::~TSessionServerFrame()
{
   // Destructor
   Cleanup();
}

//______________________________________________________________________________
void TSessionServerFrame::Build(TSessionViewer *gui)
{
   SetLayoutManager(new TGVerticalLayout(this));
   TGCompositeFrame *tmp;
   TGButton* btnTmp;

   SetCleanup(kDeepCleanup);

   fViewer = gui;
   fFrmNewServer = new TGGroupFrame(this, "New Server");
   fFrmNewServer->SetCleanup(kDeepCleanup);

   AddFrame(fFrmNewServer, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

   fFrmNewServer->SetLayoutManager(new TGMatrixLayout(fFrmNewServer, 0, 2, 8));

   fFrmNewServer->AddFrame(new TGLabel(fFrmNewServer, "Connection Name:"),
                           new TGLayoutHints(kLHintsLeft, 3, 3, 3, 3));
   fFrmNewServer->AddFrame(fTxtName = new TGTextEntry(fFrmNewServer),
                           new TGLayoutHints());
   fTxtName->Resize(156, fTxtName->GetDefaultHeight());
   fFrmNewServer->AddFrame(new TGLabel(fFrmNewServer, "Server name:"),
                           new TGLayoutHints(kLHintsLeft, 3, 3, 3, 3));
   fFrmNewServer->AddFrame(fTxtAddress = new TGTextEntry(fFrmNewServer),
                           new TGLayoutHints());
   fTxtAddress->Resize(156, fTxtAddress->GetDefaultHeight());
   fFrmNewServer->AddFrame(new TGLabel(fFrmNewServer, "Port (default: 1093):"),
                           new TGLayoutHints(kLHintsLeft, 3, 3, 3, 3));
   fFrmNewServer->AddFrame(fNumPort = new TGNumberEntry(fFrmNewServer, 1093, 5,
            -1, TGNumberFormat::kNESInteger,TGNumberFormat::kNEANonNegative,
            TGNumberFormat::kNELLimitMinMax, 0, 65535),new TGLayoutHints());
   fFrmNewServer->AddFrame(new TGLabel(fFrmNewServer, "Configuration File:"),
                           new TGLayoutHints(kLHintsLeft, 3, 3, 3, 3));
   fFrmNewServer->AddFrame(fTxtConfig = new TGTextEntry(fFrmNewServer),
                           new TGLayoutHints());
   fTxtConfig->Resize(156, fTxtConfig->GetDefaultHeight());
   fFrmNewServer->AddFrame(new TGLabel(fFrmNewServer, "Log Level:"),
                           new TGLayoutHints(kLHintsLeft, 3, 3, 3, 3));

   fFrmNewServer->AddFrame(fLogLevel = new TGNumberEntry(fFrmNewServer, 0, 5, -1,
                           TGNumberFormat::kNESInteger,
                           TGNumberFormat::kNEANonNegative,
                           TGNumberFormat::kNELLimitMinMax, 0, 5),
                           new TGLayoutHints(kLHintsLeft, 3, 3, 3, 3));

   fFrmNewServer->AddFrame(new TGLabel(fFrmNewServer, "User Name:"),
            new TGLayoutHints(kLHintsLeft, 3, 3, 3, 3));
   fFrmNewServer->AddFrame(fTxtUsrName = new TGTextEntry(fFrmNewServer),
                           new TGLayoutHints());
   fTxtUsrName->Resize(156, fTxtUsrName->GetDefaultHeight());

   fFrmNewServer->AddFrame(new TGLabel(fFrmNewServer, "Process mode :"),
            new TGLayoutHints(kLHintsLeft | kLHintsBottom | kLHintsExpandX,
            3, 3, 3, 3));
   fFrmNewServer->AddFrame(fSync = new TGCheckButton(fFrmNewServer,
      "&Synchronous"), new TGLayoutHints(kLHintsLeft | kLHintsBottom |
      kLHintsExpandX, 3, 3, 3, 3));
   fSync->SetToolTipText("Connect to the server in synchronous mode.");
   fSync->SetState(kButtonDown);

   AddFrame(tmp = new TGCompositeFrame(this, 140, 10, kHorizontalFrame),
                       new TGLayoutHints(kLHintsLeft | kLHintsExpandX));
   tmp->SetCleanup(kDeepCleanup);
   tmp->AddFrame(btnTmp = new TGTextButton(tmp, "     Add     "),
                 new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 3, 3, 3, 3));
   tmp->Resize(155, btnTmp->GetDefaultHeight());
   btnTmp->Connect("Clicked()", "TSessionServerFrame", this,
                   "OnBtnAddClicked()");
   tmp->AddFrame(btnTmp = new TGTextButton(tmp, "   Connect   "),
                 new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 3, 3, 3, 3));
   tmp->Resize(155, btnTmp->GetDefaultHeight());
   btnTmp->Connect("Clicked()", "TSessionServerFrame", this,
                   "OnBtnConnectClicked()");

   AddFrame(tmp = new TGCompositeFrame(this, 140, 20, kHorizontalFrame),
                       new TGLayoutHints(kLHintsLeft | kLHintsExpandX));
   tmp->SetCleanup(kDeepCleanup);
   tmp->AddFrame(btnTmp = new TGTextButton(tmp, "  New server  "),
                 new TGLayoutHints(kLHintsLeft | kLHintsBottom |
                 kLHintsExpandX, 3, 3, 15, 3));
   btnTmp->Connect("Clicked()", "TSessionServerFrame", this,
                   "OnBtnNewServerClicked()");
   tmp->AddFrame(btnTmp = new TGTextButton(tmp, "    Delete    "),
                 new TGLayoutHints(kLHintsLeft | kLHintsBottom |
                 kLHintsExpandX, 3, 3, 15, 3));
   btnTmp->Connect("Clicked()", "TSessionServerFrame", this,
                   "OnBtnDeleteClicked()");
   fTxtConfig->Connect("DoubleClicked()", "TSessionServerFrame", this,
                       "OnConfigFileClicked()");
}

//______________________________________________________________________________
void TSessionServerFrame::OnConfigFileClicked()
{
   TGFileInfo fi;
   fi.fFileTypes = ofiletypes;
   new TGFileDialog(fClient->GetRoot(), fViewer, kFDOpen, &fi);
   if (!fi.fFilename) return;
   fTxtConfig->SetText(gSystem->BaseName(fi.fFilename));
}

//______________________________________________________________________________
void TSessionServerFrame::OnBtnDeleteClicked()
{
   TString name(fTxtName->GetText());
   if (name.CompareTo("Local", TString::kIgnoreCase) == 0) {
      Int_t retval;
      new TGMsgBox(fClient->GetRoot(), this, "Error Deleting Session",
                   "Deleting Local Sessions is not allowed !",
                   kMBIconExclamation,kMBOk,&retval);
   }
   TIter next(fViewer->GetSessions());
   TSessionDescription *desc = 0;
   while ((desc = (TSessionDescription *)next())) {
      if (desc->fName == name) {
         if (desc->fConnected)
            desc->fProof->Close();
         TString m;
         m.Form("Are you sure to delete the server \"%s\"",
                desc->fName.Data());
         Int_t result;
         new TGMsgBox(fClient->GetRoot(), this, "", m.Data(), 0,
                      kMBOk | kMBCancel, &result);

         // msgbox
         if (result == kMBOk) {
            fViewer->GetSessions()->Remove((TObject *)desc);
            WriteConfigFile(kPROOF_GuiConfFile, fViewer->GetSessions());
            fViewer->BuildSessionHierarchy(fViewer->GetSessions());
         }
         break;
      }
   }
}

//______________________________________________________________________________
void TSessionServerFrame::OnBtnConnectClicked()
{
   // bb test
   char url[128];
   fViewer->GetStatusBar()->GetBarPart(0)->ShowFrame(fViewer->GetConnectProg());
   TQObject::Connect("TProof", "StartupMessage(char *,Bool_t,Int_t,Int_t)",
         "TSessionViewer", fViewer, "StartupMessage(char *,Bool_t,Int_t,Int_t)");
   sprintf(url, "%s@%s", fTxtUsrName->GetText(), fTxtAddress->GetText());
   fViewer->GetActDesc()->fProof = gROOT->Proof(url,
            fViewer->GetActDesc()->fConfigFile, 0,
            fViewer->GetActDesc()->fLogLevel);
   if (fViewer->GetActDesc()->fProof &&
       fViewer->GetActDesc()->fProof->IsValid()) {
      fViewer->GetActDesc()->fProof->SetQueryType(fViewer->GetActDesc()->fSync ?
                             TVirtualProof::kSync : TVirtualProof::kAsync);
      fViewer->GetActDesc()->fConnected = kTRUE;
      TGListTreeItem *item = fViewer->GetSessionHierarchy()->FindChildByData(
                             fViewer->GetSessionItem(),fViewer->GetActDesc());
      item->SetPictures(fViewer->GetProofConPict(), fViewer->GetProofConPict());
      fViewer->OnListTreeClicked(item, 1, 0, 0);
      fClient->NeedRedraw(fViewer->GetSessionHierarchy());

      fViewer->GetActDesc()->fProof->Connect("Progress(Long64_t,Long64_t)",
                                 "TSessionFrame", fViewer->GetSessionFrame(),
                                 "Progress(Long64_t,Long64_t)");
      fViewer->GetActDesc()->fProof->Connect("StopProcess(Bool_t)",
                                 "TSessionFrame", fViewer->GetSessionFrame(),
                                 "IndicateStop(Bool_t)");
      fViewer->GetActDesc()->fProof->Connect(
                  "ResetProgressDialog(const char*,Int_t,Long64_t,Long64_t)",
                  "TSessionFrame", fViewer->GetSessionFrame(),
                  "ResetProgressDialog(const char*,Int_t,Long64_t,Long64_t)");

      fViewer->GetActDesc()->fProof->Connect("Feedback(TList*)",
             "TSessionFrame", fViewer->GetSessionFrame(), "Feedback(TList*)");

      fViewer->EnableTimer();
      fViewer->ChangeRightLogo("monitor01.xpm");
      fViewer->SetChangePic(kFALSE);
      fViewer->GetActDesc()->fProof->Connect("QueryResultReady(char *)",
                       "TSessionViewer", fViewer, "QueryResultReady(char *)");
      fViewer->GetStatusBar()->SetText("Proof Cluster Ready", 1);
   }
   fViewer->GetStatusBar()->GetBarPart(0)->HideFrame(fViewer->GetConnectProg());
}

//______________________________________________________________________________
void TSessionServerFrame::OnBtnNewServerClicked()
{
   fTxtName->SetText("");
   fTxtAddress->SetText("");
   fNumPort->SetIntNumber(1093);
   fLogLevel->SetIntNumber(0);
   fTxtUsrName->SetText("");
}

//______________________________________________________________________________
void TSessionServerFrame::OnBtnAddClicked()
{
   TSessionDescription* desc = new TSessionDescription();
   desc->fName = TString(fTxtName->GetText());
   desc->fAddress = TString(fTxtAddress->GetText());
   desc->fPort = fNumPort->GetIntNumber();
   desc->fConnected = kFALSE;
   if (strlen(fTxtConfig->GetText()) > 1)
      desc->fConfigFile = TString(fTxtConfig->GetText());
   else
      desc->fConfigFile = "";
   desc->fLogLevel = fLogLevel->GetIntNumber();
   desc->fUserName = TString(fTxtUsrName->GetText());
   desc->fSync = (fSync->GetState() == kButtonDown);
   fViewer->GetSessions()->Add((TObject *)desc);
   WriteConfigFile(kPROOF_GuiConfFile, fViewer->GetSessions());
   fViewer->BuildSessionHierarchy(fViewer->GetSessions());
}

//______________________________________________________________________________
void TSessionServerFrame::Update(TSessionDescription* desc)
{
   fTxtName->SetText(desc->fName);
   fTxtAddress->SetText(desc->fAddress);
   fNumPort->SetIntNumber(desc->fPort);
   fLogLevel->SetIntNumber(desc->fLogLevel);

   if (desc->fConfigFile.Length() > 1) {
      fTxtConfig->SetText(desc->fConfigFile);
   }
   else {
      fTxtConfig->SetText("");
   }
   fTxtUsrName->SetText(desc->fUserName);
}

//______________________________________________________________________________
Bool_t TSessionServerFrame::WriteConfigFile(const TString &filePath, TList *vec)
{
   char line[2048];
   char c = kPROOF_GuiConfFileSeparator;
   TString homefilePath(gSystem->UnixPathName(gSystem->HomeDirectory()));
   homefilePath.Append('/');
   homefilePath.Append(filePath);
   FILE* f = fopen(homefilePath.Data(), "w");
   if (!f) {
      Error("WriteConfigFile", "Cannot open the config file %s for writing",
            filePath.Data());
      return kFALSE;
   }
   TIter next(vec);
   TSessionDescription *desc = 0;
   while ((desc = (TSessionDescription *)next())) {
      sprintf(line, "%s%c%s%c%d%c%s%c", desc->fName.Data(), c,
              desc->fAddress.Data(), c, desc->fPort, c,
              desc->fConfigFile.Data(), c);
      sprintf(line, "%s%s%c%d%c%s%c%s%c%s%c%d",line, "loglevel", c,
              desc->fLogLevel, c, "user", c, desc->fUserName.Data(),
              c, "sync", c, desc->fSync);
      sprintf(line,"%s\n", line);
      if (fprintf(f, line) == 0) {
         Error("WriteConfigFile", "Error writing to the config file");
         fclose(f);
         return kFALSE;
      }
   }
   fclose(f);
   return kTRUE;
}

//______________________________________________________________________________
TList *TSessionServerFrame::ReadConfigFile(const TString &filePath)
{
   TList *vec = new TList;
   TString homefilePath(gSystem->UnixPathName(gSystem->HomeDirectory()));
   homefilePath.Append('/');
   homefilePath.Append(filePath);
   FILE* f = fopen(homefilePath.Data(), "r");
   if (!f) {
      Error("ReadConfigFile", "Cannot open the config file %s", filePath.Data());
      return vec;
   }
   char line[2048];
   while (fgets(line, sizeof(line), f)) {
      Int_t len = strlen(line);
      if (len > 0 && line[len-1] == '\n')
         line[len-- -1] = '\0';
      if (line[0] == '#' || line[0] == '\0')
         continue;         // skip comment and empty lines
      char* parts[10];
      Int_t noParts = 0;
      parts[noParts++] = line;
      for (int i = 0; i < len && noParts < 10; i++) {
         if (line[i] == kPROOF_GuiConfFileSeparator) {
            parts[noParts++] = &line[i + 1];
            line[i] = '\0';
         }
      }
      if (noParts < 8) {
         Error("ReadConfigFile", "PROOF Servers config file corrupted; skipping (1)\n");
         continue;
      }
      Int_t port, loglevel, sync;
      if (sscanf(parts[2], "%d", &port) != 1) {
         Error("ReadConfigFile", "PROOF Servers config file corrupted; skipping (2)\n");
         continue;
      }
      if (strcmp(parts[4], "loglevel") != 0) {
         Error("ReadConfigFile", "PROOF Servers config file corrupted; skipping (3)\n");
         continue;
      }
      if (sscanf(parts[5], "%d", &loglevel) != 1) {
         Error("ReadConfigFile", "PROOF Servers config file corrupted; skipping (4)\n");
         continue;
      }
      TSessionDescription *proofDesc = new TSessionDescription();
      proofDesc->fName = TString(parts[0]);
      proofDesc->fAddress = TString(parts[1]);
      proofDesc->fPort = port;
      proofDesc->fConfigFile = TString(parts[3]);
      proofDesc->fLogLevel = loglevel;
      proofDesc->fConnected = kFALSE;
      proofDesc->fLocal = kFALSE;
      if (strcmp(parts[8], "sync") != 0) {
         Error("ReadConfigFile", "PROOF Servers config file corrupted; skipping (5)\n");
         continue;
      }
      if (sscanf(parts[9], "%d", &sync) != 1) {
         Error("ReadConfigFile", "PROOF Servers config file corrupted; skipping (6)\n");
         continue;
      }
      proofDesc->fSync = (Bool_t)sync;
      if (strcmp(parts[6], "user") == 0) {
         if (noParts != 10) {
            Error("ReadConfigFile",  "PROOF Servers config file corrupted; skipping (7)\n");
            delete proofDesc;
            continue;
         }
         proofDesc->fUserName = TString(parts[7]);
      }
      else {
         Error("ReadConfigFile", "PROOF Servers config file corrupted; skipping (8)\n");
         delete proofDesc;
         continue;
      }
      vec->Add((TObject *)proofDesc);
   }
   fclose(f);
   return vec;
}

////////////////////////////////////////////////////////////////////////////////
// Session Frame

//______________________________________________________________________________
TSessionFrame::TSessionFrame(TGWindow* p, Int_t w, Int_t h) :
   TGCompositeFrame(p, w, h)
{
   // Constructor
}

//______________________________________________________________________________
TSessionFrame::~TSessionFrame()
{
   // Destructor
   Cleanup();
}

//______________________________________________________________________________
void TSessionFrame::Build(TSessionViewer *gui)
{
   SetLayoutManager(new TGVerticalLayout(this));
   SetCleanup(kDeepCleanup);
   fFirst = fEntries = fPrevTotal = 0;
   fStatus    = kRunning;
   fViewer  = gui;

   fTab = new TGTab(this, 200, 200);
   AddFrame(fTab, new TGLayoutHints(kLHintsTop | kLHintsExpandX |
            kLHintsExpandY, 2, 2, 2, 2));

   TGCompositeFrame *tf = fTab->AddTab("Status");
   fFA = new TGCompositeFrame(tf, 100, 100, kVerticalFrame);
   tf->AddFrame(fFA, new TGLayoutHints(kLHintsTop | kLHintsLeft |
                kLHintsExpandX | kLHintsExpandY));

   // Status
   TGCompositeFrame* frmStat = new TGHorizontalFrame(fFA, 350, 100);
   frmStat->SetCleanup(kDeepCleanup);
   frmStat->AddFrame(new TGLabel(frmStat, "Status :"),
         new TGLayoutHints(kLHintsLeft, 5, 5, 5, 0));
   fLabStatus = new TGLabel(frmStat, "");
   frmStat->AddFrame(fLabStatus,
         new TGLayoutHints(kLHintsLeft, 35, 5, 5, 0));
   fFA->AddFrame(frmStat,  new TGLayoutHints(kLHintsExpandX, 0, 0, 5, 0));

   //progress bar
   frmProg = new TGHProgressBar(fFA, TGProgressBar::kFancy, 350 - 20);
   frmProg->ShowPosition();
   frmProg->SetBarColor("green");
   fFA->AddFrame(frmProg, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

   fFA->AddFrame(fProcessed = new TGLabel(fFA, " Estimated time left : "),
            new TGLayoutHints(kLHintsLeft, 5, 5, 5, 5));
   fFA->AddFrame(fTotal = new TGLabel(fFA, " 00:00:00 (--- events of --- processed) "),
            new TGLayoutHints(kLHintsLeft, 5, 5, 5, 5));
   fFA->AddFrame(fRate = new TGLabel(fFA, " -- events/sec "),
            new TGLayoutHints(kLHintsLeft, 5, 5, 5, 5));

   TGCompositeFrame* frmBut0 = new TGHorizontalFrame(fFA, 350, 100);
   frmBut0->SetCleanup(kDeepCleanup);

   frmBut0->AddFrame(fBtnDisconnect = new TGTextButton(frmBut0,
      " Disconnect "),new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5));
   fBtnShowLog = new TGTextButton(frmBut0, "Show log...");
   frmBut0->AddFrame(fBtnShowLog, new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5));
   fFA->AddFrame(frmBut0, new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 0, 0, 45, 0));

   //Abort Disconnect
   TGCompositeFrame* frmBut1 = new TGHorizontalFrame(fFA, 350, 100);
   frmBut1->SetCleanup(kDeepCleanup);
   frmBut1->AddFrame(fBtnNewQuery = new TGTextButton(frmBut1, "New Query..."),
      new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5));
   frmBut1->AddFrame(fBtnGetQueries = new TGTextButton(frmBut1, " Get Queries  "),
       new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5));
   fFA->AddFrame(frmBut1, new TGLayoutHints(kLHintsLeft | kLHintsExpandX));

   // REsults URL + Update
   TGCompositeFrame* frmRes = new TGHorizontalFrame(fFA, 350, 100);
   frmRes->SetCleanup(kDeepCleanup);
   frmRes->AddFrame(new TGLabel(frmRes, "Results URL :"),
                    new TGLayoutHints(kLHintsLeft, 5, 5, 5, 5));
   fTexBufResultsURL = new TGTextBuffer(20);
   frmRes->AddFrame(fTexEntResultsURL = new TGTextEntry(frmRes,
      fTexBufResultsURL ),new TGLayoutHints(kLHintsRight |
      kLHintsExpandX, 5, 5, 5, 5));
   fFA->AddFrame(frmRes, new TGLayoutHints(kLHintsExpandX));

   tf = fTab->AddTab("Feedback");
   fFB = new TGCompositeFrame(tf, 100, 100, kVerticalFrame);
   tf->AddFrame(fFB, new TGLayoutHints(kLHintsTop | kLHintsLeft |
                kLHintsExpandX | kLHintsExpandY));

   fFeedbackFrame = new TSessionFeedbackFrame(fFB, 350, 310);
   fFeedbackFrame->Build(fViewer);
   fFB->AddFrame(fFeedbackFrame, new TGLayoutHints(kLHintsExpandX,
                 0, 0, 0, 0));

   //connecting button actions to functions
   fBtnDisconnect->Connect("Clicked()", "TSessionFrame", this,
                           "OnBtnDisconnectClicked()");
   fBtnShowLog->Connect("Clicked()", "TSessionFrame", this,
                        "OnBtnShowLogClicked()");
   fBtnNewQuery->Connect("Clicked()", "TSessionFrame", this,
                         "OnBtnNewQueryClicked()");
   fBtnGetQueries->Connect("Clicked()", "TSessionFrame", this,
                           "OnBtnGetQueriesClicked()");
}


//______________________________________________________________________________
void TSessionFrame::Feedback(TList *objs)
{
   if (objs && objs->GetEntries()) {
      TObject *obj;
      TIter next(objs);
      while ((obj = next())) {
         fFeedbackFrame->AddToFeedback(obj);
      }
   }
}

//______________________________________________________________________________
void TSessionFrame::Progress(Long64_t total, Long64_t processed)
{
   // Update progress bar and status labels.

   static const char *cproc[] = { "running", "done", "STOPPED", "ABORTED" };

   if (total < 0)
      total = fPrevTotal;
   else
      fPrevTotal = total;

   char buf[256];
   if (fEntries != total) {
      fEntries = total;
      sprintf(buf, " %d files, number of events %lld, starting event %lld",
              fFiles, fEntries, fFirst);
      fLabStatus->SetText(buf);
   }

   Float_t pos = Float_t(Double_t(processed * 100)/Double_t(total));
   frmProg->SetPosition(pos);
   if (pos >= 100.0) {
      fViewer->SetChangePic(kFALSE);
      fViewer->ChangeRightLogo("monitor01.xpm");
   }

   // get current time
   fEndTime = gSystem->Now();
   TTime tdiff = fEndTime - fStartTime;
   Float_t eta = 0;
   if (processed)
      eta = ((Float_t)((Long_t)tdiff)*total/Float_t(processed) -
            Long_t(tdiff))/1000.;

   if (processed == total) {
      fProcessed->SetText(" Processed:");
      sprintf(buf, " %lld events in %.1f sec", total, Long_t(tdiff)/1000.);
      fTotal->SetText(buf);
   } else {
      if (fStatus > kDone) {
         sprintf(buf, " %.1f sec (%lld events of %lld processed) - %s",
                      eta, processed, total, cproc[fStatus]);
      } else {
         sprintf(buf, " %.1f sec (%lld events of %lld processed)",
                      eta, processed, total);
      }
      fTotal->SetText(buf);
      sprintf(buf, " %.1f events/sec", Float_t(processed)/Long_t(tdiff)*1000.);
      fRate->SetText(buf);
   }
   Layout();
}

//______________________________________________________________________________
void TSessionFrame::IndicateStop(Bool_t aborted)
{
   // Indicate that Cancel or Stop was clicked.

   if (aborted == kTRUE) {
      frmProg->SetBarColor("red");
      fStatus = kAborted;
   }
   else {
      frmProg->SetBarColor("yellow");
      fStatus = kStopped;
   }

   if (fViewer->GetActDesc()->fProof &&
       fViewer->GetActDesc()->fProof->IsValid()) {
      fViewer->GetActDesc()->fProof->Disconnect("Progress(Long64_t,Long64_t)",
                                          this, "Progress(Long64_t,Long64_t)");
      fViewer->GetActDesc()->fProof->Disconnect("StopProcess(Bool_t)", this,
                                                "IndicateStop(Bool_t)");
   }
}

//______________________________________________________________________________
void TSessionFrame::ResetProgressDialog(const char *, Int_t, Long64_t, Long64_t)
{
   fPrevTotal     = 0;

   frmProg->SetBarColor("green");
   frmProg->Reset();

   // Reconnect the slots
   if (fViewer->GetActDesc()->fProof &&
       fViewer->GetActDesc()->fProof->IsValid()) {
      fViewer->GetActDesc()->fProof->Connect("Progress(Long64_t,Long64_t)",
                     "TSessionFrame", this, "Progress(Long64_t,Long64_t)");
      fViewer->GetActDesc()->fProof->Connect("StopProcess(Bool_t)",
                     "TSessionFrame", this, "IndicateStop(Bool_t)");
   }
}


//______________________________________________________________________________
void TSessionFrame::OnBtnDisconnectClicked()
{
   if (fViewer->GetActDesc()->fLocal) return;
   if (fViewer->GetActDesc()->fProof &&
       fViewer->GetActDesc()->fProof->IsValid())
      fViewer->GetActDesc()->fProof->Close();
   fViewer->GetActDesc()->fConnected = kFALSE;
   fViewer->DisableTimer();
   TGListTreeItem *item = fViewer->GetSessionHierarchy()->FindChildByData(
                          fViewer->GetSessionItem(), fViewer->GetActDesc());
   item->SetPictures(fViewer->GetProofDisconPict(),
                     fViewer->GetProofDisconPict());
   fViewer->OnListTreeClicked(fViewer->GetSessionItem(), 1, 0, 0);
   fClient->NeedRedraw(fViewer->GetSessionHierarchy());
   fViewer->GetStatusBar()->SetText("", 1);
}

//______________________________________________________________________________
void TSessionFrame::OnBtnShowLogClicked()
{
   fViewer->ShowLog(0);
}

//______________________________________________________________________________
void TSessionFrame::OnBtnNewQueryClicked()
{
   TNewQueryDlg *dlg = new TNewQueryDlg(fViewer, 350, 310);
   dlg->Popup();
}

//______________________________________________________________________________
void TSessionFrame::OnBtnGetQueriesClicked()
{
   TList *lqueries = 0;
   TQueryResult *query;
   TQueryDescription *newquery;
   if (fViewer->GetActDesc()->fProof &&
       fViewer->GetActDesc()->fProof->IsValid()) {
      lqueries = fViewer->GetActDesc()->fProof->GetListOfQueries("A");
   }
   if (lqueries) {
      TIter nextp(lqueries);

      while ((query = (TQueryResult *)nextp())) {
         newquery = new TQueryDescription();
         newquery->fReference       = Form("%s:%s", query->GetTitle(),
                                      query->GetName());
         TGListTreeItem *item =
            fViewer->GetSessionHierarchy()->FindChildByData(
                     fViewer->GetSessionItem(), fViewer->GetActDesc());
         if (fViewer->GetSessionHierarchy()->FindChildByName(item,
            newquery->fReference.Data()))
            continue;
         newquery->fStatus = query->IsFinalized() ?
               TQueryDescription::kSessionQueryFinalized :
               (TQueryDescription::ESessionQueryStatus)query->GetStatus();
         newquery->fSelectorString  = query->GetSelecImp()->GetName();
         newquery->fQueryName       = Form("%s:%s", query->GetTitle(),
                                           query->GetName());
         newquery->fOptions         = query->GetOptions();
         newquery->fEventList       = "";
         newquery->fParFile         = "";
         newquery->fNoEntries       = query->GetEntries();
         newquery->fFirstEntry      = query->GetFirst();
         newquery->fResult          = query;
         fViewer->GetActDesc()->fQueries->Add((TObject *)newquery);
         TGListTreeItem *item2 = fViewer->GetSessionHierarchy()->AddItem(item,
                  newquery->fQueryName, fViewer->GetQueryConPict(),
                  fViewer->GetQueryConPict());
         item2->SetUserData(newquery);
         if (query->GetInputList())
            fViewer->GetSessionHierarchy()->AddItem(item2, "InputList");
         if (query->GetInputList())
            fViewer->GetSessionHierarchy()->AddItem(item2, "OutputList");
      }
   }
   fClient->NeedRedraw(fViewer->GetSessionHierarchy());
}


////////////////////////////////////////////////////////////////////////////////
// Query Frame

//______________________________________________________________________________
TSessionQueryFrame::TSessionQueryFrame(TGWindow* p, Int_t w, Int_t h) :
   TGCompositeFrame(p, w, h)
{
   // Constructor
}

//______________________________________________________________________________
TSessionQueryFrame::~TSessionQueryFrame()
{
   // Destructor
   Cleanup();
}

//______________________________________________________________________________
void TSessionQueryFrame::Build(TSessionViewer *gui)
{
   SetLayoutManager(new TGVerticalLayout(this));
   SetCleanup(kDeepCleanup);
   fViewer  = gui;

   fTab = new TGTab(this, 200, 200);
   AddFrame(fTab, new TGLayoutHints(kLHintsTop | kLHintsExpandX |
            kLHintsExpandY, 2, 2, 2, 2));

   TGCompositeFrame *tf = fTab->AddTab("Session Infos");
   fFA = new TGCompositeFrame(tf, 100, 100, kVerticalFrame);
   tf->AddFrame(fFA, new TGLayoutHints(kLHintsTop | kLHintsLeft |
                kLHintsExpandX | kLHintsExpandY));
   fFA->SetLayoutManager(new TGTableLayout(fFA, 6, 2));

   fInfoTextView = new TGTextView(fFA, 330, 150, "", kSunkenFrame |
                                  kDoubleBorder);
   fFA->AddFrame(fInfoTextView, new TGTableLayoutHints(0, 2, 0, 1,
                 kLHintsCenterY | kLHintsExpandX | kLHintsShrinkX |
                 kLHintsFillX, 5, 5, 2, 2));

   fBtnSubmit = new TGTextButton(fFA, "Submit");
   fFA->AddFrame(fBtnSubmit,new TGTableLayoutHints(0, 1, 1, 2,
            kLHintsCenterY | kLHintsExpandX | kLHintsShrinkX |
            kLHintsFillX, 5, 5, 3, 3));
   fBtnFinalize = new TGTextButton(fFA, "Finalize");
   fFA->AddFrame(fBtnFinalize,new TGTableLayoutHints(1, 2, 1, 2,
            kLHintsCenterY | kLHintsExpandX | kLHintsShrinkX |
            kLHintsFillX, 5, 5, 3, 3));
   fBtnStop = new TGTextButton(fFA, "Stop");
   fFA->AddFrame(fBtnStop,new TGTableLayoutHints(0, 1, 2, 3,
            kLHintsCenterY | kLHintsExpandX | kLHintsShrinkX |
            kLHintsFillX, 5, 5, 3, 3));
   fBtnAbort = new TGTextButton(fFA, "Abort");
   fFA->AddFrame(fBtnAbort,new TGTableLayoutHints(1, 2, 2, 3,
            kLHintsCenterY | kLHintsExpandX | kLHintsShrinkX |
            kLHintsFillX, 5, 5, 3, 3));
   fBtnShowLog = new TGTextButton(fFA, "Show Log");
   fFA->AddFrame(fBtnShowLog,new TGTableLayoutHints(0, 1, 3, 4,
            kLHintsCenterY | kLHintsExpandX | kLHintsShrinkX |
            kLHintsFillX, 5, 5, 3, 3));
   fBtnRetrieve = new TGTextButton(fFA, "Retrieve");
   fFA->AddFrame(fBtnRetrieve,new TGTableLayoutHints(1, 2, 3, 4,
            kLHintsCenterY | kLHintsExpandX | kLHintsShrinkX |
            kLHintsFillX, 5, 5, 3, 3));

   TGCompositeFrame* frmRes = new TGHorizontalFrame(fFA, 350, 100);
   frmRes->SetCleanup(kDeepCleanup);
   frmRes->AddFrame(new TGLabel(frmRes, "Results URL :"),
                    new TGLayoutHints(kLHintsLeft | kLHintsExpandX |
                    kLHintsShrinkX | kLHintsCenterY,
                    5, 5, 3, 3));
   frmRes->AddFrame(fTexEntResultsURL = new TGTextEntry(frmRes),
                    new TGLayoutHints(kLHintsRight | kLHintsExpandX |
                    kLHintsShrinkX | kLHintsCenterY |
                    kLHintsExpandX, 5, 5, 3, 3));
   fFA->AddFrame(frmRes, new TGTableLayoutHints(0, 2, 5, 6,
                 kLHintsCenterY | kLHintsExpandX | kLHintsShrinkX |
                 kLHintsFillX, 0, 0, 2, 5));

   tf = fTab->AddTab("Statistics");
   fFB = new TGCompositeFrame(tf, 300, 150, kVerticalFrame);
   tf->AddFrame(fFB, new TGLayoutHints(kLHintsTop | kLHintsLeft |
                kLHintsExpandX | kLHintsExpandY));
   fFB->SetLayoutManager(new TGTableLayout(fFB, 4, 4));

   fECanvas = new TRootEmbeddedCanvas("fECanvas", fFB, 400, 150);
   fFB->AddFrame(fECanvas, new TGTableLayoutHints(0, 4, 0, 2,
            kLHintsFillX | kLHintsTop | kLHintsExpandX | kLHintsExpandY |
            kLHintsShrinkX | kLHintsShrinkY, 5, 5, 5, 5));
   fECanvas->GetCanvas()->SetBorderMode(0);

   fBtnSubmit->Connect("Clicked()", "TSessionQueryFrame", this,
                       "OnBtnSubmit()");
   fBtnFinalize->Connect("Clicked()", "TSessionQueryFrame", this,
                         "OnBtnFinalize()");
   fBtnStop->Connect("Clicked()", "TSessionQueryFrame", this,
                     "OnBtnStop()");
   fBtnAbort->Connect("Clicked()", "TSessionQueryFrame", this,
                      "OnBtnAbort()");
   fBtnShowLog->Connect("Clicked()", "TSessionQueryFrame", this,
                        "OnBtnShowLog()");
   fBtnRetrieve->Connect("Clicked()", "TSessionQueryFrame", this,
                         "OnBtnRetrieve()");

   Resize(350, 310);
}

//______________________________________________________________________________
void TSessionQueryFrame::OnBtnFinalize()
{
   if (fViewer->GetActDesc()->fProof &&
       fViewer->GetActDesc()->fProof->IsValid()) {
      gPad->SetEditable(kFALSE);
      TGListTreeItem *item = fViewer->GetSessionHierarchy()->GetSelected();
      TQueryDescription *query = (TQueryDescription *)item->GetUserData();
      fViewer->GetActDesc()->fProof->Finalize(query->fReference);
      UpdateButtons(query);
   }
}

//______________________________________________________________________________
void TSessionQueryFrame::OnBtnStop()
{
   if (fViewer->GetActDesc()->fProof &&
       fViewer->GetActDesc()->fProof->IsValid()) {
      fViewer->GetActDesc()->fProof->StopProcess(kFALSE);
   }
   fViewer->ChangeRightLogo("monitor01.xpm");
   fViewer->SetChangePic(kFALSE);
}

//______________________________________________________________________________
void TSessionQueryFrame::OnBtnShowLog()
{
   TGListTreeItem *item = fViewer->GetSessionHierarchy()->GetSelected();
   TQueryDescription *query = (TQueryDescription *)item->GetUserData();
   fViewer->ShowLog(query->fReference.Data());
}

//______________________________________________________________________________
void TSessionQueryFrame::OnBtnRetrieve()
{
   if (fViewer->GetActDesc()->fProof &&
       fViewer->GetActDesc()->fProof->IsValid()) {
      TGListTreeItem *item = fViewer->GetSessionHierarchy()->GetSelected();
      TQueryDescription *query = (TQueryDescription *)item->GetUserData();
      fViewer->GetActDesc()->fProof->Retrieve(query->fReference);
   }
}

//______________________________________________________________________________
void TSessionQueryFrame::OnBtnAbort()
{
   if (fViewer->GetActDesc()->fProof &&
       fViewer->GetActDesc()->fProof->IsValid()) {
      fViewer->GetActDesc()->fProof->StopProcess(kTRUE);
   }
   fViewer->ChangeRightLogo("monitor01.xpm");
   fViewer->SetChangePic(kFALSE);
}

//______________________________________________________________________________
void TSessionQueryFrame::OnBtnSubmit()
{
   Long64_t id = 0;
   TGListTreeItem *item = fViewer->GetSessionHierarchy()->GetSelected();
   TQueryDescription *newquery = (TQueryDescription *)item->GetUserData();
   fViewer->GetSessionFrame()->SetStartTime(gSystem->Now());

   if (fViewer->GetActDesc()->fProof &&
       fViewer->GetActDesc()->fProof->IsValid()) {

      fViewer->GetActDesc()->fProof->cd();
      if (newquery->fChain) {
         if (newquery->fChain->IsA() == TChain::Class()) {
            newquery->fStatus = TQueryDescription::kSessionQuerySubmitted;
            ((TChain *)newquery->fChain)->SetProof(fViewer->GetActDesc()->fProof);
            id = ((TChain *)newquery->fChain)->Process(newquery->fSelectorString,
                    newquery->fOptions,
                    newquery->fNoEntries > 0 ? newquery->fNoEntries : 1234567890,
                    newquery->fFirstEntry);
         }
         else if (newquery->fChain->IsA() == TDSet::Class()) {
            id = ((TDSet *)newquery->fChain)->Process(newquery->fSelectorString,
                    newquery->fOptions,
                    newquery->fNoEntries,
                    newquery->fFirstEntry);
         }
         fViewer->GetQueryFrame()->UpdateButtons(newquery);
      }
      newquery->fReference= Form("session-%s:q%d",
            fViewer->GetActDesc()->fProof->GetSessionTag(), id);

      fViewer->SetChangePic(kTRUE);
   }
   else if (fViewer->GetActDesc()->fLocal){
      if (newquery->fChain) {
         if (newquery->fChain->IsA() == TChain::Class()) {
            id = ((TChain *)newquery->fChain)->Process(newquery->fSelectorString,
                            newquery->fOptions,
                            newquery->fNoEntries > 0 ? newquery->fNoEntries : 1234567890,
                            newquery->fFirstEntry);
         }
         else if (newquery->fChain->IsA() == TDSet::Class()) {
            id = ((TDSet *)newquery->fChain)->Process(newquery->fSelectorString,
                                                      newquery->fOptions,
                                                      newquery->fNoEntries,
                                                      newquery->fFirstEntry);
         }
      }
      newquery->fReference = Form("local-session-%s:q%d", newquery->fQueryName.Data(), id);
   }
}

//______________________________________________________________________________
void TSessionQueryFrame::UpdateButtons(TQueryDescription *desc)
{
   // Update buttons state for the current query status

   TGListTreeItem *item = fViewer->GetSessionHierarchy()->GetSelected();
   TQueryDescription *query = (TQueryDescription *)item->GetUserData();
   if (desc != query) return;
   switch (desc->fStatus) {

      case TQueryDescription::kSessionQueryFromProof:
         fBtnSubmit->SetEnabled(kFALSE);
         fBtnFinalize->SetEnabled(kTRUE);
         fBtnStop->SetEnabled(kFALSE);
         fBtnAbort->SetEnabled(kFALSE);
         fBtnShowLog->SetEnabled(kTRUE);
         fBtnRetrieve->SetEnabled(kTRUE);
         break;

      case TQueryDescription::kSessionQueryCompleted:
         fBtnSubmit->SetEnabled(kFALSE);
         fBtnFinalize->SetEnabled(kTRUE);
         if (desc->fResult && desc->fResult->IsFinalized())
            fBtnFinalize->SetEnabled(kFALSE);
         fBtnStop->SetEnabled(kFALSE);
         fBtnAbort->SetEnabled(kFALSE);
         fBtnShowLog->SetEnabled(kTRUE);
         fBtnRetrieve->SetEnabled(kTRUE);
         break;

      case TQueryDescription::kSessionQueryCreated:
         fBtnSubmit->SetEnabled(kTRUE);
         fBtnFinalize->SetEnabled(kFALSE);
         fBtnStop->SetEnabled(kFALSE);
         fBtnAbort->SetEnabled(kFALSE);
         fBtnShowLog->SetEnabled(kTRUE);
         fBtnRetrieve->SetEnabled(kFALSE);
         break;

      case TQueryDescription::kSessionQuerySubmitted:
         fBtnSubmit->SetEnabled(kFALSE);
         fBtnFinalize->SetEnabled(kFALSE);
         fBtnStop->SetEnabled(kFALSE);
         fBtnAbort->SetEnabled(kFALSE);
         fBtnShowLog->SetEnabled(kTRUE);
         fBtnRetrieve->SetEnabled(kFALSE);
         break;

      case TQueryDescription::kSessionQueryRunning:
         fBtnSubmit->SetEnabled(kFALSE);
         fBtnFinalize->SetEnabled(kFALSE);
         fBtnStop->SetEnabled(kTRUE);
         fBtnAbort->SetEnabled(kTRUE);
         fBtnShowLog->SetEnabled(kTRUE);
         fBtnRetrieve->SetEnabled(kFALSE);
         break;

      case TQueryDescription::kSessionQueryStopped:
         fBtnSubmit->SetEnabled(kFALSE);
         fBtnFinalize->SetEnabled(kTRUE);
         fBtnStop->SetEnabled(kFALSE);
         fBtnAbort->SetEnabled(kFALSE);
         fBtnShowLog->SetEnabled(kTRUE);
         fBtnRetrieve->SetEnabled(kFALSE);
         break;

      case TQueryDescription::kSessionQueryAborted:
         fBtnSubmit->SetEnabled(kTRUE);
         fBtnFinalize->SetEnabled(kFALSE);
         fBtnStop->SetEnabled(kFALSE);
         fBtnAbort->SetEnabled(kFALSE);
         fBtnShowLog->SetEnabled(kTRUE);
         fBtnRetrieve->SetEnabled(kFALSE);
         break;

      case TQueryDescription::kSessionQueryFinalized:
         fBtnSubmit->SetEnabled(kFALSE);
         fBtnFinalize->SetEnabled(kFALSE);
         fBtnStop->SetEnabled(kFALSE);
         fBtnAbort->SetEnabled(kFALSE);
         fBtnShowLog->SetEnabled(kTRUE);
         fBtnRetrieve->SetEnabled(kFALSE);
         break;

      default:
         break;
   }
}

//______________________________________________________________________________
void TSessionQueryFrame::UpdateInfos()
{
   char buffer[8192];
   const char *qst[] = {"aborted  ", "submitted", "running  ",
                        "stopped  ", "completed"};

   fInfoTextView->Clear();
   if (!fViewer->GetActDesc()->fActQuery ||
       !fViewer->GetActDesc()->fActQuery->fResult) {
      return;
   }
   TQueryResult *result = fViewer->GetActDesc()->fActQuery->fResult;

   // Status label
   Int_t st = (result->GetStatus() > 0 && result->GetStatus() <=
               TQueryResult::kCompleted) ? result->GetStatus() : 0;

   Int_t qry = result->GetSeqNum();

   sprintf(buffer,"------------------------------------------------------\n");
   // Print header
   if (!result->IsDraw()) {
      const char *fin = result->IsFinalized() ? "finalized" : qst[st];
      const char *arc = result->IsArchived() ? "(A)" : "";
      sprintf(buffer,"%s Query No  : %d\n", buffer, qry);
      sprintf(buffer,"%s Ref       : \"%s:%s\"\n", buffer, result->GetTitle(),
              result->GetName());
      sprintf(buffer,"%s Selector  : %s\n", buffer,
              result->GetSelecImp()->GetTitle());
      sprintf(buffer,"%s Status    : %9s%s\n", buffer, fin, arc);
      sprintf(buffer,"%s------------------------------------------------------\n",
              buffer);
   } else {
      sprintf(buffer,"%s Query No  : %d\n", buffer, qry);
      sprintf(buffer,"%s Ref       : \"%s:%s\"\n", buffer, result->GetTitle(),
              result->GetName());
      sprintf(buffer,"%s Selector  : %s\n", buffer,
              result->GetSelecImp()->GetTitle());
      sprintf(buffer,"%s------------------------------------------------------\n",
              buffer);
   }

   // Time information
   Int_t elapsed = (Int_t)(result->GetEndTime().Convert() -
                           result->GetStartTime().Convert());
   sprintf(buffer,"%s Started   : %s\n",buffer,
           result->GetStartTime().AsString());
   sprintf(buffer,"%s Real time : %d sec (CPU time: %.1f sec)\n", buffer, elapsed,
           result->GetUsedCPU());

   // Number of events processed, rate, size
   Double_t rate = 0.0;
   if (result->GetEntries() > -1 && elapsed > 0)
      rate = result->GetEntries() / (Double_t)elapsed ;
   Float_t size = ((Float_t)result->GetBytes())/(1024*1024);
   sprintf(buffer,"%s Processed : %lld events (size: %.3f MBs)\n",buffer,
          result->GetEntries(), size);
   sprintf(buffer,"%s Rate      : %.1f evts/sec\n",buffer, rate);

   // Package information
   if (strlen(result->GetParList()) > 1) {
      sprintf(buffer,"%s Packages  :  %s\n",buffer, result->GetParList());
   }

   // Result information
   TString res = result->GetResultFile();
   if (!result->IsArchived()) {
      Int_t dq = res.Index("queries");
      if (dq > -1) {
         res.Remove(0,res.Index("queries"));
         res.Insert(0,"<PROOF_SandBox>/");
      }
      if (res.BeginsWith("-")) {
         res = (result->GetStatus() == TQueryResult::kAborted) ?
               "not available" : "sent to client";
      }
   }
   if (res.Length() > 1) {
      sprintf(buffer,"%s------------------------------------------------------\n",
              buffer);
      sprintf(buffer,"%s Results   : %s\n",buffer, res.Data());
   }

   if (result->GetOutputList() && result->GetOutputList()->GetSize() > 0) {
      sprintf(buffer,"%s Outlist   : %d objects\n",buffer,
              result->GetOutputList()->GetSize());
      sprintf(buffer,"%s------------------------------------------------------\n",
              buffer);
   }
   fInfoTextView->LoadBuffer(buffer);
}

////////////////////////////////////////////////////////////////////////////////
// Feedback Frame

//______________________________________________________________________________
TSessionFeedbackFrame::TSessionFeedbackFrame(TGWindow *p, Int_t w, Int_t h) :
   TGCompositeFrame(p, w, h), fLVContainer(0), fPopupMenu(0)
{
   // Constructor
}

//____________________________________________________________________________
TSessionFeedbackFrame::~TSessionFeedbackFrame()
{
   // Destructor
   delete fLVContainer; // this container is inside the TGListView and is not
                        // deleted automatically
   Cleanup();
}

//______________________________________________________________________________
void TSessionFeedbackFrame::Build(TSessionViewer *gui)
{

   fViewer = gui;
   SetCleanup(kDeepCleanup);
   SetLayoutManager(new TGVerticalLayout(this));
   // Container of object TGListView
   fListView = new TGListView(this, 340, 190);
   fLVContainer = new TGLVContainer(fListView, kSunkenFrame, GetWhitePixel());
   fLVContainer->Associate(fListView);
   fLVContainer->SetCleanup(kDeepCleanup);
   AddFrame(fListView, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,
            4, 4, 4, 4));

   fListView->Connect("Clicked(TGLVEntry*, Int_t, Int_t, Int_t)",
      "TSessionFeedbackFrame", this,
      "OnElementClicked(TGLVEntry*, Int_t, Int_t, Int_t)");

    //Adding object

   TGCompositeFrame* frmAdd = new TGHorizontalFrame(this, 350);
   frmAdd->SetCleanup(kDeepCleanup);

   frmAdd->AddFrame(new TGLabel(frmAdd, "Add object"),
      new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 2, 2));

   fTBAdd = new TGTextBuffer(20);
   fTexEntAdd = new TGTextEntry(frmAdd, fTBAdd);
   fTexEntAdd->Resize(85, fTexEntAdd->GetDefaultHeight());
   frmAdd->AddFrame(fTexEntAdd, new TGLayoutHints(kLHintsLeft | kLHintsExpandX,
                    25, 12, 2, 2));

   TGVerticalFrame *vbf = new TGVerticalFrame(frmAdd, 10, 10, kFixedWidth);
   vbf->SetCleanup(kDeepCleanup);
   fBtnAdd = new TGTextButton(vbf, new TGHotString("&Add"));
   vbf->AddFrame(fBtnAdd, new TGLayoutHints(kLHintsLeft | kLHintsExpandX,
                 5, 2, 2, 2));
   vbf->Resize(85, fBtnAdd->GetDefaultHeight());
   frmAdd->AddFrame(vbf, new TGLayoutHints(kLHintsRight | kLHintsCenterY));

   fBtnAdd->Connect("Clicked()", "TSessionFeedbackFrame", this,
                    "OnBtnAddClicked()");

   AddFrame(frmAdd, new TGLayoutHints(kLHintsExpandX, 4, 4, 4, 4));

   //Feedback

   TGCompositeFrame* frmFeed = new TGHorizontalFrame(this, 350, 100);
   frmFeed->SetCleanup(kDeepCleanup);
   TGCheckButton *chbtn = new TGCheckButton(frmFeed, "Feedback", 1);
   chbtn->SetState(kButtonDown);
   chbtn->Connect("Pressed()", "TSessionFeedbackFrame", this,
                  "OnChBtnPressed()");
   chbtn->Connect("Released()", "TSessionFeedbackFrame", this,
                  "OnChBtnReleased()");

   frmFeed->AddFrame(chbtn, new TGLayoutHints(kLHintsLeft | kLHintsCenterY,
                     5, 5, 2, 2));

   fLabFrequency = new TGLabel(frmFeed, "Frequency [s]: ");
   frmFeed->AddFrame(fLabFrequency, new TGLayoutHints(kLHintsLeft |
                     kLHintsCenterY, 5, 10, 2, 2));
   fNumEntFrequency = new TGNumberEntry(frmFeed, 1, 5, 5,
      TGNumberFormat::kNESInteger, TGNumberFormat::kNEAPositive,
      TGNumberFormat::kNELNoLimits );

   frmFeed->AddFrame(fNumEntFrequency,new TGLayoutHints(kLHintsLeft |
               kLHintsCenterY, 5, 10, 2, 2));

   vbf = new TGVerticalFrame(frmFeed, 10, 10, kFixedWidth);
   vbf->SetCleanup(kDeepCleanup);
   fBtnSet = new TGTextButton(vbf, new TGHotString("&Set"));
   vbf->AddFrame(fBtnSet, new TGLayoutHints(kLHintsLeft | kLHintsExpandX,
               5, 2, 2, 2));
   vbf->Resize(85, fBtnAdd->GetDefaultHeight());
   frmFeed->AddFrame(vbf, new TGLayoutHints(kLHintsRight | kLHintsCenterY));

   AddFrame(frmFeed, new TGLayoutHints(kLHintsExpandX, 4, 4, 4, 4));

   fPopupMenu = new TGPopupMenu(fClient->GetRoot());
   fPopupMenu->AddEntry("Remove",kMenuRemoveFromFeedback);
   fPopupMenu->Connect("Activated(Int_t)","TSessionFeedbackFrame", this,
               "MyHandleMenu(Int_t)");
}

//______________________________________________________________________________
void TSessionFeedbackFrame::OnBtnAddClicked()
{
   //adding element to the container
   if (!fTBAdd->GetTextLength())
      return;
   TGLVEntry* entry = new TGLVEntry(fLVContainer, fTBAdd->GetString(),
            fTBAdd->GetString());
   fLVContainer->AddItem(entry);
   Layout();
   //updating the view
   //clearing the input field
   fTexEntAdd->Clear();
   fTBAdd->Clear();
   fClient->NeedRedraw(fTexEntAdd);
   fClient->NeedRedraw(fLVContainer);
}

//______________________________________________________________________________
void TSessionFeedbackFrame::OnChBtnPressed()
{
   fBtnSet->SetEnabled(kTRUE);
   fNumEntFrequency->SetState(kTRUE);
   fLabFrequency->Enable();
}

//______________________________________________________________________________
void TSessionFeedbackFrame::OnChBtnReleased()
{
   fBtnSet->SetEnabled(kFALSE);
   fNumEntFrequency->SetState(kFALSE);
   fLabFrequency->Disable();
}

//______________________________________________________________________________
void TSessionFeedbackFrame::OnElementClicked(TGLVEntry* entry, Int_t btn,
                                             Int_t x, Int_t y)
{
   if (btn == 3) { //right button
      fPopupMenu->PlaceMenu(x, y, 1, 1);
      fDelEntry = entry;
   }
}

//______________________________________________________________________________
void TSessionFeedbackFrame::MyHandleMenu(Int_t id)
{
   switch(id) {

   case kMenuRemoveFromFeedback :
      if (fDelEntry) {
         fLVContainer->RemoveItem(fDelEntry);
         ShowFrame(fListView);
      }
      break;
   default:
      Error("MyHandleMenu", "Unknown menu option");
      break;
   }
   fClient->NeedRedraw(fLVContainer);
}

//______________________________________________________________________________
void TSessionFeedbackFrame::AddToFeedback(TGLVEntry* entry)
{
   if (entry) {
      //because copy constructor does not work/exist we do it this way:
      TGLVEntry* ent = new TGLVEntry(fLVContainer, entry->GetTitle(), "Feedback");
      if (!fLVContainer->FindFrameByName(entry->GetTitle())) {
         fLVContainer->AddItem(ent);
      }
      else {
         delete ent;
      }
   }
   fClient->NeedRedraw(fLVContainer);
}

//______________________________________________________________________________
void TSessionFeedbackFrame::AddToFeedback(TObject* obj)
{
   TGLVEntry *entry;
   if (obj) {
      entry = new TGLVEntry(fLVContainer, obj->GetName(), obj->GetName());
      entry->SetTitle(obj->GetName());
      AddToFeedback(entry);
   }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Output frame

//______________________________________________________________________________
TSessionOutputFrame::TSessionOutputFrame(TGWindow* p, Int_t w, Int_t h) :
   TGCompositeFrame(p, w, h), fLVContainer(0), fPopupMenu(0)
{
   // constructor
}

//______________________________________________________________________________
TSessionOutputFrame::~TSessionOutputFrame()
{
   // destructor
   delete fLVContainer; // this container is inside the TGListView and is not
                        // deleted automatically
   Cleanup();
}

//______________________________________________________________________________
void TSessionOutputFrame::Build(TSessionViewer *gui)
{

   fViewer = gui;
   SetLayoutManager(new TGVerticalLayout(this));
   SetCleanup(kDeepCleanup);

   // Container of object TGListView
   TGListView *frmListView = new TGListView(this, 340, 190);
   fLVContainer = new TGLVContainer(frmListView, kSunkenFrame,
                  GetWhitePixel());
   fLVContainer->Associate(frmListView);
   fLVContainer->SetCleanup(kDeepCleanup);
   AddFrame(frmListView, new TGLayoutHints(kLHintsExpandX, 4, 4, 4, 4));

   frmListView->Connect("Clicked(TGLVEntry*, Int_t, Int_t, Int_t)",
      "TSessionOutputFrame", this,
      "OnElementClicked(TGLVEntry* ,Int_t, Int_t, Int_t)");
   frmListView->Connect("DoubleClicked(TGLVEntry*, Int_t, Int_t, Int_t)",
      "TSessionOutputFrame", this,
      "OnElementDblClicked(TGLVEntry* ,Int_t, Int_t, Int_t)");
   fPopupMenu = new TGPopupMenu(fClient->GetRoot());
   fPopupMenu->AddEntry("Add to Feedback",kMenuAddToFeedback);
   fPopupMenu->AddEntry("Show",kMenuShow);
   fPopupMenu->Connect("Activated(Int_t)","TSessionOutputFrame", this,
                       "MyHandleMenu(Int_t)");
}

//______________________________________________________________________________
void TSessionOutputFrame::OnElementClicked(TGLVEntry* entry, Int_t btn, Int_t x,
                                           Int_t y)
{
   TObject *obj = (TObject *)entry->GetUserData();
   if ((obj) && (btn ==3)) {
      fViewer->GetContextMenu()->Popup(x, y, obj, (TBrowser *)0);
   }
}

//______________________________________________________________________________
void TSessionOutputFrame::OnElementDblClicked(TGLVEntry* entry, Int_t , Int_t, Int_t)
{
   char action[512];
   TString act;
   TObject *obj = (TObject *)entry->GetUserData();
   TString ext = obj->GetName();
   gPad->SetEditable(kFALSE);
   if (fClient->GetMimeTypeList()->GetAction(obj->IsA()->GetName(), action)) {
      act = Form("((%s*)0x%lx)%s", obj->IsA()->GetName(), (Long_t)obj, action);
      if (act[0] == '!') {
         act.Remove(0, 1);
         gSystem->Exec(act.Data());
      } else {
         gROOT->ProcessLine(act.Data());
      }
   }
}

//______________________________________________________________________________
void TSessionOutputFrame::MyHandleMenu(Int_t id)
{

   switch(id) {

   case kMenuAddToFeedback :
      if (fEntryTmp) {
         fFeedbackFrame->AddToFeedback(fEntryTmp);
         fEntryTmp = NULL;
      }
      break;
   case kMenuShow :
      break;
   default:
      Error("MyHandleMenu", "Unknown menu option");
      break;
   }
}

//______________________________________________________________________________
void TSessionOutputFrame::AddObject(TObject *obj)
{
   TGLVEntry *item;
   if (obj) {
      item = new TGLVEntry(fLVContainer, obj->GetName(), obj->IsA()->GetName());
      item->SetUserData(obj);
      fLVContainer->AddItem(item);
   }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Input Frame

//______________________________________________________________________________
TSessionInputFrame::TSessionInputFrame(TGWindow* p, Int_t w, Int_t h) :
   TGCompositeFrame(p, w, h), fLVContainer(0)
{
   // constructor
}

//______________________________________________________________________________
TSessionInputFrame::~TSessionInputFrame()
{
   delete fLVContainer; // this container is inside the TGListView and is not
                        // deleted automatically
   Cleanup();
}

//______________________________________________________________________________
void TSessionInputFrame::Build(TSessionViewer *gui)
{

   fViewer = gui;
   SetLayoutManager(new TGVerticalLayout(this));
   SetCleanup(kDeepCleanup);

   // Container of object TGListView
   TGListView *frmListView = new TGListView(this, 340, 190);
   fLVContainer = new TGLVContainer(frmListView, kSunkenFrame, GetWhitePixel());
   fLVContainer->Associate(frmListView);
   fLVContainer->SetCleanup(kDeepCleanup);
   AddFrame(frmListView, new TGLayoutHints(kLHintsExpandX, 4, 4, 4, 4));

   TGLVEntry* entry1 = new TGLVEntry(fLVContainer, "name", "mane");
   TGLVEntry* entry2 = new TGLVEntry(fLVContainer, "name2", "mane222222");

   fLVContainer->AddItem(entry1);
   fLVContainer->AddItem(entry2);
}

//______________________________________________________________________________
void TSessionInputFrame::AddObject(TObject *obj)
{
   TGLVEntry *item;
   if (obj) {
      item = new TGLVEntry(fLVContainer, obj->GetName(), obj->IsA()->GetName());
      item->SetUserData(obj);
      fLVContainer->AddItem(item);
   }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Session Viewer Main Frame

//______________________________________________________________________________
TSessionViewer::TSessionViewer(const char *name, UInt_t w, UInt_t h) :
   TGMainFrame(gClient->GetRoot(), w, h), fSessionHierarchy(0), fSessionItem(0)
{
   Build();
   SetWindowName(name);
   Resize(w, h);
}

//______________________________________________________________________________
TSessionViewer::TSessionViewer(const char *name, Int_t x, Int_t y, UInt_t w,
                              UInt_t h) : TGMainFrame(gClient->GetRoot(), w, h),
                              fSessionHierarchy(0), fSessionItem(0)
{
   Build();
   SetWindowName(name);
   Move(x, y);
   Resize(w, h);
}

//______________________________________________________________________________
void TSessionViewer::Build()
{
   char line[120];
   fActDesc = 0;
   fLogWindow = 0;
   SetCleanup(kDeepCleanup);
   SetWMSizeHints(350 + 200, 310+50, 2000, 1000, 1, 1);

   fLocal = fClient->GetPicture("local_session.xpm");
   fProofCon = fClient->GetPicture("proof_connected.xpm");
   fProofDiscon = fClient->GetPicture("proof_disconnected.xpm");
   fQueryCon = fClient->GetPicture("query_connected.xpm");
   fQueryDiscon = fClient->GetPicture("query_disconnected.xpm");
   fBaseIcon = fClient->GetPicture("proof_base.xpm");


   fFileMenu = new TGPopupMenu(fClient->GetRoot());
   fFileMenu->AddEntry("&Load Library...", kFileLoadLibrary);
   fFileMenu->AddEntry("&Close Viewer",    kFileCloseViewer);
   fFileMenu->AddSeparator();
   fFileMenu->AddEntry("&Quit ROOT",       kFileQuit);

   //--- Edit menu
   fSessionMenu = new TGPopupMenu(gClient->GetRoot());
   fSessionMenu->AddEntry("&Connect...", kSessionConnect);
   fSessionMenu->AddEntry("&Disconnect", kSessionDisconnect);
   fSessionMenu->AddSeparator();
   fSessionMenu->AddEntry("&Cleanup", kSessionCleanup);

   //---Run menu
   fQueryMenu = new TGPopupMenu(gClient->GetRoot());
   fQueryMenu->AddEntry("&New...", kQueryNew);
   fQueryMenu->AddEntry("&Edit", kQueryEdit);
   fQueryMenu->AddEntry("&Submit", kQuerySubmit);
   fQueryMenu->AddSeparator();
   fQueryMenu->AddEntry("Start &Viewer", kQueryStartViewer);
   fQueryMenu->AddSeparator();
   fQueryMenu->AddEntry("&Delete", kQueryDelete);
   fQueryMenu->AddEntry("&Remove", kQueryRemove);

   fHelpMenu = new TGPopupMenu(gClient->GetRoot());
   fHelpMenu->AddEntry("&About ROOT...",  kHelpAbout);

   fFileMenu->Associate(this);
   fSessionMenu->Associate(this);
   fQueryMenu->Associate(this);
   fHelpMenu->Associate(this);

   //--- create menubar and add popup menus
   fMenuBar = new TGMenuBar(this, 1, 1, kHorizontalFrame);

   fMenuBar->AddPopup("&File", fFileMenu, new TGLayoutHints(kLHintsTop |
                      kLHintsLeft, 0, 4, 0, 0));
   fMenuBar->AddPopup("&Session", fSessionMenu, new TGLayoutHints(kLHintsTop |
                      kLHintsLeft, 0, 4, 0, 0));
   fMenuBar->AddPopup("&Query",  fQueryMenu,  new TGLayoutHints(kLHintsTop |
            kLHintsLeft, 0, 4, 0, 0));
   fMenuBar->AddPopup("&Help", fHelpMenu, new TGLayoutHints(kLHintsTop |
            kLHintsRight));

   TGHorizontal3DLine *toolBarSep = new TGHorizontal3DLine(this);
   AddFrame(toolBarSep, new TGLayoutHints(kLHintsTop | kLHintsExpandX));

   AddFrame(fMenuBar, new TGLayoutHints(kLHintsTop | kLHintsLeft |
            kLHintsExpandX, 0, 0, 1, 1));

   toolBarSep = new TGHorizontal3DLine(this);
   AddFrame(toolBarSep, new TGLayoutHints(kLHintsTop | kLHintsExpandX));

   fPopupSrv = new TGPopupMenu(fClient->GetRoot());
   fPopupSrv->AddEntry("Connect",kSessionConnect);
   fPopupSrv->AddEntry("Disconnect",kSessionDisconnect);
   fPopupSrv->AddEntry("Browse",kSessionBrowse);
   fPopupSrv->AddSeparator();
   fPopupSrv->AddEntry("&Cleanup", kSessionCleanup);
   fPopupSrv->Connect("Activated(Int_t)","TSessionViewer", this,
            "MyHandleMenu(Int_t)");

   fPopupQry = new TGPopupMenu(fClient->GetRoot());
   fPopupQry->AddEntry("Edit",kQueryEdit);
   fPopupQry->AddEntry("Submit",kQuerySubmit);
   fPopupQry->AddSeparator();
   fPopupQry->AddEntry("Start &Viewer", kQueryStartViewer);
   fPopupQry->AddSeparator();
   fPopupQry->AddEntry("Delete",kQueryDelete);
   fPopupQry->AddEntry("Remove",kQueryRemove);
   fPopupQry->Connect("Activated(Int_t)","TSessionViewer", this,
            "MyHandleMenu(Int_t)");

   fPopupSrv->DisableEntry(kSessionDisconnect);
   fPopupSrv->DisableEntry(kSessionCleanup);
   fSessionMenu->DisableEntry(kSessionDisconnect);
   fSessionMenu->DisableEntry(kSessionCleanup);

   //--- Horizontal mother frame -----------------------------------------------
   fHf = new TGHorizontalFrame(this, 10, 10);
   fHf->SetCleanup(kDeepCleanup);

   //--- fV1 -------------------------------------------------------------------
   fV1 = new TGVerticalFrame(fHf, 100, 100, kFixedWidth);
   fV1->SetCleanup(kDeepCleanup);

   fTreeView = new TGCanvas(fV1, 100, 200, kSunkenFrame | kDoubleBorder);
   fV1->AddFrame(fTreeView, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,
                 2, 0, 0, 0));
   fSessionHierarchy = new TGListTree(fTreeView, kHorizontalFrame);
   fSessionHierarchy->Connect("Clicked(TGListTreeItem*,Int_t,Int_t,Int_t)",
            "TSessionViewer", this,
            "OnListTreeClicked(TGListTreeItem*, Int_t, Int_t, Int_t)");
   fV1->Resize(fTreeView->GetDefaultWidth()+100, fV1->GetDefaultHeight());

   //--- fV2
   fV2 = new TGVerticalFrame(fHf, 350, 310, kFixedWidth);
   fV2->SetCleanup(kDeepCleanup);

   fServerFrame = new TSessionServerFrame(fV2, 350, 310);
   fSessions = fServerFrame->ReadConfigFile(kPROOF_GuiConfFile);
   BuildSessionHierarchy(fSessions);
   fServerFrame->Build(this);
   fV2->AddFrame(fServerFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX |
                 kLHintsExpandY, 2, 0, 1, 2));

   fSessionFrame = new TSessionFrame(fV2, 350, 310);
   fSessionFrame->Build(this);
   fV2->AddFrame(fSessionFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX |
                 kLHintsExpandY, 2, 0, 1, 2));

   fQueryFrame = new TSessionQueryFrame(fV2, 350, 310);
   fQueryFrame->Build(this);
   fV2->AddFrame(fQueryFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX |
                 kLHintsExpandY, 2, 0, 1, 2));

   fFeedbackFrame = new TSessionFeedbackFrame(fV2, 350, 310);
   fFeedbackFrame->Build(this);
   fV2->AddFrame(fFeedbackFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX |
                 kLHintsExpandY, 2, 0, 1, 2));

   fOutputFrame = new TSessionOutputFrame(fV2, 350, 310);
   fOutputFrame->Build(this);
   fV2->AddFrame(fOutputFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX |
                 kLHintsExpandY, 2, 0, 1, 2));

   fInputFrame = new TSessionInputFrame(fV2, 350, 310);
   fInputFrame->Build(this);
   fV2->AddFrame(fInputFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX |
                 kLHintsExpandY, 2, 0, 1, 2));

   fOutputFrame->SetFeedbackFrame(fFeedbackFrame);

   fHf->AddFrame(fV1, new TGLayoutHints(kLHintsLeft | kLHintsExpandX |
                 kLHintsExpandY));

   TGVSplitter *splitter = new TGVSplitter(fHf, 4);
   splitter->SetFrame(fV2, kFALSE);
   fHf->AddFrame(splitter,new TGLayoutHints(kLHintsLeft | kLHintsExpandY));
   fHf->AddFrame(new TGVertical3DLine(fHf), new TGLayoutHints(kLHintsLeft |
                 kLHintsExpandY));

   fHf->AddFrame(fV2, new TGLayoutHints(kLHintsRight | kLHintsExpandY));

   AddFrame(fHf, new TGLayoutHints(kLHintsRight | kLHintsExpandX |
            kLHintsExpandY));

   if (fActDesc)
      fServerFrame->Update(fActDesc);

   int parts[] = { 36, 49, 15 };
   fStatusBar = new TGStatusBar(this, 10, 10);
   fStatusBar->SetCleanup(kDeepCleanup);
   fStatusBar->SetParts(parts, 3);
   for (int i = 0; i < 3; i++)
      fStatusBar->GetBarPart(i)->SetCleanup(kDeepCleanup);
   AddFrame(fStatusBar, new TGLayoutHints(kLHintsTop | kLHintsLeft |
            kLHintsExpandX, 0, 0, 1, 1));

   fStatusBar->SetText("      00:00:00", 2);
   TGCompositeFrame *leftpart = fStatusBar->GetBarPart(2);
   fRightIconPicture = (TGPicture *)fClient->GetPicture("proof_disconnected.xpm");
   fRightIcon = new TGIcon(leftpart, fRightIconPicture,
                    fRightIconPicture->GetWidth(),
                    fRightIconPicture->GetHeight());
   leftpart->AddFrame(fRightIcon, new TGLayoutHints(kLHintsLeft, 2, 0, 0, 0));

   TGCompositeFrame *rightpart = fStatusBar->GetBarPart(0);
   fConnectProg = new TGHProgressBar(rightpart, TGProgressBar::kStandard, 100);
   fConnectProg->ShowPosition();
   fConnectProg->SetBarColor("green");
   rightpart->AddFrame(fConnectProg, new TGLayoutHints(kLHintsExpandX, 1, 1, 1, 1));

   UserGroup_t *fUserGroup = gSystem->GetUserInfo();
   sprintf(line,"User : %s - %s",fUserGroup->fRealName.Data(),
            fUserGroup->fGroup.Data());
   fStatusBar->SetText(line, 1);

   fTimer = 0;

   fContextMenu = new TContextMenu("SessionViewerContextMenu") ;

   SetWindowName("ROOT Session Viewer");
   MapSubwindows();
   MapWindow();

   fStatusBar->GetBarPart(0)->HideFrame(fConnectProg);
   fV2->HideFrame(fSessionFrame);
   fV2->HideFrame(fQueryFrame);
   fV2->HideFrame(fFeedbackFrame);
   fV2->HideFrame(fOutputFrame);
   fV2->HideFrame(fInputFrame);
   fActFrame = fServerFrame;
   Resize(GetDefaultSize());
}

//______________________________________________________________________________
TSessionViewer::~TSessionViewer()
{
   Cleanup();
}

//______________________________________________________________________________
void TSessionViewer::OnListTreeClicked(TGListTreeItem *entry, Int_t btn,
                                       Int_t x, Int_t y)
{

   TList *objlist;
   TObject *obj;
   TQueryDescription *desc;
   if (entry->GetParent() == 0) {  // PROOF
      if (fActFrame != fServerFrame) {
         fV2->HideFrame(fActFrame);
         fV2->ShowFrame(fServerFrame);
         fActFrame = fServerFrame;
      }
   }
   else if (entry->GetParent()->GetParent() == 0) {   // Server
      if (entry->GetUserData()) {
         fServerFrame->Update((TSessionDescription*)entry->GetUserData());
         fActDesc = (TSessionDescription*)entry->GetUserData();
      }
      if ((fActDesc->fLocal) && (fActFrame != fSessionFrame)) {
         fV2->HideFrame(fActFrame);
         fV2->ShowFrame(fSessionFrame);
         fActFrame = fSessionFrame;
      }
      if ((!fActDesc->fLocal) && (!fActDesc->fConnected) &&
           (fActFrame != fServerFrame)) {
         fV2->HideFrame(fActFrame);
         fV2->ShowFrame(fServerFrame);
         fActFrame = fServerFrame;
      }
      if ((!fActDesc->fLocal) && (fActDesc->fConnected) &&
           (fActFrame != fSessionFrame)) {
         fV2->HideFrame(fActFrame);
         fV2->ShowFrame(fSessionFrame);
         fActFrame = fSessionFrame;
      }
   }
   else if (entry->GetParent()->GetParent()->GetParent() == 0) { // query
      fActDesc = (TSessionDescription*)entry->GetParent()->GetUserData();
      fActDesc->fActQuery = (TQueryDescription*)entry->GetUserData();
      fQueryFrame->UpdateInfos();
      fQueryFrame->UpdateButtons(fActDesc->fActQuery);
      if (fActDesc->fLocal) {
         fQueryFrame->SetTab(0);
         fQueryFrame->SetTabEnabled(1, kFALSE);
      }
      else
         fQueryFrame->SetTabEnabled(1, kTRUE);
      if (fActFrame != fQueryFrame) {
         fV2->HideFrame(fActFrame);
         fV2->ShowFrame(fQueryFrame);
         fActFrame = fQueryFrame;
      }
   }
   else {      // a list (input, output, feedback
      fActDesc = (TSessionDescription*)entry->GetParent()->GetParent()->GetUserData();
      fActDesc->fActQuery = (TQueryDescription*)entry->GetParent()->GetUserData();

      desc = (TQueryDescription *)entry->GetParent()->GetUserData();
      if (desc) {
         fInputFrame->RemoveAll();
         fOutputFrame->RemoveAll();
         if (desc->fResult) {
            objlist = desc->fResult->GetOutputList();
            if (objlist) {
               TIter nexto(objlist);
               while ((obj = (TObject *) nexto())) {
                  fOutputFrame->AddObject(obj);
               }
            }
            objlist = desc->fResult->GetInputList();
            if (objlist) {
               TIter nexti(objlist);
               while ((obj = (TObject *) nexti())) {
                  fInputFrame->AddObject(obj);
               }
            }
         }
         fInputFrame->Resize();
         fOutputFrame->Resize();
         fClient->NeedRedraw(fOutputFrame->GetLVContainer());
         fClient->NeedRedraw(fInputFrame->GetLVContainer());
      }

      if (strstr(entry->GetText(),"Feedback")) {
         if (fActFrame != fFeedbackFrame) {
            fV2->HideFrame(fActFrame);
            fV2->ShowFrame(fFeedbackFrame);
            fActFrame = fFeedbackFrame;
         }
      }
      else if (strstr(entry->GetText(),"Output")) {
         if (fActFrame != fOutputFrame) {
            fV2->HideFrame(fActFrame);
            fV2->ShowFrame(fOutputFrame);
            fActFrame = fOutputFrame;
         }
      }
      else if (strstr(entry->GetText(),"Input")) {
         if (fActFrame != fInputFrame) {
            fV2->HideFrame(fActFrame);
            fV2->ShowFrame(fInputFrame);
            fActFrame = fInputFrame;
         }
      }
   }
   if (btn == 3) { //right button
      TGListTreeItem *item = fSessionHierarchy->GetSelected();
      obj = (TObject *)item->GetUserData();
      if (obj && obj->IsA() == TQueryDescription::Class()) {
         fPopupQry->PlaceMenu(x, y, 1, 1);
      }
      else if (obj && obj->IsA() == TSessionDescription::Class()) {
         if (!fActDesc->fLocal)
            fPopupSrv->PlaceMenu(x, y, 1, 1);
      }
   }
   if (fActDesc->fConnected) {
      fPopupSrv->DisableEntry(kSessionConnect);
      fPopupSrv->EnableEntry(kSessionDisconnect);
      fPopupSrv->EnableEntry(kSessionCleanup);
      fSessionMenu->DisableEntry(kSessionConnect);
      fSessionMenu->EnableEntry(kSessionDisconnect);
      fSessionMenu->EnableEntry(kSessionCleanup);
   }
   else {
      fPopupSrv->DisableEntry(kSessionDisconnect);
      fPopupSrv->DisableEntry(kSessionCleanup);
      fPopupSrv->EnableEntry(kSessionConnect);
      fSessionMenu->DisableEntry(kSessionDisconnect);
      fSessionMenu->DisableEntry(kSessionCleanup);
      fSessionMenu->EnableEntry(kSessionConnect);
   }
   if (fActDesc->fLocal) {
      fSessionMenu->DisableEntry(kSessionConnect);
      fSessionMenu->DisableEntry(kSessionDisconnect);
      fSessionMenu->DisableEntry(kSessionCleanup);
   }
}

//______________________________________________________________________________
void TSessionViewer::BuildSessionHierarchy(TList *list)
{
   // Read the list of proof servers from the configuration file.
   // Get the list of proof servers and running queries from gROOT.
   // Build the hierarchy.

   if (fSessionItem)
      fSessionHierarchy->DeleteChildren(fSessionItem);
   else
      fSessionItem = fSessionHierarchy->AddItem(0, "Sessions", fBaseIcon,
            fBaseIcon);
   TGListTreeItem *item = fSessionHierarchy->AddItem(fSessionItem, "Local",
            fLocal, fLocal);
   fSessionHierarchy->SetToolTipItem(item, "Local Session");
   TSessionDescription *localdesc = new TSessionDescription();
   localdesc->fName = "Local";
   localdesc->fAddress = "Local";
   localdesc->fPort = 0;
   localdesc->fConfigFile = "";
   localdesc->fLogLevel = 0;
   localdesc->fUserName = "";
   localdesc->fQueries = new TList();
   localdesc->fActQuery = 0;
   localdesc->fProof = 0;
   localdesc->fLocal = kTRUE;
   localdesc->fSync = kTRUE;
   item ->SetUserData(localdesc);

   TSeqCollection *proofs = gROOT->GetListOfProofs();
   if (proofs) {
      TIter nextp(proofs);
      TVirtualProof *proof;
      TQueryResult *query;
      TQueryDescription *newquery;
      TSessionDescription *newdesc;
      while ((proof = (TVirtualProof *)nextp())) {

         newdesc = new TSessionDescription();
         newdesc->fName       = proof->GetMaster();
         newdesc->fConfigFile = proof->GetConfFile();
         newdesc->fUserName   = proof->GetUser();
         newdesc->fPort       = proof->GetPort();
         newdesc->fLogLevel   = proof->GetLogLevel();
         newdesc->fAddress    = proof->GetMaster();
         newdesc->fQueries    = new TList();
         newdesc->fActQuery   = 0;
         newdesc->fConnected = kFALSE;
         newdesc->fLocal = kFALSE;
         newdesc->fSync = kFALSE;

         TIter nextq(proof->GetListOfQueries());

         while ((query = (TQueryResult *)nextp())) {
            newquery = new TQueryDescription();
            newquery->fStatus = query->IsFinalized() ?
                 TQueryDescription::kSessionQueryFinalized :
               (TQueryDescription::ESessionQueryStatus)query->GetStatus();
            newquery->fSelectorString  = query->GetSelecImp()->GetName();
            newquery->fQueryName       = query->GetName();
            newquery->fOptions         = query->GetOptions();
            newquery->fEventList       = "";
            newquery->fParFile         = "";
            newquery->fNoEntries       = query->GetEntries();
            newquery->fFirstEntry      = query->GetFirst();
            newquery->fResult          = query;
            newdesc->fQueries->Add((TObject *)newquery);
         }
         item = fSessionHierarchy->AddItem(fSessionItem, newdesc->fName.Data(),
                  fProofCon, fProofCon);
         fSessionHierarchy->SetToolTipItem(item, "Proof Session");
         item ->SetUserData(newdesc);
         fActDesc = newdesc;
      }
   }

   TIter next(list);
   TSessionDescription *desc = 0;
   while ((desc = (TSessionDescription *)next())) {
      item = fSessionHierarchy->AddItem(fSessionItem, desc->fName.Data(),
               fProofDiscon, fProofDiscon);
      fSessionHierarchy->SetToolTipItem(item, "Proof Session");
      desc->fProof = 0;
      desc->fConnected = kFALSE;
      desc->fQueries = new TList();
      desc->fActQuery   = 0;
      item->SetUserData(desc);
      fActDesc = desc;
   }

   fSessionHierarchy->OpenItem(fSessionItem);
   fSessionHierarchy->OpenItem(item);
   fSessionHierarchy->SetSelected(item);
   fClient->NeedRedraw(fSessionHierarchy);
}

//______________________________________________________________________________
void TSessionViewer::CloseWindow()
{
   TIter next(fSessions);
   TSessionDescription *desc = 0;
   while ((desc = (TSessionDescription *)next())) {
      if (desc->fProof && desc->fProof->IsValid())
         desc->fProof->Close();
   }
   delete fSessionHierarchy; // this has been put int TGCanvas which isn't a
                             // TGComposite frame and doesn't do cleanups.
   fClient->FreePicture(fLocal);
   fClient->FreePicture(fProofCon);
   fClient->FreePicture(fProofDiscon);
   fClient->FreePicture(fQueryCon);
   fClient->FreePicture(fQueryDiscon);
   fClient->FreePicture(fBaseIcon);
   delete fTimer;
   DeleteWindow();
}

//______________________________________________________________________________
void TSessionViewer::ChangeRightLogo(const char *name)
{
    // Change the right logo ( used for animation )
    fClient->FreePicture(fRightIconPicture);
    fRightIconPicture = (TGPicture *)fClient->GetPicture(name);
    fRightIcon->SetPicture(fRightIconPicture);
}

//______________________________________________________________________________
void TSessionViewer::EnableTimer()
{
   if (!fTimer) fTimer = new TTimer(this, 500);
   fTimer->Reset();
   fTimer->TurnOn();
   time( &fStart );
}

//______________________________________________________________________________
void TSessionViewer::DisableTimer()
{
   if (fTimer)
      fTimer->TurnOff();
   ChangeRightLogo("proof_disconnected.xpm");
}

//______________________________________________________________________________
Bool_t TSessionViewer::HandleTimer(TTimer *)
{
   char line[120];
   struct tm *connected;
   Int_t count = gRandom->Integer(4);
   if (count > 3) {
      count = 0;
   }
   if (fChangePic)
      ChangeRightLogo(xpm_names[count]);
   time( &fElapsed );
   time_t elapsed_time = (time_t)difftime( fElapsed, fStart );
   connected = gmtime( &elapsed_time );
   sprintf(line,"      %02d:%02d:%02d", connected->tm_hour,
           connected->tm_min, connected->tm_sec);
   fStatusBar->SetText(line, 2);
   fTimer->Reset();
   return kTRUE;
}

//______________________________________________________________________________
void TSessionViewer::LogMessage(const char *msg, Bool_t all)
{
   // Load/append a log msg in the log frame, if open

   if (fLogWindow) {
      if (all) {
         // load buffer
         fLogWindow->LoadBuffer(msg);
      } else {
         // append
         fLogWindow->AddBuffer(msg);
      }
   }
}

//______________________________________________________________________________
void TSessionViewer::QueryResultReady(char *query)
{
   char strtmp[256];
   sprintf(strtmp,"Query Result Ready for %s\n", query);
   ShowInfo(strtmp);
   TGListTreeItem *item=0, *item2=0;
   TQueryDescription *lquery = 0;
   TIter nextp(fActDesc->fQueries);
   while ((lquery = (TQueryDescription *)nextp())) {
      if (lquery->fReference.Contains(query)) {
         lquery->fResult = fActDesc->fProof->GetQueryResult(query);
         lquery->fStatus = TQueryDescription::kSessionQueryFromProof;
         if (!lquery->fResult)
            break;
         lquery->fStatus = lquery->fResult->IsFinalized() ?
           TQueryDescription::kSessionQueryFinalized :
           (TQueryDescription::ESessionQueryStatus)lquery->fResult->GetStatus();
         if (lquery->fResult->GetDSet())
            lquery->fChain = lquery->fResult->GetDSet();
         item = fSessionHierarchy->FindItemByObj(fSessionItem, fActDesc);
         if (item) {
            item2 = fSessionHierarchy->FindItemByObj(item, lquery);
         }
         if (item2) {
            if (lquery->fResult->GetInputList())
               if (!fSessionHierarchy->FindChildByName(item2, "InputList"))
                  fSessionHierarchy->AddItem(item2, "InputList");
            if (lquery->fResult->GetInputList())
               if (!fSessionHierarchy->FindChildByName(item2, "OutputList"))
                  fSessionHierarchy->AddItem(item2, "OutputList");
         }
         fClient->NeedRedraw(fSessionHierarchy);
         fQueryFrame->UpdateButtons(lquery);
         break;
      }
   }
}

//______________________________________________________________________________
void TSessionViewer::CleanupSession()
{
   TGListTreeItem *item = fSessionHierarchy->GetSelected();
   TObject *obj = (TObject *)item->GetUserData();
   if (obj->IsA() != TQueryDescription::Class()) return;
   if (!fActDesc->fProof || !fActDesc->fProof->IsValid()) return;
   TQueryDescription *query = (TQueryDescription *)item->GetUserData();
   TString m;
   m.Form("Are you sure to cleanup the session \"%s::%s\"",
           fActDesc->fAddress.Data(), fActDesc->fName.Data());
   Int_t result;
   new TGMsgBox(fClient->GetRoot(), this, "", m.Data(), 0,
                kMBOk | kMBCancel, &result);
   if (result == kMBOk) {
      fActDesc->fProof->CleanupSession(query->fReference.Data());
      fSessionHierarchy->DeleteChildren(item->GetParent());
      fSessionFrame->OnBtnGetQueriesClicked();
   }
   fClient->NeedRedraw(fSessionHierarchy);
}

//______________________________________________________________________________
void TSessionViewer::RemoveQuery()
{
   TGListTreeItem *item = fSessionHierarchy->GetSelected();
   TObject *obj = (TObject *)item->GetUserData();
   if (obj->IsA() != TQueryDescription::Class()) return;
   if (!fActDesc->fProof || !fActDesc->fProof->IsValid()) return;
   TQueryDescription *query = (TQueryDescription *)item->GetUserData();
   TString m;
   m.Form("Are you sure to remove the query \"%s\"",
            query->fQueryName.Data());
   Int_t result;
   new TGMsgBox(fClient->GetRoot(), this, "", m.Data(), 0,
                kMBOk | kMBCancel, &result);
   if (result == kMBOk) {
      fActDesc->fProof->Remove(query->fReference.Data());
      fSessionHierarchy->DeleteItem(item);
   }
   fClient->NeedRedraw(fSessionHierarchy);
}

//______________________________________________________________________________
void TSessionViewer::DeleteQuery()
{
   TGListTreeItem *item = fSessionHierarchy->GetSelected();
   TObject *obj = (TObject *)item->GetUserData();
   if (obj->IsA() != TQueryDescription::Class()) return;
   TQueryDescription *query = (TQueryDescription *)item->GetUserData();
   TString m;
   m.Form("Are you sure to delete the query \"%s\"",
            query->fQueryName.Data());
   Int_t result;
   new TGMsgBox(fClient->GetRoot(), this, "", m.Data(), 0,
            kMBOk | kMBCancel, &result);
   if (result == kMBOk) {
      fActDesc->fQueries->Remove((TObject *)query);
      fSessionHierarchy->DeleteItem(item);
      delete query;
   }
   fClient->NeedRedraw(fSessionHierarchy);
}

//______________________________________________________________________________
void TSessionViewer::EditQuery()
{
   TGListTreeItem *item = fSessionHierarchy->GetSelected();
   TObject *obj = (TObject *)item->GetUserData();
   if (obj->IsA() != TQueryDescription::Class()) return;
   TQueryDescription *query = (TQueryDescription *)obj;
   TNewQueryDlg *dlg = new TNewQueryDlg(this, 350, 310, query, kTRUE);
   dlg->Popup();
}

//______________________________________________________________________________
void TSessionViewer::StartViewer()
{
   TGListTreeItem *item = fSessionHierarchy->GetSelected();
   TObject *obj = (TObject *)item->GetUserData();
   if (obj->IsA() != TQueryDescription::Class()) return;
   TQueryDescription *query = (TQueryDescription *)obj;
   if (!query->fChain && query->fResult && query->fResult->GetDSet()) {
      query->fChain = query->fResult->GetDSet();
   }
   if (query->fChain->IsA() == TChain::Class())
      ((TChain *)query->fChain)->StartViewer();
   else if (query->fChain->IsA() == TDSet::Class())
      ((TDSet *)query->fChain)->StartViewer();
}

//______________________________________________________________________________
void TSessionViewer::ShowLog(const char *queryref)
{
   Window_t wdummy;
   Int_t  ax, ay;

   if (fActDesc->fProof) {
      if (!fLogWindow) {
         fLogWindow = new TSessionLogView(this, 700, 100);
      } else {
         // Clear window
         fLogWindow->Clear();
      }
      fActDesc->fProof->Connect("LogMessage(const char*,Bool_t)",
            "TSessionViewer", this, "LogMessage(const char*,Bool_t)");
      Bool_t logonly = fActDesc->fProof->SendingLogToWindow();
      fActDesc->fProof->SendLogToWindow(kTRUE);
      if (queryref)
         fActDesc->fProof->ShowLog(queryref);
      else
         fActDesc->fProof->ShowLog(0);
      fActDesc->fProof->SendLogToWindow(logonly);
      gVirtualX->TranslateCoordinates(GetId(),
                                      fClient->GetDefaultRoot()->GetId(),
                                      0, 0, ax, ay, wdummy);
      fLogWindow->Move(ax, ay + GetHeight() + 35);
      fLogWindow->Popup();
   }
}

//______________________________________________________________________________
void TSessionViewer::ShowInfo(const char *txt)
{
   fStatusBar->SetText(txt,0);
   fClient->NeedRedraw(fStatusBar);
   gSystem->ProcessEvents();
}

//______________________________________________________________________________
void TSessionViewer::StartupMessage(char *msg, Bool_t, Int_t done, Int_t total)
{
   Float_t pos = Float_t(Double_t(done * 100)/Double_t(total));
   fConnectProg->SetPosition(pos);
   fStatusBar->SetText(msg, 1);
}

//______________________________________________________________________________
void TSessionViewer::MyHandleMenu(Int_t id)
{

   switch (id) {

      case kSessionConnect:
         fServerFrame->OnBtnConnectClicked();
         break;

      case kSessionDisconnect:
         fSessionFrame->OnBtnDisconnectClicked();
         break;

      case kSessionCleanup:
         CleanupSession();
         break;

      case kSessionBrowse:
         if (fActDesc->fProof && fActDesc->fProof->IsValid()) {
            TBrowser *b = new TBrowser();
            fActDesc->fProof->Browse(b);
         }
         break;

      case kQueryEdit:
         EditQuery();
         break;

      case kQueryDelete:
         DeleteQuery();
         break;

      case kQueryRemove:
         RemoveQuery();
         break;

      case kQueryStartViewer:
         StartViewer();
         break;

      case kQuerySubmit:
         fServerFrame->OnBtnConnectClicked();
         break;
   }
}

//______________________________________________________________________________
Bool_t TSessionViewer::ProcessMessage(Long_t msg, Long_t parm1, Long_t)
{
   // Handle messages send to the TSessionViewer object. E.g. all menu entries
   // messages.
   TNewQueryDlg *dlg;

   switch (GET_MSG(msg)) {
      case kC_COMMAND:
         switch (GET_SUBMSG(msg)) {
            case kCM_MENU:
               switch (parm1) {

                  case kFileCloseViewer:
                     CloseWindow();
                     break;

                  case kFileLoadLibrary:
                     break;

                  case kFileQuit:
                     gApplication->Terminate(0);
                     break;

                  case kSessionCleanup:
                     CleanupSession();
                     break;

                  case kSessionConnect:
                     fServerFrame->OnBtnConnectClicked();
                     break;

                  case kSessionDisconnect:
                     fSessionFrame->OnBtnDisconnectClicked();
                     break;

                  case kQueryNew:
                     dlg = new TNewQueryDlg(this, 350, 310);
                     dlg->Popup();
                     break;

                  case kQueryEdit:
                     EditQuery();
                     break;

                  case kQueryDelete:
                     DeleteQuery();
                     break;

                  case kQueryRemove:
                     RemoveQuery();
                     break;

                  case kQueryStartViewer:
                     StartViewer();
                     break;

                  case kQuerySubmit:
                     fServerFrame->OnBtnConnectClicked();
                     break;

                  case kHelpAbout:
                     {
#ifdef R__UNIX
                        TString rootx;
# ifdef ROOTBINDIR
                        rootx = ROOTBINDIR;
# else
                        rootx = gSystem->Getenv("ROOTSYS");
                        if (!rootx.IsNull()) rootx += "/bin";
# endif
                        rootx += "/root -a &";
                        gSystem->Exec(rootx);
#else
#ifdef WIN32
                        new TWin32SplashThread(kTRUE);
#else
                        char str[32];
                        sprintf(str, "About ROOT %s...", gROOT->GetVersion());
                        TRootHelpDialog *hd = new TRootHelpDialog(this, str, 600, 400);
                        hd->SetText(gHelpAbout);
                        hd->Popup();
#endif
#endif
                     }
                     break;

                  default:
                     break;
               }
            default:
               break;
         }
      default:
         break;
   }

   return kTRUE;
}


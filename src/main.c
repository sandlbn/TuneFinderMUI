#define MUI_OBSOLETE

#include <clib/alib_protos.h>
#include <exec/types.h>
#include <libraries/mui.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/app.h"
#include "../include/main.h"

// Global variables
static CONST_STRPTR Codecs[32];
static CONST_STRPTR Countries[32];
struct ObjApp *objApp;
struct Library *MUIMasterBase;

// Static test data
struct Tune tune1;
struct Tune tune2;
struct Tune tune3;
struct Tune tune4;

// External references
extern struct GfxBase *GfxBase;
extern struct IntuitionBase *IntuitionBase;

// Function implementations
BOOL APP_About(void) {
  PutStr("APP_About()\n");

  MUI_RequestA(objApp->App, objApp->WIN_Main, 0, NULL, "Continue",
               "\33c"
               "\0338" APP_NAME " " APP_VERSION " (" APP_DATE
               ")"
               "\n\n"
               "\0332" APP_DESCRIPTION
               "\n"
               "\0332Written by " APP_AUTHORS "\n\n" APP_COPYRIGHT
               "\n\n"
               "This is a MUI-Application.\n"
               "MUI is copyrighted by Stefan Stuntz.",
               NULL);

  return TRUE;
}

BOOL APP_About_MUI(void) {
  PutStr("APP_About_MUI()\n");

  if (!objApp->WIN_About) {
    objApp->WIN_About = AboutmuiObject, MUIA_Window_RefWindow, objApp->WIN_Main,
    MUIA_Aboutmui_Application, objApp->App, End;
  }

  set(objApp->WIN_About, MUIA_Window_Open, TRUE);
  return TRUE;
}

BOOL APP_Find_Init(void) {
  PutStr("APP_Find_Init()\n");

  // Initialize Codecs array
  Codecs[0] = "ALL";
  Codecs[1] = "AAC";
  Codecs[2] = "MP3";
  Codecs[3] = "OGG";
  Codecs[4] = "FLAC";
  Codecs[5] = NULL;

  // Initialize Countries array
  Countries[0] = "ALL";
  Countries[1] = "AT - Austria";
  Countries[2] = "CA - Canada";
  Countries[3] = "CZ - Czech";
  Countries[4] = "DE - Germany";
  Countries[5] = "DK - Denmark";
  Countries[6] = "ES - Spain";
  Countries[7] = "FR - France";
  Countries[8] = "GB - United";
  Countries[9] = "JP - Japan";
  Countries[10] = "NO - Norway";
  Countries[11] = "PL - Poland";
  Countries[12] = "SE - Sweden";
  Countries[13] = "US - United States";
  Countries[14] = NULL;

  // Set initial values
  set(objApp->STR_Find_Name, MUIA_String_Contents, "Chillout");
  set(objApp->STR_Find_Tags, MUIA_String_Contents, "");
  set(objApp->CYC_Find_Codec, MUIA_Cycle_Entries, Codecs);
  set(objApp->CYC_Find_Codec, MUIA_Cycle_Active, 2);
  set(objApp->CYC_Find_Country, MUIA_Cycle_Entries, Countries);
  set(objApp->CYC_Find_Country, MUIA_Cycle_Active, 13);
  set(objApp->CHK_Find_HTTPS_Only, MUIA_Selected, FALSE);
  set(objApp->CHK_Find_Hide_Broken, MUIA_Selected, TRUE);

  return TRUE;
}

BOOL APP_Find(void) {
  static char buf[1024];
  STRPTR name, tags;
  ULONG codec, country;
  ULONG httpsOnly, hideBroken;
  ULONG numEntries = 0;

  // Get search parameters
  get(objApp->STR_Find_Name, MUIA_String_Contents, &name);
  get(objApp->STR_Find_Tags, MUIA_String_Contents, &tags);
  get(objApp->CYC_Find_Codec, MUIA_Cycle_Active, &codec);
  get(objApp->CYC_Find_Country, MUIA_Cycle_Active, &country);
  get(objApp->CHK_Find_HTTPS_Only, MUIA_Selected, &httpsOnly);
  get(objApp->CHK_Find_Hide_Broken, MUIA_Selected, &hideBroken);



  tune1.tu_Name = "Chillout Radio";
  tune1.tu_BitRate = "128bits";
  tune1.tu_Codec = "MP3";
  tune1.tu_Country = "UK - United Kingdom";
  tune1.tu_Description =  "Chillout Radio description";
  tune2.tu_Name = "Chillout Radio";
  tune2.tu_BitRate = "128bits";
  tune2.tu_Codec = "MP3";
  tune2.tu_Country = "UK - United Kingdom";
  tune2.tu_Description = "Chillout Radio description";

  // Update list
  DoMethod(objApp->LSV_Tune_List, MUIM_List_Clear);
  set(objApp->LSV_Tune_List, MUIA_List_Quiet, TRUE);
 DoMethod(objApp->LSV_Tune_List, MUIM_List_InsertSingle, &tune1, MUIV_List_Insert_Bottom);
DoMethod(objApp->LSV_Tune_List, MUIM_List_InsertSingle, &tune2, MUIV_List_Insert_Bottom);

  // ... Insert other tunes ...
  set(objApp->LSV_Tune_List, MUIA_List_Quiet, FALSE);

  // Update result count
  get(objApp->LSV_Tune_List, MUIA_List_Entries, &numEntries);
  sprintf(buf, "Found %lu tune(s), in 0.8 second(s) [Limit: %lu].", numEntries,
          API_LIMIT_DEFAULT);
  set(objApp->LAB_Tune_Result, MUIA_Text_Contents, buf);

  set(objApp->LSV_Tune_List, MUIA_List_Active, MUIV_List_Active_Top);

  return TRUE;
}

int main(void) {
  int result = RETURN_FAIL;

  if ((MUIMasterBase = OpenLibrary(MUIMASTER_NAME, 19))) {
    if ((objApp = CreateApp())) {
      BOOL running = TRUE;

      // Initialize application
      PutStr("Initializing...\n");
      APP_Find_Init();
      APP_Tune_List_Init();
      APP_Tune_Details_Init();
      APP_Settings_Init();

      PutStr("Opening main window...\n");
      DoMethod(objApp->WIN_Main, MUIM_Set, MUIA_Window_Open, TRUE);
      PutStr("Window open command sent\n");

      // Ensure window is actually open
      ULONG isOpen = FALSE;
      get(objApp->WIN_Main, MUIA_Window_Open, &isOpen);
      if (!isOpen) {
        PutStr("Failed to open window!\n");
        running = FALSE;
      } else {
        PutStr("Window opened successfully\n");
      }

      // Main event loop
      while (running) {
        ULONG signals = 0;
        ULONG id = DoMethod(objApp->App, MUIM_Application_NewInput, &signals);

        if (id > 0) {
          switch (id) {
            case EVENT_TUNE_ACTIVE:
              APP_Tune_Active();
              break;
            case EVENT_TUNE_DBLCLICK:
              APP_Tune_DblClick();
              break;
            case EVENT_TUNE_PLAY:
              APP_Tune_Play();
              break;
            case EVENT_TUNE_STOP:
              APP_Tune_Stop();
              break;
            case EVENT_TUNE_SAVE:
              APP_Tune_Save();
              break;
            case EVENT_FIND:
              APP_Find();
              break;
            case EVENT_ICONIFY:
              APP_Iconify();
              break;
            case EVENT_SAVE:
              APP_Save();
              break;
            case EVENT_ABOUT:
              APP_About();
              break;
            case EVENT_ABOUT_MUI:
              APP_About_MUI();
              break;
            case EVENT_QUIT:
              running = FALSE;
              break;

            case EVENT_SETTINGS:
              APP_Settings();
              break;
            case EVENT_SETTINGS_MUI:
              APP_Settings_MUI();
              break;
            case EVENT_SETTINGS_API_PORT_INC:
              APP_Settings_API_Port_Inc();
              break;
            case EVENT_SETTINGS_API_PORT_DEC:
              APP_Settings_API_Port_Dec();
              break;
            case EVENT_SETTINGS_API_LIMIT_INC:
              APP_Settings_API_Limit_Inc();
              break;
            case EVENT_SETTINGS_API_LIMIT_DEC:
              APP_Settings_API_Limit_Dec();
              break;
            case EVENT_SETTINGS_SAVE:
              APP_Settings_Save();
              break;
            case EVENT_SETTINGS_CANCEL:
              APP_Settings_Cancel();
              break;

            case MUIV_Application_ReturnID_Quit:
              running = FALSE;
              break;
          }
        }

        if (signals) Wait(signals);
      }

      PutStr("Cleaning up...\n");
      set(objApp->WIN_Main, MUIA_Window_Open, FALSE);
      DisposeApp(objApp);
      result = RETURN_OK;
    } else {
      PutStr("Failed to create application!\n");
    }

    CloseLibrary(MUIMasterBase);
  } else {
    PutStr("Failed to open MUIMaster library!\n");
  }

  return result;
}
#define MUI_OBSOLETE

#include <clib/alib_protos.h>
#include <exec/types.h>
#include <libraries/mui.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <libraries/asl.h>
#include <clib/asl_protos.h>
#include <proto/asl.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/app.h"
#include "../include/main.h"
#include "../include/network.h"
#include "../include/settings.h"
#include "../include/countries.h"
#include "../include/country_config.h"
#include "../include/unicode.h"

// Global variables
static CONST_STRPTR Codecs[32];
static CONST_STRPTR Countries[32];
struct ObjApp *objApp;
struct Library *MUIMasterBase;
struct Library *AslBase;


// External references
extern struct GfxBase *GfxBase;
extern struct IntuitionBase *IntuitionBase;

static BOOL InitLibs(void)
{
    if (!(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39)))
        return FALSE;

    if (!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0))) {
        CloseLibrary((struct Library *)IntuitionBase);
        return FALSE;
    }

    if (!(MUIMasterBase = OpenLibrary(MUIMASTER_NAME, 19))) {
        CloseLibrary((struct Library *)GfxBase);
        CloseLibrary((struct Library *)IntuitionBase);
        return FALSE;
    }

    if (!(AslBase = OpenLibrary("asl.library", 37))) {
        CloseLibrary(MUIMasterBase);
        CloseLibrary((struct Library *)GfxBase);
        CloseLibrary((struct Library *)IntuitionBase);
        return FALSE;
    }

    return TRUE;
}

static void CleanupLibs(void)
{
    if (AslBase) CloseLibrary(AslBase);
    if (MUIMasterBase) CloseLibrary(MUIMasterBase);
    if (GfxBase) CloseLibrary((struct Library *)GfxBase);
    if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
}
// Function implementations
BOOL APP_About(void)
{
    DEBUG("%s", "APP_About()\n");
    
    // Create the about text with proper translations
    static char aboutText[512];
    // Using special formatting to keep the layout nice
    sprintf(aboutText,
        "\33c"                     // Center alignment
        "\0338%s %s (%s)\n\n"     // Title, version, date in bold
        "\0332%s\n"               // Description
        "\0332%s\n\n"             // Author line
        "%s\n\n"                  // Copyright
        "%s",                     // MUI line
        GetTFString(MSG_STATE_ABOUT),  // "About"
        APP_VERSION,
        APP_DATE,
        APP_DESCRIPTION,
        APP_AUTHORS,
        APP_COPYRIGHT,
        "This is a MUI-Application.\nMUI is copyrighted by Stefan Stuntz.");

    MUI_RequestA(objApp->App, 
                objApp->WIN_Main, 
                0, 
                NULL,
                GetTFString(MSG_ACTION_CANCEL),  // "Continue" button
                aboutText,
                NULL);
    
    return TRUE;
}

BOOL APP_About_MUI(void) {
  DEBUG("%s", "APP_About_MUI()\n");
  

  if (!objApp->WIN_About) {
    objApp->WIN_About = AboutmuiObject, MUIA_Window_RefWindow, objApp->WIN_Main,
    MUIA_Aboutmui_Application, objApp->App, End;
  }

  set(objApp->WIN_About, MUIA_Window_Open, TRUE);
  return TRUE;
}

BOOL APP_Find_Init(void) {
  DEBUG("%s", "APP_Find_Init()\n");
  struct APISettings settings;

  // Initialize Codecs array
  Codecs[0] = "";
  Codecs[1] = "AAC";
  Codecs[2] = "MP3";
  Codecs[3] = "OGG";
  Codecs[4] = "FLAC";
  Codecs[5] = NULL;
  
  if (!LoadCountryConfig(TUNEFINDER_DIR "countries.cfg", &objApp->countryConfig))
  {
        DEBUG("Failed to load country configuration\n");
        return FALSE;
  }

  // Initialize cycle gadgets with country list

  if (LoadSettings(&settings)) {     
        DEBUG("After LoadSettings - Country: %ld, Codec: %ld", settings.countryCode, settings.codec);

        // Initialize cycle gadgets with lists
        set(objApp->CYC_Find_Codec, MUIA_Cycle_Entries, Codecs);
        set(objApp->CYC_Find_Country, MUIA_Cycle_Entries, objApp->countryConfig.choices);

        // Set saved positions

        set(objApp->CYC_Find_Country, MUIA_Cycle_Active, settings.countryCode);
        set(objApp->CYC_Find_Codec, MUIA_Cycle_Active, settings.codec);

        // Set other controls
        set(objApp->STR_Find_Name, MUIA_String_Contents, "");
        set(objApp->STR_Find_Tags, MUIA_String_Contents, "");
        set(objApp->CHK_Find_HTTPS_Only, MUIA_Selected, FALSE);
        set(objApp->CHK_Find_Hide_Broken, MUIA_Selected, TRUE);
        set(objApp->LAB_Tune_Result, MUIA_Text_Contents, GetTFString(MSG_STATE_READY));

 }
  return TRUE;
}

BOOL APP_Find(void)
{
    static char buf[1024];
    STRPTR name, tags;
    ULONG codec, country;
    ULONG httpsOnly, hideBroken;
    struct SearchParams params;
    struct APISettings settings;
    struct Tune *stations;
    static char limitStr[32];
    LONG numEntries = 0;
    ULONG startTime, endTime;
    
    DEBUG("APP_Find()\n");
    
    // Get search parameters from GUI
    get(objApp->STR_Find_Name, MUIA_String_Contents, &name);
    get(objApp->STR_Find_Tags, MUIA_String_Contents, &tags);
    get(objApp->CYC_Find_Codec, MUIA_Cycle_Active, &codec);
    get(objApp->CYC_Find_Country, MUIA_Cycle_Active, &country);
    get(objApp->CHK_Find_HTTPS_Only, MUIA_Selected, &httpsOnly);
    get(objApp->CHK_Find_Hide_Broken, MUIA_Selected, &hideBroken);
    
    startTime = IntuitionBase->Seconds;

    // Load current settings or use defaults
    if (!LoadSettings(&settings)) {
        memcpy(&settings, &DEFAULT_SETTINGS, sizeof(struct APISettings));
    }
    DEBUG("Settings after load - limit: %lu", settings.limit);

    params.name = name;
    params.tag_list = tags;
    params.codec = Codecs[codec];
    
    // Add country code handling here
    if (country > 0 && country < objApp->countryConfig.count)  // Skip first empty entry
    {
        params.country_code = objApp->countryConfig.entries[country].code;
    }
    else
    {
        params.country_code = NULL;  // No country filter
    }
    
    sprintf(limitStr, "%lu", settings.limit);  

    params.state = NULL;
    params.hidebroken = hideBroken;
    params.is_https = httpsOnly ? HTTPS_TRUE : HTTPS_ALL;
    params.limit = limitStr;

    // Clear existing list
    DoMethod(objApp->LSV_Tune_List, MUIM_List_Clear);
    set(objApp->LSV_Tune_List, MUIA_List_Quiet, TRUE);

    // Perform search
    DEBUG("Search settings - Host: %s, Port: %d, Limit: %d", 
          settings.host, settings.port, settings.limit);
          
    stations = SearchStations(&settings, &params, &numEntries);
    if (stations)
    {
        for(int i = 0; i < numEntries; i++)
        {
            // Allocate new tune structure for each entry
            struct Tune *tune = AllocVec(sizeof(struct Tune), MEMF_CLEAR);
            if (tune)
            {
                // Convert station to tune structure

                char *converted_name = convertToASCII((const unsigned char *)stations[i].name);
                tune->name = converted_name ? converted_name : strdup(stations[i].name);
                tune->codec = strdup(stations[i].codec);
                // Convert bitrate to string
                char bitrate[32];
                sprintf(bitrate, "%ld", stations[i].bitrate);
                tune->bitrate = strdup(bitrate);
                tune->country = strdup(stations[i].country);
                tune->url = strdup(stations[i].url);

                DoMethod(objApp->LSV_Tune_List, MUIM_List_InsertSingle, tune, 
                        MUIV_List_Insert_Bottom);
            }
        }
        
        // Free stations array
        free(stations);
    }

    endTime = IntuitionBase->Seconds;
    ULONG duration = endTime - startTime;
    set(objApp->LSV_Tune_List, MUIA_List_Quiet, FALSE);
    GetTFFormattedString(buf, sizeof(buf), MSG_STATUS_SEARCH_RESULT, numEntries, duration, settings.limit);
    set(objApp->LAB_Tune_Result, MUIA_Text_Contents, buf);
    
    set(objApp->LSV_Tune_List, MUIA_List_Active, MUIV_List_Active_Top);
    
    return TRUE;
}

int main(void) {
  int result = RETURN_FAIL;
  struct APISettings settings;
  LONG country, codec;
  ULONG httpsOnly;

    if (!InitLocaleSystem()) {
        DEBUG("%s", "Warning: Failed to initialize locale system\n");
        // Continue anyway, will use built-in strings
    }
    if (!InitLibs()) {
        return 20;

    }
    // Initialize network
    if (!InitNetworkSystem())
    {
        DEBUG("Failed to initialize network system\n");
        CleanupLocaleSystem();
        return RETURN_FAIL;
    }

    if ((objApp = CreateApp())) {
      BOOL running = TRUE;

      // Initialize application
      DEBUG("%s", "Initializing...\n");
      APP_Find_Init();
      APP_Tune_List_Init();
      APP_Tune_Details_Init();
      APP_Settings_Init();

      DEBUG("%s", "Opening main window...\n");
      DoMethod(objApp->WIN_Main, MUIM_Set, MUIA_Window_Open, TRUE);
      DEBUG("%s", "Window open command sent\n");

      // Ensure window is actually open
      ULONG isOpen = FALSE;
      get(objApp->WIN_Main, MUIA_Window_Open, &isOpen);
      if (!isOpen) {
        DEBUG("%s", "Failed to open window!\n");
        running = FALSE;
      } else {
        DEBUG("%s", "Window opened successfully\n");
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
            case EVENT_FAVORITES:
              APP_ShowFavorites();
              break;
            case EVENT_SETTINGS_CANCEL:
              APP_Settings_Cancel();
              break;  
            case EVENT_SETTINGS_NEXT_SERVER:
              APP_Settings_Next_Server();
              break;
            case EVENT_SETTINGS_BROWSE_AMIGAAMP: {
            struct FileRequester *req;
            char path[256];
                          DEBUG("%s", "ASL up...\n");

            if (AslBase)
            {

                req = AllocAslRequestTags(ASL_FileRequest,
                    ASLFR_TitleText, GetTFString(MSG_ASLREQ_SELECT_AMIGAAMP),
                    ASLFR_DoPatterns, TRUE,
                    ASLFR_InitialPattern, "#?",
                    TAG_DONE);
                
                if (req)
                {
                    if (AslRequest(req, NULL))
                    {
                        strcpy(path, req->fr_Drawer);
                        AddPart(path, req->fr_File, sizeof(path));
                        set(objApp->STR_Settings_AmigaAmp, MUIA_String_Contents, path);
                    }
                    FreeAslRequest(req);
                }
            }
        }
    
            break;
            case EVENT_FAV_ADD:
            APP_Fav_Add();
            break;
            case EVENT_FAV_REMOVE:
            APP_Fav_Remove();
            break;

            case MUIV_Application_ReturnID_Quit:
              running = FALSE;
              break;
          }
        }

        if (signals) Wait(signals);
      }

      DEBUG("%s", "Cleaning up...\n");
      set(objApp->WIN_Main, MUIA_Window_Open, FALSE);
      DisposeApp(objApp);
      result = RETURN_OK;
    } else {
      DEBUG("%s", "Failed to create application!\n");
    }
    if (objApp && objApp->WIN_Main) {
        get(objApp->CYC_Find_Country, MUIA_Cycle_Active, &country);
        get(objApp->CYC_Find_Codec, MUIA_Cycle_Active, &codec);
        get(objApp->CHK_Find_HTTPS_Only, MUIA_Selected, &httpsOnly);
    }
  if (LoadSettings(&settings)) {     
     settings.countryCode = country;
     settings.codec = codec;
     settings.httpsOnly = httpsOnly;

    DEBUG("Country: %ld", country);

        if (SaveSettings(&settings)) {
            DEBUG("Settings saved successfully");
        } else {
            DEBUG("Failed to save settings");
        }
        if (settings.quitAmigaAMP) {
            APP_ShutdownAmigaAMP();
        }
  }
 CleanupLibs();
 CleanupLocaleSystem();
 CleanupNetworkSystem();
return result;
}


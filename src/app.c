#define MUI_OBSOLETE

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <exec/memory.h>
#include <exec/types.h>
#include <intuition/intuition.h>
#include <libraries/mui.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>
#include <proto/dos.h>
#include <dos/dostags.h>
#include <dos/dosextens.h>  
#include <libraries/asl.h>
#include <clib/asl_protos.h>
#include <proto/asl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "SDI_compiler.h"
#include "SDI_hook.h"
#include "SDI_lib.h"
#include "SDI_stdarg.h"

#include "../include/app.h"
#include "../include/main.h"
#include "../include/locale.h"
#include "../include/utils.h"
#include "../include/amigaamp.h"
#include "../include/countries.h"
#include "../include/country_config.h"
#include "../include/settings.h"
#include "../include/favorites.h"


#ifdef __GNUC__
extern void geta4(void);
#endif

struct ObjApp *objApp;  // Global variable definition

HOOKPROTONH(DisplayCode, VOID, char **array, struct Tune *tune) {
  if (tune != NULL) {
    *array++ = tune->name;
    *array++ = tune->codec;
    *array++ = tune->bitrate;
    *array = tune->country;
  } else {
    // Column headers
    *array++ = "\033c\033uName";
    *array++ = "\033c\033uCodec";
    *array++ = "\033c\033uBitRate";
    *array = "\033c\033uCountry";
  }
}
MakeStaticHook(DisplayHook, DisplayCode);


BOOL APP_StartupAmigaAMP(const struct APISettings *settings)
{
    DEBUG("APP_StartupAmigaAMP()");
    
    if (!settings || settings->autostart[0] == '\0') {
        DEBUG("No autostart path configured");
        return FALSE;
    }

    // Check if AmigaAMP is already running
    if (IsAmigaAMPRunning()) {
        DEBUG("AmigaAMP is already running");
        // If iconification requested, just iconify
        if (settings->iconifyAmigaAMP) {
            return SendCommandToAmigaAMP("ICONIFY");
        }
        return TRUE;
    }

    // Launch AmigaAMP
    DEBUG("Launching AmigaAMP: %s", settings->autostart);
    
    SystemTags(settings->autostart,
        SYS_Input, NULL,
        SYS_Output, NULL,
        SYS_Asynch, TRUE,
        NP_Priority, 0,  // Normal priority
        TAG_DONE);

    // If iconification requested, wait and iconify
    if (settings->iconifyAmigaAMP) {
        return WaitAndIconifyAmigaAMP();
    }

    return TRUE;
}

BOOL APP_ShutdownAmigaAMP(void)
{
    DEBUG("APP_ShutdownAmigaAMP()");
    
    if (!IsAmigaAMPRunning()) {
        DEBUG("AmigaAMP is not running");
        return TRUE;
    }

    // Try to quit AmigaAMP gracefully
    if (!SendCommandToAmigaAMP("QUIT")) {
        DEBUG("Failed to send QUIT command to AmigaAMP");
        return FALSE;
    }

    // Wait for AmigaAMP to close (optional)
    int retries = 10;
    while (retries > 0 && IsAmigaAMPRunning()) {
        Delay(5); // Wait 100ms
        retries--;
    }

    return !IsAmigaAMPRunning();
}

void CreateMenu(struct ObjApp *obj) {
  APTR menu1, menu2;

  // Create menu items
    obj->MN_Project_Find = MakeMenuItem(GetTFString(MSG_ACTION_SEARCH), "F");        // "Find Tunes"
    obj->MN_Project_Save = MakeMenuItem(GetTFString(MSG_ACTION_SAVE_ALL), "S");      // "Save Tunes"
    obj->MN_Project_About = MakeMenuItem(GetTFString(MSG_STATE_ABOUT), "?");         // "About"
    obj->MN_Project_About_MUI = MakeMenuItem("About MUI...", NULL);                  // Keep MUI specific text
    obj->MN_Project_Settings = MakeMenuItem(GetTFString(MSG_STATE_SETTINGS), NULL);  // "Settings..."
    obj->MN_Project_Settings_MUI = MakeMenuItem("Settings MUI...", NULL);           // Keep MUI specific text
    obj->MN_Project_Iconify = MakeMenuItem(GetTFString(MSG_STATE_ICONIFY), "I");    // "Iconify"
    obj->MN_Project_Quit = MakeMenuItem(GetTFString(MSG_ACTION_QUIT), "Q");         // "Quit"
    obj->MN_Project_Favorites = MenuitemObject,
    MUIA_Menuitem_Title, GetTFString(MSG_STATE_FAVORITES),
    MUIA_Menuitem_Checkit, TRUE,
    MUIA_Menuitem_Toggle, TRUE,
    MUIA_Menuitem_Shortcut, "F",
    End;
    obj->MN_Tune_Play = MakeMenuItem(GetTFString(MSG_ACTION_PLAY), "P");     // "Play Tune"
    obj->MN_Tune_Stop = MakeMenuItem(GetTFString(MSG_ACTION_STOP), "T");     // "Stop Tune"
    obj->MN_Tune_Save = MakeMenuItem(GetTFString(MSG_ACTION_SAVE_ONE), NULL); // "Save Tune"


  // Create Project menu
    menu1 = MenuitemObject,
        MUIA_Menuitem_Title, GetTFString(MSG_STATE_PROJECT),  // "Project"
        MUIA_Family_Child, obj->MN_Project_Find,
        MUIA_Family_Child, obj->MN_Project_Save,
        MUIA_Family_Child, obj->MN_Project_Favorites,
        MUIA_Family_Child, MakeMenuBar(),
        MUIA_Family_Child, obj->MN_Project_About,
        MUIA_Family_Child, obj->MN_Project_About_MUI,
        MUIA_Family_Child, MakeMenuBar(),
        MUIA_Family_Child, obj->MN_Project_Settings,
        MUIA_Family_Child, obj->MN_Project_Settings_MUI,
        MUIA_Family_Child, MakeMenuBar(),
        MUIA_Family_Child, obj->MN_Project_Iconify,
        MUIA_Family_Child, MakeMenuBar(),
        MUIA_Family_Child, obj->MN_Project_Quit,
    End;


  // Create Tune menu
    menu2 = MenuitemObject,
        MUIA_Menuitem_Title, GetTFString(MSG_STATE_STATION_DETAILS),  // "Tune Details"
        MUIA_Family_Child, obj->MN_Tune_Play,
        MUIA_Family_Child, obj->MN_Tune_Stop,
        MUIA_Family_Child, MakeMenuBar(),
        MUIA_Family_Child, obj->MN_Tune_Save,
    End;
  // Create menu strip
  obj->MN_Main = MenustripObject, MUIA_Family_Child, menu1, MUIA_Family_Child,
  menu2, End;
}

void CreateMenuEvents(struct ObjApp *obj) {
  // Project menu
  DoMethod(obj->MN_Project_About, MUIM_Notify, MUIA_Menuitem_Trigger,
           MUIV_EveryTime, obj->App, 2, MUIM_Application_ReturnID, EVENT_ABOUT);

  DoMethod(obj->MN_Project_About_MUI, MUIM_Notify, MUIA_Menuitem_Trigger,
           MUIV_EveryTime, obj->App, 2, MUIM_Application_ReturnID,
           EVENT_ABOUT_MUI);

  DoMethod(obj->MN_Project_Find, MUIM_Notify, MUIA_Menuitem_Trigger,
           MUIV_EveryTime, obj->App, 2, MUIM_Application_ReturnID, EVENT_FIND);

  DoMethod(obj->MN_Project_Save, MUIM_Notify, MUIA_Menuitem_Trigger,
           MUIV_EveryTime, obj->App, 2, MUIM_Application_ReturnID, EVENT_SAVE);

  DoMethod(obj->MN_Project_Settings, MUIM_Notify, MUIA_Menuitem_Trigger,
           MUIV_EveryTime, obj->App, 2, MUIM_Application_ReturnID,
           EVENT_SETTINGS);

  DoMethod(obj->MN_Project_Settings_MUI, MUIM_Notify, MUIA_Menuitem_Trigger,
           MUIV_EveryTime, obj->App, 2, MUIM_Application_ReturnID,
           EVENT_SETTINGS_MUI);

  DoMethod(obj->MN_Project_Iconify, MUIM_Notify, MUIA_Menuitem_Trigger,
           MUIV_EveryTime, obj->App, 2, MUIM_Application_ReturnID,
           EVENT_ICONIFY);

  DoMethod(obj->MN_Project_Quit, MUIM_Notify, MUIA_Menuitem_Trigger,
           MUIV_EveryTime, obj->App, 2, MUIM_Application_ReturnID,
           MUIV_Application_ReturnID_Quit);
  DoMethod(obj->MN_Project_Favorites, MUIM_Notify, MUIA_Menuitem_Trigger,
         MUIV_EveryTime, obj->App, 2, MUIM_Application_ReturnID, EVENT_FAVORITES);

  // Tune menu
  DoMethod(obj->MN_Tune_Play, MUIM_Notify, MUIA_Menuitem_Trigger,
           MUIV_EveryTime, obj->App, 2, MUIM_Application_ReturnID,
           EVENT_TUNE_PLAY);

  DoMethod(obj->MN_Tune_Stop, MUIM_Notify, MUIA_Menuitem_Trigger,
           MUIV_EveryTime, obj->App, 2, MUIM_Application_ReturnID,
           EVENT_TUNE_STOP);

  DoMethod(obj->MN_Tune_Save, MUIM_Notify, MUIA_Menuitem_Trigger,
           MUIV_EveryTime, obj->App, 2, MUIM_Application_ReturnID,
           EVENT_TUNE_SAVE);
}

BOOL APP_Tune_List_Init(void) {
  DEBUG("%s", "APP_Tune_List_Init()\n");

  DoMethod(objApp->LSV_Tune_List, MUIM_List_Clear);
  set(objApp->LSV_Tune_List, MUIA_List_Quiet, FALSE);
  set(objApp->LSV_Tune_List, MUIA_List_Active, MUIV_List_Active_Top);
  set(objApp->LAB_Tune_Result, MUIA_Text_Contents, "No Tune.");

  return TRUE;
}

BOOL APP_Tune_Details_Init(void) {
  DEBUG("%s", "APP_Tune_Details_Init()\n");

  set(objApp->TXT_Tune_Name, MUIA_Text_Contents, "\033bWelcome to " APP_NAME);
  set(objApp->TXT_Tune_URL, MUIA_Text_Contents, APP_DESCRIPTION);

  set(objApp->BTN_Tune_Play, MUIA_Disabled, TRUE);
  set(objApp->BTN_Tune_Stop, MUIA_Disabled, TRUE);
  set(objApp->BTN_Tune_Save, MUIA_Disabled, TRUE);

  return TRUE;
}

BOOL APP_Settings_Init(void)
{
    struct APISettings settings;
    
    if (LoadSettings(&settings))
    {
        set(objApp->STR_Settings_API_Host, MUIA_String_Contents, settings.host);
        set(objApp->STR_Settings_API_Port, MUIA_String_Integer, settings.port);
        set(objApp->STR_Settings_API_Limit, MUIA_String_Integer, settings.limit);
        set(objApp->STR_Settings_AmigaAmp, MUIA_String_Contents, settings.autostart);
        set(objApp->CHK_Settings_Iconify, MUIA_Selected, settings.iconifyAmigaAMP);
        set(objApp->CYC_Find_Country, MUIA_Cycle_Active, settings.countryCode);
        set(objApp->CYC_Find_Codec, MUIA_Cycle_Active, settings.codec);
        set(objApp->CHK_Settings_QuitAmigaAMP, MUIA_Selected, settings.quitAmigaAMP);

        if (!APP_StartupAmigaAMP(&settings)) {
            DEBUG("Warning: Failed to start AmigaAMP");
        }
    
        return TRUE;
    }
    
    return FALSE;
}

BOOL APP_Settings_API_Limit_Inc(void)
{
    LONG limit;
    static char buf[128];
    
    DEBUG("%s", "APP_Settings_API_Limit_Inc()\n");
    get(objApp->STR_Settings_API_Limit, MUIA_String_Integer, &limit);
    
    if (limit < 10000)
    {
        limit++;
        set(objApp->STR_Settings_API_Limit, MUIA_String_Integer, limit);
    }
    else
    {
        // Show error message
        GetTFFormattedString(buf, sizeof(buf), MSG_ERR_INVALID_LIMIT, limit);
        set(objApp->LAB_Tune_Result, MUIA_Text_Contents, buf);
    }
    
    return TRUE;
}


BOOL APP_Tune_Active(void) {
  LONG index;
  static char buf[1024];

  DEBUG("%s", "APP_Tune_Active()\n");
  get(objApp->LSV_Tune_List, MUIA_List_Active, &index);
  sprintf(buf, "Active index: %ld\n", index);
  DEBUG("%s", buf);

  if (index == MUIV_List_Active_Off) {
    set(objApp->BTN_Tune_Play, MUIA_Disabled, TRUE);
    set(objApp->BTN_Tune_Stop, MUIA_Disabled, TRUE);
    set(objApp->BTN_Tune_Save, MUIA_Disabled, TRUE);
  } else {
    struct Tune *tune;
    DoMethod(objApp->LSV_Tune_List, MUIM_List_GetEntry, index, &tune);

    if (tune) {
      sprintf(buf, "\33b%s", tune->name);
      set(objApp->TXT_Tune_Name, MUIA_Text_Contents, buf);
      sprintf(buf, "\33bURL: \033n%s", tune->url);
      set(objApp->TXT_Tune_URL, MUIA_Text_Contents, buf);
      sprintf(buf, "\033bCodec:\033n %s, \033bBitrate:\033n %s, \033bCountry:\033n %s", tune->codec, tune->bitrate, GetCountryNameFromCode(tune->country));
      set(objApp->TXT_Tune_Details, MUIA_Text_Contents, buf);
      set(objApp->BTN_Tune_Play, MUIA_Disabled, FALSE);
      set(objApp->BTN_Tune_Stop, MUIA_Disabled, FALSE);
      set(objApp->BTN_Tune_Save, MUIA_Disabled, FALSE);
    }
  }

  return TRUE;
}


BOOL APP_Tune_Save(void)
{
    LONG index;
    struct Tune *tune = NULL;
    
    DEBUG("%s", "APP_Tune_Save()\n");
    get(objApp->LSV_Tune_List, MUIA_List_Active, &index);
    
    if (index != MUIV_List_Active_Off)
    {
        DoMethod(objApp->LSV_Tune_List, MUIM_List_GetEntry, index, &tune);
        if (tune)
        {
            if (SaveSingleStationToPLS(tune)) {
                return TRUE;
            }
        }
        UpdateStatusMessage(GetTFString(MSG_ERR_SAVE_FILE));
    }
    
    return FALSE;
}

BOOL APP_Tune_DblClick(void)
{
    LONG index;
    struct Tune *tune = NULL;
    
    DEBUG("%s", "APP_Tune_DblClick()\n");
    get(objApp->LSV_Tune_List, MUIA_List_Active, &index);
    
    if (index != MUIV_List_Active_Off)
    {
        DoMethod(objApp->LSV_Tune_List, MUIM_List_GetEntry, index, &tune);
        if (tune)
        {
            if (!IsAmigaAMPRunning())
            {
                UpdateStatusMessage(GetTFString(MSG_ERR_AMIGAAMP_NOT_RUNNING));
                return FALSE;
            }
            
            if (OpenStreamInAmigaAMPWithName(tune->url, tune->name))
            {
                char buffer[256];
                GetTFFormattedString(buffer, sizeof(buffer), 
                    MSG_STATUS_PLAYING, tune->name);
                UpdateStatusMessage(buffer);
                return TRUE;
            }
            else
            {
                UpdateStatusMessage(GetTFString(MSG_ERR_START_PLAYBACK));
            }
        }
    }
    
    return FALSE;
}


BOOL APP_Iconify(void) {
  DEBUG("%s", "APP_Iconify()\n");
  set(objApp->App, MUIA_Application_Iconified, TRUE);
  return TRUE;
}
BOOL APP_Tune_Play(void)
{
    LONG index;
    struct Tune *tune = NULL;
    
    DEBUG("APP_Tune_Play()\n");
    get(objApp->LSV_Tune_List, MUIA_List_Active, &index);
    
    if (index != MUIV_List_Active_Off)
    {
        DoMethod(objApp->LSV_Tune_List, MUIM_List_GetEntry, index, &tune);
        if (tune)
        {
            if (!IsAmigaAMPRunning())
            {
                set(objApp->LAB_Tune_Result, MUIA_Text_Contents, 
                    GetTFString(MSG_ERR_AMIGAAMP_NOT_RUNNING));
                return FALSE;
            }
            
            if (OpenStreamInAmigaAMPWithName(tune->url, tune->name))
            {
                // Show "Playing: [tune name]" message
                char buffer[256];
                GetTFFormattedString(buffer, sizeof(buffer), 
                    MSG_STATUS_PLAYING, tune->name);
                set(objApp->LAB_Tune_Result, MUIA_Text_Contents, buffer);
                return TRUE;
            }
            else
            {
                set(objApp->LAB_Tune_Result, MUIA_Text_Contents, 
                    GetTFString(MSG_ERR_START_PLAYBACK));
            }
        }
    }
    
    return FALSE;
}

BOOL APP_Tune_Stop(void)
{
    DEBUG("APP_Tune_Stop()\n");
    
    if (!IsAmigaAMPRunning())
    {
        set(objApp->LAB_Tune_Result, MUIA_Text_Contents, 
            GetTFString(MSG_ERR_AMIGAAMP_NOT_RUNNING));
        return FALSE;
    }
    
    if (StopAmigaAMP())
    {
        set(objApp->LAB_Tune_Result, MUIA_Text_Contents, 
            GetTFString(MSG_ERR_PLAYBACK_STOPPED));
        return TRUE;
    }
    
    return FALSE;
}
BOOL APP_Settings_API_Port_Dec(void)
{
    LONG port;
    static char buf[128];
    
    DEBUG("%s", "APP_Settings_API_Port_Dec()\n");
    get(objApp->STR_Settings_API_Port, MUIA_String_Integer, &port);
    
    if (port > 1)
    {
        port--;
        set(objApp->STR_Settings_API_Port, MUIA_String_Integer, port);
    }
    else
    {
        // Show error message
        GetTFFormattedString(buf, sizeof(buf), MSG_ERR_INVALID_PORT, port);
        set(objApp->LAB_Tune_Result, MUIA_Text_Contents, buf);
    }
    
    return TRUE;
}
BOOL APP_Fav_Add(void)
{
    LONG index;
    struct Tune *tune = NULL;
    
    DEBUG("%s", "APP_Fav_Add()\n");
    get(objApp->LSV_Tune_List, MUIA_List_Active, &index);
    
    if (index != MUIV_List_Active_Off)
    {
        DoMethod(objApp->LSV_Tune_List, MUIM_List_GetEntry, index, &tune);
        if (tune)
        {

            DEBUG("Adding favorite - Name: %s, Bitrate: %d", tune->name, tune->bitrate);
            if (!IsTuneInFavorites(tune)) {
                if (SaveFavorite(tune)) {
                    return TRUE;
                }
            } else {
                UpdateStatusMessage("Station already in favorites");
            }
        }
        else
        {
            UpdateStatusMessage(GetTFString(MSG_ERR_ADD_FAV));
        }
    }
    
    return FALSE;
}

BOOL APP_Fav_Remove(void)
{
    LONG index;
    struct Tune *tune = NULL;
    
    DEBUG("%s", "APP_Fav_Remove()\n");
    get(objApp->LSV_Tune_List, MUIA_List_Active, &index);
    
    if (index != MUIV_List_Active_Off)
    {
        DoMethod(objApp->LSV_Tune_List, MUIM_List_GetEntry, index, &tune);
        if (tune)
        {
            if (IsTuneInFavorites(tune)) {
                if (RemoveFavorite(tune)) {
                    BOOL isFavorites = FALSE;
                    get(objApp->MN_Project_Favorites, MUIA_Menuitem_Checked, &isFavorites);
                    if (isFavorites) {
                        APP_ShowFavorites();
                    }
                    return TRUE;
                }
            } else {
                UpdateStatusMessage("Station not in favorites");
            }
        }
        else
        {
            UpdateStatusMessage(GetTFString(MSG_ERR_REMOVE_FAV));
        }
    }
    
    return FALSE;
}

BOOL APP_Settings_Cancel(void)
{
    DEBUG("%s", "APP_Settings_Cancel()\n");
    
    // Could show a message that settings were not saved
    set(objApp->LAB_Tune_Result, MUIA_Text_Contents, 
        GetTFString(MSG_ACTION_CANCEL));  // "Cancel"
    
    // Close settings window
    set(objApp->WIN_Settings, MUIA_Window_Open, FALSE);
    
    return TRUE;
}

BOOL APP_Settings_Save(void)
{
    struct APISettings settings;
    char *tempStr; // Temporary string pointer for MUI string objects
    LONG tempLong; // Temporary long for numeric values
    
    // Initialize settings with defaults
    memset(&settings, 0, sizeof(struct APISettings));
    
    // Get host - using temporary string pointer
    get(objApp->STR_Settings_API_Host, MUIA_String_Contents, &tempStr);
    if (tempStr) {
        strncpy(settings.host, tempStr, sizeof(settings.host)-1);
        settings.host[sizeof(settings.host)-1] = '\0';
        DEBUG("Host setting: %s", settings.host);
    }

    // Get port - using MUIA_String_Integer
    get(objApp->STR_Settings_API_Port, MUIA_String_Integer, &tempLong);
    settings.port = (ULONG)tempLong;
    DEBUG("Port setting: %lu", settings.port);

    // Get limit
    get(objApp->STR_Settings_API_Limit, MUIA_String_Integer, &tempLong);
    settings.limit = (ULONG)tempLong;
    DEBUG("Limit setting: %lu", settings.limit);

    // Get autostart path
    get(objApp->STR_Settings_AmigaAmp, MUIA_String_Contents, &tempStr);
    if (tempStr) {
        strncpy(settings.autostart, tempStr, sizeof(settings.autostart)-1);
        settings.autostart[sizeof(settings.autostart)-1] = '\0';
        DEBUG("Autostart setting: %s", settings.autostart);
    }

    // Get iconify setting
    get(objApp->CHK_Settings_Iconify, MUIA_Selected, &tempLong);
    settings.iconifyAmigaAMP = (BOOL)tempLong;

    // Get AmigaAmp Quit
    get(objApp->CHK_Settings_QuitAmigaAMP, MUIA_Selected, &tempLong);
    settings.quitAmigaAMP = (BOOL)tempLong;
    
    // Get country code
    get(objApp->CYC_Find_Country, MUIA_Cycle_Active, &tempLong);
    settings.countryCode = tempLong;
    
    // Get codec setting
    get(objApp->CYC_Find_Codec, MUIA_Cycle_Active, &tempLong);
    settings.codec = tempLong;

    // Debug output of all settings before saving
    DEBUG("Saving settings:");
    DEBUG("Host: %s", settings.host);
    DEBUG("Port: %lu", settings.port);
    DEBUG("Limit: %lu", settings.limit);
    DEBUG("Autostart: %s", settings.autostart);
    DEBUG("Iconify: %d", settings.iconifyAmigaAMP);
    DEBUG("Country: %ld", settings.countryCode);
    DEBUG("Codec: %ld", settings.codec);

    if (SaveSettings(&settings))
    {
        set(objApp->WIN_Settings, MUIA_Window_Open, FALSE);
        char buf[256];
        GetTFFormattedString(buf, sizeof(buf), MSG_STATUS_SETTINGS_SAVED_HOST, 
                            settings.host, settings.port);
        set(objApp->LAB_Tune_Result, MUIA_Text_Contents, buf);
        return TRUE;
    }
    
    return FALSE;
}
BOOL APP_Settings_API_Limit_Dec(void)
{
    LONG limit;
    static char buf[128];
    
    DEBUG("%s", "APP_Settings_API_Limit_Dec()\n");
    get(objApp->STR_Settings_API_Limit, MUIA_String_Integer, &limit);
    
    if (limit > 1)
    {
        limit--;
        set(objApp->STR_Settings_API_Limit, MUIA_String_Integer, limit);
    }
    else
    {
        // Show error message
        GetTFFormattedString(buf, sizeof(buf), MSG_ERR_INVALID_LIMIT, limit);
        set(objApp->LAB_Tune_Result, MUIA_Text_Contents, buf);
    }
    
    return TRUE;
}

BOOL APP_Settings_MUI(void)
{
    DEBUG("%s", "APP_Settings_MUI()\n");
    
    // Open MUI settings window
    DoMethod(objApp->App, MUIM_Application_OpenConfigWindow, 0,
        APP_ID_WIN_SETTINGS_MUI);
    
    return TRUE;
}

BOOL APP_Settings_API_Port_Inc(void)
{
    LONG port;
    static char buf[128];
    
    DEBUG("%s", "APP_Settings_API_Port_Inc()\n");
    get(objApp->STR_Settings_API_Port, MUIA_String_Integer, &port);
    
    if (port < 65535)
    {
        port++;
        set(objApp->STR_Settings_API_Port, MUIA_String_Integer, port);
    }
    else
    {
        // Show error message
        GetTFFormattedString(buf, sizeof(buf), MSG_ERR_INVALID_PORT, port);
        set(objApp->LAB_Tune_Result, MUIA_Text_Contents, buf);
    }
    
    return TRUE;
}

BOOL APP_Save(void)
{
    struct FileRequester *fileReq;
    char filepath[256];
    
    if (!AslBase) {
        DEBUG("ASL library not available");
        return FALSE;
    }
    
    fileReq = AllocAslRequest(ASL_FileRequest, NULL);
    if (!fileReq) {
        DEBUG("Failed to allocate ASL request");
        return FALSE;
    }

    if (AslRequestTags(fileReq, 
        ASLFR_TitleText, "Save All Stations",
        ASLFR_InitialFile, "radio.pls",
        ASLFR_DoPatterns, TRUE,
        ASLFR_InitialPattern, "#?.pls",
        TAG_DONE)) 
    {
        strcpy(filepath, fileReq->rf_Dir);
        AddPart(filepath, fileReq->rf_File, sizeof(filepath));

        if (!strstr(filepath, ".pls")) {
            strcat(filepath, ".pls");
        }

        if (SaveStationsToPLS(filepath)) {
            UpdateStatusMessage(GetTFString(MSG_STATUS_FILE_SAVED));
            FreeAslRequest(fileReq);
            return TRUE;
        }
    }

    FreeAslRequest(fileReq);
    return FALSE;
}

BOOL APP_Settings(void)
{
    DEBUG("%s", "APP_Settings()\n");
    
    // Before opening settings window, we could show a status message
    set(objApp->LAB_Tune_Result, MUIA_Text_Contents, 
        GetTFString(MSG_STATE_SETTINGS));  // "Settings..."
    
    // Open settings window
    set(objApp->WIN_Settings, MUIA_Window_Open, TRUE);
    
    return TRUE;
}

VOID CreateWindowMain(struct ObjApp *obj) {
  APTR lbl1, lbl2, child1, child2, group0, group1, group2, group3, group4;

  // Controls

  obj->BTN_Find = SimpleButton(GetTFString(MSG_ACTION_SEARCH));
  obj->BTN_Fav_Add = SimpleButton(GetTFString(MSG_ACTION_FAV_ADD)),
  obj->BTN_Fav_Remove = SimpleButton(GetTFString(MSG_ACTION_FAV_REMOVE)),
  obj->BTN_Save = SimpleButton(GetTFString(MSG_ACTION_SAVE_ALL));   // "Save Tunes"
  obj->BTN_Quit = SimpleButton(GetTFString(MSG_ACTION_QUIT));       // "Quit"
  obj->BTN_Tune_Play = SimpleButton(GetTFString(MSG_ACTION_PLAY));  // "Play Tune"
  obj->BTN_Tune_Stop = SimpleButton(GetTFString(MSG_ACTION_STOP));  // "Stop Tune"
  obj->BTN_Tune_Save = SimpleButton(GetTFString(MSG_ACTION_SAVE_ONE)); // "Save Tune"
  obj->STR_Find_Name = StringObject, MUIA_Frame, MUIV_Frame_String, End;
  obj->STR_Find_Tags = StringObject, MUIA_Frame, MUIV_Frame_String, End;
  obj->CYC_Find_Country = CycleObject, MUIA_Frame, MUIV_Frame_Button, End;
  obj->CYC_Find_Codec = CycleObject, MUIA_Frame, MUIV_Frame_Button, End;
  obj->CHK_Find_HTTPS_Only = CheckMark(FALSE);
  obj->CHK_Find_Hide_Broken = CheckMark(FALSE);
    
  lbl1 = Label2(GetTFString(MSG_OPTION_HTTPS_ONLY));    // "HTTPS Only"
  lbl2 = Label2(GetTFString(MSG_OPTION_HIDE_BROKEN));   // "Hide Broken"


  set(obj->BTN_Tune_Play, MUIA_Weight, 25);
  set(obj->BTN_Tune_Stop, MUIA_Weight, 25);
  set(obj->BTN_Tune_Save, MUIA_Weight, 25);

  set(lbl1, MUIA_Weight, 75);
  set(lbl2, MUIA_Weight, 75);
  set(obj->CHK_Find_HTTPS_Only, MUIA_Weight, 25);
  set(obj->CHK_Find_Hide_Broken, MUIA_Weight, 25);

  child1 = GroupObject, MUIA_Group_Columns, 2, MUIA_Group_Rows, 1,
  MUIA_Group_SameWidth, FALSE, Child, lbl1, Child, obj->CHK_Find_HTTPS_Only,
  End;

  child2 = GroupObject, MUIA_Group_Columns, 2, MUIA_Group_Rows, 1,
  MUIA_Group_SameWidth, FALSE, Child, lbl2, Child, obj->CHK_Find_Hide_Broken,
  End;

  obj->LSV_Tune_List = ListviewObject, MUIA_Listview_Input, TRUE,
  MUIA_Listview_DoubleClick, TRUE, MUIA_Listview_DragType,
  MUIV_Listview_DragType_None, MUIA_Listview_MultiSelect,
  MUIV_Listview_MultiSelect_None, MUIA_Listview_ScrollerPos,
  MUIV_Listview_ScrollerPos_Default, 
  MUIA_Listview_List, obj->LSV_Tune_List,
  End;

  // Details

  obj->LAB_Tune_Result = TextObject, MUIA_Text_PreParse, "\033r",
  MUIA_InnerLeft, 0, MUIA_InnerRight, 0, End;

  obj->TXT_Tune_Name = TextObject, MUIA_Background, MUII_WindowBack,
  MUIA_Text_SetMin, TRUE, MUIA_Weight, 75, End;

  obj->TXT_Tune_URL = TextObject, MUIA_Background, MUII_WindowBack,
  MUIA_Text_SetMin, TRUE, MUIA_Weight, 75, End;

  obj->TXT_Tune_Details = TextObject, MUIA_Background, MUII_WindowBack,
  MUIA_Text_SetMin, TRUE, MUIA_Weight, 75, End;

  // Groups

  group1 = GroupObject, MUIA_Frame, MUIV_Frame_Group, MUIA_FrameTitle,
  "Tunes Filters", MUIA_Group_Columns, 4, Child, Label("Name"), Child,
  obj->STR_Find_Name, Child, HSpace(10), Child, child1, Child, Label("Tags"),
  Child, obj->STR_Find_Tags, Child, HSpace(10), Child, child2, Child,
  Label("Codec"), Child, obj->CYC_Find_Codec, Child, HSpace(10), Child,
  HSpace(10), Child, Label("Country"), Child, obj->CYC_Find_Country, Child,
  HSpace(10), Child, obj->BTN_Find, End;

  group2 = GroupObject, MUIA_Frame, MUIV_Frame_Group,
  MUIA_Background, MUII_GroupBack, MUIA_Group_Horiz, FALSE,
  MUIA_Group_SameWidth, TRUE, MUIA_Group_SameHeight, TRUE, Child, VGroup,
  MUIA_Group_SameWidth, TRUE, Child, obj->LSV_Tune_List = ListviewObject,
  MUIA_Listview_List, ListObject, MUIA_Frame, MUIV_Frame_InputList,
  MUIA_List_Active, MUIV_List_Active_Top,
      MUIA_List_Format,
  "BAR MIW=70 P=\033l, BAR MIW=10 P=\033c, BAR MIW=10 P=\033c, BAR MIW=10 "
  "P=\033c",
  MUIA_List_Title, TRUE, MUIA_List_DisplayHook, &DisplayHook,
  End, End, Child,
  obj->LAB_Tune_Result, End, End;

  group3 = GroupObject, MUIA_Frame, MUIV_Frame_Group, MUIA_FrameTitle,
  "Tune Details", MUIA_Group_Columns, 2, Child, obj->TXT_Tune_Name, Child,
  obj->BTN_Tune_Play, Child, obj->TXT_Tune_URL, Child, obj->BTN_Tune_Stop,
  Child, obj->TXT_Tune_Details, Child, obj->BTN_Tune_Save, End;

  group4 = GroupObject,
  MUIA_Group_Rows, 2, 
  MUIA_Group_SameHeight, TRUE, 
  Child, obj->BTN_Fav_Add,
  Child, obj->BTN_Fav_Remove,
  Child, obj->BTN_Save, 
  Child, obj->BTN_Quit,
  End;

  group0 = GroupObject, MUIA_Group_Columns, 1, MUIA_Group_SameWidth, TRUE,
  Child, group1, Child, group2, Child, group3, Child, group4, End;

  // Window

  obj->WIN_Main = WindowObject, MUIA_Window_Title, APP_NAME " - " APP_VERSION, MUIA_Window_ID,
  APP_ID_WIN_MAIN, MUIA_Window_AppWindow, FALSE, MUIA_Window_SizeGadget, TRUE,
  WindowContents, group0, End;

  // Cycle Chain

  DoMethod(obj->WIN_Main, MUIM_Window_SetCycleChain, obj->STR_Find_Name,
           obj->STR_Find_Tags, obj->CYC_Find_Codec, obj->CYC_Find_Country,
           obj->CHK_Find_HTTPS_Only, obj->CHK_Find_Hide_Broken, obj->BTN_Find,
           obj->LSV_Tune_List, obj->BTN_Tune_Play, obj->BTN_Tune_Stop,
           obj->BTN_Tune_Save, obj->BTN_Save, obj->BTN_Fav_Add, obj->BTN_Fav_Remove,
           obj->BTN_Quit, obj->WIN_Settings, 0);
}
void CreateWindowMainEvents(struct ObjApp *obj) {
  // Buttons
  DoMethod(obj->BTN_Find, MUIM_Notify, MUIA_Pressed, FALSE, obj->App, 2,
           MUIM_Application_ReturnID, EVENT_FIND);

  DoMethod(obj->BTN_Tune_Play, MUIM_Notify, MUIA_Pressed, FALSE, obj->App, 2,
           MUIM_Application_ReturnID, EVENT_TUNE_PLAY);

  DoMethod(obj->BTN_Tune_Stop, MUIM_Notify, MUIA_Pressed, TRUE, obj->App, 2,
           MUIM_Application_ReturnID, EVENT_TUNE_STOP);

  DoMethod(obj->BTN_Tune_Save, MUIM_Notify, MUIA_Pressed, TRUE, obj->App, 2,
           MUIM_Application_ReturnID, EVENT_TUNE_SAVE);
  // Fav 
  DoMethod(obj->BTN_Fav_Add, MUIM_Notify, MUIA_Pressed, FALSE,
        obj->App, 2, MUIM_Application_ReturnID, EVENT_FAV_ADD);
        
  DoMethod(obj->BTN_Fav_Remove, MUIM_Notify, MUIA_Pressed, FALSE,
        obj->App, 2, MUIM_Application_ReturnID, EVENT_FAV_REMOVE);
  // Listview
  DoMethod(obj->LSV_Tune_List, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime,
           obj->App, 2, MUIM_Application_ReturnID, EVENT_TUNE_ACTIVE);

  DoMethod(obj->LSV_Tune_List, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE,
           obj->App, 2, MUIM_Application_ReturnID, EVENT_TUNE_DBLCLICK);

  // Controls
  DoMethod(obj->BTN_Save, MUIM_Notify, MUIA_Pressed, FALSE, obj->App, 2,
           MUIM_Application_ReturnID, EVENT_SAVE);

  DoMethod(obj->BTN_Quit, MUIM_Notify, MUIA_Pressed, FALSE, obj->App, 2,
           MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);


  // Window
  DoMethod(obj->WIN_Main, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
           obj->WIN_Main, 3, MUIM_Set, MUIA_Window_Open, FALSE);

  DoMethod(obj->WIN_Main, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj->App,
           2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);



}

void CreateWindowSettings(struct ObjApp *obj) {
  APTR child1, child2;
  APTR group0, group1, group2;

  // Controls
    obj->BTN_Settings_Save = SimpleButton(GetTFString(MSG_ACTION_SAVE_ALL));     // "Save"
    obj->BTN_Settings_Cancel = SimpleButton(GetTFString(MSG_ACTION_CANCEL));     // "Cancel"

  obj->STR_Settings_AmigaAmp = StringObject,
        MUIA_Frame, MUIV_Frame_String,
        MUIA_String_MaxLen, MAX_PATH_LEN-1,
  End;
  obj->BTN_Settings_AmigaAmp_Browse = SimpleButton(GetTFString(MSG_OPTION_BROWSE));  // "Browse"

  APTR pathGroup = HGroup,
      Child, obj->STR_Settings_AmigaAmp,
      Child, obj->BTN_Settings_AmigaAmp_Browse,
  End;

  obj->CHK_Settings_Iconify = CheckMark(FALSE);
  APTR iconifyGroup = HGroup,
      MUIA_Group_SameWidth, FALSE,
      Child, obj->CHK_Settings_Iconify,
      Child, HSpace(0),
      MUIA_Weight, 200,    
  End;

  // Quit AmigaAmp

  obj->CHK_Settings_QuitAmigaAMP = CheckMark(TRUE);

  APTR quitGroup = HGroup,
    MUIA_Group_SameWidth, FALSE,
    Child, obj->CHK_Settings_QuitAmigaAMP,
    Child, HSpace(0),
    MUIA_Weight, 200,
  End;

  // Host (URI string)
  obj->STR_Settings_API_Host = StringObject, MUIA_Frame, MUIV_Frame_String,
  MUIA_String_AdvanceOnCR, TRUE, MUIA_String_Accept, API_HOST_ACCEPT,
  MUIA_String_Format, MUIV_String_Format_Left, MUIA_String_MaxLen,
  API_HOST_MAX_LEN, MUIA_String_Contents, API_HOST_DEFAULT, 
  MUIA_FixWidthTxt, "wwwwwwwwwwwwwwwwwwwwwwwwww", End;

  // Port (Integer 0 to 65535)
  obj->STR_Settings_API_Port = StringObject, MUIA_Frame, MUIV_Frame_String,
  MUIA_String_Accept, API_PORT_ACCEPT, MUIA_String_AdvanceOnCR, TRUE,
  MUIA_String_Format, MUIV_String_Format_Right, MUIA_String_Integer,
  API_PORT_DEFAULT, MUIA_String_MaxLen, API_PORT_MAX_LEN, End;

  obj->BTN_Settings_API_Port_Spc = HSpace(0);
  obj->BTN_Settings_API_Port_Inc = SimpleButton("+");
  obj->BTN_Settings_API_Port_Dec = SimpleButton("-");

  set(obj->BTN_Settings_API_Port_Inc, MUIA_Weight, 0);
  set(obj->BTN_Settings_API_Port_Dec, MUIA_Weight, 0);
  set(obj->BTN_Settings_API_Port_Spc, MUIA_Weight, 80);
  set(obj->STR_Settings_API_Port, MUIA_Weight, 20);

  child1 = GroupObject, MUIA_Group_Columns, 2, MUIA_Group_Rows, 1,
  MUIA_Group_SameSize, FALSE, MUIA_Group_SameWidth, FALSE,
  MUIA_Group_SameHeight, TRUE, Child, obj->STR_Settings_API_Port, Child,
  obj->BTN_Settings_API_Port_Inc, Child, obj->BTN_Settings_API_Port_Dec, Child,
  obj->BTN_Settings_API_Port_Spc, End;

  // Limit (Integer 0 to 99999)
  obj->STR_Settings_API_Limit = StringObject, MUIA_Frame, MUIV_Frame_String,
  MUIA_String_Accept, API_LIMIT_ACCEPT, MUIA_String_AdvanceOnCR, TRUE,
  MUIA_String_Format, MUIV_String_Format_Right, MUIA_String_Integer,
  API_LIMIT_DEFAULT, MUIA_String_MaxLen, API_LIMIT_MAX_LEN, End;

  obj->BTN_Settings_API_Limit_Spc = HSpace(0);
  obj->BTN_Settings_API_Limit_Inc = SimpleButton("+");
  obj->BTN_Settings_API_Limit_Dec = SimpleButton("-");

  set(obj->BTN_Settings_API_Limit_Inc, MUIA_Weight, 0);
  set(obj->BTN_Settings_API_Limit_Dec, MUIA_Weight, 0);
  set(obj->BTN_Settings_API_Limit_Spc, MUIA_Weight, 80);
  set(obj->STR_Settings_API_Limit, MUIA_Weight, 20);

  child2 = GroupObject, MUIA_Group_Columns, 2, MUIA_Group_Rows, 1,
  MUIA_Group_SameSize, FALSE, MUIA_Group_SameWidth, FALSE,
  MUIA_Group_SameHeight, TRUE, Child, obj->STR_Settings_API_Limit, Child,
  obj->BTN_Settings_API_Limit_Inc, Child, obj->BTN_Settings_API_Limit_Dec,
  Child, obj->BTN_Settings_API_Limit_Spc, End;

    group1 = ColGroup(2),
        MUIA_Frame, MUIV_Frame_Group,
        MUIA_HorizWeight, 200, 
        Child, Label(GetTFString(MSG_OPTION_API_HOST)),
        Child, obj->STR_Settings_API_Host,
        Child, Label(GetTFString(MSG_OPTION_API_PORT)),
        Child, child1,
        Child, Label(GetTFString(MSG_GUI_LIMIT)), 
        Child, child2,
        Child, Label(GetTFString(MSG_OPTION_SELECT_PROGRAM)),
        Child, pathGroup,
        Child, Label(GetTFString(MSG_OPTION_ICONIFY_AMIGAAMP)),
        Child, iconifyGroup,
        Child, Label(GetTFString(MSG_OPTION_QUIT_AMIGAAMP)),  // "Quit AmigaAMP on exit"
        Child, quitGroup, 

    End;
    
    group2 = HGroup,
        Child, obj->BTN_Settings_Save,
        Child, HVSpace,
        Child, obj->BTN_Settings_Cancel,
    End;
    
    // Main group
    group0 = VGroup,
        Child, VSpace(2),
        Child, group1,
        Child, VSpace(2),
        Child, group2,
        Child, VSpace(2),
    End;
    
    obj->WIN_Settings = WindowObject,
        MUIA_Window_Title, GetTFString(MSG_STATE_SETTINGS),
        MUIA_Window_ID, APP_ID_WIN_SETTINGS,
        MUIA_Window_SizeGadget, TRUE,
        MUIA_Window_DragBar, TRUE,
        MUIA_Window_CloseGadget, TRUE,
        MUIA_Window_DepthGadget, TRUE,
        WindowContents, group0,
    End;

  // Cycle Chain
  DoMethod(obj->WIN_Settings, MUIM_Window_SetCycleChain,
           obj->STR_Settings_API_Host, obj->STR_Settings_API_Port,
           obj->STR_Settings_API_Limit, obj->STR_Settings_AmigaAmp,
           obj->BTN_Settings_AmigaAmp_Browse, obj->CHK_Settings_Iconify, 
           obj->BTN_Settings_Save, obj->BTN_Settings_Cancel, 0);
}

void CreateWindowSettingsEvents(struct ObjApp *obj) {
  // Port buttons
  DoMethod(obj->BTN_Settings_API_Port_Inc, MUIM_Notify, MUIA_Pressed, FALSE,
           obj->App, 2, MUIM_Application_ReturnID, EVENT_SETTINGS_API_PORT_INC);

  DoMethod(obj->BTN_Settings_API_Port_Dec, MUIM_Notify, MUIA_Pressed, FALSE,
           obj->App, 2, MUIM_Application_ReturnID, EVENT_SETTINGS_API_PORT_DEC);

  // Limit buttons
  DoMethod(obj->BTN_Settings_API_Limit_Inc, MUIM_Notify, MUIA_Pressed, FALSE,
           obj->App, 2, MUIM_Application_ReturnID,
           EVENT_SETTINGS_API_LIMIT_INC);

  DoMethod(obj->BTN_Settings_API_Limit_Dec, MUIM_Notify, MUIA_Pressed, FALSE,
           obj->App, 2, MUIM_Application_ReturnID,
           EVENT_SETTINGS_API_LIMIT_DEC);

  // Control buttons
  DoMethod(obj->BTN_Settings_Save, MUIM_Notify, MUIA_Pressed, FALSE, obj->App,
           2, MUIM_Application_ReturnID, EVENT_SETTINGS_SAVE);

  DoMethod(obj->BTN_Settings_Cancel, MUIM_Notify, MUIA_Pressed, FALSE, obj->App,
           2, MUIM_Application_ReturnID, EVENT_SETTINGS_CANCEL);
  //
    DoMethod(obj->BTN_Settings_AmigaAmp_Browse, MUIM_Notify, MUIA_Pressed, FALSE,
  obj->App, 2, MUIM_Application_ReturnID, EVENT_SETTINGS_BROWSE_AMIGAAMP);

  // Window
  DoMethod(obj->WIN_Settings, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
           obj->App, 2, MUIM_Application_ReturnID, EVENT_SETTINGS_CANCEL);
}

struct ObjApp *CreateApp(void) {
    struct ObjApp *obj;

    DEBUG("%s", "Creating App...\n");

    if ((obj = AllocVec(sizeof(struct ObjApp), MEMF_PUBLIC | MEMF_CLEAR))) {
        DEBUG("%s", "Memory allocated\n");

        /* Create main window */
        CreateMenu(obj);
        CreateWindowMain(obj);
        CreateWindowSettings(obj);

        if (obj->WIN_Main) {
            /* Create Application object */
            obj->App = ApplicationObject,
                MUIA_Application_Title,        APP_NAME,
                MUIA_Application_Version,      APP_VERSTRING,
                MUIA_Application_Copyright,    APP_COPYRIGHT,
                MUIA_Application_Author,       APP_AUTHORS,
                MUIA_Application_Description,  APP_DESCRIPTION,
                MUIA_Application_Base,         APP_NAME,
                MUIA_Application_Menustrip,    obj->MN_Main,
                SubWindow,                     obj->WIN_Main,
                SubWindow,                     obj->WIN_Settings,
                End;

            if (obj->App) {
                DEBUG("%s", "Application object created\n");

                /* Create MUI events */
                DEBUG("%s", "Creating Menu Events...\n");
                CreateMenuEvents(obj);

                DEBUG("%s", "Creating Window Events...\n");
                CreateWindowMainEvents(obj);

                DEBUG("%s", "Creating Settings Events...\n");
                CreateWindowSettingsEvents(obj);

                DEBUG("%s", "App creation complete\n");
                return obj;
            }
        }
    }
    return NULL;
}

void DisposeApp(struct ObjApp *obj)
{
    if (obj)
    {
        if (obj->App)
        {
            MUI_DisposeObject(obj->App);
        }
        
        // Free country configuration
        FreeCountryConfig(&obj->countryConfig);
        
        FreeVec(obj);
    }
}

BOOL APP_ShowFavorites(void)
{
    LONG count = 0;
    struct Tune *favorites;
    
    DEBUG("APP_ShowFavorites()\n");

    set(objApp->MN_Project_Favorites, MUIA_Menuitem_Checked, TRUE);
    
    // Clear existing list
    DoMethod(objApp->LSV_Tune_List, MUIM_List_Clear);
    set(objApp->LSV_Tune_List, MUIA_List_Quiet, TRUE);
    
    // Load favorites
    favorites = LoadFavorites(&count);
    if (favorites)
    {
        for(int i = 0; i < count; i++)
        {
            // Allocate new tune structure for each entry
            struct Tune *tune = AllocVec(sizeof(struct Tune), MEMF_CLEAR);
            if (tune)
            {
                tune->name = strdup(favorites[i].name);
                tune->codec = strdup(favorites[i].codec);
                tune->bitrate = favorites[i].bitrate;
                tune->country = strdup(favorites[i].country);
                tune->url = strdup(favorites[i].url);

                DoMethod(objApp->LSV_Tune_List, MUIM_List_InsertSingle, tune, MUIV_List_Insert_Bottom);
            }
        }
        
        // Free loaded favorites
        free(favorites);
        
        set(objApp->LSV_Tune_List, MUIA_List_Quiet, FALSE);
        char buf[256];
        sprintf(buf, "Loaded %ld favorites", count);
        set(objApp->LAB_Tune_Result, MUIA_Text_Contents, buf);
    }
    else
    {
        set(objApp->LAB_Tune_Result, MUIA_Text_Contents, "No favorites found");
    }
    
    return TRUE;
}
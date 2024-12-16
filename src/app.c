#define MUI_OBSOLETE

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <ctype.h>
#include <exec/memory.h>
#include <exec/types.h>
#include <intuition/intuition.h>
#include <libraries/mui.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/app.h"
#include "../include/main.h"
#include "../include/locale.h"

#include "SDI_compiler.h"
#include "SDI_hook.h"
#include "SDI_lib.h"
#include "SDI_stdarg.h"
#ifdef __GNUC__
extern void geta4(void);
#endif
struct ObjApp *objApp;  // Global variable definition

HOOKPROTONH(DisplayCode, VOID, char **array, struct Tune *tune) {
    //static char buf[1024];
    //static char buf[128];
    //sprintf(buf, "Instance tune size: %ld, string length: %ld\n", 
    //    (long)sizeof(*tune),                    // Size of the tune structure
    //    (long)strlen(tune->tu_Name));           // Length of name string
    //PutStr(buf);

  if (tune != NULL) {
    //  sprintf(buf, "Active tune: %d\n", sizeof(*tune));
    //  PutStr(buf);
    *array++ = tune->tu_Name;
    *array++ = tune->tu_Codec;
    *array++ = tune->tu_BitRate;
    *array = tune->tu_Country;
  } else {
    // Column headers
    *array++ = "\033c\033uName";
    *array++ = "\033c\033uCodec";
    *array++ = "\033c\033uBitRate";
    *array = "\033c\033uCountry";
  }
}
MakeStaticHook(DisplayHook, DisplayCode);

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
    
    obj->MN_Tune_Play = MakeMenuItem(GetTFString(MSG_ACTION_PLAY), "P");     // "Play Tune"
    obj->MN_Tune_Stop = MakeMenuItem(GetTFString(MSG_ACTION_STOP), "T");     // "Stop Tune"
    obj->MN_Tune_Save = MakeMenuItem(GetTFString(MSG_ACTION_SAVE_ONE), NULL); // "Save Tune"


  // Create Project menu
    menu1 = MenuitemObject,
        MUIA_Menuitem_Title, GetTFString(MSG_STATE_PROJECT),  // "Project"
        MUIA_Family_Child, obj->MN_Project_Find,
        MUIA_Family_Child, obj->MN_Project_Save,
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
  PutStr("APP_Tune_List_Init()\n");

  DoMethod(objApp->LSV_Tune_List, MUIM_List_Clear);
  set(objApp->LSV_Tune_List, MUIA_List_Quiet, FALSE);
  set(objApp->LSV_Tune_List, MUIA_List_Active, MUIV_List_Active_Top);
  set(objApp->LAB_Tune_Result, MUIA_Text_Contents, "No Tune.");

  return TRUE;
}

BOOL APP_Tune_Details_Init(void) {
  PutStr("APP_Tune_Details_Init()\n");

  set(objApp->TXT_Tune_Name, MUIA_Text_Contents, "\033bWelcome to " APP_NAME);
  set(objApp->TXT_Tune_URL, MUIA_Text_Contents, "By " APP_AUTHORS);
  set(objApp->TXT_Tune_Details, MUIA_Text_Contents, APP_DESCRIPTION);

  set(objApp->BTN_Tune_Play, MUIA_Disabled, TRUE);
  set(objApp->BTN_Tune_Stop, MUIA_Disabled, TRUE);
  set(objApp->BTN_Tune_Save, MUIA_Disabled, TRUE);

  return TRUE;
}

BOOL APP_Settings_Init(void) {
  PutStr("APP_Settings_Init()\n");

  set(objApp->STR_Settings_API_Host, MUIA_String_Contents, API_HOST_DEFAULT);
  set(objApp->STR_Settings_API_Port, MUIA_String_Integer, API_PORT_DEFAULT);
  set(objApp->STR_Settings_API_Limit, MUIA_String_Integer, API_LIMIT_DEFAULT);

  return TRUE;
}

BOOL APP_Settings_API_Limit_Inc(void)
{
    LONG limit;
    static char buf[128];
    
    PutStr("APP_Settings_API_Limit_Inc()\n");
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

  PutStr("APP_Tune_Active()\n");
  get(objApp->LSV_Tune_List, MUIA_List_Active, &index);
  sprintf(buf, "Active index: %ld\n", index);
  PutStr(buf);

  if (index == MUIV_List_Active_Off) {
    set(objApp->BTN_Tune_Play, MUIA_Disabled, TRUE);
    set(objApp->BTN_Tune_Stop, MUIA_Disabled, TRUE);
    set(objApp->BTN_Tune_Save, MUIA_Disabled, TRUE);
  } else {
    struct Tune *tune;
    DoMethod(objApp->LSV_Tune_List, MUIM_List_GetEntry, index, &tune);

    if (tune) {
      sprintf(buf, "\33b%s", tune->tu_Name);
      set(objApp->TXT_Tune_Name, MUIA_Text_Contents, buf);
      set(objApp->TXT_Tune_URL, MUIA_Text_Contents, tune->tu_Description);
      sprintf(buf, "%s, %s, %s", tune->tu_Codec, tune->tu_BitRate,
              tune->tu_Country);
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
    static char buf[128];
    struct Tune *tune = NULL;
    
    PutStr("APP_Tune_Save()\n");
    get(objApp->LSV_Tune_List, MUIA_List_Active, &index);
    
    if (index != MUIV_List_Active_Off)
    {
        DoMethod(objApp->LSV_Tune_List, MUIM_List_GetEntry, index, &tune);
        if (tune)
        {
            // Create save message with tune name
            GetTFFormattedString(buf, sizeof(buf), MSG_STATUS_FILE_SAVED, tune->tu_Name);
            set(objApp->LAB_Tune_Result, MUIA_Text_Contents, buf);
        }
        else
        {
            // Show error message
            set(objApp->LAB_Tune_Result, MUIA_Text_Contents, 
                GetTFString(MSG_ERR_SAVE_FILE));
        }
    }
    
    return TRUE;
}


BOOL APP_Tune_DblClick(void)
{
    LONG index;
    static char buf[128];
    struct Tune *tune = NULL;
    
    PutStr("APP_Tune_DblClick()\n");
    get(objApp->LSV_Tune_List, MUIA_List_Active, &index);
    
    if (index != MUIV_List_Active_Off)
    {
        DoMethod(objApp->LSV_Tune_List, MUIM_List_GetEntry, index, &tune);
        if (tune)
        {
            // Start playing on double click
            GetTFFormattedString(buf, sizeof(buf), MSG_STATUS_PLAYING, tune->tu_Name);
            set(objApp->LAB_Tune_Result, MUIA_Text_Contents, buf);
        }
    }
    
    return TRUE;
}


BOOL APP_Iconify(void) {
  PutStr("APP_Iconify()\n");
  set(objApp->App, MUIA_Application_Iconified, TRUE);
  return TRUE;
}

BOOL APP_Tune_Play(void)
{
    LONG index;
    static char buf[128];
    struct Tune *tune = NULL;
    
    PutStr("APP_Tune_Play()\n");
    get(objApp->LSV_Tune_List, MUIA_List_Active, &index);
    
    if (index != MUIV_List_Active_Off)
    {
        DoMethod(objApp->LSV_Tune_List, MUIM_List_GetEntry, index, &tune);
        if (tune)
        {
            // Show "Playing: [tune name]" message
            GetTFFormattedString(buf, sizeof(buf), MSG_STATUS_PLAYING, tune->tu_Name);
            set(objApp->LAB_Tune_Result, MUIA_Text_Contents, buf);
        }
        else
        {
            // Show error message
            set(objApp->LAB_Tune_Result, MUIA_Text_Contents, 
                GetTFString(MSG_ERR_START_PLAYBACK));
        }
    }
    
    return TRUE;
}

BOOL APP_Tune_Stop(void)
{
    PutStr("APP_Tune_Stop()\n");
    
    // Show "Playback stopped" message
    set(objApp->LAB_Tune_Result, MUIA_Text_Contents, 
        GetTFString(MSG_ERR_PLAYBACK_STOPPED));
    
    return TRUE;
}
BOOL APP_Settings_API_Port_Dec(void)
{
    LONG port;
    static char buf[128];
    
    PutStr("APP_Settings_API_Port_Dec()\n");
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
    static char buf[128];
    struct Tune *tune = NULL;
    
    PutStr("APP_Fav_Add()\n");
    get(objApp->LSV_Tune_List, MUIA_List_Active, &index);
    
    if (index != MUIV_List_Active_Off)
    {
        DoMethod(objApp->LSV_Tune_List, MUIM_List_GetEntry, index, &tune);
        if (tune)
        {
            set(objApp->LAB_Tune_Result, MUIA_Text_Contents, 
                GetTFString(MSG_STATUS_ADDED_FAV));  // "Added to favorites"
        }
        else
        {
            // Show error
            set(objApp->LAB_Tune_Result, MUIA_Text_Contents, 
                GetTFString(MSG_ERR_ADD_FAV));  // "Failed to add to favorites"
        }
    }
    
    return TRUE;
}

BOOL APP_Fav_Remove(void)
{
    LONG index;
    static char buf[128];
    struct Tune *tune = NULL;
    
    PutStr("APP_Fav_Remove()\n");
    get(objApp->LSV_Tune_List, MUIA_List_Active, &index);
    
    if (index != MUIV_List_Active_Off)
    {
        DoMethod(objApp->LSV_Tune_List, MUIM_List_GetEntry, index, &tune);
        if (tune)
        {
            set(objApp->LAB_Tune_Result, MUIA_Text_Contents, 
                GetTFString(MSG_STATUS_REMOVED_FAV));  // "Removed from favorites"
        }
        else
        {
            // Show error
            set(objApp->LAB_Tune_Result, MUIA_Text_Contents, 
                GetTFString(MSG_ERR_REMOVE_FAV));  // "Failed to remove from favorites"
        }
    }
    
    return TRUE;
}

BOOL APP_Settings_Cancel(void)
{
    PutStr("APP_Settings_Cancel()\n");
    
    // Could show a message that settings were not saved
    set(objApp->LAB_Tune_Result, MUIA_Text_Contents, 
        GetTFString(MSG_ACTION_CANCEL));  // "Cancel"
    
    // Close settings window
    set(objApp->WIN_Settings, MUIA_Window_Open, FALSE);
    
    return TRUE;
}

BOOL APP_Settings_Save(void) {
  static char buf[512];
  STRPTR apiHost;
  ULONG apiPort, apiLimit;

  PutStr("APP_Settings_Save()\n");

  get(objApp->STR_Settings_API_Host, MUIA_String_Contents, &apiHost);
  get(objApp->STR_Settings_API_Port, MUIA_String_Integer, &apiPort);
  get(objApp->STR_Settings_API_Limit, MUIA_String_Integer, &apiLimit);

  GetTFFormattedString(buf, sizeof(buf), MSG_STATUS_SETTINGS_SAVED_HOST, 
                        apiHost, apiPort);
  DEBUG("%s", buf);


  set(objApp->WIN_Settings, MUIA_Window_Open, FALSE);

  return TRUE;
}

BOOL APP_Settings_API_Limit_Dec(void)
{
    LONG limit;
    static char buf[128];
    
    PutStr("APP_Settings_API_Limit_Dec()\n");
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
    PutStr("APP_Settings_MUI()\n");
    
    // Open MUI settings window
    DoMethod(objApp->App, MUIM_Application_OpenConfigWindow, 0,
        APP_ID_WIN_SETTINGS_MUI);
    
    return TRUE;
}

BOOL APP_Settings_API_Port_Inc(void)
{
    LONG port;
    static char buf[128];
    
    PutStr("APP_Settings_API_Port_Inc()\n");
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
    PutStr("APP_Save()\n");
    
    // Assuming we're saving all tunes
    set(objApp->LAB_Tune_Result, MUIA_Text_Contents, 
        GetTFString(MSG_STATUS_SETTINGS_SAVED));  // "Settings saved."
    
    return TRUE;
}

BOOL APP_Settings(void)
{
    PutStr("APP_Settings()\n");
    
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

  group2 = GroupObject, MUIA_Frame, MUIV_Frame_Group, MUIA_FrameTitle,
  "Tunes List", MUIA_Background, MUII_GroupBack, MUIA_Group_Horiz, FALSE,
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

  obj->WIN_Main = WindowObject, MUIA_Window_Title, APP_NAME, MUIA_Window_ID,
  APP_ID_WIN_MAIN, MUIA_Window_AppWindow, FALSE, MUIA_Window_SizeGadget, TRUE,
  WindowContents, group0, End;

  // Cycle Chain

  DoMethod(obj->WIN_Main, MUIM_Window_SetCycleChain, obj->STR_Find_Name,
           obj->STR_Find_Tags, obj->CYC_Find_Codec, obj->CYC_Find_Country,
           obj->CHK_Find_HTTPS_Only, obj->CHK_Find_Hide_Broken, obj->BTN_Find,
           obj->LSV_Tune_List, obj->BTN_Tune_Play, obj->BTN_Tune_Stop,
           obj->BTN_Tune_Save, obj->BTN_Save, obj->BTN_Fav_Add, obj->BTN_Fav_Remove,
           obj->BTN_Quit, 0);
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
        MUIA_String_MaxLen, 256,
  End;
    obj->BTN_Settings_AmigaAmp_Browse = SimpleButton(GetTFString(MSG_OPTION_BROWSE));  // "Browse"

    APTR pathGroup = HGroup,
        Child, obj->STR_Settings_AmigaAmp,
        Child, obj->BTN_Settings_AmigaAmp_Browse,
    End;

  obj->CHK_Settings_Iconify = CheckMark(FALSE);
  // Host (URI string)
  obj->STR_Settings_API_Host = StringObject, MUIA_Frame, MUIV_Frame_String,
  MUIA_String_AdvanceOnCR, TRUE, MUIA_String_Accept, API_HOST_ACCEPT,
  MUIA_String_Format, MUIV_String_Format_Left, MUIA_String_MaxLen,
  API_HOST_MAX_LEN, MUIA_Weight, 200, End;

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

  // Groups
group1 = GroupObject,
    MUIA_Frame, MUIV_Frame_Group,
    MUIA_FrameTitle, GetTFString(MSG_STATE_SETTINGS),    // "Settings"
    MUIA_Group_SameWidth, FALSE,
    MUIA_Group_Horiz, FALSE,
    MUIA_Group_Columns, 2,
    Child, Label(GetTFString(MSG_OPTION_API_HOST)),      // "API Host :"
    Child, obj->STR_Settings_API_Host,
    Child, Label(GetTFString(MSG_OPTION_API_PORT)),      // "API Port :"
    Child, child1,
    Child, Label(GetTFString(MSG_GUI_LIMIT)),           // "API Limit :"
    Child, child2,
    Child, Label(GetTFString(MSG_OPTION_SELECT_PROGRAM)), // "AmigaAmp Path :"
    Child, pathGroup,
    Child, Label(GetTFString(MSG_OPTION_ICONIFY_AMIGAAMP)), // "Iconify AmigaAmp :"
    Child, obj->CHK_Settings_Iconify,
End;

  group2 = GroupObject, MUIA_Group_Rows, 1, Child, obj->BTN_Settings_Save,
  Child, HSpace(0), Child, obj->BTN_Settings_Cancel, End;

  group0 = GroupObject, MUIA_Group_Columns, 1, Child, group1, Child, group2,
  End;

  // Window
  obj->WIN_Settings = WindowObject, MUIA_Window_Title, GetTFString(MSG_STATE_SETTINGS),
  MUIA_Window_ID, APP_ID_WIN_SETTINGS, MUIA_Window_AppWindow, FALSE, MUIA_Window_Width, 500, MUIA_Window_DragBar, TRUE, MUIA_Window_CloseGadget, TRUE,  
  MUIA_Window_NoMenus, TRUE, MUIA_Window_Width, MUIV_Window_Width_Scaled,
  WindowContents, group0, End;

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

  PutStr("Creating App...\n");

  if ((obj = AllocVec(sizeof(struct ObjApp), MEMF_PUBLIC | MEMF_CLEAR))) {
    PutStr("Memory allocated\n");

    // Create main window
    CreateMenu(obj);
    CreateWindowMain(obj);
    CreateWindowSettings(obj);

    if (obj->WIN_Main) {
      // Create Application object
      obj->App = ApplicationObject, MUIA_Application_Title, APP_NAME,
      MUIA_Application_Version, APP_VERSTRING, MUIA_Application_Copyright,
      APP_COPYRIGHT, MUIA_Application_Author, APP_AUTHORS,
      MUIA_Application_Description, APP_DESCRIPTION, MUIA_Application_Base,
      APP_NAME, MUIA_Application_Menustrip, obj->MN_Main, SubWindow,
      obj->WIN_Main, SubWindow, obj->WIN_Settings, End;

      if (obj->App) {
        PutStr("Application object created\n");

        // Create MUI events
        PutStr("Creating Menu Events...\n");
        CreateMenuEvents(obj);

        PutStr("Creating Window Events...\n");
        CreateWindowMainEvents(obj);

        PutStr("Creating Settings Events...\n");
        CreateWindowSettingsEvents(obj);

        PutStr("App creation complete\n");
        return obj;
      }
    }

    FreeVec(obj);
  }

  return NULL;
}
void DisposeApp(struct ObjApp *obj) {
  if (obj) {
    if (obj->App) {
      MUI_DisposeObject(obj->App);
    }
    FreeVec(obj);
  }
}
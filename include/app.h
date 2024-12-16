#ifndef APP_H
#define APP_H

#include <exec/types.h>
#include "main.h"

// Main App functions
struct ObjApp *CreateApp(void);
void DisposeApp(struct ObjApp *obj);

// Initialization functions
BOOL APP_Find_Init(void);
BOOL APP_Tune_List_Init(void);
BOOL APP_Tune_Details_Init(void);
BOOL APP_Settings_Init(void);

// Event handlers
BOOL APP_About(void);
BOOL APP_About_MUI(void);
BOOL APP_Find(void);
BOOL APP_Iconify(void);
BOOL APP_Save(void);
BOOL APP_Sleep(void);

// Settings related functions
BOOL APP_Settings(void);
BOOL APP_Settings_MUI(void);
BOOL APP_Settings_API_Port_Inc(void);
BOOL APP_Settings_API_Port_Dec(void);
BOOL APP_Settings_API_Limit_Inc(void);
BOOL APP_Settings_API_Limit_Dec(void);
BOOL APP_Settings_Save(void);
BOOL APP_Settings_Cancel(void);

// Tune related functions
BOOL APP_Tune_Active(void);
BOOL APP_Tune_DblClick(void);
BOOL APP_Tune_Play(void);
BOOL APP_Tune_Stop(void);
BOOL APP_Tune_Save(void);
BOOL APP_Fav_Add(void);
BOOL APP_Fav_Remove(void);

// Internal functions declarations
void CreateMenu(struct ObjApp *obj);
void CreateMenuEvents(struct ObjApp *obj);
void CreateWindowMain(struct ObjApp *obj);
void CreateWindowMainEvents(struct ObjApp *obj);
void CreateWindowSettings(struct ObjApp *obj);
void CreateWindowSettingsEvents(struct ObjApp *obj);

#endif // APP_H
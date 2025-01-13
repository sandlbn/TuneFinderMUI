#ifndef TUNEFINDER_MUI_MAIN_H
#define TUNEFINDER_MUI_MAIN_H

#include <exec/types.h>
#include <intuition/intuition.h>
#include <libraries/mui.h>
#include "locale.h"
#include "country_config.h"

#define HOOKFUNC LONG (*)(struct Hook *, APTR, APTR)
#define ENV_PATH "ENVARC:TuneFinder/"

#ifndef MAKE_ID
#define MAKE_ID(a, b, c, d) \
  ((ULONG)(a) << 24 | (ULONG)(b) << 16 | (ULONG)(c) << 8 | (ULONG)(d))
#endif

#ifdef DEBUG_BUILD
    #define DEBUG(msg, ...) printf("DEBUG [%s:%d]: " msg "\n", __func__, __LINE__, ##__VA_ARGS__)
#else
    #define DEBUG(msg, ...) ((void)0)  // Does nothing in release build
#endif


// MUI helper macros
#define MakeMenuBar() MenuitemObject, MUIA_Menuitem_Title, "", End

#define MakeMenuItem(title, shortcut)                                 \
  MenuitemObject, MUIA_Menuitem_Title, title, MUIA_Menuitem_Shortcut, \
      shortcut, End

// Application defines
#define APP_NAME "TuneFinder MUI"
#define APP_DATE "12.01.2025"
#define APP_VERSION "0.6-beta"
#define APP_VERSTRING "$VER: " APP_NAME " " APP_VERSION " (" APP_DATE ")"
#define APP_AUTHORS "Coding: Marcin Spoczynski\nGUI Design: Philippe Carpentier\nIcons: Thomas Blatt\n"
#define APP_COPYRIGHT "Free to use and distribute"
#define APP_DESCRIPTION "An Internet Radio browser for AmigaOS 3.x"
#define APP_HELPFILE APP_NAME ".guide"
#define MAX_STATUS_MSG_LEN 256

// Window IDs
#define APP_ID_WIN_MAIN MAKE_ID('T', 'F', 'M', '0')
#define APP_ID_WIN_SETTINGS MAKE_ID('T', 'F', 'M', '1')
#define APP_ID_WIN_SETTINGS_MUI MAKE_ID('T', 'F', 'M', '2')

// API Settings
#define API_HOST_ACCEPT \
  "0123456789abcdefghijklmnopqrstuvwxyz:/?#[]@!$&'()*+,;=%-_.~"
#define API_HOST_DEFAULT "de1.api.radio-browser.info"
#define API_HOST_MAX_LEN (1024)

#define API_PORT_ACCEPT "0123456789"
#define API_PORT_DEFAULT (80)
#define API_PORT_MAX_LEN (6)

#define API_LIMIT_ACCEPT "0123456789"
#define API_LIMIT_DEFAULT (100)
#define API_LIMIT_MAX_LEN (6)

// Event IDs
enum EVENT_IDS {
  EVENT_ABOUT = 100,
  EVENT_ABOUT_MUI,
  EVENT_FIND,
  EVENT_ICONIFY,
  EVENT_QUIT,
  EVENT_SAVE,
  EVENT_SETTINGS,
  EVENT_SETTINGS_API_PORT_INC,
  EVENT_SETTINGS_API_PORT_DEC,
  EVENT_SETTINGS_API_LIMIT_INC,
  EVENT_SETTINGS_API_LIMIT_DEC,
  EVENT_SETTINGS_MUI,
  EVENT_SETTINGS_SAVE,
  EVENT_SETTINGS_CANCEL,
  EVENT_TUNE_ACTIVE,
  EVENT_TUNE_DBLCLICK,
  EVENT_TUNE_PLAY,
  EVENT_TUNE_STOP,
  EVENT_TUNE_SAVE,
  EVENT_FAVORITES,
  EVENT_SETTINGS_BROWSE_AMIGAAMP,
  EVENT_FAV_ADD,
  EVENT_FAV_REMOVE
  };

// Structures
struct Tune {
    struct Node node; 
    char *name;     
    char *url;        
    char *codec;  
    char *country;     
    int bitrate;     
};

struct ObjApp {
  APTR App;

  APTR WIN_About;
  APTR WIN_Main;
  APTR WIN_Settings;

  APTR MN_Main;
  APTR MN_Project_Save;
  APTR MN_Project_Find;
  APTR MN_Project_Iconify;
  APTR MN_Project_About;
  APTR MN_Project_About_MUI;
  APTR MN_Project_Settings;
  APTR MN_Project_Settings_MUI;
  APTR MN_Project_Favorites;
  APTR MN_Project_Quit;
  APTR MN_Tune_Play;
  APTR MN_Tune_Stop;
  APTR MN_Tune_Save;
  APTR BTN_Fav_Add;
  APTR BTN_Fav_Remove;
  APTR BTN_Find;
  APTR BTN_Quit;
  APTR BTN_Save;
  APTR STR_Find_Name;
  APTR STR_Find_Tags;
  APTR CYC_Find_Codec;
  APTR CYC_Find_Country;
  APTR CHK_Find_Hide_Broken;
  APTR CHK_Find_HTTPS_Only;
  APTR LSV_Tune_List;
  APTR LAB_Tune_Result;
  APTR TXT_Tune_Name;
  APTR TXT_Tune_URL;
  APTR TXT_Tune_Details;
  APTR BTN_Tune_Play;
  APTR BTN_Tune_Stop;
  APTR BTN_Tune_Save;
  APTR BTN_Favorites;

  APTR STR_Settings_API_Host;
  APTR STR_Settings_API_Port;
  APTR BTN_Settings_API_Port_Inc;
  APTR BTN_Settings_API_Port_Dec;
  APTR BTN_Settings_API_Port_Spc;
  APTR STR_Settings_API_Limit;
  APTR BTN_Settings_API_Limit_Inc;
  APTR BTN_Settings_API_Limit_Dec;
  APTR BTN_Settings_API_Limit_Spc;
  APTR BTN_Settings_Save;
  APTR BTN_Settings_Cancel;
  APTR BTN_Settings_AmigaAmp_Browse;
  APTR BTN_Settings;
  APTR STR_Settings_AmigaAmp;   // String for AmigaAmp path
  APTR CHK_Settings_Iconify;     // Checkbox for iconify option
  APTR CHK_Settings_QuitAmigaAMP;
  struct CountryConfig countryConfig;

};

// Function prototypes
extern struct ObjApp *CreateApp(void);
extern void DisposeApp(struct ObjApp *);

#endif  // TUNEFINDER_MUI_MAIN_H
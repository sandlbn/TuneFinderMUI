#ifndef LOCALE_H
#define LOCALE_H

#include <exec/types.h>

// String IDs
enum {
    // GUI Labels (1-8)
    MSG_GUI_NAME = 1,
    MSG_GUI_COUNTRY,
    MSG_GUI_CODEC,
    MSG_GUI_TAGS,
    MSG_GUI_BITRATE,
    MSG_GUI_UNKNOWN,
    MSG_GUI_LIMIT,
    MSG_GUI_URL,

    // GUI Actions (10-18)
    MSG_ACTION_SEARCH = 10,
    MSG_ACTION_SAVE_ALL,
    MSG_ACTION_CANCEL,
    MSG_ACTION_PLAY,
    MSG_ACTION_QUIT,
    MSG_ACTION_SAVE_ONE,
    MSG_ACTION_STOP,
    MSG_ACTION_FAV_ADD,
    MSG_ACTION_FAV_REMOVE,

    // GUI States (20-28)
    MSG_STATE_READY = 20,
    MSG_STATE_SETTINGS,
    MSG_STATE_API_SETTINGS,
    MSG_STATE_STATION_DETAILS,
    MSG_STATE_PROJECT,
    MSG_STATE_ABOUT,
    MSG_STATE_SEARCHING,
    MSG_STATE_FAVORITES,
    MSG_STATE_ICONIFY,

    // Options and Settings (30-37)
    MSG_OPTION_API_HOST = 30,
    MSG_OPTION_API_PORT,
    MSG_OPTION_HTTPS_ONLY,
    MSG_OPTION_HIDE_BROKEN,
    MSG_OPTION_AUTOSTART,
    MSG_OPTION_BROWSE,
    MSG_OPTION_SELECT_PROGRAM,
    MSG_OPTION_ICONIFY_AMIGAAMP,

    // Status Messages (40-49)
    MSG_STATUS_FOUND_STATIONS = 40,
    MSG_STATUS_PLAYING,
    MSG_STATUS_SETTINGS_LOADED,
    MSG_STATUS_SETTINGS_SAVED,
    MSG_STATUS_SEARCH_COMPLETE,
    MSG_STATUS_FILE_SAVED,
    MSG_STATUS_SETTINGS_SAVED_HOST,
    MSG_STATUS_NO_STATIONS,
    MSG_STATUS_ADDED_FAV,
    MSG_STATUS_REMOVED_FAV,

    // Error Messages - Settings (50-56)
    MSG_ERR_INVALID_PORT = 50,
    MSG_ERR_WRITE_PORT,
    MSG_ERR_WRITE_HOST,
    MSG_ERR_INVALID_LIMIT,
    MSG_ERR_WRITE_LIMIT,
    MSG_ERR_ADD_FAV,
    MSG_ERR_REMOVE_FAV,

    // Error Messages - File Operations (60-66)
    MSG_ERR_SAVE_FILE = 60,
    MSG_ERR_ACCESS_FILE,
    MSG_ERR_CREATE_PORT_FILE,
    MSG_ERR_CREATE_HOST_FILE,
    MSG_ERR_CREATE_LIMIT_FILE,
    MSG_ERR_CREATE_AUTOSTART_FILE,
    MSG_ERR_WRITE_AUTOSTART,

    // Error Messages - Directory Operations (70-71)
    MSG_ERR_DIR_CREATED = 70,
    MSG_ERR_DIR_CREATE_FAILED,

    // Error Messages - Network (80-87)
    MSG_ERR_HTTP_REQUEST = 80,
    MSG_ERR_RESOLVE_HOST,
    MSG_ERR_CONNECT,
    MSG_ERR_SEND_REQUEST,
    MSG_ERR_TIMEOUT,
    MSG_ERR_CREATE_SOCKET,
    MSG_ERR_ALLOC_BUFFERS,
    MSG_ERR_INVALID_HOST,

    // Error Messages - External Programs (90-92)
    MSG_ERR_START_PLAYBACK = 90,
    MSG_ERR_AMIGAAMP_NOT_RUNNING,
    MSG_ERR_PLAYBACK_STOPPED
};

// External references
//extern struct Library *LocaleBase;
//extern struct Catalog *Catalog;

// Function prototypes
BOOL InitLocaleSystem(void);
void CleanupLocaleSystem(void);
const char *GetTFString(LONG stringNum);
void GetTFFormattedString(char *buffer, size_t bufSize, LONG stringNum, ...);

#endif // LOCALE_H
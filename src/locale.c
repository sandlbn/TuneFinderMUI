#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/locale.h>
#include <stdarg.h>  // For va_list
#include <stdio.h>   // For vsnprintf

#include "../include/locale.h"
#include "../include/main.h"

#define CATALOG_NAME "tunefinder.catalog"
#define CATALOG_VERSION 1

// static struct Library *LocaleBase = NULL;
static struct Catalog *Catalog = NULL;

// Built-in default strings (English)
// locale.c
static const char *built_in_strings[] = {
    // GUI Labels (1-8)
    "Name",         // 1
    "Country",      // 2
    "Codec",        // 3
    "Tags",         // 4
    "BitRate",      // 5
    "Unknown",      // 6
    "Limit",        // 7
    "URL",          // 8
    NULL,           // 9

    // GUI Actions (10-18)
    "Find Tunes",   // 10
    "Save Tunes",   // 11
    "Cancel",       // 12
    "Play Tune",    // 13
    "Quit",         // 14
    "Save Tune",    // 15
    "Stop Tune",    // 16
    "Fav+",         // 17
    "Fav-",         // 18
    NULL,           // 19

    // GUI States (20-28)
    "Ready",        // 20
    "Settings...",  // 21
    "API Settings", // 22
    "Tune Details", // 23
    "Project",      // 24
    "About",        // 25
    "Searching...", // 26
    "Favorites",    // 27
    "Iconify",      // 28
    NULL,           // 29

    // Options and Settings (30-37)
    "API Host",     // 30
    "API Port",     // 31
    "HTTPS Only",   // 32
    "Hide Broken",  // 33
    "Autostart",    // 34
    "Browse",       // 35
    "Select Program", // 36
    "Iconify AmigaAmp", // 37
    NULL,           // 38
    NULL,           // 39

    // Status Messages (40-49)
    "Found %d tune(s), in 0.8 second(s)", // 40
    "Playing: %s",   // 41
    "Settings loaded.", // 42
    "Settings saved.", // 43
    "Search completed. Found %d tunes.", // 44
    "File saved: %s", // 45
    "Settings saved: %s:%u", // 46
    "No tunes.",      // 47
    "Added to favorites", // 48
    "Removed from favorites", // 49

    // Error Messages - Settings (50-56)
    "Invalid port number, keeping current: %ld", // 50
    "Failed to write port setting",        // 51
    "Failed to write host setting",        // 52
    "Invalid limit number, keeping current: %ld", // 53
    "Failed to write limit setting",       // 54
    "Failed to add to favorites",          // 55
    "Failed to remove from favorites",     // 56
    NULL,           // 57
    NULL,           // 58
    NULL,           // 59

    // Error Messages - File Operations (60-66)
    "Failed to save file",                 // 60
    "Failed to access %s",                 // 61
    "Failed to create port settings file: %s", // 62
    "Failed to create host settings file: %s", // 63
    "Failed to create limit settings file: %s", // 64
    "Failed to create autostart settings file: %s", // 65
    "Failed to write autostart setting",   // 66
    NULL,           // 67
    NULL,           // 68
    NULL,           // 69

    // Error Messages - Directory Operations (70-71)
    "Created settings directory: %s",      // 70
    "Failed to create directory: %s",      // 71
    NULL,           // 72
    NULL,           // 73
    NULL,           // 74
    NULL,           // 75
    NULL,           // 76
    NULL,           // 77
    NULL,           // 78
    NULL,           // 79

    // Error Messages - Network (80-87)
    "HTTP request failed",                 // 80
    "Failed to resolve host",              // 81
    "Failed to connect to server",         // 82
    "Failed to send request",              // 83
    "Timeout waiting for data",            // 84
    "Failed to create socket",             // 85
    "Failed to allocate buffers",          // 86
    "Invalid host",                        // 87
    NULL,           // 88
    NULL,           // 89

    // Error Messages - External Programs (90-92)
    "Failed to start playback",            // 90
    "AmigaAMP is not running",            // 91
    "Playback stopped",                    // 92
    "Failed to create request",            // 93
    "HTTP request failed",                 // 94
    "Failed to parse response",            // 95
    "Searching",                          // 96
    "Failed to create socket",            // 97
    "Failed to allocate buffers",         // 98
    "Failed to resolve host",             // 99
    "Failed to connect to server",        // 100
    "Failed to send request",             // 101
    "Connection timeout",                 // 102


    NULL  // Final terminator
};
BOOL InitLocaleSystem(void) {
  // Open locale.library
  LocaleBase = OpenLibrary("locale.library", 38);
  if (!LocaleBase) {
    DEBUG("Failed to open locale.library");
    return FALSE;
  }

  // Try to open catalog for current locale
  Catalog = OpenCatalog(NULL, CATALOG_NAME,
                        TAG_DONE);  // No tags needed

  // Note: It's OK if catalog doesn't open - we'll use built-in strings
  if (!Catalog) {
    DEBUG("No catalog found - using built-in strings");
  }

  return TRUE;
}

void CleanupLocaleSystem(void) {
  if (Catalog) {
    CloseCatalog(Catalog);
    Catalog = NULL;
  }
  if (LocaleBase) {
    CloseLibrary(LocaleBase);
    LocaleBase = NULL;
  }
}

const char *GetTFString(LONG stringNum) {
  const char *str;

  // Adjust for 0-based array vs 1-based catalog IDs
  LONG arrayIndex = stringNum - 1;

  // Check bounds
  if (arrayIndex < 0 ||
      arrayIndex >= (LONG)(sizeof(built_in_strings) / sizeof(char *) - 1)) {
    DEBUG("String ID %ld out of range", stringNum);
    return "???";
  }

  // Try to get string from catalog
  if (Catalog) {
    str = GetCatalogStr(Catalog, stringNum, built_in_strings[arrayIndex]);
  } else {
    str = built_in_strings[arrayIndex];
  }

  return str;
}

void GetTFFormattedString(char *buffer, size_t bufSize, LONG stringNum, ...) {
  va_list args;
  const char *format;

  // Get the format string (either from catalog or built-in)
  format = GetTFString(stringNum);

  // Format the string with the variable arguments
  va_start(args, stringNum);
  vsnprintf(buffer, bufSize, format, args);
  va_end(args);
}
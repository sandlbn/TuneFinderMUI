#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <ctype.h>
#include <dos/dos.h>
#include <exec/memory.h>
#include <exec/types.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/gadtools.h>
#include <proto/intuition.h>
#include <libraries/asl.h>
#include <clib/asl_protos.h>
#include <proto/asl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/main.h"
#include "../include/settings.h"
#include "../include/data.h"

struct ObjApp *objApp;  // Global variable definition

void *allocate(size_t size, int type) { return AllocVec(size, MEMF_CLEAR); }

void deallocate(void *ptr, int type) { FreeVec(ptr); }



static inline int isPrintableASCII(unsigned char c) {
  return c >= 32 && c <= 126;  // Standard ASCII printable range
}

void SanitizeAmigaFilename(const char *input, char *output, size_t maxLen) {
  size_t i, j = 0;       // Changed to size_t to match maxLen type
  int lastWasSpace = 1;  // Start true to prevent leading space

  if (strncmp(input, "Title", 5) == 0) {
    const char *p = strchr(input, '=');
    if (p) input = p + 1;
  }

  // Skip leading spaces
  while (*input == ' ') input++;

  for (i = 0; input[i] && j < (maxLen - 5);
       i++) {  // Added parentheses for clarity
    unsigned char c = (unsigned char)input[i];

    // Only process ASCII printable characters
    if (!isPrintableASCII(c)) {
      continue;  // Skip non-ASCII characters
    }

    // Convert character to valid filename character
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9')) {
      // Direct copy of alphanumeric
      if (lastWasSpace) {
        output[j++] = (char)toupper(c);  // Cast result of toupper
      } else {
        output[j++] = c;
      }
      lastWasSpace = 0;
    } else if (c == ' ' || c == '-' || c == '_' || c == '.') {
      // Convert spaces and similar to underscore, but collapse multiple
      if (!lastWasSpace && j < (maxLen - 5)) {
        output[j++] = '_';
        lastWasSpace = 1;
      }
    }
    // Ignore all other characters
  }

  // Remove trailing underscore if present
  if (j > 0 && output[j - 1] == '_') {
    j--;
  }

  output[j] = '\0';

  // Ensure we have at least one character
  if (j == 0) {
    strcpy(output, "station");
  }

  // Truncate if too long (leaving room for .pls)
  if (strlen(output) > 14) {
    output[14] = '\0';
  }
}

BOOL EnsureSettingsPath(void) {
  BPTR lock;
  char msg[MAX_STATUS_MSG_LEN];
  const char *ENV_ROOT = "ENVARC:";

  lock = Lock(ENV_ROOT, ACCESS_READ);
  if (!lock) {
    snprintf(msg, MAX_STATUS_MSG_LEN, "Failed to access %s", ENV_ROOT);
    DEBUG("%s", msg);
    return FALSE;
  }
  UnLock(lock);

  // Check if settings directory exists
  lock = Lock(CONFIG_PATH, ACCESS_READ);
  if (lock) {
    UnLock(lock);
    return TRUE;
  }

  // Try to create settings directory
  lock = CreateDir(CONFIG_PATH);
  if (lock) {
    UnLock(lock);
    snprintf(msg, MAX_STATUS_MSG_LEN, "Created settings directory: %s",
             CONFIG_PATH);
    DEBUG("%s", msg);
    return TRUE;
  }

  snprintf(msg, MAX_STATUS_MSG_LEN, "Failed to create directory: %s", CONFIG_PATH);
  DEBUG("%s", msg);
  return FALSE;
}

void cleanNonAscii(char *dst, const char *src, size_t maxLen) {
  if (!dst || !src || maxLen == 0) {
    DEBUG("Invalid parameters in cleanNonAscii");
    return;
  }

  int i, j;
  for (i = 0, j = 0; src[i] != '\0' && j < maxLen - 1; i++) {
    // Only copy printable ASCII characters (32-126)
    if (src[i] >= 32 && src[i] <= 126) {
      dst[j++] = src[i];
    }
  }
  dst[j] = '\0';  // Ensure null termination

  if (j == 0) {
    DEBUG("No valid characters found in source string");
    strcpy(dst, "Unknown");
  }
} 
void UpdateStatusMessage(const char *msg)
{
    if (objApp && objApp->LAB_Tune_Result)
    {
        set(objApp->LAB_Tune_Result, MUIA_Text_Contents, msg);
    }
}

BOOL SaveStationsToPLS(const char *filename)
{
    BPTR file;
    LONG count = 0;
    struct Tune *tune;
    char buffer[512];
    
    // Get number of entries in list
    get(objApp->LSV_Tune_List, MUIA_List_Entries, &count);
    if (count == 0) {
        DEBUG("No stations to save");
        return FALSE;
    }

    file = Open(filename, MODE_NEWFILE);
    if (!file) {
        DEBUG("Failed to open file: %s", filename);
        return FALSE;
    }

    // Write header
    FPuts(file, "[playlist]\n");
    sprintf(buffer, "NumberOfEntries=%ld\n", count);
    FPuts(file, buffer);

    // Write entries
    for (LONG i = 0; i < count; i++) {
        DoMethod(objApp->LSV_Tune_List, MUIM_List_GetEntry, i, &tune);
        if (tune) {
            sprintf(buffer, "File%ld=%s\n", i + 1, tune->url);
            FPuts(file, buffer);
            sprintf(buffer, "Title%ld=%s\n", i + 1, tune->name);
            FPuts(file, buffer);
            sprintf(buffer, "Length%ld=-1\n", i + 1);
            FPuts(file, buffer);
        }
    }

    Close(file);
    return TRUE;
}

BOOL SaveSingleStationToPLS(const struct Tune *station)
{
    struct FileRequester *fileReq;
    char filepath[256];
    char sanitized_name[32];  // Increased for safety

    if (!station) return FALSE;

    // Sanitize the station name for use as filename
    SanitizeAmigaFilename(station->name, sanitized_name, sizeof(sanitized_name) - 5); // Leave room for .pls
    strcat(sanitized_name, ".pls");

    fileReq = AllocAslRequestTags(ASL_FileRequest,
        ASLFR_TitleText, "Save Station Playlist",
        ASLFR_InitialFile, sanitized_name,
        ASLFR_DoPatterns, TRUE,
        ASLFR_InitialPattern, "#?.pls",
        TAG_DONE);

    if (!fileReq) return FALSE;

    if (AslRequest(fileReq, NULL)) {
        strcpy(filepath, fileReq->rf_Dir);
        AddPart(filepath, fileReq->rf_File, sizeof(filepath));

        // Add .pls extension if missing
        if (!strstr(filepath, ".pls")) {
            strcat(filepath, ".pls");
        }

        BPTR file = Open(filepath, MODE_NEWFILE);
        if (file) {
            char buffer[512];

            FPuts(file, "[playlist]\n");
            FPuts(file, "NumberOfEntries=1\n");
            
            sprintf(buffer, "File1=%s\n", station->url);
            FPuts(file, buffer);
            
            sprintf(buffer, "Title1=%s\n", station->name);
            FPuts(file, buffer);
            
            FPuts(file, "Length1=-1\n");
            
            Close(file);
            DisplayBeep(NULL);
            UpdateStatusMessage(GetTFString(MSG_STATUS_FILE_SAVED));
            return TRUE;
        }
    }

    FreeAslRequest(fileReq);
    return FALSE;

}

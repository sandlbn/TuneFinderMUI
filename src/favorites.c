#include <proto/dos.h>
#include <proto/exec.h>
#include <clib/alib_protos.h>
#include <dos/dos.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/favorites.h"
#include "../include/locale.h"
#include "../include/main.h"
#include "../include/utils.h"
#include "../include/settings.h"  

struct ObjApp *objApp; 

BOOL SaveFavorite(const struct Tune *tune) {
    BPTR file;
    char filepath[256];
    BOOL success = FALSE;
    
    sprintf(filepath, FAVORITES_CONFIG);
    
    // Open file in append mode
    file = Open(filepath, MODE_OLDFILE);
    if (!file) {
        file = Open(filepath, MODE_NEWFILE);
        if (!file) {
            UpdateStatusMessage(GetTFString(MSG_ERR_ADD_FAV));
            return FALSE;
        }
    }
    
    // Seek to end
    Seek(file, 0, OFFSET_END);
    
    // Write station data: name|url|codec|country|bitrate
    char buffer[1024];
    sprintf(buffer, "%s|%s|%s|%s|%s\n", 
            tune->name ? tune->name : "",
            tune->url ? tune->url : "",
            tune->codec ? tune->codec : "",
            tune->country ? tune->country : "",
            tune->bitrate);
            
    LONG len = strlen(buffer);
    if (Write(file, buffer, len) == len) {
        success = TRUE;
        UpdateStatusMessage(GetTFString(MSG_STATUS_ADDED_FAV));
    } else {
        UpdateStatusMessage(GetTFString(MSG_ERR_ADD_FAV));
    }
    
    Close(file);
    return success;
}

BOOL IsTuneInFavorites(const struct Tune *tune) {
    BPTR file;
    char buffer[1024];
    char filepath[256];
    BOOL found = FALSE;

    if (!tune || !tune->url) return FALSE;

    sprintf(filepath, FAVORITES_CONFIG);

    file = Open(filepath, MODE_OLDFILE);
    if (!file) return FALSE;

    while (FGets(file, buffer, sizeof(buffer))) {
        char *url_start = strchr(buffer, '|');
        if (url_start) {
            url_start++; // Skip the separator
            char *url_end = strchr(url_start, '|');
            if (url_end) {
                *url_end = '\0';
                if (strcmp(url_start, tune->url) == 0) {
                    found = TRUE;
                    break;
                }
            }
        }
    }

    Close(file);
    return found;
}

BOOL RemoveFavorite(const struct Tune *tune) {
    BPTR oldFile, newFile;
    char buffer[1024];
    char tempPath[256];
    char filepath[256];
    BOOL found = FALSE;

    sprintf(filepath, FAVORITES_CONFIG);
    sprintf(tempPath, TUNEFINDER_DIR "favorites.tmp");

    oldFile = Open(filepath, MODE_OLDFILE);
    if (!oldFile) return FALSE;

    newFile = Open(tempPath, MODE_NEWFILE);
    if (!newFile) {
        Close(oldFile);
        return FALSE;
    }

    // Copy all lines except the one to remove
    while (FGets(oldFile, buffer, sizeof(buffer))) {
        char *url_start = strchr(buffer, '|');
        if (url_start) {
            url_start++; // Skip the separator
            char *url_end = strchr(url_start, '|');
            if (url_end) {
                *url_end = '\0';
                if (strcmp(url_start, tune->url) != 0) {
                    // Not the station to remove, write it to new file
                    *url_end = '|'; // Restore separator
                    Write(newFile, buffer, strlen(buffer));
                } else {
                    found = TRUE;
                }
            }
        }
    }

    Close(oldFile);
    Close(newFile);

    if (found) {
        // Replace old file with new one
        DeleteFile(filepath);
        Rename(tempPath, filepath);
        UpdateStatusMessage(GetTFString(MSG_STATUS_REMOVED_FAV));
        return TRUE;
    } else {
        DeleteFile(tempPath);
        return FALSE;
    }
}
struct Tune *LoadFavorites(LONG *count) {
    BPTR file;
    char buffer[1024];
    struct Tune *favorites = NULL;
    LONG maxEntries = 100;  // Initial allocation size
    LONG curEntry = 0;
    char filepath[256];
    char *token;

    *count = 0;
    sprintf(filepath, FAVORITES_CONFIG);

    file = Open(filepath, MODE_OLDFILE);
    if (!file) {
        DEBUG("No favorites file found");
        return NULL;
    }

    // Initial allocation
    favorites = AllocVec(sizeof(struct Tune) * maxEntries, MEMF_CLEAR);
    if (!favorites) {
        Close(file);
        return NULL;
    }

    while (FGets(file, buffer, sizeof(buffer))) {
        char *saveptr;

        // Remove newline if present
        char *newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';

        // Check if we need to grow array
        if (curEntry >= maxEntries) {
            LONG newSize = maxEntries * 2;
            struct Tune *newArray = AllocVec(sizeof(struct Tune) * newSize, MEMF_CLEAR);
            if (!newArray) {
                break;  // Keep what we have so far
            }
            // Copy existing entries
            CopyMem(favorites, newArray, sizeof(struct Tune) * maxEntries);
            FreeVec(favorites);
            favorites = newArray;
            maxEntries = newSize;
        }

        // Parse line: name|url|codec|country|bitrate
        token = strtok_r(buffer, "|", &saveptr);
        if (token) favorites[curEntry].name = strdup(token);

        token = strtok_r(NULL, "|", &saveptr);
        if (token) favorites[curEntry].url = strdup(token);

        token = strtok_r(NULL, "|", &saveptr);
        if (token) favorites[curEntry].codec = strdup(token);

        token = strtok_r(NULL, "|", &saveptr);
        if (token) favorites[curEntry].country = strdup(token);

        token = strtok_r(NULL, "|", &saveptr);
        if (token) favorites[curEntry].bitrate = strdup(token);

        // Only increment if we got all fields
        if (favorites[curEntry].name && favorites[curEntry].url) {
            curEntry++;
        }
    }

    Close(file);

    *count = curEntry;
    
    if (curEntry == 0) {
        // No valid entries found
        if (favorites) FreeVec(favorites);
        return NULL;
    }

    return favorites;
}
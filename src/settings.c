#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <proto/asl.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>
#include <clib/asl_protos.h>
#include <dos/dos.h>
#include "../include/main.h"
#include "../include/locale.h"
#include "../include/settings.h"
#include "../include/main.h"
#include "../include/utils.h"

BOOL SaveSettings(const struct APISettings *settings) {
    char filepath[256];
    char portStr[MAX_PORT_LEN];
    char limitStr[MAX_PORT_LEN];
    BPTR file = NULL;
    BOOL success = FALSE;
    char msg[MAX_STATUS_MSG_LEN];
    
    if (!EnsureSettingsPath()) {
        return FALSE;
    }
    
    // Save host
    sprintf(filepath, TUNEFINDER_DIR ENV_HOST);
    file = Open(filepath, MODE_NEWFILE);
    if (!file) {
        char msg[MAX_STATUS_MSG_LEN];
        GetTFFormattedString(msg, sizeof(msg), MSG_ERR_FAILED_CREAT_HOST_SET_FILE, filepath);
        UpdateStatusMessage(msg);
        return FALSE;
    }
    
    LONG len = strlen(settings->host);
    if (Write(file, settings->host, len) != len) {
        UpdateStatusMessage(GetTFString(MSG_ERR_FAILED_CREAT_HOST_SET_FILE));
        Close(file);
        file = NULL;
        return FALSE;
    }
    Close(file);
    
    // Save port
    sprintf(filepath, TUNEFINDER_DIR ENV_PORT);
    file = Open(filepath, MODE_NEWFILE);
    if (!file) {
        char msg[MAX_STATUS_MSG_LEN];
        GetTFFormattedString(msg, sizeof(msg), MSG_FAILED_CREAT_PORT_FILE, filepath);
        UpdateStatusMessage(msg);
        return FALSE;
    }
    
    len = snprintf(portStr, sizeof(portStr), "%u", (unsigned int)settings->port);
    if (Write(file, portStr, len) != len) {
        UpdateStatusMessage(GetTFString(MSG_FAILED_WRITE_PORT_SET));
        Close(file);
        return FALSE;
    }
    Close(file);
        // Save country code
    sprintf(filepath, TUNEFINDER_DIR ENV_COUNTRY);
    file = Open(filepath, MODE_NEWFILE);
    if (file)
    {
        char countryStr[8];
        len = sprintf(countryStr, "%ld", settings->countryCode);
        if (Write(file, countryStr, len) != len)
        {
            Close(file);
            return FALSE;
        }
        Close(file);
    }

    // Save codec
    sprintf(filepath, TUNEFINDER_DIR ENV_CODEC);
    file = Open(filepath, MODE_NEWFILE);
    if (file)
    {
        char codecStr[8];
        len = sprintf(codecStr, "%ld", settings->codec);
        if (Write(file, codecStr, len) != len)
        {
            Close(file);
            return FALSE;
        }
        Close(file);
    }
    // Save limit
    sprintf(filepath, TUNEFINDER_DIR ENV_LIMIT);
    file = Open(filepath, MODE_NEWFILE);
    if (!file) {
        char msg[MAX_STATUS_MSG_LEN];
        GetTFFormattedString(msg, sizeof(msg), MSG_FAILED_CREAT_LIMIT_FILE, filepath);
        UpdateStatusMessage(msg);
        return FALSE;
    }
    
    len = snprintf(limitStr, sizeof(limitStr), "%u", (unsigned int)settings->limit);
    if (Write(file, limitStr, len) != len) {
        UpdateStatusMessage(GetTFString(MSG_FAILED_WRITE_LIMIT_SET));
        Close(file);
        return FALSE;
    }
    Close(file);
    // Save autostart program
    sprintf(filepath, TUNEFINDER_DIR ENV_AUTOSTART);
    file = Open(filepath, MODE_NEWFILE);
    if (!file) {
        char msg[MAX_STATUS_MSG_LEN];
        GetTFFormattedString(msg, sizeof(msg), MSG_FAILED_CREAT_AUTO_FILE, filepath);
        UpdateStatusMessage(msg);
        return FALSE;
    }
    len = strlen(settings->autostart);
    if (Write(file, settings->autostart, len) != len) {
        UpdateStatusMessage(GetTFString(MSG_FAILED_WRITE_AUTO_SET));
        Close(file);
        return FALSE;
    }
    Close(file);
    // Save iconify setting
    sprintf(filepath, TUNEFINDER_DIR ENV_ICONIFY_AMIGAAMP);
    file = Open(filepath, MODE_NEWFILE);
    if (file) {
        char value = settings->iconifyAmigaAMP ? '1' : '0';
        Write(file, &value, 1);
        Close(file);
    }
    GetTFFormattedString(msg, sizeof(msg), MSG_SET_SAVED,  settings->host, settings->port, settings->limit);
    UpdateStatusMessage(msg);
    return TRUE;
}

BOOL LoadSettings(struct APISettings *settings) {
    BOOL hostLoaded = FALSE;
    BOOL portLoaded = FALSE;
    BOOL limitLoaded = FALSE;
    BOOL defaultProgram = FALSE;
    char filepath[256];
    char portStr[MAX_PORT_LEN];
    char limitStr[MAX_PORT_LEN];
    BPTR file;
    char msg[MAX_STATUS_MSG_LEN];
    
    // Set defaults first
    strncpy(settings->host, API_HOST_DEFAULT, MAX_HOST_LEN-1);
    settings->host[MAX_HOST_LEN-1] = '\0';
    settings->port = API_HOST_DEFAULT; 
    settings->limit = API_LIMIT_DEFAULT;
    
    // Load host
    sprintf(filepath, TUNEFINDER_DIR ENV_HOST);
    file = Open(filepath, MODE_OLDFILE);
    if (file) {
        LONG len = Read(file, settings->host, MAX_HOST_LEN-1);
        if (len > 0) {
            settings->host[len] = '\0';
            hostLoaded = TRUE;
        }
        Close(file);
    } 
    // Load country code
    sprintf(filepath, TUNEFINDER_DIR ENV_COUNTRY);
    file = Open(filepath, MODE_OLDFILE);
    if (file)
    {
        char buffer[8];
        LONG len = Read(file, buffer, sizeof(buffer) - 1);
        if (len > 0)
        {
            buffer[len] = '\0';
            settings->countryCode = atoi(buffer);
        }
        Close(file);
    }
    else
    {
        settings->countryCode = 0;  // Default value
    }

    // Load codec
    sprintf(filepath, TUNEFINDER_DIR ENV_CODEC);
    file = Open(filepath, MODE_OLDFILE);
    if (file)
    {
        char buffer[8];
        LONG len = Read(file, buffer, sizeof(buffer) - 1);
        if (len > 0)
        {
            buffer[len] = '\0';
            settings->codec = atoi(buffer);
        }
        Close(file);
    }
    else
    {
        settings->codec = 0;  // Default value
    }
    // Load port
    sprintf(filepath, TUNEFINDER_DIR ENV_PORT);
    file = Open(filepath, MODE_OLDFILE);
    if (file) {
        memset(portStr, 0, sizeof(portStr));
        LONG len = Read(file, portStr, sizeof(portStr) - 1);
        if (len > 0) {
            portStr[len] = '\0';
            unsigned int tempPort = 0;
            if (sscanf(portStr, "%u", &tempPort) == 1) {
                settings->port = (UWORD)tempPort;
                portLoaded = TRUE;
            } else {
                GetTFFormattedString(msg, sizeof(msg), MSG_INVALID_PORT, API_PORT_DEFAULT);
                UpdateStatusMessage(msg);
            }
        }
        Close(file);
    }

    // Load limit
    sprintf(filepath, TUNEFINDER_DIR ENV_LIMIT);
    file = Open(filepath, MODE_OLDFILE);
    if (file) {
        memset(limitStr, 0, sizeof(limitStr));
        LONG len = Read(file, limitStr, sizeof(limitStr) - 1);
        if (len > 0) {
            limitStr[len] = '\0';
            int tempLimit = 0;
            if (sscanf(limitStr, "%d", &tempLimit) == 1 && tempLimit >= 0) {
                settings->limit = tempLimit;
                limitLoaded = TRUE;
            } else {
                GetTFFormattedString(msg, sizeof(msg), MSG_ERR_INVALID_PORT, API_LIMIT_DEFAULT);
                UpdateStatusMessage(msg);
            }
        }
        Close(file);
    }
    sprintf(filepath, TUNEFINDER_DIR ENV_AUTOSTART);
    file = Open(filepath, MODE_OLDFILE);
    if (file) {
        LONG len = Read(file, settings->autostart, MAX_PATH_LEN-1);
        if (len > 0) {
            settings->autostart[len] = '\0';
            DEBUG("Loaded autostart: %s", settings->autostart);
        }
        Close(file);
        defaultProgram = TRUE;
    } else {
        settings->autostart[0] = '\0';
    }

    sprintf(filepath, TUNEFINDER_DIR ENV_ICONIFY_AMIGAAMP);
    file = Open(filepath, MODE_OLDFILE);
    if (file) {
        char value;
        if (Read(file, &value, 1) == 1) {
            settings->iconifyAmigaAMP = (value == '1');
        }
        Close(file);
    } else {
        settings->iconifyAmigaAMP = FALSE;  // Default to not iconifying
    }

    UpdateStatusMessage(GetTFString(MSG_SETTINGS_LOADED));
    
    return (hostLoaded || portLoaded || limitLoaded || defaultProgram);
}
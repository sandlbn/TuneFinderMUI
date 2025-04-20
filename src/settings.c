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


const struct APISettings DEFAULT_SETTINGS = {
    API_HOST_DEFAULT,  // host
    API_PORT_DEFAULT,  // port
    API_LIMIT_DEFAULT, // limit
    "",               // autostart (empty string)
    FALSE,            // iconifyAmigaAMP
    0,                // countryCode
    0,                // codec
    TRUE,            // exit AmigaAmp true as it is default option
    FALSE

};

BOOL SaveSettings(const struct APISettings *settings) {
    char filepath[256];
    BPTR file = NULL;
    char msg[MAX_STATUS_MSG_LEN];
    
    if (!EnsureSettingsPath()) {
        return FALSE;
    }
    
    // Save host
    sprintf(filepath, TUNEFINDER_DIR ENV_HOST);
    file = Open(filepath, MODE_NEWFILE);
    if (!file) {
        return FALSE;
    }
    LONG len = strlen(settings->host);
    if (Write(file, settings->host, len) != len) {
        Close(file);
        return FALSE;
    }
    Close(file);
    
    // Save port using FPrintf for reliable numeric output
    sprintf(filepath, TUNEFINDER_DIR ENV_PORT);
    file = Open(filepath, MODE_NEWFILE);
    if (!file) {
        return FALSE;
    }
    FPrintf(file, "%ld", settings->port);
    Close(file);

    // Save limit using FPrintf for reliable numeric output
    sprintf(filepath, TUNEFINDER_DIR ENV_LIMIT);
    file = Open(filepath, MODE_NEWFILE);
    if (!file) {
        return FALSE;
    }
    FPrintf(file, "%ld", settings->limit);
    Close(file);

    // Save autostart
    sprintf(filepath, TUNEFINDER_DIR ENV_AUTOSTART);
    file = Open(filepath, MODE_NEWFILE);
    if (!file) {
        return FALSE;
    }
    len = strlen(settings->autostart);
    if (Write(file, settings->autostart, len) != len) {
        Close(file);
        return FALSE;
    }
    Close(file);

    // Save iconify setting
    sprintf(filepath, TUNEFINDER_DIR ENV_ICONIFY_AMIGAAMP);
    file = Open(filepath, MODE_NEWFILE);
    if (file) {
        FPutC(file, settings->iconifyAmigaAMP ? '1' : '0');
        Close(file);
    }

    // Save ENV quit Amigaamp 
    sprintf(filepath, TUNEFINDER_DIR ENV_QUIT_AMIGAAMP);
    file = Open(filepath, MODE_NEWFILE);
    if (file) {
        FPutC(file, settings->quitAmigaAMP ? '1' : '0');
        Close(file);
    }
    
    // Save country code
    sprintf(filepath, TUNEFINDER_DIR ENV_COUNTRY);
    file = Open(filepath, MODE_NEWFILE);
    if (file) {
        // Write number as explicit decimal with newline
        FPrintf(file, "%ld\n", (LONG)settings->countryCode);
        Close(file);
        DEBUG("Saved country code: %ld", (LONG)settings->countryCode);
    }

    // Save codec
    sprintf(filepath, TUNEFINDER_DIR ENV_CODEC);
    file = Open(filepath, MODE_NEWFILE);
    if (file) {
        FPrintf(file, "%ld\n", (LONG)settings->codec);
        Close(file);
        DEBUG("Saved codec: %ld", (LONG)settings->codec);
    }
    // Save HTTPS setting
    sprintf(filepath, TUNEFINDER_DIR ENV_HTTPS);
    file = Open(filepath, MODE_NEWFILE);
    if (file) {
        FPutC(file, settings->httpsOnly ? '1' : '0');
        Close(file);
    }
    return TRUE;
}

BOOL LoadSettings(struct APISettings *settings) {
    char filepath[256];
    BPTR file;
    char buffer[256];
    
    // Set defaults first
    strcpy(settings->host, API_HOST_DEFAULT);
    settings->port = API_PORT_DEFAULT;
    settings->limit = API_LIMIT_DEFAULT;
    settings->autostart[0] = '\0';
    settings->iconifyAmigaAMP = FALSE;
    settings->countryCode = 0;
    settings->codec = 0;
    settings->httpsOnly = FALSE;


    // Load host
    sprintf(filepath, TUNEFINDER_DIR ENV_HOST);
    file = Open(filepath, MODE_OLDFILE);
    if (file) {
        LONG len = Read(file, buffer, sizeof(buffer) - 1);
        if (len > 0) {
            buffer[len] = '\0';
            strncpy(settings->host, buffer, sizeof(settings->host) - 1);
            settings->host[sizeof(settings->host) - 1] = '\0';
        }
        Close(file);
    }
    
    // Load port
    sprintf(filepath, TUNEFINDER_DIR ENV_PORT);
    file = Open(filepath, MODE_OLDFILE);
    if (file) {
        LONG len = Read(file, buffer, sizeof(buffer) - 1);
        if (len > 0) {
            buffer[len] = '\0';
            ULONG tempPort;
            if (sscanf(buffer, "%lu", &tempPort) == 1) {
                settings->port = tempPort;
            }
        }
        Close(file);
    }

    // Load limit 
    sprintf(filepath, TUNEFINDER_DIR ENV_LIMIT);
    file = Open(filepath, MODE_OLDFILE);
    if (file) {
        LONG len = Read(file, buffer, sizeof(buffer) - 1);
        if (len > 0) {
            buffer[len] = '\0';
            ULONG tempLimit;
            if (sscanf(buffer, "%lu", &tempLimit) == 1) {
                settings->limit = tempLimit;
            }
        }
        Close(file);
    }

    // Load autostart path
    sprintf(filepath, TUNEFINDER_DIR ENV_AUTOSTART);
    file = Open(filepath, MODE_OLDFILE);
    if (file) {
        LONG len = Read(file, buffer, sizeof(buffer) - 1);
        if (len > 0) {
            buffer[len] = '\0';
            strncpy(settings->autostart, buffer, sizeof(settings->autostart) - 1);
            settings->autostart[sizeof(settings->autostart) - 1] = '\0';
        }
        Close(file);
    }

    // Load iconify setting
    sprintf(filepath, TUNEFINDER_DIR ENV_ICONIFY_AMIGAAMP);
    file = Open(filepath, MODE_OLDFILE);
    if (file) {
        char value;
        if (Read(file, &value, 1) == 1) {
            settings->iconifyAmigaAMP = (value == '1');
        }
        Close(file);
    }

    // Load AmigaAMp quit

    sprintf(filepath, TUNEFINDER_DIR ENV_QUIT_AMIGAAMP);
    file = Open(filepath, MODE_OLDFILE);
    if (file) {
        char value;
        if (Read(file, &value, 1) == 1) {
            settings->quitAmigaAMP = (value == '1');
        }
        Close(file);
        } else {
        settings->quitAmigaAMP = TRUE;  // Default to true for backward compatibility
    }

    // Load country code
    sprintf(filepath, TUNEFINDER_DIR ENV_COUNTRY);
    file = Open(filepath, MODE_OLDFILE);
    if (file) {
        char buffer[32];
        LONG len = Read(file, buffer, sizeof(buffer) - 1);
        if (len > 0) {
            buffer[len] = '\0';
            // Remove any trailing whitespace or newlines
            while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r' || buffer[len-1] == ' ')) {
                buffer[--len] = '\0';
            }
            DEBUG("Read country string: '%s'", buffer);
            
            // Simple atoi-style conversion
            LONG val = 0;
            char *p = buffer;
            while (*p >= '0' && *p <= '9') {
                val = val * 10 + (*p - '0');
                p++;
            }
            settings->countryCode = val;
            DEBUG("Converted country code: %ld", settings->countryCode);
        }
        Close(file);
    }
    // Loading codec
    sprintf(filepath, TUNEFINDER_DIR ENV_CODEC);
    file = Open(filepath, MODE_OLDFILE);
    if (file) {
    char buffer[32];
    LONG len = Read(file, buffer, sizeof(buffer) - 1);
    if (len > 0) {
        buffer[len] = '\0';
        // Remove any trailing whitespace or newlines
        while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r' || buffer[len-1] == ' ')) {
            buffer[--len] = '\0';
        }
        DEBUG("Read codec string: '%s'", buffer);
        
        // Simple atoi-style conversion
        LONG val = 0;
        char *p = buffer;
        while (*p >= '0' && *p <= '9') {
            val = val * 10 + (*p - '0');
            p++;
        }
        settings->codec = val;
        DEBUG("Converted codec value: %ld", settings->codec);
    }
    Close(file);
    }
    // Load HTTPS setting
    sprintf(filepath, TUNEFINDER_DIR ENV_HTTPS);
    file = Open(filepath, MODE_OLDFILE);
    if (file) {
        char value;
        if (Read(file, &value, 1) == 1) {
            settings->httpsOnly = (value == '1');
        }
        Close(file);
    } else {
        settings->httpsOnly = FALSE;  // Default to false
    }
    return TRUE;
}
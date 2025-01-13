#ifndef SETTINGS_H
#define SETTINGS_H

#include <exec/types.h>
#include "data.h" 
extern const struct APISettings DEFAULT_SETTINGS;
#define TUNEFINDER_DIR "ENVARC:TuneFinderMUI/"
#define CONFIG_PATH "ENVARC:TuneFinderMUI"
#define FULL_COUNTRY_CONFIG_PATH "ENVARC:TuneFinderMUI/countries.cfg"
#define FAVORITES_CONFIG "ENVARC:TuneFinderMUI/favorites.cfg"

#define ENV_HOST "apihost"
#define ENV_PORT "apiport"
#define ENV_LIMIT "apilimit"
#define MAX_HOST_LEN 256
#define MAX_PORT_LEN 6
#define ENV_COUNTRY "country"
#define ENV_CODEC "codec"
#define ENV_AUTOSTART "autostart"
#define ENV_ICONIFY_AMIGAAMP "iconify_amigaamp"
#define ENV_QUIT_AMIGAAMP "quit_amigaamp"
#define ENV_HTTPS "https_only"
#define MAX_PATH_LEN 256
#define PLS_HEADER "[playlist]\n"
#define PLS_NUMBER_OF_ENTRIES "NumberOfEntries=%d\n"
#define PLS_FILE_ENTRY "File%d=%s\n"
#define PLS_TITLE_ENTRY "Title%d=%s\n"
#define PLS_LENGTH_ENTRY "Length%d=-1\n"
#define DEFAULT_PLS_FILENAME "radio.pls"

BOOL LoadSettings(struct APISettings *settings);
BOOL SaveSettings(const struct APISettings *settings);

#endif /* SETTINGS_H */
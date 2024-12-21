#ifndef SETTINGS_H
#define SETTINGS_H

#include <exec/types.h>
#include "data.h" 
extern const struct APISettings DEFAULT_SETTINGS;
#define TUNEFINDER_DIR "ENVARC:TuneFinderMUI/"
#define CONFIG_PATH "ENVARC:TuneFinderMUI/"
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
#define MAX_PATH_LEN 256


BOOL LoadSettings(struct APISettings *settings);
BOOL SaveSettings(const struct APISettings *settings);

#endif /* SETTINGS_H */
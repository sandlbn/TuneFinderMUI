#ifndef NETWORK_H
#define NETWORK_H

#include "data.h"
#include "main.h"

#define API_ENDPOINT "/json/stations/search"
#define MAX_URL_LENGTH 2048
#define INITIAL_BUFFER_SIZE (64 * 1024)  // 64 kb :) 
#define MAX_BUFFER_SIZE (64 * 1024 * 1024) // 4MB maximum
#define PREFERRED_BUFFER_SIZE (2 * 1024 * 1024) // 2MB preferred
#define READ_CHUNK_SIZE (8 * 1024)         // Read 8KB at a time

enum {
    HTTPS_ALL = 0,   // Any stations
    HTTPS_TRUE = 1,  // Only HTTPS stations
    HTTPS_FALSE = 2  // Only non-HTTPS stations
};


char* make_http_request(const struct APISettings *settings, const char *path);
char* build_search_url(const struct APISettings *settings, const struct SearchParams *params);
char* url_encode(const char *str);
struct Tune* parse_stations_json(const char *json_str, int *count);
BOOL InitNetworkSystem(void);
void CleanupNetworkSystem(void);
struct Tune *SearchStations(const struct APISettings *settings, 
                                  const struct SearchParams *params,
                                  LONG *count);
void UpdateStatusMessage(const char *msg);


#endif /* NETWORK_H */
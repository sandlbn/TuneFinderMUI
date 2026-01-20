#include <ctype.h>
#include <exec/memory.h>
#include <exec/types.h>
#include <json-c/json.h>
#include <netdb.h>
#include <netinet/in.h>
#include <proto/exec.h>
#include <proto/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "../include/locale.h"
#include "../include/network.h"
#include "../include/utils.h"
struct ObjApp *objApp;
struct Library *SocketBase;


struct Tune * SearchStations(const struct APISettings * settings,
  const struct SearchParams * params,
    LONG * count) {
  char * url = NULL;
  char * response = NULL;
  struct Tune * tunes = NULL;
  struct APISettings tempSettings;
  int retries = 0;
  const int MAX_RETRIES = 3; // Try up to 3 different servers

  // Small safety check
  if (!settings || !params || !count) {
    DEBUG("Invalid parameters to SearchStations");
    return NULL;
  }

  * count = 0;

  // Make a copy of settings so we can modify the host
  memcpy( & tempSettings, settings, sizeof(struct APISettings));

  // Get the current server
  const char * currentServer = GetCurrentAPIServer();
  if (currentServer && * currentServer) {
    strncpy(tempSettings.host, currentServer, sizeof(tempSettings.host) - 1);
    tempSettings.host[sizeof(tempSettings.host) - 1] = '\0';
  }
  // If GetCurrentAPIServer failed, let us use the original settings
  while (retries < MAX_RETRIES) {
    DEBUG("Trying server: %s (attempt %d)", tempSettings.host, retries + 1);
    UpdateStatusMessage(GetTFString(MSG_SEARCHING));

    url = build_search_url( & tempSettings, params);
    if (!url) {
      UpdateStatusMessage(GetTFString(MSG_FAILED_CREATE_REQUEST));
      // Try next server
      const char * nextServer = GetNextAPIServer();
      if (nextServer && * nextServer) {
        strncpy(tempSettings.host, nextServer, sizeof(tempSettings.host) - 1);
        tempSettings.host[sizeof(tempSettings.host) - 1] = '\0';
      }
      retries++;
      continue;
    }
    response = make_http_request( & tempSettings, url);
    free(url);
    if (!response) {
      UpdateStatusMessage(GetTFString(MSG_FAILED_HTTP_REQ));
      // Try next server
      const char * nextServer = GetNextAPIServer();
      if (nextServer && * nextServer) {
        strncpy(tempSettings.host, nextServer, sizeof(tempSettings.host) - 1);
        tempSettings.host[sizeof(tempSettings.host) - 1] = '\0';
      }
      retries++;
      continue;
    }
    tunes = parse_stations_json(response, count);
    free(response);

    if (!tunes || * count == 0) {
      UpdateStatusMessage(GetTFString(MSG_FAILED_PARSE_RESP));
      // Try next server
      const char * nextServer = GetNextAPIServer();
      if (nextServer && * nextServer) {
        strncpy(tempSettings.host, nextServer, sizeof(tempSettings.host) - 1);
        tempSettings.host[sizeof(tempSettings.host) - 1] = '\0';
      }
      retries++;
      continue;
    }

    // Success!
    return tunes;
  }

  // All retries failed
  UpdateStatusMessage("Failed after trying multiple servers");
  return NULL;
}

BOOL InitNetworkSystem(void)
{
    DEBUG("Initializing network system\n");
    
    if (!(SocketBase = OpenLibrary("bsdsocket.library", 4))) 
    {
        DEBUG("Failed to open bsdsocket.library\n");
        return FALSE;
    }
    
    // Initialize socket library
    if (SocketBaseTags(SBTM_SETVAL(SBTC_ERRNOPTR(sizeof(errno))), (ULONG)&errno,
                      SBTM_SETVAL(SBTC_LOGTAGPTR), (ULONG)"TuneFinderMUI",
                      TAG_DONE))
    {
        DEBUG("Failed to initialize socket library\n");
        CloseLibrary(SocketBase);
        SocketBase = NULL;
        return FALSE;
    }
    // Initialize the API server list
    if (!GetAPIServerList()) {
        DEBUG("Warning: Failed to get API server list, using defaults\n");
        // Continue anyway with defaults
    }

    DEBUG("Network system initialized\n");
    return TRUE;
}

void CleanupNetworkSystem(void)
{
    if (SocketBase)
    {
        CloseLibrary(SocketBase);
        SocketBase = NULL;
    }
}

char *url_encode(const char *str) {
  if (!str) return NULL;

  int len = strlen(str);
  char *encoded = AllocVec(len * 3 + 1, MEMF_CLEAR);
  if (!encoded) return NULL;

  char *penc = encoded;
  while (*str) {
    if ((*str >= 'A' && *str <= 'Z') || (*str >= 'a' && *str <= 'z') ||
        (*str >= '0' && *str <= '9') || *str == '-' || *str == '_' ||
        *str == '.' || *str == '~') {
      *penc++ = *str;
    } else if (*str == ' ') {
      *penc++ = '+';
    } else {
      sprintf(penc, "%%%02X", (unsigned char)*str);
      penc += 3;
    }
    str++;
  }
  *penc = '\0';

  return encoded;
}
char *build_search_url(const struct APISettings *settings,
                      const struct SearchParams *params)
{
    char *url;
    int pos = 0;
    int space_left;
    
    url = AllocVec(MAX_URL_LENGTH, MEMF_CLEAR);
    if (!url)
    {
        return NULL;
    }
    
    // Start with the API endpoint
    pos = snprintf(url, MAX_URL_LENGTH, "%s?", API_ENDPOINT);
    space_left = MAX_URL_LENGTH - pos;
    
    // Add parameters if space available
    if (space_left > 0)
    {
        // Add hidebroken parameter
        pos += snprintf(url + pos, space_left, "hidebroken=%s&",
                       params->hidebroken ? "true" : "false");
        space_left = MAX_URL_LENGTH - pos;
        
        // Add HTTPS parameter
        if (params->is_https == HTTPS_TRUE && space_left > 0)
        {
            pos += snprintf(url + pos, space_left, "is_https=true&");
            space_left = MAX_URL_LENGTH - pos;
        }
        
        // Add name if provided
        if (params->name && strlen(params->name) > 0 && space_left > 0)
        {
            char *encoded_name = url_encode(params->name);
            if (encoded_name)
            {
                pos += snprintf(url + pos, space_left, "name=%s&", encoded_name);
                FreeVec(encoded_name);
                space_left = MAX_URL_LENGTH - pos;
            }
        }
        
        // Add codec if provided
        if (params->codec && strlen(params->codec) > 0 && space_left > 0)
        {
            char *encoded_codec = url_encode(params->codec);
            if (encoded_codec)
            {
                pos += snprintf(url + pos, space_left, "codec=%s&", encoded_codec);
                FreeVec(encoded_codec);
                space_left = MAX_URL_LENGTH - pos;
            }
        }
        
        // Add country if provided
        if (params->country_code && strlen(params->country_code) > 0 && space_left > 0)
        {
            pos += snprintf(url + pos, space_left, "countrycode=%s&", 
                          params->country_code);
            space_left = MAX_URL_LENGTH - pos;
        }
        
        // Add tags if provided
        if (params->tag_list && strlen(params->tag_list) > 0 && space_left > 0)
        {
            char *encoded_tags = url_encode(params->tag_list);
            if (encoded_tags)
            {
                pos += snprintf(url + pos, space_left, "tagList=%s&", encoded_tags);
                FreeVec(encoded_tags);
                space_left = MAX_URL_LENGTH - pos;
            }
        }
        
        // Add limit
        if (space_left > 0)
        {
            pos += snprintf(url + pos, space_left, "limit=%s", params->limit);
        }
    }
    
    // Remove trailing '&' if exists
    if (pos > 0 && url[pos - 1] == '&')
    {
        url[pos - 1] = '\0';
    }
    DEBUG("%s", url);
    return url;
}
static void UpdateSearchStatus(int chunk_count) {
  static int dot_count = 0;
  char dot_buffer[MAX_STATUS_MSG_LEN];
  char dots[4] = {0};
  int i;

  // Create dots string
  for (i = 0; i < (dot_count % 4); i++) {
    dots[i] = '.';
  }
  // Format message with dots
  sprintf(dot_buffer, "%s%s", GetTFString(MSG_SEARCHING), dots);
  UpdateStatusMessage(dot_buffer);
  // Increment dot counter
  dot_count++;
  if (dot_count >= 4) {
    dot_count = 0;
  }
}

char *make_http_request(const struct APISettings *settings, const char *path) {
    int sockfd = -1;
    char *response = NULL;
    struct sockaddr_in server_addr;
    struct hostent *server = NULL;
    char request[512];
    char *chunk_buffer = NULL;
    char *response_buffer = NULL;
    BOOL dns_ok = FALSE;
    size_t buffer_size = PREFERRED_BUFFER_SIZE;
    size_t total_size = 0;
    int bytes_received;
    char *json_start;
    char *new_buffer;
    int retry_count = 0;
    const int MAX_RETRIES = 3;
    int chunk_count = 0;
    
    DEBUG("Starting HTTP request for: %s", settings->host);

    // Create socket first
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        UpdateStatusMessage(GetTFString(MSG_FAILED_CR_SOC));
        return NULL;
    }

    // Set socket options BEFORE connect
    struct timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        DEBUG("Failed to set receive timeout");
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        DEBUG("Failed to set send timeout");
    }

    // Allocate buffers
    chunk_buffer = malloc(READ_CHUNK_SIZE);
    response_buffer = malloc(buffer_size);
    if (!chunk_buffer || !response_buffer) {
        if (!response_buffer && buffer_size > INITIAL_BUFFER_SIZE) {
            buffer_size = INITIAL_BUFFER_SIZE;
            response_buffer = malloc(buffer_size);
        }
        if (!chunk_buffer || !response_buffer) {
            UpdateStatusMessage(GetTFString(MSG_FAILED_ALL_BUFF));
            goto cleanup;
        }
    }

    // DNS resolution with protection
    UpdateStatusMessage("Resolving host...");
    DEBUG("DNS lookup for: %s", settings->host);
    
    Forbid();
    if (settings && settings->host) {
        server = gethostbyname(settings->host);
        if (server && server->h_addr) {
            dns_ok = TRUE;
        }
    }
    Permit();

    if (!dns_ok || !server) {
        DEBUG("DNS lookup failed");
        UpdateStatusMessage(GetTFString(MSG_FAILED_RESOLV_HOST));
        goto cleanup;
    }

    // Setup server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    CopyMem(server->h_addr, &server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(settings->port);

    DEBUG("Connecting to server...");
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        UpdateStatusMessage(GetTFString(MSG_FAILED_CONN_SERV));
        goto cleanup;
    }

    // Prepare and send HTTP request
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.0\r\n"
             "Host: %s\r\n"
             "User-Agent: AmigaRadioBrowser/1.0\r\n"
             "Accept: application/json\r\n"
             "Connection: close\r\n"
             "\r\n",
             path, settings->host);

    DEBUG("Sending request");
    if (send(sockfd, request, strlen(request), 0) < 0) {
        UpdateStatusMessage(GetTFString(MSG_FAILED_SEND_REQ));
        goto cleanup;
    }

    // Receive response
    DEBUG("Receiving response");
    while (1) {
        struct timeval select_timeout;
        ULONG read_mask;

        select_timeout.tv_sec = 5;
        select_timeout.tv_usec = 0;
        read_mask = 1L << sockfd;

        int ready = WaitSelect(sockfd + 1, &read_mask, NULL, NULL, &select_timeout, NULL);
        if (ready < 0) {
            DEBUG("Select error");
            break;
        }
        if (ready == 0) {
            UpdateStatusMessage(GetTFString(MSG_TIMEOUT));
            retry_count++;
            if (retry_count >= MAX_RETRIES) {
                DEBUG("Max retries reached");
                break;
            }
            continue;
        }

        if (read_mask & (1L << sockfd)) {
            bytes_received = recv(sockfd, chunk_buffer, READ_CHUNK_SIZE - 1, 0);
            if (bytes_received < 0) {
                DEBUG("Error receiving data");
                break;
            }
            if (bytes_received == 0) {
                DEBUG("Connection closed by server");
                break;
            }

            UpdateSearchStatus(chunk_count);
            chunk_count++;

            // Check if buffer needs to grow
            if (total_size + bytes_received + 1 > buffer_size) {
                size_t new_size = buffer_size * 2;
                if (new_size > MAX_BUFFER_SIZE) {
                    new_size = buffer_size + (1024 * 1024);
                    if (new_size > MAX_BUFFER_SIZE) {
                        DEBUG("Response too large");
                        goto cleanup;
                    }
                }

                DEBUG("Growing buffer from %lu to %lu bytes", 
                      (unsigned long)buffer_size, (unsigned long)new_size);

                new_buffer = realloc(response_buffer, new_size);
                if (!new_buffer) {
                    DEBUG("Failed to grow response buffer");
                    goto cleanup;
                }
                response_buffer = new_buffer;
                buffer_size = new_size;
            }

            memcpy(response_buffer + total_size, chunk_buffer, bytes_received);
            total_size += bytes_received;
            response_buffer[total_size] = '\0';
        }
    }

    if (total_size == 0) {
        DEBUG("No data received");
        goto cleanup;
    }

    // Find start of JSON content
    json_start = strstr(response_buffer, "\r\n\r\n");
    if (json_start) {
        json_start += 4;
        DEBUG("Found JSON content");
    } else {
        DEBUG("Could not find JSON content, using entire response");
        json_start = response_buffer;
    }

    response = strdup(json_start);

cleanup:
    if (sockfd >= 0) {
        DEBUG("Closing socket");
        CloseSocket(sockfd);
    }
    if (response_buffer) {
        free(response_buffer);
    }
    if (chunk_buffer) {
        free(chunk_buffer);
    }

    return response;
}
struct Tune *parse_stations_json(const char *json_str, int *count)
{
    struct json_object *root;
    struct Tune *tunes = NULL;
    int array_len;
    int i;

    *count = 0;

    root = json_tokener_parse(json_str);
    if (!root) {
        return NULL;
    }

    // Check if root is actually an array before calling array functions
    if (!json_object_is_type(root, json_type_array)) {
        DEBUG("JSON response is not an array");
        json_object_put(root);
        return NULL;
    }

    array_len = json_object_array_length(root);
    if (array_len <= 0) {
        json_object_put(root);
        return NULL;
    }

    tunes = calloc(array_len, sizeof(struct Tune));
    if (!tunes) {
        json_object_put(root);
        return NULL;
    }

    for (i = 0; i < array_len; i++) {
        struct json_object *station_obj = json_object_array_get_idx(root, i);
        struct json_object *name_obj, *url_obj, *codec_obj, *bitrate_obj, *country_obj;
        const char *name, *url, *codec, *country;
        int bitrate;

        UpdateSearchStatus(i);

        if (json_object_object_get_ex(station_obj, "name", &name_obj) &&
            json_object_object_get_ex(station_obj, "url", &url_obj) &&
            json_object_object_get_ex(station_obj, "codec", &codec_obj) &&
            json_object_object_get_ex(station_obj, "countrycode", &country_obj) &&
            json_object_object_get_ex(station_obj, "bitrate", &bitrate_obj)) {

            name = json_object_get_string(name_obj);
            url = json_object_get_string(url_obj);
            codec = json_object_get_string(codec_obj);
            country = json_object_get_string(country_obj);
            bitrate = json_object_get_int(bitrate_obj);

            tunes[*count].name = strdup(name);
            tunes[*count].url = strdup(url);
            tunes[*count].codec = strdup(codec);
            tunes[*count].country = strdup(country);
            tunes[*count].bitrate = bitrate;

            (*count)++;
        }
    }

    json_object_put(root);
    return tunes;
}
// Function to add a default server to the list
static void AddDefaultServer(const char *server) {
    if (g_serverList.count < MAX_API_SERVERS) {
        strncpy(g_serverList.servers[g_serverList.count], server, MAX_HOST_LEN-1);
        g_serverList.servers[g_serverList.count][MAX_HOST_LEN-1] = '\0';
        g_serverList.count++;
    }
}
// Function to get the list of available API servers via DNS lookup
BOOL GetAPIServerList(void) {
  struct hostent * host = NULL;
  int i;

  DEBUG("Performing DNS lookup for all.api.radio-browser.info");

  // Reset server list
  memset( & g_serverList, 0, sizeof(g_serverList));
  g_serverList.initialized = FALSE;

  // Add default servers first so we have fallbacks
  AddDefaultServer("de1.api.radio-browser.info");
  AddDefaultServer("fi1.api.radio-browser.info");
  AddDefaultServer("de2.api.radio-browser.info");
  AddDefaultServer("at1.api.radio-browser.info");

  // Now try DNS lookup to get the dynamic list
  Forbid();
  host = gethostbyname("all.api.radio-browser.info");

  if (host && host -> h_addr_list && host -> h_addr_list[0]) {
    // replace default servers
    g_serverList.count = 0;

    for (i = 0; i < MAX_API_SERVERS; i++) {
      // Try to do a DNS lookup for each country code
      char hostname[MAX_HOST_LEN];
      struct hostent * country_host;

      // Try different country codes
      const char * country_codes[] = {
        "de1",
        "fi1",
        "nl1",
        "at1",
        "uk1",
        "us1"
      };

      if (i >= sizeof(country_codes) / sizeof(country_codes[0]))
        break;

      snprintf(hostname, sizeof(hostname), "%s.api.radio-browser.info",
        country_codes[i]);

      country_host = gethostbyname(hostname);
      if (country_host && country_host -> h_addr) {
        // This hostname is valid
        strncpy(g_serverList.servers[g_serverList.count], hostname, MAX_HOST_LEN - 1);
        g_serverList.servers[g_serverList.count][MAX_HOST_LEN - 1] = '\0';
        g_serverList.count++;
        DEBUG("Added server %d: %s", g_serverList.count - 1, hostname);
      }
    }
  }
  Permit();

  if (g_serverList.count == 0) {
    DEBUG("DNS lookup failed or no servers found, using defaults");
    AddDefaultServer("de1.api.radio-browser.info");
    AddDefaultServer("fi1.api.radio-browser.info");
  }

  g_serverList.current = 0;
  g_serverList.initialized = TRUE;

  DEBUG("Server list initialized with %d servers", g_serverList.count);
  return (g_serverList.count > 0);
}

// Function to get the next server in the list
const char * GetNextAPIServer(void) {
  if (!g_serverList.initialized || g_serverList.count == 0) {
    GetAPIServerList();
  }

  if (g_serverList.count == 0) {
    // Still no servers? Return a safe default
    return "de1.api.radio-browser.info";
  }

  // Move to next server
  g_serverList.current = (g_serverList.current + 1) % g_serverList.count;

  return g_serverList.servers[g_serverList.current];
}

// Function to get the current server
const char * GetCurrentAPIServer(void) {
  if (!g_serverList.initialized || g_serverList.count == 0) {
    GetAPIServerList();
  }

  if (g_serverList.count == 0) {
    // Still no servers? Return a safe default
    return "de1.api.radio-browser.info";
  }

  return g_serverList.servers[g_serverList.current];
}
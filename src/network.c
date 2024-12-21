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


struct Tune *SearchStations(const struct APISettings *settings, 
                          const struct SearchParams *params,
                          LONG *count)
{
    char *url = NULL;
    char *response = NULL;
    struct Tune *tunes = NULL;
    *count = 0;

    url = build_search_url(settings, params);
    if (!url) {
        UpdateStatusMessage(GetTFString(MSG_FAILED_CREATE_REQUEST));
        return NULL;
    }

    response = make_http_request(settings, url);
    free(url);

    if (!response) {
        UpdateStatusMessage(GetTFString(MSG_FAILED_HTTP_REQ));
        return NULL;
    }

    tunes = parse_stations_json(response, count);
    free(response);

    if (!tunes) {
        UpdateStatusMessage(GetTFString(MSG_FAILED_PARSE_RESP));
        return NULL;
    }

    return tunes;
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
        pos += snprintf(url + pos, space_left, "hidebroken=%d&",
                       params->hidebroken ? 1 : 0);
        space_left = MAX_URL_LENGTH - pos;
        
        // Add HTTPS parameter
        if (params->is_https && space_left > 0)
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
            pos += snprintf(url + pos, space_left, "limit=%lu", params->limit);
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
             "GET %s HTTP/1.1\r\n"
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

            tunes[i].name = strdup(name);
            tunes[i].url = strdup(url);
            tunes[i].codec = strdup(codec);
            tunes[i].country = strdup(country);
            tunes[i].bitrate = bitrate;

            (*count)++;
        }
    }

    json_object_put(root);
    return tunes;
}
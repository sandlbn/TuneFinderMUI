#ifndef DATA_H
#define DATA_H

#include <exec/types.h>
#include <exec/lists.h>

struct APISettings {
    char host[256];
    ULONG port;
    ULONG limit;
};

struct SearchParams {
    const char *name;
    const char *country_code;
    const char *state;
    const char *codec;
    const char *tag_list;
    const char *limit;
    int hidebroken;
    int is_https;
};

struct RadioStation {
    struct Node node;
    char *name;
    char *url;
    char *codec;
    char *country;
    int bitrate;
};

void *allocate(size_t size, int type);
void deallocate(void *ptr, int type);
void free_stations(struct RadioStation *stations, int count);

#endif /* DATA_H */
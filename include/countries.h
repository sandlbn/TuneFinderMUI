#ifndef COUNTRIES_H
#define COUNTRIES_H

struct CountryMapping {
    const char *code;
    const char *name;
};

const char* GetCountryNameFromCode(const char *code);

#endif
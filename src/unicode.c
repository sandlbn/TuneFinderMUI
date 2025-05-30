#include <proto/exec.h>
#include <string.h>
#include "../include/unicode.h"
#include "../include/main.h"

static const struct CharMapping char_mappings[] = {
    // Polish
    {{0xC4, 0x85, 0x00, 0x00}, 'a', 2},  // ą
    {{0xC4, 0x87, 0x00, 0x00}, 'c', 2},  // ć
    {{0xC4, 0x99, 0x00, 0x00}, 'e', 2},  // ę
    {{0xC5, 0x82, 0x00, 0x00}, 'l', 2},  // ł
    {{0xC5, 0x84, 0x00, 0x00}, 'n', 2},  // ń
    {{0xC3, 0xB3, 0x00, 0x00}, 'o', 2},  // ó
    {{0xC5, 0x9B, 0x00, 0x00}, 's', 2},  // ś
    {{0xC5, 0xBA, 0x00, 0x00}, 'z', 2},  // ź
    {{0xC5, 0xBC, 0x00, 0x00}, 'z', 2},  // ż

    // German
    {{0xC3, 0xA4, 0x00, 0x00}, 'a', 2},  // ä
    {{0xC3, 0xB6, 0x00, 0x00}, 'o', 2},  // ö
    {{0xC3, 0xBC, 0x00, 0x00}, 'u', 2},  // ü
    {{0xC3, 0x9F, 0x00, 0x00}, 's', 2},  // ß

    // French
    {{0xC3, 0xA0, 0x00, 0x00}, 'a', 2},  // à
    {{0xC3, 0xA2, 0x00, 0x00}, 'a', 2},  // â
    {{0xC3, 0xA7, 0x00, 0x00}, 'c', 2},  // ç
    {{0xC3, 0xA9, 0x00, 0x00}, 'e', 2},  // é
    {{0xC3, 0xA8, 0x00, 0x00}, 'e', 2},  // è
    {{0xC3, 0xAA, 0x00, 0x00}, 'e', 2},  // ê
    {{0xC3, 0xAB, 0x00, 0x00}, 'e', 2},  // ë
    {{0xC3, 0xAE, 0x00, 0x00}, 'i', 2},  // î
    {{0xC3, 0xAF, 0x00, 0x00}, 'i', 2},  // ï
    {{0xC3, 0xB4, 0x00, 0x00}, 'o', 2},  // ô
    {{0xC5, 0x93, 0x00, 0x00}, 'o', 2},  // œ
    {{0xC3, 0xB9, 0x00, 0x00}, 'u', 2},  // ù
    {{0xC3, 0xBB, 0x00, 0x00}, 'u', 2},  // û

    // Spanish
    {{0xC3, 0xA1, 0x00, 0x00}, 'a', 2},  // á
    {{0xC3, 0xA9, 0x00, 0x00}, 'e', 2},  // é
    {{0xC3, 0xAD, 0x00, 0x00}, 'i', 2},  // í
    {{0xC3, 0xB1, 0x00, 0x00}, 'n', 2},  // ñ
    {{0xC3, 0xB3, 0x00, 0x00}, 'o', 2},  // ó
    {{0xC3, 0xBA, 0x00, 0x00}, 'u', 2},  // ú
    {{0xC2, 0xBF, 0x00, 0x00}, '?', 2},  // ¿
    {{0xC2, 0xA1, 0x00, 0x00}, '!', 2},  // ¡

    // Scandinavian
    {{0xC3, 0xA5, 0x00, 0x00}, 'a', 2},  // å
    {{0xC3, 0xA6, 0x00, 0x00}, 'a', 2},  // æ
    {{0xC3, 0xB8, 0x00, 0x00}, 'o', 2},  // ø

    // Polish Uppercase
    {{0xC4, 0x84, 0x00, 0x00}, 'A', 2},  // Ą
    {{0xC4, 0x86, 0x00, 0x00}, 'C', 2},  // Ć
    {{0xC4, 0x98, 0x00, 0x00}, 'E', 2},  // Ę
    {{0xC5, 0x81, 0x00, 0x00}, 'L', 2},  // Ł
    {{0xC5, 0x83, 0x00, 0x00}, 'N', 2},  // Ń
    {{0xC3, 0x93, 0x00, 0x00}, 'O', 2},  // Ó
    {{0xC5, 0x9A, 0x00, 0x00}, 'S', 2},  // Ś
    {{0xC5, 0xB9, 0x00, 0x00}, 'Z', 2},  // Ź
    {{0xC5, 0xBB, 0x00, 0x00}, 'Z', 2},  // Ż

    // German Uppercase
    {{0xC3, 0x84, 0x00, 0x00}, 'A', 2},  // Ä
    {{0xC3, 0x96, 0x00, 0x00}, 'O', 2},  // Ö
    {{0xC3, 0x9C, 0x00, 0x00}, 'U', 2},  // Ü

    // French Uppercase
    {{0xC3, 0x80, 0x00, 0x00}, 'A', 2},  // À
    {{0xC3, 0x82, 0x00, 0x00}, 'A', 2},  // Â
    {{0xC3, 0x87, 0x00, 0x00}, 'C', 2},  // Ç
    {{0xC3, 0x89, 0x00, 0x00}, 'E', 2},  // É
    {{0xC3, 0x88, 0x00, 0x00}, 'E', 2},  // È
    {{0xC3, 0x8A, 0x00, 0x00}, 'E', 2},  // Ê
    {{0xC3, 0x8B, 0x00, 0x00}, 'E', 2},  // Ë
    {{0xC3, 0x8E, 0x00, 0x00}, 'I', 2},  // Î
    {{0xC3, 0x8F, 0x00, 0x00}, 'I', 2},  // Ï
    {{0xC3, 0x94, 0x00, 0x00}, 'O', 2},  // Ô
    {{0xC5, 0x92, 0x00, 0x00}, 'O', 2},   // Œ
    {{0xC3, 0x99, 0x00, 0x00}, 'U', 2},  // Ù
    {{0xC3, 0x9B, 0x00, 0x00}, 'U', 2},  // Û

    // Spanish Uppercase
    {{0xC3, 0x81, 0x00, 0x00}, 'A', 2},  // Á
    {{0xC3, 0x89, 0x00, 0x00}, 'E', 2},  // É
    {{0xC3, 0x8D, 0x00, 0x00}, 'I', 2},  // Í
    {{0xC3, 0x91, 0x00, 0x00}, 'N', 2},  // Ñ
    {{0xC3, 0x93, 0x00, 0x00}, 'O', 2},  // Ó
    {{0xC3, 0x9A, 0x00, 0x00}, 'U', 2},  // Ú

    // Scandinavian Uppercase
    {{0xC3, 0x85, 0x00, 0x00}, 'A', 2},  // Å
    {{0xC3, 0x86, 0x00, 0x00}, 'A', 2},  // Æ
    {{0xC3, 0x98, 0x00, 0x00}, 'O', 2},   // Ø
    
    // Czech Lowercase
    {{0xC4, 0x8D, 0x00, 0x00}, 'c', 2},  // č
    {{0xC4, 0x8F, 0x00, 0x00}, 'd', 2},  // ď
    {{0xC4, 0x9B, 0x00, 0x00}, 'e', 2},  // ě
    {{0xC5, 0x88, 0x00, 0x00}, 'n', 2},  // ň
    {{0xC5, 0x99, 0x00, 0x00}, 'r', 2},  // ř
    {{0xC5, 0xA1, 0x00, 0x00}, 's', 2},  // š
    {{0xC5, 0xA5, 0x00, 0x00}, 't', 2},  // ť
    {{0xC5, 0xAF, 0x00, 0x00}, 'u', 2},  // ů
    {{0xC5, 0xBE, 0x00, 0x00}, 'z', 2},  // ž

    // Czech Uppercase
    {{0xC4, 0x8C, 0x00, 0x00}, 'C', 2},  // Č
    {{0xC4, 0x8E, 0x00, 0x00}, 'D', 2},  // Ď
    {{0xC4, 0x9A, 0x00, 0x00}, 'E', 2},  // Ě
    {{0xC5, 0x87, 0x00, 0x00}, 'N', 2},  // Ň
    {{0xC5, 0x98, 0x00, 0x00}, 'R', 2},  // Ř
    {{0xC5, 0xA0, 0x00, 0x00}, 'S', 2},  // Š
    {{0xC5, 0xA4, 0x00, 0x00}, 'T', 2},  // Ť
    {{0xC5, 0xAE, 0x00, 0x00}, 'U', 2},  // Ů
    {{0xC5, 0xBD, 0x00, 0x00}, 'Z', 2},  // Ž
    // Romanian
    {{0xC4, 0x83, 0x00, 0x00}, 'a', 2},  // ă
    {{0xC3, 0xA2, 0x00, 0x00}, 'a', 2},  // â
    {{0xC3, 0xAE, 0x00, 0x00}, 'i', 2},  // î
    {{0xC8, 0x99, 0x00, 0x00}, 's', 2},  // ș
    {{0xC8, 0x9B, 0x00, 0x00}, 't', 2},  // ț
    
    {{0xC4, 0x82, 0x00, 0x00}, 'A', 2},  // Ă
    {{0xC3, 0x82, 0x00, 0x00}, 'A', 2},  // Â
    {{0xC3, 0x8E, 0x00, 0x00}, 'I', 2},  // Î
    {{0xC8, 0x98, 0x00, 0x00}, 'S', 2},  // Ș
    {{0xC8, 0x9A, 0x00, 0x00}, 'T', 2},  // Ț

    // Hungarian
    {{0xC3, 0xA1, 0x00, 0x00}, 'a', 2},  // á
    {{0xC3, 0xa9, 0x00, 0x00}, 'e', 2},  // é
    {{0xC3, 0xad, 0x00, 0x00}, 'i', 2},  // í
    {{0xC3, 0xb3, 0x00, 0x00}, 'o', 2},  // ó
    {{0xC5, 0x91, 0x00, 0x00}, 'o', 2},  // ő
    {{0xC3, 0xb6, 0x00, 0x00}, 'o', 2},  // ö
    {{0xC3, 0xba, 0x00, 0x00}, 'u', 2},  // ú
    {{0xC5, 0xb1, 0x00, 0x00}, 'u', 2},  // ű
    {{0xC3, 0xbc, 0x00, 0x00}, 'u', 2},  // ü

    {{0xC3, 0x81, 0x00, 0x00}, 'A', 2},  // Á
    {{0xC3, 0x89, 0x00, 0x00}, 'E', 2},  // É
    {{0xC3, 0x8D, 0x00, 0x00}, 'I', 2},  // Í
    {{0xC3, 0x93, 0x00, 0x00}, 'O', 2},  // Ó
    {{0xC5, 0x90, 0x00, 0x00}, 'O', 2},  // Ő
    {{0xC3, 0x96, 0x00, 0x00}, 'O', 2},  // Ö
    {{0xC3, 0x9A, 0x00, 0x00}, 'U', 2},  // Ú
    {{0xC5, 0xB0, 0x00, 0x00}, 'U', 2},  // Ű
    {{0xC3, 0x9C, 0x00, 0x00}, 'U', 2},  // Ü

    {{0x00, 0x00, 0x00, 0x00}, 0, 0}     // terminator
};

char *convertToASCII(const unsigned char *input) {
    if (!input) return NULL;

    char *output = AllocVec(strlen((char *)input) + 1, MEMF_CLEAR);
    if (!output) return NULL;

    const unsigned char *src = input;
    char *dst = output;

    while (*src) {
        if (*src < 0x80) {
            // ASCII character
            if (*src >= 32 && *src <= 126) {
                *dst++ = *src;
            }
            src++;
            continue;
        }

        // Check for known byte sequences
        BOOL found = FALSE;
        for (int i = 0; char_mappings[i].len > 0; i++) {
            if (memcmp(src, char_mappings[i].bytes, char_mappings[i].len) == 0) {
                *dst++ = char_mappings[i].ascii;
                src += char_mappings[i].len;
                found = TRUE;
                break;
            }
        }

        // Skip unknown byte sequence
        if (!found) {
            src++;
        }
    }

    *dst = '\0';
    return output;
}
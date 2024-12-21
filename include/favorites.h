#ifndef FAVORITES_H
#define FAVORITES_H

#include <exec/types.h>
#include "data.h"
#include "main.h"

BOOL SaveFavorite(const struct Tune *tune);
BOOL RemoveFavorite(const struct Tune *tune);
BOOL IsTuneInFavorites(const struct Tune *tune);
struct Tune *LoadFavorites(LONG *count);  // Added to load favorites into list

#endif /* FAVORITES_H */

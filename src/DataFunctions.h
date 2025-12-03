#ifndef DATAFUNCTIONS_H
#define DATAFUNCTIONS_H

#include "DataTypes.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// Saves the V1 player data
int saveDataV1(PlayerDataV1 *data, size_t count, FILE *file);

// Saves the V2 player data
int saveDataV2(PlayerDataV2 *data, size_t count, FILE *file);

// Loads the V1 player data
PlayerDataV1 *loadDataV1(FILE *file, size_t count);

// Loads the V2 player data
PlayerDataV2 *loadDataV2(FILE *file, size_t count);

// Migrates V1 data to V2 data
PlayerDataV2 *migrateDataV1ToV2(PlayerDataV1 *playersData, size_t count);

// Migrates V2 data to V1 data for backwards compatibility
PlayerDataV1 *migrateDataV2ToV1(PlayerDataV2 *playersData, size_t count);

// Fetches the header data
BinaryHeader *getHeader(FILE *file);

// Writes new header data to the file
int writeNewHeader(FILE *file, char magic[5], uint32_t version, uint32_t count);

// Writes header data to the file
int writeHeader(FILE *file, BinaryHeader *header);

#endif

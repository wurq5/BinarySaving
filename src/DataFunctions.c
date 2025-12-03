#include "DataFunctions.h"
#include "DataTypes.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int saveDataV1(PlayerDataV1 *data, size_t count, FILE *file) {
    if (!file)
        return -1;

    fwrite(data, sizeof(PlayerDataV1), count, file);
    return 0;
}

PlayerDataV1 *loadDataV1(FILE *file, size_t count) {
    if (!file)
        return NULL;

    PlayerDataV1 *data = malloc(sizeof(PlayerDataV1) * count);
    if (!data)
        return NULL;

    fseek(file, sizeof(BinaryHeader), SEEK_SET); // Move our position in file

    fread(data, sizeof(PlayerDataV1), count,
          file); // Now read the size of the data struct * count

    return data;
}

int saveDataV2(PlayerDataV2 *data, size_t count, FILE *file) {
    if (!file)
        return -1;

    fwrite(data, sizeof(PlayerDataV2), count, file);
    return 0;
}

PlayerDataV2 *loadDataV2(FILE *file, size_t count) {
    if (!file)
        return NULL;

    PlayerDataV2 *data = malloc(sizeof(PlayerDataV2) * count);
    if (!data)
        return NULL;

    fseek(file, sizeof(BinaryHeader), SEEK_SET); // Skip our header again

    fread(data, sizeof(PlayerDataV2), count, file);

    return data;
}

PlayerDataV2 *migrateDataV1ToV2(PlayerDataV1 *playersData, size_t count) {
    PlayerDataV2 *newData = malloc(sizeof(PlayerDataV2) * count);
    if (!newData)
        return NULL;

    for (size_t i = 0; i < count; i++) {
        newData[i].health = playersData[i].health;
        strcpy(newData[i].name, playersData[i].name);
        newData[i].level = 1; // Or the default value for this field
    }

    return newData;
}

PlayerDataV1 *migrateDataV2ToV1(PlayerDataV2 *playersData, size_t count) {
    PlayerDataV1 *newData = malloc(sizeof(PlayerDataV1) * count);
    if (!newData)
        return NULL;

    for (size_t i = 0; i < count; i++) {
        newData[i].health = playersData[i].health;
        strcpy(newData[i].name, playersData[i].name);
        // Disregard our new field as it is no longer compatible in this version
    }

    return newData;
}

BinaryHeader *getHeader(FILE *file) {
    BinaryHeader *header = malloc(sizeof(BinaryHeader));
    if (!header)
        return NULL;

    fseek(file, 0, SEEK_SET); // Ensure we're at the start
    fread(header, sizeof(BinaryHeader), 1, file);

    return header;
}

int writeNewHeader(FILE *file, char magic[5], uint32_t version,
                   uint32_t count) {
    if (!file)
        return -1; // If file isnt open then stop here

    BinaryHeader *header =
        malloc(sizeof(BinaryHeader)); // Create the new header
    strcpy(header->magic, magic);     // Give it the magic identifier
    header->version = version;        // Set the version
    header->count = count;            // And the count

    fwrite(header, sizeof(BinaryHeader), 1,
           file); // Now write the data and return successfully

    return 0;
}

int writeHeader(FILE *file, BinaryHeader *header) {
    if (!file)
        return -1;

    fwrite(header, sizeof(BinaryHeader), 1, file);
    return 0;
}

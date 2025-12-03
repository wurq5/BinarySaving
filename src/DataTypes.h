#ifndef DATATYPES_H
#define DATATYPES_H

#include <stdint.h>

// Version 1 player data
typedef struct {
    int health;
    char name[32];
} PlayerDataV1;

// Version 2 player data
typedef struct {
    int health;
    char name[32];
    int level; // New addition
} PlayerDataV2;

// This is how we can identify a save type and migrate / load safely
typedef struct {
    char magic[5]; // Our identifier to know its our file, not some random file
    uint32_t version; // This is how we get our version
    uint32_t count;   // Finally, this is the amount of objects saved
} BinaryHeader;

#endif

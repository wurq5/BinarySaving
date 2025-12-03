#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Setup our helper functions to load / save and identify the save data
int saveDataV1(PlayerDataV1 *data, size_t count, FILE *file) {
    fwrite(data, sizeof(PlayerDataV1), count, file);
    return 0;
}

PlayerDataV1 *loadDataV1(FILE *file, size_t count) {
    PlayerDataV1 *data = malloc(sizeof(PlayerDataV1) * count);
    if (!data) return NULL;

    fseek(file, sizeof(BinaryHeader), SEEK_SET); // Move our position in file

    fread(data, sizeof(PlayerDataV1), count,
          file); // Now read the size of the data struct * count

    return data;
}

int saveDataV2(PlayerDataV2 *data, size_t count, FILE *file) {
    fwrite(data, sizeof(PlayerDataV2), count, file);
    return 0;
}

PlayerDataV2 *loadDataV2(FILE *file, size_t count) {
    PlayerDataV2 *data = malloc(sizeof(PlayerDataV2) * count);
    if (!data) return NULL;

    fseek(file, sizeof(BinaryHeader), SEEK_SET); // Skip our header again

    fread(data, sizeof(PlayerDataV2), count, file);

    return data;
}

PlayerDataV2 *migrateDataV1ToV2(PlayerDataV1 *playersData, size_t count) {
    PlayerDataV2 *newData = malloc(sizeof(PlayerDataV2) * count);
    if (!newData) return NULL;

    for (size_t i = 0; i < count; i++) {
        newData[i].health = playersData[i].health;
        strcpy(newData[i].name, playersData[i].name);
        newData[i].level = 1; // Or the default value for this field
    }

    return newData;
}

PlayerDataV1* migrateDataV2ToV1(PlayerDataV2* playersData, size_t count) {
    PlayerDataV1 *newData = malloc(sizeof(PlayerDataV1) * count);
    if (!newData) return NULL;

    for (size_t i = 0; i < count; i++) {
        newData[i].health = playersData[i].health;
        strcpy(newData[i].name, playersData[i].name);
        // Disregard our new field as it is no longer compatible in this version
    }

    return newData;
}

BinaryHeader *getHeader(FILE *file) {
    BinaryHeader *header = malloc(sizeof(BinaryHeader));
    if (!header) return NULL;

    fseek(file, 0, SEEK_SET); // Ensure we're at the start
    fread(header, sizeof(BinaryHeader), 1, file);

    return header;
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Error: Correct usage ./binarySaving --{load/save/migrate}\n");
        return -1;
    }

    // Initialise our file in read binary mode
    FILE *saveFile = fopen("data.bin", "rb");

    if (!saveFile) {
        printf("Warn: Creating file with header\n");
        BinaryHeader* header = malloc(sizeof(BinaryHeader));
        strcpy(header->magic, "SAVE");
        header->version = 0;
        header->count = 0;
        saveFile = fopen("data.bin", "wb");
        fwrite(header, sizeof(BinaryHeader), 1, saveFile);
        fclose(saveFile);
        saveFile = fopen("data.bin", "rb");
    }

    // Load our header
    BinaryHeader *header = getHeader(saveFile);

    // Validate its our file
    if (strcmp(header->magic, "SAVE") != 0) {
        printf("Error: Invalid header or not our file\n");
        printf("Header info -> Version %d | Magic %s | Count %d", header->version, header->magic, header->count);
        return -1;
    }

    printf("Loading save version %d\n", header->version);

    // Now close it from read mode and reopen in the selected mode
    fclose(saveFile);

    // Check its inbounds
    if (header->version > 2 || header->version < 0) {
        printf("Error: Version '%d' is out of bounds\n", header->version);
        return -1;
    }

    // Lets figure out the use case
    if (strcmp(argv[1], "--load") == 0) {
        // If we need to load it then we're reading so we use "rb" to read the binary data
        saveFile = fopen("data.bin", "rb");

        if (!saveFile) {
            printf("Error: Invalid file path\n");
            return -1;
        }

        if (header->version == 1) {
            PlayerDataV1 *playersData = loadDataV1(saveFile, header->count);

            for (size_t i = 0; i < header->count; i++) {
                printf("\t--|Player%d|--\nName: %s\nHealth: %d\n", i,
                       playersData[i].name, playersData->health);
            }
        } else if (header->version == 2) {
            PlayerDataV2 *playersData = loadDataV2(saveFile, header->count);

            for (size_t i = 0; i < header->count; i++) {
                printf("\t--|Player%d|--\nName: %s\nHealth: %d\nLevel: %d\n", i,
                       playersData[i].name, playersData[i].health,
                       playersData[i].level);
            }
        }
    } else if (strcmp(argv[1], "--save") == 0) {
        saveFile = fopen("data.bin", "wb"); // Now open in write binary mode
        // Now create our new header
        BinaryHeader* newHeader = malloc(sizeof(BinaryHeader));
        strcpy(newHeader->magic, "SAVE");
        
        if (header->version == 0) {
            printf("Please enter a valid version to save to (1-2): ");
            scanf("%d", &header->version);
        }

        newHeader->version = header->version;

        int count;

        printf("How many players would you like to create? ");
        scanf("%d", &count);

        newHeader->count = count;

        fwrite(newHeader, sizeof(BinaryHeader), 1, saveFile); 

        if (header->version == 1) {
            PlayerDataV1 *playersData = malloc(sizeof(PlayerDataV1) * count);
            if (!playersData) {
                printf("Error: Malloc failed\n");
                fclose(saveFile);
                return -1;
            }

            // Get all our necessary values
            for (size_t i = 0; i < count; i++) {
                printf("Name: ");
                scanf("%s", playersData[i].name);
                printf("Health: ");
                scanf("%d", &playersData[i].health);
            }

            saveDataV1(playersData, count, saveFile); // Now write it all
        } else if (header->version == 2) {
            PlayerDataV2 *playersData = malloc(sizeof(PlayerDataV2) * count);
            if (!playersData) {
                printf("Error: Malloc failed\n");
                fclose(saveFile);
                return -1;
            }

            for (size_t i = 0; i < count; i++) {
                printf("Name: ");
                scanf("%s", playersData[i].name);
                printf("Health: ");
                scanf("%d", &playersData[i].health);
                printf("Level: ");
                scanf("%d", &playersData[i].level);
            }

            saveDataV2(playersData, count, saveFile);
        }
    } else if (strcmp(argv[1], "--migrate") == 0) {
        saveFile = fopen("data.bin", "rb"); // Read to get data

        int targetVersion;
        printf("Enter the target version to migrate to: ");
        scanf("%d", &targetVersion);

        if (targetVersion == header->version) {
            printf("Error: Data is already in Version '%d'", header->version);
            fclose(saveFile);
            return -1;
        } else {
            if (targetVersion == 1 && header->version == 2) {
                PlayerDataV2* oldData = loadDataV2(saveFile, header->count); // Fetch old data
                PlayerDataV1* playersData = migrateDataV2ToV1(oldData, header->count); // Migrate it
                fclose(saveFile); // Exit read mode
                saveFile = fopen("data.bin", "wb"); // Now we are writing the new data
                // Write our header
                header->version = targetVersion; // Update version
                fwrite(header, sizeof(BinaryHeader), 1, saveFile); // Write header first
                fwrite(playersData, sizeof(PlayerDataV1), header->count, saveFile); // Write new data
            } else if (targetVersion == 2 && header->version == 1) {
                PlayerDataV1* oldData = loadDataV1(saveFile, header->count); // Fetch old data
                PlayerDataV2* playersData = migrateDataV1ToV2(oldData, header->count); // Migrate old data
                fclose(saveFile); // Exit read mode
                saveFile = fopen("data.bin", "wb");
                // Write the header
                header->version = targetVersion; // Update new version
                fwrite(header, sizeof(BinaryHeader), 1, saveFile);
                fwrite(playersData, sizeof(PlayerDataV2), header->count, saveFile); 
            }
        }
    } else {
        printf("Error: Incorrect flag use either '--load' or '--save' or '--migrate'\n");
        return -1;
    }

    // Finally, clean up
    if (saveFile) { // Safely catches it if the file is still open
        fclose(saveFile);
    }

    return 0;
}

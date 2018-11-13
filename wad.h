#ifndef __WAD2__
#define __WAD2__

#include <stdio.h>
#include "data.h"

enum wad23_resource_t
{
    WAD2_PALETTE = 0x40,
    WAD2_PICTURE = 0x42,
    WAD3_TEXTURE = 0x43,
    WAD2_TEXTURE = 0x44,
    WAD2_CONCHARS = 0x45,
    WAD3_FONT = 0x46
};

const int WAD_IDENT_LENGTH = 4;

#pragma pack(push, 1)
struct wad2Header_t
{
    char ident[WAD_IDENT_LENGTH];
    unsigned long number;
    unsigned long offset;
};
#pragma pack(pop)

const int WAD_NAME_LENGTH = 16;

#pragma pack(push, 1)
struct wad2Entry_t
{
    unsigned long offset;
    unsigned long dsize;
    unsigned long size;
    unsigned char type;
    unsigned char compression;
    unsigned short reserved;
    char name[WAD_NAME_LENGTH];
};
#pragma pack(pop)

class wad2_t
{
    int number;
    wad2Entry_t *dir;
    FILE *file;

public:
    wad2_t(void);
    ~wad2_t(void);

    int openWAD(const char *fileName);
    int getNumber(void);
    int getName(const int i, char **name);
    int getType(const int i);
    int loadData(const int i, data_t *dat);
};

#endif

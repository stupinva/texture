/*
14-07-2002
 - image_t: setWidthHeight, getWidth, getHeight, setMap, setPixel, putPixel
15-07-2002
 - data_t: loadData, saveData, tellData, seekData
17-07-2002
 - data_t: readData, writeData, readChar, writeChar, sizeData
 - image_t: readTGA, writeTGA, readBMP, writeBMP
18-07-2002
 - image_t: readMIP, readWAL
 - wad2_t: openWAD, getNumber, getName, loadData
19-07-2002
 - image_t: readM8, readM32, readLMP
 - wad2_t: getType
20-07-2002
 - image_t: setQuakeMap, setQuake2Map, setImage, buildMipImage, writeLMP, writeMIP, writeWAL
22-07-2002
 - colorStat_t: addImageColors, getMap
 - image_t: writeLMP Half-Life mode, writeMIP Half-Life mode, writeM8
23-07-2002
 - image_t: setPixelA, getPixelA, writeM32, readTGA 32-bit types
09-11-2018
 - bool_t
 - s_str
*/
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "str.h"
#include "data.h"
#include "image.h"
#include "wad.h"

typedef enum format_e
{
    F_NONE,
    F_TGA,
    F_BMP,
    F_LMP,
    F_MIP,
    F_WAL,
    F_M8,
    F_M32
} format_t;

void fprintHelp(FILE *file)
{
    fprintf(file, "Texture v0.8 by Vladimir Stupin\n"
                  "Usage: texture [<options>] <inputName> [<outputName>]\n"
                  "Options for specify output format:\n"
                  "\t[-tga] (default), -bmp, -lmp, -mip, -wal, -m32.\n"
                  "Special options:\n"
                  "\t[-quake] - output format is Quake LMP or MIP (default).\n"
                  "\t-halflife - output format is Half-Life LMP or MIP.\n"
                  "\t-alpha - enable alpha-channel by input 32-bits TGA.\n"
                  "Notes:\n"
                  "\tIf outputName not specify, then outputName is inputName with extension\n"
                  "\t\tof output format.\n"
                  "\tIf input file is WAD-file, then outputName is name of output directory.\n"
                  "Supported formats:\n"
                  "\tTGA - TARGA image format (partial input/partial output).\n"
                  "\tBMP - Windows Bitmap image format (partial input/partial output).\n"
                  "\tMIP - Quake/Half-Life texture format (input/output).\n"
                  "\tLMP - Quake/Half-Life picture format (input/output).\n"
                  "\tWAL - Quake 2 texture format (input/output).\n"
                  "\tM8  - Heretic 2 texture/picture format (input/output).\n"
                  "\tM32 - Heretic 2 picture format (input/output).\n"
                  "\tWAD - Quake/Half-Life WAD2/WAD3 group graphics format (only input).\n");
}

int main(const int carg, const char *varg[])
{
    char *srcName = NULL;
    s_strcpy(&srcName, "");

    char *dstName = NULL;
    s_strcpy(&dstName, "");

    char *dstExt = NULL;
    s_strcpy(&dstExt, "");

    format_t dstFormat = F_NONE;
    int lmMode = LM_MODE_QUAKE;
    int alpha = 0;

    // Сканируем опции и имена файлов
    for(int i = 1; i < carg; i++)
    {
        if (strcasecmp(varg[i], "-tga") == 0)
        {
            dstFormat = F_TGA;
        }
        else if (strcasecmp(varg[i], "-bmp") == 0)
        {
            dstFormat = F_BMP;
        }
        else if (strcasecmp(varg[i], "-lmp") == 0)
        {
            dstFormat = F_LMP;
        }
        else if (strcasecmp(varg[i], "-mip") == 0)
        {
            dstFormat = F_MIP;
        }
        else if (strcasecmp(varg[i], "-wal") == 0)
        {
            dstFormat = F_WAL;
        }
        else if (strcasecmp(varg[i], "-m8") == 0)
        {
            dstFormat = F_M8;
        }
        else if (strcasecmp(varg[i], "-m32") == 0)
        {
            dstFormat = F_M32;
        }
        else if (strcasecmp(varg[i], "-quake") == 0)
        {
            lmMode = LM_MODE_QUAKE;
        }
        else if (strcasecmp(varg[i], "-halflife") == 0)
        {
            lmMode = LM_MODE_HALFLIFE;
        }
        else if (strcasecmp(varg[i], "-alpha") == 0)
        {
            alpha = 1;
        }
        else
        {
            s_strcpy(&srcName, varg[i]);
            i++;
            if (i < carg)
            {
                s_strcpy(&dstName, varg[i]);
            }
        }
    }

    // Программа не будет работать, если не указано имя исходного файла
    if (strlen(srcName) == 0)
    {
        s_free(&dstExt);
        s_free(&dstName);
        s_free(&srcName);
        fprintHelp(stderr);
        return 1;
    }

    // Если не указано имя целевого файла, то берётся имя исходного файла без расширения
    // и добавляется расширение, соответствующее целевому формату
    if (strlen(dstName) == 0)
    {
        s_pathname(&dstName, srcName);

        // Заполняем расширение целевого файла
        switch (dstFormat)
        {
            default:
            case F_NONE:
            case F_TGA:
                s_strcpy(&dstExt, ".tga");
                break;
            case F_BMP:
                s_strcpy(&dstExt, ".bmp");
                break;
            case F_LMP:
                s_strcpy(&dstExt, ".lmp");
                break;
            case F_MIP:
                s_strcpy(&dstExt, ".mip");
                break;
            case F_WAL:
                s_strcpy(&dstExt, ".wal");
                break;
            case F_M8:
                s_strcpy(&dstExt, ".m8");
                break;
            case F_M32:
                s_strcpy(&dstExt, ".m32");
                break;
        }
    }
    // Если не указан целевой формат, пробуем определить его по расширению имени целевого файла
    else if (dstFormat == F_NONE)
    {
        for(int i = strlen(dstName) - 1; i >= 0; i--)
        {
            if (dstName[i] == SYS_PATH_DELIM)
            {
                break;
            }
            else if (dstName[i] == '.')
            {
                if (strcasecmp(&dstName[i], ".tga") == 0)
                {
                    dstFormat = F_TGA;
                }
                else if (strcasecmp(&dstName[i], ".bmp") == 0)
                {
                    dstFormat = F_BMP;
                }
                else if (strcasecmp(&dstName[i], ".lmp") == 0)
                {
                    dstFormat = F_LMP;
                }
                else if (strcasecmp(&dstName[i], ".mip") == 0)
                {
                    dstFormat = F_MIP;
                }
                else if (strcasecmp(&dstName[i], ".wal") == 0)
                {
                    dstFormat = F_WAL;
                }
                else if (strcasecmp(&dstName[i], ".m8") == 0)
                {
                    dstFormat = F_M8;
                }
                else if (strcasecmp(&dstName[i], ".m32") == 0)
                {
                    dstFormat = F_M32;
                }
                dstName[i] = 0;
                break;
            }
        }

        // Если формат целевого файла не удалось определить по расширению, то по умолчанию используем формат TGA
        if (dstFormat == F_NONE)
        {
            dstFormat = F_TGA;
        }
    }
 
    // Открываем исходный файл
    wad2_t wad;
    data_t data;
    image_t image;
    if (wad.openWAD(srcName) == 0)
    {
        mkdir(dstName, 640);
        for(int i = 0; i < wad.getNumber(); i++)
        {
            if (wad.loadData(i, &data) != 0)
            {
                continue;
            }
            if (image.readLMP(&data) != 0)
            {
                if (image.readMIP(&data) != 0)
                {
                    continue;
                }
            }
            switch (dstFormat)
            {
                default:
                case F_NONE:
                case F_TGA:
                    if (image.writeTGA(&data) != 0)
                    {
                        continue;
                    }
                    break;

                case F_BMP:
                    if (image.writeBMP(&data) != 0)
                    {
                        continue;
                    }
                    break;

                case F_LMP:
                    if (image.writeLMP(&data, lmMode) != 0)
                    {
                        continue;
                    }
                    break;

                case F_MIP:
                    if (image.writeMIP(&data, lmMode) != 0)
                    {
                        continue;
                    }
                    break;

                case F_WAL:
                    if (image.writeWAL(&data) != 0)
                    {
                        continue;
                    }
                    break;

                case F_M8:
                    if (image.writeM8(&data) != 0)
                    {
                        continue;
                    }
                    break;

                case F_M32:
                    if (image.writeM32(&data) != 0)
                    {
                        continue;
                    }
                    break;
            }

            char *name = NULL;
            wad.getName(i, &name);

            char *fileName = NULL;
            s_strcpy(&fileName, dstName);
            s_strcat(&fileName, "\\");
            s_strcat(&fileName, name);
            s_strcat(&fileName, dstExt);

            data.saveData(fileName);

            s_free(&name);
            s_free(&fileName);
        }
    }
    else
    {
        if (data.loadData(srcName) != 0)
        {
            s_free(&dstExt);
            s_free(&dstName);
            s_free(&srcName);
            return 2;
        }

        if (image.readBMP(&data) != 0)
        {
            if (image.readTGA(&data,alpha) != 0)
            {
                if (image.readLMP(&data) != 0)
                {
                    if (image.readMIP(&data) != 0)
                    {
                        if (image.readWAL(&data) != 0)
                        {
                            if (image.readM8(&data) != 0)
                            {
                                if (image.readM32(&data) != 0)
                                {
                                    s_free(&dstExt);
                                    s_free(&dstName);
                                    s_free(&srcName);
                                    return 3;
                                }
                            }
                        }
                    }
                }
            }
        }

        switch (dstFormat)
        {
            default:
            case F_NONE:
            case F_TGA:
                if (image.writeTGA(&data) != 0)
                {
                    s_free(&dstExt);
                    s_free(&dstName);
                    s_free(&srcName);
                    return 4;
                }
                break;

            case F_BMP:
                if (image.writeBMP(&data) != 0)
                {
                    s_free(&dstExt);
                    s_free(&dstName);
                    s_free(&srcName);
                    return 5;
                }
                break;

            case F_LMP:
                if (image.writeLMP(&data, lmMode) != 0)
                {
                    s_free(&dstExt);
                    s_free(&dstName);
                    s_free(&srcName);
                    return 6;
                }
                break;

            case F_MIP:
                if (image.writeMIP(&data, lmMode) != 0)
                {
                    s_free(&dstExt);
                    s_free(&dstName);
                    s_free(&srcName);
                    return 7;
                }
                break;

            case F_WAL:
                if (image.writeWAL(&data) != 0)
                {
                    s_free(&dstExt);
                    s_free(&dstName);
                    s_free(&srcName);
                    return 8;
                }
                break;

            case F_M8:
                if (image.writeM8(&data) != 0)
                {
                    s_free(&dstExt);
                    s_free(&dstName);
                    s_free(&srcName);
                    return 9;
                }
                break;

            case F_M32:
                if (image.writeM32(&data) != 0)
                {
                    s_free(&dstExt);
                    s_free(&dstName);
                    s_free(&srcName);
                    return 10;
                }
                break;
        }

        char *fileName = NULL;
        s_strcpy(&fileName, dstName);
        s_strcat(&fileName, dstExt);
        data.saveData(fileName);
        s_free(&fileName);
    }

    s_free(&dstExt);
    s_free(&dstName);
    s_free(&srcName);
}

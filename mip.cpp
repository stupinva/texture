#include <string.h>
#include <stdio.h>
#include "str.h"
#include "image.h"

const int MIP_NAME0_LENGTH = 16;
const int MIP_LEVELS = 4;

#pragma pack(push, 1)
struct mipHeader_t
{
    char name0[MIP_NAME0_LENGTH];
    unsigned long width;
    unsigned long height;
    unsigned long offset[MIP_LEVELS];
};
#pragma pack(pop)

int image_t::readMIP(data_t *dat)
{    
    // Читаем заголовок
    mipHeader_t header;
    dat->seekData(0);
    if (dat->readData(&header, sizeof(header)) != sizeof(header))
    {
        return -1;
    }

    // Если ширина или высота изображения нулевые, то это не файл в формате MIP
    if ((header.width == 0) || (header.height == 0))
    {
        return -1;
    }

    // Если ширина и высота изображения не кратны 8, то это не файл в формате MIP
    if (((header.width % 8) != 0) || ((header.height % 8) != 0))
    {
        return -1;
    }
    // Проверяем, что изображения всех уровней умещаются в файле
    if (dat->sizeData() < header.offset[0] + header.width * header.height)
    {
        return -1;
    }
    if (dat->sizeData() < header.offset[1] + header.width * header.height / 4)
    {
        return -1;
    }
    if (dat->sizeData() < header.offset[2] + header.width * header.height / 16)
    {
        return -1;
    }
    if (dat->sizeData() < header.offset[3] + header.width * header.height / 64)
    {
        return -1;
    }
    // Проверяем, что изображения всех уровней идут друг за другом
    if ((header.offset[0] > header.offset[1]) || (header.offset[1] > header.offset[2]) || (header.offset[2] > header.offset[3]))
    {
        return -1;
    }
    // Загружаем изображение
    if (setWidthHeight(header.width, header.height, T_INDEX_BGR) != 0)
    {
        return -1;
    }
    dat->seekData(header.offset[0]);
    dat->readData(data, width * height);

    // Заполняем дополнительную информацию
    s_strncpy(&text, header.name0, MIP_NAME0_LENGTH);
    xOffset = 0;
    yOffset = 0;

    // Читаем размер палитры, если она есть
    dat->seekData(header.offset[3] + (width * height / 64));
    unsigned short length;
    // Если палитры нет, используем палитру Quake
    if (dat->readData(&length, sizeof(length)) != sizeof(length))
    {
        setQuakeMap();
        fprintf(stdout, "readMIP: %ix%ix8 %s\n", width, height, text);
        return 0;
    }
    // Если палитра есть, загружаем её
    map.length = length;
    if (map.length > 256)
    {
        map.length = 256;
    }
    for(int i = 0; i < map.length; i++)
    {
        map.map[i].red = dat->readChar();
        map.map[i].green = dat->readChar();
        map.map[i].blue = dat->readChar();
    }

    // Выводим информацию
    fprintf(stdout, "readMIP: %ix%ix8 %ix24 %s\n", width, height, map.length, text);
    return 0;
}

int image_t::writeMIP(data_t *dat, const int lmMode)
{
    if (type == T_NULL)
    {
        return -1;
    }
    // Если размеры изображения не кратны 8, то из него нельзя создать пирамидные уровни
    if ((width % 8 != 0) || (height % 8 != 0))
    {
        return -1;
    }

    mipHeader_t header;
    image_t image[MIP_LEVELS];
    image_t qImage[MIP_LEVELS];
    colorStat_t stat;
    map_t map;
    int i;
    char name0[MIP_NAME0_LENGTH];
    unsigned short length;
    switch (lmMode)
    {
        case LM_MODE_QUAKE:
            if (dat->allocData(sizeof(header) + 85 * width * height / 64) != 0)
            {
                return -1;
            }

            // Создаём основное изображение
            if (image[0].setImage(this, T_BGR) != 0)
            {
                return -1;
            }

            // И создаём копии, уменьшенные в 2 раза по ширине и высоте
            for(i = 1; i < MIP_LEVELS; i++)
            {
                if (image[i].buildMipImage(&image[i-1], T_BGR) != 0)
                {
                    return -1;
                }
            }

            // Приводим все копии к палитре Quake
            for(i = 0; i < MIP_LEVELS; i++)
            {
                qImage[i].setQuakeMap();
                if (qImage[i].setImage(&image[i], T_INDEX_BGR) != 0)
                {
                    return -1;
                }
            }

            // Записываем все уровни изображения
            dat->seekData(sizeof(header));
            header.offset[0] = dat->tellData();
            dat->writeData(qImage[0].data, width * height);
            header.offset[1] = dat->tellData();
            dat->writeData(qImage[1].data, width * height / 4);
            header.offset[2] = dat->tellData();
            dat->writeData(qImage[2].data, width * height / 16);
            header.offset[3] = dat->tellData();
            dat->writeData(qImage[3].data, width * height / 64);

            // Заполняем заголовок
            header.width = width;
            header.height = height;
            memset(header.name0, 0, MIP_NAME0_LENGTH);
            strncpy(header.name0, text, MIP_NAME0_LENGTH);

            // Записываем заголовок
            dat->seekData(0);
            dat->writeData(&header, sizeof(header));

            // Выводим информацию
            strncpy(name0, text, MIP_NAME0_LENGTH);
            name0[MIP_NAME0_LENGTH] = 0;
            fprintf(stdout, "writeMIP: %ix%ix8 %s\n", width, height, name0);
            break;

        case LM_MODE_HALFLIFE:
            if (dat->allocData(sizeof(header) + 85 * width * height / 64 + sizeof(unsigned short) + 256 * sizeof(BGR)) != 0)
            {
                return -1;
            }

            // Создаём основное изображение
            if (image[0].setImage(this, T_BGR) != 0)
            {
                return -1;
            }

            // И создаём копии, уменшенные в 2 раза по ширине и высоте
            for(i = 1; i < MIP_LEVELS; i++)
            {
                if (image[i].buildMipImage(&image[i - 1], T_BGR) != 0)
                {
                    return -1;
                }
            }

            // Вычисляем палитру, общую для всех копий изображения
            for(i = 0; i < MIP_LEVELS; i++)
            {
                if (stat.addImageColors(&image[i]))
                {
                    return -1;
                }
            }
            stat.getMap(&map);

            // Приводим все уровни изображения к общей палитре
            for(i = 0; i < MIP_LEVELS; i++)
            {
                qImage[i].setMap(&map);
                if (qImage[i].setImage(&image[i], T_INDEX_BGR) != 0)
                {
                    return -1;
                }
            }

            // Записываем все изображения
            dat->seekData(sizeof(header));
            header.offset[0] = dat->tellData();
            dat->writeData(qImage[0].data, width * height);
            header.offset[1] = dat->tellData();
            dat->writeData(qImage[1].data, width * height / 4);
            header.offset[2] = dat->tellData();
            dat->writeData(qImage[2].data, width * height / 16);
            header.offset[3] = dat->tellData();
            dat->writeData(qImage[3].data, width * height / 64);

            length = map.length;

            // Записываем палитру
            dat->writeData(&length, sizeof(length));
            for(i = 0; i < map.length; i++)
            {
                dat->writeChar(map.map[i].red);
                dat->writeChar(map.map[i].green);
                dat->writeChar(map.map[i].blue);
            }

            // Заполняем заголовок
            header.width = width;
            header.height = height;
            memset(header.name0, 0, MIP_NAME0_LENGTH);
            strncpy(header.name0, name0, MIP_NAME0_LENGTH);

            // Записываем заголовок
            dat->seekData(0);
            dat->writeData(&header, sizeof(header));

            // Выводим информацию
            strncpy(name0, text, MIP_NAME0_LENGTH);
            name0[MIP_NAME0_LENGTH] = 0;
            fprintf(stdout, "writeMIP: %ix%ix8 %ix24 %s\n", width, height, map.length, name0);
            break;

        default:
            break;
    }
    return 0;
}

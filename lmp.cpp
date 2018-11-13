#include <stdio.h>
#include <string.h>
#include "str.h"
#include "image.h"

#pragma pack(push, 1)
struct lmpHeader_t
{
    unsigned long width;
    unsigned long height;
};
#pragma pack(pop)

int image_t::readLMP(data_t *dat)
{
    // Чтение заголовка
    lmpHeader_t header;
    dat->seekData(0);
    if (dat->readData(&header, sizeof(header)) != sizeof(header))
    {
        return -1;
    }

    // Проверяем наличие изображения
    if ((header.width == 0) || (header.height == 0))
    {
        return -1;
    }

    // Проверяем, что в файл имеет размер, позволяющий вместить изображение
    if (dat->sizeData() < sizeof(header) + header.width * header.height)
    {
        return -1;
    }

    // Выделяем память для загрузки изображения
    if (setWidthHeight(header.width, header.height, T_INDEX_BGR) != 0)
    {
        return -1;
    }

    // Считываем изображение
    dat->readData(data, width * height);

    // Заполняем дополнительные поля
    s_strcpy(&text, "");
    xOffset = 0;
    yOffset = 0;

    // Считываем размер палитры, если она есть
    unsigned short length;
    if (dat->readData(&length, sizeof(length)) != sizeof(length))
    {
        // Если размер палитры не прочитался, используем палитру Quake
        setQuakeMap();
        fprintf(stdout, "readLMP: %ix%ix8\n", width, height);
        return 0;
    }
    map.length = length;
    if (map.length > 256)
    {
        map.length = 256;
    }
    // Считываем палитру
    for(int i = 0; i < map.length; i++)
    {
        map.map[i].red = dat->readChar();
        map.map[i].green = dat->readChar();
        map.map[i].blue = dat->readChar();
    }

    // Выводим информацию
    fprintf(stdout, "readLMP: %ix%ix8 %ix24\n", width, height, map.length);
    return 0;
}

int image_t::writeLMP(data_t *dat, const int lmMode)
{
    if (type == T_NULL)
    {
        return -1;
    }

    image_t image;
    lmpHeader_t header;
    int i;
    unsigned short length;
    map_t map;
    colorStat_t stat;
    switch (lmMode)
    {
        case LM_MODE_QUAKE:
            if (dat->allocData(sizeof(header) + width * height) != 0)
            {
                return -1;
            }

            // Устанавливаем палитру Quake
            image.setQuakeMap();

            // Устанавливаем изображение
            image.setImage(this, T_INDEX_BGR);

            // Заполняем заголовок
            header.width = width;
            header.height = height;

            // Записываем заголовок
            dat->writeData(&header, sizeof(header));

            // Записываем изображение
            dat->writeData(image.data, width * height);

            // Выводим информацию
            fprintf(stdout, "writeLMP: %ix%ix8\n", width, height);
            break;

        case LM_MODE_HALFLIFE:
            stat.addImageColors(this);
            stat.getMap(&map);

            // Устанавливаем расчитанную палитру
            image.setMap(&map);

            // Устанавливаем изображение
            image.setImage(this, T_INDEX_BGR);

            // Заполняем заголовок
            header.width = width;
            header.height = height;

            // Выделяем память под файл
            if (dat->allocData(sizeof(header) + width * height + sizeof(unsigned short) + map.length * sizeof(BGR)) != 0)
            {
                return -1;
            }

            // Записываем заголовок
            dat->writeData(&header, sizeof(header));

            // Записываем изображение
            dat->writeData(image.data, width * height);

            // Записываем палитру
            length = map.length;
            dat->writeData(&length, sizeof(length));
            for(i = 0; i < map.length; i++)
            {
                dat->writeChar(map.map[i].red);
                dat->writeChar(map.map[i].green);
                dat->writeChar(map.map[i].blue);
            }

            // Выводим информацию
            fprintf(stdout, "writeLMP: %ix%ix8 %ix8\n", width, height, map.length);
            break;

        default:
            break;
    }
    return 0;
}

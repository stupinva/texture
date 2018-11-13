#include <stdio.h>
#include "str.h"
#include "image.h"

#pragma pack(push, 1)
struct bmpHeader_t
{
    unsigned char ident[2]; // Идентификатор формата - латинские буквы BM
    unsigned long fileSize; // Размер файла = размер заголовка + размер палитры + размер изображения
    unsigned short reserved0; // Всегда ноль
    unsigned short reserved1; // Всегда ноль
    unsigned long headerSize; // Начало изображения в файле

    unsigned long infoSize; // Всегда 0x28
    unsigned long width;
    unsigned long height;
    unsigned short planes; // Количество битовых плоскостей. Эта программа поддерживает только одну битовую плоскость
    unsigned short bits;
    unsigned long compression; // Сжатие данных. Эта программа не поддерживает сжатия
    unsigned long imageSize;
    unsigned long xPelsPerMeter;
    unsigned long yPelsPerMeter;
    unsigned long clrUsed;
    unsigned long clrImportant;
};
#pragma pack(pop)

int image_t::writeBMP(data_t *dat)
{
    if (type == T_NULL)
    {
        return 0;
    }

    // Заполняем заголовок
    bmpHeader_t header;
    header.ident[0] = 'B';
    header.ident[1] = 'M';
    header.infoSize = 0x28;
    header.reserved0 = 0;
    header.reserved1 = 0;
    header.planes = 1;
    header.compression = 0;
    header.width = width;
    header.height = height;

    switch (type)
    {
        int i;
        int lineSize;

        case T_INDEX_MONO:
            lineSize = (header.width + 3) & 0xfffffffc;

            // Продолжаем заполнение заголовка
            header.headerSize = sizeof(header) + 1024; // 256 * sizeof(BGRF) = 256 * 4
            header.imageSize = lineSize * height;
            header.fileSize = header.headerSize + header.imageSize;
            header.bits = 8;
            header.clrUsed = 8;
            header.clrImportant = 256;

            // Выделяем память под файл 
            if (dat->allocData(header.fileSize) != 0)
            {
                return -1;
            }

            // Записываем заголовок
            dat->seekData(0);
            dat->writeData(&header, sizeof(header));
 
            // Записываем палитру
            for(i = 0; i < 256; i++)
            {
                dat->writeChar(i); // Синий
                dat->writeChar(i); // Зелёный
                dat->writeChar(i); // Красный
                dat->writeChar(0); // Заполнитель
            }

            // Записываем изображение
            writeIndexDnUp(dat, lineSize);

            // Выводим информацию
            fprintf(stdout, "writeBMP: %ix%ix8 256x32\n", width, height);
            break;

        case T_INDEX_BGR:
            lineSize = (header.width + 3) & 0xfffffffc;

            // Продолжаем заполнение заголовка
            header.headerSize = sizeof(header) + map.length * 4; // sizeof(BGRF) = 4
            header.imageSize = lineSize * height;
            header.fileSize = header.headerSize + header.imageSize;
            header.bits = 8;
            header.clrUsed = 0;
            header.clrImportant = map.length;
            if (header.clrImportant == 256)
            {
                header.clrImportant = 0;
            }

            // Выделяем память под файл
            if (dat->allocData(header.fileSize) != 0)
            {
                return -1;
            }

            // Записываем заголовок
            dat->seekData(0);
            dat->writeData(&header, sizeof(header));

            // Записываем палитру
            for(i = 0; i < map.length; i++)
            {
                dat->writeChar(map.map[i].blue);
                dat->writeChar(map.map[i].green);
                dat->writeChar(map.map[i].red);
                dat->writeChar(0);
            }

            // Записываем изображение
            writeIndexDnUp(dat, lineSize);

            // Выводим информацию
            fprintf(stdout, "writeBMP: %ix%ix8 %ix32\n", width, height, map.length);
            break;

        case T_BGR:
            lineSize = (header.width * sizeof(BGR) + 3) & 0xfffffffc;

            // Продолжаем заполнение заголовка
            header.headerSize = sizeof(header);
            header.imageSize = lineSize * height;
            header.fileSize = header.headerSize + header.imageSize;
            header.bits = 24;
            // header.clrUsed = 24;
            header.clrUsed = 0;
            header.clrImportant = 0;

            // Выделяем память под файл 
            if (dat->allocData(header.fileSize) != 0)
            {
                return -1;
            }

            // Записываем заголовок
            dat->seekData(0);
            dat->writeData(&header, sizeof(header));

            // Записываем изображение
            writeBGRDnUp(dat, lineSize);

            // Выводим информацию
            fprintf(stdout, "writeBMP: %ix%ix24\n", width, height);
            break;

        case T_BGRA:
            fprintf(stderr, "writeBMP: writing image with alpha-channel is not supported.\n");
            return -1;

        default:
            return -1;
    }
    return 0;
}

int image_t::readBMP(data_t *dat)
{
    // Читаем заголовок
    dat->seekData(0);
    bmpHeader_t header;
    if (dat->readData(&header, sizeof(header)) != sizeof(header))
    {
        return -1;
    }

    // Проверяем наличие идентификатора формата
    if ((header.ident[0] != 'B') || (header.ident[1] != 'M'))
    {
        return -1;
    }

    // Проверяем, поддерживается ли програмой вариант формата этого файла
    if ((header.compression != 0) || (header.infoSize != 0x28) || (header.planes != 1))
    {
        return -1;
    }
    if ((header.bits != 1) && (header.bits != 4) && (header.bits != 8) && (header.bits != 24))
    {
        return -1;
    }

    // Продолжаем чтение
    switch (header.bits)
    {
        int i;
        int lineSize;

        case 8:
            lineSize = (header.width + 3) & 0xfffffffc;
            map.length = header.clrImportant;
            if (map.length == 0)
            {
                map.length = 256;
            }
            if (dat->sizeData() < sizeof(header) + map.length * 4 + lineSize * header.height)
            {
                return -1;
            }
            if (setWidthHeight(header.width, header.height, T_INDEX_BGR) != 0)
            {
                return -1;
            }

            // Считываем палитру 
            dat->seekData(header.headerSize - map.length * 4);
            for(i = 0; i < map.length; i++)
            {
                map.map[i].blue = dat->readChar();
                map.map[i].green = dat->readChar();
                map.map[i].red = dat->readChar();
                dat->readChar();
            }

            // Считываем изображение
            readIndexDnUp(dat, lineSize);

            // Заполняем дополнительную информацию об изображении
            s_strcpy(&text, "");
            xOffset = 0;
            yOffset = 0;

            // Выводим информацию
            fprintf(stdout, "readBMP: %ix%ix8 %ix32\n", width, height, map.length);
            break;

        case 24:
            lineSize = (header.width * sizeof(BGR) + 3) & 0xfffffffc;
            if (dat->sizeData() < sizeof(header) + lineSize * header.height)
            {
                return -1;
            }

            if (setWidthHeight(header.width, header.height, T_BGR) != 0)
            {
                return -1;
            }

            // Считываем изображение
           readBGRDnUp(dat, lineSize);

            // Заполняем дополнительную информацию об изображении
            s_strcpy(&text, "");
            xOffset = 0;
            yOffset = 0;

            // Выводим информацию
            fprintf(stdout, "readBMP: %ix%ix24\n", width, height);
            break;

        default:
            fprintf(stderr, "readBMP: bits=%i, supported only 8 and 24.\n", header.bits);
            return -1;
    }
    return 0;
}

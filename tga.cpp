#include <stdio.h>
#include <string.h>
#include "str.h"
#include "image.h"

#pragma pack(push, 1)
struct tgaHeader_t
{
    unsigned char textSize;
    unsigned char mapType;
    unsigned char dataType;
    unsigned short mapOrg;
    unsigned short mapLength;
    unsigned char mapBits;
    unsigned short xOffset;
    unsigned short yOffset;
    unsigned short width;
    unsigned short height;
    unsigned char dataBits;
    unsigned char imType;
};
#pragma pack(pop)

int image_t::writeTGA(data_t *dat)
{
    if (type == T_NULL)
    {
        return 0;
    }

    dat->seekData(0);
    tgaHeader_t header;
    int n = strlen(text);
    if (n > 255)
    {
        n = 255;
    }
    header.textSize = (unsigned char)n;
    unsigned int size;
    switch (type)
    {
        case T_INDEX_MONO:
            // Выделяем память под файл
            size = sizeof(header) + header.textSize + width * height;
            if (dat->allocData(size) != 0)
            {
                return -1;
            }

            // Записываем заголовок
            header.mapType = 0;
            header.dataType = 3;
            header.mapOrg = 0;
            header.mapLength = 0;
            header.mapBits = 0;
            header.xOffset = xOffset;
            header.yOffset = yOffset;
            header.width = width;
            header.height = height;
            header.dataBits = 8;
            header.imType = 32;
            dat->writeData(&header, sizeof(header));

            // Записываем текст
            dat->writeData(text, header.textSize);

            // Записываем изображение
            dat->writeData(data, width * height);

            // Выводим информацию
            fprintf(stdout, "writeTGA: %ix%ix8 %s\n", width, height, text);
            break;

        case T_INDEX_BGR:
            // Выделяем память под файл
            size = sizeof(header) + header.textSize + map.length * sizeof(BGR) + width * height;
            if (dat->allocData(size) != 0)
            {
                return -1;
            }

            // Записываем заголовок
            header.mapType = 1;
            header.dataType = 1;
            header.mapOrg = 0;
            header.mapLength = map.length;
            header.mapBits = 24;
            header.xOffset = xOffset;
            header.yOffset = yOffset;
            header.width = width;
            header.height = height;
            header.dataBits = 8;
            header.imType = 32;
            dat->writeData(&header, sizeof(header));

            // Записываем текст
            dat->writeData(text, header.textSize);

            // Записываем палитру
            dat->writeData(map.map, map.length * sizeof(BGR));

            // Записываем изображение
            dat->writeData(data, width * height);

            // Выводим информацию
            fprintf(stdout, "writeTGA: %ix%ix8 %ix24 %s\n", width, height, map.length, text);
            break;

        case T_BGR:
            // Выделяем память под файл
            size = sizeof(header) + header.textSize + width * height * sizeof(BGR);
            if (dat->allocData(size) != 0)
            {
                return -1;
            }

            // Записываем заголовок
            header.mapType = 0;
            header.dataType = 2;
            header.mapOrg = 0;
            header.mapLength = 0;
            header.mapBits = 0;
            header.xOffset = xOffset;
            header.yOffset = yOffset;
            header.width = width;
            header.height = height;
            header.dataBits = 24;
            header.imType = 32;
            dat->writeData(&header, sizeof(header));

            // Записываем текст
            dat->writeData(text, header.textSize);

            // Записываем изображение
            dat->writeData(data, width * height * sizeof(BGR));

            // Выводим информацию
            fprintf(stdout, "writeTGA: %ix%ix24 %s\n", width, height, text);
            break;

        case T_BGRA:
            // Выделяем память под файл
            size = sizeof(header) + header.textSize + width * height * sizeof(BGRA);
            if (dat->allocData(size) != 0)
            {
                return -1;
            }

            // Записываем заголовок
            header.mapType = 0;
            header.dataType = 2;
            header.mapOrg = 0;
            header.mapLength = 0;
            header.mapBits = 0;
            header.xOffset = xOffset;
            header.yOffset = yOffset;
            header.width = width;
            header.height = height;
            header.dataBits = 32;
            header.imType = 32;
            dat->writeData(&header, sizeof(header));

            // Записываем текст
            dat->writeData(text, header.textSize);

            // Записываем изображение
            dat->writeData(data, width * height * sizeof(BGRA));

            // Выводим информацию
            fprintf(stdout, "writeTGA: %ix%ix32 %s\n", width, height, text);
            break;

        default:
            return -1;
    }
    return 0;
}

int image_t::readTGA(data_t *dat, const int alpha)
{
    char txt[255];

    // Читаем заголовок
    dat->seekData(0);
    tgaHeader_t header;
    if (dat->readData(&header, sizeof(header)) != sizeof(header))
    {
        return -1;
    }
    switch (header.dataType)
    {
        case 1: // T_INDEX_BGR
            // Проверяем значения полей заголовка
            if (header.mapType != 1)
            {
                return -1;
            }
            if (header.mapLength > 256)
            {
                return -1;
            }
            if ((header.mapBits != 16) && (header.mapBits != 24))
            {
                return -1;
            }
            if (header.dataBits != 8)
            {
                return -1;
            }

            // Провреяем размер файла
            if (dat->sizeData() < sizeof(header) + header.textSize + header.mapLength * sizeof(BGR) + header.width * header.height)
            {
                return -1;
            }

            // Загрузка палитры с 16-битными цветами не поддерживается
            if (header.mapBits == 16)
            {
                fprintf(stderr, "readTGA: mapBits=16, support only mapBits=24\n");
                return -1;
            }

            // Задаем размеры изображения
            if (setWidthHeight(header.width, header.height, T_INDEX_BGR) != 0)
            {
                return -1;
            }

            // Считываем текст
            dat->readData(txt, header.textSize);
            s_strncpy(&text, txt, header.textSize);

            // Считываем палитру
            map.length = header.mapLength;
            dat->readData(map.map, map.length * sizeof(BGR));

            // Считываем изображение
            if ((header.imType & 0x20) == 0x20)
            {
                readIndexUpDn(dat, width);
            }
            else
            {
                readIndexDnUp(dat, width);
            }

            // Выводим информацию
            fprintf(stdout, "readTGA: %ix%ix8 %ix24 %s\n", width, height, map.length, text);
            break;

        case 2: //T_BGR
            // Проверяем значений полей заголовка
            if (header.mapType != 0)
            {
                return -1;
            }
            if (header.mapLength != 0)
            {
                return -1;
            }
            if (header.mapBits != 0)
            {
                return -1;
            }
            if ((header.dataBits != 16) && (header.dataBits != 24) && (header.dataBits != 32))
            {
                return -1;
            }
            if (header.dataBits == 16)
            {
                fprintf(stderr, "readTGA: dataBits=16, support only dataBits=24 or 32\n");
                return -1;
            }
            else if (header.dataBits == 24)
            {
                // Проверяем размер файла
                if (dat->sizeData() < sizeof(header) + header.textSize + header.width * header.height * sizeof(BGR))
                {
                    return -1;
                }

                // Устанавливаем размеры изображения
                if (setWidthHeight(header.width, header.height, T_BGR) != 0)
                {
                    return -1;
                }

                // Считываем текст
                dat->readData(txt, header.textSize);
                s_strncpy(&text, txt, header.textSize);

                // Считываем изображение
                if ((header.imType & 0x20) == 0x20)
                {
                    readBGRUpDn(dat, width * sizeof(BGR));
                }
                else
                {
                    readBGRDnUp(dat, width * sizeof(BGR));
                }

                // Выводим информацию
                fprintf(stdout, "readTGA: %ix%ix24 %s\n", width, height, text);
            }
            else if (header.dataBits == 32)
            {
                if (alpha != 0)
                {
                    // Проверяем размер файла
                    if (dat->sizeData() < sizeof(header) + header.textSize + header.width * header.height * sizeof(BGRA))
                    {
                        return -1;
                    }

                    // Устанавливаем размеры изображения
                    if (setWidthHeight(header.width, header.height, T_BGRA) != 0)
                    {
                        return -1;
                    }

                    // Считываем текст
                    dat->readData(txt, header.textSize);
                    s_strncpy(&text, txt, header.textSize);

                    // Считываем изображение
                    if ((header.imType & 0x20) == 0x20)
                    {
                        readBGRAUpDn(dat, width * sizeof(BGRA));
                    }
                    else
                    {
                        readBGRADnUp(dat, width * sizeof(BGRA));
                    }

                    // Выводим информацию
                    fprintf(stdout, "readTGA: %ix%ix32 alpha %s\n", width, height, text);
                }
                else
                {
                    // Проверяем размер файла
                    if (dat->sizeData() < sizeof(header) + header.textSize + header.width * header.height * sizeof(BGRA))
                    {
                        return -1;
                    }

                    // Устанавливаем размеры изображения
                    if (setWidthHeight(header.width, header.height, T_BGR) != 0)
                    {
                        return -1;
                    }

                    // Считываем текст
                    dat->readData(txt, header.textSize);
                    s_strncpy(&text, txt, header.textSize);

                    // Считываем изображение
                    if ((header.imType & 0x20) == 0x20)
                    {
                        readBGRFUpDn(dat, width * sizeof(BGRA));
                    }
                    else
                    {
                        readBGRFDnUp(dat, width * sizeof(BGRA));
                    }

                    // Выводим информацию
                    fprintf(stdout, "readTGA: %ix%ix32 %s\n", width, height, text);
                }
            }
            else
            {
                return -1;
            }
            break;

        case 3: // T_INDEX_MONO
            // Проверяем значения полей заголовка
            if (header.mapType != 0)
            {
                return -1;
            }
            if (header.mapLength != 0)
            {
                return -1;
            }
            if (header.mapBits != 0)
            {
                return -1;
            }
            if (header.dataBits != 8)
            {
                return -1;
            }

            // Проверяем размер файла
            if (dat->sizeData() < sizeof(header) + header.textSize + width * height)
            {
                return -1;
            }

            // Устанавливаем размер изображения
            if (setWidthHeight(header.width, header.height, T_INDEX_MONO) != 0)
            {
                return -1;
            }

            // Считываем текст
            dat->readData(txt, header.textSize);
            s_strncpy(&text, txt, header.textSize);

            // Считываем данные
            if ((header.imType & 0x20) == 0x20)
            {
                readIndexUpDn(dat, width);
            }
            else
            {
                readIndexDnUp(dat, width);
            }

            // Выводим информацию
            fprintf(stdout, "readTGA: %ix%ix8 %s\n", width, height, text);
            break;

        default:
            return -1;
    }
    return 0;
}

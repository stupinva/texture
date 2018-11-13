#include <stdio.h>
#include <string.h>
#include "str.h"
#include "image.h"

const unsigned long M32_IDENT = 4;
const int M32_NAME0_LENGTH = 32;
const int M32_MIP_LEVELS = 16;

#pragma pack(push, 1)
struct m32Header_t
{
    unsigned long ident;
    char name0[M32_NAME0_LENGTH];
    char reserved0[480];
    unsigned long width[M32_MIP_LEVELS];
    unsigned long height[M32_MIP_LEVELS];
    unsigned long offset[M32_MIP_LEVELS];
    char reserved1[256];
};
#pragma pack(pop)

int image_t::readM32(data_t *dat)
{
    // Читаем заголовок
    m32Header_t header;
    dat->seekData(0);
    if (dat->readData(&header, sizeof(header)) != sizeof(header))
    {
        return -1;
    }

    // Проверяем идентификатор
    if (header.ident != M32_IDENT)
    {
        return -1;
    }

    // Проверяем, что файл может содержать изображение
    if (dat->sizeData() < header.offset[0] + header.width[0] * header.height[0] * sizeof(BGRA))
    {
        return -1;
    }
    // Устанавливаем размеры изображения
    if (setWidthHeight(header.width[0], header.height[0], T_BGRA) != 0)
    {
        return -1;
    }

    // Читаем изображение 
    dat->seekData(header.offset[0]);
    for(int i = 0; i < width * height; i++)
    {
        BGRA tColor;
        tColor.red = dat->readChar();
        tColor.green = dat->readChar();
        tColor.blue = dat->readChar();
        tColor.alpha = dat->readChar();
        ((BGRA *)data)[i] = tColor;
    }

    // Заполняем дополнительные поля
    s_strncpy(&text, header.name0, M32_NAME0_LENGTH);
    xOffset = 0;
    yOffset = 0;

    // Выводим информацию
    fprintf(stdout, "readM32: %ix%ix32 %s\n", width, height, text);
    return 0;
}

/*
int image_t::writeM32(data_t *dat)
{
    // Выделяем память для формируемого файла
    m32Header_t header;
    if (dat->allocData(sizeof(header) + width * height * sizeof(BGRA)) != 0)
    {
        return -1;
    }

    // Заполняем заголовок
    header.ident = M32_IDENT;
    header.width[0] = width;
    header.height[0] = height;
    header.offset[0] = sizeof(header);
    memset(header.name0, 0, M32_NAME0_LENGTH);
    strncpy(header.name0, text, M32_NAME0_LENGTH);
    dat->writeData(&header, sizeof(header));

    // Записываем изображение
    if (type == T_BGRA)
    {
        for(int i = 0; i < width * height; i++)
        {
            BGRA tColor = ((BGRA *)data)[i];
            dat->writeChar(tColor.red);
            dat->writeChar(tColor.green);
            dat->writeChar(tColor.blue);
            dat->writeChar(tColor.alpha);
        }
    }
    else
    {
        image_t image;
        image.setImage(this, T_BGRA);
        for(int i = 0; i < width * height; i++)
        {
            BGRA tColor = ((BGRA *)image.data)[i];
            dat->writeChar(tColor.red);
            dat->writeChar(tColor.green);
            dat->writeChar(tColor.blue);
            dat->writeChar(tColor.alpha);
        }
    }

    // Выводим информацию
    char name0[M32_NAME0_LENGTH + 1];
    strncpy(name0, text, M32_NAME0_LENGTH);
    name0[M32_NAME_LENGTH] = 0;
    fprintf(stdout, "writeM32: %ix%ix32 %s\n", width, height, name0);
    return 0;
}
*/

int image_t::writeM32(data_t *dat)
{
    image_t image[M32_MIP_LEVELS];
    image[0].setImage(this, T_BGRA);
    for(int i = 1; i < M32_MIP_LEVELS; i++)
    {
        image[i].buildMipImage(&image[i - 1], T_BGRA);
    }

    // Заполняем заголовок
    m32Header_t header;
    memset(&header, 0, sizeof(header));
    unsigned int size = sizeof(header);
    for(int i = 0; i < M32_MIP_LEVELS; i++)
    {
        header.offset[i] = size;
        header.width[i] = image[i].width;
        header.height[i] = image[i].height;
        size += header.width[i] * header.height[i] * sizeof(BGRA);
    }

    // Выделяем память под формируемый файл
    if (dat->allocData(size) != 0)
    {
        return -1;
    }

    // Заполняем поле name0
    memset(header.name0, 0, M32_NAME0_LENGTH);
    strncpy(header.name0, text, M32_NAME0_LENGTH);
    header.ident = M32_IDENT;

    // Записываем заголовок
    dat->writeData(&header, sizeof(header));

    // Записываем изображение
    for(int i = 0; i < M32_MIP_LEVELS; i++)
    {
        for(int j = 0; j < image[i].width * image[i].height; j++)
        {
            BGRA tColor = ((BGRA *)image[i].data)[j];
            dat->writeChar(tColor.red);
            dat->writeChar(tColor.green);
            dat->writeChar(tColor.blue);
            dat->writeChar(tColor.alpha);
        }
    }

    // Выводим информацию
    char name0[M32_NAME0_LENGTH + 1];
    strncpy(name0, text, M32_NAME0_LENGTH);
    name0[M32_NAME0_LENGTH] = 0;
    fprintf(stdout, "writeM32: %ix%ix32 %s\n", width, height, name0);
    return 0;
}

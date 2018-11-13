#include <stdio.h>
#include <string.h>
#include "str.h"
#include "image.h"

const int WAL_NAME0_LENGTH = 32;
const int WAL_NAME1_LENGTH = 32;
const int WAL_MIP_LEVELS = 4;

#pragma pack(push,1)
struct walHeader_t
{
    char name0[WAL_NAME0_LENGTH];
    unsigned long width;
    unsigned long height;
    unsigned long offset[WAL_MIP_LEVELS];
    char name1[WAL_NAME1_LENGTH];
};
#pragma pack(pop)

int image_t::readWAL(data_t *dat)
{    
    // Считываем заголовок
    walHeader_t header;
    dat->seekData(0);
    if (dat->readData(&header, sizeof(header)) != sizeof(header))
    {
        return -1;
    }

    // Проверяем размеры изображения
    if ((header.width == 0) || (header.height == 0))
    {
        return -1;
    }
    if (((header.width % 8) != 0) || ((header.height % 8) != 0))
    {
        return -1;
    }

    // Проверяем, что файл может содержать все уровни изображения
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
    if ((header.offset[0] > header.offset[1]) || (header.offset[1] > header.offset[2]) || (header.offset[2] > header.offset[3]))
    {
        return -1;
    } 

    // Задаём размер изображения
    if (setWidthHeight(header.width, header.height, T_INDEX_BGR) != 0)
    {
        return -1;
    }

    // Считываем изображение
    dat->seekData(header.offset[0]);
    dat->readData(data, width * height);
 
    // Считываем прочую информацию
    char text0[WAL_NAME0_LENGTH + 1];
    strncpy(text0, header.name0, WAL_NAME0_LENGTH);
    text0[WAL_NAME0_LENGTH] = 0;

    char text1[WAL_NAME1_LENGTH + 1];
    strncpy(text1, header.name1, WAL_NAME1_LENGTH);
    text1[WAL_NAME1_LENGTH] = 0;

    s_strcpy(&text, text0);
    s_strcat(&text, " ");
    s_strcat(&text, text1);
    xOffset = 0;
    yOffset = 0;

    // Устанавливаем палитру изображения из Quake 2
    setQuake2Map();

    // Выводим информацию
    fprintf(stdout, "readWAL: %ix%ix8 %s\n", width, height, text);
    return 0;
}

int image_t::writeWAL(data_t *dat)
{
    if (type == T_NULL)
    {
        return -1;
    }

    // Проверяем размер изображения
    if ((width % 8 != 0) || (height % 8 != 0))
    {
        return -1;
    }

    // Выдяеляем памят под файл
    walHeader_t header;
    // size = sizeof(header) + width * height + (width / 2) * (height / 2) + (width / 4) * (height / 4) + (width / 8) * (height / 8)
    if (dat->allocData(sizeof(header) + 85 * width * height / 64) != 0)
    {
        return -1;
    }

    // Создаём исходный уровень изображения
    image_t image[WAL_MIP_LEVELS];
    if (image[0].setImage(this, T_BGR) != 0)
    {
        return -1;
    }

    // Строим копии изображений, уменьшенные по ширине и высоте в 2 раза
    for(unsigned int i = 1; i < WAL_MIP_LEVELS; i++)
    {
        if (image[i].buildMipImage(&image[i - 1], T_BGR) != 0)
        {
            return -1;
        }
    }

    // Приводим все уровни изображений к палитре Quake 2
    image_t qImage[WAL_MIP_LEVELS];
    for(unsigned i = 0; i < WAL_MIP_LEVELS; i++)
    {
        qImage[i].setQuake2Map();
        if (qImage[i].setImage(&image[i], T_INDEX_BGR) != 0)
        {
            return -1;
        }
    }

    // Записываем все уровни изображений
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

    char name0[WAL_NAME0_LENGTH + 1];
    strncpy(name0, text, WAL_NAME0_LENGTH);
    name0[WAL_NAME0_LENGTH] = 0;

    char name1[WAL_NAME1_LENGTH + 1];
    name1[0] = 0;

    for(unsigned i = 0; i < strlen(text); i++)
    {
        if (text[i] == ' ')
        {
            int n = i;
            if (n > WAL_NAME0_LENGTH)
            {
                n = WAL_NAME0_LENGTH;
            }
            strncpy(name0, text, n);
            name0[n] = 0;
            strncpy(name1, &text[i + 1], WAL_NAME1_LENGTH);
            name1[WAL_NAME1_LENGTH] = 0;
        }
    }
    memset(header.name0, 0, WAL_NAME0_LENGTH);
    strncpy(header.name0, name0, WAL_NAME0_LENGTH);
    memset(header.name1, 0, WAL_NAME1_LENGTH);
    strncpy(header.name1, name1, WAL_NAME1_LENGTH);

    // Записываем заголовок
    dat->seekData(0);
    dat->writeData(&header, sizeof(header));

    // Выводим информацию
    fprintf(stdout, "writeWAL: %ix%ix8 %s %s\n", width, height, name0, name1);
    return 0;
}

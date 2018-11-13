#include <stdio.h>
#include <string.h>
#include "str.h"
#include "image.h"

const unsigned long M8_IDENT = 2;
const int M8_NAME0_LENGTH = 32;
const int M8_NAME1_LENGTH = 32;
const int M8_MIP_LEVELS = 16;
const int M8_MAP_LENGTH = 256;

#pragma pack(push, 1)
struct m8Header_t
{
    unsigned long ident;
    char name0[M8_NAME0_LENGTH];
    unsigned long width[M8_MIP_LEVELS];
    unsigned long height[M8_MIP_LEVELS];
    unsigned long offset[M8_MIP_LEVELS];
    char name1[M8_NAME1_LENGTH];
    unsigned char map[M8_MAP_LENGTH * 3]; // rgb
};
#pragma pack(pop)

int image_t::readM8(data_t *dat)
{
    // Чтение заголовка
    dat->seekData(0);
    m8Header_t header;
    if (dat->readData(&header, sizeof(header)) != sizeof(header))
    {
        return -1;
    }

    // Проверяем идентификатор
    if (header.ident != M8_IDENT)
    {
        return -1;
    }
    // Провреяем, что файл может содержать изображение
    if (dat->sizeData() < header.offset[0] + header.width[0] * header.height[0])
    {
        return -1;
    }

    // Устанавливаем размер изображения
    if (setWidthHeight(header.width[0], header.height[0], T_INDEX_BGR) != 0)
    {
        return -1;
    }

    // Считываем изображение
    dat->seekData(header.offset[0]);
    dat->readData(data, width * height);

    // Заполняем палитру
    map.length = M8_MAP_LENGTH;
    int srcOffset = 0;
    for(int i = 0; i < map.length; i++)
    {
        map.map[i].red = header.map[srcOffset++];
        map.map[i].green = header.map[srcOffset++];
        map.map[i].blue = header.map[srcOffset++];
    }

    // Запоминаем дополнительную информацию
    char text0[M8_NAME0_LENGTH + 1];
    strncpy(text0, header.name0, M8_NAME0_LENGTH);
    text0[M8_NAME0_LENGTH] = 0;
    char text1[M8_NAME1_LENGTH + 1];
    strncpy(text1, header.name1, M8_NAME1_LENGTH);
    text1[M8_NAME1_LENGTH] = 0;

    s_strcpy(&text, text0);
    s_strcat(&text, " ");
    s_strcat(&text, text1);
    xOffset = 0;
    yOffset = 0;

    // Выводим информацию
    fprintf(stdout, "readM8: %ix%ix8 256x24 %s\n", width, height, text);
    return 0;
}

int image_t::writeM8(data_t *dat)
{
    // Готовим копии текстуры, уменьшенные в два раза по ширине и по высоте
    image_t image[M8_MIP_LEVELS];
    image[0].setImage(this, T_BGR);
    for(int i = 1; i < M8_MIP_LEVELS; i++)
    {
        image[i].buildMipImage(&image[i - 1], T_BGR);
    }

    // Собираем статистику по цветам, используемых на всех изображениях
    colorStat_t stat;
    for(int i = 0; i < M8_MIP_LEVELS; i++)
    {
        stat.addImageColors(&image[i]);
    }

    // Формируем палитру, общую для всех изображений
    map_t map;
    stat.getMap(&map);
    image_t qImage[M8_MIP_LEVELS];
    for(int i = 0; i < M8_MIP_LEVELS; i++)
    {
        qImage[i].setMap(&map);
        qImage[i].setImage(&image[i], T_INDEX_BGR);
    }

    // Заполняем заголовок
    m8Header_t header;
    header.ident = M8_IDENT;
    unsigned int dstOffset = 0;

    // Запоминаем в заголовке палитру
    for(int i = 0; i < map.length; i++)
    {
        header.map[dstOffset++] = map.map[i].red;
        header.map[dstOffset++] = map.map[i].green;
        header.map[dstOffset++] = map.map[i].blue;
    }

    // Запоминаем в заголовке размеры и положение всех изображений в файле
    unsigned int size = sizeof(header);
    for(int i = 0; i < M8_MIP_LEVELS; i++)
    {
        header.offset[i] = size;
        header.width[i] = qImage[i].width;
        header.height[i] = qImage[i].height;
        size += header.width[i] * header.height[i];
    }

    // Выделяем память под формируемый файл
    if (dat->allocData(size) != 0)
    {
        return -1;
    }

    // Заполняем поля name0 и name1 в заголовке
    char name0[M8_NAME0_LENGTH + 1];
    strncpy(name0, text, M8_NAME0_LENGTH);
    name0[M8_NAME0_LENGTH] = 0;

    char name1[M8_NAME1_LENGTH + 1];
    name1[0] = 0;

    for(unsigned int j = 0; j < strlen(text); j++)
    {
        if (text[j] == ' ')
        {
            int n = j;
            if (n > M8_NAME0_LENGTH)
            {
                n = M8_NAME0_LENGTH;
            }
            strncpy(name0, text, n);
            name0[n] = 0;
            strncpy(name1, &text[j + 1], M8_NAME1_LENGTH);
            name1[M8_NAME1_LENGTH] = 0;
        }
    }
    memset(header.name0, 0, M8_NAME0_LENGTH);
    strcpy(header.name0, name0);
    memset(header.name1, 0, M8_NAME1_LENGTH);
    strcpy(header.name1, name1);

    // Записываем заголовок
    dat->seekData(0);
    dat->writeData(&header, sizeof(header));

    // Записываем изображения
    for(int i = 0; i < M8_MIP_LEVELS; i++)
    {
        dat->writeData(qImage[i].data, qImage[i].width * qImage[i].height);
    }

    // Выводим информацию
    fprintf(stdout, "writeM8: %ix%ix8 %ix24 %s %s\n", width, height, map.length, name0, name1);
    return 0;
}

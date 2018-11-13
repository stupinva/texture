#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "str.h"
#include "image.h"
#include "q1map.h"
#include "q2map.h"

image_t::image_t(void)
{
    xOffset = 0;
    yOffset = 0;
    text = NULL;
    s_strcpy(&text, "");
    type = T_NULL;
}

image_t::~image_t(void)
{
    s_free(&text);
    if (type != T_NULL)
    {
        free(data);
    }
}

int image_t::getWidth(void)
{
    if (type == T_NULL)
    {
        return 0;
    }

    return width;
}

int image_t::getHeight(void)
{
    if (type == T_NULL)
    {
        return 0;
    }

    return height;
}

int image_t::setWidthHeight(const int w, const int h, const imageType_t t)
{
    if (type != T_NULL)
    {
        free(data);
        type = T_NULL;
    }

    switch (t)
    {
        case T_INDEX_MONO:
        case T_INDEX_BGR:
            data = malloc(w * h);
            if (data == NULL)
            {
                fprintf(stderr, "setWidthHeight: failed to allocate %d bytes of memory\n", w * h);
                return -1;
            }

            width = w;
            height = h;
            type = t;
            break;

        case T_BGR:
            data = malloc(w * h * sizeof(BGR));
            if (data == NULL)
            {
                fprintf(stderr, "setWidthHeight: failed to allocate %ld bytes of memory\n", w * h * sizeof(BGR));
                return -1;
            }

            width = w;
            height = h;
            type = t;
            break;

        case T_BGRA:
            data = malloc(w * h * sizeof(BGRA));
            if (data == NULL)
            {
                fprintf(stderr, "setWidthHeight: failed to allocate %ld bytes of memory\n", w * h * sizeof(BGRA));
                return -1;
            }

            width = w;
            height = h;
            type = t;
            break;

        default:
            break;
    }

    return 0;
}

void image_t::setMap(const map_t *m)
{
    if (m == NULL)
    {
        map.length = 0;
        return;
    }

    map.length = m->length;
    for(int i=0; i < map.length; i++)
    {
        map.map[i] = m->map[i];
    }
}

inline int sqr(int x)
{
    return x * x;
}

void image_t::setPixel(const int x, const int y, const BGR color)
{
    int minDist;
    int dist;
    int index;
    int i;

    if ((x < 0) || (x >= width) || (y < 0) || (y >= height))
    {
        return;
    }

    switch (type)
    {
        case T_BGR:
            ((BGR *)data)[y * width + x] = color;
            break;

        case T_BGRA:
            ((BGRA *)data)[y * width + x] = BGRA(color.blue, color.green, color.red, 255);
            break;

        case T_INDEX_MONO:
            ((unsigned char *)data)[y * width + x] = (unsigned char)((color.red + color.green + color.blue) / 3);
            break;

        case T_INDEX_BGR:
            minDist = 200000;

            // Ищем ближайший подходящий цвет в палитре. Просто и точно, но медленно
            for(i = 0; i < map.length; i++)
            {
                dist = sqr(map.map[i].blue - color.blue) +
                       sqr(map.map[i].green - color.green) +
                       sqr(map.map[i].red - color.red);

                if (dist < minDist)
                {
                    minDist = dist;
                    index = i;
                }
            }

            ((unsigned char *)data)[y * width + x] = (unsigned char)index;
            break;

        default:
            break;
    }
}

BGR image_t::getPixel(const int x, const int y)
{
    unsigned char index;
    BGRA tColor;

    if ((x < 0) || (x >= width) || (y < 0) || (y >= height))
    {
        return BGR(0, 0, 0);
    }

    switch (type)
    {
        case T_BGR:
            return ((BGR *)data)[y * width + x];
            break;

        case T_BGRA:
            tColor = ((BGRA *)data)[y * width + x];
            return BGR(tColor.blue * tColor.alpha / 255,
                       tColor.green * tColor.alpha / 255,
                       tColor.red * tColor.alpha / 255);

        case T_INDEX_MONO:
            index = ((unsigned char *)data)[y * width + x];
            return BGR(index, index, index);

        case T_INDEX_BGR:
            return map.map[((unsigned char *)data)[y * width + x]];
            break;

        default:
            break;
    }

    return BGR(0,0,0);
}

void image_t::setPixelA(const int x, const int y, const BGRA color)
{
    int minDist;
    int dist;
    int index;
    int i;

    if ((x < 0) || (x >= width) || (y < 0) || (y >= height))
    {
        return;
    }

    BGR tColor;
    switch (type)
    {
        case T_BGR:
            tColor.blue = color.blue * color.alpha / 255;
            tColor.green = color.green * color.alpha / 255;
            tColor.red = color.red * color.alpha / 255;
            ((BGR *)data)[y * width + x] = tColor;
            break;

        case T_BGRA:
            ((BGRA *)data)[y * width + x] = color;
            break;

        case T_INDEX_MONO:
            ((unsigned char *)data)[y * width + x] = (unsigned char)((color.red + color.green + color.blue) * color.alpha / 765); // 765 == 255 * 3
            break;

        case T_INDEX_BGR:
            tColor.blue = color.blue * color.alpha / 255;
            tColor.green = color.green * color.alpha / 255;
            tColor.red = color.red * color.alpha / 255;

            minDist = 200000;

            // Ищем ближайший подходящий цвет в палитре. Просто и точно, но медленно
            for(i = 0; i < map.length; i++)
            {
                dist = sqr(map.map[i].blue - tColor.blue) +
                       sqr(map.map[i].green - tColor.green) +
                       sqr(map.map[i].red - tColor.red);

                if (dist < minDist)
                {
                    minDist = dist;
                    index = i;
                }
            }

            ((unsigned char *)data)[y * width + x] = (unsigned char)index;
            break;

        default:
            break;
    }
}

BGRA image_t::getPixelA(const int x, const int y)
{
    unsigned char index;
    BGR tColor;

    if ((x < 0) || (x >= width) || (y < 0) || (y >= height))
    {
        return BGRA(0, 0, 0, 0);
    }

    switch (type)
    {
        case T_BGR:
            tColor = ((BGR *)data)[y * width + x];
            return BGRA(tColor.blue, tColor.green, tColor.red, 255);

        case T_BGRA:
            return ((BGRA *)data)[y * width + x];

        case T_INDEX_MONO:
            index = ((unsigned char *)data)[y * width + x];
            return BGRA(index, index, index, 255);

        case T_INDEX_BGR:
            tColor = map.map[((unsigned char *)data)[y * width + x]];
            return BGRA(tColor.blue, tColor.green, tColor.red, 255);

        default:
            break;
    }

    return BGRA(0, 0, 0, 0);
}

// Защищённые методы
int image_t::readIndexUpDn(data_t *dat, const unsigned int lineSize)
{
    if ((type != T_INDEX_MONO) && (type != T_INDEX_BGR))
    {
        return -1;
    }

    if (lineSize < width * sizeof(unsigned char))
    {
        return -1;
    }

    int dstOffset = 0;
    int srcOffset = dat->tellData();
    for(int y = 0; y < height; y++)
    {
        dat->seekData(srcOffset);
        dat->readData(&((unsigned char *)data)[dstOffset], width);
        dstOffset += width;
        srcOffset += lineSize;
    }

    return 0;
}

int image_t::readIndexDnUp(data_t *dat, const unsigned int lineSize)
{
    if ((type != T_INDEX_MONO) && (type != T_INDEX_BGR))
    {
        return -1;
    }

    if (lineSize < width * sizeof(unsigned char))
    {
        return -1;
    }

    int dstOffset = width * height;
    int srcOffset = dat->tellData();
    for(int y = 0; y < height; y++)
    {
        dstOffset -= width;
        dat->seekData(srcOffset);
        dat->readData(&((unsigned char *)data)[dstOffset], width);
        srcOffset += lineSize;
    }

    return 0;
}

int image_t::readBGRUpDn(data_t *dat, const unsigned int lineSize)
{
    if (type != T_BGR)
    {
        return -1;
    }

    if (lineSize < width * sizeof(BGR))
    {
        return -1;
    }

    int dstOffset = 0;
    int srcOffset = dat->tellData();
    for(int y = 0; y < height; y++)
    {
        dat->seekData(srcOffset);
        dat->readData(&((BGR *)data)[dstOffset], width * sizeof(BGR));
        dstOffset += width;
        srcOffset += lineSize;
    }

    return 0;
}

int image_t::readBGRDnUp(data_t *dat, const unsigned int lineSize)
{
    if (type != T_BGR)
    {
        return -1;
    }

    if (lineSize < width * sizeof(BGR))
    {
        return -1;
    }

    int dstOffset = width * height;
    int srcOffset = dat->tellData();
    for(int y = 0; y < height; y++)
    {
        dstOffset -= width;
        dat->seekData(srcOffset);
        dat->readData(&((BGR *)data)[dstOffset], width * sizeof(BGR));
        srcOffset += lineSize;
    }

    return 0;
}

int image_t::readBGRAUpDn(data_t *dat, const unsigned int lineSize)
{
    if (type != T_BGRA)
    {
        return -1;
    }

    if (lineSize < width * sizeof(BGRA))
    {
        return -1;
    }

    int dstOffset = 0;
    int srcOffset = dat->tellData();
    for(int y = 0; y < height; y++)
    {
        dat->seekData(srcOffset);
        dat->readData(&((BGRA *)data)[dstOffset], width * sizeof(BGRA));
        dstOffset += width;
        srcOffset += lineSize;
    }

    return 0;
}

int image_t::readBGRADnUp(data_t *dat, const unsigned int lineSize)
{
    if (type != T_BGRA)
    {
        return -1;
    }

    if (lineSize < width * sizeof(BGRA))
    {
        return -1;
    }

    int dstOffset = width * height;
    int srcOffset = dat->tellData();
    for(int y = 0; y < height; y++)
    {
        dstOffset -= width;
        dat->seekData(srcOffset);
        dat->readData(&((BGRA *)data)[dstOffset], width * sizeof(BGRA));
        srcOffset += lineSize;
    }

    return 0;
}

int image_t::readBGRFUpDn(data_t *dat, const unsigned int lineSize)
{
    if (type != T_BGR)
    {
        return -1;
    }

    if (lineSize < width * sizeof(BGRA))
    {
        return -1;
    }

    int dstOffset = 0;
    int srcOffset = dat->tellData();
    for(int y = 0; y < height; y++)
    {
        dat->seekData(srcOffset);
        for(int x = 0; x < width; x++)
        {
            BGR color;
            color.blue = dat->readChar();
            color.green = dat->readChar();
            color.red = dat->readChar();
            dat->readChar();

            ((BGR *)data)[dstOffset + x] = color;
        }
        dstOffset += width;
        srcOffset += lineSize;
    }

    return 0;
}

int image_t::readBGRFDnUp(data_t *dat, const unsigned int lineSize)
{
    if (type != T_BGR)
    {
        return -1;
    }

    if (lineSize < width * sizeof(BGRA))
    {
        return -1;
    }

    int dstOffset = width*height;
    int srcOffset = dat->tellData();
    for(int y = 0; y < height; y++)
    {
        dstOffset -= width;
        dat->seekData(srcOffset);
        for(int x = 0; x < width; x++)
        {
            BGR color;
            color.blue = dat->readChar();
            color.green = dat->readChar();
            color.red = dat->readChar();
            dat->readChar();

            ((BGR *)data)[dstOffset + x] = color;
                
        }
        srcOffset += lineSize;
    }
    return 0;
}

int image_t::writeIndexDnUp(data_t *dat, const unsigned int lineSize)
{
    if ((type != T_INDEX_MONO) && (type != T_INDEX_BGR))
    {
        return -1;
    }

    if (lineSize < width * sizeof(unsigned char))
    {
        return -1;
    }

    int srcOffset = width * height;
    int dstOffset = dat->tellData();
    for(int y = 0; y < height; y++)
    {
        srcOffset -= width;
        dat->seekData(dstOffset);
        dat->writeData(&((unsigned char *)data)[srcOffset], width);
        dstOffset += lineSize;
    }

    return 0;
}

int image_t::writeBGRDnUp(data_t *dat, const unsigned int lineSize)
{
    if (type != T_BGR)
    {
        return -1;
    }

    if (lineSize < width * sizeof(BGR))
    {
        return -1;
    }

    int srcOffset = width * height;
    int dstOffset = dat->tellData();
    for(int y = 0; y < height; y++)
    {
        srcOffset -= width;
        dat->seekData(dstOffset);
        dat->writeData(&((BGR *)data)[srcOffset], width * sizeof(BGR));
        dstOffset += lineSize;
    }

    return 0;
}

int image_t::setImage(image_t *image, const imageType_t t)
{
    if (type != T_NULL)
    {
        free(data);
        type = T_NULL;
    }

    if ((image->type == T_NULL) || (t == T_NULL))
    {
        return -1;
    }

    if (setWidthHeight(image->width, image->height, t) != 0)
    {
        return -1;
    }

    if ((image->type == T_BGRA) && (t == T_BGRA))
    {
        for(int y = 0; y < height; y++)
        {
            for(int x = 0; x < width; x++)
            {
                setPixelA(x, y, image->getPixelA(x, y));
            }
        }
    }
    else
    { 
        for(int y = 0; y < height; y++)
        {
            for(int x = 0; x < width; x++)
            {
                setPixel(x, y, image->getPixel(x, y));
            }
        }
    }

    return 0;
}

void image_t::setQuakeMap(void)
{
    map.length = 256;
    memcpy(map.map, quakeMapData, map.length * sizeof(BGR));
}

void image_t::setQuake2Map(void)
{
    map.length = 256;
    memcpy(map.map, quake2MapData, map.length * sizeof(BGR));
}

int image_t::buildMipImage(image_t *image, const imageType_t t)
{
    if (type != T_NULL)
    {
        free(data);
        type = T_NULL;
    }

    if ((image->type == T_NULL) || (t == T_NULL))
    {
        return -1;
    }

    if ((image->width % 2 != 0) || (image->height % 2 != 0))
    {
        return -1;
    }

    if (setWidthHeight(image->width / 2, image->height / 2, t) != 0)
    {
        return -1;
    }

    if ((image->type == T_BGRA) && (t == T_BGRA))
    {
        for(int y = 0; y < height; y++)
        {
               for(int x = 0; x < width; x++)
               {
                   BGRA c00 = image->getPixelA(x + x, y + y);
                   BGRA c10 = image->getPixelA(x + x + 1, y + y);
                   BGRA c01 = image->getPixelA(x + x, y + y + 1);
                   BGRA c11 = image->getPixelA(x + x + 1, y + y + 1);

                   BGRA c;
                   c.blue =(unsigned char)((c00.blue + c10.blue + c01.blue + c11.blue) / 4);
                   c.green =(unsigned char)((c00.green + c10.green + c01.green + c11.green) / 4);
                   c.red =(unsigned char)((c00.red + c10.red + c01.red + c11.red) / 4);
                   c.alpha =(unsigned char)((c00.alpha + c10.alpha + c01.alpha + c11.alpha) / 4);

                   setPixelA(x, y, c);
               }
        }
    }
    else
    {
        for(int y = 0; y < height; y++)
        {
               for(int x = 0; x < width; x++)
               {
                   BGR c00 = image->getPixel(x + x, y + y);
                   BGR c10 = image->getPixel(x + x + 1, y + y);
                   BGR c01 = image->getPixel(x + x, y + y + 1);
                   BGR c11 = image->getPixel(x + x + 1, y + y + 1);

                   BGR c;
                   c.blue = (unsigned char)((c00.blue + c10.blue + c01.blue + c11.blue) / 4);
                   c.green = (unsigned char)((c00.green + c10.green + c01.green + c11.green) / 4);
                   c.red = (unsigned char)((c00.red + c10.red + c01.red + c11.red) / 4);

                   setPixel(x, y, c);
               }
        }
    }

    return 0;
}

// colorStat_t
colorStat_t::colorStat_t(void)
{
    stat = NULL;
}

colorStat_t::~colorStat_t(void)
{
    if (stat != NULL)
    {
        free(stat);
    }
}

int colorStat_t::addImageColors(image_t *image)
{
    if (stat == NULL)
    {
        if ((stat = (int *)calloc(32768, sizeof(int))) == NULL)
        {
            fprintf(stderr, "addImageColor: failed to allocate %ld bytes of memory\n", 32768 * sizeof(int));
            return -1;
        }
    }

    for(int y = 0; y < image->getHeight(); y++)
    {
        for(int x = 0; x < image->getWidth(); x++)
        {
            BGR color = image->getPixel(x, y);
            int sColor = ((color.blue << 7) & 0x7C00) |
                         ((color.green << 2) & 0x03E0) |
                         (color.red >> 3);
            stat[sColor]++;
        }
    }

    return 0;
}

int colorStat_t::getMap(map_t *map)
{
    if (stat == NULL)
    {
        return -1;
    }

    for(map->length = 0; map->length < 256; )
    {
        int maxValue = 0;
        int sColor = 0;

        for(int j = 0; j < 32768; j++)
        {
            if (stat[j] > maxValue)
            {
                maxValue = stat[j];
                sColor = j;
            }
        }

        if (maxValue == 0)
        {
            break;
        }

        stat[sColor] = 0;

        map->map[map->length].blue = (sColor & 0x7C00) >> 7;
        map->map[map->length].green = (sColor & 0x03E0) >> 2;
        map->map[map->length].red = (sColor & 0x001F) << 3;
        map->length++;
    }

    memset(stat, 0, 32768 * sizeof(int));
    return 0;
}

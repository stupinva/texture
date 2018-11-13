#ifndef __IMAGE__
#define __IMAGE__

#include "data.h"

typedef enum lm_mode_e
{
    LM_MODE_QUAKE,
    LM_MODE_HALFLIFE
} lm_mode_t;

#ifndef __BGR__
#define __BGR__

#pragma pack(push, 1)
struct BGR
{
    unsigned char blue;
    unsigned char green;
    unsigned char red;

    BGR(void)
    {
    };

    BGR(const unsigned char b, const unsigned char g, const unsigned char r)
    {
        blue = b;
        green = g;
        red = r;
    };
};
#pragma pack(pop)

#endif

#ifndef __BGRA__
#define __BGRA__

#pragma pack(push, 1)
struct BGRA
{
    unsigned char blue;
    unsigned char green;
    unsigned char red;
    unsigned char alpha;

    BGRA(void)
    {
    };

    BGRA(const unsigned char b, const unsigned char g, const unsigned char r, const unsigned char a)
    {
        blue = b;
        green = g;
        red = r;
        alpha = a;
    };
};
#pragma pack(pop)

#endif

// Класс для работы с палитрой
struct map_t
{
    int length;
    BGR map[256];

    map_t(void)
    {
        length = 0;
    };
};

enum imageType_t
{
    T_NULL,
    T_INDEX_MONO,
    T_INDEX_BGR,
    T_BGR,
    T_BGRA
};

// Класс для работы с полноцветными и палитровыми изображениями
class image_t
{
    // Защищённые переменные
    imageType_t type;
    int width;
    int height;
    void *data;
    map_t map;

    // Защищённые методы
    int readIndexUpDn(data_t *dat, const unsigned int lineSize);
    int readIndexDnUp(data_t *dat, const unsigned int lineSize);
    //int readRGB5UpDn(data_t *dat, const unsigned int lineSize);
    //int readRGB5DnUp(data_t *dat, const unsigned int lineSize);
    int readBGRUpDn(data_t *dat, const unsigned int lineSize);
    int readBGRDnUp(data_t *dat, const unsigned int lineSize);
    int readBGRAUpDn(data_t *dat, const unsigned int lineSize);
    int readBGRADnUp(data_t *dat, const unsigned int lineSize);
    int readBGRFUpDn(data_t *dat, const unsigned int lineSize);
    int readBGRFDnUp(data_t *dat, const unsigned int lineSize);

    int writeIndexDnUp(data_t *dat, const unsigned int lineSize);
    int writeBGRDnUp(data_t *dat, const unsigned int lineSize);

public:
    // Публичные переменные
    int xOffset;
    int yOffset;
    char *text;

    // Публичные методы
    image_t(void);
    ~image_t(void);

    int getWidth(void);
    int getHeight(void);
    int setWidthHeight(const int w, const int h, const imageType_t t);
    void setMap(const map_t *m);
    void setQuakeMap(void);
    void setQuake2Map(void);
    void setPixel(const int x, const int y, const BGR color);
    BGR getPixel(const int x, const int y);
    void setPixelA(const int x, const int y, const BGRA color);
    BGRA getPixelA(const int x, const int y);
    int setImage(image_t *image, const imageType_t t);
    int buildMipImage(image_t *image, const imageType_t t);

    int readLMP(data_t *dat);
    int writeLMP(data_t *dat, const int lmMode);
    int readM8(data_t *dat);
    int writeM8(data_t *dat);
    int readM32(data_t *dat);
    int writeM32(data_t *dat);
    int readMIP(data_t *dat);
    int writeMIP(data_t *dat, const int lmMode);
    int readWAL(data_t *dat);
    int writeWAL(data_t *dat);
    int writeTGA(data_t *dat);
    int readTGA(data_t *dat, const int alpha);
    int writeBMP(data_t *dat);
    int readBMP(data_t *dat);
};

// Класс со статистикой использования цветов для генерации палитры
class colorStat_t
{
    int *stat;

public:
    colorStat_t(void);
    ~colorStat_t(void);

    int addImageColors(image_t *image);
    int getMap(map_t *map);
};

#endif

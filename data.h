#ifndef __DATA__
#define __DATA__

class data_t
{
    unsigned int offset;
    unsigned int size;
    unsigned char *data;
    friend class wad2_t;

public:
    data_t(void);
    ~data_t(void);

    // Методы для работы с памятью
    int allocData(const unsigned int s);
    void freeData(void);

    // Методы для чтения и записи
    unsigned int sizeData(void);
    unsigned int tellData(void);
    void seekData(const unsigned int o);
    unsigned int readData(void *p, const unsigned int s);
    unsigned int writeData(const void *p, const unsigned int s);
    unsigned char readChar(void);
    void writeChar(const unsigned char c);

    // Методы для загрузки и сохранения файла
    int loadData(const char *fileName);
    int saveData(const char *fileName);
};

#endif

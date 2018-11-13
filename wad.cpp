#include <stdlib.h>
#include <string.h>
#include "str.h"
#include "wad.h"

wad2_t::wad2_t(void)
{
    number = 0;
    dir = NULL;
    file = NULL;
}

wad2_t::~wad2_t(void)
{
    if (number!=0)
    {
        free(dir);
        fclose(file);
    }
}

int wad2_t::getNumber(void)
{
    return number;
}

int wad2_t::getType(const int i)
{
    if ((number == 0) || (i > number))
    {
        return 0;
    }

    return dir[i].type;
}

int wad2_t::openWAD(const char *fileName)
{
    if (number != 0)
    {
        free(dir);
        fclose(file);
        number = 0;
    }

    // Открываем файл для чтения
    if ((file = fopen(fileName, "rb")) == NULL)
    {
        fprintf(stderr, "openWAD: failed to open file \"%s\".\n", fileName);
        return -1;
    }

    // Считываем заголовок
    wad2Header_t header;
    if (fread(&header, sizeof(header), 1, file) != 1)
    {
        fprintf(stderr, "openWAD: failed to read header.\n");
        fclose(file);
        return -1;
    }

    // Проверяем идентификатор
    if ((strncmp(header.ident, "WAD2", WAD_IDENT_LENGTH) != 0) && (strncmp(header.ident, "WAD3", WAD_IDENT_LENGTH) != 0))
    {
        fprintf(stderr, "openWAD: invalid WAD-file \"%s\".\n", fileName);
        fclose(file);
        return -1;
    }

    // Считываем каталог
    dir = (wad2Entry_t *)malloc(header.number * sizeof(wad2Entry_t));
    if (dir == NULL)
    {
        fprintf(stderr, "openWAD: failed to allocate %ld bytes of memory.\n", header.number * sizeof(wad2Entry_t));
        fclose(file);
        return -1;
    }

    number = header.number;
    fseek(file, header.offset, SEEK_SET);
    if (fread(dir, number * sizeof(wad2Entry_t), 1, file) != 1)
    {
        fprintf(stderr, "openWAD: failed to read dir.\n");
        free(dir);
        fclose(file);
        return -1;
    }

    // Выводим информацию
    fprintf(stdout, "openWAD: %i files in %s\n", number, fileName);
    return 0;
}

int wad2_t::loadData(const int i, data_t *dat)
{
    if ((number == 0) || (i > number))
    {
        return -1;
    }

    // Выделяем память под ресурс и считываем его
    if (dat->allocData(dir[i].size) != 0)
    {
        return -1;
    }
    fseek(file, dir[i].offset, SEEK_SET);
    if (fread(dat->data, dat->size, 1, file) != 1)
    {
        fprintf(stderr, "loadData: failed to read file\n");
        dat->freeData();
        return -1;
    }

    // Заполняем информацию
    char name[WAD_NAME_LENGTH + 1];
    strncpy(name, dir[i].name, WAD_NAME_LENGTH);
    name[WAD_NAME_LENGTH] = 0;
 
    // Выводим информацию
    fprintf(stdout, "loadData: %i %s\n", dat->size, name);
    return 0;
}

int wad2_t::getName(const int i, char **name)
{
    if ((number == 0) || (i > number))
    {
        return -1;
    }

    s_strncpy(name, dir[i].name, WAD_NAME_LENGTH);
    return 0;
}

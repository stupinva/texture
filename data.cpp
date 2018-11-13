#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "data.h"

data_t::data_t(void)
{
    data = NULL;
}

data_t::~data_t(void)
{
    if (data != NULL)
    {
        free(data);
    }
}

int data_t::allocData(const unsigned int s)
{
    if (data != NULL)
    {
        free(data);
    }

    if (s == 0)
    {
        data = NULL;
        return 0;
    }

    data = (unsigned char *)malloc(s);
    if (data == NULL)
    {
        fprintf(stderr, "allocData: failed to allocate %d bytes of memory\n", s);
        return -1;
    }

    offset = 0;
    size = s;
    return 0;
}

void data_t::freeData(void)
{
    if (data != NULL)
    {
        free(data);
    }

    data = NULL;
}

int data_t::loadData(const char *fileName)
{
    FILE *file;
    if ((file = fopen(fileName, "rb")) == NULL)
    {
        fprintf(stderr, "loadData: failed to open file \"%s\".\n", fileName);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    unsigned int fileSize = ftell(file);
    if (allocData(fileSize) != 0)
    {
        fprintf(stderr, "loadData: failed to allocate %i bytes of memory for file \"%s\".\n", fileSize, fileName);
        fclose(file);
        return -1;
    }

    fseek(file, 0, SEEK_SET);
    if (fread(data, size, 1, file) != 1)
    {
        fprintf(stderr, "loadData: failed to read %i bytes from file \"%s\".\n", size, fileName);
        fclose(file);
        return -1;
    }
    fclose(file);

    fprintf(stdout, "loadData: %i %s\n", size, fileName);
    return 0;
}

int data_t::saveData(const char *fileName)
{
    if (data == NULL)
    {
        return 0;
    }

    FILE *file;
    if ((file = fopen(fileName, "wb")) == NULL)
    {
        fprintf(stderr, "saveData: failed to create file \"%s\".\n", fileName);
        return -1;
    }

    if (fwrite(data, size, 1, file) != 1)
    {
        fprintf(stderr, "saveData: failed to write %i bytes to file \"%s\".\n", size, fileName);
        fclose(file);
        return -1;
    }
    fclose(file);

    fprintf(stdout, "saveData: %i %s\n", size, fileName);
    return 0;
}

void data_t::seekData(const unsigned int o)
{
    if (data == NULL)
    {
        return;
    }

    if (o < size)
    {
        offset = o;
    }
    else
    {
        offset = size;
    }
}

unsigned int data_t::tellData(void)
{
    if (data == NULL)
    {
        return 0;
    }
    return offset;
}

unsigned int data_t::readData(void *p, const unsigned int s)
{
    if (data == NULL)
    {
        return 0;
    }

    if (offset + s > size)
    {
        unsigned int ts = size - offset;
        memcpy(p, &data[offset], ts);
        offset += ts;
        return ts;
    }
    memcpy(p, &data[offset], s);
    offset += s;
    return s;
}

unsigned int data_t::writeData(const void *p, const unsigned int s)
{
    if (data == NULL)
    {
        return 0;
    }

    if (offset + s > size)
    {
        unsigned int ts = size - offset;
        memcpy(&data[offset], p, ts);
        offset += ts;
        return ts;
    }
    memcpy(&data[offset], p, s);
    offset += s;
    return s;
}

unsigned int data_t::sizeData(void)
{
    if (data == NULL)
    {
        return 0;
    }

    return size;
}

unsigned char data_t::readChar(void)
{
    if ((data == NULL) || (offset + 1 > size))
    {
        return 0;
    }

    return data[offset++];
}

void data_t::writeChar(const unsigned char c)
{
    if ((data == NULL) || (offset + 1 > size))
    {
        return;
    }

    data[offset++] = c;
}

#include "header.h"

ppm *readImage(FILE *image)
{
    ppm *img = calloc(1, sizeof(ppm));
    char random[255];
    fscanf(image, "%s\n", random);
    fscanf(image, "%u %u\n", &img->size, &img->size);
    fscanf(image, "%s", random);
    fread(random, sizeof(char), 1, image);

    img->pixels = calloc(img->size, sizeof(pxl *));
    for (unsigned int i = 0; i < img->size; i++)
        img->pixels[i] = calloc(img->size, sizeof(pxl));

    for (unsigned int i = 0; i < img->size; i++)
        for (unsigned int j = 0; j < img->size; j++)
        {
            fread(&img->pixels[i][j].channels[0], sizeof(unsigned char), 1, image);
            fread(&img->pixels[i][j].channels[1], sizeof(unsigned char), 1, image);
            fread(&img->pixels[i][j].channels[2], sizeof(unsigned char), 1, image);
        }

    return img;
}

void freeImage(ppm *img)
{
    for (unsigned int i = 0; i < img->size; i++)
        free(img->pixels[i]);

    free(img->pixels);
    free(img);
}

void printImage(ppm *img)
{
    for (unsigned int i = 0; i < img->size; i++)
        for (unsigned int j = 0; j < img->size; j++)
            printf("[%u][%u] -- R: %d, G: %d, B: %d\n", i, j, img->pixels[i][j].channels[0], img->pixels[i][j].channels[1], img->pixels[i][j].channels[2]);
}

unsigned long long *computeChannelMeans(ppm *img, int x, int y, unsigned int size)
{
    unsigned long long *channelMeans = calloc(3, sizeof(unsigned long long));
    for (int channel = 0; channel < 3; channel++)
    {
        for (unsigned int i = x; i < x + size; i++)
            for (unsigned int j = y; j < y + size; j++)
                channelMeans[channel] += (unsigned long long)img->pixels[i][j].channels[channel];

        channelMeans[channel] /= size * size;
    }

    return channelMeans;
}

unsigned long long computeMean(ppm *img, int x, int y, unsigned int size, pxl *pixel)
{
    unsigned long long *channelMeans = computeChannelMeans(img, x, y, size);
    unsigned long long mean = 0;

    for (unsigned int i = x; i < x + size; i++)
        for (unsigned int j = y; j < y + size; j++)
            for (int channel = 0; channel < 3; channel++){
                pixel->channels[channel] = channelMeans[channel];
                mean += ((unsigned long long)(channelMeans[channel] - img->pixels[i][j].channels[channel])) *
                        ((unsigned long long)(channelMeans[channel] - img->pixels[i][j].channels[channel]));
            }
    mean = mean / (3 * size * size);
    free(channelMeans);
    return mean;
}

int compressRegion(ppm *img, int x, int y, unsigned int size, int similarityFactor, pxl *pixel)
{
    unsigned long long mean = computeMean(img, x, y, size, pixel);
    return mean > similarityFactor;
}

void emptyPixel(pxl *pixel)
{
    pixel->channels[0] = 0;
    pixel->channels[1] = 0;
    pixel->channels[2] = 0;
}

decompressPixel **readPixels(FILE *compressed, int **pixelsPerLevel, int *levels)
{
    queue *pixelsQ = initQueue();
    unsigned char holder = 0;
    *pixelsPerLevel = calloc(*levels + 2, sizeof(int));
    *pixelsPerLevel[0] = 1;
    *levels = 0;
    int countedOnCurrentLevel = 0;
    while (!feof(compressed))
    {
        if ((*pixelsPerLevel)[*levels] == 0)
            break;
        
        fread(&holder, sizeof(unsigned char), 1, compressed);
        if (holder != 1 && holder != 0)
            continue;

        pxl pixel;
        TEE_TYPE type = DEAD;
        if (holder == 1)
        {
            type = ALIVE;
            fread(&pixel.channels[0], sizeof(unsigned char), 1, compressed);
            fread(&pixel.channels[1], sizeof(unsigned char), 1, compressed);
            fread(&pixel.channels[2], sizeof(unsigned char), 1, compressed);
        }
        if (holder == 0)
        {
            emptyPixel(&pixel);
            type = DEAD;
            (*pixelsPerLevel)[*levels + 1] += 4;
        }

        qtee leaf = makeLeaf(type, pixel);
        addToQueue(pixelsQ, leaf, *levels);

        countedOnCurrentLevel += 1;
        if (countedOnCurrentLevel >= (*pixelsPerLevel)[*levels]){
            *levels += 1;
            countedOnCurrentLevel = 0;
            *pixelsPerLevel = realloc(*pixelsPerLevel, (*levels + 2) * sizeof(int));
            (*pixelsPerLevel)[*levels + 1] = 0;
        }
    }

    decompressPixel **pixels = calloc(*levels + 1, sizeof(decompressPixel*));
    for (int lvl = 0; lvl < *levels; lvl++)
        pixels[lvl] = calloc((*pixelsPerLevel)[lvl], sizeof(decompressPixel));
    
    int *counts = calloc(*levels, sizeof(int));
    while (!isQueueEmpty(pixelsQ)){
        node _node = removeFromQueue(pixelsQ);
        pixels[_node->depth][counts[_node->depth]].pixel = _node->tee->pixel;
        pixels[_node->depth][counts[_node->depth]].type = _node->tee->type;
        counts[_node->depth] += 1;
        if (_node->tee)
            free(_node->tee);
        free(_node);
    }

    clearQueue(pixelsQ);
    free(counts);
    return pixels;
}

void populateImage(qtee tee, ppm *img, int x, int y, unsigned int size)
{
    if (tee->type == ALIVE)
    {
        for (int i = x; i < x + size; i++)
            for (int j = y; j < y + size; j++)
                img->pixels[i][j] = tee->pixel;
        return;
    }

    populateImage(tee->summer, img, x, y, size / 2);
    populateImage(tee->autumn, img, x, y + size / 2, size / 2);
    populateImage(tee->winter, img, x + size / 2, y + size / 2, size / 2);
    populateImage(tee->spring, img, x + size / 2, y, size / 2);
}

ppm *reconstructImage(qtee tee, unsigned int size)
{
    ppm *img = calloc(1, sizeof(ppm));
    img->size = size;
    img->pixels = calloc(img->size, sizeof(pxl *));
    for (unsigned int i = 0; i < img->size; i++)
        img->pixels[i] = calloc(img->size, sizeof(pxl));

    populateImage(tee, img, 0, 0, size);
    return img;
}

void reconstructPPM(ppm *img, FILE *ppmToWrite)
{
    fprintf(ppmToWrite, "P6\n%u %u\n255\n", img->size, img->size);
    for (int i = 0; i < img->size; i++)
        for (int j = 0; j < img->size; j++)
            for (int channel = 0; channel < 3; channel++)
                fwrite(&img->pixels[i][j].channels[channel], sizeof(unsigned char), 1, ppmToWrite);
}

void decompress(FILE *compressed, FILE *ppmToWrite)
{
    unsigned int size;
    fread(&size, sizeof(unsigned int), 1, compressed);
    int *pixelsPerLevel = NULL, levels = 0;
    decompressPixel **pixels = readPixels(compressed, &pixelsPerLevel, &levels);
    qtee tee = reconstructTee(pixels, &pixelsPerLevel, 0);

    free(pixelsPerLevel);
    for (int lvl = 0; lvl < levels; lvl++)
        if (pixels[lvl])
            free(pixels[lvl]);
    free(pixels);

    ppm *img = reconstructImage(tee, size);
    burnTee(tee);

    reconstructPPM(img, ppmToWrite);
    freeImage(img);
}
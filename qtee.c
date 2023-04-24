#include "header.h"

qtee makeLeaf(TEE_TYPE type, pxl pixel)
{
    qtee leaf = calloc(1, sizeof(Qtee));
    if (!leaf)
        return NULL;

    leaf->pixel = pixel;
    leaf->type = type;
    return leaf;
}

void burnLeaf(qtee leaf)
{
    free(leaf);
}

void burnTee(qtee tee)
{
    if (!tee)
        return;

    burnTee(tee->summer);
    burnTee(tee->autumn);
    burnTee(tee->winter);
    burnTee(tee->spring);

    burnLeaf(tee);
}

qtee makeTee(ppm *img, int x, int y, unsigned int size, int similarityFactor)
{
    pxl pixel;
    int compress = compressRegion(img, x, y, size, similarityFactor, &pixel);
    if (!compress)
        return makeLeaf(ALIVE, pixel);

    pxl nullPixel;
    emptyPixel(&nullPixel);
    qtee tee = makeLeaf(DEAD, nullPixel);

    tee->summer = makeTee(img, x, y, size / 2, similarityFactor);
    tee->autumn = makeTee(img, x, y + size / 2, size / 2, similarityFactor);
    tee->winter = makeTee(img, x + size / 2, y + size / 2, size / 2, similarityFactor);
    tee->spring = makeTee(img, x + size / 2, y, size / 2, similarityFactor);

    return tee;
}

void printTee(qtee tee, int lvl)
{
    if (!tee)
        return;

    printf("lvl: %d, type: %d, R: %d, G: %d, B: %d\n", lvl, tee->type, tee->pixel.channels[0], tee->pixel.channels[1], tee->pixel.channels[2]);
    printTee(tee->summer, lvl + 1);
    printTee(tee->autumn, lvl + 1);
    printTee(tee->winter, lvl + 1);
    printTee(tee->spring, lvl + 1);
}

void parseTee(qtee tee, int crtLvl, int *lvl, int *leafs, int *lowestLeaf)
{
    if (!tee)
        return;

    if (!tee->summer && !tee->autumn && !tee->winter && !tee->spring)
    {
        *leafs += 1;
        if (*lowestLeaf == -1)
            *lowestLeaf = crtLvl;
        else if (crtLvl < *lowestLeaf)
            *lowestLeaf = crtLvl;

        if (crtLvl > *lvl)
            *lvl = crtLvl;
    }

    parseTee(tee->summer, crtLvl + 1, lvl, leafs, lowestLeaf);
    parseTee(tee->autumn, crtLvl + 1, lvl, leafs, lowestLeaf);
    parseTee(tee->winter, crtLvl + 1, lvl, leafs, lowestLeaf);
    parseTee(tee->spring, crtLvl + 1, lvl, leafs, lowestLeaf);
}

void makeStats(qtee tee, unsigned int size, FILE *output)
{
    int lvls = 0, leafs = 0, lowestLeaf = -1;
    parseTee(tee, 0, &lvls, &leafs, &lowestLeaf);
    fprintf(output, "%d\n", lvls + 1);
    fprintf(output, "%d\n", leafs);

    if (lowestLeaf == -1)
        fprintf(output, "%d\n", size);
    else
    {
        while (lowestLeaf)
        {
            size /= 2;
            lowestLeaf -= 1;
        }
        fprintf(output, "%d\n", size);
    }
}

void printLeaf(qtee leaf, FILE *compressed)
{
    char type = 0;
    if (leaf->type == DEAD)
    {
        fwrite(&type, sizeof(char), 1, compressed);
        return;
    }

    type = 1;
    fwrite(&type, sizeof(char), 1, compressed);
    fwrite(&leaf->pixel.channels[0], sizeof(unsigned char), 1, compressed);
    fwrite(&leaf->pixel.channels[1], sizeof(unsigned char), 1, compressed);
    fwrite(&leaf->pixel.channels[2], sizeof(unsigned char), 1, compressed);
}

void parseSideways(qtee tee, unsigned int *size, FILE *compressedPPM)
{
    queue *parseQ = initQueue();
    addToQueue(parseQ, tee, 0);
    fwrite(size, sizeof(unsigned int), 1, compressedPPM);

    while (!isQueueEmpty(parseQ))
    {
        node _node = removeFromQueue(parseQ);
        if (_node->tee->summer)
            addToQueue(parseQ, _node->tee->summer, _node->depth + 1);
        if (_node->tee->autumn)
            addToQueue(parseQ, _node->tee->autumn, _node->depth + 1);
        if (_node->tee->winter)
            addToQueue(parseQ, _node->tee->winter, _node->depth + 1);
        if (_node->tee->spring)
            addToQueue(parseQ, _node->tee->spring, _node->depth + 1);

        printLeaf(_node->tee, compressedPPM);
        free(_node);
    }

    clearQueue(parseQ);
}

qtee reconstructTee(decompressPixel **pixels, int **pixelsPerLevel, int level)
{
    if (pixels[level][(*pixelsPerLevel)[level] - 1].type == ALIVE)
    {
        (*pixelsPerLevel)[level] -= 1;
        return makeLeaf(ALIVE, pixels[level][(*pixelsPerLevel)[level]].pixel);
    }
    qtee tee = makeLeaf(DEAD, pixels[level][(*pixelsPerLevel)[level] - 1].pixel);
    tee->spring = reconstructTee(pixels, pixelsPerLevel, level + 1);
    tee->winter = reconstructTee(pixels, pixelsPerLevel, level + 1);
    tee->autumn = reconstructTee(pixels, pixelsPerLevel, level + 1);
    tee->summer = reconstructTee(pixels, pixelsPerLevel, level + 1);

    (*pixelsPerLevel)[level] -= 1;
    // pixels[level] = realloc(pixels[level], (*pixelsPerLevel)[level] * sizeof(decompressPixel*));
    return tee;
}
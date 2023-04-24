#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
	DEAD,
	ALIVE
} TEE_TYPE;

typedef struct {
    unsigned char channels[3]; //0 - red, 1 - green, 2 - blue
} pxl;

typedef struct {
    pxl **pixels;
    unsigned int size;
} ppm;

typedef struct {
    pxl pixel;
    TEE_TYPE type;
} decompressPixel;

typedef struct QT {
    struct QT *summer, *autumn, *winter, *spring;
    pxl pixel;
    TEE_TYPE type;
}  Qtee, *qtee;

typedef struct NODE {
    struct NODE *next;
    qtee tee;
    int depth;
} Node, *node;

typedef struct {
    node head, tail;
} queue;


ppm *readImage(FILE *image);
void freeImage(ppm *img);
void printImage(ppm *img);
int compressRegion(ppm *img, int x, int y, unsigned int size, int similarityFactor, pxl *pixel);
void emptyPixel(pxl *pixel);
void decompress(FILE *compress, FILE *ppmToWrite);

void burnLeaf(qtee leaf);
void burnTee(qtee tee);
qtee makeLeaf(TEE_TYPE type, pxl pixel);
qtee makeTee(ppm *img, int x, int y, unsigned int size, int similarityFactor);
void printTee(qtee tee, int lvl);
void makeStats(qtee tee, unsigned int size, FILE *output);
void parseSideways(qtee tee, unsigned int *size, FILE *compressedPPM);
qtee reconstructTee(decompressPixel **pixels, int **pixelsPerLevel, int level);

void addToQueue(queue *_queue, qtee tee, int depth);
node removeFromQueue(queue *_queue);
void clearQueue(queue *_queue);
queue* initQueue();
int isQueueEmpty(queue *queue);

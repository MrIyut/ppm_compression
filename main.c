#include "header.h"

int main(int argc, char *argv[]){
    if (strcmp(argv[1], "-d") == 0){
        FILE *compress = fopen(argv[2], "rb");
        FILE *ppmToWrite = fopen(argv[3], "wb");
        decompress(compress, ppmToWrite);
        fclose(compress);
        fclose(ppmToWrite);
        return 0;
    }

    int similarityFactor = atoi(argv[2]);
    FILE *image = fopen(argv[3], "rb");
    ppm *img = readImage(image);
    fclose(image);

    qtee tee = makeTee(img, 0, 0, img->size, similarityFactor);
    FILE *output = fopen(argv[4], "w");
    if (strcmp(argv[1], "-c1") == 0)
        makeStats(tee, img->size, output);
    else
        parseSideways(tee, &img->size, output);
        
    burnTee(tee);
    freeImage(img);
    fclose(output);
    return 0;
}
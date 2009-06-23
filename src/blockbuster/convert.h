#ifndef BLOCKBUSTER_CONVERT_H
#define BLOCKBUSTER_CONVERT_H

#include "frames.h"
/* Conversion utilities from convert.c */
Image *ConvertImageToFormat(const Image *image, Canvas *canvas);
Image *ScaleImage(const Image *image, Canvas *canvas,
                  int srcX, int srcY, int srcWidth, int srcHeight,
                  int zoomedWidth, int zoomedHeight);

#endif
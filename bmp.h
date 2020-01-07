#ifndef _BMP_H_
#define _BMP_H_

#include "general.h"
#include "math.h"

#ifdef SWITCH_EINDIAN

#define SWAP32(_x) ((_x >> 24) | \
                   ((_x & 0x00FF0000) >> 8) | \
                   ((_x & 0x0000FF00) << 8) | \
                   ((_x & 0x000000FF) << 24))

#define SWAP16(_x) ((((_x) >> 8) | \
                   (((_x)  << 8) & 0xFF00)))

#else

#define SWAP32(_x) _x
#define SWAP16(_x) _x

#endif

#pragma pack(push, 1)
typedef struct pixelColor {
    u8 r;
    u8 g;
    u8 b;
} pixelColor;

typedef struct bmpHeader {
    u8 header[2]; // BM
    u32 sizeOfFile;
    u16 reserved[2];
    u32 imageDataOffset; // size of this + size of image header
} bmpHeader;

typedef struct bmpImageHeader {
    u32 headerSize; // = 40
    i32 width;
    i32 height;
    u16 colorPlanes; // 1
    u16 bitsPerPixel; // 24
    u32 compressionMethod; // 0 
    u32 imageSize; // 0 
    u32 hResolution; // 2835 
    u32 vResolution; // 2835 
    u32 numberOfColors; // 0
    u32 numberOfImportantColors; // 0
} bmpImageHeader;
#pragma pack(pop)

void writeBmpFile(u8* data, u32 width, u32 height){
    FILE* file = fopen("result.bmp", "wb");

    bmpHeader header;
    bmpImageHeader imageHeader;

    header.header[0] = 'B';
    header.header[1] = 'M';
    header.sizeOfFile = SWAP32(sizeof(bmpHeader) + sizeof(bmpImageHeader) + width * height * 3);
    header.reserved[0] = 0;
    header.reserved[1] = 0;
    header.imageDataOffset = SWAP32(sizeof(bmpHeader) + sizeof(bmpImageHeader));

    imageHeader.headerSize = SWAP32(sizeof(bmpImageHeader));
    imageHeader.width = SWAP32(width);
    imageHeader.height = SWAP32(-height);
    imageHeader.colorPlanes = SWAP16(1);
    imageHeader.bitsPerPixel = SWAP16(24);
    imageHeader.compressionMethod = SWAP32(0);
    imageHeader.imageSize = SWAP32(0);
    imageHeader.hResolution = SWAP32(2835);
    imageHeader.vResolution = SWAP32(2835);
    imageHeader.numberOfColors = SWAP32(0);
    imageHeader.numberOfImportantColors = SWAP32(0);

    fwrite(&header, sizeof(u8), sizeof(bmpHeader), file);
    fwrite(&imageHeader, sizeof(u8), sizeof(bmpImageHeader), file);
    fwrite(data, sizeof(u8), width * height * 3, file);
}

struct Bitmap {
    u32 width;
    u32 height;
    u8* data;
};

#endif // _BMP_H_
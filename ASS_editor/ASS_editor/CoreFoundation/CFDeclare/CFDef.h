//
//  CFDef.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/12.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFDef_h
#define CFDef_h

typedef enum CF2DAxis {
    CF2DAxisX,
    CF2DAxisY
} CF2DAxis;

typedef enum CF3DAxis {
    CF3DAxisX,
    CF3DAxisY,
    CF3DAxisZ
} CF3DAxis;

typedef enum CFTextEncoding
{
    CFTextEncodingUnkown,
    CFTextEncodingUTF8,
    CFTextEncodingUTF16BE,
    CFTextEncodingUTF16LE,
    CFTextEncodingUTF32BE,
    CFTextEncodingUTF32LE
} CFTextEncoding;

typedef struct CFUSize
{
    unsigned int x, y;
} CFUSize;

typedef struct CFSize
{
    int x, y;
} CFSize;

#endif /* CFDef_h */

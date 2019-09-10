//
//  CFTextProvider.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFTextProvider_h
#define CFTextProvider_h

#include <stdio.h>

typedef struct CFTextProvider *CFTextProviderRef;

/**
 CFTextProviderCreateWithFileUrl will not create with File that hasn't got 0x00 in it content
 
 @param url the standard C url describes the file location
 @return if success a CFTextProvider Object
 */
CFTextProviderRef CFTextProviderCreateWithFileUrl(const char *url);

CFTextProviderRef CFTextProviderCreateWithString(const char *string);

CFTextProviderRef CFTextProviderCreateWithWcharString(const wchar_t *string);

void CFTextProviderDestory(CFTextProviderRef textProvider);

#endif /* CFTextProvider_h */

//
//  CFASSFileStyle_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/9.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileStyle_Private_h
#define CFASSFileStyle_Private_h

#include "CFASSFileStyle.h"
#include "CFMacro.h"

CLANG_ASSUME_NONNULL_BEGIN

#pragma mark - Read File

/*!
 @function CFASSFileStyleCreateWithString
 @abstract Create a CFASSFileStyle Object by parsing content
 @param content should be a string start with "Style:" ended with '\0' or '\\n'
        sample: "Style: Name,FontName,25,&H00FFFFFF,&HF0000000,&H00000000,&HF0000000,-1,0,0,0,100,100,0,0.00,1,0,0,2,30,30,10,1"
 @param parsingResult [ownership not transferred] used to log info
 */
CFASSFileStyleRef CFASSFileStyleCreateWithString(const wchar_t * _Nonnull content, CFASSFileParsingResultRef _Nonnull parsingResult);

#pragma mark - Allocate Result

wchar_t *CFASSFileStyleAllocateFileContent(CFASSFileStyleRef style);

CLANG_ASSUME_NONNULL_END

#endif /* CFASSFileStyle_Private_h */

//
//  CFASSFileDialogue_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/17.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogue_Private_h
#define CFASSFileDialogue_Private_h

#include "CFASSFileDialogue.h"
#include "CFUnicodeString.h"
#include "CFASSFileParsingResult.h"
#include "CFASSFileParsingResult_Macro.h"
#include "CFMacro.h"

CLANG_ASSUME_NONNULL_BEGIN

#pragma mark - Read File

/*!
 @function CFASSFileDialogueCreateWithString
 @abstract Create ASS Dialogue representation with string
 @discussion this function should handle the empty source, that is point to '\\n'
 @param source begin with "Dialogue:", end with '\\n' or '\0'
 @param parsingResult [ownership not transferred]
 */
CFASSFileDialogueRef CFASSFileDialogueCreateWithString(const wchar_t * _Nonnull source, CFASSFileParsingResultRef _Nonnull parsingResult);

#pragma mark - Store Result

/*!
 @function CFASSFileDialogueStoreStringResult
 @abstract Store dialoge result into the targetPointer
 @param targetPoint this could be NULL
 @return if anything went wrong, function return -1
         if targetPoint is NULL, it will not try to store string in it
         return will be the store string Length if has infinite strage even targetPointer is NULL
         his begin with "Dialogue:" end with '\\n'
 */
int CFASSFileDialogueStoreStringResult(CFASSFileDialogueRef dialogue, wchar_t *targetPoint);

CLANG_ASSUME_NONNULL_END

#endif /* CFASSFileDialogue_Private_h */

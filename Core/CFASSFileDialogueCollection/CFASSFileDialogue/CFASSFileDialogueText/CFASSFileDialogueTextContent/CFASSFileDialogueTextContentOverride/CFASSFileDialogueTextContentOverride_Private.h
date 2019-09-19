//
//  CFASSFileDialogueTextContentOverride_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/11.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueTextContentOverride_Private_h
#define CFASSFileDialogueTextContentOverride_Private_h

#include <stdbool.h>
#include <stddef.h>
#include "CFASSFileDialogueTextContentOverride.h"
#include "CFASSFileParsingResult.h"
#include "CFASSFileParsingResult_Macro.h"
#include "CFMacro.h"

CLANG_ASSUME_NONNULL_BEGIN

/*!
 @function CFASSFileDialogueTextContentOverrideCreateWithString
 @abstract parsing string for a dialogueTextContentOverride object
 @param leftBrace point to '{'
 @param rightBrace point to '}'
 @return a valid dialogueTextContentOverride object if possible, NULL if error
 */
CFASSFileDialogueTextContentOverrideRef CFASSFileDialogueTextContentOverrideCreateWithString(const wchar_t * _Nonnull leftBrace,
                                                                                             const wchar_t * _Nonnull rightBrace,
                                                                                             CFASSFileParsingResultRef _Nonnull parsingResult);


int CFASSFileDialogueTextContentOverrideStoreStringResult(CFASSFileDialogueTextContentOverrideRef override, wchar_t * targetPoint);
/* targetPoint */
// this could be NULL
/* Return */
// if anything went wrong, function return -1
// if targetPoint is NULL, it will not try to store string in it
// return will be the store string Length if targetPoint is long enough even is NULL

CLANG_ASSUME_NONNULL_END

#endif /* CFASSFileDialogueTextContentOverride_Private_h */

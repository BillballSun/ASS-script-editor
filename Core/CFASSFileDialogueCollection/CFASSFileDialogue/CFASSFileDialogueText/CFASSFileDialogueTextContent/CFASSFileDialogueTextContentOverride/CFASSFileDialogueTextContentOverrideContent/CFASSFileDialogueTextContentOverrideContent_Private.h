//
//  CFASSFileDialogueTextContentOverrideContent_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/15.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueTextContentOverrideContent_Private_h
#define CFASSFileDialogueTextContentOverrideContent_Private_h

#include <wchar.h>
#include "CFASSFileDialogueTextContentOverrideContent.h"
#include "CFASSFileParsingResult.h"
#include "CFMacro.h"

CLANG_ASSUME_NONNULL_BEGIN

#pragma mark - Read File

/*!
 @function CFASSFileDialogueTextContentOverrideContentCreateWithString
 @abstract create a dialogueTextContentOverrideContent object
 @param tokenBegin point to override start, which is definite '\'
 @param tokenEnd points to the last possible content, this could be NULL
        for example, \move (1, 1), then the endPoint points to ')' is correct
        if not NULL, tokenEnd >= tokenBegin
 @param parsingResult apart from other, this could be NULL for some kind support
 */
CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideContentCreateWithString(const wchar_t * _Nonnull tokenBegin,
                                                                                                           const wchar_t * _Nullable tokenEnd,
                                                                                                           CFASSFileParsingResultRef _Nonnull parsingResult);

#pragma mark - Store Result

int CFASSFileDialogueTextContentOverrideContentStoreStringResult(CFASSFileDialogueTextContentOverrideContentRef overrideContent, wchar_t * targetPoint);
/* targetPoint */
// this could be NULL
/* Return */
// if anything went wrong, function return -1
// if targetPoint is NULL, it will not try to store string in it
// return will be the store string Length if targetPoint is long enough even is NULL

CLANG_ASSUME_NONNULL_END

#endif /* CFASSFileDialogueTextContentOverrideContent_Private_h */

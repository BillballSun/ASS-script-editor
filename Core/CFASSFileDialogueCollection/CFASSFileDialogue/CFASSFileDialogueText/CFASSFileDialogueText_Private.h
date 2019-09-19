//
//  CFASSFileDialogueText_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/11.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueText_Private_h
#define CFASSFileDialogueText_Private_h

#include "CFASSFileDialogueText.h"
#include "CFASSFileParsingResult.h"
#include "CFMacro.h"

CLANG_ASSUME_NONNULL_BEGIN

#pragma mark - Read File

/*!
 @function CFASSFileDialogueTextCreateWithString
 @abstract parsing string for a dialogueText object
 @param source a string just point to the content ended with '\0' or '\\n'
        any prefix ','
 @param parsingResult [ownership not transferred]
 @return a valid dialogue object, or NULL if error encountered
 */
CFASSFileDialogueTextRef CFASSFileDialogueTextCreateWithString(const wchar_t * _Nonnull source,
                                                               CFASSFileParsingResultRef _Nonnull parsingResult);
#pragma mark - Store Result

/*!
 @function CFASSFileDialogueTextStoreStringResult
 @param targetPoint it size should be large enough to store result, passing NULL
        to first query for size
 @return the actually size needed to store string
 */
int CFASSFileDialogueTextStoreStringResult(CFASSFileDialogueTextRef _Nonnull text, wchar_t * _Nullable targetPoint);

CLANG_ASSUME_NONNULL_END

#endif /* CFASSFileDialogueText_Private_h */

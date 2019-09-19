//
//  CFASSFileDialogueEffect_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/16.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueEffect_Private_h
#define CFASSFileDialogueEffect_Private_h

#include "CFASSFileDialogueEffect.h"
#include "CFASSFileParsingResult.h"
#include "CFMacro.h"

CLANG_ASSUME_NONNULL_BEGIN

/*!
 @function CFASSFileDialogueEffectCreateWithString
 @abstract parsing the content of dialogue effect
 @discussion beginPoint and endPoint should just enclose the effect content
             without any prefix subfix ',' character which should be taken
             already, and beginPoint <= endPoint
 @return a valid effect object, or NULL if parsing failed, as it is not a valid
         effect sequence
 */
CFASSFileDialogueEffectRef CFASSFileDialogueEffectCreateWithString(const wchar_t * _Nonnull beginPoint,
                                                                   const wchar_t * _Nonnull endPoint,
                                                                   CFASSFileParsingResultRef _Nonnull parsingResult);


int CFASSFileDialogueEffectStoreStringResult(CFASSFileDialogueEffectRef effect, wchar_t * _Nonnull targetPoint);
/* targetPoint */
// this could be NULL
/* Return */
// if anything went wrong, function return -1
// if targetPoint is NULL, it will not try to store string in it
// return will be the store string Length if targetPoint is long enough even is NULL

CLANG_ASSUME_NONNULL_END

#endif /* CFASSFileDialogueEffect_Private_h */

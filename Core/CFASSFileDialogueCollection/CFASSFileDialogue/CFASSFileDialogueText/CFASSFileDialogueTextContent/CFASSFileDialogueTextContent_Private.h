//
//  CFASSFileDialogueTextContent_Prvivate.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/11.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueTextContent_Private_h
#define CFASSFileDialogueTextContent_Private_h

#include "CFASSFileDialogueTextContent.h"
#include "CFASSFileParsingResult.h"
#include "CFMacro.h"

CLANG_ASSUME_NONNULL_BEGIN

/*!
 @function CFASSFileDialogueTextContentCreateWithString
 @abstract parsing string for a dialogueTextContent object
 @discussion This is used to parsing string that got some content without end
             character '\0' or '\\n', but inclosing '{' '}' braces
             if this is an empty text this should be called with
             CFASSFileDialogueTextContentCreateEmptyString
             otherwise what kind of input is data and endPoint
 @param beginPoint the content beginPoint, enclosing '{'
 @param endPoint the content endPoint, enclosing '}', exclusive '\0' and '\\n'
 @return a valid dialogueTextContent if parsing success, NULL otherwise
 */
CFASSFileDialogueTextContentRef CFASSFileDialogueTextContentCreateWithString(CFASSFileDialogueTextContentType type,
                                                                             const wchar_t * _Nonnull beginPoint,
                                                                             const wchar_t * _Nonnull endPoint,
                                                                             CFASSFileParsingResultRef _Nonnull parsingResult);

CFASSFileDialogueTextContentRef CFASSFileDialogueTextContentCreateEmptyString(void);

/*!
 @function CFASSFileDialogueTextContentStoreStringResult
 @abstract try to store content into targetPoint
           if targetPoint is NULL, it will not try to store string in it
 @param targetPoint assing NULL first to get string length
 @return the actual store string Length
         if anything went wrong, return -1
 */
int CFASSFileDialogueTextContentStoreStringResult(CFASSFileDialogueTextContentRef textContent, wchar_t * _Nullable targetPoint);

CLANG_ASSUME_NONNULL_END

#endif /* CFASSFileDialogueTextContent_Private_h */

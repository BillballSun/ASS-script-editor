//
//  CFUnicodeStringArray.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFUnicodeStringArray_h
#define CFUnicodeStringArray_h

#include <stdlib.h>
#include <stdbool.h>

typedef struct CFUnicodeStringArray *CFUnicodeStringArrayRef;

#pragma mark - Create / Destory

CFUnicodeStringArrayRef CFUnicodeStringArrayCreateEmpty(void);

CFUnicodeStringArrayRef CFUnicodeStringArrayCopy(CFUnicodeStringArrayRef array);

void CFUnicodeStringArrayDestory(CFUnicodeStringArrayRef array);

#pragma mark - Get Element

size_t CFUnicodeStringArrayGetLength(CFUnicodeStringArrayRef array);
/* Return Value */
// the length of the added string

const wchar_t *CFUnicodeStringArrayGetStringAtIndex(CFUnicodeStringArrayRef array, size_t index);

#pragma mark - Manipulate String

size_t CFUnicodeStringArrayAddString(CFUnicodeStringArrayRef array, const wchar_t * string, bool transferOwnership);
/* Return Value */
// the length of the added string, return (size_t)-1 if failed

void CFUnicodeStringArrayRemoveStringAtIndex(CFUnicodeStringArrayRef array, size_t index);

size_t CFUnicodeStringArrayInsertStringAtIndex(CFUnicodeStringArrayRef array, const wchar_t * string, size_t index);

void CFUnicodeStringArraySwapStringPosition(CFUnicodeStringArrayRef array, size_t index1, size_t index2);

/**
 CFUnicodeStringArrayAddStringWithEndChar will add string to the Array, but will treat the endChar as the '\0' in the string
 
 @param endChar treat as '\0' in the string
 @param encounterEndChar if not NULL, contain whether the string taken as element has ends with endChar in previous string, that is if endChar is '\0', this definitely contains YES value
 @return the length of the added string
 */
size_t CFUnicodeStringArrayAddStringWithEndChar(CFUnicodeStringArrayRef array, const wchar_t *string, const wchar_t endChar, bool *encounterEndChar);

#endif /* CFUnicodeStringArray_h */

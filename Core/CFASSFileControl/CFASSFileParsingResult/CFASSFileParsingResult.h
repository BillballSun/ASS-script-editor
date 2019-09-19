//
//  CFASSFileParsingResult.h
//  CACoreFoundation
//
//  Created by Bill Sun on 2019/9/13.
//

#ifndef CFASSFileParsingResult_h
#define CFASSFileParsingResult_h

#include <stddef.h>
#include <stdbool.h>
#include "CFUnicodeString.h"
#include "CFMacro.h"

typedef enum {
    CFASSFileParsingResultLevelInfo,
    CFASSFileParsingResultLevelWarning,
    CFASSFileParsingResultLevelError,
}   CFASSFileParsingResultLevel;

typedef struct CFASSFileParsingResult *CFASSFileParsingResultRef;

CLANG_ASSUME_NONNULL_BEGIN

#pragma mark - Create / Copy / Destory

CFASSFileParsingResultRef CFASSFileParsingResultCreate(void);

CFASSFileParsingResultRef CFASSFileParsingResultCopy(CFASSFileParsingResultRef _Nonnull ref);

void CFASSFileParsingResultDestory(CFASSFileParsingResultRef _Nullable ref);

#pragma mark - Info

size_t CFASSFileParsingResultGetCount(CFASSFileParsingResultRef _Nonnull ref);

void CFASSFileParsingResultAdd(CFASSFileParsingResultRef _Nonnull ref,
                               CFASSFileParsingResultLevel level,
                               const wchar_t * _Nullable point,
                               CFUnicodeStringRef _Nonnull string);

void CFASSFileParsingResultAddWCSFormat(CFASSFileParsingResultRef _Nonnull ref,
                                        CFASSFileParsingResultLevel level,
                                        const wchar_t * _Nullable point,
                                        const wchar_t * _Nonnull format, ...);

bool CFASSFileParsingResultGet(CFASSFileParsingResultRef _Nonnull ref,
                               size_t index,
                               CFASSFileParsingResultLevel * _Nullable level,
                               const wchar_t * _Nullable * _Nullable point,
                               CFUnicodeStringRef _Nonnull * _Nullable string_withoutOwnership);

#pragma mark - optional info

void CFASSFileParsingResultSetParsingString(CFASSFileParsingResultRef _Nonnull ref,
                                            wchar_t * _Nullable parsingString,
                                            bool ownershipTransferred);

wchar_t * _Nullable CFASSFileParsingResultGetParsingString(CFASSFileParsingResultRef _Nonnull ref);

void CFASSFileParsingResultSetFileName(CFASSFileParsingResultRef _Nonnull ref,
                                       wchar_t * _Nullable fileName,
                                       bool ownershipTransferred);

wchar_t * _Nullable CFASSFileParsingResultGetFileName(CFASSFileParsingResultRef _Nonnull ref);

#pragma mark - debug

void CFASSFileParsingResultDebugInStdout(CFASSFileParsingResultRef _Nonnull ref);

CLANG_ASSUME_NONNULL_END

#endif /* CFASSFileParsingResult_h */

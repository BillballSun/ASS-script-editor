//
//  CFASSFileParsingResultContent.h
//  ASS_editor
//
//  Created by Bill Sun on 2019/9/14.
//

#ifndef CFASSFileParsingResultContent_h
#define CFASSFileParsingResultContent_h

#include "CFMacro.h"
#include "CFASSFileParsingResult.h"

typedef struct CFASSFileParsingResultContent *CFASSFileParsingResultContentRef;

CLANG_ASSUME_NONNULL_BEGIN

CFASSFileParsingResultContentRef CFASSFileParsingResultContentCreate(CFASSFileParsingResultLevel level,
                                                                     const wchar_t * _Nullable point,
                                                                     CFUnicodeStringRef _Nonnull string,
                                                                     bool transferOwnership);

CFASSFileParsingResultContentRef CFASSFileParsingResultContentCopy(CFASSFileParsingResultContentRef _Nonnull content);

void CFASSFileParsingResultContentDestory(CFASSFileParsingResultContentRef _Nullable content);

bool CFASSFileParsingResultContentGetInfo(CFASSFileParsingResultContentRef _Nonnull content,
                                          CFASSFileParsingResultLevel * _Nullable level,
                                          const wchar_t * _Nullable * _Nullable point,
                                          CFUnicodeStringRef _Nonnull * _Nullable string);

CLANG_ASSUME_NONNULL_END

#endif /* CFASSFileParsingResultContent_h */

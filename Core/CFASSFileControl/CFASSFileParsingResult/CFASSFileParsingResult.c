//
//  CFASSFileParsingResult.c
//  CACoreFoundation
//
//  Created by Bill Sun on 2019/9/13.
//

#include <stdlib.h>
#include <stdarg.h>
#include <wchar.h>
#include <stdio.h>
#include <stdbool.h>
#include "CFPointerArray.h"
#include "CFEnumerator.h"
#include "CFASSFileParsingResult.h"
#include "CFASSFileParsingResultContent.h"
#include "CFStringTool.h"

struct CFASSFileParsingResult {
    CFPointerArrayRef array;           // store CFASSFileParsingResultContentRef, nonnull
    wchar_t * _Nullable parsingString; // nullable (has ownership)
    wchar_t * _Nullable fileName;      // nullable (has ownership)
};

CLANG_DIAGNOSTIC_PUSH
CLANG_DIAGNOSTIC_IGNORE_NONNULL

#pragma mark - Content

#pragma mark - Create / Copy / Destory

CFASSFileParsingResultRef CFASSFileParsingResultCreate(void) {
    CFASSFileParsingResultRef result;
    if ((result = malloc(sizeof(struct CFASSFileParsingResult))) != NULL) {
        if ((result->array = CFPointerArrayCreateEmpty()) != NULL) {
            result->fileName = NULL;
            result->parsingString = NULL;
            return result;
        }
        free(result);
    }
    return NULL;
}

CFASSFileParsingResultRef CFASSFileParsingResultCopy(CFASSFileParsingResultRef _Nonnull ref) {
    DEBUG_ASSERT(ref != NULL); if(ref != NULL) {
        CFASSFileParsingResultRef copy;
        if ((copy = CFASSFileParsingResultCreate()) != NULL) {
            CFEnumeratorRef enumerator = CFEnumeratorCreateFromArray(ref->array);
            CFASSFileParsingResultContentRef eachContent;
            while((eachContent = CFEnumeratorNextObject(enumerator)) != NULL) {
                CFASSFileParsingResultLevel level;
                const wchar_t * _Nullable point;
                CFUnicodeStringRef _Nonnull string;
                if(CFASSFileParsingResultContentGetInfo(eachContent, &level, &point, &string)) {
                    DEBUG_ASSERT(string != NULL);
                    CFASSFileParsingResultAdd(copy, level, point, string);
                } DEBUG_ELSE    // to make compatibility
            }
            CFEnumeratorDestory(enumerator);
            wchar_t * _Nullable fileName = CFASSFileParsingResultGetFileName(ref);
            if(fileName != NULL) CFASSFileParsingResultSetFileName(copy, fileName, false);
            wchar_t * _Nullable parsingString = CFASSFileParsingResultGetParsingString(ref);
            if(parsingString != NULL) CFASSFileParsingResultSetParsingString(copy, parsingString, false);
            return copy;
        }
    }
    return NULL;
}

void CFASSFileParsingResultDestory(CFASSFileParsingResultRef _Nullable ref) {
    if(ref != NULL) {
        CFEnumeratorRef enumerator = CFEnumeratorCreateFromArray(ref->array);
        CFASSFileParsingResultContentRef eachContent;
        while((eachContent = CFEnumeratorNextObject(enumerator)) != NULL)
            CFASSFileParsingResultContentDestory(eachContent);
        CFEnumeratorDestory(enumerator);
        CFPointerArrayDestory(ref->array);
        if(ref->fileName != NULL) free(ref->fileName);
        if(ref->parsingString != NULL) free(ref->parsingString);
        free(ref);
    }
}

#pragma mark - Info

size_t CFASSFileParsingResultGetCount(CFASSFileParsingResultRef _Nonnull ref) {
    DEBUG_ASSERT(ref != NULL); if(ref != NULL)
        return CFPointerArrayGetCount(ref->array);
    return 0u;
}

void CFASSFileParsingResultAddWCSFormat(CFASSFileParsingResultRef _Nonnull ref,
                                        CFASSFileParsingResultLevel level,
                                        const wchar_t * _Nullable point,
                                        const wchar_t * _Nonnull format, ...) {
    DEBUG_ASSERT(ref != NULL && format != NULL); if(ref != NULL && format != NULL) {
        va_list ap; va_start(ap, format); CFUnicodeStringRef str = CFUnicodeStringCreateWithVAGFormat(format, ap); va_end(ap);
        if(str != NULL) {
            CFASSFileParsingResultContentRef content;
            if((content = CFASSFileParsingResultContentCreate(level, point, str, true)) != NULL) {
                CFPointerArrayAddPointer(ref->array, content, false);
                return;     // success exit
            } DEBUG_ELSE
            CFUnicodeStringDestory(str);
        } DEBUG_ELSE
    } DEBUG_ELSE
}

void CFASSFileParsingResultAdd(CFASSFileParsingResultRef _Nonnull ref,
                               CFASSFileParsingResultLevel level,
                               const wchar_t * _Nullable point,
                               CFUnicodeStringRef _Nonnull string) {
    DEBUG_ASSERT(ref != NULL); if(ref != NULL) {
        CFASSFileParsingResultContentRef content;
        if((content = CFASSFileParsingResultContentCreate(level, point, string, false)) != NULL) {
            CFPointerArrayAddPointer(ref->array, content, false);
        } DEBUG_ELSE
    }
}

bool CFASSFileParsingResultGet(CFASSFileParsingResultRef _Nonnull ref,
                               size_t index,
                               CFASSFileParsingResultLevel * _Nullable level,
                               const wchar_t * _Nullable * _Nullable point,
                               CFUnicodeStringRef _Nonnull * _Nullable string_withoutOwnership) {
    DEBUG_ASSERT(ref != NULL); if(ref != NULL) {
        size_t count;
        if(index < (count = CFPointerArrayGetCount(ref->array))) {
            CFASSFileParsingResultContentRef content = CFPointerArrayGetPointerAtIndex(ref->array, index);
            DEBUG_ASSERT(content != NULL);
            CFASSFileParsingResultContentGetInfo(content, level, point, string_withoutOwnership);
            return true;
        } DEBUG_ELSE
    } DEBUG_ELSE
    return false;
}

#pragma mark - optional info

void CFASSFileParsingResultSetParsingString(CFASSFileParsingResultRef _Nonnull ref,
                                            wchar_t * _Nullable parsingString,
                                            bool ownershipTransferred) {
    DEBUG_ASSERT(ref != NULL); if(ref != NULL) {
        if(parsingString != NULL && !ownershipTransferred)
            if((parsingString = CF_Dump_wchar_string(parsingString)) == NULL) return;
        if(ref->parsingString != NULL) free(ref->parsingString);
        ref->parsingString = parsingString;
    }
}

wchar_t * _Nullable CFASSFileParsingResultGetParsingString(CFASSFileParsingResultRef _Nonnull ref) {
    DEBUG_ASSERT(ref != NULL); if(ref != NULL)
        return ref->parsingString;
    return NULL;
}

void CFASSFileParsingResultSetFileName(CFASSFileParsingResultRef _Nonnull ref,
                                       wchar_t * _Nullable fileName,
                                       bool ownershipTransferred) {
    DEBUG_ASSERT(ref != NULL); if(ref != NULL) {
        if(fileName != NULL && !ownershipTransferred)
            if((fileName = CF_Dump_wchar_string(fileName)) == NULL) return;
        if(ref->fileName != NULL) free(ref->fileName);
        ref->fileName = fileName;
    }
}

wchar_t * _Nullable CFASSFileParsingResultGetFileName(CFASSFileParsingResultRef _Nonnull ref) {
    DEBUG_ASSERT(ref != NULL); if(ref != NULL)
        return ref->fileName;
    return NULL;
}

#pragma mark - debug

void CFASSFileParsingResultDebugInStdout(CFASSFileParsingResultRef _Nonnull ref) {
    DEBUG_ASSERT(ref != NULL); if(ref == NULL) return;
    size_t count = CFPointerArrayGetCount(ref->array);
    for(size_t index = 0; index < count; index++) {
        CFASSFileParsingResultContentRef content;
        if((content = CFPointerArrayGetPointerAtIndex(ref->array, index)) != NULL) {
            CFASSFileParsingResultLevel level; const wchar_t *point; CFUnicodeStringRef string;
            if(CFASSFileParsingResultContentGetInfo(content, &level, &point, &string)) {
                wchar_t *allocatedRaw; DEBUG_ASSERT(string != NULL);
                if((allocatedRaw = CFUnicodeStringAllocateWCStringRepresentation(string)) != NULL) {
                    switch (level) {
                        case CFASSFileParsingResultLevelInfo:
                            fprintf(stdout, " [INFO] %ls\n", allocatedRaw);
                            break;
                        case CFASSFileParsingResultLevelWarning:
                            fprintf(stdout, " [WARN] %ls\n", allocatedRaw);
                            break;
                        case CFASSFileParsingResultLevelError:
                            fprintf(stdout, "[ERROR] %ls\n", allocatedRaw);
                            break;
                        default: DEBUG_POINT; break;
                    }
                    if(ref->parsingString != NULL && point != NULL) {
                        int line;
                        if((line = CF_find_line_number_in_wchar_text(ref->parsingString, point)) >= 0) {
                            fwprintf(stdout, L"LINE %d: ", line);
                            const wchar_t *endPoint = point;
                            while (endPoint[0] != L'\0' && endPoint[0] != L'\n') endPoint++;
                            fwprintf(stdout, L"SAMPLE: ");
                            while (point != endPoint) {
                                putwc(point[0], stdout);
                                point++;
                            } putwc(L'\n', stdout); putwc(L'\n', stdout);
                        }
                    }
                    free(allocatedRaw);
                }
            } DEBUG_ELSE
        } DEBUG_ELSE
    }
}

CLANG_DIAGNOSTIC_POP

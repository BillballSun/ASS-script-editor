//
//  CFASSFileParsingResultContent.c
//  ASS_editor
//
//  Created by Bill Sun on 2019/9/14.
//

#include <stdlib.h>
#include "CFStringTool.h"
#include "CFASSFileParsingResultContent.h"

struct CFASSFileParsingResultContent {
    CFASSFileParsingResultLevel level;
    const wchar_t * _Nullable point;        // nullable
    CFUnicodeStringRef _Nonnull string;     // nonnull
};

static bool CFASSFileParsingResultContentValidate(CFASSFileParsingResultContentRef content);

CLANG_DIAGNOSTIC_PUSH
CLANG_DIAGNOSTIC_IGNORE_NONNULL

#pragma mark - Content

CFASSFileParsingResultContentRef CFASSFileParsingResultContentCreate(CFASSFileParsingResultLevel level,
                                                                     const wchar_t * _Nullable point,
                                                                     CFUnicodeStringRef _Nonnull string,
                                                                     bool transferOwnership) {
    DEBUG_ASSERT(string != NULL); if(string != NULL) {
        if(!transferOwnership) string = CFUnicodeStringCopy(string);
        CFASSFileParsingResultContentRef result;
        if((result = malloc(sizeof(struct CFASSFileParsingResultContent))) != NULL) {
            result->level = level;
            result->point = point;
            result->string = string;
            return result;
        }
        CFUnicodeStringDestory(string);
    }
    return NULL;
}

static bool CFASSFileParsingResultContentValidate(CFASSFileParsingResultContentRef _Nonnull content) {
    DEBUG_ASSERT(content != NULL); if(content != NULL) {
        if(content->level == CFASSFileParsingResultLevelInfo ||
           content->level == CFASSFileParsingResultLevelWarning ||
           content->level == CFASSFileParsingResultLevelError) {
            if(content->string != NULL && CFUnicodeStringValidate(content->string))
                return true;
        } DEBUG_ELSE
    } DEBUG_ELSE
    return false;
}

CFASSFileParsingResultContentRef CFASSFileParsingResultContentCopy(CFASSFileParsingResultContentRef _Nonnull content) {
    DEBUG_ASSERT(content != NULL && CFASSFileParsingResultContentValidate(content));
    if(content != NULL && CFASSFileParsingResultContentValidate(content)) {
        CFASSFileParsingResultContentRef copy;
        if((copy = CFASSFileParsingResultContentCreate(content->level, content->point, content->string, false)) != NULL)
            return copy;
    } DEBUG_ELSE
    return NULL;
}

void CFASSFileParsingResultContentDestory(CFASSFileParsingResultContentRef _Nullable content) {
    DEBUG_ASSERT(content == NULL || CFASSFileParsingResultContentValidate(content));
    if (content != NULL && CFASSFileParsingResultContentValidate(content)) {
        CFUnicodeStringDestory(content->string);
        free(content);
    }
}

bool CFASSFileParsingResultContentGetInfo(CFASSFileParsingResultContentRef _Nonnull content,
                                          CFASSFileParsingResultLevel * _Nullable level,
                                          const wchar_t * _Nullable * _Nullable point,
                                          CFUnicodeStringRef _Nonnull * _Nullable string) {
    DEBUG_ASSERT(content != NULL && CFASSFileParsingResultContentValidate(content));
    if(content != NULL && CFASSFileParsingResultContentValidate(content)) {
        if(level != NULL) *level = content->level;
        if(point != NULL) *point = content->point;
        if(string != NULL) *string = content->string;
        return true;
    }
    return false;
}

CLANG_DIAGNOSTIC_POP

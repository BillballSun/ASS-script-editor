//
//  CFASSFileDialogueTextContentOverride.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/10.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdlib.h>
#include <stddef.h>

#include "CFPointerArray.h"
#include "CFASSFileDialogueTextContentOverride.h"
#include "CFASSFileDialogueTextContentOverride_Private.h"
#include "CFASSFileDialogueTextContentOverrideContent.h"
#include "CFASSFileDialogueTextContentOverrideContent_Private.h"
#include "CFStringTool.h"
#include "CFException.h"
#include "CFEnumerator.h"
#include "CFASSFileChange.h"
#include "CFASSFileChange_Private.h"
#include "CFASSFileParsingResult.h"
#include "CFASSFileParsingResult_Macro.h"
#include "CFMacro.h"

struct CFASSFileDialogueTextContentOverride
{
    CFPointerArrayRef contentArray;
};

CLANG_DIAGNOSTIC_PUSH
CLANG_DIAGNOSTIC_IGNORE_NONNULL

void CFASSFileDialogueTextContentOverrideMakeChange(CFASSFileDialogueTextContentOverrideRef override, CFASSFileChangeRef change)
{
    if(override == NULL || change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContentOverride %p MakeChange %p", override, change);
    CFEnumeratorRef enumerator = CFASSFileDialogueTextContentOverrideCreateEnumerator(override);
    CFASSFileDialogueTextContentOverrideContentRef eachContent;
    while((eachContent = CFEnumeratorNextObject(enumerator)) != NULL)
        CFASSFileDialogueTextContentOverrideContentMakeChange(eachContent, change);
    CFEnumeratorDestory(enumerator);
}

CFEnumeratorRef CFASSFileDialogueTextContentOverrideCreateEnumerator(CFASSFileDialogueTextContentOverrideRef override)
{
    if(override == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContentOverride NULL CreateEnumerator");
    return CFEnumeratorCreateFromArray(override->contentArray);
}

CFASSFileDialogueTextContentOverrideRef CFASSFileDialogueTextContentOverrideCreateWithContent(CFASSFileDialogueTextContentOverrideContentRef content)
{
    if(content == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContentOverrideCreateWithContent NULL");
        return NULL;
    }
    CFASSFileDialogueTextContentOverrideRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverride))) != NULL)
    {
        if((result->contentArray = CFPointerArrayCreateEmpty()) != NULL)
        {
            CFPointerArrayAddPointer(result->contentArray, content, false);
            return result;
        }
        free(result);
    }
    return NULL;
}

void CFASSFileDialogueTextContentOverrideAddContent(CFASSFileDialogueTextContentOverrideRef override, CFASSFileDialogueTextContentOverrideContentRef content)
{
    if(override == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContentOverride NULL AddContent");
        return;
    }
    if(content == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContentOverride %X AddContent NULL", (unsigned int)override);
        return;
    }
    CFPointerArrayAddPointer(override->contentArray, content, false);
}

CFASSFileDialogueTextContentOverrideRef CFASSFileDialogueTextContentOverrideCopy(CFASSFileDialogueTextContentOverrideRef override)
{
    CFASSFileDialogueTextContentOverrideRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverride))) != NULL)
    {
        if((result->contentArray = CFPointerArrayCreateEmpty()) != NULL)
        {
            size_t arrayCount = CFPointerArrayGetCount(override->contentArray);
            bool copyCheck = true;
            CFASSFileDialogueTextContentOverrideContentRef eachOverrideContent;
            for(size_t index = 0; index < arrayCount && copyCheck; index++)
            {
                eachOverrideContent = CFASSFileDialogueTextContentOverrideContentCopy
                ((CFASSFileDialogueTextContentOverrideContentRef)CFPointerArrayGetPointerAtIndex(override->contentArray, index));
                if(eachOverrideContent == NULL)
                    copyCheck = false;
                else
                    CFPointerArrayAddPointer(result->contentArray, eachOverrideContent, false);
            }
            if(copyCheck)
                return result;
            arrayCount = CFPointerArrayGetCount(result->contentArray);
            for(size_t index = 0; index < arrayCount; index++)
                CFASSFileDialogueTextContentOverrideContentDestory
                ((CFASSFileDialogueTextContentOverrideContentRef)CFPointerArrayGetPointerAtIndex(result->contentArray, index));
        }
        free(result);
    }
    return NULL;
}

void CFASSFileDialogueTextContentOverrideDestory(CFASSFileDialogueTextContentOverrideRef override)
{
    if(override == NULL) return;
    size_t arrayCount = CFPointerArrayGetCount(override->contentArray);
    for(size_t index = 0; index < arrayCount; index++)
        CFASSFileDialogueTextContentOverrideContentDestory
        ((CFASSFileDialogueTextContentOverrideContentRef)CFPointerArrayGetPointerAtIndex(override->contentArray, index));
    CFPointerArrayDestory(override->contentArray);
    free(override);
}

int CFASSFileDialogueTextContentOverrideStoreStringResult(CFASSFileDialogueTextContentOverrideRef override, wchar_t * targetPoint)
{
    if(override == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContentOverride NULL StoreStringResult");
        return -1;
    }
    int result = 0, temp;
    if(targetPoint == NULL)
    {
        size_t arrayCount = CFPointerArrayGetCount(override->contentArray);
        for(size_t index = 0; index < arrayCount; index++)
        {
            temp = CFASSFileDialogueTextContentOverrideContentStoreStringResult
            ((CFASSFileDialogueTextContentOverrideContentRef)CFPointerArrayGetPointerAtIndex(override->contentArray, index), NULL);
            if(temp < 0) return -1;
            else result += temp;
        }
        return result + 2;  /* braces */
    }
    else
    {
        *targetPoint++ = L'{';
        size_t arrayCount = CFPointerArrayGetCount(override->contentArray);
        for(size_t index = 0; index < arrayCount; index++)
        {
            temp = CFASSFileDialogueTextContentOverrideContentStoreStringResult
            ((CFASSFileDialogueTextContentOverrideContentRef)CFPointerArrayGetPointerAtIndex(override->contentArray, index), targetPoint);
            if(temp < 0)
                return -1;
            else
            {
                result += temp;
                targetPoint += temp;
            }
        }
        targetPoint[0] = L'}';
        targetPoint[1] = L'\0';
        return result + 2;  /* braces */
    }
}

CFASSFileDialogueTextContentOverrideRef CFASSFileDialogueTextContentOverrideCreateWithString(const wchar_t * _Nonnull leftBrace,
                                                                                             const wchar_t * _Nonnull rightBrace,
                                                                                             CFASSFileParsingResultRef _Nonnull parsingResult)
{
    DEBUG_ASSERT(leftBrace != NULL && rightBrace != NULL && leftBrace < rightBrace && leftBrace[0] == L'{' && rightBrace[0] == L'}' && parsingResult != NULL);
    if(leftBrace == NULL || rightBrace == NULL || leftBrace >= rightBrace || leftBrace[0] != L'{' || rightBrace[0] != L'}' || parsingResult == NULL) return NULL;
    
    CFASSFileDialogueTextContentOverrideRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverride))) != NULL)
    {
        if((result->contentArray = CFPointerArrayCreateEmpty()) != NULL)
        {
            if(leftBrace + 1 == rightBrace) return result;      // empty braces
            
            bool formatCheck = true;
            for(const wchar_t *tokenBegin = leftBrace + 1, *tokenEnd; tokenBegin < rightBrace; tokenBegin = tokenEnd + 1) {
                if(tokenBegin[0] != L'\\') {
                    PR_ERROR(tokenBegin, L"CFASSFileDialogueTextContentOverrideContent not begin with \\");
                    formatCheck = false; break;
                }
                
                tokenEnd = tokenBegin;
                while(tokenEnd + 1 < rightBrace) {  // exact the tokenEnd
                    if(tokenEnd[1] == L'\\') break;  // exact the tokenEnd
                    else if(tokenEnd[1] == L'(') {
                        const wchar_t *braceMatch;
                        if((braceMatch = CF_match_next_braces(tokenEnd + 1, rightBrace - 1, true)) != NULL)
                            tokenEnd = braceMatch - 1;  // it is advanced later
                        else {
                            PR_ERROR(tokenEnd + 1, L"CFASSFileDialogueTextContentOverrideContent brace match failure");
                            formatCheck = false; break;
                        }
                    }
                    tokenEnd++;
                }
                
                if(formatCheck) {   // [tokenBegin, tokenEnd] just enclosing "\some(1, 2, 3)"
                    CFASSFileDialogueTextContentOverrideContentRef override;
                    if((override = CFASSFileDialogueTextContentOverrideContentCreateWithString(tokenBegin, tokenEnd, parsingResult)) != NULL)
                        CFPointerArrayAddPointer(result->contentArray, override, false);
                    else PR_ERROR(tokenBegin, L"CFASSFileDialogueTextContentOverrideContent create failed");
                }
            }
            
            if(formatCheck) return result;
            
            size_t arrayCount = CFPointerArrayGetCount(result->contentArray);
            for(size_t index = 0; index < arrayCount; index++)
                CFASSFileDialogueTextContentOverrideContentDestory
                ((CFASSFileDialogueTextContentOverrideContentRef)CFPointerArrayGetPointerAtIndex(result->contentArray, index));
            CFPointerArrayDestory(result->contentArray);
        }
        free(result);
    }
    return NULL;
}

CLANG_DIAGNOSTIC_POP

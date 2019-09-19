//
//  CFASSFileStyleCollection.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "CFASSFile.h"
#include "CFASSFile_Private.h"
#include "CFASSFileStyleCollection.h"
#include "CFASSFileStyleCollection_Private.h"
#include "CFPointerArray.h"
#include "CFStringTool.h"
#include "CFASSFileStyle.h"
#include "CFASSFileStyle_Private.h"
#include "CFUnicodeStringArray.h"
#include "CFException.h"
#include "CFEnumerator.h"
#include "CFASSFileControl.h"
#include "CFASSFileParsingResult.h"
#include "CFASSFileParsingResult_Macro.h"
#include "CFMacro.h"

CLANG_DIAGNOSTIC_PUSH
CLANG_DIAGNOSTIC_IGNORE_NONNULL

struct CFASSFileStyleCollection
{
    CFPointerArrayRef styleCollection;
    CFASSFileRef registeredFile;            // don't have ownership
};

static wchar_t const * const CFASSFileStyleCollectionDiscription = L"Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\n";

CFEnumeratorRef CFASSFileStyleCollectionCreateEnumerator(CFASSFileStyleCollectionRef styleCollection)
{
    if(styleCollection == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileStyleCollection NULL CreateEnumerator");
    return CFEnumeratorCreateFromArray(styleCollection->styleCollection);
}

void CFASSFileStyleCollectionMakeChange(CFASSFileStyleCollectionRef styleCollection, CFASSFileChangeRef change)
{
    if(styleCollection == NULL || change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileStyleCollection %p MakeChange %p", styleCollection, change);
    CFEnumeratorRef enumerator = CFASSFileStyleCollectionCreateEnumerator(styleCollection);
    CFASSFileStyleRef eachStyle;
    while ((eachStyle = CFEnumeratorNextObject(enumerator)) != NULL)
        CFASSFileStyleMakeChange(eachStyle, change);
    CFEnumeratorDestory(enumerator);
}

CFASSFileStyleCollectionRef CFASSFileStyleCollectionCopy(CFASSFileStyleCollectionRef styleCollection)
{
    CFASSFileStyleCollectionRef result;
    if((result = malloc(sizeof(struct CFASSFileStyleCollection))) != NULL)
    {
        if((result->styleCollection = CFPointerArrayCreateEmpty()) != NULL)
        {
            size_t arrayCount = CFPointerArrayGetCount(styleCollection->styleCollection);
            CFASSFileStyleRef eachStyle;
            bool copyCheck = true;
            for(size_t index = 0; index < arrayCount && copyCheck; index++)
            {
                eachStyle = CFASSFileStyleCopy((CFASSFileStyleRef)CFPointerArrayGetPointerAtIndex(styleCollection->styleCollection, index));
                if(eachStyle == NULL)
                    copyCheck = false;
                else
                    CFPointerArrayAddPointer(result->styleCollection, eachStyle, false);
            }
            if(copyCheck)
            {
                result->registeredFile = NULL;
                return result;
            }
            arrayCount = CFPointerArrayGetCount(result->styleCollection);
            for(size_t index = 0; index<arrayCount; index++)
                CFASSFileStyleDestory((CFASSFileStyleRef)CFPointerArrayGetPointerAtIndex(result->styleCollection, index));
            CFPointerArrayDestory(result->styleCollection);
        }
        free(result);
    }
    return NULL;
}

wchar_t *CFASSFileStyleCollectionAllocateFileContent(CFASSFileStyleCollectionRef styleCollection)
{
    size_t styleAmount = CFPointerArrayGetCount(styleCollection->styleCollection);
    CFUnicodeStringArrayRef stringArray;
    if((stringArray = CFUnicodeStringArrayCreateEmpty()) != NULL)
    {
        const wchar_t *eachStyle;
        bool isAllocatingSuccess = true;
        for(size_t count = 1; isAllocatingSuccess && count <= styleAmount; count++)
        {
            if((eachStyle =
                CFASSFileStyleAllocateFileContent
                ((CFASSFileStyleRef)CFPointerArrayGetPointerAtIndex(styleCollection->styleCollection,
                                                                    count - 1))) != NULL)
                CFUnicodeStringArrayAddString(stringArray, eachStyle, true);
            else
                isAllocatingSuccess = false;
        }
        if(isAllocatingSuccess)
        {
            size_t styleCollectionStringLength = wcslen(L"[V4+ Styles]\n") + wcslen(CFASSFileStyleCollectionDiscription);
            for(size_t count = 1; count <= styleAmount; count++)
                styleCollectionStringLength += wcslen(CFUnicodeStringArrayGetStringAtIndex(stringArray, count - 1));
            
            wchar_t *result;
            if((result = malloc(sizeof(wchar_t)*(styleCollectionStringLength + 1))) != NULL)
            {
                wchar_t *beginPoint = result;
                size_t eachLength;
                
                eachLength = wcslen(L"[V4+ Styles]\n");
                wmemcpy(beginPoint, L"[V4+ Styles]\n", eachLength);
                beginPoint += eachLength;
                
                eachLength = wcslen(CFASSFileStyleCollectionDiscription);
                wmemcpy(beginPoint, CFASSFileStyleCollectionDiscription, eachLength);
                beginPoint += eachLength;
                
                for(size_t count = 1; count <= styleAmount; count++)
                {
                    eachStyle = CFUnicodeStringArrayGetStringAtIndex(stringArray, count - 1);
                    eachLength = wcslen(eachStyle);
                    wmemcpy(beginPoint, eachStyle, eachLength);
                    beginPoint += eachLength;
                }
                beginPoint[0] = L'\0';
                
                DEBUG_ASSERT(styleCollectionStringLength == beginPoint - result);
                
                CFUnicodeStringArrayDestory(stringArray);
                return result;
            }
        }
        CFUnicodeStringArrayDestory(stringArray);
    }
    return NULL;
}

void CFASSFileStyleCollectionDestory(CFASSFileStyleCollectionRef styleCollection)
{
    if(styleCollection == NULL) return;
    size_t arrayCount = CFPointerArrayGetCount(styleCollection->styleCollection);
    for(size_t index = 0; index<arrayCount; index++)
        CFASSFileStyleDestory((CFASSFileStyleRef)CFPointerArrayGetPointerAtIndex(styleCollection->styleCollection, index));
    CFPointerArrayDestory(styleCollection->styleCollection);
    free(styleCollection);
}

CFASSFileStyleCollectionRef CFASSFileStyleCollectionCreateWithUnicodeFileContent(const wchar_t * _Nonnull content,
                                                                                 CFASSFileParsingResultRef _Nonnull parsingResult) {
    DEBUG_ASSERT(content != NULL && parsingResult != NULL);
    if(content == NULL || parsingResult == NULL) return NULL;
    
    CFASSFileStyleCollectionRef result;
    if((result = malloc(sizeof(struct CFASSFileStyleCollection))) != NULL) {
        if((result->styleCollection = CFPointerArrayCreateEmpty()) != NULL) {
            const wchar_t *beginPoint, *endPoint;
            if((beginPoint = wcsstr(content, L"\n[V4+ Styles]")) != NULL &&
               (endPoint = wcsstr(beginPoint, L"\n[Events]\n")) != NULL) {
                bool isFormatCorrect = true;
                beginPoint += wcslen(L"\n[V4+ Styles]");
                
                // beginPoint: next character after [V4+ Styles], maybe overlapped with endPoint
                // endPoint '\n' Just before [Events]
                
                CFASSFileControlLevel level = CFASSFileControlGetLevel();
                
                CFASSFileStyleRef eachStyle;
                while(isFormatCorrect && (beginPoint = CF_wcsstr_with_end_point(beginPoint, L"\nStyle:", endPoint)) != NULL) {
                    beginPoint++;
                    if((eachStyle = CFASSFileStyleCreateWithString(beginPoint, parsingResult)) == NULL) {
                        
                        if(!(level & CFASSFileControlLevelIgnore)) isFormatCorrect = false; // error handling
                        PR_ERROR(beginPoint, L"CFASSFileStyle create failed");
                        
                    }
                    else CFPointerArrayAddPointer(result->styleCollection, eachStyle, false);
                }
                
                if(isFormatCorrect)
                {
                    result->registeredFile = NULL;
                    return result;
                } else PR_ERROR(beginPoint, L"CFASSFileStyleCollection create failed");
            } else PR_ERROR(content, L"matching [V4+ Style] & [Events] failed");
            size_t arrayCount = CFPointerArrayGetCount(result->styleCollection);
            for(size_t index = 0; index < arrayCount; index++)
                CFASSFileStyleDestory((CFASSFileStyleRef)CFPointerArrayGetPointerAtIndex(result->styleCollection, index));
            CFPointerArrayDestory(result->styleCollection);
        } else PR_INFO(NULL, L"CFASSFileStyleCollection styleCollection intialization failed");
        free(result);
    } else PR_INFO(NULL, L"CFASSFileStyleCollection allocation failed");
    return NULL;
}

int CFASSFileStyleCollectionRegisterAssociationwithFile(CFASSFileStyleCollectionRef collection, CFASSFileRef assFile)
{
    if(collection->registeredFile == NULL)
    {
        collection->registeredFile = assFile;
        return 0;
    }
    else
        return -1;
}

CLANG_DIAGNOSTIC_POP

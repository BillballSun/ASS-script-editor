//
//  CFASSFileDialogueCollection.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <wchar.h>
#include <stdlib.h>
#include "CFASSFile.h"
#include "CFASSFile_Private.h"
#include "CFASSFileDialogueCollection.h"
#include "CFASSFileDialogueCollection_Private.h"
#include "CFASSFileDialogue.h"
#include "CFASSFileDialogue_Private.h"
#include "CFPointerArray.h"
#include "CFEnumerator.h"
#include "CFException.h"
#include "CFASSFileControl.h"
#include "CFASSFileParsingResult.h"
#include "CFASSFileParsingResult_Macro.h"
#include "CFMacro.h"

CLANG_DIAGNOSTIC_PUSH
CLANG_DIAGNOSTIC_IGNORE_NONNULL

struct CFASSFileDialogueCollection
{
    CFPointerArrayRef collectionArray;
    CFASSFileRef registeredFile;
};

void CFASSFileDialogueCollectionMakeChange(CFASSFileDialogueCollectionRef dialogueCollection, CFASSFileChangeRef change)
{
    if(dialogueCollection == NULL || change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueCollection %p MakeChange %p", dialogueCollection, change);
    CFEnumeratorRef enumerator = CFASSFileDialogueCollectionCreateEnumerator(dialogueCollection);
    CFASSFileDialogueRef eachDialogue;
    while ((eachDialogue = CFEnumeratorNextObject(enumerator)) != NULL)
        CFASSFileDialogueMakeChange(eachDialogue, change);
    CFEnumeratorDestory(enumerator);
}

CFASSFileDialogueCollectionRef CFASSFileDialogueCollectionCopy(CFASSFileDialogueCollectionRef dialogueCollection)
{
    CFASSFileDialogueCollectionRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueCollection))) != NULL)
    {
        result->registeredFile = NULL;
        if((result->collectionArray = CFPointerArrayCreateEmpty()) != NULL)
        {
            size_t arrayCount = CFPointerArrayGetCount(dialogueCollection->collectionArray);
            bool copyCheck = true;
            CFASSFileDialogueRef eachDialogue;
            for(size_t index = 0; index < arrayCount && copyCheck; index++)
            {
                eachDialogue = CFASSFileDialogueCopy
                ((CFASSFileDialogueRef)CFPointerArrayGetPointerAtIndex(dialogueCollection->collectionArray, index));
                if(eachDialogue == NULL)
                    copyCheck = false;
                else
                    CFPointerArrayAddPointer(result->collectionArray, eachDialogue, false);
            }
            if(copyCheck)
                return result;
            arrayCount = CFPointerArrayGetCount(result->collectionArray);
            for(size_t index = 0; index < arrayCount; index++)
                CFASSFileDialogueDestory
                ((CFASSFileDialogueRef)CFPointerArrayGetPointerAtIndex(result->collectionArray, index));
            CFPointerArrayDestory(result->collectionArray);
        }
        free(result);
    }
    return NULL;
}

CFEnumeratorRef CFASSFileDialogueCollectionCreateEnumerator(CFASSFileDialogueCollectionRef dialogueCollection)
{
    if(dialogueCollection == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueCollection NULL CreateEnumerator");
    return CFEnumeratorCreateFromArray(dialogueCollection->collectionArray);
}

void CFASSFileDialogueCollectionDestory(CFASSFileDialogueCollectionRef dialogueCollection)
{
    if(dialogueCollection == NULL) return;
    size_t arrayCount = CFPointerArrayGetCount(dialogueCollection->collectionArray);
    for(size_t index = 0; index < arrayCount; index++)
        CFASSFileDialogueDestory(CFPointerArrayGetPointerAtIndex(dialogueCollection->collectionArray, index));
    CFPointerArrayDestory(dialogueCollection->collectionArray);
    free(dialogueCollection);
}

int CFASSFileDialogueCollectionRegisterAssociationwithFile(CFASSFileDialogueCollectionRef collection, CFASSFileRef assFile)
{
    if(collection->registeredFile == NULL)
    {
        collection->registeredFile = assFile;
        return 0;
    }
    else
        return -1;
}

CFASSFileDialogueCollectionRef CFASSFileDialogueCollectionCreateWithUnicodeFileContent(const wchar_t * _Nonnull content,
                                                                                       CFASSFileParsingResultRef _Nonnull parsingResult) {
    DEBUG_ASSERT(content != NULL && parsingResult != NULL);
    if(content == NULL || parsingResult == NULL) return NULL;
    
    wchar_t *searchPoint;
    if((searchPoint = wcsstr(content, L"\n[Events]")) != NULL)
    {
        searchPoint = wcsstr(searchPoint, L"\nDialogue:");
        if(searchPoint == NULL) return NULL;
        
        CFASSFileDialogueCollectionRef result;
        if((result = malloc(sizeof(struct CFASSFileDialogueCollection))) != NULL)
        {
            if((result->collectionArray = CFPointerArrayCreateEmpty()) != NULL)
            {
                CFASSFileControlLevel controlLevel = CFASSFileControlGetLevel();
                size_t skipLength = wcslen(L"\nDialogue:");
                CFASSFileDialogueRef eachDialogue;
                bool formatCheck = true;
                while(formatCheck && (searchPoint = wcsstr(searchPoint, L"\nDialogue:")) != NULL)
                {
                    eachDialogue = CFASSFileDialogueCreateWithString(searchPoint + 1, parsingResult);
                    if(eachDialogue == NULL)
                    {
                        if(!(controlLevel & CFASSFileControlLevelIgnore)) formatCheck = false; // error handling
                        PR_ERROR(searchPoint + 1, L"CFASSFileDialogue create failed");
                    }
                    else CFPointerArrayAddPointer(result->collectionArray, eachDialogue, false);
                    searchPoint += skipLength;
                }
                if(formatCheck) {
                    result->registeredFile = NULL;
                    return result;
                }
                size_t arrayCount = CFPointerArrayGetCount(result->collectionArray);
                for(size_t index = 0; index < arrayCount; index++)
                    CFASSFileDialogueDestory(CFPointerArrayGetPointerAtIndex(result->collectionArray, index));
                CFPointerArrayDestory(result->collectionArray);
            }
            free(result);
        }
    } else PR_ERROR(searchPoint, L"CFASSFileDialogueCollection \"[Events]\" match failed");
    return NULL;
}

wchar_t *CFASSFileDialogueCollectionAllocateFileContent(CFASSFileDialogueCollectionRef dialogueCollection)
{
    size_t stringLength = 0,
    prefixLength = wcslen(L"[Events]\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n");
    
    stringLength += prefixLength;
    
    int temp;
    size_t arrayCount = CFPointerArrayGetCount(dialogueCollection->collectionArray);
    for(size_t index = 0; index<arrayCount; index++)
    {
        temp = CFASSFileDialogueStoreStringResult(CFPointerArrayGetPointerAtIndex(dialogueCollection->collectionArray, index), NULL);
        if(temp == -1) return NULL;
        else stringLength += temp;
    }
    
    wchar_t *result;
    if((result = malloc(sizeof(wchar_t) * (stringLength + 1))) != NULL)
    {
        swprintf(result, stringLength + 1, L"[Events]\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n");
        wchar_t *currentPoint = result + prefixLength;
        bool formatCheck = true;
        for(size_t index = 0; index < arrayCount && formatCheck; index++)
        {
            temp = CFASSFileDialogueStoreStringResult(CFPointerArrayGetPointerAtIndex(dialogueCollection->collectionArray, index), currentPoint);
            if(temp < 0)
                formatCheck = false;
            else
                currentPoint += temp;
        }
        DEBUG_ASSERT(formatCheck && (wcslen(result) == stringLength));
        if(formatCheck) return result;
        free(result);
    }
    return NULL;
}

CLANG_DIAGNOSTIC_POP

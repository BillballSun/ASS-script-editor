//
//  CFASSFileDialogueText.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/10.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <wchar.h>
#include <stdbool.h>
#include "CFASSFileDialogueText.h"
#include "CFASSFileDialogueText_Private.h"
#include "CFASSFileDialogueTextContent.h"
#include "CFASSFileDialogueTextContent_Private.h"
#include "CFPointerArray.h"
#include "CFException.h"
#include "CFEnumerator.h"
#include "CFASSFileChange.h"
#include "CFASSFileChange_Private.h"
#include "CFASSFileParsingResult.h"
#include "CFASSFileParsingResult_Macro.h"
#include "CFASSFileControl.h"
#include "CFMacro.h"

struct CFASSFileDialogueText
{
    CFPointerArrayRef contentArray;     // Can't be empty, must at least have CFASSFileDialogueTextContentCreateEmptyString();
};

static CFPointerArrayRef _Nullable CFASSFileDialogueTextCreateArrayForSeperateRawString(const wchar_t * _Nonnull string);

CLANG_DIAGNOSTIC_PUSH
CLANG_DIAGNOSTIC_IGNORE_NONNULL

void CFASSFileDialogueTextMakeChange(CFASSFileDialogueTextRef dialogueText, CFASSFileChangeRef change)
{
    if(dialogueText == NULL || change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueText %p MakeChange %p", dialogueText, change);
    CFEnumeratorRef enumerator;
    CFASSFileDialogueTextContentRef eachTextContent = NULL;
    
    if(change->type & CFASSFileChangeTypeDiscardAllOverride)
    {
		bool checkPoint;
		size_t index, arrayCount;
		do
		{
			checkPoint = false;
			arrayCount = CFPointerArrayGetCount(dialogueText->contentArray);
			for (index = 0; index < arrayCount; index++)
			{
				eachTextContent = CFPointerArrayGetPointerAtIndex(dialogueText->contentArray, index);
				if (CFASSFileDialogueTextContentGetType(eachTextContent) == CFASSFileDialogueTextContentTypeOverride)
				{
					checkPoint = true;
					break;
				}
			}
			if (checkPoint)
			{
				CFPointerArrayRemovePointerAtIndex(dialogueText->contentArray, index, false);
				CFASSFileDialogueTextContentDestory(eachTextContent);
			}
		} while (checkPoint);
        if(CFPointerArrayGetCount(dialogueText->contentArray) == 0)
            CFPointerArrayAddPointer(dialogueText->contentArray, CFASSFileDialogueTextContentCreateEmptyString(), false);
    }
    enumerator = CFASSFileDialogueTextCreateEnumerator(dialogueText);
    while ((eachTextContent = CFEnumeratorNextObject(enumerator)) != NULL)
        CFASSFileDialogueTextContentMakeChange(eachTextContent, change);
    CFEnumeratorDestory(enumerator);
}

CFEnumeratorRef CFASSFileDialogueTextCreateEnumerator(CFASSFileDialogueTextRef dialogueText)
{
    if(dialogueText == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueText NULL CreateEnumerator");
    return CFEnumeratorCreateFromArray(dialogueText->contentArray);
}

void CFASSFileDialogueTextAddContent(CFASSFileDialogueTextRef dialogueText, CFASSFileDialogueTextContentRef content)
{
    if(dialogueText == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueText NULL AddContent");
        return;
    }
    if(content == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextAddContent NULL");
        return;
    }
    CFPointerArrayAddPointer(dialogueText->contentArray, content, false);
}

CFASSFileDialogueTextRef CFASSFileDialogueTextCopy(CFASSFileDialogueTextRef dialogueText)
{
    CFASSFileDialogueTextRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueText))) != NULL)
    {
        if((result->contentArray = CFPointerArrayCreateEmpty()) != NULL)
        {
            size_t arrayCount = CFPointerArrayGetCount(dialogueText->contentArray);
            bool copyCheck = true;
            CFASSFileDialogueTextContentRef eachContent;
            for (size_t index = 0; index<arrayCount && copyCheck; index++) {
                eachContent = CFASSFileDialogueTextContentCopy
                ((CFASSFileDialogueTextContentRef)CFPointerArrayGetPointerAtIndex(dialogueText->contentArray, index));
                if(eachContent == NULL)
                    copyCheck = false;
                else
                    CFPointerArrayAddPointer(result->contentArray, eachContent, false);
            }
            if(copyCheck)
                return result;
            arrayCount = CFPointerArrayGetCount(result->contentArray);
            for (size_t index = 0; index<arrayCount; index++)
                CFASSFileDialogueTextContentDestory
                ((CFASSFileDialogueTextContentRef)CFPointerArrayGetPointerAtIndex(result->contentArray, index));
            CFPointerArrayDestory(result->contentArray);
        }
        free(result);
    }
    return NULL;
}

int CFASSFileDialogueTextStoreStringResult(CFASSFileDialogueTextRef _Nonnull text, wchar_t * _Nullable targetPoint)
{
    DEBUG_ASSERT(text != NULL); if(text == NULL) return 0;
    
    int result = 0, temp;
    size_t arrayCount = CFPointerArrayGetCount(text->contentArray);
    for(size_t index = 0; index < arrayCount; index++)
    {
        temp = CFASSFileDialogueTextContentStoreStringResult
        ((CFASSFileDialogueTextContentRef)CFPointerArrayGetPointerAtIndex(text->contentArray, index), targetPoint);
        if(temp == -1) return -1;
        if(targetPoint != NULL)
            targetPoint += temp;
        result += temp;
    }
    if(targetPoint != NULL)
    {
        targetPoint[0] = L'\n';
        targetPoint[1] = L'\0';
    }
    return result + 1;
}

CFASSFileDialogueTextRef CFASSFileDialogueTextCreateWithString(const wchar_t * _Nonnull source,
                                                               CFASSFileParsingResultRef _Nonnull parsingResult)
{
    DEBUG_ASSERT(source != NULL && parsingResult != NULL);
    if(source == NULL || parsingResult == NULL) return NULL;
    
    CFASSFileDialogueTextRef result;
    CFASSFileDialogueTextContentRef eachContent;
    
    const wchar_t *contentEndPoint;
    contentEndPoint = wcsstr(source, L"\n");                                  // contentEndPoint to '\n'
    if(contentEndPoint == NULL) contentEndPoint = source + wcslen(source);    // contentEndPoint to '\0'
    
    if((result = malloc(sizeof(struct CFASSFileDialogueText))) != NULL)
    {
        if((result->contentArray = CFPointerArrayCreateEmpty()) != NULL)
        {
            if(contentEndPoint == source)
            {
                if((eachContent = CFASSFileDialogueTextContentCreateEmptyString()) != NULL)
                {
                    CFPointerArrayAddPointer(result->contentArray, eachContent, false);
                    return result;
                } else PR_INFO(NULL, L"CFASSFileDialogueTextContent create empty failed");
            }
            else
            {
                CFPointerArrayRef seperationArray;
                if((seperationArray = CFASSFileDialogueTextCreateArrayForSeperateRawString(source)) != NULL) {
                    size_t count = CFPointerArrayGetCount(seperationArray); DEBUG_ASSERT(count != 0 && count % 2 == 0);
                    if(count == 0 || count % 2 != 0) {    // impossible
                        if((eachContent = CFASSFileDialogueTextContentCreateEmptyString()) != NULL)
                        {
                            CFPointerArrayAddPointer(result->contentArray, eachContent, false);
                            CFPointerArrayDestory(seperationArray);
                            return result;
                        } else PR_INFO(NULL, L"CFASSFileDialogueTextContent create empty string failed");
                        DEBUG_POINT // create empty string failed ?
                    }
                    else {
                        CFASSFileControlLevel controlLevel = CFASSFileControlGetLevel();
                        
                        bool formatCheck = true;
                        
                        for(size_t index = 0; index < count; index += 2) {
                            const wchar_t *beginPoint = CFPointerArrayGetPointerAtIndex(seperationArray, index);
                            const wchar_t *endPoint = CFPointerArrayGetPointerAtIndex(seperationArray, index + 1);
                            DEBUG_ASSERT(endPoint >= beginPoint && endPoint[0] != L'\n' && endPoint[0] != L'\0');
                            CFASSFileDialogueTextContentType contentType;
                            if(beginPoint[0] == L'{') contentType = CFASSFileDialogueTextContentTypeOverride;
                            else contentType = CFASSFileDialogueTextContentTypeText;
                            if((eachContent = CFASSFileDialogueTextContentCreateWithString(contentType, beginPoint, endPoint, parsingResult)) != NULL)
                                CFPointerArrayAddPointer(result->contentArray, eachContent, false);
                            else {
                                PR_ERROR(beginPoint, L"CFASSFileDialogueTextContent create failed");
                                if(!(controlLevel & CFASSFileControlLevelIgnore)) {
                                    formatCheck = false;
                                    break;
                                }
                            }
                        }
                        
                        if(formatCheck) {
                            CFPointerArrayDestory(seperationArray);
                            return result;
                        }
                    }
                    
                    CFPointerArrayDestory(seperationArray);
                } else PR_ERROR(source, L"CFASSFileDialogueText content format error");
            }
            
            size_t arrayCount = CFPointerArrayGetCount(result->contentArray);
            for(unsigned int index = 0; index < arrayCount; index++)
                CFASSFileDialogueTextContentDestory((CFASSFileDialogueTextContentRef)
                                                    CFPointerArrayGetPointerAtIndex(result->contentArray, index));
            
            CFPointerArrayDestory(result->contentArray);
        } else PR_INFO(NULL, L"CFASSFileDialogueText create contentArray failed");
        free(result);
    } else PR_INFO(NULL, L"CFASSFileDialogueText allocation failed");
    return NULL;
}

void CFASSFileDialogueTextDestory(CFASSFileDialogueTextRef dialogueText)
{
    size_t arrayCount = CFPointerArrayGetCount(dialogueText->contentArray);   
    for(unsigned int index = 0; index<arrayCount; index++)
        CFASSFileDialogueTextContentDestory((CFASSFileDialogueTextContentRef)CFPointerArrayGetPointerAtIndex(dialogueText->contentArray, index));
    CFPointerArrayDestory(dialogueText->contentArray);
    free(dialogueText);
}

/*!
 @function CFASSFileDialogueTextCreateArrayForSeperateRawString
 @abstract seperate raw dialogueText string into override or plainText
 @discussion remind of escape sequence and overrides
             escape sequence '\\n' '\N' '\h' '\{'
             override {...} disable all escape sequence
 @param string content begin, may begin '\0' or '\\n'
        must end with '\0' or '\\n'
 @return CFPointerArray object if seperation complete, or NULL if error happen
         its content should be doubled, as one for beginPoint, one for endPoint
         just enclosing each content part
 */
static CFPointerArrayRef _Nullable CFASSFileDialogueTextCreateArrayForSeperateRawString(const wchar_t * _Nonnull string) {
    DEBUG_ASSERT(string != NULL); if(string == NULL) return NULL;
    CFPointerArrayRef array;
    if((array = CFPointerArrayCreateEmpty()) != NULL) {
        const wchar_t *searchPoint = string;
        bool formatCheck = true;
        while(searchPoint[0] != L'\n' && searchPoint[0] != L'\0') {
            if(searchPoint[0] == L'{') {
                const wchar_t *endPoint = searchPoint;      // endPoint to '}' or just error happened to '\0' or '\n'
                while(endPoint[0] != L'}' && endPoint[0] != L'\0' && endPoint[0] != L'\0') endPoint++;
                if(endPoint[0] != L'}') {
                    formatCheck = false;
                    break;  // format match failure
                }
                CFPointerArrayAddPointer(array, (void *)searchPoint, false);
                CFPointerArrayAddPointer(array, (void *)endPoint, false);
                searchPoint = endPoint + 1;
            }
            else {
                const wchar_t *endPoint = searchPoint;
                
                // advance end to the '\0' and '\n' or just another override '{' begin
                // note: don't forget escape sequence '\{'
                DEBUG_ASSERT(endPoint[0] != L'{');
                LOOP {
                    if(endPoint[0] == L'\\' && endPoint[1] == L'{')
                        endPoint++;
                    else if(endPoint[0] == L'{' || endPoint[0] == L'\0' || endPoint[0] == L'\n')
                        break;
                    endPoint++;
                }
                DEBUG_ASSERT(endPoint > searchPoint);
                CFPointerArrayAddPointer(array, (void *)searchPoint, false);
                CFPointerArrayAddPointer(array, (void *)(endPoint - 1), false);
                searchPoint = endPoint;
            }
        }
        if(formatCheck) return array;
        CFPointerArrayDestory(array);
    }
    return NULL;
}

CLANG_DIAGNOSTIC_POP

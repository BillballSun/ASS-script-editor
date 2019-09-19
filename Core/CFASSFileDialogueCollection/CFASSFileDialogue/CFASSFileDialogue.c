//
//  CFASSFileDialogue.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/10.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <wchar.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "CFASSFileDialogue.h"
#include "CFASSFileDialogue_Private.h"
#include "CFASSFileDialogueEffect.h"
#include "CFASSFileDialogueEffect_Private.h"
#include "CFASSFileDialogueText.h"
#include "CFASSFileDialogueText_Private.h"
#include "CFStringTool.h"
#include "CFException.h"
#include "CFASSFileChange.h"
#include "CFASSFileChange_Private.h"
#include "CFASSFileControl.h"
#include "CFASSFileParsingResult.h"
#include "CFASSFileParsingResult_Macro.h"
#include "CFMacro.h"

//[Events]
//Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text
//Dialogue: 0,0:03:54.09,0:04:04.04,zhizuorenyuan,NTP,0,0,0,,{\fad(150,150)}翻译

typedef struct CFASSFileDialogueTime {
    unsigned int hour, min, sec, hundredths;
} CFASSFileDialogueTime;

struct CFASSFileDialogue
{
    unsigned int layer;                     // Subtitles having different layer number will be ignore during the collusion detection.
                                            // Higher numberer layers will be drawn over the lower numbers.
    CFASSFileDialogueTime start, end;       // 0:00:00.00 format ie. Hrs:Mins:Secs:hundredths.
    wchar_t *style;                         // name for style, NULL for empty
    wchar_t *name;                          // name for speaker, NULL for empty
    unsigned int marginL, marginR, marginV;
    CFASSFileDialogueEffectRef effect;      // if not have, assign NULL
    CFASSFileDialogueTextRef text;          // if not hace, don't assign NULL
};

/*!
 @function CFASSFileDialogueTimeFormatCorrect
 @abstract Correct a CFASSFileDialogueTime
 @return if format is already correct, return true
         otherwise correct it and return false
 */
static bool CFASSFileDialogueTimeFormatCorrect(CFASSFileDialogueTime * _Nonnull time);

static CFASSFileDialogueTime CFASSFileDialogueTimeOffset(CFASSFileDialogueTime fromTime, long hundredths);

CLANG_DIAGNOSTIC_PUSH
CLANG_DIAGNOSTIC_IGNORE_NONNULL

void CFASSFileDialogueMakeChange(CFASSFileDialogueRef dialogue, CFASSFileChangeRef change)
{
    if(dialogue == NULL || change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogue %p MakeChange %p", dialogue, change);
    if(change->type & CFASSFileChangeTypeTimeOffset)
    {
        dialogue->start = CFASSFileDialogueTimeOffset(dialogue->start, change->timeOffset.hundredths);
        dialogue->end = CFASSFileDialogueTimeOffset(dialogue->end, change->timeOffset.hundredths);
    }
    if(dialogue->effect != NULL)
        CFASSFileDialogueEffectMakeChange(dialogue->effect, change);
    CFASSFileDialogueTextMakeChange(dialogue->text, change);
}

static CFASSFileDialogueTime CFASSFileDialogueTimeOffset(CFASSFileDialogueTime fromTime, long hundredths)
{
    struct tm temp;
    temp.tm_hour = fromTime.hour;
    temp.tm_min = fromTime.min;
    temp.tm_sec = fromTime.sec;
    temp.tm_year = 0;
    hundredths += fromTime.hundredths;
    long absoluteOffset = labs(hundredths)/100;
    // hundredths %= 100;
    if(hundredths>=0)
        temp.tm_sec += absoluteOffset;
    else
        temp.tm_sec -= absoluteOffset;
    if(hundredths<0)
    {
        temp.tm_sec--;
        hundredths = labs(hundredths);
        hundredths %= 100;
        hundredths = 100 - hundredths;
    }
    else
        hundredths %= 100;
    mktime(&temp);
    return (CFASSFileDialogueTime){.hour = temp.tm_hour, .min = temp.tm_min, .sec = temp.tm_sec, .hundredths = (unsigned int)hundredths};
}

CFASSFileDialogueRef CFASSFileDialogueCopy(CFASSFileDialogueRef dialogue)
{
    CFASSFileDialogueRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogue))) != NULL)
    {
        *result = *dialogue;
        if(dialogue->style == NULL || (result->style = CF_Dump_wchar_string(dialogue->style)) != NULL)
        {
            if(dialogue->name == NULL || (result->name = CF_Dump_wchar_string(dialogue->name)) != NULL)
            {
                if(dialogue->effect == NULL || (result->effect = CFASSFileDialogueEffectCopy(dialogue->effect)) != NULL)
                {
                    if((result->text = CFASSFileDialogueTextCopy(dialogue->text)) != NULL)
                        return result;
                    if(result->effect != NULL) CFASSFileDialogueEffectDestory(result->effect);
                }
                if(result->name != NULL) free(result->name);
            }
            if(result->style != NULL) free(result->style);
        }
        free(result);
    }
    return NULL;
}

#pragma mark - Content parsing [解析]

CFASSFileDialogueRef CFASSFileDialogueCreateWithString(const wchar_t * _Nonnull source, CFASSFileParsingResultRef _Nonnull parsingResult)
{
    DEBUG_ASSERT(source != NULL && parsingResult != NULL);
    if(source == NULL || parsingResult == NULL) return NULL;
    
    unsigned int layer;
    CFASSFileDialogueTime start, end;
    wchar_t *style, *name;
    unsigned int marginL, marginR, marginV;
    CFASSFileDialogueEffectRef effect;      // if not have, assign NULL
    CFASSFileDialogueTextRef text;          // if not hace assign NULL
    int temp, scanAmount = 0;
    wchar_t timeFormat1[2], timeFormat2[2];
    
#pragma mark time
    
    // scanf only skip blank character if it is %x format, and counted into %n
    temp = swscanf(source, L"Dialogue:%u,%u:%u:%u%1l[:.]%u,%u:%u:%u%1l[:.]%u,%n",
                   &layer,
                   &start.hour, &start.min, &start.sec, timeFormat1, &start.hundredths,
                     &end.hour,   &end.min,   &end.sec, timeFormat2,   &end.hundredths, &scanAmount);
    if(temp != 11                                                           ||
       scanAmount <= 0                    /* mostly impossible */           ||
       source[scanAmount - 1] != L','     /* just make access valid */      ||
       source[scanAmount] == L'\0'                                          ||
       source[scanAmount] == L'\n') {
        PR_ERROR(source, L"CFASSFileDialogue parsing time format failed, patten "
                          "\"Dialogue:%%u,%%u:%%u:%%u%%1l[:.]%%u,%%u:%%u:%%u%%1l[:.]%%u,\"");
        return NULL;
    }
    
    CFASSFileControlLevel controlLevel = CFASSFileControlGetLevel();
    
    if(*timeFormat1 == L':')
    {
        if(start.hundredths > 59) {
            if(controlLevel & CFASSFileControlLevelIgnore) {
                PR_WARN(source, L"CFASSFileDialogue beginTime format patten hour:min:sec:ms ms larger than 59, auto-correct to 59");
                start.hundredths = 59;
            }
            else {
                PR_ERROR(source, L"CFASSFileDialogue beginTime format patten hour:min:sec:ms ms larger than 59");
                return NULL;
            }
        }
        start.hundredths *= (10.0 / 3);
        if(start.hundredths > 99) start.hundredths = 99;
    }
    
    if(*timeFormat2 == L':')
    {
        if(end.hundredths > 59) {
            if(controlLevel & CFASSFileControlLevelIgnore) {
                PR_WARN(source, L"CFASSFileDialogue endTime format patten hour:min:sec:ms ms larger than 59, auto-correct to 59");
                end.hundredths = 59;
            }
            else {
                PR_ERROR(source, L"CFASSFileDialogue endTime format patten hour:min:sec:ms ms larger than 59");
                return NULL;
            }
        }
        end.hundredths *= (10.0 / 3);
        if(end.hundredths > 99) end.hundredths = 99;
    }
    
    bool timeFormatCheck1 = CFASSFileDialogueTimeFormatCorrect(&start);
    bool timeFormatCheck2 = CFASSFileDialogueTimeFormatCorrect(&end);
    
    if((!(controlLevel & CFASSFileControlLevelIgnore)) && (!timeFormatCheck1 || !timeFormatCheck2)) {
        PR_ERROR(source, L"CFASSFileDialogue time format check failed");
        return NULL;
    }
    
    source += scanAmount;
    const wchar_t *tokenEnd;
    tokenEnd = source;
    
#pragma mark style
    
    // tokenEnd, at most left is source
    // find to set tokenEnd to the token ','
    // may search to the end of line or string
    while(*tokenEnd != L',' && *tokenEnd != L'\0' && *tokenEnd != L'\n') tokenEnd++;
    if(*tokenEnd != L',') {
        PR_ERROR(source, L"CFASSFileDialogue find style name token failed");
        return NULL;
    }
    tokenEnd--;
    if(tokenEnd < source) {
        PR_WARN(source, L"CFASSFileDialogue style name not found");
        style = NULL;
    }
    else
    {
        if((style = malloc(sizeof(wchar_t) * (tokenEnd - source + 1 + 1))) == NULL) {
            PR_INFO(NULL, L"CFASSFileDialogue allocation for style name failed");
            return NULL;
        }
        wmemcpy(style, source, tokenEnd - source + 1);
        style[tokenEnd - source + 1] = L'\0';
    }
    source = tokenEnd + 2;  // when searched tokenEnd is at ',' but tokenEnd-- backward one
    tokenEnd = source;
    
#pragma mark character name
    
    while(*tokenEnd != L',' && *tokenEnd != L'\0' && *tokenEnd != L'\n') tokenEnd++;
    if(*tokenEnd != L',') {
        PR_ERROR(source, L"CFASSFileDialogue find character name token failed");
        return NULL;
    }
    tokenEnd--;
    if(tokenEnd < source)
        name = NULL;        // mostly acceptable
    else
    {
        if((name = malloc(sizeof(wchar_t) * (tokenEnd - source + 1 + 1))) == NULL)
        {
            PR_INFO(NULL, L"CFASSFileDialogue allocation for character name failed");
            if(style != NULL) free(style);
            return NULL;
        }
        wmemcpy(name, source, tokenEnd - source + 1);
        name[tokenEnd - source + 1] = L'\0';
    }
    source  = tokenEnd + 2;
    
#pragma mark marginL marginR marginV
    
    scanAmount = 0;
    
    temp = swscanf(source, L"%u,%u,%u,%n", &marginL, &marginR, &marginV, &scanAmount);
    if(temp != 3                                ||
       scanAmount <= 0                          ||
       source[scanAmount - 1] != L','           ||
       source[scanAmount] == L'\0'              ||
       source[scanAmount] == L'\n') {
        PR_ERROR(source, L"CFASSFileDialogue match marginL marginR marginV failed, patten \"%%u,%%u,%%u,\"");
        if(style != NULL) free(style);
        if(name != NULL) free(name);
        return NULL;
    }
    source += scanAmount;
    tokenEnd = source;
    
#pragma mark effect
    
    while(*tokenEnd != L',' && *tokenEnd != L'\0' && *tokenEnd != L'\n') tokenEnd++;
    if(*tokenEnd != L',')
    {
        PR_ERROR(source, L"CFASSFileDialogue find effect token failed");
        if(style != NULL) free(style);
        if(name != NULL) free(name);
        return NULL;
    }
    tokenEnd--;
    if(tokenEnd < source)
        effect = NULL;          // mostly accpetable
    else
    {
        effect = CFASSFileDialogueEffectCreateWithString(source, tokenEnd, parsingResult);
        if(effect == NULL && (controlLevel & CFASSFileControlLevelIgnore))
        {
            PR_ERROR(source, L"CFASSFileDialogue effect create failed");
            if(style != NULL) free(style);
            if(name != NULL) free(name);
            return NULL;
        }
    }
    source = tokenEnd + 2;
    
#pragma mark dialogue text
    
    // source may point to '\0' or '\n' but just point to raw content
    text = CFASSFileDialogueTextCreateWithString(source, parsingResult);
    if(text == NULL)
    {
        PR_ERROR(source, L"CFASSFileDialogue text create failed");
        if(style != NULL) free(style);
        if(name != NULL) free(name);
        if(effect != NULL) CFASSFileDialogueEffectDestory(effect);
        return NULL;
    }
    
#pragma mark construction
    
    CFASSFileDialogueRef result = malloc(sizeof(struct CFASSFileDialogue));
    if(result == NULL)
    {
        PR_INFO(NULL, L"CFASSFileDialogue allocation failed");
        if(style != NULL) free(style);
        if(name != NULL) free(name);
        if(effect != NULL) CFASSFileDialogueEffectDestory(effect);
        if(text != NULL) CFASSFileDialogueTextDestory(text);
        return NULL;
    }
    result->layer = layer;
    result->start = start;
    result->end = end;
    result->style = style;
    result->name = name;
    result->marginL = marginL;
    result->marginR = marginR;
    result->marginV = marginV;
    result->effect = effect;
    result->text = text;
    return result;
}

static bool CFASSFileDialogueTimeFormatCorrect(CFASSFileDialogueTime * _Nonnull time)
{
    DEBUG_ASSERT(time != NULL); if(time == NULL) return false;
    bool result = true;
    if(time->min > 59) { time->min = 59; result = false; }
    if(time->sec > 59) { time->sec = 59; result = false; }
    if(time->hundredths > 59) { time->hundredths = 59; result = false; }
    return result;
}

int CFASSFileDialogueStoreStringResult(CFASSFileDialogueRef dialogue, wchar_t *targetPoint)
{
    int result = 0, temp;
    if(targetPoint == NULL)
    {
        FILE *fp = tmpfile();
        if(fp == NULL) return -1;
        
        temp = fwprintf(fp, L"Dialogue: %u,%u:%02u:%02u.%02u,%u:%02u:%02u.%02u,",
                        dialogue->layer,
                        dialogue->start.hour, dialogue->start.min, dialogue->start.sec, dialogue->start.hundredths,
                        dialogue->end.hour, dialogue->end.min, dialogue->end.sec, dialogue->end.hundredths);
        if(temp < 0) return -1;
        result += temp;

        temp = fwprintf(fp, L"%ls,%ls,%u,%u,%u,",
                        dialogue->style == NULL?L"":dialogue->style,
                        dialogue->name == NULL?L"":dialogue->name,
                        dialogue->marginL, dialogue->marginR, dialogue->marginV);
        if(temp < 0) return -1;
        result += temp;

        if(dialogue->effect != NULL)
        {
            temp = CFASSFileDialogueEffectStoreStringResult(dialogue->effect, NULL);
            if(temp < 0) return -1;
            result += temp;
        }

        temp = fwprintf(fp, L",");
        if(temp < 0) return -1;
        result += temp;
        
        temp = CFASSFileDialogueTextStoreStringResult(dialogue->text, NULL);
        if(temp < 0) return -1;
        result += temp;
        
        fclose(fp);
    }
    else
    {
        temp = swprintf(targetPoint, SIZE_MAX, L"Dialogue: %u,%u:%02u:%02u.%02u,%u:%02u:%02u.%02u,",
                        dialogue->layer,
                        dialogue->start.hour, dialogue->start.min, dialogue->start.sec, dialogue->start.hundredths,
                        dialogue->end.hour, dialogue->end.min, dialogue->end.sec, dialogue->end.hundredths);
        if(temp < 0) return -1;
        result += temp;
        targetPoint += temp;
        temp = swprintf(targetPoint, SIZE_MAX, L"%ls,%ls,%u,%u,%u,",
                        dialogue->style == NULL?L"":dialogue->style,
                        dialogue->name == NULL?L"":dialogue->name,
                        dialogue->marginL, dialogue->marginR, dialogue->marginV);
        if(temp < 0) return -1;
        result += temp;
        targetPoint += temp;
        
        if(dialogue->effect != NULL)
        {
            temp = CFASSFileDialogueEffectStoreStringResult(dialogue->effect, targetPoint);
            if(temp < 0) return -1;
            result += temp;
            targetPoint += temp;
        }
        
        temp = swprintf(targetPoint, SIZE_MAX, L",");
        if(temp < 0) return -1;
        result += temp;
        targetPoint += temp;
        
        temp = CFASSFileDialogueTextStoreStringResult(dialogue->text, targetPoint);
        if(temp < 0) return -1;
        result += temp;
    }
    return result;
}

//Dialogue: 0,0:03:54.09,0:04:04.04,zhizuorenyuan,NTP,0,0,0,,{\fad(150,150)}翻译

void CFASSFileDialogueDestory(CFASSFileDialogueRef dialogue)
{
    if(dialogue->style != NULL) free(dialogue->style);
    if(dialogue->name != NULL) free(dialogue->name);
    if(dialogue->effect != NULL) CFASSFileDialogueEffectDestory(dialogue->effect);
    CFASSFileDialogueTextDestory(dialogue->text);
    free(dialogue);
}

CLANG_DIAGNOSTIC_POP

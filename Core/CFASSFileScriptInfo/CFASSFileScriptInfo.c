//
//  CFASSFileScriptInfo.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <wchar.h>
#include <stdbool.h>

#include "CFASSFile.h"
#include "CFASSFile_Private.h"
#include "CFASSFileScriptInfo.h"
#include "CFASSFileScriptInfo_Private.h"
#include "CFUnicodeStringArray.h"
#include "CFStringTool.h"
#include "CFException.h"
#include "CFASSFileChange.h"
#include "CFASSFileChange_Private.h"
#include "CFASSFileParsingResult.h"
#include "CFASSFileParsingResult_Macro.h"

#define CFASSFileScriptInfoSearchEachContentName(WSTR) (L"\n" WSTR L":")
#define CFASSFileScriptInfoFileContentSearchInOrder 1
struct CFASSFileScriptInfo {
    CFUnicodeStringArrayRef comment;
    wchar_t *title,                 // This is a description of the script
            *original_script,       // The original author(s) of the script
                                    // If the original author(s) did not provide then <unknown> is auto added
            *original_translation,  // (optional) The original translator of the dialogue
            *original_editing,      // (optional) The original script editor(s)
            *original_timing,       // (optional) whoever timed the original script
            *synch_point,           // (optional) Description of where in the video the script should begin playback
            *script_updated_by,     // (optional) Names of any other subtitling groups who edited the original script
            *update_details,        // (optional) The details of any updates to the original script
            *script_type;           // This is the SSA script format Version eg. V4.00+
    bool is_collisions_normal;       // Normal Reverse
    unsigned int play_res_x, play_res_y;
    wchar_t *play_depth;            // (optional)This is the colour depth used by the script's author(s) when playing the script
    double timer;                   // This is the Timer Speed for the script, as a percentage
                                    // eg. 100.0000 is eactly 100%. It has four digits following the decimal point.
    unsigned int wrap_style;        // (optional)0 - 4
    CFASSFileRef registeredFile;    // don't have ownership
};

CLANG_DIAGNOSTIC_PUSH
CLANG_DIAGNOSTIC_IGNORE_NONNULL

void CFASSFileScriptInfoMakeChange(CFASSFileScriptInfoRef scriptInfo, CFASSFileChangeRef change)
{
    if(scriptInfo == NULL || change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileScriptInfo %p MakeChange %p", scriptInfo, change);
    if(change->type & CFASSFileChangeTypeResolution)
    {
        scriptInfo->play_res_x = change->resolution.newSize.x;
        scriptInfo->play_res_y = change->resolution.newSize.y;
    }
}

int CFASSFileScriptInfoRegisterAssociationwithFile(CFASSFileScriptInfoRef scriptInfo, CFASSFileRef assFile)
{
    if(scriptInfo->registeredFile == NULL)
    {
        scriptInfo->registeredFile = assFile;
        return 0;
    }
    else
        return -1;
}

#pragma mark - construction for output

wchar_t *CFASSFileScriptInfoAllocateFileContent(CFASSFileScriptInfoRef scriptInfo)
{
    size_t commentAmount = CFUnicodeStringArrayGetCount(scriptInfo->comment),
    scriptStringLength = 0;
    int temp;
    
#pragma mark caculation of size
    
    scriptStringLength += wcslen(L"[Script Info]\n");
    
    for(size_t count = 1; count<=commentAmount; count++)
        scriptStringLength += wcslen(CFUnicodeStringArrayGetStringAtIndex(scriptInfo->comment, count-1))+2;
    
    FILE *fp;
    if((fp=tmpfile()) == NULL) return NULL;
    
    temp =
    fwprintf(fp, L"Title: %ls\n"
                 L"Original Script: %ls\n",
                scriptInfo->title == NULL ? L"" : scriptInfo->title,
                scriptInfo->original_script == NULL ? L"" : scriptInfo->original_script);
    if(temp < 0) { fclose(fp); return NULL; }
    else scriptStringLength += temp;
    
    if(scriptInfo->original_translation != NULL) {
        temp = fwprintf(fp, L"Original Translation: %ls\n", scriptInfo->original_translation);
        if(temp < 0) { fclose(fp); return NULL; }
        else scriptStringLength += temp;
    }
    if(scriptInfo->original_editing != NULL) {
        temp = fwprintf(fp, L"Original Editing: %ls\n", scriptInfo->original_editing);
        if(temp < 0) { fclose(fp); return NULL; }
        else scriptStringLength += temp;
    }
    if(scriptInfo->original_timing != NULL) {
        temp = fwprintf(fp, L"Original Timing: %ls\n", scriptInfo->original_timing);
        if(temp < 0) { fclose(fp); return NULL; }
        else scriptStringLength += temp;
    }
    if(scriptInfo->synch_point != NULL) {
        temp = fwprintf(fp, L"Synch Point: %ls\n", scriptInfo->synch_point);
        if(temp < 0) { fclose(fp); return NULL; }
        else scriptStringLength += temp;
    }
    if(scriptInfo->script_updated_by != NULL) {
        temp = fwprintf(fp, L"Script Updated By: %ls\n", scriptInfo->script_updated_by);
        if(temp < 0) { fclose(fp); return NULL; }
        else scriptStringLength += temp;
    }
    if(scriptInfo->update_details != NULL) {
        temp = fwprintf(fp, L"Update Details: %ls\n", scriptInfo->update_details);
        if(temp < 0) {fclose(fp); return NULL;}
        else scriptStringLength += temp;
    }
    if(scriptInfo->script_type != NULL) {
        temp = fwprintf(fp, L"ScriptType: %ls\n", scriptInfo->script_type);
        if(temp < 0) { fclose(fp); return NULL; }
        else scriptStringLength += temp;
    }
    
    if(scriptInfo->is_collisions_normal)
        scriptStringLength+=wcslen(L"Collisions: Normal\n");
    else
        scriptStringLength+=wcslen(L"Collisions: Reversed\n");
    
    temp =
    fwprintf(fp, L"PlayResX: %u\n"
                 L"PlayResY: %u\n",
                 scriptInfo->play_res_x,
                 scriptInfo->play_res_y);
    if(temp < 0) {fclose(fp); return NULL;}
    else scriptStringLength += temp;
    
    if(scriptInfo->play_depth != NULL) {
        temp = fwprintf(fp, L"PlayDepth: %ls\n", scriptInfo->play_depth);
        if(temp < 0) { fclose(fp); return NULL; }
        else scriptStringLength += temp;
    }
    
    temp =
    fwprintf(fp, L"Timer: %.4f\n"
                 L"WrapStyle: %u\n",
                 scriptInfo->timer,
                 scriptInfo->wrap_style);
    if(temp < 0) { fclose(fp); return NULL; }
    else scriptStringLength += temp;
    
#pragma mark output
    
    wchar_t *result;
    if((result = malloc(sizeof(wchar_t) * (scriptStringLength + 1))) == NULL)
        { fclose(fp); return NULL; }
    
    wchar_t *writingPoint = result;
    
    temp = swprintf(writingPoint, scriptStringLength + 1, L"[Script Info]\n");
    writingPoint += temp; scriptStringLength -= temp;
    
    for(size_t count = 1; count <= commentAmount; count++)
    {
        temp = swprintf(writingPoint, scriptStringLength + 1, L";%ls\n", CFUnicodeStringArrayGetStringAtIndex(scriptInfo->comment, count-1));
        writingPoint += temp; scriptStringLength -= temp;
    }
    
    temp =
    swprintf(writingPoint, scriptStringLength + 1,
             L"Title: %ls\n"
             L"Original Script: %ls\n",
             scriptInfo->title == NULL ? L"" : scriptInfo->title,
             scriptInfo->original_script == NULL ? L"" : scriptInfo->original_script);
    writingPoint += temp; scriptStringLength -= temp;
    
    if(scriptInfo->original_translation != NULL) {
        temp =
        swprintf(writingPoint, scriptStringLength + 1,
                 L"Original Translation: %ls\n", scriptInfo->original_translation);
        writingPoint += temp; scriptStringLength -= temp;
    }
    if(scriptInfo->original_editing != NULL) {
        temp =
        swprintf(writingPoint, scriptStringLength + 1,
                 L"Original Editing: %ls\n", scriptInfo->original_editing);
        writingPoint += temp; scriptStringLength -= temp;
    }
    if(scriptInfo->original_timing != NULL) {
        temp =
        swprintf(writingPoint, scriptStringLength + 1,
                 L"Original Timing: %ls\n", scriptInfo->original_timing);
        writingPoint += temp; scriptStringLength -= temp;
    }
    if(scriptInfo->synch_point != NULL) {
        temp =
        swprintf(writingPoint, scriptStringLength + 1,
                 L"Synch Point: %ls\n", scriptInfo->synch_point);
        writingPoint += temp; scriptStringLength -= temp;
    }
    if(scriptInfo->script_updated_by != NULL) {
        temp =
        swprintf(writingPoint, scriptStringLength + 1,
                 L"Script Updated By: %ls\n", scriptInfo->script_updated_by);
        writingPoint += temp; scriptStringLength -= temp;
    }
    if(scriptInfo->update_details != NULL) {
        temp =
        swprintf(writingPoint, scriptStringLength + 1,
                 L"Update Details: %ls\n", scriptInfo->update_details);
        writingPoint += temp; scriptStringLength -= temp;
    }
    if(scriptInfo->script_type != NULL) {
        temp =
        swprintf(writingPoint, scriptStringLength + 1,
                 L"ScriptType: %ls\n", scriptInfo->script_type);
        writingPoint += temp; scriptStringLength -= temp;
    }
    
    if(scriptInfo->is_collisions_normal)
    {
        temp =
        swprintf(writingPoint, scriptStringLength + 1, L"Collisions: Normal\n");
        writingPoint += temp; scriptStringLength -= temp;
    }
    else
    {
        temp =
        swprintf(writingPoint, scriptStringLength + 1, L"Collisions: Reversed\n");
        writingPoint += temp; scriptStringLength -= temp;
    }
    
    temp =
    swprintf(writingPoint, scriptStringLength + 1,
             L"PlayResX: %u\n"
             L"PlayResY: %u\n",
             scriptInfo->play_res_x,
             scriptInfo->play_res_y);
    writingPoint += temp; scriptStringLength -= temp;
    
    if(scriptInfo->play_depth != NULL) {
        temp = swprintf(writingPoint, scriptStringLength + 1,
                 L"PlayDepth: %ls\n", scriptInfo->play_depth);
        writingPoint += temp; scriptStringLength -= temp;
    }
    temp =
    swprintf(writingPoint, scriptStringLength + 1,
             L"Timer: %.4f\n"
             L"WrapStyle: %u\n",
             scriptInfo->timer,
             scriptInfo->wrap_style);
    // writingPoint += temp; scriptStringLength -= temp;
    
    DEBUG_ASSERT((scriptStringLength -= temp) == 0);
    
    fclose(fp);
    return result;
}

static wchar_t *CFASSFileScriptInfoAllocateEachContent(const wchar_t *name,         /* sample: L"Tiltle", without comma */
                                                       const wchar_t *beginPoint,
                                                       const wchar_t *endPoint,
                                                       size_t *shouldSkip);

CFASSFileScriptInfoRef CFASSFileScriptInfoCreateEssential(wchar_t *title,
                                                          wchar_t *original_script,
                                                          bool is_collisions_normal,
                                                          unsigned int play_res_x,
                                                          unsigned int play_res_y,
                                                          double timePercentage)
{
    CFASSFileScriptInfoRef result;
    if((result = malloc(sizeof(struct CFASSFileScriptInfo))) != NULL)
    {
        if((result->comment = CFUnicodeStringArrayCreateEmpty()) != NULL)
        {
            result->title = NULL; result->original_script = NULL; result->original_translation = NULL;
            result->original_editing = NULL; result->original_timing = NULL; result->synch_point = NULL;
            result->script_updated_by = NULL; result->update_details = NULL; result->script_type = NULL;
            result->play_depth = NULL;
            if(title == NULL || (result->title=CF_Dump_wchar_string(title)) != NULL){
                if(original_script == NULL || (result->original_script=CF_Dump_wchar_string(original_script)) != NULL){
                    result->is_collisions_normal = is_collisions_normal;
                    result->play_res_x = play_res_x;
                    result->play_res_y = play_res_y;
                    result->timer = timePercentage;
                    result->wrap_style = 0u;
                    result->registeredFile = NULL;
                    return result;
                }
                free(result->title);
            }
            CFUnicodeStringArrayDestory(result->comment);
        }
        free(result);
    }
    return NULL;
}

CFASSFileScriptInfoRef CFASSFileScriptInfoCreateWithUnicodeFileContent(const wchar_t * _Nonnull content,
                                                                       CFASSFileParsingResultRef _Nonnull parsingResult) {
    DEBUG_ASSERT(content != NULL && parsingResult != NULL);
    if(content == NULL || parsingResult == NULL) return NULL;
    
    CFASSFileScriptInfoRef result;
    if((result = malloc(sizeof(struct CFASSFileScriptInfo))) == NULL) {
        PR_INFO(NULL, L"CFASSFileScriptInfo allocation failed");
        return NULL;
    }
    
    if((result->comment = CFUnicodeStringArrayCreateEmpty()) == NULL) {
        PR_INFO(NULL, L"CFASSFileScriptInfo comment intialization failed");
        free(result);
        return NULL;
    }
    result->registeredFile = NULL;
    
    const wchar_t *beginPoint = content;
    
    int compareResult;
    if((compareResult = wmemcmp(beginPoint, L"[Script Info]\n", wcslen(L"[Script Info]\n"))) != 0) {
        CFUnicodeStringArrayDestory(result->comment);
        free(result);
        PR_ERROR(beginPoint, L"file not begin with [Script Info], parsing content stop");
        return NULL;
    }
    beginPoint += wcslen(L"[Script Info]\n");
    
    const wchar_t *endPoint;
    if((endPoint = wcsstr(beginPoint, L"\n[V4+ Styles]")) == NULL) {
        CFUnicodeStringArrayDestory(result->comment);
        free(result);
        PR_ERROR(beginPoint, L"[V4+ Styles] not found, script format not supported, parsing content stop");
        return NULL;
    }
    bool encounterEndChar; size_t length;
    while(*beginPoint == L';') {    // comment begin sign
        length = CFUnicodeStringArrayAddStringWithEndChar(result->comment, beginPoint + 1, L'\n', &encounterEndChar);
        if(!encounterEndChar) {
            CFUnicodeStringArrayDestory(result->comment);
            free(result);
            PR_ERROR(beginPoint, L"comment begin ; detected without matching line terminate");
            return NULL;
        }
        beginPoint += length + 2;   // percede to next ';' or other
    }
    beginPoint--;                   // backoff make space for '\n', this always valid as "[Script Info]\n" first searched
    
    int scanAmount = 0;
    
    // begin point set to '\n', just after "[Script Info]" or any comments
    //   end point set to '\n' just before "[V4+ Styles]"
    
    #if CFASSFileScriptInfoFileContentSearchInOrder
    
    size_t skipAmount;
    
    result->title =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Title"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint += skipAmount;
    
    result->original_script =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Original Script"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint += skipAmount;
    
    result->original_translation =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Original Translation"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint += skipAmount;
    
    result->original_editing =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Original Editing"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint += skipAmount;
    
    result->original_timing =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Original Timing"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint += skipAmount;
    
    result->synch_point =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Synch Point"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint += skipAmount;
    
    result->script_updated_by =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Script Updated By"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint += skipAmount;
    
    result->update_details =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Update Details"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint += skipAmount;
    
    result->script_type =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"ScriptType"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint += skipAmount;
    
    /* begin of content interprate */
    
    wchar_t *dataPoint;
    
    /* Collisions */
    dataPoint = CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Collisions"),
                                                       beginPoint,
                                                       endPoint,
                                                       &skipAmount);
    if(dataPoint != NULL) {
        scanAmount = 0;
        swscanf(dataPoint, L"Normal%n", &scanAmount);
        if(scanAmount == wcslen(L"Normal"))
            result->is_collisions_normal = true;
        else
        {
            scanAmount = 0;
            swscanf(dataPoint, L"Reverse%n", &scanAmount);
            if(scanAmount == wcslen(L"Reverse")) result->is_collisions_normal = false;
            else {
                PR_WARN(beginPoint, L"unkown Collisions format: %ls, default to normal", dataPoint);
                result->is_collisions_normal = true;
            }
        }
        free(dataPoint);
    }
    else result->is_collisions_normal = true;
    beginPoint += skipAmount;
    
    
    /* PlayResX */
    dataPoint = CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"PlayResX"),
                                                       beginPoint,
                                                       endPoint,
                                                       &skipAmount);
    if(dataPoint != NULL) {
        if(swscanf(dataPoint, L"%u", &result->play_res_x) != 1) {
            PR_WARN(beginPoint, L"unable to parse PlayResX: %ls, default to zero", dataPoint);
            result->play_res_x = 0u;
        }
        free(dataPoint);
    }
    else result->play_res_x = 0u;
    beginPoint += skipAmount;
    
    
    /* PlayResY */
    dataPoint = CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"PlayResY"),
                                                       beginPoint,
                                                       endPoint,
                                                       &skipAmount);
    if(dataPoint != NULL) {
        if(swscanf(dataPoint, L"%u", &result->play_res_y) != 1) {
            PR_WARN(beginPoint, L"unable to parse PlayResY: %ls, default to zero", dataPoint);
            result->play_res_y = 0u;
        }
        free(dataPoint);
    }
    else result->play_res_y = 0u;
    beginPoint += skipAmount;
    
    
    /* PlayDepth */
    result->play_depth =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"PlayDepth"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint += skipAmount;
    
    
    /* Timer */
    dataPoint = CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Timer"),
                                                       beginPoint,
                                                       endPoint,
                                                       &skipAmount);
    if(dataPoint != NULL) {
        if(swscanf(dataPoint, L"%lf", &result->timer) != 1) {
            PR_WARN(beginPoint, L"unable to parse Timer(speed): %ls, default to zero", dataPoint);
            result->timer = 100.0;
        }
        free(dataPoint);
    }
    else result->timer = 100.0;
    beginPoint += skipAmount;
    
        
    /* WrapStyle */
    dataPoint = CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"WrapStyle"),
                                                       beginPoint,
                                                       endPoint,
                                                       &skipAmount);
    if(dataPoint != NULL) {
        if(swscanf(dataPoint, L"%u", &result->wrap_style) != 1 || result->wrap_style > 4) {
            PR_WARN(beginPoint, L"unable to parse WrapStyle: %ls, default to zero", dataPoint);
            result->wrap_style = 0u;
        }
        free(dataPoint);
    }
    else result->wrap_style = 0u;
    beginPoint += skipAmount;
    
    #else
    
    result->title =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Title"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    result->original_script =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Original Script"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    result->original_translation =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Original Translation"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    result->original_editing =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Original Editing"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    result->original_timing =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Original Timing"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    result->synch_point =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Synch Point"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    result->script_updated_by =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Script Updated By"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    result->update_details =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Update Details"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    result->script_type =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"ScriptType"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    
    wchar_t *dataPoint;
    
    /* Collisions */
    dataPoint = CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Collisions"),
                                                       beginPoint,
                                                       endPoint,
                                                       NULL);
    if(dataPoint != NULL) {
        scanAmount = 0;
        swscanf(dataPoint, L"Normal%n", &scanAmount);
        if(scanAmount == wcslen(L"Normal"))
            result->is_collisions_normal = true;
        else
        {
            scanAmount = 0;
            swscanf(dataPoint, L"Reverse%n", &scanAmount);
            if(scanAmount == wcslen(L"Reverse")) result->is_collisions_normal = false;
            else {
                PR_WARN(beginPoint, L"unkown Collisions format: %ls, default to normal", dataPoint);
                result->is_collisions_normal = true;
            }
        }
        free(dataPoint);
    }
    else result->is_collisions_normal = true;
    
    /* PlayResX */
    dataPoint = CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"PlayResX"),
                                                       beginPoint,
                                                       endPoint,
                                                       NULL);
    if(dataPoint != NULL) {
        if(swscanf(dataPoint, L"%u", &result->play_res_x) != 1) {
            PR_WARN(beginPoint, L"unable to parse PlayResX: %ls, default to zero", dataPoint);
            result->play_res_x = 0u;
        }
        free(dataPoint);
    }
    else result->play_res_x = 0u;
    
    /* PlayResY */
    dataPoint = CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"PlayResY"),
                                                       beginPoint,
                                                       endPoint,
                                                       NULL);
    if(dataPoint != NULL) {
        if(swscanf(dataPoint, L"%u", &result->play_res_y) != 1) {
            PR_WARN(beginPoint, L"unable to parse PlayResY: %ls, default to zero", dataPoint);
            result->play_res_y = 0u;
        }
        free(dataPoint);
    }
    else result->play_res_y = 0u;
    
    /* PlayDepth */
    result->play_depth =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"PlayDepth"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    
    /* Timer */
    dataPoint = CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Timer"),
                                                       beginPoint,
                                                       endPoint,
                                                       NULL);
    if(dataPoint != NULL) {
        if(swscanf(dataPoint, L"%lf", &result->timer) != 1) {
            PR_WARN(beginPoint, L"unable to parse Timer(speed): %ls, default to zero", dataPoint);
            result->timer = 100.0;
        }
        free(dataPoint);
    }
    else result->timer = 100.0;
    
    /* WrapStyle */
    dataPoint = CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"WrapStyle"),
                                                       beginPoint,
                                                       endPoint,
                                                       NULL);
    if(dataPoint != NULL) {
        if(swscanf(dataPoint, L"%u", &result->wrap_style) != 1 || result->wrap_style > 4) {
            PR_WARN(beginPoint, L"unable to parse WrapStyle: %ls, default to zero", dataPoint);
            result->wrap_style = 0u;
        }
        free(dataPoint);
    }
    else result->wrap_style = 0u;
    
    #endif
    
    return result;
}

/*!
 @function CFASSFileScriptInfoAllocateEachContent
 @abstract search for string within [beginPoint, endPoint] for string start with
           name. That is (beginPoint, nameBegin, nameEnd, endPoint), then try to
           search '\\n' within [nameEnd + 1, endPoint] if possible, find the
           returnPoint, if [nameEnd + 1, returnPoint - 1] makes some string,
           that string is actually returned. Note that prefixed blank is not
           ignore, but considered as content.
 @param name the match name, sample "\nTitile:"
 @param beginPoint the beginPoint to search for content
 @param endPoint the lastPoint to search for content
 @param shouldSkip if not NULL, this should always returned an interpreted skip
        length, which should skipped the trying search part
 @return only when [nameEnd + 1, returnPoint - 1] makes some string, that is
         dumpped and returned
 */
static wchar_t *CFASSFileScriptInfoAllocateEachContent(const wchar_t *name,
                                                       const wchar_t *beginPoint,
                                                       const wchar_t *endPoint,
                                                       size_t *shouldSkip)
{
    if(shouldSkip != NULL) *shouldSkip = 0;
    const wchar_t *searchPoint;
    if((searchPoint = CF_wcsstr_with_end_point(beginPoint, name, endPoint)) != NULL)
    {
        searchPoint += wcslen(name);
        const wchar_t *trailReturnPoint = CF_wcsstr_with_end_point(searchPoint, L"\n", endPoint);
        if(trailReturnPoint != NULL)
        {
            if(shouldSkip != NULL) *shouldSkip = trailReturnPoint - beginPoint;
            // in any kind of condition, just point to next character of '\n'
            // maybe just '\0'
            
            if(searchPoint[0] == L' ') searchPoint++;   // skip prefixed blanks
            
            ptrdiff_t length;      // actual allocated size
            if((length = trailReturnPoint - searchPoint) != 0) {
                wchar_t *result;
                if((result = malloc(sizeof(wchar_t) * (length + 1))) != NULL) {
                    wmemcpy(result, searchPoint, length);
                    result[length] = L'\0';
                    return result;
                }
            }
        }
        else if(shouldSkip != NULL) *shouldSkip = searchPoint - beginPoint;
        // beginPoint + shouldSkip point to next character after name("\nTitle:"), maybe just '\0'
    }
    return NULL;
}

CFASSFileScriptInfoRef CFASSFileScriptInfoCopy(CFASSFileScriptInfoRef scriptInfo)
{
    /* the fucking loop makes my code seems ugly */
    CFASSFileScriptInfoRef duplicated = malloc(sizeof(struct CFASSFileScriptInfo));
    if(duplicated == NULL) return NULL;
    if((duplicated->comment = CFUnicodeStringArrayCopy(scriptInfo->comment)) != NULL) {
        if(scriptInfo->title == NULL || (duplicated->title = CF_Dump_wchar_string(scriptInfo->title)) != NULL){
            if(scriptInfo->original_script == NULL || (duplicated->original_script = CF_Dump_wchar_string(scriptInfo->original_script)) != NULL){
                if(scriptInfo->original_translation == NULL || (duplicated->original_translation = CF_Dump_wchar_string(scriptInfo->original_translation)) != NULL){
                    if(scriptInfo->original_editing == NULL || (duplicated->original_editing = CF_Dump_wchar_string(scriptInfo->original_editing)) != NULL){
                        if(scriptInfo->original_timing == NULL || (duplicated->original_timing = CF_Dump_wchar_string(scriptInfo->original_timing)) != NULL){
                            if(scriptInfo->synch_point == NULL || (duplicated->synch_point = CF_Dump_wchar_string(scriptInfo->synch_point)) != NULL){
                                if(scriptInfo->script_updated_by == NULL || (duplicated->script_updated_by = CF_Dump_wchar_string(scriptInfo->script_updated_by)) != NULL){
                                    if(scriptInfo->update_details == NULL || (duplicated->update_details = CF_Dump_wchar_string(scriptInfo->update_details)) != NULL){
                                        if(scriptInfo->script_type == NULL || (duplicated->script_type = CF_Dump_wchar_string(scriptInfo->script_type)) != NULL){
                                            if(scriptInfo->play_depth == NULL || (duplicated->play_depth = CF_Dump_wchar_string(scriptInfo->play_depth)) != NULL){
                                                duplicated->registeredFile = NULL;
                                                return duplicated;
                                            }
                                            free(duplicated->script_type);
                                        }
                                        free(duplicated->update_details);
                                    }
                                    free(duplicated->script_updated_by);
                                }
                                free(duplicated->synch_point);
                            }
                            free(duplicated->original_timing);
                        }
                        free(duplicated->original_editing);
                    }
                    free(duplicated->original_translation);
                }
                free(duplicated->original_script);
            }
            free(duplicated->title);
        }
        CFUnicodeStringArrayDestory(duplicated->comment);
    }
    free(duplicated);
    return NULL;
}

void CFASSFileScriptInfoDestory(CFASSFileScriptInfoRef scriptInfo)
{
    if(scriptInfo == NULL) return;
    CFUnicodeStringArrayDestory(scriptInfo->comment);
    free(scriptInfo->title);
    free(scriptInfo->original_script);
    free(scriptInfo->original_translation);
    free(scriptInfo->original_editing);
    free(scriptInfo->original_timing);
    free(scriptInfo->synch_point);
    free(scriptInfo->script_updated_by);
    free(scriptInfo->update_details);
    free(scriptInfo->script_type);
    free(scriptInfo->play_depth);
    free(scriptInfo);
}

CLANG_DIAGNOSTIC_POP

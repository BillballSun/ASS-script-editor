//
//  CFASSFileStyle.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/9.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <wchar.h>
#include <stdlib.h>
#include <stdbool.h>
#include <wctype.h>
#include <limits.h>
#include <string.h>

#include "CFASSFileStyle.h"
#include "CFASSFile_Private.h"
#include "CFException.h"
#include "CFASSFileChange.h"
#include "CFASSFileChange_Private.h"
#include "CFASSFileControl.h"
#include "CFASSFileParsingResult.h"
#include "CFASSFileParsingResult_Macro.h"
#include "CFMacro.h"

#define CFASSFileStyleCreateWithStringScanBySteps 0

struct CFASSFileStyle
{
    wchar_t *name,                              // Can't include comma
            *font_name;
    unsigned int font_size;
    CFASSFileStyleColor primary_colour,
                        secondary_colour,
                        outline_colour,
                        back_colour;
    bool blod, italic, underline, strike_out;   // -1 for true, 0 for false
    double scale_x, scale_y;                    // percentage, without digitpoint
    double spacing;                             // extra spacing between characters, in pixels, may be negative and has decimalPoints
    double angle;                               // the origin is defined be alignment
    int border_style;                           // 1 = outline+drop shadow, 3 = opaque box
    unsigned int outline;                       // if BorderStyle is 1, this specify width of ouline around text in pixels
                                                // value may be 0-4
    unsigned int shadow;                        // if BorderStyle is 1, this specify depth of dropping shadow behind text in pixels
                                                // value may be 0-4, Drop shadow is always used in addition to an outline.
                                                // SSA will force an outline of 1 pixel if no outline width is given.
    int alignment;                              // 1 - 9, numberPad
    unsigned int marginL, marginR, marginV;     // margins in pixels
    unsigned int encoding;
};

static bool CFASSFileStyleCreateWithStringIsSkip(wchar_t input);

static const wchar_t * _Nonnull CFASSFileStylePattenMatch(int matchCount);

void CFASSFileStyleMakeChange(CFASSFileStyleRef style, CFASSFileChangeRef change)
{
    if(style == NULL || change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileStyle %p MakeChange %p", style, change);
    if(change->type & CFASSFileChangeTypeFontSize)
    {
        if(change->fontSize.byPercentage)
            style->font_size *= change->fontSize.percentage;
        else
            style->font_size = change->fontSize.fontSize;
    }
    if(change->type & CFASSFileChangeTypeFontName)
    {
        wchar_t *dumped = CF_Dump_wchar_string(change->fontName.fontName);
        if(dumped != NULL)
        {
            free(style->font_name);
            style->font_name = dumped;
        }
    }
    if(change->type & CFASSFileChangeTypePrimaryColor)
    {
        style->primary_colour = CFASSFileStyleColorMake(change->primaryColor.alpha, change->primaryColor.blue, change->primaryColor.green, change->primaryColor.red);
    }
    if(change->type & CFASSFileChangeTypeOutlineColor)
    {
        style->outline_colour = CFASSFileStyleColorMake(change->outlineColor.alpha, change->outlineColor.blue, change->outlineColor.green, change->outlineColor.red);
    }
    if(change->type & CFASSFileChangeTypeBlod)
    {
        style->blod = change->blod.isBlod;
    }
    if(change->type & CFASSFileChangeTypeBorderStyle)
    {
        style->border_style = change->borderStyle.borderStyle;
    }
    if(change->type & CFASSFileChangeTypeShadow)
    {
        if(change->shadow.forceBorderStyle)
            style->border_style = 1;
        if(style->border_style == 1)
            style->shadow = change->shadow.pixel;
    }
    if(change->type & CFASSFileChangeTypeOutline)
    {
        if(change->outline.forceBorderStyle)
            style->border_style = 1;
        if(style->border_style == 1)
            style->outline = change->outline.pixel;
    }
}

CFASSFileStyleRef CFASSFileStyleCopy(CFASSFileStyleRef style)
{
    CFASSFileStyleRef result;
    if((result = malloc(sizeof(struct CFASSFileStyle))) != NULL)
    {
        if((result->name = CF_Dump_wchar_string(style->name)) != NULL)
        {
            if((result->font_name = CF_Dump_wchar_string(style->font_name)) != NULL)
            {
                result->font_size = style->font_size;
                result->primary_colour = style->primary_colour;
                result->secondary_colour = style->secondary_colour;
                result->outline_colour = style->outline_colour;
                result->blod = style->blod;
                result->italic = style->italic;
                result->underline = style->underline;
                result->strike_out = style->strike_out;
                result->scale_x = style->scale_x;
                result->scale_y = style->scale_y;
                result->spacing = style->spacing;
                result->angle = style->angle;
                result->border_style = style->border_style;
                result->outline = style->outline;
                result->shadow = style->shadow;
                result->alignment = style->alignment;
                result->marginL = style->marginL;
                result->marginR = style->marginR;
                result->marginV = style->marginV;
                result->encoding = style->encoding;
                
                return result;
            }
            free(result->name);
        }
        free(result);
    }
    return NULL;
}

void CFASSFileStyleDestory(CFASSFileStyleRef style)
{
    if(style == NULL) return;
    free(style->name);
    free(style->font_name);
    free(style);
}

CFASSFileStyleRef CFASSFileStyleCreate(wchar_t *name,
                                       wchar_t *font_name, unsigned int font_size,
                                       CFASSFileStyleColor primary_colour,
                                       CFASSFileStyleColor secondary_colour,
                                       CFASSFileStyleColor outline_colour,
                                       bool blod, bool italic, bool underline, bool strike_out,
                                       double scale_x, double scale_y,
                                       double spacing,
                                       double angle,
                                       int border_style,
                                       unsigned int outline, unsigned int shadow,
                                       int alignment,
                                       unsigned int marginL, unsigned int marginR, unsigned marginV,
                                       unsigned encoding)
{
    if(name == NULL || font_name == NULL || font_size == 0) return NULL;
    if(border_style!=1 && border_style!=3) return NULL;
    if(outline>4) return NULL;
    if((border_style!=1 && shadow!=0) || shadow>4 || (shadow!=0 && outline==0)) return NULL;
    if(alignment<1 || alignment>9) return NULL;
    
    CFASSFileStyleRef result;
    
    if((result = malloc(sizeof(struct CFASSFileStyle))) != NULL)
    {
        if((result->name = CF_Dump_wchar_string(name)) != NULL)
        {
            if((result->font_name = CF_Dump_wchar_string(font_name)) != NULL)
            {
                result->font_size = font_size;
                result->primary_colour = primary_colour;
                result->secondary_colour = secondary_colour;
                result->outline_colour = outline_colour;
                result->blod = blod;
                result->italic = italic;
                result->underline = underline;
                result->strike_out = strike_out;
                result->scale_x = scale_x;
                result->scale_y = scale_y;
                result->spacing = spacing;
                result->angle = angle;
                result->border_style = border_style;
                result->outline = outline;
                result->shadow = shadow;
                result->alignment = alignment;
                result->marginL = marginL;
                result->marginR = marginR;
                result->marginV = marginV;
                result->encoding = encoding;
                
                return result;
            }
            free(result->name);
        }
        free(result);
    }
    
    return NULL;
}

wchar_t *CFASSFileStyleAllocateFileContent(CFASSFileStyleRef style)
{
    FILE *fp;
    if((fp = tmpfile()) == NULL) return NULL;
    
    int stringLength =
    fwprintf(fp,
             L"Style: %ls,%ls,%u,"                  /* font */
             L"&H%2.2hX%2.2hX%2.2hX%2.2hX,"         /* colour */
             L"&H%2.2hX%2.2hX%2.2hX%2.2hX,"
             L"&H%2.2hX%2.2hX%2.2hX%2.2hX,"
             L"&H%2.2hX%2.2hX%2.2hX%2.2hX,"
             L"%d,%d,%d,%d,"                        /* blod, italic, underline, strike_out */
             L"%g,%g,"                              /* scale_x, scale_y */
             L"%g,"                                 /* spacing */
             L"%g,"                                 /* angle */
             L"%d,"                                 /* border_style */
             L"%u,%u,"                              /* outline, shadow */
             L"%d,"                                 /* alignment */
             L"%u,%u,%u,"                           /* margins */
             L"%u\n",                              /* encoding */
             style->name, style->font_name, style->font_size,
             
             (unsigned int)style->primary_colour.alpha,
             (unsigned int)style->primary_colour.blue,
             (unsigned int)style->primary_colour.green,
             (unsigned int)style->primary_colour.red,
             
             (unsigned int)style->secondary_colour.alpha,
             (unsigned int)style->secondary_colour.blue,
             (unsigned int)style->secondary_colour.green,
             (unsigned int)style->secondary_colour.red,
             
             (unsigned int)style->outline_colour.alpha,
             (unsigned int)style->outline_colour.blue,
             (unsigned int)style->outline_colour.green,
             (unsigned int)style->outline_colour.red,
             
             (unsigned int)style->back_colour.alpha,
             (unsigned int)style->back_colour.blue,
             (unsigned int)style->back_colour.green,
             (unsigned int)style->back_colour.red,
             
             style->blod?-1:0,style->italic?-1:0,style->underline?-1:0,style->strike_out?-1:0,
             style->scale_x, style->scale_y,
             style->spacing,
             style->angle,
             style->border_style,
             style->outline, style->shadow,
             style->alignment,
             style->marginL, style->marginR, style->marginV,
             style->encoding);
    
#warning HuaQ
    DEBUG_ASSERT(style->outline == 2);
    
    wchar_t *result;
    if((result = malloc(sizeof(wchar_t) * (stringLength + 1))) != NULL)
    {
        swprintf(result, stringLength + 1,
                 L"Style: %ls,%ls,%u,"                  /* font */
                 L"&H%2.2hX%2.2hX%2.2hX%2.2hX,"         /* colour */
                 L"&H%2.2hX%2.2hX%2.2hX%2.2hX,"
                 L"&H%2.2hX%2.2hX%2.2hX%2.2hX,"
                 L"&H%2.2hX%2.2hX%2.2hX%2.2hX,"
                 L"%d,%d,%d,%d,"                        /* blod, italic, underline, strike_out */
                 L"%g,%g,"                              /* scale_x, scale_y */
                 L"%g,"                                 /* spacing */
                 L"%g,"                                 /* angle */
                 L"%d,"                                 /* border_style */
                 L"%u,%u,"                              /* outline, shadow */
                 L"%d,"                                 /* alignment */
                 L"%u,%u,%u,"                           /* margins */
                 L"%u\n",                               /* encoding */
                 style->name, style->font_name, style->font_size,
                 
                 (unsigned int)style->primary_colour.alpha,
                 (unsigned int)style->primary_colour.blue,
                 (unsigned int)style->primary_colour.green,
                 (unsigned int)style->primary_colour.red,
                 
                 (unsigned int)style->secondary_colour.alpha,
                 (unsigned int)style->secondary_colour.blue,
                 (unsigned int)style->secondary_colour.green,
                 (unsigned int)style->secondary_colour.red,
                 
                 (unsigned int)style->outline_colour.alpha,
                 (unsigned int)style->outline_colour.blue,
                 (unsigned int)style->outline_colour.green,
                 (unsigned int)style->outline_colour.red,
                 
                 (unsigned int)style->back_colour.alpha,
                 (unsigned int)style->back_colour.blue,
                 (unsigned int)style->back_colour.green,
                 (unsigned int)style->back_colour.red,
                 
                 style->blod?-1:0,style->italic?-1:0,style->underline?-1:0,style->strike_out?-1:0,
                 style->scale_x, style->scale_y,
                 style->spacing,
                 style->angle,
                 style->border_style,
                 style->outline, style->shadow,
                 style->alignment,
                 style->marginL, style->marginR, style->marginV,
                 style->encoding);
        DEBUG_ASSERT(wcslen(result) == stringLength);
        return result;
    }
    fclose(fp);
    return NULL;
}

#pragma mark - string content parsing [解析]

CFASSFileStyleRef CFASSFileStyleCreateWithString(const wchar_t *content, CFASSFileParsingResultRef parsingResult)
{
    const wchar_t *beginPoint = content,        // beginPoint to Style:
                  *endPoint = content;          // then make endPoint to '\n' or '\0'
    while(*endPoint != L'\n' && *endPoint != L'\0') endPoint++;
    
    int scanedAmount = (int)wcslen(L"Style:");
    if(wcsncmp(beginPoint, L"Style:", wcslen(L"Style:")) == 0) {
        beginPoint += scanedAmount;
        
        // skip any pending blanks
        while(*beginPoint == ' ' && beginPoint < endPoint) beginPoint++;
        
        // beginPoint points to character after "Style:" and skip any blanks at least to '\0' or '\n'
        // endPoint just right '\n' or '\0'
        
        // begin point may be at endPoint
        if(beginPoint == endPoint) {
            PR_ERROR(beginPoint, L"CFASSFileStyle unable to find style name");
            return NULL;
        }
        
        CFASSFileStyleRef result;
        if((result = malloc(sizeof(struct CFASSFileStyle))) != NULL) {
            
#pragma mark style_name
            
            if((result->name = CF_allocate_wchar_string_with_endPoint_skip_terminate(beginPoint,
                                                                                     endPoint - 1,
                                                                                     CFASSFileStyleCreateWithStringIsSkip,
                                                                                     NULL)) != NULL) {
                while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                
                // at least beginPoint to endPoint, may point to any ',' before endPoint
                if(*beginPoint == L',')
                {
                    beginPoint++;
                    
#pragma mark font_name
                    
                    if((result->font_name = CF_allocate_wchar_string_with_endPoint_skip_terminate(beginPoint, endPoint - 1, CFASSFileStyleCreateWithStringIsSkip, NULL)) != NULL)
                    {
                        CFASSFileControlLevel controlLevel = CFASSFileControlGetLevel();
                        
                        #if CFASSFileStyleCreateWithStringScanBySteps
                        
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match font_size");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark font_size
                        
                        if(swscanf(beginPoint, L"%u%n", &result->font_size, &scanedAmount) != 1) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing font_size failed");
                            goto LABEL_ParsingFailed;
                        }
                        if(result->font_size == 0) {
                            if(controlLevel & CFASSFileControlLevelIgnore) {
                                PR_WARN(beginPoint, L"CFASSFileStyle font_size zero, auto set to default 16");
                                result->font_size = 16;
                            }
                            else {
                                PR_ERROR(beginPoint, L"CFASSFileStyle font_size zero, parsing failed");
                                goto LABEL_ParsingFailed;
                            }
                        }
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match primary_colour");
                            goto LABEL_ParsingFailed;
                        }
                    
                        /* colors, primary, secondary, outline, back */
                        
                        unsigned short alpha, blue, green, red;
                        
#pragma mark primary_colour
                        
                        if(swscanf(beginPoint, L"&H%2hX%2hX%2hX%2hX%n", &alpha,
                                                                        &blue,
                                                                        &green,
                                                                        &red,
                                                                        &scanedAmount) != 4) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing primary_colour failed, patten "
                                                  "\"&H%%2hX%%2hX%%2hX%%2hX%%n\"");
                            goto LABEL_ParsingFailed;
                        }
                        if(alpha > 0xFF || blue > 0xFF || green > 0xFF || red > 0xFF) {
                            if(controlLevel & CFASSFileControlLevelIgnore) {
                                PR_WARN(beginPoint, L"CFASSFileStyle primary_colour ABGR[%hu, %hu, %hu, %hu] range overflow, limited in range [0..0xFF]", alpha, blue, green, red);
                                if(alpha > 0xFF) alpha = 0xFF;
                                if(blue  > 0xFF) blue  = 0xFF;
                                if(green > 0xFF) green = 0xFF;
                                if(red   > 0xFF) red   = 0xFF;
                            }
                            else {
                                PR_ERROR(beginPoint, L"CFASSFileStyle primary_colour ABGR[%hu, %hu, %hu, %hu] range overflow", alpha, blue, green, red);
                                goto LABEL_ParsingFailed;
                            }
                        }
                        result->primary_colour = CFASSFileStyleColorMake(alpha, blue, green, red);
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match secondary_colour");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark secondary_colour
                        
                        if(swscanf(beginPoint, L"&H%2hX%2hX%2hX%2hX%n", &alpha,
                                                                        &blue,
                                                                        &green,
                                                                        &red,
                                                                        &scanedAmount) != 4) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing secondary_colour failed, patten "
                                                  "\"&H%%2hX%%2hX%%2hX%%2hX%%n\"");
                            goto LABEL_ParsingFailed;
                        }
                        
                        if(alpha > 0xFF || blue > 0xFF || green > 0xFF || red > 0xFF) {
                            if(controlLevel & CFASSFileControlLevelIgnore) {
                                PR_WARN(beginPoint, L"CFASSFileStyle secondary_colour ABGR[%hu, %hu, %hu, %hu] range overflow, limited in range [0..0xFF]", alpha, blue, green, red);
                                if(alpha > 0xFF) alpha = 0xFF;
                                if(blue  > 0xFF) blue  = 0xFF;
                                if(green > 0xFF) green = 0xFF;
                                if(red   > 0xFF) red   = 0xFF;
                            }
                            else {
                                PR_ERROR(beginPoint, L"CFASSFileStyle secondary_colour ABGR[%hu, %hu, %hu, %hu] range overflow", alpha, blue, green, red);
                                goto LABEL_ParsingFailed;
                            }
                        }
                        
                        result->secondary_colour = CFASSFileStyleColorMake(alpha, blue, green, red);
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match outline_colour");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark outline_colour
                        
                        if(swscanf(beginPoint, L"&H%2hX%2hX%2hX%2hX%n", &alpha,
                                                                        &blue,
                                                                        &green,
                                                                        &red,
                                                                        &scanedAmount) != 4) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing secondary_colour failed, patten "
                                                  "\"&H%%2hX%%2hX%%2hX%%2hX%%n\"");
                            goto LABEL_ParsingFailed;
                        }
                        
                        if(alpha > 0xFF || blue > 0xFF || green > 0xFF || red > 0xFF) {
                            if(controlLevel & CFASSFileControlLevelIgnore) {
                                PR_WARN(beginPoint, L"CFASSFileStyle outline_colour ABGR[%hu, %hu, %hu, %hu] range overflow, limited in range [0..0xFF]", alpha, blue, green, red);
                                if(alpha > 0xFF) alpha = 0xFF;
                                if(blue  > 0xFF) blue  = 0xFF;
                                if(green > 0xFF) green = 0xFF;
                                if(red   > 0xFF) red   = 0xFF;
                            }
                            else {
                                PR_ERROR(beginPoint, L"CFASSFileStyle outline_colour ABGR[%hu, %hu, %hu, %hu] range overflow", alpha, blue, green, red);
                                goto LABEL_ParsingFailed;
                            }
                        }
                        
                        result->outline_colour = CFASSFileStyleColorMake(alpha, blue, green, red);
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match back_colour");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark back_colour
                        
                        if(swscanf(beginPoint, L"&H%2hX%2hX%2hX%2hX%n", &alpha,
                                                                        &blue,
                                                                        &green,
                                                                        &red,
                                                                        &scanedAmount) != 4) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing back_colour failed, patten "
                                                  "\"&H%%2hX%%2hX%%2hX%%2hX%%n\"");
                            goto LABEL_ParsingFailed;
                        }
                        
                        if(alpha > 0xFF || blue > 0xFF || green > 0xFF || red > 0xFF) {
                            if(controlLevel & CFASSFileControlLevelIgnore) {
                                PR_WARN(beginPoint, L"CFASSFileStyle back_colour ABGR[%hu, %hu, %hu, %hu] range overflow, limited in range [0..0xFF]", alpha, blue, green, red);
                                if(alpha > 0xFF) alpha = 0xFF;
                                if(blue  > 0xFF) blue  = 0xFF;
                                if(green > 0xFF) green = 0xFF;
                                if(red   > 0xFF) red   = 0xFF;
                            }
                            else {
                                PR_ERROR(beginPoint, L"CFASSFileStyle back_colour ABGR[%hu, %hu, %hu, %hu] range overflow", alpha, blue, green, red);
                                goto LABEL_ParsingFailed;
                            }
                        }
                        
                        result->back_colour = CFASSFileStyleColorMake(alpha, blue, green, red);
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match blod");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark blod
                        
                        int temp;
                        if(swscanf(beginPoint, L"%d%n", &temp, &scanedAmount) != 1) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing blod failed, patten \"%%d\"");
                            goto LABEL_ParsingFailed;
                        }
                        if(temp == -1)
                            result->blod = true;
                        else if(temp == 0)
                            result->blod = false;
                        else {
                            if(controlLevel & CFASSFileControlLevelIgnore) {
                                if(temp != 0) result->blod = true;
                                else result->blod = false;
                                PR_WARN(beginPoint, L"CFASSFileStyle blod, unrecognized value: %d, auto-correct to %s", temp, temp != 0 ? "-1(true)" : "0(false)");
                            }
                            else {
                                PR_ERROR(beginPoint, L"CFASSFileStyle blod, unrecognized value: %d", temp);
                                goto LABEL_ParsingFailed;
                            }
                        }
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match italic");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark italic
                        
                        if(swscanf(beginPoint, L"%d%n", &temp, &scanedAmount) != 1) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing italic failed, patten \"%%d\"");
                            goto LABEL_ParsingFailed;
                        }
                        if(temp == -1)
                            result->italic = true;
                        else if(temp == 0)
                            result->italic = false;
                        else {
                            if(controlLevel & CFASSFileControlLevelIgnore) {
                                if(temp != 0) result->italic = true;
                                else result->italic = false;
                                PR_WARN(beginPoint, L"CFASSFileStyle italic, unrecognized value: %d, auto-correct to %s", temp, temp != 0 ? "-1(true)" : "0(false)");
                            }
                            else {
                                PR_ERROR(beginPoint, L"CFASSFileStyle italic, unrecognized value: %d", temp);
                                goto LABEL_ParsingFailed;
                            }
                        }
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match underline");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark underline
                        
                        if(swscanf(beginPoint, L"%d%n", &temp, &scanedAmount) != 1) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing underline failed, patten \"%%d\"");
                            goto LABEL_ParsingFailed;
                        }
                        if(temp == -1)
                            result->underline = true;
                        else if(temp == 0)
                            result->underline = false;
                        else {
                            if(controlLevel & CFASSFileControlLevelIgnore) {
                                if(temp != 0) result->underline = true;
                                else result->underline = false;
                                PR_WARN(beginPoint, L"CFASSFileStyle underline, unrecognized value: %d, auto-correct to %s", temp, temp != 0 ? "-1(true)" : "0(false)");
                            }
                            else {
                                PR_ERROR(beginPoint, L"CFASSFileStyle underline, unrecognized value: %d", temp);
                                goto LABEL_ParsingFailed;
                            }
                        }
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match strike_out");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark strike_out
                        
                        if(swscanf(beginPoint, L"%d%n", &temp, &scanedAmount) != 1) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing strike_out failed, patten \"%%d\"");
                            goto LABEL_ParsingFailed;
                        }
                        if(temp == -1)
                            result->strike_out = true;
                        else if(temp == 0)
                            result->strike_out = false;
                        else {
                            if(controlLevel & CFASSFileControlLevelIgnore) {
                                if(temp != 0) result->strike_out = true;
                                else result->strike_out = false;
                                PR_WARN(beginPoint, L"CFASSFileStyle strike_out, unrecognized value: %d, auto-correct to %s", temp, temp != 0 ? "-1(true)" : "0(false)");
                            }
                            else {
                                PR_ERROR(beginPoint, L"CFASSFileStyle strike_out, unrecognized value: %d", temp);
                                goto LABEL_ParsingFailed;
                            }
                        }
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match scale_x");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark scale_x
                        
                        if(swscanf(beginPoint, L"%lf%n", &result->scale_x, &scanedAmount) != 1) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing scale_x failed, patten \"%%lf\"");
                            goto LABEL_ParsingFailed;
                        }
                        if(result->scale_x < 0.0) {
                            if(controlLevel & CFASSFileControlLevelIgnore) {
                                PR_ERROR(beginPoint, L"CFASSFileStyle scale_x, unrecognized value: %f, auto-corrected to 100.0", result->scale_x);
                                result->scale_x = 100.0;
                            }
                            else {
                                PR_ERROR(beginPoint, L"CFASSFileStyle scale_x, unrecognized value: %f", result->scale_x);
                                goto LABEL_ParsingFailed;
                            }
                        }
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match scale_y");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark scale_y
                        
                        if(swscanf(beginPoint, L"%lf%n", &result->scale_y, &scanedAmount) != 1) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing scale_y failed, patten \"%%lf\"");
                            goto LABEL_ParsingFailed;
                        }
                        if(result->scale_y < 0.0) {
                            if(controlLevel & CFASSFileControlLevelIgnore) {
                                PR_WARN(beginPoint, L"CFASSFileStyle scale_y, unrecognized value: %f, auto-corrected to 100.0", result->scale_y);
                                result->scale_y = 100.0;
                            }
                            else {
                                PR_ERROR(beginPoint, L"CFASSFileStyle scale_y, unrecognized value: %f", result->scale_y);
                                goto LABEL_ParsingFailed;
                            }
                        }
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match spacing");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark spacing
                        
                        if(swscanf(beginPoint, L"%lf%n", &result->spacing, &scanedAmount) != 1) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing spacing failed, patten \"%%lf\"");
                            goto LABEL_ParsingFailed;
                        }
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match angle");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark angle
                        
                        if(swscanf(beginPoint, L"%lf%n", &result->angle, &scanedAmount) != 1) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing angle failed, patten \"%%lf\"");
                            goto LABEL_ParsingFailed;
                        }
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match border_style");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark border_style
                        
                        if(swscanf(beginPoint, L"%d%n", &result->border_style, &scanedAmount) != 1) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing border_style failed, patten \"%%d\"");
                            goto LABEL_ParsingFailed;
                        }
                        if(result->border_style != 1 && result->border_style != 3) {
                            if(controlLevel & CFASSFileControlLevelIgnore) {
                                PR_WARN(beginPoint, L"CFASSFileStyle border_style, unrecognized value: %d, auto-corrected to 1(outline + drop shadow)", result->border_style);
                                result->border_style = 1;
                            }
                            else {
                                PR_ERROR(beginPoint, L"CFASSFileStyle border_style, unrecognized value: %d", result->border_style);
                                goto LABEL_ParsingFailed;
                            }
                        }
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match outline");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark outline
                        
                        if(swscanf(beginPoint, L"%u%n", &result->outline, &scanedAmount) != 1) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing outline failed, patten \"%%u\"");
                            goto LABEL_ParsingFailed;
                        }
                        
                        if(result->outline > 4) {
                            if(controlLevel & CFASSFileControlLevelIgnore) {
                                PR_WARN(beginPoint, L"CFASSFileStyle outline, unrecognized value: %u, auto-corrected to 0", result->outline);
                                result->outline = 0u;
                            }
                            else {
                                PR_ERROR(beginPoint, L"CFASSFileStyle outline, unrecognized value: %u", result->outline);
                                goto LABEL_ParsingFailed;
                            }
                        }
                        
                        if(result->border_style == 0 && result->outline != 0) {
                            PR_WARN(beginPoint, L"CFASSFileStyle outline value %u meanless when border_style 0, auto-corrected to 0", result->outline);
                            result->outline = 0u;
                        }
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match shadow");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark shadow
                        
                        if(swscanf(beginPoint, L"%u%n", &result->shadow, &scanedAmount) != 1) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing shadow failed, patten \"%%u\"");
                            goto LABEL_ParsingFailed;
                        }
                        
                        if(result->shadow > 4) {
                            if(controlLevel & CFASSFileControlLevelIgnore) {
                                PR_WARN(beginPoint, L"CFASSFileStyle shadow, unrecognized value: %u, auto-corrected to 0", result->shadow);
                                result->shadow = 0u;
                            }
                            else {
                                PR_ERROR(beginPoint, L"CFASSFileStyle shadow, unrecognized value: %u", result->shadow);
                                goto LABEL_ParsingFailed;
                            }
                        }
                        
                        if(result->border_style == 0 && result->shadow != 0) {
                            PR_WARN(beginPoint, L"CFASSFileStyle shadow value %u meanless when border_style 0, auto-corrected to 0", result->shadow);
                            result->shadow = 0u;
                        }
                        
                        if(result->border_style == 1 && result->shadow != 0 && result->outline == 0) {
                            PR_WARN(beginPoint, L"CFASSFileStyle shadow value %u not zero when border_style 1, auto-corrected outline to 1", result->shadow);
                            result->outline = 1u;
                        }
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match alignment");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark alignment
                        
                        if(swscanf(beginPoint, L"%d%n", &result->alignment, &scanedAmount) != 1) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing alignment failed, patten \"%%d\"");
                            goto LABEL_ParsingFailed;
                        }
                        
                        if(result->alignment < 1 || result->alignment > 9) {
                            if(controlLevel & CFASSFileControlLevelIgnore) {
                                PR_WARN(beginPoint, L"CFASSFileStyle alignment, unrecognized value: %d, auto-corrected to 5", result->alignment);
                                result->alignment = 5;
                            }
                            else {
                                PR_ERROR(beginPoint, L"CFASSFileStyle alignment, unrecognized value: %d", result->alignment);
                                goto LABEL_ParsingFailed;
                            }
                        }
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match marginL");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark marginL
                        
                        if(swscanf(beginPoint, L"%u%n", &result->marginL, &scanedAmount) != 1) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing marginL failed, patten \"%%u\"");
                            goto LABEL_ParsingFailed;
                        }
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match marginR");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark marginR
                        
                        if(swscanf(beginPoint, L"%u%n", &result->marginR, &scanedAmount) != 1) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing marginR failed, patten \"%%u\"");
                            goto LABEL_ParsingFailed;
                        }
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match marginV");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark marginV
                        
                        if(swscanf(beginPoint, L"%u%n", &result->marginV, &scanedAmount) != 1) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing marginV failed, patten \"%%u\"");
                            goto LABEL_ParsingFailed;
                        }
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else {
                            PR_ERROR(beginPoint, L"CFASSFileStyle search next token failed, unable to match encoding");
                            goto LABEL_ParsingFailed;
                        }
                        
#pragma mark encoding
                        
                        if(swscanf(beginPoint, L"%u%n", &result->encoding, &scanedAmount) != 1) {
                            PR_ERROR(beginPoint, L"CFASSFileStyle parsing encoding failed, patten \"%%u\"");
                            goto LABEL_ParsingFailed;
                        }
                        
                        return result;
                        
                        LABEL_ParsingFailed: free(result->font_name);
                        
                        #else
                        
                        while(*beginPoint != L',' && beginPoint < endPoint) beginPoint++;
                        if(*beginPoint == L',')
                        {
                            beginPoint++;
                            
                            int blod, italic, underline, strike_out;
                            
                            unsigned short primary_alpha, primary_blue, primary_green, primary_red;
                            unsigned short secondary_alpha, secondary_blue, secondary_green, secondary_red;
                            unsigned short outline_alpha, outline_blue, outline_green, outline_red;
                            unsigned short back_alpha, back_blue, back_green, back_red;
                            
#pragma mark batch [combined search]
                            
                            int matchCount;
                            if((matchCount = swscanf(beginPoint,
                                       L"%u,"               /* font_size */
                                       L"&H%2hX%2hX%2hX%2hX,&H%2hX%2hX%2hX%2hX,&H%2hX%2hX%2hX%2hX,&H%2hX%2hX%2hX%2hX,"
                                       L"%d,%d,%d,%d,"      /* blod, italic, underline, strike_out */
                                       L"%lf,%lf,"          /* scale_x, scale_y */
                                       L"%lf,"              /* spacing */
                                       L"%lf,"              /* angle */
                                       L"%d,"               /* boarder_style */
                                       L"%u,%u,"            /* outline, shadow */
                                       L"%d,"               /* alignment */
                                       L"%u,%u,%u,"         /* margins */
                                       L"%u",               /* encoding */
                                       &result->font_size,
                                       &primary_alpha, &primary_blue, &primary_green, &primary_red,
                                       &secondary_alpha, &secondary_blue, &secondary_green, &secondary_red,
                                       &outline_alpha, &outline_blue, &outline_green, &outline_red,
                                       &back_alpha, &back_blue, &back_green, &back_red,
                                       &blod, &italic, &underline, &strike_out,
                                       &result->scale_x, &result->scale_y,
                                       &result->spacing,
                                       &result->angle,
                                       &result->border_style,
                                       &result->outline, &result->shadow,
                                       &result->alignment,
                                       &result->marginL, &result->marginR, &result->marginV,
                                       &result->encoding,
                                       &scanedAmount)) == 33)
                            {
                                bool formatCheckFlag = true;        // this should always be true if controlLevel ignored set
                                
#pragma mark format check [font_size]
                                
                                if(result->font_size == 0) {
                                    if(controlLevel & CFASSFileControlLevelIgnore) {
                                        PR_WARN(beginPoint, L"CFASSFileStyle font_size zero, auto set to default 16");
                                        result->font_size = 16;
                                    }
                                    else {
                                        PR_ERROR(beginPoint, L"CFASSFileStyle font_size zero, parsing failed");
                                        formatCheckFlag = false;
                                    }
                                }
                                
#pragma mark format check [primary_colour]
                                
                                if(primary_alpha > 0xFF || primary_blue > 0xFF || primary_green > 0xFF || primary_red > 0xFF) {
                                    if(controlLevel & CFASSFileControlLevelIgnore) {
                                        PR_WARN(beginPoint, L"CFASSFileStyle primary_colour ABGR[%hu, %hu, %hu, %hu] range overflow, limited in range [0..0xFF]",
                                                primary_alpha,
                                                primary_blue,
                                                primary_green,
                                                primary_red);
                                        if(primary_alpha > 0xFF) primary_alpha = 0xFF;
                                        if(primary_blue  > 0xFF) primary_blue  = 0xFF;
                                        if(primary_green > 0xFF) primary_green = 0xFF;
                                        if(primary_red   > 0xFF) primary_red   = 0xFF;
                                    }
                                    else {
                                        PR_ERROR(beginPoint, L"CFASSFileStyle primary_colour ABGR[%hu, %hu, %hu, %hu] range overflow",
                                                 primary_alpha,
                                                 primary_blue,
                                                 primary_green,
                                                 primary_red);
                                        formatCheckFlag = false;
                                    }
                                }
                                
                                if(formatCheckFlag)
                                    result->primary_colour = CFASSFileStyleColorMake(primary_alpha,
                                                                                     primary_blue,
                                                                                     primary_green,
                                                                                     primary_red);
                                
#pragma mark format check [secondary_colour]
                                
                                if(secondary_alpha > 0xFF || secondary_blue > 0xFF || secondary_green > 0xFF || secondary_red > 0xFF) {
                                    if(controlLevel & CFASSFileControlLevelIgnore) {
                                        PR_WARN(beginPoint, L"CFASSFileStyle secondary_colour ABGR[%hu, %hu, %hu, %hu] range overflow, limited in range [0..0xFF]",
                                                secondary_alpha,
                                                secondary_blue,
                                                secondary_green,
                                                secondary_red);
                                        if(secondary_alpha > 0xFF) secondary_alpha = 0xFF;
                                        if(secondary_blue  > 0xFF) secondary_blue  = 0xFF;
                                        if(secondary_green > 0xFF) secondary_green = 0xFF;
                                        if(secondary_red   > 0xFF) secondary_red   = 0xFF;
                                    }
                                    else {
                                        PR_ERROR(beginPoint, L"CFASSFileStyle secondary_colour ABGR[%hu, %hu, %hu, %hu] range overflow",
                                                 secondary_alpha,
                                                 secondary_blue,
                                                 secondary_green,
                                                 secondary_red);
                                        formatCheckFlag = false;
                                    }
                                }
                                
                                if(formatCheckFlag)
                                    result->secondary_colour = CFASSFileStyleColorMake(secondary_alpha,
                                                                                       secondary_blue,
                                                                                       secondary_green,
                                                                                       secondary_red);
                                
#pragma mark format check [outline_colour]
                                
                                if(outline_alpha > 0xFF || outline_blue > 0xFF || outline_green > 0xFF || outline_red > 0xFF) {
                                    if(controlLevel & CFASSFileControlLevelIgnore) {
                                        PR_WARN(beginPoint, L"CFASSFileStyle outline_colour ABGR[%hu, %hu, %hu, %hu] range overflow, limited in range [0..0xFF]",
                                                outline_alpha,
                                                outline_blue,
                                                outline_green,
                                                outline_red);
                                        if(outline_alpha > 0xFF) outline_alpha = 0xFF;
                                        if(outline_blue  > 0xFF) outline_blue  = 0xFF;
                                        if(outline_green > 0xFF) outline_green = 0xFF;
                                        if(outline_red   > 0xFF) outline_red   = 0xFF;
                                    }
                                    else {
                                        PR_ERROR(beginPoint, L"CFASSFileStyle outline_colour ABGR[%hu, %hu, %hu, %hu] range overflow",
                                                 outline_alpha,
                                                 outline_blue,
                                                 outline_green,
                                                 outline_red);
                                        formatCheckFlag = false;
                                    }
                                }
                                
                                if(formatCheckFlag)
                                    result->outline_colour = CFASSFileStyleColorMake(outline_alpha,
                                                                                     outline_blue,
                                                                                     outline_green,
                                                                                     outline_red);
                                
#pragma mark format check [back_colour]
                                
                                if(back_alpha > 0xFF || back_blue > 0xFF || back_green > 0xFF || back_red > 0xFF) {
                                    if(controlLevel & CFASSFileControlLevelIgnore) {
                                        PR_WARN(beginPoint, L"CFASSFileStyle back_colour ABGR[%hu, %hu, %hu, %hu] range overflow, limited in range [0..0xFF]",
                                                back_alpha,
                                                back_blue,
                                                back_green,
                                                back_red);
                                        if(back_alpha > 0xFF) back_alpha = 0xFF;
                                        if(back_blue  > 0xFF) back_blue  = 0xFF;
                                        if(back_green > 0xFF) back_green = 0xFF;
                                        if(back_red   > 0xFF) back_red   = 0xFF;
                                    }
                                    else {
                                        PR_ERROR(beginPoint, L"CFASSFileStyle back_colour ABGR[%hu, %hu, %hu, %hu] range overflow",
                                                 back_alpha,
                                                 back_blue,
                                                 back_green,
                                                 back_red);
                                        formatCheckFlag = false;
                                    }
                                }
                                
                                if(formatCheckFlag)
                                    result->back_colour = CFASSFileStyleColorMake(back_alpha,
                                                                                     back_blue,
                                                                                     back_green,
                                                                                     back_red);

#pragma mark format check [blod]
                                
                                if(blod == -1)
                                    result->blod = true;
                                else if(blod == 0)
                                    result->blod = false;
                                else {
                                    if(controlLevel & CFASSFileControlLevelIgnore) {
                                        if(blod != 0) result->blod = true;
                                        else result->blod = false;
                                        PR_WARN(beginPoint, L"CFASSFileStyle blod, unrecognized value: %d, auto-correct to %s", blod, blod != 0 ? "-1(true)" : "0(false)");
                                    }
                                    else {
                                        PR_ERROR(beginPoint, L"CFASSFileStyle blod, unrecognized value: %d", blod);
                                        formatCheckFlag = false;
                                    }
                                }
                                
#pragma mark format check [italic]
                                
                                if(italic == -1)
                                    result->italic = true;
                                else if(italic == 0)
                                    result->italic = false;
                                else {
                                    if(controlLevel & CFASSFileControlLevelIgnore) {
                                        if(italic != 0) result->italic = true;
                                        else result->italic = false;
                                        PR_WARN(beginPoint, L"CFASSFileStyle italic, unrecognized value: %d, auto-correct to %s", italic, italic != 0 ? "-1(true)" : "0(false)");
                                    }
                                    else {
                                        PR_ERROR(beginPoint, L"CFASSFileStyle italic, unrecognized value: %d", italic);
                                        formatCheckFlag = false;
                                    }
                                }
                                
#pragma mark format check [underline]
                                
                                if(underline == -1)
                                    result->underline = true;
                                else if(underline == 0)
                                    result->underline = false;
                                else {
                                    if(controlLevel & CFASSFileControlLevelIgnore) {
                                        if(underline != 0) result->underline = true;
                                        else result->underline = false;
                                        PR_WARN(beginPoint, L"CFASSFileStyle underline, unrecognized value: %d, auto-correct to %s", underline, underline != 0 ? "-1(true)" : "0(false)");
                                    }
                                    else {
                                        PR_ERROR(beginPoint, L"CFASSFileStyle underline, unrecognized value: %d", underline);
                                        formatCheckFlag = false;
                                    }
                                }
                                
#pragma mark format check [strike_out]
                                
                                if(strike_out == -1)
                                    result->strike_out = true;
                                else if(strike_out == 0)
                                    result->strike_out = false;
                                else {
                                    if(controlLevel & CFASSFileControlLevelIgnore) {
                                        if(strike_out != 0) result->strike_out = true;
                                        else result->strike_out = false;
                                        PR_WARN(beginPoint, L"CFASSFileStyle strike_out, unrecognized value: %d, auto-correct to %s", strike_out, strike_out != 0 ? "-1(true)" : "0(false)");
                                    }
                                    else {
                                        PR_ERROR(beginPoint, L"CFASSFileStyle strike_out, unrecognized value: %d", strike_out);
                                        formatCheckFlag = false;
                                    }
                                }
                                
#pragma mark format check [border_style]
                                
                                if(result->border_style != 1 && result->border_style != 3) {
                                    if(controlLevel & CFASSFileControlLevelIgnore) {
                                        PR_WARN(beginPoint, L"CFASSFileStyle border_style, unrecognized value: %d, auto-corrected to 1(outline + drop shadow)", result->border_style);
                                        result->border_style = 1;
                                    }
                                    else {
                                        PR_ERROR(beginPoint, L"CFASSFileStyle border_style, unrecognized value: %d", result->border_style);
                                        formatCheckFlag = false;
                                    }
                                }
                                
#pragma mark format check [outline]
                                
                                if(result->outline > 4) {
                                    if(controlLevel & CFASSFileControlLevelIgnore) {
                                        PR_WARN(beginPoint, L"CFASSFileStyle outline, unrecognized value: %u, auto-corrected to 0", result->outline);
                                        result->outline = 0u;
                                    }
                                    else {
                                        PR_ERROR(beginPoint, L"CFASSFileStyle outline, unrecognized value: %u", result->outline);
                                        formatCheckFlag = false;
                                    }
                                }
                                
                                if(formatCheckFlag) {
                                    if(result->border_style == 0 && result->outline != 0) {
                                        PR_WARN(beginPoint, L"CFASSFileStyle outline value %u meanless when border_style 0, auto-corrected to 0", result->outline);
                                        result->outline = 0u;
                                    }
                                }
                                
#pragma mark format check [shadow]
                                
                                if(result->shadow > 4) {
                                    if(controlLevel & CFASSFileControlLevelIgnore) {
                                        PR_WARN(beginPoint, L"CFASSFileStyle shadow, unrecognized value: %u, auto-corrected to 0", result->shadow);
                                        result->shadow = 0u;
                                    }
                                    else {
                                        PR_ERROR(beginPoint, L"CFASSFileStyle shadow, unrecognized value: %u", result->shadow);
                                        formatCheckFlag = false;
                                    }
                                }
                                
                                if(formatCheckFlag) {
                                    if(result->border_style == 0 && result->shadow != 0) {
                                        PR_WARN(beginPoint, L"CFASSFileStyle shadow value %u meanless when border_style 0, auto-corrected to 0", result->shadow);
                                        result->shadow = 0u;
                                    }
                                    
                                    if(result->border_style == 1 && result->shadow != 0 && result->outline == 0) {
                                        PR_WARN(beginPoint, L"CFASSFileStyle shadow value %u not zero when border_style 1, auto-corrected outline to 1", result->shadow);
                                        result->outline = 1u;
                                    }
                                }
                                
#pragma mark format check [alignment]
                                
                                if(result->alignment < 1 || result->alignment > 9) {
                                    if(controlLevel & CFASSFileControlLevelIgnore) {
                                        PR_WARN(beginPoint, L"CFASSFileStyle alignment, unrecognized value: %d, auto-corrected to 5", result->alignment);
                                        result->alignment = 5;
                                    }
                                    else {
                                        PR_ERROR(beginPoint, L"CFASSFileStyle alignment, unrecognized value: %d", result->alignment);
                                        formatCheckFlag = false;
                                    }
                                }
                                
                                if(formatCheckFlag) return result;
                            } else PR_ERROR(beginPoint, L"CFASSFileStyle batch patten match failure for [%ls]", CFASSFileStylePattenMatch(matchCount));
                        } else PR_ERROR(beginPoint, L"CFASSFileStyle found font size failed");
                        free(result->font_name);
                        
                        #endif

                    } else PR_ERROR(beginPoint, L"CFASSFileStyle allocate font name failed");
                } else PR_ERROR(beginPoint, L"CFASSFileStyle find font name failed");
                free(result->name);
            } else PR_ERROR(beginPoint, L"CFASSFileStyle find style name failed");
            free(result);
        } else PR_INFO(NULL, L"CFASSFileStyle allocation failed");
    } else PR_ERROR(beginPoint, L"CFASSFileStyle string not begin with \"Style:\"");
    return NULL;
}

static bool CFASSFileStyleCreateWithStringIsSkip(wchar_t input)
{
    if(input == L',') return true;
    else return false;
}

CFASSFileStyleColor CFASSFileStyleColorMake(unsigned char alpha,
                                            unsigned char blue,
                                            unsigned char green,
                                            unsigned char red)
{
    return (CFASSFileStyleColor){alpha, blue, green, red};
}

static const wchar_t * _Nonnull CFASSFileStylePattenMatch(int matchCount) {
    switch (matchCount) {
        case -1: return L"error occur (may not support unicode env)";
         case 0: return L"font size";
         case 1: return L"primary colour alpha";
         case 2: return L"primary colour blue";
         case 3: return L"primary colour green";
         case 4: return L"primary colour red";
         case 5: return L"secondary colour alpha";
         case 6: return L"secondary colour blue";
         case 7: return L"secondary colour green";
         case 8: return L"secondary colour red";
         case 9: return L"outline colour alpha";
        case 10: return L"outline colour blue";
        case 11: return L"outline colour green";
        case 12: return L"outline colour red";
        case 13: return L"back colour alpha";
        case 14: return L"back colour blue";
        case 15: return L"back colour green";
        case 16: return L"back colour red";
        case 17: return L"blod";
        case 18: return L"italic";
        case 19: return L"underline";
        case 20: return L"strike out";
        case 21: return L"scale x";
        case 22: return L"scale y";
        case 23: return L"spacing";
        case 24: return L"angle";
        case 25: return L"boarder style";
        case 26: return L"outline";
        case 27: return L"shadow";
        case 28: return L"alignment";
        case 29: return L"marginL";
        case 30: return L"marginR";
        case 31: return L"marginV";
        case 32: return L"encoding";
        default: DEBUG_POINT; return L"unkown";
    }
}

//
//  CFASSFileDialogueTextContentOverrideContent.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/11.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "CFASSFileDialogueTextContentOverrideContent.h"
#include "CFStringTool.h"
#include "CFException.h"
#include "CFASSFileChange.h"
#include "CFASSFileChange_Private.h"
#include "CFASSFileDialogueTextDrawingContext.h"
#include "CFASSFileDialogueTextDrawingContext_Private.h"
#include "CFASSFileParsingResult.h"
#include "CFASSFileParsingResult_Macro.h"
#include "CFASSFileControl.h"
#include "CFMacro.h"

struct CFASSFileDialogueTextContentOverrideContent {
    CFASSFileDialogueTextContentOverrideContentType type;
    union
    {
        struct
        {
            unsigned int weight;
        }blod;
        struct
        {
            bool isItalic;
        }italic;
        struct
        {
            bool isUnderline;
        }underline;
        struct
        {
            bool isStrikeOut;
        }strikeOut;
        struct
        {
            bool usingComplexed;        // using specific axis X Y
            CF2DAxis axis;
            double resolutionPixels;    // non-negative
        }border;
        struct
        {
            bool usingComplexed;    // using specific axis X Y
            CF2DAxis axis;
            double depth;           // non-negative
        }shadow;
        struct
        {
            bool usingGuassian;
            unsigned int strength;
        }blurEdge;
        struct
        {
            wchar_t *name;
        }fontName;
        struct
        {
            unsigned int resolutionPixels;
        }fontSize;
        struct
        {
            CF2DAxis axis;
            unsigned int percentage;
        }fontScale;
        struct
        {
            double resolutionPixels; // can be negative and have decimal point
        } spacing;
        struct
        {
            bool usingComplexed;    // using specific axis X Y Z
            CF3DAxis axis;
            int degrees;
        } rotation;
        struct
        {
            CF2DAxis axis;
            double factor;
        } shearing;
        struct
        {
            unsigned int identifier;
        }fontEncoding;
        struct
        {
            bool usingComplexed;        // using specific number [1,2,3,4] stand for [primary, secondary, outline, back] but prepare for even further print, and input constraint
            unsigned int componentNumber;
            unsigned char blue, green, red;
        } color;
        struct
        {
            bool setAllComponent;
            unsigned int componentNumber;
            unsigned char transparent;
        } alpha;
        struct
        {
            bool legacy;
            unsigned int position;
        } alignment;
        struct
        {
            bool hasSweepEffect;
            bool removeBorderOutlineBeforeHighlight;
            unsigned int sentiSeconds;
        } karaokeDuration;
        struct
        {
            unsigned int style;
        } wrapStyle;
        struct
        {
            bool resetToDefault;
            wchar_t *styleName;     // NULL for reset to default
        } reset;
        struct
        {
            int x, y;
        } position;
        struct
        {
            int fromX, fromY;
            int toX, toY;
            bool hasTimeControl;
            unsigned int startFromMS, endFromMS;
        } movement;
        struct
        {
            int x, y;
        }rotationOrigin;
        struct
        {
            bool isComplexed;
            unsigned int fadeInMS, fadeOutMS;
            unsigned char beginAlpha, middleAlpha,endAlpha;      // 0-255, decimal
            unsigned int fadeInBeginMS, fadeInEndMS, fadeOutBeginMS, fadeOutEndMS;
        }fade;
        struct
        {
            bool hasTimeOffset;
            unsigned int beginOffsetMS, endOffsetMS;
            bool hasAcceleration;
            unsigned int acceleration;
            wchar_t *modifiers;
        }animation;
        struct
        {
            bool reverse;
            bool usingDrawingCommand;
            bool hasScale;
            int upLeftX, upLeftY;
            int lowRightX, lowRightY;
            unsigned int scale;
            wchar_t *drawingCommand;
        }clip;
        struct
        {
            unsigned int mode;
        }drawing;
        struct
        {
            int towardsBottomPixels;
        }baselineOffset;
    } data;
};

static bool CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(const wchar_t * _Nonnull tokenBegin,
                                                                              const wchar_t * _Nonnull tokenEnd,
                                                                              const wchar_t * _Nonnull matchTypeString);

static bool CFASSFileDialogueTextContentOverrideContentCheckAnimationModifierSupport(CFASSFileDialogueTextContentOverrideContentRef content);

static bool CFASSFileDialogueTextContentOverrideContentCheckAnimationModifiers(wchar_t *modifiers);

#pragma mark - Translation between strings

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideContentCreateWithString(const wchar_t * _Nonnull tokenBegin,
                                                                                                           const wchar_t * _Nullable tokenEnd,
                                                                                                           CFASSFileParsingResultRef _Nonnull parsingResult)
{
    DEBUG_ASSERT(tokenBegin != NULL && parsingResult != NULL);
    if(tokenBegin == NULL || parsingResult == NULL) return NULL;
    if(tokenEnd != NULL && tokenEnd < tokenBegin) return NULL;
    
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        CFASSFileControlLevel controlLevel = CFASSFileControlGetLevel();
        
        int temp;
        if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* italic */
                                                                             tokenEnd,
                                                                             L"i"))
        {
            int isItalic;
            if((temp = swscanf(tokenBegin, L"\\i%d", &isItalic)) == 1) {
                if(isItalic != 0 && isItalic != 1) {
                    if(controlLevel & CFASSFileControlLevelIgnore) {
                        isItalic = 1;   // as is non-zero
                        PR_WARN(tokenBegin, L"unkown italic value, auto-corrected to 1");
                    }
                    else {
                        PR_ERROR(tokenBegin, L"unkown italic value");
                        goto LABEL_failure;
                    }
                }
                result->type = CFASSFileDialogueTextContentOverrideContentTypeItalic;
                result->data.italic.isItalic = isItalic;
                return result;
            } else PR_ERROR(tokenBegin, L"italic match failure, patten \"\\i%%d\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* blod */
                                                                                  tokenEnd,
                                                                                  L"b"))
        {
            int weight;
            if((temp = swscanf(tokenBegin, L"\\b%d", &weight)) == 1) {
                if(weight < 0) {
                    if(controlLevel & CFASSFileControlLevelIgnore) {
                        weight = 16;
                        PR_WARN(tokenBegin, L"unkown blod value, auto-corrected to 16");
                    }
                    else {
                        PR_ERROR(tokenBegin, L"unkown blod value");
                        goto LABEL_failure;
                    }
                }
                result->type = CFASSFileDialogueTextContentOverrideContentTypeBlod;
                result->data.blod.weight = weight;
                return result;
            } else PR_ERROR(tokenBegin, L"blod match failure, patten \"\\b%%d\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* underline */
                                                                                  tokenEnd,
                                                                                  L"u"))
        {
            int isUnderline;
            if((temp = swscanf(tokenBegin, L"\\u%d", &isUnderline)) == 1) {
                if(isUnderline != 0 && isUnderline != 1) {
                    if(controlLevel & CFASSFileControlLevelIgnore) {
                        isUnderline = 1;
                        PR_WARN(tokenBegin, L"unkown underline value, auto-corrected to 1");
                    }
                    else {
                        PR_ERROR(tokenBegin, L"unkown underline value");
                        goto LABEL_failure;
                    }
                }
                result->type = CFASSFileDialogueTextContentOverrideContentTypeUnderline;
                result->data.underline.isUnderline = isUnderline;
                return result;
            } else PR_ERROR(tokenBegin, L"underline match failure, patten \"\\u%%d\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* strikeOut */
                                                                                  tokenEnd,
                                                                                  L"s"))
        {
            int isStrikeOut;
            if((temp = swscanf(tokenBegin, L"\\s%d", &isStrikeOut)) == 1) {
                if(isStrikeOut != 0 && isStrikeOut != 1) {
                    if(controlLevel & CFASSFileControlLevelIgnore) {
                        isStrikeOut = 1;
                        PR_WARN(tokenBegin, L"unkown strikeOut value, auto-corrected to 1");
                    }
                    else {
                        PR_ERROR(tokenBegin, L"unkown strikeOut value");
                        goto LABEL_failure;
                    }
                }
                result->type = CFASSFileDialogueTextContentOverrideContentTypeStrikeOut;
                result->data.strikeOut.isStrikeOut = isStrikeOut;
                return result;
            } else PR_ERROR(tokenBegin, L"strikeOut match failure, patten \"\\s%%d\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* bord */
                                                                                  tokenEnd,
                                                                                  L"bord") ||
                CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* bord x */
                                                                                  tokenEnd,
                                                                                  L"xbord") ||
                CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* bord y */
                                                                                  tokenEnd,
                                                                                  L"ybord"))
        {
            double borderSize;
            result->data.border.usingComplexed = true;  // this means using specific X, Y axis
            
            if((temp = swscanf(tokenBegin, L"\\bord%lf", &borderSize)) == 1)
                result->data.border.usingComplexed = false;
            else if((temp = swscanf(tokenBegin, L"\\xbord%lf", &borderSize)) == 1)
                result->data.border.axis = CF2DAxisX;
            else if((temp = swscanf(tokenBegin, L"\\ybord%lf", &borderSize)) == 1)
                result->data.border.axis = CF2DAxisY;
            
            if(temp == 1)
            {
                if(borderSize < 0.0) {
                    if(controlLevel & CFASSFileControlLevelIgnore) {
                        borderSize = 0.0;
                        PR_WARN(tokenBegin, L"unacceptable borderSize value, auto-corrected to 0.0");
                    }
                    else {
                        PR_ERROR(tokenBegin, L"unacceptable borderSize value");
                        goto LABEL_failure;
                    }
                }
                result->type = CFASSFileDialogueTextContentOverrideContentTypeBorder;
                result->data.border.resolutionPixels = borderSize;
                return result;
            } else PR_ERROR(tokenBegin, L"bord match failure, patten \"\\[x/y]bord%%lf\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* shadow */
                                                                                  tokenEnd,
                                                                                  L"shad") ||
                CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* shadow x */
                                                                                  tokenEnd,
                                                                                  L"xshad") ||
                CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* shadow y */
                                                                                  tokenEnd,
                                                                                  L"yshad"))
        {
            double shadowDepth;
            result->data.shadow.usingComplexed = true;  // this means using specific X, Y axis
            
            if((temp = swscanf(tokenBegin, L"\\shad%lf", &shadowDepth)) == 1)
                result->data.shadow.usingComplexed = false;
            else if((temp = swscanf(tokenBegin, L"\\xshad%lf", &shadowDepth)) == 1)
                result->data.shadow.axis = CF2DAxisX;
            else if((temp = swscanf(tokenBegin, L"\\yshad%lf", &shadowDepth)) == 1)
                result->data.shadow.axis = CF2DAxisY;
            
            if(temp == 1)
            {
                if(shadowDepth < 0.0) {
                    if(controlLevel & CFASSFileControlLevelIgnore) {
                        shadowDepth = 0.0;
                        PR_WARN(tokenBegin, L"unacceptable shadowDepth value, auto-corrected to 0.0");
                    }
                    else {
                        PR_ERROR(tokenBegin, L"unacceptable shadowDepth value");
                        goto LABEL_failure;
                    }
                }
                result->type = CFASSFileDialogueTextContentOverrideContentTypeShadow;
                result->data.shadow.depth = shadowDepth;
                return result;
            } else PR_ERROR(tokenBegin, L"shadowDepth match failure, patten \"\\[x/y]shad%%lf\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* legacy blur */
                                                                                  tokenEnd,
                                                                                  L"be") ||
                CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* guassian blur */
                                                                                  tokenEnd,
                                                                                  L"blur"))
        {
            int strength;
            
            if((temp = swscanf(tokenBegin, L"\\be%d", &strength)) == 1)
                result->data.blurEdge.usingGuassian = false;
            else if((temp = swscanf(tokenBegin, L"\\blur%d", &strength)) == 1)
                result->data.blurEdge.usingGuassian = true;
            
            if(temp == 1)
            {
                if(strength < 0) {
                    if(controlLevel & CFASSFileControlLevelIgnore) {
                        strength = 0;
                        PR_WARN(tokenBegin, L"unacceptable blur value, auto-corrected to 0");
                    }
                    else {
                        PR_ERROR(tokenBegin, L"unacceptable blur value");
                        goto LABEL_failure;
                    }
                }
                result->type = CFASSFileDialogueTextContentOverrideContentTypeBlurEdge;
                result->data.blurEdge.strength = strength;
                return result;
            } else PR_ERROR(tokenBegin, L"blur match failure, patten \"\\(be/blur)%%d\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* font size */
                                                                                  tokenEnd,
                                                                                  L"fs"))
        {
            int resolutionPixels;
            if((temp = swscanf(tokenBegin, L"\\fs%d", &resolutionPixels)) == 1)
            {
                if(resolutionPixels < 0) {
                    if(controlLevel & CFASSFileControlLevelIgnore) {
                        resolutionPixels = 0;
                        PR_WARN(tokenBegin, L"unacceptable fontSize value, auto-corrected to 0");
                    }
                    else {
                        PR_ERROR(tokenBegin, L"unacceptable fontSize value");
                        goto LABEL_failure;
                    }
                }
                result->type = CFASSFileDialogueTextContentOverrideContentTypeFontSize;
                result->data.fontSize.resolutionPixels = resolutionPixels;
                return result;
            } else PR_ERROR(tokenBegin, L"fontSize match failure, patten \"\\fs%%d\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* font scale x */
                                                                                  tokenEnd,
                                                                                  L"fscx") ||
                CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* font scale y */
                                                                                  tokenEnd,
                                                                                  L"fscy"))
        {
            int percentage;
            if((temp = swscanf(tokenBegin, L"\\fscx%d", &percentage)) == 1)
                result->data.fontScale.axis = CF2DAxisX;
            else if((temp = swscanf(tokenBegin, L"\\fscy%d", &percentage)) == 1)
                result->data.fontScale.axis = CF2DAxisY;
            
            if(temp == 1)
            {
                if(percentage < 0) {
                    if(controlLevel & CFASSFileControlLevelIgnore) {
                        percentage = 100;
                        PR_WARN(tokenBegin, L"unacceptable fontScale value, auto-corrected to 100");
                    }
                    else {
                        PR_ERROR(tokenBegin, L"unacceptable fontScale value");
                        goto LABEL_failure;
                    }
                }
                result->type = CFASSFileDialogueTextContentOverrideContentTypeFontScale;
                result->data.fontScale.percentage = percentage;
                return result;
            } else PR_ERROR(tokenBegin, L"fontScale match failure, patten \"\\fsc(x/y)%%d\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* text spacing */
                                                                                  tokenEnd,
                                                                                  L"fsp"))
        {
            double resolutionPixels;
            if((temp = swscanf(tokenBegin, L"\\fsp%lf", &resolutionPixels)) == 1)
            {
                result->type = CFASSFileDialogueTextContentOverrideContentTypeFontSpacing;
                result->data.spacing.resolutionPixels = resolutionPixels;
                return result;
            } else PR_ERROR(tokenBegin, L"textSpacing match failure, patten \"\\fsp%%lf\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* rotation default */
                                                                                  tokenEnd,
                                                                                  L"fr") ||
                CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* rotation x */
                                                                                  tokenEnd,
                                                                                  L"frx") ||
                CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* rotation y */
                                                                                  tokenEnd,
                                                                                  L"fry")  ||
                CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* rotation z */
                                                                                  tokenEnd,
                                                                                  L"frz"))
        {
            int degrees;
            result->data.rotation.usingComplexed = true;
            if((temp = swscanf(tokenBegin, L"\\fr%d", &degrees)) == 1)
                result->data.rotation.usingComplexed = false;
            else if((temp = swscanf(tokenBegin, L"\\frx%d", &degrees)) == 1)
                result->data.rotation.axis = CF3DAxisX;
            else if((temp = swscanf(tokenBegin, L"\\fry%d", &degrees)) == 1)
                result->data.rotation.axis = CF3DAxisY;
            else if((temp = swscanf(tokenBegin, L"\\frz%d", &degrees)) == 1)
                result->data.rotation.axis = CF3DAxisZ;
            if(temp == 1)
            {
                result->type = CFASSFileDialogueTextContentOverrideContentTypeTextRotation;
                result->data.rotation.degrees = degrees;
                return result;
            } else PR_ERROR(tokenBegin, L"rotation match failure, patten \"\\fr[x/y/z]%%d\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* text shearing x */
                                                                                  tokenEnd,
                                                                                  L"fax") ||
                CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* text shearing y */
                                                                                  tokenEnd,
                                                                                  L"fay"))
        {
            double factor;
            if((temp = swscanf(tokenBegin, L"\\fax%lf", &factor)) == 1)
                result->data.shearing.axis = CF2DAxisX;
            else if((temp = swscanf(tokenBegin, L"\\fay%lf", &factor)) == 1)
                result->data.shearing.axis = CF2DAxisY;
            if(temp == 1)
            {
                result->type = CFASSFileDialogueTextContentOverrideContentTypeTextShearing;
                result->data.shearing.factor = factor;
                return result;
            } else PR_ERROR(tokenBegin, L"text shearing match failure, patten \"\\fa(x/y)%%lf\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* font encoding */
                                                                                  tokenEnd,
                                                                                  L"fe"))
        {
            int identifier;
            if((temp = swscanf(tokenBegin, L"\\fe%d", &identifier)) == 1)
            {
                if(identifier < 0) {
                    PR_ERROR(tokenBegin, L"unacceptable fontEncoding value");
                    goto LABEL_failure;
                }
                result->type = CFASSFileDialogueTextContentOverrideContentTypeTextEncoding;
                result->data.fontEncoding.identifier = identifier;
                return result;
            } else PR_ERROR(tokenBegin, L"fontEncoding match failure, patten \"\\fe%%d\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* color default */
                                                                                  tokenEnd,
                                                                                  L"c") ||
                CF_wchar_string_match_beginning(tokenBegin, L"\\1c") ||
                CF_wchar_string_match_beginning(tokenBegin, L"\\2c") ||
                CF_wchar_string_match_beginning(tokenBegin, L"\\3c") ||
                CF_wchar_string_match_beginning(tokenBegin, L"\\4c"))
        {
            int componentNumber;
            unsigned long colorInfo;    // B G R
            bool checkMark = false;
            if((temp = swscanf(tokenBegin, L"\\c&H%lx&", &colorInfo)) == 1)
            {
                result->data.color.usingComplexed = false;
                result->data.color.componentNumber = 0u;
                checkMark = true;
            }
            else if((temp = swscanf(tokenBegin, L"\\%dc&H%lx&", &componentNumber, &colorInfo)) == 2)
                if(componentNumber >= 1 && componentNumber <= 4)
                {
                    result->data.color.usingComplexed = true;
                    result->data.color.componentNumber = componentNumber;
                    checkMark = true;
                }
            
            if(!checkMark) {
                PR_ERROR(tokenBegin, L"color match failure, patten \"\\[1/2/3/4]c&H%%lx&\"");
            }
            else if(colorInfo > 0xFFFFFF) {
                PR_ERROR(tokenBegin, L"color match exceed max value 0xFFFFFF");
            }
            else
            {
                result->type = CFASSFileDialogueTextContentOverrideContentTypeColor;
                result->data.color.red = colorInfo % (0xFF + 1);
                colorInfo /= (0xFF + 1);
                result->data.color.green = colorInfo % (0xFF + 1);
                colorInfo /= (0xFF + 1);
                result->data.color.blue = colorInfo % (0xFF + 1);
                return result;
            }
            
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* alpha all */
                                                                                  tokenEnd,
                                                                                  L"alpha") ||
                CF_wchar_string_match_beginning(tokenBegin, L"\\1a")  ||                           /* alpha 1 */
                CF_wchar_string_match_beginning(tokenBegin, L"\\2a")  ||                           /* alpha 1 */
                CF_wchar_string_match_beginning(tokenBegin, L"\\3a")  ||                           /* alpha 1 */
                CF_wchar_string_match_beginning(tokenBegin, L"\\4a"))                              /* alpha 1 */
        {
            int componentNumber;
            unsigned int transparent_uint;
            if((temp = swscanf(tokenBegin, L"\\alpha&H%X&", &transparent_uint)) == 1)
            {
                if(transparent_uint <= 0xFF)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeAlpha;
                    result->data.alpha.setAllComponent = true;
                    result->data.alpha.componentNumber = 0;
                    result->data.alpha.transparent = transparent_uint;
                    return result;
                } else PR_ERROR(tokenBegin, L"alpha exceed range limit [0 .. 0xFF]");
            }
            else if((temp = swscanf(tokenBegin, L"\\%da&H%X&", &componentNumber, &transparent_uint)) == 2) {
                if(transparent_uint <= 0xFF && componentNumber >= 1 && componentNumber <= 4)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeAlpha;
                    result->data.alpha.setAllComponent = false;
                    result->data.alpha.componentNumber = componentNumber;
                    result->data.alpha.transparent = transparent_uint;
                    return result;
                } else PR_ERROR(tokenBegin, L"alpha exceed limit a(1-4) [0 .. 0xFF]");
            } else PR_ERROR(tokenBegin, L"alpha match failure, patten \"\\[1/2/3/4]a(lpha)&H%%X&\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* alignment legacy */
                                                                                  tokenEnd,
                                                                                  L"a") ||
                CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* alignment numPad style */
                                                                                  tokenEnd,
                                                                                  L"an"))
        {
            int position;
            if((temp = swscanf(tokenBegin, L"\\a%d", &position)) == 1)
                result->data.alignment.legacy = true;
            else if((temp = swscanf(tokenBegin, L"\\an%d", &position)) == 1)
                result->data.alignment.legacy = false;
            
            if(temp == 1)
            {
                if(result->data.alignment.legacy) {
                    /*
                        1: Bottom left
                        2: Bottom center
                        3: Bottom right
                        5: Top left
                        6: Top center
                        7: Top right
                        9: Middle left
                        10: Middle center
                        11: Middle right
                     */
                    if(position != 1 && position != 2 && position != 3 &&
                       position != 5 && position != 6 && position != 7 &&
                       position != 9 && position != 10 && position != 11) {
                        if(controlLevel & CFASSFileControlLevelIgnore) {
                            position = 6;
                            PR_WARN(tokenBegin, L"alignment(legacy) unkown value %d, auto-correct to 6(center)", position);
                        }
                        else {
                            PR_ERROR(tokenBegin, L"alignment(legacy) unkown value %d", position);
                            goto LABEL_failure;
                        }
                    }
                }
                else {
                    /*
                        1: Bottom left
                        2: Bottom center
                        3: Bottom right
                        4: Middle left
                        5: Middle center
                        6: Middle right
                        7: Top left
                        8: Top center
                        9: Top right
                     */
                    if(position < 1 || position > 9) {
                        if(controlLevel & CFASSFileControlLevelIgnore) {
                            position = 5;
                            PR_WARN(tokenBegin, L"alignment(numPad) unkown value %d, auto-correct to 5(center)", position);
                        }
                        else {
                            PR_ERROR(tokenBegin, L"alignment(numPad) unkown value %d", position);
                            goto LABEL_failure;
                        }
                    }
                }
                result->type = CFASSFileDialogueTextContentOverrideContentTypeAlignment;
                result->data.alignment.position = position;
                return result;
            } else PR_ERROR(tokenBegin, L"alignment match failure, patten \"\\a[n]%%d\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* karaoke duration without sweep */
                                                                                  tokenEnd,
                                                                                  L"k") ||
                CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* karaoke duration with sweep */
                                                                                  tokenEnd,
                                                                                  L"K") ||
                CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* karaoke duration with sweep */
                                                                                  tokenEnd,
                                                                                  L"kf") ||
                CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* karaoke duration remove b/s */
                                                                                  tokenEnd,
                                                                                  L"ko"))
        {
            int sentiSeconds;
            if((temp = swscanf(tokenBegin, L"\\k%d", &sentiSeconds)) == 1)
            {
                result->data.karaokeDuration.hasSweepEffect = false;
                result->data.karaokeDuration.removeBorderOutlineBeforeHighlight = false;
            }
            else if((temp = swscanf(tokenBegin, L"\\K%d", &sentiSeconds)) == 1)
            {
                result->data.karaokeDuration.hasSweepEffect = true;
                result->data.karaokeDuration.removeBorderOutlineBeforeHighlight = false;
            }
            else if((temp = swscanf(tokenBegin, L"\\kf%d", &sentiSeconds)) == 1)
            {
                result->data.karaokeDuration.hasSweepEffect = true;
                result->data.karaokeDuration.removeBorderOutlineBeforeHighlight = false;
            }
            else if((temp = swscanf(tokenBegin, L"\\ko%d", &sentiSeconds)) == 1)
            {
                result->data.karaokeDuration.hasSweepEffect = false;
                result->data.karaokeDuration.removeBorderOutlineBeforeHighlight = true;
            }
            if(temp == 1)
            {
                if(sentiSeconds < 0) {
                    if(controlLevel & CFASSFileControlLevelIgnore) {
                        sentiSeconds = 0;
                        PR_WARN(tokenBegin, L"karaoke unacceptable value, auto-correct to 0");
                    }
                    else {
                        PR_ERROR(tokenBegin, L"karaoke unacceptable value");
                        goto LABEL_failure;
                    }
                }
                result->type = CFASSFileDialogueTextContentOverrideContentTypeKaraokeDuration;
                result->data.karaokeDuration.sentiSeconds = sentiSeconds;
                return result;
            } else PR_ERROR(tokenBegin, L"karaoke match failure, patten \"\\[k/K/kr/ko]%%d\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* Wrap style */
                                                                                  tokenEnd,
                                                                                  L"q"))
        {
            int style;
            if((temp = swscanf(tokenBegin, L"\\q%d", &style)) == 1) {
                if(style < 0 || style > 3) {
                    if(controlLevel & CFASSFileControlLevelIgnore) {
                        style = 0;
                        PR_WARN(tokenBegin, L"wrapStyle unacceptable value, auto-correct to 0(smart wrap)");
                    }
                    else {
                        PR_ERROR(tokenBegin, L"wrapStyle unacceptable value");
                        goto LABEL_failure;
                    }
                }
                result->type = CFASSFileDialogueTextContentOverrideContentTypeWrapingStyle;
                result->data.wrapStyle.style = style;
                return result;
            } else PR_ERROR(tokenBegin, L"wrapStyle match failure, patten \"\\q%%d\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* position */
                                                                                  tokenEnd,
                                                                                  L"pos"))
        {
            int x, y;
            if((temp = swscanf(tokenBegin, L"\\pos(%d,%d)", &x, &y)) == 2)
            {
                result->type = CFASSFileDialogueTextContentOverrideContentTypePosition;
                result->data.position.x = x;
                result->data.position.y = y;
                return result;
            } else PR_ERROR(tokenBegin, L"wrapStyle match failure, patten \"\\pos(%%d,%%d)\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* movement */
                                                                                  tokenEnd,
                                                                                  L"move"))
        {
            int fromX, fromY, toX, toY;
            int startFromMS, endFromMS;
            if((temp = swscanf(tokenBegin, L"\\move(%d,%d,%d,%d,%d,%d)", &fromX, &fromY, &toX, &toY, &startFromMS, &endFromMS)) == 6)
            {
                result->data.movement.hasTimeControl = true;
                result->data.movement.startFromMS = startFromMS;
                result->data.movement.endFromMS = endFromMS;
            }
            else if((temp = swscanf(tokenBegin, L"\\move(%d,%d,%d,%d)", &fromX, &fromY, &toX, &toY)) == 4)
            {
                result->data.movement.hasTimeControl = false;
                result->data.movement.startFromMS = 0;
                result->data.movement.endFromMS = 0;
            }
            else temp = 0;  // indicate match failure
            
            if(temp >= 4)
            {
                if(result->data.movement.hasTimeControl) {
                    if(startFromMS < 0 || endFromMS < 0 || startFromMS > endFromMS) {
                        if(controlLevel & CFASSFileControlLevelIgnore) {
                            if(startFromMS < 0) startFromMS = 0;
                            if(endFromMS < 0) endFromMS = 0;
                            if(startFromMS > endFromMS) startFromMS = endFromMS;
                            PR_WARN(tokenBegin, L"movement unacceptable value, auto-correct");
                        }
                        else {
                            PR_ERROR(tokenBegin, L"movement unacceptable value");
                            goto LABEL_failure;
                        }
                    }
                }
                result->type = CFASSFileDialogueTextContentOverrideContentTypeMove;
                result->data.movement.fromX = fromX;
                result->data.movement.fromY = fromY;
                result->data.movement.toX = toX;
                result->data.movement.toY = toY;
                return result;
            } else PR_ERROR(tokenBegin, L"movement match failure, patten \"\\move(%%d,%%d,%%d,%%d[,%%d,%%d])\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* rotation origin */
                                                                                  tokenEnd,
                                                                                  L"org"))
        {
            int x, y;
            if((temp = swscanf(tokenBegin, L"\\org(%d,%d)", &x, &y)) == 2)
            {
                result->type = CFASSFileDialogueTextContentOverrideContentTypeRotationOrigin;
                result->data.rotationOrigin.x = x;
                result->data.rotationOrigin.y = y;
                return result;
            } else PR_ERROR(tokenBegin, L"rotationOrigin match failure, patten \"\\org(%%d,%%d)\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* fade */
                                                                                  tokenEnd,
                                                                                  L"fad") ||
                CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* fade complex */
                                                                                  tokenEnd,
                                                                                  L"fade"))
        {
            int fadeInMS, fadeOutMS,
                beginAlpha, middleAlpha, endAlpha,
                fadeInBeginMS, fadeInEndMS, fadeOutBeginMS, fadeOutEndMS;
            
            if((temp = swscanf(tokenBegin, L"\\fad(%d,%d)", &fadeInMS, &fadeOutMS)) == 2)
            {
                if(fadeInMS < 0 || fadeOutMS < 0 || fadeInMS > fadeOutMS) {
                    if(controlLevel & CFASSFileControlLevelIgnore) {
                        if(fadeInMS < 0) fadeInMS = 0;
                        if(fadeOutMS < 0) fadeOutMS = 0;
                        if(fadeInMS > fadeOutMS) fadeInMS = fadeOutMS;
                        PR_WARN(tokenBegin, L"fade unacceptable value, auto-correct");
                    }
                    else {
                        PR_ERROR(tokenBegin, L"fade unacceptable value");
                        goto LABEL_failure;
                    }
                }
                result->type = CFASSFileDialogueTextContentOverrideContentTypeFade;
                result->data.fade.isComplexed = false;
                result->data.fade.fadeInMS = fadeInMS;
                result->data.fade.fadeOutMS = fadeOutMS;
                result->data.fade.beginAlpha = 0;
                result->data.fade.middleAlpha = 0;
                result->data.fade.endAlpha = 0;
                result->data.fade.fadeInBeginMS = 0;
                result->data.fade.fadeInEndMS = 0;
                result->data.fade.fadeOutBeginMS = 0;
                result->data.fade.fadeOutEndMS = 0;
                return result;
                
            }
            else if((temp = swscanf(tokenBegin, L"\\fade(%d,%d,%d,%d,%d,%d,%d)",
                                    &beginAlpha, &middleAlpha, &endAlpha,
                                    &fadeInBeginMS, &fadeInEndMS,
                                    &fadeOutBeginMS, &fadeOutEndMS)) == 7) {
                if(fadeInMS < 0 || fadeOutMS < 0 || fadeInMS > fadeOutMS) {
                    if(controlLevel & CFASSFileControlLevelIgnore) {
                        if(fadeInMS < 0) fadeInMS = 0;
                        if(fadeOutMS < 0) fadeOutMS = 0;
                        if(fadeInMS > fadeOutMS) fadeInMS = fadeOutMS;
                        PR_WARN(tokenBegin, L"fade unacceptable value, auto-correct");
                    }
                    else {
                        PR_ERROR(tokenBegin, L"fade unacceptable value");
                        goto LABEL_failure;
                    }
                }
                if(beginAlpha > 0xFF) {
                    beginAlpha = 0xFF;
                    PR_WARN(tokenBegin, L"fade beginAlpha unacceptable value, auto-correct to 0xFF");
                } else if(beginAlpha < 0) {
                    beginAlpha = 0;
                    PR_WARN(tokenBegin, L"fade beginAlpha unacceptable value, auto-correct to 0");
                }
                if(middleAlpha > 0xFF) {
                    middleAlpha = 0xFF;
                    PR_WARN(tokenBegin, L"fade middleAlpha unacceptable value, auto-correct to 0xFF");
                } else if(middleAlpha < 0) {
                    middleAlpha = 0;
                    PR_WARN(tokenBegin, L"fade middleAlpha unacceptable value, auto-correct to 0");
                }
                if(endAlpha > 0xFF) {
                    endAlpha = 0xFF;
                    PR_WARN(tokenBegin, L"fade endAlpha unacceptable value, auto-correct to 0xFF");
                } else if(endAlpha < 0) {
                    endAlpha = 0;
                    PR_WARN(tokenBegin, L"fade endAlpha unacceptable value, auto-correct to 0");
                }
                result->type = CFASSFileDialogueTextContentOverrideContentTypeFade;
                result->data.fade.isComplexed = true;
                result->data.fade.fadeInMS = fadeInMS;
                result->data.fade.fadeOutMS = fadeOutMS;
                result->data.fade.beginAlpha = beginAlpha;
                result->data.fade.middleAlpha = middleAlpha;
                result->data.fade.endAlpha = endAlpha;
                result->data.fade.fadeInBeginMS = fadeInBeginMS;
                result->data.fade.fadeInEndMS = fadeInEndMS;
                result->data.fade.fadeOutBeginMS = fadeOutBeginMS;
                result->data.fade.fadeOutEndMS = fadeOutEndMS;
                return result;
            } else PR_ERROR(tokenBegin, L"fade match failure, patten \"\\fad(%%d,%%d)\" or \"\\fade(%%d,%%d,%%d,%%d)\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* animation */
                                                                                  tokenEnd,
                                                                                  L"t"))
        {
            int beginOffsetMS, endOffsetMS;
            int acceleration; int scanAmount = 0;
            if((temp = swscanf(tokenBegin, L"\\t(%d,%d,%d,\\%n", &beginOffsetMS, &endOffsetMS, &acceleration, &scanAmount)) == 3)
            {
                result->data.animation.hasAcceleration = true;
                result->data.animation.hasTimeOffset = true;
                result->data.animation.acceleration = acceleration;
                result->data.animation.beginOffsetMS = beginOffsetMS;
                result->data.animation.endOffsetMS = endOffsetMS;
            }
            else if((temp = swscanf(tokenBegin, L"\\t(%d,%d,\\%n", &beginOffsetMS, &endOffsetMS, &scanAmount)) == 2)
            {
                result->data.animation.hasAcceleration = false;
                result->data.animation.hasTimeOffset = true;
                result->data.animation.beginOffsetMS = beginOffsetMS;
                result->data.animation.endOffsetMS = endOffsetMS;
            }
            else if((temp = swscanf(tokenBegin, L"\\t(%d,\\%n", &acceleration, &scanAmount)) == 1)
            {
                result->data.animation.hasAcceleration = true;
                result->data.animation.hasTimeOffset = false;
                result->data.animation.acceleration = acceleration;
            }
            else temp = 0;      // explicit assign failure value
            
            if(temp >= 1 && scanAmount > 0)
            {
                const wchar_t *dataBeginPoint = tokenBegin + 1;     // safe, previous scaned
                DEBUG_ASSERT(dataBeginPoint[0] == L't');
                
                while(dataBeginPoint[0] != L'\0' &&
                      dataBeginPoint[0] != L'\0' &&
                      dataBeginPoint[0] != L'\\' &&
                      (tokenEnd == NULL ? true : dataBeginPoint < tokenEnd))
                    dataBeginPoint++;
                
                DEBUG_ASSERT(dataBeginPoint[0] == L'\\');           // impossible
                if(dataBeginPoint[0] != L'\\') goto LABEL_failure;
                
                const wchar_t *leftBrace = tokenBegin + 2;          // safe, previous scaned
                DEBUG_ASSERT(leftBrace[0] == L'(');
                
                const wchar_t *matchBrace = CF_match_next_braces(leftBrace, tokenEnd, true);
                if(matchBrace == NULL || matchBrace <= dataBeginPoint) {
                    PR_ERROR(dataBeginPoint, L"animation parsing modifiers failed");
                    goto LABEL_failure;
                }
                
                const wchar_t *dataEndPoint = matchBrace - 1;      // should valid, may equal to dataBeginPoint
                
                if((result->data.animation.modifiers = malloc(sizeof(wchar_t) * (dataEndPoint - dataBeginPoint + 1 + 1))) != NULL)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeAnimation;
                    wmemcpy(result->data.animation.modifiers, dataBeginPoint, dataEndPoint - dataBeginPoint + 1);
                    result->data.animation.modifiers[dataEndPoint-dataBeginPoint+1] = L'\0';
                    if(CFASSFileDialogueTextContentOverrideContentCheckAnimationModifiers(result->data.animation.modifiers))
                        return result;
                    else PR_ERROR(dataBeginPoint, L"animation check failure");
                    free(result->data.animation.modifiers);
                } else PR_INFO(NULL, L"CFASSFileDialogueTextContentOverrideContent animation modifiers allocation failed");
            } else PR_ERROR(tokenBegin, L"animation match failure, patten \"\\t(%%d[,%%d[,%%d]],\(animateContent))\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* clip */
                                                                                  tokenEnd,
                                                                                  L"clip") ||
                CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* clip reversed */
                                                                                  tokenEnd,
                                                                                  L"iclip"))
        {
            int upLeftX, upLeftY;
            int lowRightX, lowRightY;
            int scale;
            int scanAmount = 0;
            const wchar_t *braceBegin = NULL;
            bool checkMark = false;
            if((temp = swscanf(tokenBegin, L"\\clip(%d,%d,%d,%d)%n", &upLeftX, &upLeftY, &lowRightX, &lowRightY, &scanAmount)) == 4 && scanAmount > 0)
            {
                result->data.clip.reverse = false;
                result->data.clip.usingDrawingCommand = false;
                result->data.clip.hasScale = false;
                result->data.clip.upLeftX = upLeftX;
                result->data.clip.upLeftY = upLeftY;
                result->data.clip.lowRightX = lowRightX;
                result->data.clip.lowRightY = lowRightY;
                checkMark = true;
            }
            else if((temp = swscanf(tokenBegin, L"\\iclip(%d,%d,%d,%d)%n", &upLeftX, &upLeftY, &lowRightX, &lowRightY, &scanAmount)) == 4 && scanAmount > 0)
            {
                result->data.clip.reverse = true;
                result->data.clip.usingDrawingCommand = false;
                result->data.clip.hasScale = false;
                result->data.clip.upLeftX = upLeftX;
                result->data.clip.upLeftY = upLeftY;
                result->data.clip.lowRightX = lowRightX;
                result->data.clip.lowRightY = lowRightY;
                checkMark = true;
            }
            else if((temp = swscanf(tokenBegin, L"\\clip(%d,%*l[^)])%n", &scale, &scanAmount)) == 1 && scanAmount > 0)
            {
                braceBegin = tokenBegin;
                while (braceBegin[0] != L'(') braceBegin++;                     // note: may excede tokenEnd
                result->data.clip.reverse = false;
                result->data.clip.usingDrawingCommand = true;
                result->data.clip.hasScale = true;
                result->data.clip.scale = scale;
                checkMark = true;
            }
            else if((temp = swscanf(tokenBegin, L"\\iclip(%d,%*l[^)])%n", &scale)) == 1 && scanAmount > 0)
            {
                braceBegin = tokenBegin;
                while (braceBegin[0] != L'(') braceBegin++;                     // note: may excede tokenEnd
                result->data.clip.reverse = true;
                result->data.clip.usingDrawingCommand = true;
                result->data.clip.hasScale = true;
                result->data.clip.scale = scale;
                checkMark = true;
            }
            else if(CF_wchar_string_match_beginning(tokenBegin, L"\\clip("))
            {
                braceBegin = tokenBegin + wcslen(L"\\clip(") - 1;               // note: may excede tokenEnd
                result->data.clip.reverse = false;
                result->data.clip.usingDrawingCommand = true;
                result->data.clip.hasScale = false;
                checkMark = true;
            }
            else if(CF_wchar_string_match_beginning(tokenBegin, L"\\iclip("))
            {
                braceBegin = tokenBegin + wcslen(L"\\iclip(") - 1;              // note: may excede tokenEnd
                result->data.clip.reverse = true;
                result->data.clip.usingDrawingCommand = true;
                result->data.clip.hasScale = false;
                checkMark = true;
            }
            if(checkMark)       // manage clip.drawingCommand
            {
                result->type = CFASSFileDialogueTextContentOverrideContentTypeClip;
                if(result->data.clip.usingDrawingCommand)
                {
                    DEBUG_ASSERT(braceBegin[0] == L'(');
                    const wchar_t *braceEnd = CF_match_next_braces(braceBegin, tokenEnd, true);
                    if(braceEnd == NULL || braceEnd > tokenEnd) {
                        PR_ERROR(braceBegin, L"animate brace match failure");
                        goto LABEL_failure;
                    }
                    const wchar_t *drawingBegin = braceBegin + 1;
                    
                    if(result->data.clip.hasScale)
                    {
                        while(drawingBegin[0] != L',')
                            if(drawingBegin[0] == L'\0' || drawingBegin[0] == L'\n' || drawingBegin + 1 >= braceEnd) { DEBUG_POINT; goto LABEL_failure; }
                            else drawingBegin++;
                        
                        DEBUG_ASSERT(drawingBegin[0] == L',');
                        drawingBegin++;     // advanced to the drawing begin
                    }
                    
                    const wchar_t *drawingEnd = braceEnd - 1;
                    
                    if(drawingEnd < drawingBegin) {     // drawingBegin == drawingEnd is acceptable
                        PR_ERROR(drawingBegin, L"clip could not find drawing command");
                        goto LABEL_failure;
                    }
                    
                    if((result->data.clip.drawingCommand = malloc(sizeof(wchar_t) * (drawingEnd - drawingBegin + 1 + 1))) != NULL)
                    {
                        wmemcpy(result->data.clip.drawingCommand, drawingBegin, drawingEnd - drawingBegin + 1);
                        result->data.clip.drawingCommand[drawingEnd - drawingBegin + 1] = L'\0';
                        CFASSFileDialogueTextDrawingContextRef drawingContext;
                        if((drawingContext = CFASSFileDialogueTextDrawingContextCreateFromString(result->data.clip.drawingCommand)) != NULL)
                        {
                            CFASSFileDialogueTextDrawingContextDestory(drawingContext);
                            return result;
                        } PR_ERROR(drawingBegin, L"clip not valid drawing command");
                        free(result->data.clip.drawingCommand);
                    } PR_INFO(NULL, L"CFASSFileDialogueTextContentOverrideContent drawingCommand allocation failed");
                }
                else
                {
                    result->data.clip.drawingCommand = NULL;
                    return result;
                }
            } else PR_ERROR(tokenBegin, L"animation match failure, patten \"\\[i]clip([%%d,]<drawing commands>)\" or \"\\[i]clip(%%d,%%d,%%d,%%d)\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* drawing mode */
                                                                                  tokenEnd,
                                                                                  L"p"))
        {
            int mode;
            if((temp = swscanf(tokenBegin, L"\\p%d", &mode)) == 1)
            {
                if(mode < 0) {
                    if(controlLevel & CFASSFileControlLevelIgnore) {
                        mode = 0;
                        PR_WARN(tokenBegin, L"drawingMode unacceptable value, auto-correct to 0(disable)");
                    }
                    else {
                        PR_ERROR(tokenBegin, L"drawingMode unacceptable value");
                        goto LABEL_failure;
                    }
                }
                result->type = CFASSFileDialogueTextContentOverrideContentTypeDrawing;
                result->data.drawing.mode = mode;
                return result;
            } else PR_ERROR(tokenBegin, L"drawingMode match failure, patten \"\\p%%d\"");
        }
        else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(tokenBegin,     /* baseline offset */
                                                                                  tokenEnd,
                                                                                  L"pbo"))
        {
            int towardsBottomPixels;
            if((temp = swscanf(tokenBegin, L"\\pbo%d", &towardsBottomPixels)) == 1)
            {
                result->type = CFASSFileDialogueTextContentOverrideContentTypeBaselineOffset;
                result->data.baselineOffset.towardsBottomPixels = towardsBottomPixels;
                return result;
            } else PR_ERROR(tokenBegin, L"baselineOffset match failure, patten \"\\pbo%%d\"");
        }
        else if(CF_wchar_string_match_beginning(tokenBegin, L"\\fn"))                             /* font name */
        {
            const wchar_t *dataBeginPoint = tokenBegin + wcslen(L"\\fn"), *dataEndPoint;
            if((tokenEnd == NULL ? true : dataBeginPoint <= tokenEnd) &&
               dataBeginPoint[0] != L'\\' &&
               dataBeginPoint[0] != L'}' &&
               dataBeginPoint[0] != L'\0' &&
               dataBeginPoint[0] != L'\n')
            {
                dataEndPoint = dataBeginPoint;
                while((tokenEnd == NULL ? true : dataEndPoint + 1 <= tokenEnd) &&
                      dataEndPoint[1] != L'\\' &&
                      dataEndPoint[1] != L'}' &&
                      dataEndPoint[1] != L'\0' &&
                      dataEndPoint[1] != L'\n')
                    dataEndPoint++;
                DEBUG_ASSERT(tokenEnd == NULL ? true : dataEndPoint == tokenEnd);
                if((result->data.fontName.name = malloc(sizeof(wchar_t) * (dataEndPoint - dataBeginPoint + 1 + 1))) != NULL)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeFontName;
                    wmemcpy(result->data.fontName.name, dataBeginPoint, dataEndPoint - dataBeginPoint + 1);
                    result->data.fontName.name[dataEndPoint - dataBeginPoint + 1] = L'\0';
                    return result;
                } else PR_INFO(NULL, L"CFASSFileDialogueTextContentOverrideContent fontName allocation failed");
            } else PR_ERROR(tokenBegin, L"fontName interprate failed");
        }
        else if(CF_wchar_string_match_beginning(tokenBegin, L"\\r"))                              /* reset */
        {
            const wchar_t *dataBeginPoint = tokenBegin + wcslen(L"\\r"), *dataEndPoint;
            if((tokenEnd == NULL ? true : dataBeginPoint <= tokenEnd) &&
               dataBeginPoint[0] != L'\\' &&
               dataBeginPoint[0] != L'}' &&
               dataBeginPoint[0] != L'\0' &&
               dataBeginPoint[0] != L'\n')
            {
                dataEndPoint = dataBeginPoint;
                while((tokenEnd == NULL ? true : dataEndPoint + 1 <= tokenEnd) &&
                      dataEndPoint[1] != L'\\' &&
                      dataEndPoint[1] != L'}' &&
                      dataEndPoint[1] != L'\0' &&
                      dataEndPoint[1] != L'\n')
                    dataEndPoint++;
                DEBUG_ASSERT(tokenEnd == NULL ? true : dataEndPoint == tokenEnd);
                if((result->data.reset.styleName = malloc(sizeof(wchar_t)*(dataEndPoint - dataBeginPoint + 1 + 1))) != NULL)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeReset;
                    result->data.reset.resetToDefault = false;
                    wmemcpy(result->data.reset.styleName, dataBeginPoint, dataEndPoint - dataBeginPoint + 1);
                    result->data.reset.styleName[dataEndPoint - dataBeginPoint + 1] = L'\0';
                    return result;
                } else PR_INFO(NULL, L"CFASSFileDialogueTextContentOverrideContent style reset allocation failed");
            }
            else
            {
                result->type = CFASSFileDialogueTextContentOverrideContentTypeReset;
                result->data.reset.resetToDefault = true;
                result->data.reset.styleName = NULL;
                return result;
            }
        }
        else PR_ERROR(tokenBegin, L"unkown override type");
        LABEL_failure: free(result);
    }
    return NULL;
}

static bool CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(const wchar_t * _Nonnull tokenBegin,
                                                                              const wchar_t * _Nonnull tokenEnd,
                                                                              const wchar_t * _Nonnull matchTypeString)
{
    DEBUG_ASSERT(tokenBegin != NULL && tokenEnd != NULL && tokenBegin <= tokenEnd && matchTypeString != NULL);
    if(tokenBegin == NULL || tokenEnd == NULL || tokenBegin > tokenEnd || matchTypeString == NULL) return false;
    if(tokenBegin[0] != L'\\') return false;
    
    const wchar_t *matchBegin = tokenBegin + 1;
    size_t matchLength = 0u;
    
    while(matchBegin[matchLength] != L'\0' && matchBegin + matchLength <= tokenEnd && iswalpha(matchBegin[matchLength])) matchLength++;
    size_t matchTypeStringLength = wcslen(matchTypeString);
    
    if(matchTypeStringLength == matchLength && wmemcmp(matchBegin, matchTypeString, matchTypeStringLength) == 0) return true;
    return false;
}

int CFASSFileDialogueTextContentOverrideContentStoreStringResult(CFASSFileDialogueTextContentOverrideContentRef overrideContent, wchar_t *targetPoint)
{
    int result = -1;
    unsigned long colorInfo;
    if(targetPoint == NULL)
    {
        FILE *fp = tmpfile();
        if(fp == NULL) return -1;
        
        switch (overrideContent->type) {
            case CFASSFileDialogueTextContentOverrideContentTypeBlod:
                result = fwprintf(fp, L"\\b%u", overrideContent->data.blod.weight);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeItalic:
                result = fwprintf(fp, L"\\i%lc", overrideContent->data.italic.isItalic?L'1':L'0');
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeUnderline:
                result = fwprintf(fp, L"\\u%lc", overrideContent->data.underline.isUnderline?L'1':L'0');
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeStrikeOut:
                result = fwprintf(fp, L"\\u%lc", overrideContent->data.underline.isUnderline?L'1':L'0');
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeBorder:
                if(overrideContent->data.border.usingComplexed)
                    result = fwprintf(fp, L"\\%lcbord%g",
                                      overrideContent->data.border.axis == CF2DAxisX?L'x':L'y',
                                      overrideContent->data.border.resolutionPixels);
                else
                    result = fwprintf(fp, L"\\bord%g", overrideContent->data.border.resolutionPixels);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeShadow:
                if(overrideContent->data.shadow.usingComplexed)
                    result = fwprintf(fp, L"\\%lcshad%g",
                                      overrideContent->data.shadow.axis == CF2DAxisX ? L'x':L'y',
                                      overrideContent->data.shadow.depth);
                else
                    result = fwprintf(fp, L"\\shad%g", overrideContent->data.shadow.depth);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeBlurEdge:
                if(overrideContent->data.blurEdge.usingGuassian)
                    result = fwprintf(fp, L"\\blur%u", overrideContent->data.blurEdge.strength);
                else
                    result = fwprintf(fp, L"\\be%u", overrideContent->data.blurEdge.strength);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFontName:
                result = fwprintf(fp, L"\\fn%ls", overrideContent->data.fontName.name);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFontSize:
                result = fwprintf(fp, L"\\fs%u", overrideContent->data.fontSize.resolutionPixels);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFontScale:
                result = fwprintf(fp, L"\\fsc%lc%u",
                                  overrideContent->data.fontScale.axis == CF2DAxisX?L'x':L'y',
                                  overrideContent->data.fontScale.percentage);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFontSpacing:
                result = fwprintf(fp, L"\\fsp%g", overrideContent->data.spacing.resolutionPixels);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeTextRotation:
                if(overrideContent->data.rotation.usingComplexed)
                    result = fwprintf(fp, L"\\fr%lc%d",
                                      overrideContent->data.rotation.axis == CF3DAxisX?L'x':
                                      overrideContent->data.rotation.axis == CF3DAxisY?L'y':L'z',
                                      overrideContent->data.rotation.degrees);
                else
                    result = fwprintf(fp, L"\\fr%d", overrideContent->data.rotation.degrees);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeTextShearing:
                result = fwprintf(fp, L"\\fa%lc%g",
                                  overrideContent->data.shearing.axis == CF2DAxisX?L'x':L'y',
                                  overrideContent->data.shearing.factor);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeTextEncoding:
                result = fwprintf(fp, L"\\fe%u", overrideContent->data.fontEncoding.identifier);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeColor:
                colorInfo =
                      overrideContent->data.color.red
                    + overrideContent->data.color.green*(0xFF+1)
                    + overrideContent->data.color.blue *(0xFFFF+1);
                if(overrideContent->data.color.usingComplexed)
                    result = fwprintf(fp, L"\\%uc&H%lX&",
                                      overrideContent->data.color.componentNumber,
                                      colorInfo);
                else
                    result = fwprintf(fp, L"\\c&H%lX&", colorInfo);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeAlpha:
                if(overrideContent->data.alpha.setAllComponent)
                    result = fwprintf(fp, L"\\alpha&H%X&", (unsigned int)overrideContent->data.alpha.transparent);
                else
                    result = fwprintf(fp, L"\\%ua&H%X&",
                                      overrideContent->data.alpha.componentNumber,
                                      (unsigned int)overrideContent->data.alpha.transparent);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeAlignment:
                if(overrideContent->data.alignment.legacy)
                    result = fwprintf(fp, L"\\a%u", overrideContent->data.alignment.position);
                else
                    result = fwprintf(fp, L"\\an%u", overrideContent->data.alignment.position);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeKaraokeDuration:
                if(overrideContent->data.karaokeDuration.hasSweepEffect)
                    result = fwprintf(fp, L"\\kf%u", overrideContent->data.karaokeDuration.sentiSeconds);
                else if(overrideContent->data.karaokeDuration.removeBorderOutlineBeforeHighlight)
                    result = fwprintf(fp, L"\\ko%u", overrideContent->data.karaokeDuration.sentiSeconds);
                else
                    result = fwprintf(fp, L"\\k%u", overrideContent->data.karaokeDuration.sentiSeconds);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeWrapingStyle:
                result = fwprintf(fp, L"\\q%u", overrideContent->data.wrapStyle.style);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeReset:
                if(overrideContent->data.reset.resetToDefault)
                    result = fwprintf(fp, L"\\r");
                else
                    result = fwprintf(fp, L"\\r%ls", overrideContent->data.reset.styleName);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeAnimation:
                if(overrideContent->data.animation.hasTimeOffset && overrideContent->data.animation.hasAcceleration)
                    result = fwprintf(fp, L"\\t(%u,%u,%u,%ls)",
                                      overrideContent->data.animation.beginOffsetMS,
                                      overrideContent->data.animation.endOffsetMS,
                                      overrideContent->data.animation.acceleration,
                                      overrideContent->data.animation.modifiers);
                else if(overrideContent->data.animation.hasTimeOffset && !overrideContent->data.animation.hasAcceleration)
                    result = fwprintf(fp, L"\\t(%u,%u,%ls)",
                                      overrideContent->data.animation.beginOffsetMS,
                                      overrideContent->data.animation.endOffsetMS,
                                      overrideContent->data.animation.modifiers);
                else if(!overrideContent->data.animation.hasTimeOffset && overrideContent->data.animation.hasAcceleration)
                    result = fwprintf(fp, L"\\t(%u,%ls)",
                                      overrideContent->data.animation.acceleration,
                                      overrideContent->data.animation.modifiers);
                else
                    result = fwprintf(fp, L"\\t(%ls)", overrideContent->data.animation.modifiers);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeMove:
                if(overrideContent->data.movement.hasTimeControl)
                    result = fwprintf(fp, L"\\move(%d,%d,%d,%d,%u,%u)",
                                      overrideContent->data.movement.fromX,
                                      overrideContent->data.movement.fromY,
                                      overrideContent->data.movement.toX,
                                      overrideContent->data.movement.toY,
                                      overrideContent->data.movement.startFromMS,
                                      overrideContent->data.movement.endFromMS);
                else
                    result = fwprintf(fp, L"\\move(%d,%d,%d,%d)",
                                      overrideContent->data.movement.fromX,
                                      overrideContent->data.movement.fromY,
                                      overrideContent->data.movement.toX,
                                      overrideContent->data.movement.toY);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypePosition:
                result = fwprintf(fp, L"\\pos(%d,%d)", overrideContent->data.position.x, overrideContent->data.position.y);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeRotationOrigin:
                result = fwprintf(fp, L"\\org(%d,%d)",
                                  overrideContent->data.rotationOrigin.x,
                                  overrideContent->data.rotationOrigin.y);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFade:
                if(overrideContent->data.fade.isComplexed)
                    result = fwprintf(fp, L"\\fade(%u,%u,%u,%u,%u,%u,%u)",
                                      (unsigned int)overrideContent->data.fade.beginAlpha,
                                      (unsigned int)overrideContent->data.fade.middleAlpha,
                                      (unsigned int)overrideContent->data.fade.endAlpha,
                                      overrideContent->data.fade.fadeInBeginMS,
                                      overrideContent->data.fade.fadeInEndMS,
                                      overrideContent->data.fade.fadeOutBeginMS,
                                      overrideContent->data.fade.fadeOutEndMS);
                else
                    result = fwprintf(fp, L"\\fad(%u,%u)",
                                      overrideContent->data.fade.fadeInMS,
                                      overrideContent->data.fade.fadeOutMS);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeClip:
                if(overrideContent->data.clip.usingDrawingCommand)
                    if(overrideContent->data.clip.hasScale)
                        result = fwprintf(fp, L"\\%lsclip(%u,%ls)",
                                          overrideContent->data.clip.reverse?L"i":L"",
                                          overrideContent->data.clip.scale,
                                          overrideContent->data.clip.drawingCommand);
                    else
                        result = fwprintf(fp, L"\\%lsclip(%ls)",
                                          overrideContent->data.clip.reverse?L"i":L"",
                                          overrideContent->data.clip.drawingCommand);
                else
                    result = fwprintf(fp, L"\\%lsclip(%d,%d,%d,%d)",
                                      overrideContent->data.clip.reverse?L"i":L"",
                                      overrideContent->data.clip.upLeftX,
                                      overrideContent->data.clip.upLeftY,
                                      overrideContent->data.clip.lowRightX,
                                      overrideContent->data.clip.lowRightY);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeDrawing:
                result = fwprintf(fp, L"\\p%u", overrideContent->data.drawing.mode);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeBaselineOffset:
                result = fwprintf(fp, L"\\pbo%d", overrideContent->data.baselineOffset.towardsBottomPixels);
                break;
        }
        fclose(fp);
    }
    else
        switch (overrideContent->type) {
            case CFASSFileDialogueTextContentOverrideContentTypeBlod:
                result = swprintf(targetPoint,SIZE_MAX, L"\\b%u", overrideContent->data.blod.weight);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeItalic:
                result = swprintf(targetPoint,SIZE_MAX, L"\\i%lc", overrideContent->data.italic.isItalic?L'1':L'0');
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeUnderline:
                result = swprintf(targetPoint,SIZE_MAX, L"\\u%lc", overrideContent->data.underline.isUnderline?L'1':L'0');
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeStrikeOut:
                result = swprintf(targetPoint,SIZE_MAX, L"\\u%lc", overrideContent->data.underline.isUnderline?L'1':L'0');
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeBorder:
                if(overrideContent->data.border.usingComplexed)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\%lcbord%g",
                                      overrideContent->data.border.axis == CF2DAxisX?L'x':L'y',
                                      overrideContent->data.border.resolutionPixels);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\bord%g", overrideContent->data.border.resolutionPixels);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeShadow:
                if(overrideContent->data.shadow.usingComplexed)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\%lcshad%g",
                                      overrideContent->data.shadow.axis == CF2DAxisX ? L'x':L'y',
                                      overrideContent->data.shadow.depth);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\shad%g", overrideContent->data.shadow.depth);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeBlurEdge:
                if(overrideContent->data.blurEdge.usingGuassian)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\blur%u", overrideContent->data.blurEdge.strength);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\be%u", overrideContent->data.blurEdge.strength);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFontName:
                result = swprintf(targetPoint,SIZE_MAX, L"\\fn%ls", overrideContent->data.fontName.name);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFontSize:
                result = swprintf(targetPoint,SIZE_MAX, L"\\fs%u", overrideContent->data.fontSize.resolutionPixels);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFontScale:
                result = swprintf(targetPoint,SIZE_MAX, L"\\fsc%lc%u",
                                  overrideContent->data.fontScale.axis == CF2DAxisX?L'x':L'y',
                                  overrideContent->data.fontScale.percentage);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFontSpacing:
                result = swprintf(targetPoint,SIZE_MAX, L"\\fsp%g", overrideContent->data.spacing.resolutionPixels);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeTextRotation:
                if(overrideContent->data.rotation.usingComplexed)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\fr%lc%d",
                                      overrideContent->data.rotation.axis == CF3DAxisX?L'x':
                                      overrideContent->data.rotation.axis == CF3DAxisY?L'y':L'z',
                                      overrideContent->data.rotation.degrees);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\fr%d", overrideContent->data.rotation.degrees);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeTextShearing:
                result = swprintf(targetPoint,SIZE_MAX, L"\\fa%lc%g",
                                  overrideContent->data.shearing.axis == CF2DAxisX?L'x':L'y',
                                  overrideContent->data.shearing.factor);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeTextEncoding:
                result = swprintf(targetPoint,SIZE_MAX, L"\\fe%u", overrideContent->data.fontEncoding.identifier);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeColor:
                colorInfo =
                overrideContent->data.color.red
                + overrideContent->data.color.green*(0xFF+1)
                + overrideContent->data.color.blue *(0xFFFF+1);
                if(overrideContent->data.color.usingComplexed)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\%uc&H%lX&",
                                      overrideContent->data.color.componentNumber,
                                      colorInfo);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\c&H%lX&", colorInfo);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeAlpha:
                if(overrideContent->data.alpha.setAllComponent)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\alpha&H%X&", (unsigned int)overrideContent->data.alpha.transparent);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\%ua&H%X&",
                                      overrideContent->data.alpha.componentNumber,
                                      (unsigned int)overrideContent->data.alpha.transparent);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeAlignment:
                if(overrideContent->data.alignment.legacy)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\a%u", overrideContent->data.alignment.position);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\an%u", overrideContent->data.alignment.position);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeKaraokeDuration:
                if(overrideContent->data.karaokeDuration.hasSweepEffect)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\kf%u", overrideContent->data.karaokeDuration.sentiSeconds);
                else if(overrideContent->data.karaokeDuration.removeBorderOutlineBeforeHighlight)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\ko%u", overrideContent->data.karaokeDuration.sentiSeconds);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\k%u", overrideContent->data.karaokeDuration.sentiSeconds);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeWrapingStyle:
                result = swprintf(targetPoint,SIZE_MAX, L"\\q%u", overrideContent->data.wrapStyle.style);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeReset:
                if(overrideContent->data.reset.resetToDefault)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\r");
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\r%ls", overrideContent->data.reset.styleName);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeAnimation:
                if(overrideContent->data.animation.hasTimeOffset && overrideContent->data.animation.hasAcceleration)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\t(%u,%u,%u,%ls)",
                                      overrideContent->data.animation.beginOffsetMS,
                                      overrideContent->data.animation.endOffsetMS,
                                      overrideContent->data.animation.acceleration,
                                      overrideContent->data.animation.modifiers);
                else if(overrideContent->data.animation.hasTimeOffset && !overrideContent->data.animation.hasAcceleration)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\t(%u,%u,%ls)",
                                      overrideContent->data.animation.beginOffsetMS,
                                      overrideContent->data.animation.endOffsetMS,
                                      overrideContent->data.animation.modifiers);
                else if(!overrideContent->data.animation.hasTimeOffset && overrideContent->data.animation.hasAcceleration)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\t(%u,%ls)",
                                      overrideContent->data.animation.acceleration,
                                      overrideContent->data.animation.modifiers);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\t(%ls)", overrideContent->data.animation.modifiers);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeMove:
                if(overrideContent->data.movement.hasTimeControl)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\move(%d,%d,%d,%d,%u,%u)",
                                      overrideContent->data.movement.fromX,
                                      overrideContent->data.movement.fromY,
                                      overrideContent->data.movement.toX,
                                      overrideContent->data.movement.toY,
                                      overrideContent->data.movement.startFromMS,
                                      overrideContent->data.movement.endFromMS);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\move(%d,%d,%d,%d)",
                                      overrideContent->data.movement.fromX,
                                      overrideContent->data.movement.fromY,
                                      overrideContent->data.movement.toX,
                                      overrideContent->data.movement.toY);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypePosition:
                result = swprintf(targetPoint,SIZE_MAX, L"\\pos(%d,%d)", overrideContent->data.position.x, overrideContent->data.position.y);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeRotationOrigin:
                result = swprintf(targetPoint,SIZE_MAX, L"\\org(%d,%d)",
                                  overrideContent->data.rotationOrigin.x,
                                  overrideContent->data.rotationOrigin.y);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFade:
                if(overrideContent->data.fade.isComplexed)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\fade(%u,%u,%u,%u,%u,%u,%u)",
                                      (unsigned int)overrideContent->data.fade.beginAlpha,
                                      (unsigned int)overrideContent->data.fade.middleAlpha,
                                      (unsigned int)overrideContent->data.fade.endAlpha,
                                      overrideContent->data.fade.fadeInBeginMS,
                                      overrideContent->data.fade.fadeInEndMS,
                                      overrideContent->data.fade.fadeOutBeginMS,
                                      overrideContent->data.fade.fadeOutEndMS);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\fad(%u,%u)",
                                      overrideContent->data.fade.fadeInMS,
                                      overrideContent->data.fade.fadeOutMS);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeClip:
                if(overrideContent->data.clip.usingDrawingCommand)
                    if(overrideContent->data.clip.hasScale)
                        result = swprintf(targetPoint,SIZE_MAX, L"\\%lsclip(%u,%ls)",
                                          overrideContent->data.clip.reverse?L"i":L"",
                                          overrideContent->data.clip.scale,
                                          overrideContent->data.clip.drawingCommand);
                    else
                        result = swprintf(targetPoint,SIZE_MAX, L"\\%lsclip(%ls)",
                                          overrideContent->data.clip.reverse?L"i":L"",
                                          overrideContent->data.clip.drawingCommand);
                    else
                        result = swprintf(targetPoint,SIZE_MAX, L"\\%lsclip(%d,%d,%d,%d)",
                                          overrideContent->data.clip.reverse?L"i":L"",
                                          overrideContent->data.clip.upLeftX,
                                          overrideContent->data.clip.upLeftY,
                                          overrideContent->data.clip.lowRightX,
                                          overrideContent->data.clip.lowRightY);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeDrawing:
                result = swprintf(targetPoint,SIZE_MAX, L"\\p%u", overrideContent->data.drawing.mode);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeBaselineOffset:
                result = swprintf(targetPoint,SIZE_MAX, L"\\pbo%d", overrideContent->data.baselineOffset.towardsBottomPixels);
                break;
        }
    if(result < 0) return -1;
    return result;
}

void CFASSFileDialogueTextContentOverrideContentDestory(CFASSFileDialogueTextContentOverrideContentRef overrideContent)
{
    if(overrideContent == NULL) return;
    // Clip.drawingCommand  may exist
    // Animation.modifiers
    // Reset.styleName      may exist
    // FontName.name
    if(overrideContent->type == CFASSFileDialogueTextContentOverrideContentTypeClip)
    {
        if(overrideContent->data.clip.usingDrawingCommand)
            free(overrideContent->data.clip.drawingCommand);
    }
    else if(overrideContent->type == CFASSFileDialogueTextContentOverrideContentTypeAnimation)
        free(overrideContent->data.animation.modifiers);
    else if(overrideContent->type == CFASSFileDialogueTextContentOverrideContentTypeReset)
    {
        if(!overrideContent->data.reset.resetToDefault)
            free(overrideContent->data.reset.styleName);
    }
    else if(overrideContent->type == CFASSFileDialogueTextContentOverrideContentTypeFontName)
        free(overrideContent->data.fontName.name);
    free(overrideContent);
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideContentCopy(CFASSFileDialogueTextContentOverrideContentRef overrideContent)
{
    // fontName reset animation clip
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        *result = *overrideContent;
        bool checkMark = true;
        switch (overrideContent->type) {
            case CFASSFileDialogueTextContentOverrideContentTypeFontName:
                if((result->data.fontName.name = CF_Dump_wchar_string(overrideContent->data.fontName.name)) == NULL)
                    checkMark = false;
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeReset:
                if(!overrideContent->data.reset.resetToDefault)
                    if((result->data.reset.styleName = CF_Dump_wchar_string(overrideContent->data.reset.styleName)) == NULL)
                        checkMark = false;
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeAnimation:
                if((result->data.animation.modifiers = CF_Dump_wchar_string(overrideContent->data.animation.modifiers)) == NULL)
                    checkMark = false;
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeClip:
                if(overrideContent->data.clip.usingDrawingCommand)
                    if((result->data.clip.drawingCommand = CF_Dump_wchar_string(overrideContent->data.clip.drawingCommand)) == NULL)
                        checkMark = false;
                break;
            default:
                // do nothing
                break;
        }
        if(checkMark)
            return result;
        free(result);
    }
    return NULL;
}

#pragma mark - Specific Override Context create

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideBlodContentCreate(unsigned int blod)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeBlod;
        result->data.blod.weight = blod;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideItalicContentCreate(bool isItalic)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeItalic;
        result->data.italic.isItalic = isItalic;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideUnderlineContentCreate(bool isUnderline)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeUnderline;
        result->data.underline.isUnderline = isUnderline;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideStrikeOutContentCreate(bool isStrikeOut)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeStrikeOut;
        result->data.strikeOut.isStrikeOut = isStrikeOut;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideBorderSizeContentCreate(bool usingComplexed,
                                                                                                           CF2DAxis axis,
                                                                                                           double resolutionPixels)
{
    if(resolutionPixels<0) return NULL;
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeBorder;
        result->data.border.usingComplexed = usingComplexed;
        result->data.border.axis = axis;
        result->data.border.resolutionPixels = resolutionPixels;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideShadowContentCreate(bool usingComplexed,
                                                                                                       CF2DAxis axis,
                                                                                                       double depth)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeShadow;
        result->data.shadow.usingComplexed = usingComplexed;
        result->data.shadow.axis = axis;
        result->data.shadow.depth = depth;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideBlurEdgeContentCreate(bool usingGuassian,
                                                                                                         unsigned int strength)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeBlurEdge;
        result->data.blurEdge.usingGuassian = usingGuassian;
        result->data.blurEdge.strength = strength;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideFontNameContentCreate(wchar_t *fontName)
{
    if(fontName == NULL) return NULL;
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        if((result->data.fontName.name = CF_Dump_wchar_string(fontName)) != NULL)
        {
            result->type = CFASSFileDialogueTextContentOverrideContentTypeFontName;
            return result;
        }
        free(result);
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideFontSizeContentCreate(unsigned int resolutionPixels)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeFontSize;
        result->data.fontSize.resolutionPixels = resolutionPixels;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideFontScaleContentCreate(CF2DAxis axis,
                                                                                                          unsigned int percentage)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeFontScale;
        result->data.fontScale.axis = axis;
        result->data.fontScale.percentage = percentage;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideFontSpacingContentCreate(double resolutionPixels)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeFontSpacing;
        result->data.spacing.resolutionPixels = resolutionPixels;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideTextRotationContentCreate(bool defineAxis,
                                                                                                             CF3DAxis axis,
                                                                                                             int degrees)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeTextRotation;
        result->data.rotation.usingComplexed = defineAxis;
        result->data.rotation.axis = axis;
        result->data.rotation.degrees = degrees;
        
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideTextShearingContentCreate(CF2DAxis axis,
                                                                                                             double factor)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeTextShearing;
        result->data.shearing.axis = axis;
        result->data.shearing.factor = factor;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideTextEncodingContentCreate(unsigned int textEncoding)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeTextEncoding;
        result->data.fontEncoding.identifier = textEncoding;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideColorContentCreate(bool definesWhichColor,
                                                                                                      unsigned int colorCount,
                                                                                                      unsigned char blue,
                                                                                                      unsigned char green,
                                                                                                      unsigned char red)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeColor;
        result->data.color.usingComplexed = definesWhichColor;
        result->data.color.componentNumber = colorCount;
        result->data.color.blue = blue;
        result->data.color.green = green;
        result->data.color.red = red;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideAlphaContentCreate(bool setAllComponent,
                                                                                                      unsigned int alphaCount,
                                                                                                      unsigned char alpha)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeAlpha;
        result->data.alpha.setAllComponent = setAllComponent;
        result->data.alpha.componentNumber = alphaCount;
        result->data.alpha.transparent = alpha;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideAlignmentContentCreate(bool isLegacyStyle, unsigned int alignmentPosition)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeAlignment;
        result->data.alignment.legacy = isLegacyStyle;
        result->data.alignment.position = alignmentPosition;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideKaraokeDurationContentCreate(bool isSweep, unsigned int centiSeconds, bool removeBorderAndShadow)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeKaraokeDuration;
        result->data.karaokeDuration.hasSweepEffect = isSweep;
        result->data.karaokeDuration.sentiSeconds = centiSeconds;
        result->data.karaokeDuration.removeBorderOutlineBeforeHighlight = removeBorderAndShadow;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideWrapStyleContentCreate(unsigned int wrapStyle)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeWrapingStyle;
        result->data.wrapStyle.style = wrapStyle;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideResetContentCreate(bool resetToDefault, wchar_t *styleName)
{
    if(!resetToDefault && styleName == NULL) return NULL;
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        if((result->data.reset.styleName = CF_Dump_wchar_string(styleName)) != NULL)
        {
            result->type = CFASSFileDialogueTextContentOverrideContentTypeReset;
            result->data.reset.resetToDefault = resetToDefault;
            if(!resetToDefault)
            {
                if((result->data.reset.styleName = CF_Dump_wchar_string(styleName)) != NULL)
                    return result;
            }
            else
                return result;
        }
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverridePositionContentCreate(int x, int y)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypePosition;
        result->data.position.x = x;
        result->data.position.y = y;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideMovementContentCreate(int fromX,
                                                                                                         int fromY,
                                                                                                         int toX,
                                                                                                         int toY,
                                                                                                         bool hasTimeControl,
                                                                                                         unsigned int startFromMS,
                                                                                                         unsigned int endFromMS)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeMove;
        result->data.movement.fromX = fromX;
        result->data.movement.fromY = fromY;
        result->data.movement.toX = toX;
        result->data.movement.toY = toY;
        result->data.movement.hasTimeControl = hasTimeControl;
        result->data.movement.startFromMS = startFromMS;
        result->data.movement.endFromMS = endFromMS;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideRotationOriginContentCreate(int x, int y)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeRotationOrigin;
        result->data.rotationOrigin.x = x;
        result->data.rotationOrigin.y = y;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideFadeContentCreate(bool isComplexed,
                                                                                                     unsigned int fadeInMS,
                                                                                                     unsigned int fadeOutMS,
                                                                                                     unsigned char beginAlpha,
                                                                                                     unsigned char middleAlpha,
                                                                                                     unsigned char endAlpha,
                                                                                                     unsigned int fadeInBeginMS,
                                                                                                     unsigned int fadeInEndMS,
                                                                                                     unsigned int fadeOutBeginMS,
                                                                                                     unsigned int fadeOutEndMS)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeFade;
        result->data.fade.isComplexed = isComplexed;
        result->data.fade.fadeInMS = fadeInMS;
        result->data.fade.fadeOutMS = fadeOutMS;
        result->data.fade.beginAlpha = beginAlpha;
        result->data.fade.middleAlpha = middleAlpha;
        result->data.fade.endAlpha = endAlpha;
        result->data.fade.fadeInBeginMS = fadeInBeginMS;
        result->data.fade.fadeInEndMS = fadeInEndMS;
        result->data.fade.fadeOutBeginMS = fadeOutBeginMS;
        result->data.fade.fadeOutEndMS = fadeOutEndMS;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideAnimationContentCreate(bool hasTimeOffset,
                                                                                                          unsigned int beginOffsetMS,
                                                                                                          unsigned int endOffsetMS,
                                                                                                          bool hasAcceleration,
                                                                                                          unsigned int acceleration,
                                                                                                          const wchar_t *modifiers)
{
    if(modifiers == NULL) return NULL;
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        if((result->data.animation.modifiers = CF_Dump_wchar_string(modifiers)) != NULL)
        {
            result->type = CFASSFileDialogueTextContentOverrideContentTypeAnimation;
            result->data.animation.hasTimeOffset = hasTimeOffset;
            result->data.animation.beginOffsetMS = beginOffsetMS;
            result->data.animation.endOffsetMS = endOffsetMS;
            result->data.animation.hasAcceleration = hasAcceleration;
            result->data.animation.acceleration = acceleration;
            return result;
        }
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideClipContentCreate(bool reverse,
                                                                                                     bool usingDrawingCommand,
                                                                                                     bool hasScale,
                                                                                                     int upLeftX, int upLeftY,
                                                                                                     int lowRightX, int lowRightY,
                                                                                                     unsigned int scale,
                                                                                                     wchar_t *drawingCommand)
{
    if(usingDrawingCommand && drawingCommand == NULL) return NULL;
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeClip;
        result->data.clip.usingDrawingCommand = usingDrawingCommand;
        result->data.clip.hasScale = hasScale;
        result->data.clip.upLeftX = upLeftX;
        result->data.clip.upLeftY = upLeftY;
        result->data.clip.lowRightX = lowRightX;
        result->data.clip.lowRightY = lowRightY;
        result->data.clip.scale = scale;
        if(usingDrawingCommand)
        {
            if((result->data.clip.drawingCommand = CF_Dump_wchar_string(drawingCommand)) != NULL)
                return result;
        }
        else
        {
            result->data.clip.drawingCommand = NULL;
            return result;
        }
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideDrawingContentCreate(unsigned int drawingMode)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeDrawing;
        result->data.drawing.mode = drawingMode;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideBaselineOffsetContentCreate(int towardsBottomPixels)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeBaselineOffset;
        result->data.baselineOffset.towardsBottomPixels = towardsBottomPixels;
        return result;
    }
    return NULL;
}

#pragma mark - Other function

void CFASSFileDialogueTextContentOverrideContentMakeChange(CFASSFileDialogueTextContentOverrideContentRef overrideContent, CFASSFileChangeRef change)
{
    if(overrideContent == NULL || change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContentOverrideContent %p MakeChange %p", overrideContent, change);
    if(change->type & CFASSFileChangeTypeFontSize)
    {
        if(overrideContent->type == CFASSFileDialogueTextContentOverrideContentTypeFontSize && change->fontSize.affectOverride)
        {
            if(change->fontSize.byPercentage)
                overrideContent->data.fontSize.resolutionPixels *= change->fontSize.percentage;
            else
                overrideContent->data.fontSize.resolutionPixels = change->fontSize.fontSize;
        }
    }
    if(change->type & CFASSFileChangeTypeFontName)
    {
        if(overrideContent->type == CFASSFileDialogueTextContentOverrideContentTypeFontName && change->fontSize.affectOverride)
        {
            wchar_t *dumped = CF_Dump_wchar_string(change->fontName.fontName);
            if(dumped != NULL)
            {
                free(overrideContent->data.fontName.name);
                overrideContent->data.fontName.name = dumped;
            }
        }
    }
}

static bool CFASSFileDialogueTextContentOverrideContentCheckAnimationModifiers(wchar_t *modifiers)
{
    DEBUG_ASSERT(modifiers != NULL); if(modifiers == NULL) return false;
    
    // include modifier[0] == L'\0'
    if(modifiers[0] != L'\\') return false;
    
    // create fake parsingResult
    CFASSFileParsingResultRef parsingResult = CFASSFileParsingResultCreate();
    if(parsingResult != NULL) { DEBUG_POINT return false; }
    
    bool formatCheck = true;
    wchar_t *nextBeginPoint;
    CFASSFileDialogueTextContentOverrideContentRef modifierContent;
    while ((nextBeginPoint = wcschr(modifiers + 1, L'\\')) != NULL)
    {
        modifierContent = CFASSFileDialogueTextContentOverrideContentCreateWithString(modifiers, nextBeginPoint - 1, parsingResult);
        if(modifierContent == NULL) {
            formatCheck = false; break;
        }
        if(!CFASSFileDialogueTextContentOverrideContentCheckAnimationModifierSupport(modifierContent))
        {
            CFASSFileDialogueTextContentOverrideContentDestory(modifierContent);
            formatCheck = false; break;
        }
        CFASSFileDialogueTextContentOverrideContentDestory(modifierContent);
        modifiers = nextBeginPoint;
    }
    if(formatCheck) {
        wchar_t *endPoint = modifiers + wcslen(modifiers)-1;
        modifierContent = CFASSFileDialogueTextContentOverrideContentCreateWithString(modifiers, endPoint, parsingResult);
        
        if(modifierContent == NULL) formatCheck = false;
        else if(!CFASSFileDialogueTextContentOverrideContentCheckAnimationModifierSupport(modifierContent))
            formatCheck = false;
        CFASSFileDialogueTextContentOverrideContentDestory(modifierContent);
    }
    
    CFASSFileParsingResultDestory(parsingResult);
    
    if(formatCheck) return true;
    return false;
}

static bool CFASSFileDialogueTextContentOverrideContentCheckAnimationModifierSupport(CFASSFileDialogueTextContentOverrideContentRef content)
{
    CFASSFileDialogueTextContentOverrideContentType type = content->type;
    if(type == CFASSFileDialogueTextContentOverrideContentTypeFontSize ||
       type == CFASSFileDialogueTextContentOverrideContentTypeFontSpacing ||
       type == CFASSFileDialogueTextContentOverrideContentTypeColor ||
       type == CFASSFileDialogueTextContentOverrideContentTypeAlpha ||
       type == CFASSFileDialogueTextContentOverrideContentTypeFontScale ||
       type == CFASSFileDialogueTextContentOverrideContentTypeTextRotation ||
       type == CFASSFileDialogueTextContentOverrideContentTypeTextShearing ||
       type == CFASSFileDialogueTextContentOverrideContentTypeBorder ||
       type == CFASSFileDialogueTextContentOverrideContentTypeShadow ||
       type == CFASSFileDialogueTextContentOverrideContentTypeBlurEdge)
        return true;
    else if(type == CFASSFileDialogueTextContentOverrideContentTypeClip &&
            !content->data.clip.usingDrawingCommand)
        return true;
    return false;
}

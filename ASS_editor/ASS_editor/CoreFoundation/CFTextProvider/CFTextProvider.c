//
//  CFTextProvider.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "CFTextProvider.h"
#include "CFTextProvider_Private.h"
#include "CFUseTool.h"
#include "CFException.h"

struct CFTextProvider
{
    wchar_t *allocatedString;
};

CFTextProviderRef CFTextProviderCreateWithFileUrl(const char *url)
{
    FILE *fp;
    if((fp = fopen(url, "r")) != NULL)
    {
        fpos_t beginPosition;
        if(fgetpos(fp, &beginPosition) == 0)
        {
            size_t fileLength = 0;
            while(getwc(fp)!=WEOF) fileLength++;
            if(fsetpos(fp, &beginPosition) == 0)
            {
                wchar_t *allocatedFileString;
                if((allocatedFileString = malloc(sizeof(wchar_t)*(fileLength+1))) != NULL)
                {
                    wchar_t *currentPoint = allocatedFileString;
                    while((*currentPoint++ = getwc(fp)) != WEOF) continue;
                    currentPoint[-1] = L'\0';
                    CFTextProviderRef result;
                    if((result = malloc(sizeof(struct CFTextProvider))) != NULL)
                    {
                        result->allocatedString = allocatedFileString;
                        fclose(fp);
                        return result;
                    }
                    free(allocatedFileString);
                }
            }
        }
        fclose(fp);
    }
    return NULL;
}

CFTextProviderRef CFTextProviderCreateWithString(const char *string)
{
    if(string == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFTextProviderCreateWithString NULL");
        return NULL;
    }
    CFTextProviderRef result;
    if((result = malloc(sizeof(struct CFTextProvider))) != NULL)
    {
        if((result->allocatedString = CF_Dump_mbs_to_wcs(string)) != NULL)
            return result;
        free(result);
    }
    return NULL;
}

CFTextProviderRef CFTextProviderCreateWithWcharString(const wchar_t *string)
{
    if(string == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFTextProviderCreateWithString NULL");
        return NULL;
    }
    CFTextProviderRef result;
    if((result = malloc(sizeof(struct CFTextProvider))) != NULL)
    {
        if((result->allocatedString = CF_Dump_wchar_string(string)) != NULL)
            return result;
        free(result);
    }
    return NULL;
}

wchar_t *CFTextProviderAllocateTextContentwithUnicodeEncoding(CFTextProviderRef provider, size_t *length)
{
    if(provider == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFTextProvider NULL AllocateTextContentwithUnicodeEncoding");
        return NULL;
    }
    wchar_t *result = CF_Dump_wchar_string(provider->allocatedString);
    if(result!=NULL)
    {
        if(length!=NULL) *length = wcslen(result);
        return result;
    }
    return NULL;
}




















































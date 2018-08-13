//
//  main.m
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "CFASSFileKit.h"
#include "CFCrossPlatform.h"

int main(int argc, char *argv[])
{
    CFCrossPlatformUnicodeSupport();
    CFASSFileControlSetErrorHandling(CFASSFileControlErrorHandlingIgnore);
    
    const unsigned int amount = 12;
    const char *prefix = "Imouto sae Ireba Ii. ";
    const char *dictionary = "/Users/captainallred/Desktop/ASS/";
    const char *convertedSubfix = " - converted";
    const char *originalSubfix = " - original";
    char eachPath[FILENAME_MAX];
    
    for(unsigned int count = 1; count<=amount; count++)
    {
        snprintf(eachPath, FILENAME_MAX, "%s%s%02u.ass", dictionary, prefix, count);
        
        CFTextProviderRef textProvider = CFTextProviderCreateWithFileUrl(eachPath);
        CFASSFileRef eachFile = CFASSFileCreateWithTextProvider(textProvider);
        
        /* change begin */
        
        CFASSFileChangeRef changeFontName = CFASSFileChangeFontName(L"微软雅黑", true);
        CFASSFileChangeRef changeBlod =  CFASSFileChangeBlod(true);
        CFASSFileChangeRef combinedChange = CFASSFileMultiChangeCombineTermiateWithNULL(true, changeFontName, changeBlod, NULL);
        
        CFASSFileMakeChange(eachFile, combinedChange);
        
        CFASSFileChangeDestory(combinedChange);
        
        /* change end */
        
        wchar_t *allocatedString = CFASSFileAllocateFileContent(eachFile);
        snprintf(eachPath, FILENAME_MAX, "%s%s%02u%s.ass", dictionary, prefix, count, convertedSubfix);
        
        FILE *ouputFile;
        if((ouputFile = fopen(eachPath, "w")) != NULL)
            fputws(allocatedString, ouputFile);
        fclose(ouputFile);
        
        CFTextProviderDestory(textProvider);
        CFASSFileDestory(eachFile);
        free(allocatedString);
    }
    return EXIT_SUCCESS;
}

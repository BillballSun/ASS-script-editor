//
//  CFASSFile.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdlib.h>
#include <wchar.h>

#include "CFASSFile.h"
#include "CFASSFileScriptInfo.h"
#include "CFASSFileScriptInfo_Private.h"
#include "CFASSFileStyleCollection.h"
#include "CFASSFileStyleCollection_Private.h"
#include "CFASSFileDialogueCollection.h"
#include "CFASSFileDialogueCollection_Private.h"
#include "CFTextProvider.h"
#include "CFTextProvider_Private.h"
#include "CFException.h"
#include "CFASSFileChange.h"
#include "CFASSFileChange_Private.h"
#include "CFMacro.h"

CLANG_DIAGNOSTIC_PUSH
CLANG_DIAGNOSTIC_IGNORE_NONNULL

struct CFASSFile
{
    CFASSFileScriptInfoRef scriptInfo;
    CFASSFileStyleCollectionRef styleCollection;
    CFASSFileDialogueCollectionRef dialogueCollection;
};

void CFASSFileMakeChange(CFASSFileRef file, CFASSFileChangeRef change)
{
    if(file == NULL || change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFile %p MakeChange %p", file, change);
    if(CFASSFileChangeShouldDispatchToScriptInfo(change))
        CFASSFileScriptInfoMakeChange(file->scriptInfo, change);
    if(CFASSFileChangeShouldDispatchToStyleCollection(change))
        CFASSFileStyleCollectionMakeChange(file->styleCollection, change);
    if(CFASSFileChangeShouldDispatchToDialogueCollection(change))
        CFASSFileDialogueCollectionMakeChange(file->dialogueCollection, change);
}

void CFASSFileReceiveResolutionXYChangeFromScriptInfo(CFASSFileRef file, CFUSize oldSize, CFUSize newSize)
{
    if(file == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFile NULL ReceiveResolutionXYChangeFromScriptInfo");
    double percentage = ((double)newSize.x/oldSize.x + (double)newSize.y/oldSize.y)/2;
    CFASSFileChangeRef change = CFASSFileChangeFontSize(true, percentage, 0);
    if(CFASSFileChangeShouldDispatchToStyleCollection(change))
        CFASSFileStyleCollectionMakeChange(file->styleCollection, change);
    if(CFASSFileChangeShouldDispatchToDialogueCollection(change))
        CFASSFileDialogueCollectionMakeChange(file->dialogueCollection, change);
}

CFASSFileScriptInfoRef CFASSFileGetScriptInfo(CFASSFileRef file)
{
    if(file == NULL) return NULL;
    return file->scriptInfo;
}

CFASSFileStyleCollectionRef CFASSFileGetStyleCollection(CFASSFileRef file)
{
    if(file == NULL) return NULL;
    return file->styleCollection;
}

CFASSFileDialogueCollectionRef CFASSFileGetDialogueCollection(CFASSFileRef file)
{
    if(file == NULL) return NULL;
    return file->dialogueCollection;
}

CFASSFileRef CFASSFileCopy(CFASSFileRef file)
{
    if(file == NULL) return NULL;
    CFASSFileRef result;
    if((result = malloc(sizeof(struct CFASSFile))) != NULL)
    {
        if((result->scriptInfo = CFASSFileScriptInfoCopy(file->scriptInfo)) != NULL)
        {
            if((result->styleCollection = CFASSFileStyleCollectionCopy(file->styleCollection)) != NULL)
            {
                if((result->dialogueCollection = CFASSFileDialogueCollectionCopy(file->dialogueCollection)) != NULL)
                {
                    CFASSFileScriptInfoRegisterAssociationwithFile(result->scriptInfo, result);
                    CFASSFileStyleCollectionRegisterAssociationwithFile(result->styleCollection, result);
                    CFASSFileDialogueCollectionRegisterAssociationwithFile(result->dialogueCollection , result);
                    return result;
                }
                CFASSFileStyleCollectionDestory(result->styleCollection);
            }
            CFASSFileScriptInfoDestory(result->scriptInfo);
        }
        free(result);
    }
    return NULL;
}

CFASSFileRef CFASSFileCreate(CFASSFileScriptInfoRef scriptInfo,
                             CFASSFileStyleCollectionRef styleCollection,
                             CFASSFileDialogueCollectionRef dialogueCollection,
                             bool transferOwnerShip)
{
    if(scriptInfo == NULL || styleCollection == NULL || dialogueCollection == NULL)
        return NULL;
    CFASSFileRef result;
    if((result = malloc(sizeof(struct CFASSFile))) != NULL)
    {
        if(transferOwnerShip)
        {
            result->scriptInfo = scriptInfo;
            result->styleCollection = styleCollection;
            result->dialogueCollection = dialogueCollection;
        }
        else
        {
            result->scriptInfo = CFASSFileScriptInfoCopy(scriptInfo);
            result->styleCollection = CFASSFileStyleCollectionCopy(styleCollection);
            result->dialogueCollection = CFASSFileDialogueCollectionCopy(dialogueCollection);
            if(result->scriptInfo == NULL || result->styleCollection == NULL || result->dialogueCollection == NULL)
            {
                CFExceptionRaise(CFExceptionNameProcessFailed, NULL, "CFASSFile copy user-provided component failed");
                if(result->scriptInfo != NULL) CFASSFileScriptInfoDestory(result->scriptInfo);
                if(result->styleCollection != NULL) CFASSFileStyleCollectionDestory(result->styleCollection);
                if(result->dialogueCollection != NULL) CFASSFileDialogueCollectionDestory(result->dialogueCollection);
                return NULL;
            }
        }
        CFASSFileScriptInfoRegisterAssociationwithFile(result->scriptInfo, result);
        CFASSFileStyleCollectionRegisterAssociationwithFile(result->styleCollection, result);
        CFASSFileDialogueCollectionRegisterAssociationwithFile(result->dialogueCollection , result);
        return result;
    }
    return NULL;
}

CFASSFileRef CFASSFileCreateWithTextProvider(CFTextProviderRef provider, CFASSFileParsingResultRef *parsingResult_ownershipTransferred) {
    if(parsingResult_ownershipTransferred != NULL) *parsingResult_ownershipTransferred = NULL;
    if(provider == NULL) {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileCreateWithTextProvider provide is NULL");
        return NULL;
    }
    wchar_t *fileContent;
    if((fileContent = CFTextProviderAllocateTextContentwithUnicodeEncoding(provider, NULL)) != NULL) {
        CFASSFileRef result;
        if((result = malloc(sizeof(struct CFASSFile))) != NULL) {
            CFASSFileParsingResultRef parsingResult;
            if((parsingResult = CFASSFileParsingResultCreate()) != NULL) {
                if((result->scriptInfo = CFASSFileScriptInfoCreateWithUnicodeFileContent(fileContent, parsingResult)) != NULL) {
                    if((result->styleCollection = CFASSFileStyleCollectionCreateWithUnicodeFileContent(fileContent, parsingResult)) != NULL) {
                        if((result->dialogueCollection = CFASSFileDialogueCollectionCreateWithUnicodeFileContent(fileContent, parsingResult)) != NULL) {
                            if(parsingResult_ownershipTransferred != NULL) {
                                CFASSFileParsingResultSetParsingString(parsingResult, fileContent, true);
                                *parsingResult_ownershipTransferred = parsingResult;
                            }
                            else {  // DEBUG Parsing result here
                                free(fileContent);
                                CFASSFileParsingResultDestory(parsingResult);
                            }
                            CFASSFileScriptInfoRegisterAssociationwithFile(result->scriptInfo, result);
                            CFASSFileStyleCollectionRegisterAssociationwithFile(result->styleCollection, result);
                            CFASSFileDialogueCollectionRegisterAssociationwithFile(result->dialogueCollection , result);
                            return result;
                        } DEBUG_ELSE
                        CFASSFileStyleCollectionDestory(result->styleCollection);
                    } DEBUG_ELSE
                    CFASSFileScriptInfoDestory(result->scriptInfo);
                } DEBUG_ELSE
                if(parsingResult_ownershipTransferred != NULL) *parsingResult_ownershipTransferred = parsingResult;
                else CFASSFileParsingResultDestory(parsingResult);
            } DEBUG_ELSE
            free(result);
        } DEBUG_ELSE
        free(fileContent);
    } DEBUG_ELSE
    return NULL;
}

void CFASSFileDestory(CFASSFileRef file)
{
    if(file == NULL) return;
    CFASSFileScriptInfoDestory(file->scriptInfo);
    CFASSFileStyleCollectionDestory(file->styleCollection);
    CFASSFileDialogueCollectionDestory(file->dialogueCollection);
    free(file);
}

wchar_t *CFASSFileAllocateFileContent(CFASSFileRef file)
{
    if(file == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileAllocate File NULL Content");
        return NULL;
    }
    wchar_t *scriptInfo = CFASSFileScriptInfoAllocateFileContent(file->scriptInfo),
    *styleCollection = CFASSFileStyleCollectionAllocateFileContent(file->styleCollection),
    *dialogueCollection = CFASSFileDialogueCollectionAllocateFileContent(file->dialogueCollection);
    
    if(scriptInfo == NULL || styleCollection == NULL || dialogueCollection == NULL)
    {
        DEBUG_POINT;
        if(scriptInfo != NULL) free(scriptInfo);
        if(styleCollection != NULL) free(styleCollection);
        if(dialogueCollection != NULL) free(dialogueCollection);
        return NULL;
    }
    size_t stringLength = wcslen(scriptInfo) + wcslen(styleCollection) + wcslen(dialogueCollection);
    wchar_t *result;
    if((result = malloc(sizeof(wchar_t) * (stringLength + 1))) != NULL)
    {
        wcscpy(result, scriptInfo);
        wcscat(result, styleCollection);
        wcscat(result, dialogueCollection);
    }
    free(scriptInfo);
    free(styleCollection);
    free(dialogueCollection);
    DEBUG_ASSERT(wcslen(result) == stringLength);
    return result;
}

CLANG_DIAGNOSTIC_POP

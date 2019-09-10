//
//  CFASSFile.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFile_h
#define CFASSFile_h

#include <stdio.h>
#include <stdbool.h>

#include "CFASSFileScriptInfo.h"
#include "CFASSFileStyleCollection.h"
#include "CFASSFileDialogueCollection.h"
#include "CFTextProvider.h"
#include "CFGeometry.h"
#include "CFASSFileChange.h"

typedef struct CFASSFile *CFASSFileRef;

#pragma mark - Create/Copy/Destory

/**
 CFASSFileCreate is the designated intializer for compose the ASS file together

 @param transferOwnerShip this free you from the management of each conpoment
 */
CFASSFileRef CFASSFileCreate(CFASSFileScriptInfoRef scriptInfo,
                             CFASSFileStyleCollectionRef styleCollection,
                             CFASSFileDialogueCollectionRef dialogueCollection,
                             bool transferOwnerShip);

/**
 CFASSFileCreateWithTextProvider tranlate the text management to the CFTextProvider, this as because the CFTextProvider handles the cross platform text and multi-style text encoding well

 @return if not critical Error inside the CFASSFile original text, this will create a CFASSFile
 */
CFASSFileRef CFASSFileCreateWithTextProvider(CFTextProviderRef provider);

CFASSFileRef CFASSFileCopy(CFASSFileRef file);

void CFASSFileDestory(CFASSFileRef file);

#pragma mark - Allocate Result

wchar_t *CFASSFileAllocateFileContent(CFASSFileRef file);

#pragma mark - Receive Change

void CFASSFileMakeChange(CFASSFileRef file, CFASSFileChangeRef change);

#endif /* CFASSFile_h */

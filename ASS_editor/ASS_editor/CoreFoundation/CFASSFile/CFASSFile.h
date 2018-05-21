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

typedef struct CFASSFile *CFASSFileRef;

CFASSFileRef CFASSFileCreate(CFASSFileScriptInfoRef scriptInfo,
                             CFASSFileStyleCollectionRef styleCollection,
                             CFASSFileDialogueCollectionRef dialogueCollection,
                             bool transferOwnerShip);

CFASSFileRef CFASSFileCreateWithTextProvider(CFTextProviderRef provider);

void CFASSFileDestory(CFASSFileRef file);

CFASSFileRef CFASSFileCopy(CFASSFileRef file);

wchar_t *CFASSFileAllocateFileContent(CFASSFileRef file);

#endif /* CFASSFile_h */

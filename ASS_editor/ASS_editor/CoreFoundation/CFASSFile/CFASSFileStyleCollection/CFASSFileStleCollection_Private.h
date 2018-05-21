//
//  CFASSFileStleCollection_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileStleCollection_Private_h
#define CFASSFileStleCollection_Private_h

#include <wchar.h>

#include "CFASSFileStyleCollection.h"
#include "CFASSFile.h"

CFASSFileStyleCollectionRef CFASSFileStyleCollectionCreateWithUnicodeFileContent(const wchar_t *content);

wchar_t *CFASSFileStyleCollectionAllocateFileContent(CFASSFileStyleCollectionRef styleCollection);

int CFASSFileStyleCollectionRegisterAssociationwithFile(CFASSFileStyleCollectionRef scriptInfo, CFASSFileRef assFile);
/* Return */
// return zero means success, -1 means failed

#endif /* CFASSFileStleCollection_Private_h */

//
//  CFASSFileScriptInfo_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileScriptInfo_Private_h
#define CFASSFileScriptInfo_Private_h

#include <wchar.h>

#include "CFASSFileScriptInfo.h"
#include "CFASSFile.h"

CFASSFileScriptInfoRef CFASSFileScriptInfoCreateWithUnicodeFileContent(const wchar_t *content);

wchar_t *CFASSFileScriptInfoAllocateFileContent(CFASSFileScriptInfoRef scriptInfo);

int CFASSFileScriptInfoRegisterAssociationwithFile(CFASSFileScriptInfoRef scriptInfo, CFASSFileRef assFile);
/* Return */
// return zero means success, -1 means failed

#endif /* CFASSFileScriptInfo_Private_h */

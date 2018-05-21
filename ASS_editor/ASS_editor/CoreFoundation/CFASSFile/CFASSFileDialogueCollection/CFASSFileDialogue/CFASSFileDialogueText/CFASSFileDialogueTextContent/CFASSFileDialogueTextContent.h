//
//  CFASSFileDialogueTextContent.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/11.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueTextContent_h
#define CFASSFileDialogueTextContent_h

#include "CFASSFileDialogueTextContentOverride.h"

typedef enum CFASSFileDialogueTextContentType {
    CFASSFileDialogueTextContentTypeText,
    CFASSFileDialogueTextContentTypeOverride
} CFASSFileDialogueTextContentType;

typedef struct CFASSFileDialogueTextContent *CFASSFileDialogueTextContentRef;

void CFASSFileDialogueTextContentDestory(CFASSFileDialogueTextContentRef content);

CFASSFileDialogueTextContentRef CFASSFileDialogueTextContentCopy(CFASSFileDialogueTextContentRef content);

CFASSFileDialogueTextContentRef CFASSFileDialogueTextContentCreateWithText(wchar_t *text, bool transferAllocationOwnership);

CFASSFileDialogueTextContentRef CFASSFileDialogueTextContentCreateWithOverride(CFASSFileDialogueTextContentOverrideRef override, bool transferOwnership);

#endif /* CFASSFileDialogueTextContent_h */

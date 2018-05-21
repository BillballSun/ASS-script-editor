//
//  CFASSFileDialogueTextContentOverride.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/10.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueTextContentOverride_h
#define CFASSFileDialogueTextContentOverride_h

#include "CFASSFileDialogueTextContentOverrideContent.h"

typedef struct CFASSFileDialogueTextContentOverride *CFASSFileDialogueTextContentOverrideRef;

void CFASSFileDialogueTextContentOverrideDestory(CFASSFileDialogueTextContentOverrideRef override);

CFASSFileDialogueTextContentOverrideRef CFASSFileDialogueTextContentOverrideCopy(CFASSFileDialogueTextContentOverrideRef override);

CFASSFileDialogueTextContentOverrideRef CFASSFileDialogueTextContentOverrideCreateWithContent(CFASSFileDialogueTextContentOverrideContentRef content);

void CFASSFileDialogueTextContentOverrideAddContent(CFASSFileDialogueTextContentOverrideRef override, CFASSFileDialogueTextContentOverrideContentRef content);

#endif /* CFASSFileDialogueTextContentOverride_h */

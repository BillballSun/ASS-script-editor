//
//  CFASSFileDialogueText.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/10.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueText_h
#define CFASSFileDialogueText_h

#include "CFASSFileDialogueTextContent.h"

typedef struct CFASSFileDialogueText *CFASSFileDialogueTextRef;

void CFASSFileDialogueTextDestory(CFASSFileDialogueTextRef dialogueText);

CFASSFileDialogueTextRef CFASSFileDialogueTextCopy(CFASSFileDialogueTextRef dialogueText);

void CFASSFileDialogueTextAddContent(CFASSFileDialogueTextRef dialogueText, CFASSFileDialogueTextContentRef content);

#endif /* CFASSFileDialogueText_h */

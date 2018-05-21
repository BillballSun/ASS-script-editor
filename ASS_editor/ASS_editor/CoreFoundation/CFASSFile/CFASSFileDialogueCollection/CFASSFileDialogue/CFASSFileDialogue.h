//
//  CFASSFileDialogue.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/10.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogue_h
#define CFASSFileDialogue_h

typedef struct CFASSFileDialogue *CFASSFileDialogueRef;

void CFASSFileDialogueDestory(CFASSFileDialogueRef dialogue);

CFASSFileDialogueRef CFASSFileDialogueCopy(CFASSFileDialogueRef dialogue);

#endif /* CFASSFileDialogue_h */

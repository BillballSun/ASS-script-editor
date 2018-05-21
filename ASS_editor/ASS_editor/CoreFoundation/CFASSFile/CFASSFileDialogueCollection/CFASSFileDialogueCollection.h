//
//  CFASSFileDialogueCollection.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueCollection_h
#define CFASSFileDialogueCollection_h

typedef struct CFASSFileDialogueCollection *CFASSFileDialogueCollectionRef;

CFASSFileDialogueCollectionRef CFASSFileDialogueCollectionCopy(CFASSFileDialogueCollectionRef dialogueCollection);

void CFASSFileDialogueCollectionDestory(CFASSFileDialogueCollectionRef dialogueCollection);

#endif /* CFASSFileDialogueCollection_h */

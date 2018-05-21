//
//  CFASSFileDialogueEffect.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/10.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueEffect_h
#define CFASSFileDialogueEffect_h

#include <stdbool.h>

typedef struct CFASSFileDialogueEffect *CFASSFileDialogueEffectRef;

typedef enum CFASSFileDialogueEffectScrollDirection {
    CFASSFileDialogueEffectScrollDirectionUp,
    CFASSFileDialogueEffectScrollDirectionDown
} CFASSFileDialogueEffectScrollDirection;

CFASSFileDialogueEffectRef CFASSFileDialogueEffectCreateKaraoke(void);

CFASSFileDialogueEffectRef CFASSFileDialogueEffectCreateScroll(CFASSFileDialogueEffectScrollDirection type,
                                                               int positionY1,
                                                               int positionY2,
                                                               unsigned int delay,
                                                               bool hasFadeAwayHeight,
                                                               int fadeAwayHeight);

CFASSFileDialogueEffectRef CFASSFileDialogueEffectCreateBanner(unsigned int delay,
                                                               bool usingDefaultDirection,
                                                               bool leftToRight,
                                                               bool hasFadeAwayWidth,
                                                               int fadeAwayWidth);

void CFASSFileDialogueEffectDestory(CFASSFileDialogueEffectRef effect);

CFASSFileDialogueEffectRef CFASSFileDialogueEffectCopy(CFASSFileDialogueEffectRef effect);

#endif /* CFASSFileDialogueEffect_h */

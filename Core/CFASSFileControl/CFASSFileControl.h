//
//  CFASSFileControl.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/6/7.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileControl_h
#define CFASSFileControl_h

#include <stdbool.h>
#include <stdio.h>

typedef enum {
    CFASSFileControlLevelIgnore = 1 << 0,
    // if this bit set, CFASSFile will try its best to ignore some errors
    // with auto fixed
    CFASSFileControlLevelDetail = 1 << 1,
    // by default, only when encounter error (unrecoverable) will be listed
    // in parsing result, this shows all kind of warning or auto-correct info
}   CFASSFileControlLevel;

#pragma mark - Control Level

CFASSFileControlLevel CFASSFileControlGetLevel(void);

void CFASSFileControlSetLevel(CFASSFileControlLevel level);

#endif /* CFASSFileControl_h */

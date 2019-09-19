//
//  CFASSFileControl.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/6/7.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <stddef.h>
#include <wchar.h>

#include "CFASSFileControl.h"
#include "CFStringTool.h"
#include "CFException.h"
#include "CFMacro.h"

#define StringLengthMAX 256

static CFASSFileControlLevel level = CFASSFileControlLevelIgnore | CFASSFileControlLevelDetail;

CFASSFileControlLevel CFASSFileControlGetLevel(void) {
    return level;
}

void CFASSFileControlSetLevel(CFASSFileControlLevel requestLevel) {
    level = requestLevel;
}

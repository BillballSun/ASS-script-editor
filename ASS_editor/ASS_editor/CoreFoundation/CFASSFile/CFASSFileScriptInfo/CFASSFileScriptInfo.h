//
//  CFASSFileScriptInfo.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileScriptInfo_h
#define CFASSFileScriptInfo_h

#include "CFUseTool.h"

typedef struct CFASSFileScriptInfo *CFASSFileScriptInfoRef;

CFASSFileScriptInfoRef CFASSFileScriptInfoCopy(CFASSFileScriptInfoRef scriptInfo);

void CFASSFileScriptInfoDestory(CFASSFileScriptInfoRef scriptInfo);

CFASSFileScriptInfoRef CFASSFileScriptInfoCreateEssential(wchar_t *title,
                                                          wchar_t *original_script,
                                                          bool is_collisions_normal,
                                                          unsigned int play_res_x,
                                                          unsigned int play_res_y,
                                                          double timePercentage);

void CFASSFileScriptInfoChangeResolutionXY(CFASSFileScriptInfoRef scriptInfo,
                                           CFUSize newSize,
                                           bool dispatchChange);

CFUSize CFASSFileScriptInfoGetResolutionXY(CFASSFileScriptInfoRef scriptInfo);

#endif /* CFASSFileScriptInfo_h */

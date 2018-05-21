//
//  CFUseTool.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFUseTool_h
#define CFUseTool_h

#include <wchar.h>
#include <stdbool.h>

#include "CFDef.h"

CFUSize CFUSizeMake(unsigned int x, unsigned int y);

CFSize CFSizeMake(int x, int y);

const wchar_t *CF_wcsstr_with_end_point(const wchar_t *sourceString, const wchar_t *findString, const wchar_t *endPoint);
/* endPoint */
// if endPoint is NULL, it is not used

wchar_t *CF_allocate_wchar_string_with_endPoint_skip_terminate(const wchar_t *sourceString,
                                                               const wchar_t *endPoint,
                                                               bool (*isSkip)(wchar_t),
                                                               bool *terminateBecauseOfSkip);
/* endPoint */
// if endPoint is NULL, it is not used, endPoint is the last character it will possibly alloc points, and must be no more than sourceString
/* isSkip */
// this kind of character will not be treated as string, this could be NULL
/* terminateBecauseOfSkip */
// this could be NULL
/* return value */
// the NULL means can't find such string or string is length zero

wchar_t *CF_Dump_wchar_string(const wchar_t *source);
/* Return */
// return NULL if source is NULL

bool CF_wchar_string_match_beginning(const wchar_t *testString,
                                     const wchar_t *match);

const wchar_t *CF_match_next_braces(const wchar_t *beginBrace, const wchar_t *endPoint, bool loopInto);
// support braces () [] {}

long CF_mbslen(const char *mbs);
// if error, return -1

long CF_multibyte_character_amount(const char *mbs);
// if error, return -1

wchar_t *CF_Dump_mbs_to_wcs(const char *mbs);

#endif /* CFUseTool_h */




















































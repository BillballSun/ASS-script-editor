//
//  CFPointerArray.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/10.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFPointerArray_h
#define CFPointerArray_h

#include <stdbool.h>
#include <stddef.h>

typedef struct CFPointerArray *CFPointerArrayRef;

#pragma mark - Create / Destory

CFPointerArrayRef CFPointerArrayCreateEmpty(void);

CFPointerArrayRef CFPointerArrayCopy(CFPointerArrayRef array, bool transferOwnership);
/* transferOwnership */
// the previous array's ownership will be discarded and transfer to the new on

void CFPointerArrayDestory(CFPointerArrayRef array);

#pragma mark - Query For Element

size_t CFPointerArrayGetLength(CFPointerArrayRef array);

void CFPointerArrayAddPointer(CFPointerArrayRef array, void *pointer, bool transferOwnership);
/* transferOwnership */
// if the array got the ownership, it will free the pointer if it receives destory command

size_t CFPointerArrayInsertPointerAtIndex(CFPointerArrayRef array, void *pointer, size_t index, bool transferOwnership);
/* return */
// the real index of the pointer
// if anything wrong, return (size_t)-1

void *CFPointerArrayRemovePointerAtIndex(CFPointerArrayRef array, size_t index, bool transferOwnership);
/* transferOwnership */
// trying to transferOwnership from the pointer the array didn't get will be ignored
/* return */
// the pointer, if array got the ownership and transferOwnership is false, then array will free the pointer, so the return is NULL

void *CFPointerArrayGetPointerAtIndex(CFPointerArrayRef array, size_t index);

void *CFPointerArrayGetLastPointer(CFPointerArrayRef array);

void CFPointerArraySwapPointerPosition(CFPointerArrayRef array, size_t index1, size_t index2);

#pragma mark - Ownership

bool CFPointerArrayHasOwnershipAtIndex(CFPointerArrayRef array, size_t index);

#endif /* CFPointerArray_h */

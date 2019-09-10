//
//  CFException.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/18.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFException_h
#define CFException_h

#include <stdbool.h>

#pragma mark - Type defintion

typedef const char * CFExceptionNameType;

typedef bool (*CFExceptionCatchFunction)(CFExceptionNameType exceptionName, void * data);

#pragma mark - Contant excpetion name

extern CFExceptionNameType const CFExceptionNameArrayOutOfBounds;
extern CFExceptionNameType const CFExceptionNameDivideZero;
extern CFExceptionNameType const CFExceptionNameOverflow;
extern CFExceptionNameType const CFExceptionNameInvalidArgument;
extern CFExceptionNameType const CFExceptionNameProcessFailed;

#pragma mark - Raise Exception

void CFExceptionRaise(CFExceptionNameType exceptionName, void * data, const char * format, ...) __attribute__((format(printf, 3, 4)));

#pragma mark - Global Exception Handling Stack

void CFExceptionCatchStackPush(CFExceptionCatchFunction function);

CFExceptionCatchFunction CFExceptionCatchStackPop(void);

CFExceptionCatchFunction CFExceptionCatchStackCurrentFunction(void);

#endif /* CFException_h */

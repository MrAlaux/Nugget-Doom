//
// Copyright(C) 2025-2026 Alaux
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#ifndef __I_THREAD__
#define __I_THREAD__

#include "doomtype.h"

extern int cvar_worker_threads; // CVAR

void  I_InitThreading(void);

int I_MainSemaphoreIndex(void);
int I_WorkerSemaphoreIndex(void);

void  I_SemaphorePost(int index);
void  I_SemaphoreWait(int index);

size_t  I_ThreadsNum(void);

void  I_ThreadSetWorkerFunction(void (*function)(void));
void  I_WorkerMutexLock(void);
void  I_WorkerMutexUnlock(void);

#endif

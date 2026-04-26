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

#include <SDL3/SDL.h>

#include "doomtype.h"
#include "i_exit.h"
#include "i_printf.h"
#include "i_system.h"
#include "i_thread.h"
#include "m_misc.h"

int cvar_worker_threads;
static int worker_threads = 0;

typedef struct thread_data_s
{
  void (*function)(void *);
  unsigned id;
} thread_data_t;

// threads[0] is a placeholder for the main thread, contains garbage
static SDL_Thread **threads = NULL;
static size_t num_threads = 1, num_threads_alloc = 0;
static boolean destroy_threads = false;

static SDL_Semaphore **semaphores = NULL;
static size_t num_semaphores = 0, num_semaphores_alloc = 0;

static int main_semaphore_index = -1, worker_semaphore_index = -1;

static SDL_Mutex *worker_mutex = NULL;

static void I_ShutdownThreading(void);
static int  I_SemaphoreCreate(void);
static int  I_ThreadCreate(void (*function)(void *), unsigned id);
static void WorkerFunctionWrapper(void *data);

void I_InitThreading(void)
{
  worker_threads = (cvar_worker_threads == -1)
                 ? SDL_GetNumLogicalCPUCores() - 1
                 : cvar_worker_threads;

  if (worker_threads < 1) { return; }

  I_AtExit(I_ShutdownThreading, true);

  main_semaphore_index = I_SemaphoreCreate();

  if (main_semaphore_index == -1)
  { I_Error("Failed to create main semaphore."); }

  worker_semaphore_index = I_SemaphoreCreate();

  if (worker_semaphore_index == -1)
  { I_Error("Failed to create worker semaphore."); }

  worker_mutex = SDL_CreateMutex();

  if (!worker_mutex)
  { I_Error("Failed to create worker mutex."); }

  for (int i = 0;  i < worker_threads;  i++)
  {
    if (I_ThreadCreate(WorkerFunctionWrapper, i + 1) < 0)
    { I_Error("Failed to create worker threads."); }
  }

  const int total_threads = 1 + worker_threads;

  I_Printf(
    VB_INFO, "%s: Using %i thread%s.",
    __func__, total_threads, (total_threads != 1) ? "s" : ""
  );
}

static void I_ShutdownThreading(void)
{
  destroy_threads = true;

  for (int i = 1;  i < num_threads;  i++)
  { I_SemaphorePost(I_WorkerSemaphoreIndex()); }

  while (num_threads > 1)
  {
    num_threads--;
    SDL_WaitThread(threads[num_threads], NULL);
  }

  if (worker_mutex) { SDL_DestroyMutex(worker_mutex); }

  while (num_semaphores)
  {
    num_semaphores--;
    SDL_DestroySemaphore(semaphores[num_semaphores]);
  }
}

int I_MainSemaphoreIndex(void)
{
  return main_semaphore_index;
}

int I_WorkerSemaphoreIndex(void)
{
  return worker_semaphore_index;
}

// Semaphores ----------------------------------------------------------------

static void StoreSemaphore(SDL_Semaphore *const semaphore)
{
  if (num_semaphores >= num_semaphores_alloc)
  {
    num_semaphores_alloc = num_semaphores_alloc ? (num_semaphores_alloc * 2) : 2;
    semaphores = I_Realloc(semaphores, sizeof(*semaphores) * num_semaphores_alloc);
  }

  semaphores[num_semaphores] = semaphore;
  num_semaphores++;
}

static int I_SemaphoreCreate(void)
{
  SDL_Semaphore *const semaphore = SDL_CreateSemaphore(0);

  if (!semaphore) { return -1; }

  StoreSemaphore(semaphore);
  return num_semaphores - 1;
}

void I_SemaphorePost(int index)
{
  if (!semaphores) { return; }

  SDL_SignalSemaphore(semaphores[index]);
}

void I_SemaphoreWait(int index)
{
  if (!semaphores) { return; }

  SDL_WaitSemaphore(semaphores[index]);
}

// Threads -------------------------------------------------------------------

static void StoreThread(SDL_Thread *const thread)
{
  if (num_threads >= num_threads_alloc)
  {
    num_threads_alloc = num_threads_alloc ? (num_threads_alloc * 2) : 2;
    threads = I_Realloc(threads, sizeof(*threads) * num_threads_alloc);
  }

  threads[num_threads] = thread;
  num_threads++;
}

static int SDLCALL FunctionWrapper(void *data)
{
  const thread_data_t thread_data = *((thread_data_t *) data);

  // Data copied, let main continue
  I_SemaphorePost(main_semaphore_index);

  unsigned id = thread_data.id;

  thread_data.function(&id);

  return 0;
}

static int I_ThreadCreate(void (*function)(void *), unsigned id)
{
  char thread_name[24];
  M_snprintf(thread_name, sizeof(thread_name), "NugThr%i", id);

  thread_data_t thread_data = { function, id };

  SDL_Thread *const thread = SDL_CreateThread(FunctionWrapper, thread_name, &thread_data);

  if (!thread) { return -1; }

  StoreThread(thread);

  // Wait for data to be copied before continuing
  I_SemaphoreWait(main_semaphore_index);

  return num_threads - 1;
}

size_t I_ThreadsNum(void)
{
  return num_threads;
}

// Worker threads ------------------------------------------------------------

static void (*WorkerFunction)(void) = NULL;

void I_ThreadSetWorkerFunction(void (*function)(void))
{
  WorkerFunction = function;
}

void I_WorkerMutexLock(void)
{
  if (!worker_mutex) { return; }

  SDL_LockMutex(worker_mutex);
}

void I_WorkerMutexUnlock(void)
{
  if (!worker_mutex) { return; }

  SDL_UnlockMutex(worker_mutex);
}

static void WorkerFunctionWrapper(void *data)
{
  while (true)
  {
    I_SemaphoreWait(worker_semaphore_index);

    if (destroy_threads) { break; }

    WorkerFunction();

    I_SemaphorePost(I_MainSemaphoreIndex());
  }
}

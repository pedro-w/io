#pragma once

void Coro_StartWithArg(void*);
void Coro_allocStackIfNeeded(Coro *);
void Coro_freeStack(Coro *);
Coro *Coro_initBase(Coro *);

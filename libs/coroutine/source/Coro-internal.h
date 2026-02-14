#pragma once

void Coro_StartWithArg(void*);
void Coro_allocStackIfNeeded(Coro *);
Coro *Coro_initBase(Coro*);
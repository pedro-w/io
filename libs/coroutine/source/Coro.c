/*
 Credits

        Originally based on Edgar Toernig's Minimalistic cooperative
 multitasking http://www.goron.de/~froese/ reorg by Steve Dekorte and Chis
 Double Symbian and Cygwin support by Chis Double Linux/PCC, Linux/Opteron, Irix
 and FreeBSD/Alpha, ucontext support by Austin Kurahone FreeBSD/Intel support by
 Faried Nawaz Mingw support by Pit Capitain Visual C support by Daniel Vollmer
        Solaris support by Manpreet Singh
        Fibers support by Jonas Eschenburg
        Ucontext arg support by Olivier Ansaldi
        Ucontext x86-64 support by James Burgess and Jonathan Wright
        Russ Cox for the newer portable ucontext implementions.
        Mac OS X support by Jorge Acereda
        Guessed setjmp support (Android/Mac OS X/others?) by Jorge Acereda

 Notes

        This is the system dependent coro code.
        Setup a jmp_buf so when we longjmp, it will invoke 'func' using 'stack'.
        Important: 'func' must not return!

        Usually done by setting the program counter and stack pointer of a new,
        empty stack. If you're adding a new platform, look in the setjmp.h for PC and
        SP members of the stack structure

        If you don't see those members, Kentaro suggests writting a simple
        test app that calls setjmp and dumps out the contents of the jmp_buf.
        (The PC and SP should be in jmp_buf->__jmpbuf).

        Using something like GDB to be able to peek into register contents right
        before the setjmp occurs would be helpful also.
 */

#include "Base.h"
#include "Coro.h"
#include "Coro-internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "taskimpl.h"

#ifdef USE_VALGRIND
#include <valgrind/valgrind.h>
#define STACK_REGISTER(coro)                                                   \
    {                                                                          \
        Coro *c = (coro);                                                      \
        c->valgrindStackId = VALGRIND_STACK_REGISTER(                          \
            c->stack, (char *)c->stack + c->requestedStackSize);               \
    }

#define STACK_DEREGISTER(coro)                                                 \
    VALGRIND_STACK_DEREGISTER((coro)->valgrindStackId)

#else
#define STACK_REGISTER(coro) // printf("Coro_%p allocating stack size %i\n", self, self->requestedStackSize)
#define STACK_DEREGISTER(coro)
#endif

typedef struct CallbackBlock {
    void *context;
    CoroStartCallback *func;
#ifdef USE_FIBERS
    Coro *associatedCoro;
#endif
} CallbackBlock;

Coro *Coro_initBase(Coro* self) {
    self->requestedStackSize = CORO_DEFAULT_STACK_SIZE;
    self->allocatedStackSize = 0;
    return self;
}

void Coro_allocStackIfNeeded(Coro *self) {
    if (self->stack && self->requestedStackSize < self->allocatedStackSize) {
        io_free(self->stack);
        self->stack = NULL;
        self->requestedStackSize = 0;
    }

    if (!self->stack) {
        self->stack = (void *)io_calloc(1, self->requestedStackSize + 16);
        self->allocatedStackSize = self->requestedStackSize;
        STACK_REGISTER(self);
    }
}

void Coro_freeStack(Coro *self) {
    STACK_DEREGISTER(self);
    if (self->stack) {
        io_free(self->stack);
    }
    self->stack = NULL;
}

void Coro_StartWithArg(void* ptr) {
    CallbackBlock *block = ptr;
    block->func(block->context);
    fprintf(stderr, "Scheduler error: returned from coro start function\n");
    exit(-1);
}

// stack

void *Coro_stack(Coro *self) { return self->stack; }

size_t Coro_stackSize(Coro *self) { return self->requestedStackSize; }

void Coro_setStackSize_(Coro *self, size_t sizeInBytes) {
    self->requestedStackSize = sizeInBytes;
}

#ifdef _MSC_VER
#include <intrin.h>
ptrdiff_t *Coro_CurrentStackPointer(void) { return _AddressOfReturnAddress(); }
#else
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif
[[noinline]] ptrdiff_t *Coro_CurrentStackPointer(void) {
    // Use the built-in if we have it
#if __has_builtin(__builtin_stack_address) 
    return __builtin_stack_address();
#else
    ptrdiff_t a = 0;
    ptrdiff_t *b = &a; // to avoid compiler warning about unused variables
    // ptrdiff_t *c = a ^ (b ^ a); // to avoid
    return b;
#endif
}
#endif

size_t Coro_bytesLeftOnStack(Coro *self) {
    unsigned char dummy = 0;
    ptrdiff_t p1 = (ptrdiff_t)(&dummy);
    ptrdiff_t p2 = (ptrdiff_t)Coro_CurrentStackPointer();
    int stackMovesUp = p2 > p1;
    ptrdiff_t start = ((ptrdiff_t)self->stack);
    ptrdiff_t end = start + self->requestedStackSize;

    if (stackMovesUp) // like PPC
    {
        return end - p1;
    } else // like x86
    {
        return p1 - start;
    }
}

int Coro_stackSpaceAlmostGone(Coro *self) {
    return Coro_bytesLeftOnStack(self) < CORO_STACK_SIZE_MIN;
}

void Coro_startCoro_(Coro *self, Coro *other, void *context,
                     CoroStartCallback *callback) {
    CallbackBlock sblock={context, callback};
    Coro_setup(other, &sblock);
    Coro_switchTo_(self, other);
}


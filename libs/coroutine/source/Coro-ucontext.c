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

#include "Coro.h"
#include "Coro-internal.h"
#include <PortableStdint.h>
/* Define XOPEN as ucontext is not in posix (or whatever) */
#define _XOPEN_SOURCE
#include <ucontext.h>
CORO_API const char *Coro_Implementation= "ucontext"; 

typedef struct Coro_ucontext {
    struct Coro base;
    ucontext_t env;
} Coro_ucontext;

static ucontext_t* env(Coro* self) {
    return &((Coro_ucontext*) self)->env;
}

// ---- New and free --------------------------------

Coro* Coro_new(void) {
    Coro_ucontext* self = io_calloc(1, sizeof *self);
    /* Don't need any specific allocations for Coro_ucontext */
    return Coro_initBase(&self->base);
}

void Coro_free(Coro* self) {
    Coro_freeStack(self);
    /* Don't need any specific deallocs for Coro_ucontext */
    io_free(self);
}

// ---- setup ------------------------------------------

typedef void (*makecontext_func)(void);

#if UINTPTR_MAX > UINT_MAX
#define SPLIT_POINTER
/* According to makecontext(3) the args passed to `func' have to be int-sized. 
 * To pass a pointer to Coro_startWithArg, split it up into high and low
 * ints and re-assemble with this wrapper function. 
*/
static void start_with_arg_wrapper(unsigned int hi, unsigned int lo) {
    uintptr_t iptr = (uintptr_t) lo;
    iptr |= ((uintptr_t) hi) << 32; 
    Coro_StartWithArg((void*) iptr);
}
#endif

void Coro_setup(Coro *self, void *arg, CoroStartCallback* callback) {
    Coro_ucontext* uself = (Coro_ucontext*) self;
    ucontext_t *ucp = &uself->env;
    self->callback = callback;
    self->context = arg;
    getcontext(ucp);
    Coro_allocStackIfNeeded(self);

    ucp->uc_stack.ss_sp = Coro_stack(self);
    ucp->uc_stack.ss_size = Coro_stackSize(self);
    ucp->uc_stack.ss_flags = 0;
    ucp->uc_link = NULL;

    #ifdef SPLIT_POINTER
    unsigned int hiArg = (unsigned int)((uintptr_t)self >> 32);
    unsigned int loArg = (unsigned int)((uintptr_t)self & 0xFFFFFFFF);
    makecontext(ucp, (makecontext_func)start_with_arg_wrapper, 2, hiArg, loArg);
    #else
    makecontext(ucp, (makecontext_func)Coro_StartWithArg, 1, (unsigned int)self);
    #endif
}

void Coro_initializeMainCoro(Coro *self) {
    self->isMain = 1;
}

// ---- switch to --------------------------------------

void Coro_switchTo_(Coro *self, Coro *next) {
    ucontext_t* self_env = env(self);
    ucontext_t* next_env = env(next);
    swapcontext(self_env, next_env);
}


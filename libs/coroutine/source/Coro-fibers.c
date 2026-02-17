/*
* Implementation using Windows Fibers
*/

#include "Coro.h"
#include "Coro-internal.h"
#include "Windows.h"
#include <stdio.h>

struct Coro_fiber {
    struct Coro base;
    void *fiber;
};
typedef struct Coro_fiber Coro_fiber;
static void *fiber(Coro *coro) { return ((Coro_fiber *)coro)->fiber; }


// ---- New and free --------------------------------

Coro* Coro_new(void) {
    Coro_fiber* self = io_calloc(1, sizeof *self);
    if (self) return Coro_initBase(&self->base);
    return NULL;
}

void Coro_free(Coro* self) {
    self = Coro_deinitBase(self);
    void * f=fiber(self);
    if (f!=GetCurrentFiber()) {
        DeleteFiber(f);
    }
    io_free(self);
}
// ---- switch to --------------------------------------

void Coro_switchTo_(Coro *self, Coro *next) {
    void *nfiber = fiber(next);
    SwitchToFiber(nfiber);
}

// ---- setup ------------------------------------------
struct FiberCallbackBlock {
    void *arg;
    Coro_fiber *coro;
};
extern ptrdiff_t *Coro_CurrentStackPointer(void);
void fiber_start_wrapper(struct FiberCallbackBlock *fcb) {
    fcb->coro->base.stackBase = Coro_CurrentStackPointer();
    Coro_StartWithArg(fcb->arg);
}
void Coro_setup(Coro *self, void *arg) {
    Coro_fiber* uself = (Coro_fiber*) self;
    // Make sure we are in a fiber.
    if (!IsThreadAFiber()) {
        ConvertThreadToFiber(self);
    }
    // For Fibers we don't alloc our own stack
    static struct FiberCallbackBlock fcb;
    fcb.arg = arg;
    fcb.coro = uself;
    uself->fiber =
        CreateFiber(uself->base.requestedStackSize, fiber_start_wrapper, &fcb);
}



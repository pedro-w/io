/*
* Implementation using Windows Fibers
*/

#include "Coro.h"
#include "Coro-internal.h"
#include "Windows.h"
#include <intrin.h>

struct Coro_fiber {
    struct Coro base;
    void *fiber;
};
typedef struct Coro_fiber Coro_fiber;
static Coro_fiber *DATA(Coro *coro) { return (Coro_fiber *)coro; }
const char *Coro_Implementation ="fibers"; 


// ---- New and free --------------------------------

Coro *Coro_new(void) {
    Coro_fiber *self = io_calloc(1, sizeof *self);
    if (self) {
        return Coro_initBase(&self->base);
    } else {
        return NULL;
    }
}

void Coro_free(Coro* self) {
    // Didn't allocate stack so don't free it.
    Coro_fiber *uself = DATA(self);
    // Delete the fiber but not if it's the current one
    if (uself->fiber!=GetCurrentFiber()) {
        DeleteFiber(uself->fiber);
    }
    io_free(self);
}
// ---- switch to --------------------------------------

void Coro_switchTo_(Coro *self, Coro *next) {
    void *nfiber = DATA(next)->fiber;
    SwitchToFiber(nfiber);
}

// ---- setup ------------------------------------------
static void fiber_start_wrapper(Coro_fiber *fcb) {
    fcb->base.stack = _AddressOfReturnAddress();
    Coro_StartWithArg(fcb);
}
void Coro_setup(Coro *self, void *arg, CoroStartCallback* callback) {
    Coro_fiber *uself = DATA(self);

    // For Fibers we don't alloc our own stack
    self->context = arg;
    self->callback = callback;
    uself->fiber =
        CreateFiber(self->requestedStackSize, fiber_start_wrapper, uself);
}


void Coro_initializeMainCoro(Coro *self) {
    Coro_fiber *uself = DATA(self);
    self->isMain = 1;
    // Make sure we are in a fiber.
    if (IsThreadAFiber()) {
        uself->fiber = GetCurrentFiber();
    } else {
        uself->fiber = ConvertThreadToFiber(self);
    }
}
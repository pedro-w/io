/*
* Implementation using Windows Fibers
*/

#include "Coro.h"
#include "Coro-internal.h"

#error NOT IMPLEMENTED YET

struct Coro_fiber {
    struct Coro base;
};
typedef struct Coro_fiber Coro_fiber;


// ---- New and free --------------------------------

Coro* Coro_new(void) {
    Coro_fiber* self = io_calloc(1, sizeof *self);
    return Coro_initBase(&self->base);
}

void Coro_free(Coro* self) {
    self = Coro_deinitBase(self);
    /* Don't need any specific deallocs for Coro_ucontext */
    io_free(self);
}
// ---- switch to --------------------------------------

void Coro_switchTo_(Coro *self, Coro *next) {
    
}

// ---- setup ------------------------------------------

void Coro_setup(Coro *self, void *arg) {
    Coro_fiber* uself = (Coro_fiber*) self;
}



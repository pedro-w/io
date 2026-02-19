/*
* Coroutine implementation for ARM64
* ARM64 support by PedroBatista
* This is a 'bare-metal' version that uses ASM
*/
#include "Coro.h"
#include "Coro-internal.h"

const char *Coro_Implementation="arm64-native"; 


// Custom context implementation for ARM64
typedef struct {
    unsigned long x19_x20[2];  // x19, x20
    unsigned long x21_x22[2];  // x21, x22
    unsigned long x23_x24[2];  // x23, x24
    unsigned long x25_x26[2];  // x25, x26
    unsigned long x27_x28[2];  // x27, x28
    unsigned long fp_lr[2];    // x29 (fp), x30 (lr)
    unsigned long sp;          // stack pointer
    unsigned long dummy;
    unsigned long v8_v15[8];    // Floating point reg
} arm64_context_t;

typedef struct {
    Coro base;
    arm64_context_t env;
} Coro_arm64;

arm64_context_t* env(Coro* coro) {
    return &((Coro_arm64*) coro)->env;
}

typedef struct CallbackBlock {
    void *context;
    CoroStartCallback *func;
} CallbackBlock;

static CallbackBlock globalCallbackBlock;

__attribute__((naked,noinline))
static int coro_arm64_getcontext(arm64_context_t* context) {
    // x0 contains the context pointer
asm(
    "stp x19, x20, [x0, #0]\n"
    "stp x21, x22, [x0, #16]\n"
    "stp x23, x24, [x0, #32]\n"
    "stp x25, x26, [x0, #48]\n"
    "stp x27, x28, [x0, #64]\n"
    "stp x29, x30, [x0, #80]\n"
    "mov x2, sp\n"
    "str x2, [x0, #96]\n"
    "stp d8, d9, [x0, #112]\n"
    "stp d10,d11, [x0, #128]\n"
    "stp d12,d13, [x0, #144]\n"
    "stp d14,d15, [x0, #160]\n"

    "mov x0, #0\n"
    "ret\n");
}

__attribute__((naked,noinline))
static int coro_arm64_setcontext(arm64_context_t* context) {
    // x0 contains the context pointer
asm(
    "ldp x19, x20, [x0, #0]\n"
    "ldp x21, x22, [x0, #16]\n"
    "ldp x23, x24, [x0, #32]\n"
    "ldp x25, x26, [x0, #48]\n"
    "ldp x27, x28, [x0, #64]\n"
    "ldp x29, x30, [x0, #80]\n"
    "ldr x2, [x0, #96]\n"
    "mov sp, x2\n"
    "ldp d8,d9, [x0, #112]\n"
    "ldp d10,d11, [x0, #128]\n"
    "ldp d12,d13, [x0, #144]\n"
    "ldp d14,d15, [x0, #160]\n"
  
    "mov x0, #1\n"
    "ret\n");
}

void Coro_free(Coro *self) {
    Coro_deinitBase(self);
    io_free(self);
}

Coro *Coro_new(void) {
    Coro_arm64 *coro = io_calloc(1, sizeof *coro);
    return Coro_initBase(&coro->base);
}

// This function initializes the context with a new stack and entry point
void Coro_setup(Coro *self, void *arg) {
    arm64_context_t* context = env(self);
    Coro_allocStackIfNeeded(self);

    // Initialize stack pointer to top of stack (ARM64 full descending stack)
    unsigned long sp = (unsigned long)self->stack + self->allocatedStackSize - 16;
    // Ensure 16-byte alignment
    sp &= ~15UL;
    // Store stack pointer in context
    context->sp = sp;
    
    // Store entry point in link register (x30)
    context->fp_lr[1] = (unsigned long)Coro_StartWithArg;
}

void Coro_switchTo_(Coro *self, Coro *next) {
    // Get the context pointers
    arm64_context_t *from_context = env(self);
    arm64_context_t *to_context = env(next);
    
    // Save current context, if successful (returns 0), then restore the next context
    if (coro_arm64_getcontext(from_context) == 0) {
        coro_arm64_setcontext(to_context);
    }
    
}

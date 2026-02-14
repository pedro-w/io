/*
* Coroutine implementation for ARM32
* This is a 'bare-metal' version that uses ASM
*/

#include <stdio.h>

#include "Coro.h"
#include "Coro-internal.h"

// TODO these are implemented for Valgrind
#define STACK_REGISTER(coro)
#define STACK_DEREGISTER(coro)

// Custom context implementation for ARM32
typedef struct {
    unsigned int r0;
    unsigned int r1;
    unsigned int r2;
    unsigned int r3;
    unsigned int r4;
    unsigned int r5;
    unsigned int r6;
    unsigned int r7;
    unsigned int r8;
    unsigned int r9;
    unsigned int r10;
    unsigned int r11;
    unsigned int r12;
	union {
		unsigned int sp;// stack pointer
    	unsigned int r13; 
	};
	union {
		unsigned int lr;// link register
    	unsigned int r14; 
	};
} arm32_context_t;

__attribute__((naked, noinline))
static int coro_arm32_getcontext(arm32_context_t* context) {
asm (
	"str r1, [r0,#4]\n"
	"str r2, [r0,#8]\n"
	"str r3, [r0,#12]\n"
	"str r4, [r0,#16]\n"
	"str r5, [r0,#20]\n"
	"str r6, [r0,#24]\n"
	"str r7, [r0,#28]\n"
	"str r8, [r0,#32]\n"
	"str r9, [r0,#36]\n"
	"str r10, [r0,#40]\n"
	"str r11, [r0,#44]\n"
	"str r12, [r0,#48]\n"
	"str r13, [r0,#52]\n"
	"str r14, [r0,#56]\n"
	/* store 1 as r0-to-restore */
	"mov r1, #1\n"
	"str r1, [r0]\n"
	/* return 0 */
	"mov r0, #0\n"
	"bx lr");
}

__attribute__((naked, noinline))
static int coro_arm32_setcontext(arm32_context_t* context) {
	asm(
	"ldr r1, [r0,#4]\n"
	"ldr r2, [r0,#8]\n"
	"ldr r3, [r0,#12]\n"
	"ldr r4, [r0,#16]\n"
	"ldr r5, [r0,#20]\n"
	"ldr r6, [r0,#24]\n"
	"ldr r7, [r0,#28]\n"
	"ldr r8, [r0,#32]\n"
	"ldr r9, [r0,#36]\n"
	"ldr r10, [r0,#40]\n"
	"ldr r11, [r0,#44]\n"
	"ldr r12, [r0,#48]\n"
	"ldr r13, [r0,#52]\n"
	"ldr r14, [r0,#56]\n"
	"ldr r0, [r0]\n"
	"bx lr\n");
}

typedef struct {
	Coro base;
	arm32_context_t env;
} Coro_arm32;

arm32_context_t* env(Coro* coro) {
	Coro_arm32* acoro = (Coro_arm32*) coro;
	return &acoro->env;
}

// ---- New and free --------------------------------

Coro* Coro_new(void) {
    Coro_arm32* self = io_calloc(1, sizeof *self);
    memset(&self->env, 0xCC, sizeof self->env);
    return Coro_initBase(&self->base);
}

void Coro_free(Coro* self) {
    self = Coro_deinitBase(self);
    /* Don't need any specific deallocs for Coro_arm32 */
    io_free(self);
}

// --------------------------------------------------------------------

void Coro_setup(Coro *self, void *arg) {
    arm32_context_t *context = env(self);
    
    // Initialize stack pointer to top of stack (ARM32 descending stack)
    unsigned int sp = (unsigned int)self->stack + self->allocatedStackSize - 16;
    // Ensure 16-byte alignment
    sp &= ~15UL;
    
    // Store stack pointer in context
    context->sp = sp;
    context->r0 = (unsigned int) arg;
    // Store entry point in link register 
    context->lr = (unsigned int)Coro_StartWithArg;
}

// --------------------------------------------------------------------

void Coro_switchTo_(Coro *self, Coro *next) {
    // Get the context pointers
    arm32_context_t *from_context = env(self);
    arm32_context_t *to_context = env(next);
    
    // Save current context, if successful (returns 0), then restore the next context
    if (coro_arm32_getcontext(from_context) == 0) {
        coro_arm32_setcontext(to_context);
    }
}









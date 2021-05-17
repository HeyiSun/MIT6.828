/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.data; \
		.long name;  \
	.text; \
		.globl name;		/* define global symbol for 'name' */	\
		.type name, @function;	/* symbol type is function */		\
		.align 2;		/* align function definition */		\
		name:			/* function starts here */		\
		pushl $(num);							\
		jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.data; \
		.long name;  \
	.text; \
		.globl name;							\
		.type name, @function;						\
		.align 2;							\
		name:								\
		pushl $0;							\
		pushl $(num);							\
		jmp _alltraps

.data 
    .globl vectors

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */
vectors:
	TRAPHANDLER_NOEC(t_divide_handler, T_DIVIDE)  // 0
	TRAPHANDLER_NOEC(t_debug_handler, T_DEBUG)    // 1
	TRAPHANDLER_NOEC(t_nmi_handler, T_NMI)        // 2
	TRAPHANDLER_NOEC(t_brkpt_handler, T_BRKPT)    // 3
	TRAPHANDLER_NOEC(t_oflow_handler, T_OFLOW)    // 4
	TRAPHANDLER_NOEC(t_bound_handler, T_BOUND)    // 5
	TRAPHANDLER_NOEC(t_illop_handler, T_ILLOP)    // 6
	TRAPHANDLER_NOEC(t_device_handler, T_DEVICE)  // 7
	TRAPHANDLER(t_dbflt_handler, T_DBLFLT)        // 8
	TRAPHANDLER(t_9_handler, 9)           // 9
	TRAPHANDLER(t_tss_handler, T_TSS)             // 10
	TRAPHANDLER(t_segnp_handler, T_SEGNP)         // 11
	TRAPHANDLER(t_stack_handler, T_STACK)         // 12
	TRAPHANDLER(t_gpfault_handler, T_GPFLT)       // 13
	TRAPHANDLER(t_pgfault_handler, T_PGFLT)       // 14
	TRAPHANDLER(t_15_handler, 15)        // 15
	TRAPHANDLER_NOEC(t_fperr_handler, T_FPERR)    // 16

	TRAPHANDLER(t_17_handler, 17)        // 17
	TRAPHANDLER(t_18_handler, 18)        // 18
	TRAPHANDLER(t_19_handler, 19)        // 19
	TRAPHANDLER(t_20_handler, 20)        // 20
	TRAPHANDLER(t_21_handler, 21)        // 21
	TRAPHANDLER(t_22_handler, 22)        // 22
	TRAPHANDLER(t_23_handler, 23)        // 23
	TRAPHANDLER(t_24_handler, 24)        // 24
	TRAPHANDLER(t_25_handler, 25)        // 25
	TRAPHANDLER(t_26_handler, 26)        // 26
	TRAPHANDLER(t_27_handler, 27)        // 27
	TRAPHANDLER(t_28_handler, 28)        // 28
	TRAPHANDLER(t_29_handler, 29)        // 29
	TRAPHANDLER(t_30_handler, 30)        // 30
	TRAPHANDLER(t_31_handler, 31)        // 31
	TRAPHANDLER_NOEC(t_32_handler, 32)        // 32  IRQ0
	TRAPHANDLER_NOEC(t_33_handler, 33)        // 33   |
	TRAPHANDLER_NOEC(t_34_handler, 34)        // 34   |
	TRAPHANDLER_NOEC(t_35_handler, 35)        // 35   |
	TRAPHANDLER_NOEC(t_36_handler, 36)        // 36   |
	TRAPHANDLER_NOEC(t_37_handler, 37)        // 37   |
	TRAPHANDLER_NOEC(t_38_handler, 38)        // 38   |
	TRAPHANDLER_NOEC(t_39_handler, 39)        // 39   |
	TRAPHANDLER_NOEC(t_40_handler, 40)        // 40   |
	TRAPHANDLER_NOEC(t_41_handler, 41)        // 41   |
	TRAPHANDLER_NOEC(t_42_handler, 42)        // 42   |
	TRAPHANDLER_NOEC(t_43_handler, 43)        // 43   |
	TRAPHANDLER_NOEC(t_44_handler, 44)        // 44   |
	TRAPHANDLER_NOEC(t_45_handler, 45)        // 45  \|/
	TRAPHANDLER_NOEC(t_46_handler, 46)        // 46   V
	TRAPHANDLER_NOEC(t_47_handler, 47)        // 47  IRQ15
	
	TRAPHANDLER_NOEC(handler_syscall, T_SYSCALL)
/*
 * Lab 3: Your code here for _alltraps
 */

//TODO: .global _alltraps
.text
_alltraps:
	pushl %ds
	pushl %es
	pushal
	push $(GD_KD)
	pop %ds
	push $(GD_KD)
	pop %es
	pushl %esp
	call trap


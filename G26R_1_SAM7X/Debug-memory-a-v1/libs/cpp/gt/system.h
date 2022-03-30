#ifndef GT__AT91SAM7XC__SYSTEM_H__INCLUDED
#define GT__AT91SAM7XC__SYSTEM_H__INCLUDED

#define nop() __nop()

#define CLOCK_IN	18432000ul		// Hz
#define CLOCK_PLL	CLOCK_IN*(124+1)/24	// Hz
#define CLOCK_MCK	CLOCK_PLL/2		// Hz

#define EnableFIQ() {\
	asm volatile ("@ System::EnableFIQ ");\
	asm volatile ("stmdb sp!, {r0}     "); \
	asm volatile ("mrs   r0, CPSR      ");  \
	asm volatile ("bic   r0, r0, #64   ");   \
	asm volatile ("msr   CPSR_fsxc, r0 ");    \
	asm volatile ("ldmia sp!, {r0}     ");     \
	asm volatile ("@ ------------------"); }    \

#define DisableFIQ() {                         \
	asm volatile ("@ System::DisableFIQ");  \
	asm volatile ("stmdb sp!, {r0}     ");   \
	asm volatile ("mrs   r0, CPSR      ");    \
	asm volatile ("orr   r0, r0, #64   ");     \
	asm volatile ("msr   CPSR_fsxc, r0 ");      \
	asm volatile ("ldmia sp!, {r0}     ");       \
	asm volatile ("@ ------------------"); }      \

#define EnableIRQ() {                                  \
	asm volatile ("@ System::EnableIRQ ");          \
	asm volatile ("stmdb sp!, {r0}     ");           \
	asm volatile ("mrs   r0, CPSR      ");            \
	asm volatile ("bic   r0, r0, #128  ");             \
	asm volatile ("msr   CPSR_fsxc, r0 ");              \
	asm volatile ("ldmia sp!, {r0}     ");               \
	asm volatile ("@ ------------------"); }              \

#define DisableIRQ() {                                         \
	asm volatile ("@ System::DisableIRQ");                  \
	asm volatile ("stmdb sp!, {r0}     ");                   \
	asm volatile ("mrs   r0, CPSR      ");                    \
	asm volatile ("orr   r0, r0, #128  ");                     \
	asm volatile ("msr   CPSR_fsxc, r0 ");                      \
	asm volatile ("ldmia sp!, {r0}     ");                       \
	asm volatile ("@ ------------------"); }                      \

#endif//GT__AT91SAM7XC__SYSTEM_H__INCLUDED

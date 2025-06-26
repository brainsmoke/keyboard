
#include <keyboard_leds.h>

.include "m0delay.S"

	.cpu cortex-m0
	.fpu softvfp
	.section	.bss
	.thumb
	.section	.text.bitbang,"ax",%progbits
	.align	2

	.global	bitbang_56bits_stm32
	.code	16
	.thumb_func
	.type	bitbang_56bits_stm32, %function
bitbang_56bits_stm32:
	.syntax unified
	.thumb

#define rBUF   r0
#define rGPIO_DATA_CLK r1
#define rGPIO_LATCH_EN r2
#define rOUTPUT_ENABLE_MASK r3
#define rCLK   r4
#define rOUT   r5
bitbang_56bits_stm32_start:

/* 0 + 3 */ push {r4,r5}
/* 3 + 2 */ ldr rCLK, =MASK_CLK
/* 5 + 2 */ ldrb rOUT, [rBUF, #0]
/* 7 + 1 */ lsls rOUT, #8
.rept 55
/* 0 + 2 */ strh rOUT, [rGPIO_DATA_CLK, #GPIO_ODR_OFFSET]
/* 2 + 2 */ ldrb rOUT, [rBUF, #1]
/* 4 + 2 */ strh rCLK, [rGPIO_DATA_CLK, #GPIO_BSRR_OFFSET]
/* 6 + 1 */ adds rBUF, #1
/* 7 + 1 */ lsls rOUT, #8
.endr
/* 448 + 2 */ strh rOUT, [rGPIO_DATA_CLK, #GPIO_ODR_OFFSET]
#undef rBUF
#define rOUT2 r0

/* 450 + 2 */ ldr rOUT2, =BIT_LATCH
/* 452 + 2 */ strh rCLK, [rGPIO_DATA_CLK, #GPIO_BSRR_OFFSET]
/* 454 + 1 */ orrs rOUT, rCLK
/* 455 + 1 */ orrs rOUT2, rOUTPUT_ENABLE_MASK

/* 456 + 2 */ str rOUT2, [rGPIO_LATCH_EN, #GPIO_BSRR_OFFSET]
/* 458 + 2 */ str rOUT, [rGPIO_DATA_CLK, #GPIO_BRR_OFFSET]
/* 460 + 2 */ str rOUT2, [rGPIO_LATCH_EN, #GPIO_BRR_OFFSET]
/* 462 + 3 */ pop {r4,r5}

bitbang_56bits_stm32_end:
/* 465 + 3 */ bx lr
/* 468     */

#undef rOUT2
#undef rGPIO_DATA_CLK
#undef rGPIO_LATCH_EN
#undef rOUTPUT_ENABLE_MASK
#undef rCLK
#undef rOUT

	.size	bitbang_56bits_stm32, .-bitbang_56bits_stm32

	.global	write_wait_write
	.code	16
	.thumb_func
	.type	write_wait_write, %function
write_wait_write:
	.syntax	unified
	.thumb

cmp r3, #11
bge 11f
cmp r3, #9
bgt 10f
beq 9f
cmp r3, #7
bgt 8f
beq 7f
cmp r3, #5
bgt 6f
beq 5f
cmp r3, #3
bgt 4f
beq 3f
2:
str r1, [r0]
str r2, [r0]
bx lr
3:
str r1, [r0]
delay r3, 1
str r2, [r0]
bx lr
4:
str r1, [r0]
delay r3, 2
str r2, [r0]
bx lr
5:
str r1, [r0]
delay r3, 3
str r2, [r0]
bx lr
6:
str r1, [r0]
delay r3, 4
str r2, [r0]
bx lr
7:
str r1, [r0]
delay r3, 5
str r2, [r0]
bx lr
8:
str r1, [r0]
delay r3, 6
str r2, [r0]
bx lr
9:
str r1, [r0]
delay r3, 7
str r2, [r0]
bx lr
10:
str r1, [r0]
delay r3, 8
str r2, [r0]
bx lr
11:
str r1, [r0]
delay_reg r3, r1, 2
str r2, [r0]
bx lr

	.size	write_wait_write, .-write_wait_write


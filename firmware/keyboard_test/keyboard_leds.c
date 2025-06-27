
#include <stdlib.h>
#include <string.h>

#include "keyboard_leds.h"
#include "util.h"
#include "millis.h"

typedef struct __attribute__((packed,aligned(4)))
{
	uint8_t bit[16][MAX_BITS_PER_CHANNEL]; 
} frame_t;

frame_t frame_a, frame_b;

static frame_t * volatile cur_frame;
static frame_t * volatile next_frame;
static frame_t * volatile draw_frame;

static uint16_t iter;

enum
{
	B15, B14, B13, B12, B11, B10, B9, B8,
	B7, B6, B5, B4, B3, B2, B1, B0,
	E0, E1, E2, E3, E4, E5, E6, E7, E8,
	ZZZ,
};

/* SysTick dispatch table for BCM bitbang */
static const uint8_t dtable[] =
{
/*	 15    .    .    .    .    .    .    .   14    .    .    .   13    .   12  11/10 dith */

	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B11,  B8, E8,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B10,  B4, E4,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B11,  B6, E6,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12,  B9,  B2, E2,

	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B11,  B7, E7,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B10,  B3, E3,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B11,  B5, E5,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12,       B1, E2,

	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B11,  B8, E8,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B10,  B4, E4,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B11,  B6, E6,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12,  B9,  B2, E2,

	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B11,  B7, E7,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B10,  B3, E3,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12, B11,  B5, E5,
	B15, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, ZZZ, B14, ZZZ, ZZZ, ZZZ, B13, ZZZ, B12,       B0, E1,

};

#define TABLE_SIZE (sizeof(dtable)/sizeof(dtable[0]))

static void bitbang_15(void) { bitbang_56bits_stm32(cur_frame->bit[15], (void *)GPIO_LEDS_DATA, (void *)GPIO_LEDS_LATCH, 0); draw_frame = cur_frame; }
static void bitbang_14(void) { bitbang_56bits_stm32(draw_frame->bit[14], (void *)GPIO_LEDS_DATA, (void *)GPIO_LEDS_LATCH, 0); }
static void bitbang_13(void) { bitbang_56bits_stm32(draw_frame->bit[13], (void *)GPIO_LEDS_DATA, (void *)GPIO_LEDS_LATCH, 0); }
static void bitbang_12(void) { bitbang_56bits_stm32(draw_frame->bit[12], (void *)GPIO_LEDS_DATA, (void *)GPIO_LEDS_LATCH, 0); }
static void bitbang_11(void) { bitbang_56bits_stm32(draw_frame->bit[11], (void *)GPIO_LEDS_DATA, (void *)GPIO_LEDS_LATCH, 0); }
static void bitbang_10(void) { bitbang_56bits_stm32(draw_frame->bit[10], (void *)GPIO_LEDS_DATA, (void *)GPIO_LEDS_LATCH, 0); }
static void bitbang_9(void)  { bitbang_56bits_stm32(draw_frame->bit[ 9], (void *)GPIO_LEDS_DATA, (void *)GPIO_LEDS_LATCH, 0); }

static void bitbang_8(void)  { bitbang_56bits_stm32(draw_frame->bit[ 8], (void *)GPIO_LEDS_DATA, (void *)GPIO_LEDS_LATCH, BIT_NOT_OUTPUT_ENABLE); }
static void bitbang_7(void)  { bitbang_56bits_stm32(draw_frame->bit[ 7], (void *)GPIO_LEDS_DATA, (void *)GPIO_LEDS_LATCH, BIT_NOT_OUTPUT_ENABLE); }
static void bitbang_6(void)  { bitbang_56bits_stm32(draw_frame->bit[ 6], (void *)GPIO_LEDS_DATA, (void *)GPIO_LEDS_LATCH, BIT_NOT_OUTPUT_ENABLE); }
static void bitbang_5(void)  { bitbang_56bits_stm32(draw_frame->bit[ 5], (void *)GPIO_LEDS_DATA, (void *)GPIO_LEDS_LATCH, BIT_NOT_OUTPUT_ENABLE); }
static void bitbang_4(void)  { bitbang_56bits_stm32(draw_frame->bit[ 4], (void *)GPIO_LEDS_DATA, (void *)GPIO_LEDS_LATCH, BIT_NOT_OUTPUT_ENABLE); }
static void bitbang_3(void)  { bitbang_56bits_stm32(draw_frame->bit[ 3], (void *)GPIO_LEDS_DATA, (void *)GPIO_LEDS_LATCH, BIT_NOT_OUTPUT_ENABLE); }
static void bitbang_2(void)  { bitbang_56bits_stm32(draw_frame->bit[ 2], (void *)GPIO_LEDS_DATA, (void *)GPIO_LEDS_LATCH, BIT_NOT_OUTPUT_ENABLE); }
static void bitbang_1(void)  { bitbang_56bits_stm32(draw_frame->bit[ 1], (void *)GPIO_LEDS_DATA, (void *)GPIO_LEDS_LATCH, BIT_NOT_OUTPUT_ENABLE); }
static void bitbang_0(void)  { bitbang_56bits_stm32(draw_frame->bit[ 0], (void *)GPIO_LEDS_DATA, (void *)GPIO_LEDS_LATCH, BIT_NOT_OUTPUT_ENABLE); }

#define FLIP_OFF (SET(BIT_NOT_OUTPUT_ENABLE))
#define FLIP_ON  (CLEAR(BIT_NOT_OUTPUT_ENABLE))
#define SYSTICK_PERIOD ((uint32_t)(F_SYS_TICK_CLK/(TABLE_SIZE*200) ))
#define SYSTICK_CYCLES ((uint32_t)(8*SYSTICK_PERIOD))

static void enable_8(void) { write_wait_write(&GPIO_BSRR(GPIO_LEDS_LATCH), FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>1); }
static void enable_7(void) { write_wait_write(&GPIO_BSRR(GPIO_LEDS_LATCH), FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>2); }
static void enable_6(void) { write_wait_write(&GPIO_BSRR(GPIO_LEDS_LATCH), FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>3); }
static void enable_5(void) { write_wait_write(&GPIO_BSRR(GPIO_LEDS_LATCH), FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>4); }
static void enable_4(void) { write_wait_write(&GPIO_BSRR(GPIO_LEDS_LATCH), FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>5); }
static void enable_3(void) { write_wait_write(&GPIO_BSRR(GPIO_LEDS_LATCH), FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>6); }
static void enable_2(void) { write_wait_write(&GPIO_BSRR(GPIO_LEDS_LATCH), FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>7); }
static void enable_1(void) { write_wait_write(&GPIO_BSRR(GPIO_LEDS_LATCH), FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>8); }
static void enable_0(void) { write_wait_write(&GPIO_BSRR(GPIO_LEDS_LATCH), FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>9); }

static void ret(void) { }

typedef void (*func_t)(void);

const func_t dispatch[] =
{

	[B15] = bitbang_15,
	[B14] = bitbang_14,
	[B13] = bitbang_13,
	[B12] = bitbang_12,
	[B11] = bitbang_11,
	[B10] = bitbang_10,
	[B9] = bitbang_9,
	[B8] = bitbang_8,
	[B7] = bitbang_7,
	[B6] = bitbang_6,
	[B5] = bitbang_5,
	[B4] = bitbang_4,
	[B3] = bitbang_3,
	[B2] = bitbang_2,
	[B1] = bitbang_1,
	[B0] = bitbang_0,

	[E8] = enable_8,
	[E7] = enable_7,
	[E6] = enable_6,
	[E5] = enable_5,
	[E4] = enable_4,
	[E3] = enable_3,
	[E2] = enable_2,
	[E1] = enable_1,
	[E0] = enable_0,

//	[OFF] = off,
	[ZZZ]= ret,
};

void SysTick_Handler(void)
{
	dispatch[dtable[iter]]();

	if (iter < TABLE_SIZE-1)
		iter = iter+1;
	else
		iter = 0;
}

static uint16_t gamma_map[256] = {
/*
max_val=0xffff
gamma=2.6
factor = max_val / (255.**gamma)
gamma_map = [ int(.5 + x**gamma * factor) for x in range(256) ]
for i in range(0, 256, 16):
    print ( '\t'+' '.join(f'0x{x:04x},' for x in gamma_map[i:i+16]) )
*/

	0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0002, 0x0004, 0x0006, 0x0008, 0x000b, 0x000e, 0x0012, 0x0017, 0x001d, 0x0023, 0x0029,
	0x0031, 0x0039, 0x0043, 0x004d, 0x0058, 0x0063, 0x0070, 0x007e, 0x008d, 0x009c, 0x00ad, 0x00bf, 0x00d2, 0x00e6, 0x00fb, 0x0112,
	0x0129, 0x0142, 0x015c, 0x0177, 0x0194, 0x01b1, 0x01d0, 0x01f1, 0x0213, 0x0236, 0x025a, 0x0280, 0x02a8, 0x02d1, 0x02fb, 0x0327,
	0x0355, 0x0383, 0x03b4, 0x03e6, 0x041a, 0x044f, 0x0486, 0x04bf, 0x04f9, 0x0535, 0x0572, 0x05b2, 0x05f3, 0x0636, 0x067a, 0x06c1,
	0x0709, 0x0753, 0x079f, 0x07ed, 0x083d, 0x088e, 0x08e2, 0x0937, 0x098e, 0x09e8, 0x0a43, 0x0aa0, 0x0b00, 0x0b61, 0x0bc4, 0x0c2a,
	0x0c91, 0x0cfb, 0x0d67, 0x0dd5, 0x0e45, 0x0eb7, 0x0f2b, 0x0fa1, 0x101a, 0x1095, 0x1112, 0x1192, 0x1213, 0x1297, 0x131d, 0x13a6,
	0x1431, 0x14be, 0x154d, 0x15df, 0x1673, 0x170a, 0x17a3, 0x183e, 0x18dc, 0x197d, 0x1a20, 0x1ac5, 0x1b6d, 0x1c17, 0x1cc4, 0x1d73,
	0x1e25, 0x1ed9, 0x1f90, 0x204a, 0x2106, 0x21c5, 0x2286, 0x234a, 0x2411, 0x24da, 0x25a6, 0x2675, 0x2747, 0x281b, 0x28f2, 0x29cb,
	0x2aa8, 0x2b87, 0x2c69, 0x2d4e, 0x2e35, 0x2f20, 0x300d, 0x30fd, 0x31f0, 0x32e6, 0x33df, 0x34da, 0x35d9, 0x36da, 0x37df, 0x38e6,
	0x39f0, 0x3afe, 0x3c0e, 0x3d21, 0x3e38, 0x3f51, 0x406d, 0x418d, 0x42af, 0x43d5, 0x44fd, 0x4629, 0x4758, 0x488a, 0x49bf, 0x4af7,
	0x4c33, 0x4d71, 0x4eb3, 0x4ff8, 0x5140, 0x528b, 0x53da, 0x552c, 0x5681, 0x57d9, 0x5935, 0x5a94, 0x5bf6, 0x5d5b, 0x5ec4, 0x6031,
	0x61a0, 0x6313, 0x6489, 0x6603, 0x6780, 0x6900, 0x6a84, 0x6c0b, 0x6d96, 0x6f24, 0x70b6, 0x724b, 0x73e3, 0x757f, 0x771f, 0x78c2,
	0x7a69, 0x7c13, 0x7dc0, 0x7f72, 0x8126, 0x82df, 0x849b, 0x865a, 0x881e, 0x89e4, 0x8baf, 0x8d7d, 0x8f4f, 0x9124, 0x92fd, 0x94da,
	0x96ba, 0x989f, 0x9a86, 0x9c72, 0x9e61, 0xa055, 0xa24b, 0xa446, 0xa645, 0xa847, 0xaa4d, 0xac57, 0xae64, 0xb076, 0xb28b, 0xb4a5,
	0xb6c2, 0xb8e3, 0xbb08, 0xbd30, 0xbf5d, 0xc18e, 0xc3c2, 0xc5fb, 0xc837, 0xca78, 0xccbc, 0xcf04, 0xd151, 0xd3a1, 0xd5f5, 0xd84e,
	0xdaaa, 0xdd0b, 0xdf6f, 0xe1d8, 0xe444, 0xe6b5, 0xe92a, 0xeba3, 0xee20, 0xf0a1, 0xf326, 0xf5b0, 0xf83d, 0xfacf, 0xfd65, 0xffff,

};

void init_leds(void)
{
 
	gpio_mode_setup(GPIO_LEDS_DATA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, MASK_CLK|MASK_DATA);
	gpio_set_output_options(GPIO_LEDS_DATA, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH, MASK_CLK|MASK_DATA);

	GPIO_ODR(GPIO_LEDS_LATCH) |= BIT_NOT_OUTPUT_ENABLE;
	gpio_mode_setup(GPIO_LEDS_LATCH, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, BIT_NOT_OUTPUT_ENABLE|BIT_LATCH);
	gpio_set_output_options(GPIO_LEDS_LATCH, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH, BIT_NOT_OUTPUT_ENABLE|BIT_LATCH);

	cur_frame = &frame_a;
	next_frame = &frame_b;
	iter = 0;

	memset(&frame_a, 0, sizeof(frame_a));
	memset(&frame_b, 0, sizeof(frame_b));

	enable_sys_tick(SYSTICK_PERIOD);
}

static const uint8_t channel_pin[] = CHANNEL_PIN;
static const uint8_t channel_start[] = CHANNEL_START;

void write_frame(uint8_t fb[N_LEDS])
{
	uint32_t i, j, led_ix=0;

	while (draw_frame == next_frame);
	memset(next_frame, 0, sizeof(*next_frame));

	for (j=0; j<N_CHANNELS; j++)
	{
		uint32_t bit = channel_pin[j];

		for (i=channel_start[j]; i<MAX_BITS_PER_CHANNEL; i++)
		{
			uint16_t c = gamma_map[fb[led_ix++]];
			uint8_t *p = &next_frame->bit[0][i];
			for ( ; c ; c >>= 1 )
			{
				if (c&1)
					*p |= bit;

				p += sizeof(next_frame->bit[0])/sizeof(*p);
			}
		}
	}

	frame_t *tmp = cur_frame;
	cur_frame = next_frame;
	next_frame = tmp;
}


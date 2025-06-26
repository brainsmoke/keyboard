
/* Joining the FPGA cult next project, I promise */

#include <stdlib.h>
#include <string.h>

#include "obegraensad.h"
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

//#define FLIP_OFF (SET(BIT_NOT_OUTPUT_ENABLE)|CLEAR(BIT_ENABLE_HIGH))
#define FLIP_OFF (SET(BIT_NOT_OUTPUT_ENABLE))
#define FLIP_ON  (CLEAR(BIT_NOT_OUTPUT_ENABLE))
#define SYSTICK_PERIOD ((uint32_t)(F_SYS_TICK_CLK/(TABLE_SIZE*200) ))
#define SYSTICK_CYCLES ((uint32_t)(8*SYSTICK_PERIOD))

static void enable_8(void) { write_wait_write(GPIO_BSSR(GPIO_LEDS_LATCH), FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>1); }
static void enable_7(void) { write_wait_write(GPIO_BSSR(GPIO_LEDS_LATCH), FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>2); }
static void enable_6(void) { write_wait_write(GPIO_BSSR(GPIO_LEDS_LATCH), FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>3); }
static void enable_5(void) { write_wait_write(GPIO_BSSR(GPIO_LEDS_LATCH), FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>4); }
static void enable_4(void) { write_wait_write(GPIO_BSSR(GPIO_LEDS_LATCH), FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>5); }
static void enable_3(void) { write_wait_write(GPIO_BSSR(GPIO_LEDS_LATCH), FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>6); }
static void enable_2(void) { write_wait_write(GPIO_BSSR(GPIO_LEDS_LATCH), FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>7); }
static void enable_1(void) { write_wait_write(GPIO_BSSR(GPIO_LEDS_LATCH), FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>8); }
static void enable_0(void) { write_wait_write(GPIO_BSSR(GPIO_LEDS_LATCH), FLIP_ON, FLIP_OFF, SYSTICK_CYCLES>>9); }

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

uin16_t gamma_map[256] = {
/*
max_val=0xff00
gamma=2.6
factor = max_val / (255.**gamma)
gamma_map = [ int(x**gamma * factor) for x in range(256) ]
for i in range(0, 256, 16):
    print ( '\t'+' '.join(f'0x{x:04x},' for x in gamma_map[i:i+16]) )
*/
	0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0002, 0x0003, 0x0005, 0x0008, 0x000a, 0x000e, 0x0012, 0x0017, 0x001c, 0x0022, 0x0029,
	0x0030, 0x0039, 0x0042, 0x004c, 0x0057, 0x0062, 0x006f, 0x007d, 0x008c, 0x009b, 0x00ac, 0x00be, 0x00d1, 0x00e5, 0x00fa, 0x0110,
	0x0127, 0x0140, 0x015a, 0x0175, 0x0191, 0x01af, 0x01ce, 0x01ee, 0x0210, 0x0233, 0x0258, 0x027d, 0x02a5, 0x02ce, 0x02f8, 0x0323,
	0x0351, 0x037f, 0x03b0, 0x03e2, 0x0415, 0x044a, 0x0481, 0x04b9, 0x04f3, 0x052f, 0x056c, 0x05ac, 0x05ec, 0x062f, 0x0673, 0x06ba,
	0x0702, 0x074b, 0x0797, 0x07e5, 0x0834, 0x0885, 0x08d8, 0x092d, 0x0984, 0x09dd, 0x0a38, 0x0a95, 0x0af4, 0x0b55, 0x0bb8, 0x0c1d,
	0x0c84, 0x0cee, 0x0d59, 0x0dc6, 0x0e36, 0x0ea8, 0x0f1b, 0x0f91, 0x100a, 0x1084, 0x1101, 0x1180, 0x1201, 0x1284, 0x130a, 0x1392,
	0x141c, 0x14a9, 0x1538, 0x15c9, 0x165d, 0x16f3, 0x178b, 0x1826, 0x18c3, 0x1963, 0x1a05, 0x1aaa, 0x1b51, 0x1bfa, 0x1ca6, 0x1d55,
	0x1e06, 0x1eba, 0x1f70, 0x2029, 0x20e5, 0x21a3, 0x2263, 0x2327, 0x23ed, 0x24b5, 0x2580, 0x264e, 0x271f, 0x27f2, 0x28c8, 0x29a1,
	0x2a7d, 0x2b5b, 0x2c3c, 0x2d20, 0x2e07, 0x2ef0, 0x2fdd, 0x30cc, 0x31be, 0x32b3, 0x33aa, 0x34a5, 0x35a3, 0x36a3, 0x37a7, 0x38ad,
	0x39b6, 0x3ac2, 0x3bd2, 0x3ce4, 0x3df9, 0x3f11, 0x402d, 0x414b, 0x426c, 0x4391, 0x44b8, 0x45e3, 0x4710, 0x4841, 0x4975, 0x4aac,
	0x4be6, 0x4d23, 0x4e64, 0x4fa8, 0x50ef, 0x5239, 0x5386, 0x54d6, 0x562a, 0x5781, 0x58db, 0x5a39, 0x5b9a, 0x5cfe, 0x5e65, 0x5fd0,
	0x613e, 0x62b0, 0x6425, 0x659d, 0x6718, 0x6897, 0x6a19, 0x6b9f, 0x6d28, 0x6eb5, 0x7045, 0x71d8, 0x736f, 0x750a, 0x76a8, 0x7849,
	0x79ee, 0x7b97, 0x7d43, 0x7ef2, 0x80a5, 0x825c, 0x8416, 0x85d4, 0x8795, 0x895a, 0x8b23, 0x8cef, 0x8ebf, 0x9093, 0x926a, 0x9445,
	0x9624, 0x9806, 0x99ec, 0x9bd6, 0x9dc3, 0x9fb4, 0xa1a9, 0xa3a2, 0xa59e, 0xa79f, 0xa9a3, 0xabab, 0xadb6, 0xafc6, 0xb1d9, 0xb3f0,
	0xb60b, 0xb82a, 0xba4d, 0xbc73, 0xbe9e, 0xc0cc, 0xc2ff, 0xc535, 0xc76f, 0xc9ad, 0xcbef, 0xce36, 0xd080, 0xd2ce, 0xd520, 0xd776,
	0xd9d0, 0xdc2e, 0xde90, 0xe0f6, 0xe360, 0xe5cf, 0xe841, 0xeab8, 0xed32, 0xefb1, 0xf233, 0xf4ba, 0xf745, 0xf9d5, 0xfc68, 0xff00,
};

static void init(void)
{
	rcc_clock_setup_in_hsi_out_48mhz();
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
 
	usb_serial_init();
 
	GPIO_ODR(GBIT_NOT_OUTPUT_ENABL) = BIT_NOT_OUTPUT_ENABLE;
 
	uint32_t pinmask = BIT_DATA_0 | BIT_DATA_1 | BIT_DATA_2 | BIT_DATA_3 | BIT_CLK | BIT_LATCH | BIT_NOT_OUTPUT_ENABLE;
 
	gpio_mode_setup(GPIO_OUT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, pinmask);
	gpio_set_output_options(GPIO_OUT, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH, pinmask);
 
	gpio_mode_setup(GPIO_IN, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, BIT_BUTTON);
 
	millis_timer_init();

	cur_frame = &frame_a;
	next_frame = &frame_b;
	iter = 0;

	memset(&frame_a, 0, sizeof(frame_a));
	memset(&frame_b, 0, sizeof(frame_b));

	enable_sys_tick(SYSTICK_PERIOD);
}

static const uint8_t channel_pins[] = CHANNEL_PINS;
static const uint8_t channel_sizes[] = CHANNEL_SIZES;

static int read_next_frame(void)
{
	int i, j, c;

	memset(next_frame, 0, sizeof(*next_frame));

	for (j=0; j<N_CHANNELS; j++)
	{
		int bit = channel[j];
		for (i=0; i<N_BITS_PER_CHANNEL; i++)
		{
			c = get_u16le();
			if (c > 0xff00)
				return finish_frame(c);

			uint8_t *p = &next_frame->bit[0][module_map[i]];
			for ( ; c ; c >>= 1 )
			{
				if (c&1)
					*p |= bit;

				p += sizeof(next_frame->bit[0])/sizeof(*p);
			}
		}
	}

	return finish_frame(0);
}


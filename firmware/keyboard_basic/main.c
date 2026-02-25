/*
 * Copyright (c) 2023 Erik Bosman <erik@minemu.org>
 *
 * Permission  is  hereby  granted,  free  of  charge,  to  any  person
 * obtaining  a copy  of  this  software  and  associated documentation
 * files (the "Software"),  to deal in the Software without restriction,
 * including  without  limitation  the  rights  to  use,  copy,  modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the
 * Software,  and to permit persons to whom the Software is furnished to
 * do so, subject to the following conditions:
 *
 * The  above  copyright  notice  and this  permission  notice  shall be
 * included  in  all  copies  or  substantial portions  of the Software.
 *
 * THE SOFTWARE  IS  PROVIDED  "AS IS", WITHOUT WARRANTY  OF ANY KIND,
 * EXPRESS OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY,  FITNESS  FOR  A  PARTICULAR  PURPOSE  AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT,  TORT OR OTHERWISE,  ARISING FROM, OUT OF OR IN
 * CONNECTION  WITH THE SOFTWARE  OR THE USE  OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * (http://opensource.org/licenses/mit-license.html)
 *
 */

#include <stdint.h>
#include <string.h>

#include <libopencmsis/core_cm3.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

#include "config.h"
#include "util.h"
#include "millis.h"
#include "usb_hid_keyboard.h"
#include "keyboard_leds.h"
#include "codegen_header.h"
#include "keymatrix.h"

#include "codegen_tables.c"

static uint8_t fb[N_LEDS];

#define HID_CODES(a,b) (b),
static uint32_t extra_keys[] = { FN_ALTERNATIVES(HID_CODES) };
#undef HID_CODES

static uint32_t alt_keymap[KEYMATRIX_N_KEYS] =
{
#define MAPPING(a,b) [a] = b,
	FN_ALTERNATIVES(MAPPING)
#undef MAPPING
//	[MATRIX_esc]          = KEYCODE_esc,
//	[MATRIX_fn]           = KEY_NONE,
	[MATRIX_leftshift]    = KEYCODE_leftshift,
	[MATRIX_rightshift]   = KEYCODE_rightshift,
	[MATRIX_leftcontrol]  = KEYCODE_leftcontrol,
	[MATRIX_alt]          = KEYCODE_alt,
	[MATRIX_altgr]        = KEYCODE_altgr,
	[MATRIX_win]          = KEYCODE_win,
	[MATRIX_menu]         = KEYCODE_menu,
	[MATRIX_rightcontrol] = KEYCODE_rightcontrol,
};

static void init(void)
{
	rcc_clock_setup_in_hsi_out_48mhz();
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
 
	usb_hid_keyboard_init(extra_keys, sizeof(extra_keys)/sizeof(extra_keys[0]));

	millis_timer_init();

	memset(fb, 0xff, sizeof(fb));
	init_leds();
		write_frame(fb);

	keymatrix_init();
}

#define FPS (25)
static uint16_t t0=0;
static uint8_t brightness = 0;
static void time_poll(void)
{
	uint16_t t = millis_u16();
	if ( (uint16_t)(t-t0) > 1000/FPS)
	{
		write_frame(fb);
		memset(fb, brightness++, sizeof(fb));
		t0 = t;
	}
}

static int fn_pressed = 0;
static int fn_lock = 0;
 
/* to be implemented by user */
void keymatrix_down(int key)
{
	if (key == MATRIX_fn)
	{
		usb_hid_keyboard_clear_keys(); /* does not clear modifier keys */
		fn_pressed = 1;
		return;
	}

	int use_alt = fn_pressed;
	if (fn_lock && alt_keymap[key])
		use_alt = !use_alt;

	uint32_t hid_key = (use_alt ? alt_keymap : matrix_to_keycode)[key];

	if (hid_key != KEY_NONE)
		usb_hid_keyboard_key_down( hid_key );
}

void keymatrix_up(int key)
{
	if (key == MATRIX_fn)
	{
		fn_pressed = 0;
		return;
	}

	uint32_t hid_key = matrix_to_keycode[key];

	if (hid_key != KEY_NONE)
		usb_hid_keyboard_key_up(hid_key);

	hid_key = alt_keymap[key];
	if (hid_key != KEY_NONE)
		usb_hid_keyboard_key_up(hid_key);
}

int main(void)
{
	init();

	for(;;)
	{
		time_poll();
		keymatrix_poll(millis());
		usb_hid_keyboard_poll();
	}

}


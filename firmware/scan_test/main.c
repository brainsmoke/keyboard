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
#include "usb_serial.h"
#include "keyboard_leds.h"
#include "keymatrix.h"

uint8_t fb[N_LEDS];

static void init(void)
{
	rcc_clock_setup_in_hsi_out_48mhz();
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
 
	usb_serial_init();

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

 
/* to be implemented by user */
void keymatrix_up(int key)
{
uint8_t buf[7];
buf[0] = 'U';
buf[1] = 'P';
buf[2] = ' ';
buf[3]="0123456789ABCDEF"[key>>4];
buf[4]="0123456789ABCDEF"[key&15];
buf[5]='\r';
buf[6]='\n';
usb_serial_write_noblock(buf, 7);
}

void keymatrix_down(int key)
{
uint8_t buf[9];
buf[0] = 'U';
buf[1] = 'U';
buf[2] = 'U';
buf[3] = 'P';
buf[4] = ' ';
buf[5]="0123456789ABCDEF"[key>>4];
buf[6]="0123456789ABCDEF"[key&15];
buf[7]='\r';
buf[8]='\n';
usb_serial_write_noblock(buf, 9);
}

int main(void)
{
	init();

	for(;;)
	{
		usb_serial_poll();
		time_poll();
		keymatrix_poll(millis());
	}
}


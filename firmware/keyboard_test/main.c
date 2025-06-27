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
}

#define FPS (25)
static uint16_t t0=0;
static uint8_t brightness = 0;
static void time_poll(void)
{
	uint16_t t = millis_u16();
	if ( (uint16_t)(t-t0) > 1000/FPS)
//if (usb_serial_getchar() > 0)	
	{
		write_frame(fb);
//uint8_t buf[4];
//buf[0]="0123456789ABCDEF"[fb[0]>>4];
//buf[1]="0123456789ABCDEF"[fb[0]&15];
//buf[2]='\r';
//buf[3]='\n';
//usb_serial_write_noblock(buf, 4);

		memset(fb, brightness++, sizeof(fb));
		t0 = t;
	}
}

int main(void)
{
	init();

	for(;;)
	{
		usb_serial_poll();
		time_poll();
	}
}


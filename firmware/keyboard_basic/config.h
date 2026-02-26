#ifndef CONFIG_H
#define CONFIG_H

#include <libopencm3/stm32/gpio.h>
#include <libopencmsis/core_cm3.h>

#define MANUFACTURER_STRING "example co."
#define PRODUCT_STRING "usb hid keyboard demo"
#define SERIAL_STRING "00000001"

#define F_CPU (48000000)
#define F_SYS_TICK_CLK (F_CPU/8)

#define KEYMATRIX_ROWS_PORT (GPIOA)
#define KEYMATRIX_ROWS(X) X(7) X(2) X(3) X(4) X(5) X(6)
 
#define KEYMATRIX_COLUMNS_PORT (GPIOB)
#define KEYMATRIX_COLUMNS(X) X(15) X(14) X(13) X(12) X(11) X(10) X(2) X(1) X(0) X(6) X(9) X(8) X(7) X(5) X(4) X(3)

#define REPORT_MAX_KEY (0x52)

/* uncomment for thinkpad layout */
//#define SWAP_CONTROL_AND_FN (1)

#include "hid_keydef.h"
 
#define FN_ALTERNATIVES(X) \
	X(MATRIX_f1,  KEY_MUTE) \
	X(MATRIX_f2,  KEY_VOLUME_DOWN) \
	X(MATRIX_f3,  KEY_VOLUME_UP) \
/*	X(MATRIX_f4,  KEY_MUTE) \
	X(MATRIX_f5,  KEY_MUTE) \
	X(MATRIX_f6,  KEY_MUTE) \
	X(MATRIX_f7,  KEY_MUTE) \
	X(MATRIX_f8,  KEY_MUTE) \
	X(MATRIX_f9,  KEY_MUTE) \
	X(MATRIX_f10, KEY_MUTE) \
	X(MATRIX_f11, KEY_MUTE) \
	X(MATRIX_f12, KEY_MUTE)
*/

#endif // CONFIG_H

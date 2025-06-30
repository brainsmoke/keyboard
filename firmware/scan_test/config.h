#ifndef CONFIG_H
#define CONFIG_H

#include <libopencm3/stm32/gpio.h>
#include <libopencmsis/core_cm3.h>

#define MANUFACTURER_STRING "example co."
#define PRODUCT_STRING "usb serial demo"
#define SERIAL_STRING "00000001"

#define F_CPU (48000000)
#define F_SYS_TICK_CLK (F_CPU/8)

#define KEYMATRIX_ROWS_PORT (GPIOA)
#define KEYMATRIX_ROWS(X) X(7) X(2) X(3) X(4) X(5) X(6)
 
#define KEYMATRIX_COLUMNS_PORT (GPIOB)
#define KEYMATRIX_COLUMNS(X) X(15) X(14) X(13) X(12) X(11) X(10) X(2) X(1) X(0) X(6) X(9) X(8) X(7) X(5) X(4) X(3)

#endif // CONFIG_H

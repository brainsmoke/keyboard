#ifndef BITBANG_H
#define BITBANG_H

/* platform constants */

#define GPIO_ODR_OFFSET (0x14)
#define GPIO_BSRR_OFFSET (0x18)
#define GPIO_BRR_OFFSET (0x28)

/* GPIO, BSRR */

#define CLEAR(X) (X<<16)
#define SET(X)   (X)

/* GPIO C */
#define PIN_DATA_LEFT          (15)
#define PIN_DATA_RIGHT         (13)
#define PIN_CLK                (14)

/* GPIO A */
#define PIN_LATCH              (0)
#define PIN_NOT_OUTPUT_ENABLE  (1)


#define BIT_DATA_LEFT          (1<<PIN_DATA_LEFT)
#define BIT_DATA_RIGHT         (1<<PIN_DATA_RIGHT)
#define BIT_CLK                (1<<PIN_CLK)
#define BIT_LATCH              (1<<PIN_LATCH)
#define BIT_NOT_OUTPUT_ENABLE  (1<<PIN_NOT_OUTPUT_ENABLE)

#define MASK_CLK (BIT_CLK)
#define MASK_DATA (BIT_DATA_LEFT|BIT_DATA_RIGHT)

#ifndef __ASSEMBLER__

#include <stdint.h>
#include "config.h"

#define GPIO_LEDS_DATA (GPIOC)
#define GPIO_LEDS_LATCH (GPIOA)

#define BUFFER_BIT_DATA_LEFT (BIT_DATA_LEFT>>8)
#define BUFFER_BIT_DATA_RIGHT (BIT_DATA_RIGHT>>8)


#define MAX_BITS_PER_CHANNEL (56)
#define CHANNEL_PINS { BIT_DATA_LEFT, BIT_DATA_RIGHT, 0 }
#define CHANNEL_SIZES { 56, 32, 0 }

/* sends 56x bits parallel at 1/8th the clockspeed
 * gpio is the GPIO base address
 * 
 * probably close to 500 cycles including call
 */
void bitbang_56bits_stm32(uint8_t *buffer, volatile uint32_t *gpio_data_clock, volatile uint32_t *gpio_latch_enable, uint32_t not_enable_mask);
void write_wait_write(volatile uint32_t *addr, uint32_t pre_data, uint32_t post_data, uint32_t cycles);

#endif

#endif // BITBANG_H

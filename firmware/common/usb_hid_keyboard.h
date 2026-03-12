#ifndef USB_HID_KEYBOARD_H
#define USB_HID_KEYBOARD_H

/*
 * Copyright (c) 2023-2026 Erik Bosman <erik@minemu.org>
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
#include <stddef.h>

#define HID_KEYBOARD_MAX_EXTRA_KEYS (48)

int usb_hid_keyboard_init(const uint32_t extra_keys[], size_t n_extra_keys);
void usb_hid_keyboard_key_up(uint32_t hid_key);
void usb_hid_keyboard_key_down(uint32_t hid_key);
int  usb_hid_keyboard_key_is_down(uint32_t hid_key);
void usb_hid_keyboard_clear_keys(void);
void usb_hid_keyboard_clear_modifiers(void);
void usb_hid_keyboard_poll(void);

#define HID_REPORT_LED_NUM_LOCK    (1<<0)
#define HID_REPORT_LED_CAPS_LOCK   (1<<1)
#define HID_REPORT_LED_SCROLL_LOCK (1<<2)
#define HID_REPORT_LED_COMPOSE     (1<<3)
#define HID_REPORT_LED_KANA        (1<<4)
/* to be implemented by user */
void usb_hid_keyboard_led_state(uint8_t bitmask);

#endif // USB_HID_KEYBOARD_H

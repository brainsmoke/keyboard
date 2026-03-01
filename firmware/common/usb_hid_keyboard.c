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

#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>

#include <string.h>

#include "config.h"
#include "usb_hid_keyboard.h"
#include "hid_keydef.h"
#include "millis.h"

#ifndef ID_VENDOR
#define ID_VENDOR  (0x4242)
#endif
#ifndef ID_PRODUCT
#define ID_PRODUCT (0x4242)
#endif
#ifndef ID_VERSION
#define ID_VERSION (0x0000)
#endif
#ifndef MANUFACTURER_STRING
#define MANUFACTURER_STRING "manufacturer"
#endif
#ifndef PRODUCT_STRING
#define PRODUCT_STRING "keyboard"
#endif
#ifndef SERIAL_STRING
#define SERIAL_STRING "00000001"
#endif

static usbd_device *device;

#define REPORT_MOD_BITFIELD_SIZE (1) /* dictated by boot protocol */
#define REPORT_RESERVED_SIZE     (1) /* dictated by boot protocol */
#define REPORT_ARRAY_SIZE        (6) /* dictated by boot protocol */
#define REPORT_ARRAY_OFFSET (REPORT_MOD_BITFIELD_SIZE+REPORT_RESERVED_SIZE)
#define REPORT_KEYS_BITFIELD_OFFSET (REPORT_ARRAY_OFFSET+REPORT_ARRAY_SIZE)

#define REPORT_MIN_KEY           (4)
#ifndef REPORT_MAX_KEY
#define REPORT_MAX_KEY         (101)
#endif
#define REPORT_KEYS_BITFIELD_NUMBITS (REPORT_MAX_KEY-REPORT_MIN_KEY+1)
#define REPORT_KEYS_BITFIELD_PADDING (7-((REPORT_KEYS_BITFIELD_NUMBITS-1)&7))

_Static_assert(REPORT_MAX_KEY < 256);
_Static_assert(REPORT_KEYS_BITFIELD_NUMBITS < 256);

#define REPORT_KEYS_BITFIELD_SIZE ((7+REPORT_KEYS_BITFIELD_NUMBITS)>>3)
#define REPORT_EXTRA_KEYS_BITFIELD_MAX_SIZE ((7+HID_KEYBOARD_MAX_EXTRA_KEYS)>>3)

#define REPORT_MOD_BITFIELD_BIT_OFFSET        (0)
#define REPORT_KEYS_BITFIELD_BIT_OFFSET       (REPORT_KEYS_BITFIELD_OFFSET*8)
#define REPORT_EXTRA_KEYS_BITFIELD_BIT_OFFSET (REPORT_KEYS_BITFIELD_BIT_OFFSET+REPORT_KEYS_BITFIELD_SIZE*8)

#define REPORT_MAX_SIZE (REPORT_MOD_BITFIELD_SIZE+REPORT_RESERVED_SIZE+REPORT_ARRAY_SIZE+REPORT_KEYS_BITFIELD_SIZE+REPORT_EXTRA_KEYS_BITFIELD_MAX_SIZE)
#define REPORT_SIZE(n_extra_keys) ((REPORT_EXTRA_KEYS_BITFIELD_BIT_OFFSET+(n_extra_keys)+7)>>3)

static uint8_t report_descriptor[384];
static uint8_t report[REPORT_MAX_SIZE];
static uint8_t control[128];

#define OUT_REPORT_LED_STATE_OFFSET (0)
#define OUT_REPORT_LED_STATE_SIZE (1)
static uint8_t out_report[OUT_REPORT_LED_STATE_SIZE];
static uint8_t out_report_changed;

static uint8_t current_protocol;
static uint8_t idle_rate_4ms;
static volatile uint16_t last_report_ms;

static uint32_t extra_keymap[HID_KEYBOARD_MAX_EXTRA_KEYS];

static uint32_t extra_keys_bits=0;
static volatile uint32_t need_update, usb_ready;

#define VERSION_USB_2_0 (0x0200)

#define DEVICE_CLASS_LOOK_AT_INTERFACE (0)
#define NO_SUBCLASS (0)
#define NO_PROTOCOL (0)
#define PACKET_SIZE_FULL_SPEED (64)

#define BUS_POWERED   (1<<7)
#define SELF_POWERED  (1<<6)
#define REMOTE_WAKEUP (1<<5)

#define MILLIAMPS(x)    ((x+1)>>1)
#define MILLISECONDS(x)  (x)

#define COUNTRY_NONE (0)

enum
{
	NO_STRING = 0,
	MANUFACTURER,
	PRODUCT,
	SERIAL,
};

static const char *const string_descriptors[] =
{
	[MANUFACTURER-1] = MANUFACTURER_STRING,
	[PRODUCT-1]      = PRODUCT_STRING,
	[SERIAL-1]       = SERIAL_STRING,
};
#define N_STRING_DESCRIPTORS (sizeof(string_descriptors)/sizeof(string_descriptors[0]))

/* non-static descriptors */

/* .wMaxPacketSize needs to be set */
static struct usb_endpoint_descriptor endpoint_desc =
{
	.bLength           = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType   = USB_DT_ENDPOINT,
	.bEndpointAddress  = USB_ENDPOINT_ADDR_IN(1),
	.bmAttributes      = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize    = 0x4242,
	.bInterval         = MILLISECONDS(1),
};

typedef struct __attribute__((packed))
{
	struct usb_hid_descriptor d;
	uint8_t bDescriptorType;
	uint16_t wDescriptorLength;

} single_report_hid_descriptor_t;

/* .wDescriptorLength to be set */
static single_report_hid_descriptor_t usb_hid_desciptor =
{
	.d.bLength = sizeof(single_report_hid_descriptor_t),
	.d.bDescriptorType = USB_HID_DT_HID,
	.d.bcdHID = 0x111, /* 1.11 */
	.d.bCountryCode = COUNTRY_NONE,
	.d.bNumDescriptors = 1,
	.bDescriptorType = USB_HID_DT_REPORT,
	.wDescriptorLength = 0x4242,
};

//static const uint8_t hid_boot_keyboard_descriptor[] =
//{
//	0x05,0x01,      /* Usage Page (Generic Desktop) */
//	0x09,0x06,      /* Usage (Keyboard) */
//	0xa1,0x01,      /* Collection (Application) */
//	0x75,0x01,      /* Report Size (1) */
//	0x95,0x08,      /* Report Count (8) */
//	0x05,0x07,      /* Usage Page (Keyboard) */
//	0x19,0xe0,      /* Usage Minimum (224) */
//	0x29,0xe7,      /* Usage Maximum (231) */
//	0x15,0x00,      /* Logical Minimum (0) */
//	0x25,0x01,      /* Logical Maximum (1) */
//	0x81,0x02,      /* Input (Data, Variable, Absolute) */
//	0x95,0x01,      /* Report Count (1) */
//	0x75,0x08,      /* Report Size (8) */
//	0x81,0x01,      /* Input (Constant) */
//	0x95,0x05,      /* Report Count (5) */
//	0x75,0x01,      /* Report Size (1) */
//	0x05,0x08,      /* Usage Page (LED) */
//	0x19,0x01,      /* Usage Minimum (1) */
//	0x29,0x05,      /* Usage Maximum (5) */
//	0x91,0x02,      /* Output (Data, Variable, Absolute) */
//	0x95,0x01,      /* Report Count (1) */
//	0x75,0x03,      /* Report Size (3) */
//	0x91,0x01,      /* Output (Constant) */
//	0x95,0x06,      /* Report Count (6) */
//	0x75,0x08,      /* Report Size (8) */
//	0x15,0x00,      /* Logical Minimum (0) */
//	0x25,0xff,      /* Logical Maximum(255) */
//	0x05,0x07,      /* Usage Page (Keyboard) */
//	0x19,0x00,      /* Usage Minimum (0) */
//	0x29,0xff,      /* Usage Maximum (255) */
//	0x81,0x00,      /* Input (Data, Array) */
//	0xc0,           /* End Collection */
//};

static size_t create_hid_keyboard_descriptor(uint8_t buf[], size_t len, const uint32_t keys[], size_t n_keys)
{
	const uint8_t hid_keyboard_prologue[] =
	{
		0x05,0x01,      /* Usage Page (Generic Desktop) */
		0x09,0x06,      /* Usage (Keyboard) */
		0xa1,0x01,      /* Collection (Application) */
		0x95,0x08,      /* Report Count (8) */
		0x75,0x01,      /* Report Size (1) */
		0x05,0x07,      /* Usage Page (Keyboard) */
		0x19,0xe0,      /* Usage Minimum (224) */
		0x29,0xe7,      /* Usage Maximum (231) */
		0x15,0x00,      /* Logical Minimum (0) */
		0x25,0x01,      /* Logical Maximum (1) */
		0x81,0x02,      /* Input (Data, Variable, Absolute) */
		0x95,0x01,      /* Report Count (1) */
		0x75,0x08,      /* Report Size (8) */
		0x81,0x01,      /* Input (Constant) */
		0x95,0x05,      /* Report Count (5) */
		0x75,0x01,      /* Report Size (1) */
		0x05,0x08,      /* Usage Page (LED) */
		0x19,0x01,      /* Usage Minimum (1) */
		0x29,0x05,      /* Usage Maximum (5) */
		0x91,0x02,      /* Output (Data, Variable, Absolute) */
		0x95,0x01,      /* Report Count (1) */
		0x75,0x03,      /* Report Size (3) */
		0x91,0x01,      /* Output (Constant) */
		0x95,0x06,      /* Report Count (6) */
		0x75,0x08,      /* Report Size (8) */
		0x81,0x01,      /* Input (Constant) */
		0x05,0x07,                         /* Usage Page (Keyboard) */
		0x95,REPORT_KEYS_BITFIELD_NUMBITS, /* Report Count (98) */
		0x75,0x01,                         /* Report Size (1) */
		0x19,REPORT_MIN_KEY,               /* Usage Minimum (4) */
		0x29,REPORT_MAX_KEY,               /* Usage Maximum (101) */
		0x81,0x02,                         /* Input (Data, Variable, Absolute) */
#if REPORT_KEYS_BITFIELD_PADDING > 0
		0x95,0x01,                         /* Report Count (1) */
		0x75,REPORT_KEYS_BITFIELD_PADDING, /* Report Size (6) */
		0x81,0x01,                         /* Input (Constant) */
#endif
	};

	if (n_keys >= 256)
		return 0;

	size_t descriptor_size = sizeof(hid_keyboard_prologue) + n_keys*5 + 7;
	if (n_keys & 0x7)
		descriptor_size += 6;

	if (len < descriptor_size) return 0;

	memcpy(&buf[0], hid_keyboard_prologue, sizeof(hid_keyboard_prologue));

	size_t off = sizeof(hid_keyboard_prologue);
	size_t i;

	for (i=0; i<n_keys; i++)
	{
		buf[off++] = 0x0b;                 // (Long) Usage
		buf[off++] = (keys[i]>>0)  & 0xff; // key
		buf[off++] = (keys[i]>>8)  & 0xff;
		buf[off++] = (keys[i]>>16) & 0xff; // page
		buf[off++] = (keys[i]>>24) & 0xff;
	}

	buf[off++] = 0x95;   //  Report Count (n_keys) */
	buf[off++] = n_keys;

	buf[off++] = 0x75;   //  Report Size (1)
	buf[off++] = 0x01;

	buf[off++] = 0x81;   //  Input (Data, Variable, Absolute)
	buf[off++] = 0x02;

	if (n_keys & 0x7)
	{
		buf[off++] = 0x95;   //  Report Count (1) */
		buf[off++] = 1;

		buf[off++] = 0x75;   //  Report Size (8- (n_keys&0x7) )
		buf[off++] = (0x8-(n_keys&0x7) );

		buf[off++] = 0x81;   //  Input (Constant)
		buf[off++] = 0x01;
	}

	buf[off++] = 0xC0;   //  End Collection

	return off;
}


static const struct usb_device_descriptor device_desc =
{
	.bLength            = USB_DT_DEVICE_SIZE,
	.bDescriptorType    = USB_DT_DEVICE,
	.bcdUSB             = VERSION_USB_2_0,
	.bDeviceClass       = DEVICE_CLASS_LOOK_AT_INTERFACE,
	.bDeviceSubClass    = NO_SUBCLASS,
	.bDeviceProtocol    = NO_PROTOCOL,
	.bMaxPacketSize0    = PACKET_SIZE_FULL_SPEED,
	.bNumConfigurations = 1,

	.idVendor           = ID_VENDOR,
	.idProduct          = ID_PRODUCT,

	.bcdDevice          = ID_VERSION,

	.iManufacturer      = MANUFACTURER,
	.iProduct           = PRODUCT,
	.iSerialNumber      = SERIAL,
};

static const struct usb_interface_descriptor interface_desc =
{
	.bLength            = USB_DT_INTERFACE_SIZE,
	.bDescriptorType    = USB_DT_INTERFACE,
	.bInterfaceNumber   = 0,
	.bAlternateSetting  = 0,
	.bNumEndpoints      = 1,
	.bInterfaceClass    = USB_CLASS_HID,
	.bInterfaceSubClass = USB_HID_SUBCLASS_BOOT_INTERFACE,
	.bInterfaceProtocol = USB_HID_INTERFACE_PROTOCOL_KEYBOARD,
	.iInterface         = NO_STRING,

	.endpoint           = &endpoint_desc,
	.extra              = &usb_hid_desciptor,
	.extralen           = sizeof(usb_hid_desciptor),
};

const struct usb_interface interfaces[] =
{
	{
		.num_altsetting = 1,
		.altsetting = &interface_desc,
	},
};

static const struct usb_config_descriptor config_desc =
{
	.bLength             = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType     = USB_DT_CONFIGURATION,
	.wTotalLength        = 0, /* calculated by USB stack */
	.bNumInterfaces      = 1,
	.bConfigurationValue = 1,
	.iConfiguration      = NO_STRING,
	.bmAttributes        = BUS_POWERED,
	.bMaxPower           = MILLIAMPS(250),

	.interface = interfaces,
};


static enum usbd_request_return_codes hid_control_callback(usbd_device *dev,
                                                           struct usb_setup_data *req,
                                                           uint8_t **buf, uint16_t *len,
                                                           usbd_control_complete_callback *complete)
{
	(void)dev;
	(void)complete;

	if( req->bmRequestType == (USB_REQ_TYPE_IN|USB_REQ_TYPE_STANDARD|USB_REQ_TYPE_INTERFACE) &&
	    req->bRequest == USB_REQ_GET_DESCRIPTOR &&
	    req->wValue == (USB_HID_DT_REPORT<<8) )
	{
		*buf = report_descriptor;
		*len = usb_hid_desciptor.wDescriptorLength;

		usb_ready=1;

		return USBD_REQ_HANDLED;
	}

	if ( req->bmRequestType == (USB_REQ_TYPE_IN|USB_REQ_TYPE_CLASS|USB_REQ_TYPE_INTERFACE) )
	switch (req->bRequest)
	{
		case USB_HID_REQ_TYPE_GET_REPORT:
		{
			if ( req->wValue == (USB_HID_REPORT_TYPE_INPUT<<8) )
			{
				*buf = report;
				*len = endpoint_desc.wMaxPacketSize;
				return USBD_REQ_HANDLED;
			}
			else if ( req->wValue == (USB_HID_REPORT_TYPE_OUTPUT<<8) )
			{
				*buf = out_report;
				*len = sizeof(out_report);
				return USBD_REQ_HANDLED;
			}
			break;
		}
		case USB_HID_REQ_TYPE_GET_IDLE:
		{
			if (req->wValue == 0)
			{
				*buf = &idle_rate_4ms;
				*len = 1;
				return USBD_REQ_HANDLED;
			}
			break;
		}
		case USB_HID_REQ_TYPE_GET_PROTOCOL:
		{
			if (req->wValue == 0)
			{
				*buf = &current_protocol;
				*len = 1;
				return USBD_REQ_HANDLED;
			}
			break;
		}
		default:
	}

	if ( req->bmRequestType == (USB_REQ_TYPE_OUT|USB_REQ_TYPE_CLASS|USB_REQ_TYPE_INTERFACE) )
	switch (req->bRequest)
	{
		case USB_HID_REQ_TYPE_SET_REPORT:
		{
			if ( (req->wValue == (USB_HID_REPORT_TYPE_OUTPUT<<8)) &&
			     (req->wLength == sizeof(out_report)) )
			{
				memcpy(out_report, *buf, sizeof(out_report));
				out_report_changed = 1;
				return USBD_REQ_HANDLED;
			}
			break;
		}
		case USB_HID_REQ_TYPE_SET_IDLE:
		{
			if ( (req->wValue & 0xff) == 0)
			{
				if (idle_rate_4ms == 0)
					last_report_ms = millis_u16();

				idle_rate_4ms = (req->wValue >> 8);
				return USBD_REQ_HANDLED;
			}
			break;
		}
		case USB_HID_REQ_TYPE_SET_PROTOCOL:
		{
			uint16_t prot = req->wValue;
			if ( prot == USB_HID_PROTOCOL_BOOT || prot == USB_HID_PROTOCOL_REPORT )
			{
				current_protocol = req->wValue;
				return USBD_REQ_HANDLED;
			}
			break;
		}
		default:
	}

	return USBD_REQ_NEXT_CALLBACK;
}

static void hid_set_config(usbd_device *dev, uint16_t wValue)
{
	(void)wValue;

	usbd_ep_setup(dev, USB_ENDPOINT_ADDR_IN(1), USB_ENDPOINT_ATTR_INTERRUPT, endpoint_desc.wMaxPacketSize, NULL);

	usbd_register_control_callback(dev,
	                               USB_REQ_TYPE_INTERFACE,
	                               USB_REQ_TYPE_RECIPIENT,
	                               hid_control_callback);
}

#define ARRAY_NO_KEY         (0)
#define ARRAY_ERROR_ROLLOVER (1)
static void report_array_add_key(uint32_t hid_key)
{
	size_t i;
	for (i=0; i<REPORT_ARRAY_SIZE; i++)
		if ( report[REPORT_ARRAY_OFFSET+i] == ARRAY_NO_KEY )
		{
			report[REPORT_ARRAY_OFFSET+i] = hid_key;
			return;
		}

	memset(&report[REPORT_ARRAY_OFFSET], ARRAY_ERROR_ROLLOVER, REPORT_ARRAY_SIZE);
}

static void report_array_del_key(uint32_t hid_key)
{
	size_t array_ix = 0, byte_ix, bit;

	if (report[REPORT_ARRAY_OFFSET] != ARRAY_ERROR_ROLLOVER)
	{
		for (; array_ix<REPORT_ARRAY_SIZE; array_ix++)
			if (report[REPORT_ARRAY_OFFSET+array_ix] == hid_key)
				break;

		for (; array_ix<REPORT_ARRAY_SIZE-1; array_ix++)
			report[REPORT_ARRAY_OFFSET+array_ix] = report[REPORT_ARRAY_OFFSET+array_ix+1];
	}
	else /* slow fallback */
	{
		for (byte_ix = 0; byte_ix < REPORT_KEYS_BITFIELD_SIZE; byte_ix++)
		{
			uint32_t v = report[REPORT_KEYS_BITFIELD_OFFSET+byte_ix];
			if (v)
				for (bit=0; bit<8 ; bit++)
					if (v & (1<<bit))
					{
						if (array_ix < REPORT_ARRAY_SIZE)
							report[REPORT_ARRAY_OFFSET + array_ix++] = REPORT_MIN_KEY+byte_ix*8+bit;
						else
						{
							memset(&report[REPORT_ARRAY_OFFSET], ARRAY_ERROR_ROLLOVER, REPORT_ARRAY_SIZE);
							return;
						}
					}
		}
	}

	for (; array_ix<REPORT_ARRAY_SIZE; array_ix++)
		report[REPORT_ARRAY_OFFSET+array_ix] = ARRAY_NO_KEY;
}

static int get_index(uint32_t hid_key)
{
	if ( (hid_key >= HID_KEY(HID_KEYBOARD_PAGE, REPORT_MIN_KEY)) &&
	     (hid_key <= HID_KEY(HID_KEYBOARD_PAGE, REPORT_MAX_KEY)) )
		return REPORT_KEYS_BITFIELD_BIT_OFFSET + hid_key - HID_KEY(HID_KEYBOARD_PAGE, 4);

	if ( (hid_key >= KEY_LEFT_CONTROL) && (hid_key <= KEY_RIGHT_WINDOWS) )
		return REPORT_MOD_BITFIELD_BIT_OFFSET + hid_key - KEY_LEFT_CONTROL;

	size_t i;
	for (i=0; i<extra_keys_bits; i++)
		if (extra_keymap[i] == hid_key)
			return REPORT_EXTRA_KEYS_BITFIELD_BIT_OFFSET + i;

	return -1;
}

void usb_hid_keyboard_key_up(uint32_t hid_key)
{
	int ix = get_index(hid_key);
	if (ix < 0)
		return;

	else if ( report[ix>>3] & (1<<(ix&0x7)) )
	{
		report[ix>>3] &=~ (1<<(ix&0x7));

		if ( (hid_key >= HID_KEY(HID_KEYBOARD_PAGE, REPORT_MIN_KEY)) &&
		     (hid_key <= HID_KEY(HID_KEYBOARD_PAGE, REPORT_MAX_KEY)) )
			report_array_del_key(hid_key & 0xff);

		need_update = 1;
	}
}

void usb_hid_keyboard_key_down(uint32_t hid_key)
{
	int ix = get_index(hid_key);
	if (ix < 0)
		return;

	report[ix>>3] |= 1<<(ix&0x7);

	if ( (hid_key >= HID_KEY(HID_KEYBOARD_PAGE, REPORT_MIN_KEY)) &&
	     (hid_key <= HID_KEY(HID_KEYBOARD_PAGE, REPORT_MAX_KEY)) )
		report_array_add_key(hid_key & 0xff);

	need_update = 1;
}

void usb_hid_keyboard_clear_keys(void)
{
	if (report[REPORT_ARRAY_OFFSET] != 0)
		need_update = 1;

	memset(&report[1], 0, sizeof(report)-sizeof(report[0]));
}

void usb_hid_keyboard_clear_modifiers(void)
{
	report[0] = 0;
	need_update = 1;
}

void usb_hid_keyboard_poll(void)
{
	uint16_t now = 0;

	if ( idle_rate_4ms > 0 )
	{
		now = millis_u16();
		if ( (uint16_t)(now-last_report_ms) >= (idle_rate_4ms<<2) )
			need_update = 1;
	}

	if (usb_ready && need_update)
	{
		need_update = 0;

		uint32_t report_size = endpoint_desc.wMaxPacketSize;
		if (current_protocol == USB_HID_PROTOCOL_BOOT)
			report_size = 8;

		if ( usbd_ep_write_packet(device, endpoint_desc.bEndpointAddress, report,
		                          report_size) != report_size )
			need_update = 1;
		else
			last_report_ms = now;
	}

	usbd_poll(device);

	if (out_report_changed)
	{
		out_report_changed = 0;
		usb_hid_keyboard_led_state(out_report[OUT_REPORT_LED_STATE_OFFSET]);
	}
}

int usb_hid_keyboard_init(const uint32_t extra_keys[], size_t n_extra_keys)
{
	memset(report, 0, sizeof(report));
	memset(out_report, 0, sizeof(out_report));
	out_report_changed = 0;
	idle_rate_4ms = 0;
	current_protocol = USB_HID_PROTOCOL_REPORT;
	usb_ready = 0;
	need_update = 1;

	if (n_extra_keys > HID_KEYBOARD_MAX_EXTRA_KEYS)
		return 0;

	size_t i, j=0;
	for (i=0; i<n_extra_keys; i++)
		if (extra_keys[i] != KEY_NONE)
			extra_keymap[j++] = extra_keys[i];
	n_extra_keys = j;

	size_t len = create_hid_keyboard_descriptor(report_descriptor, sizeof(report_descriptor),
		                                        extra_keymap, n_extra_keys);

	if (len == 0)
		return 0;

	usb_hid_desciptor.wDescriptorLength = len;

	endpoint_desc.wMaxPacketSize = REPORT_SIZE(n_extra_keys);
	extra_keys_bits = n_extra_keys;

	device = usbd_init(&st_usbfs_v2_usb_driver, &device_desc, &config_desc,
	                   string_descriptors, N_STRING_DESCRIPTORS,
	                   control, sizeof(control));

	usbd_register_set_config_callback(device, hid_set_config);

	millis_timer_init();

	return 1;
}


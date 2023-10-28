// SPDX-License-Identifier: GPL-2.0

/*
 * Enable all buttons on Winwing Orion II throttle base
 *
 * HID report descriptor shows 111 buttons, which exceeds maximum
 * number of buttons (80) supported by Linux kernel HID subsystem.
 *
 * This module skips unused button numbers 32-63.
 */

#include <linux/device.h>
#include <linux/hid.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int winwing_probe(struct hid_device *hdev,
		const struct hid_device_id *id)
{
	int ret;

	ret = hid_parse(hdev);
	if (ret) {
		hid_err(hdev, "parse failed\n");
		return ret;
	}

	ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
	if (ret) {
		hid_err(hdev, "hw start failed\n");
		return ret;
	}

	return 0;
}

static __u8 original_rdesc_buttons[] = {
	0x05, 0x09, 0x19, 0x01, 0x29, 0x6F,
	0x15, 0x00, 0x25, 0x01, 0x35, 0x00,
	0x45, 0x01, 0x75, 0x01,	0x95, 0x6F,
	0x81, 0x02, 0x75, 0x01, 0x95, 0x01,
	0x81, 0x01
};

static __u8 *winwing_report_fixup(struct hid_device *hdev, __u8 *rdesc,
		unsigned int *rsize)
{
	int sig_length = sizeof(original_rdesc_buttons);
	int unused_button_numbers = 32;

	if (*rsize < 34) {
		return rdesc;
	}

	if (memcmp(rdesc + 8, original_rdesc_buttons, sig_length) == 0) {

		/* Usage Maximum */
		rdesc[13] -= unused_button_numbers;

		/*  Report Count for buttons */
		rdesc[25] -= unused_button_numbers;

		/*  Report Count for padding [HID1_11, 6.2.2.9] */
		rdesc[31] += unused_button_numbers;

		hid_info(hdev, "winwing descriptor fixed\n");
	}

	return rdesc;
}

static int winwing_raw_event(struct hid_device *hdev,
		struct hid_report *report, u8 *raw_data, int size)
{
        if (size >= 15) {
		/* Move throttle base buttons 64 .. 111 to 32 .. 79 */
		memmove(raw_data + 5, raw_data + 9, 6);

		/* Clear the padding */
		memset(raw_data + 11, 0, 4);
        }

	return 0;
}

static const struct hid_device_id winwing_devices[] = {
	/* Orion2 base with F/A-18 Hornet grip */
	{ HID_USB_DEVICE(0x4098, 0xbe62) },
	{}
};

MODULE_DEVICE_TABLE(hid, winwing_devices);

static struct hid_driver winwing_driver = {
	.name = "winwing",
	.id_table = winwing_devices,
	.probe = winwing_probe,
	.report_fixup = winwing_report_fixup,
	.raw_event = winwing_raw_event,
};
module_hid_driver(winwing_driver);

MODULE_AUTHOR("Ivan Gorinov <linux-kernel@altimeter.info>");
MODULE_LICENSE("GPL");

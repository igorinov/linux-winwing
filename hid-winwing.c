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

	hid_info(hdev, "probe winwing\n");

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

static __u8 *winwing_report_fixup(struct hid_device *hdev, __u8 *rdesc,
		unsigned int *rsize)
{
	int unused_button_numbers = 32;

	hid_info(hdev, "descriptor fixup\n");

	if (*rsize >= 34) {
		/* Usage Maximum */
		if (rdesc[12] == 0x29 && rdesc[13] == 111) {
			rdesc[13] -= unused_button_numbers;
		}
		/*  Report Count (button data) */
		if (rdesc[24] == 0x95 && rdesc[25] == 111) {
			rdesc[25] -= unused_button_numbers;
		}
		/*  Report Count (unused bits) */
		if (rdesc[30] == 0x95 && rdesc[31] == 1) {
			rdesc[31] += unused_button_numbers;
		}
	}

	return rdesc;
}

static int winwing_raw_event(struct hid_device *hdev,
		struct hid_report *report, u8 *raw_data, int size)
{
	__u8 tmp[4];

        if (size >= 15) {
		/* Save unused bits for buttons 32 .. 63 */
		memcpy(tmp, raw_data + 5, 4);

		/* Move throttle base buttons to 32 .. 79 */
		memmove(raw_data + 5, raw_data + 9, 6);

		/* Copy unused bits to reserved area */
		memcpy(raw_data + 11, tmp, 4);
        }

	return 0;
}

static const struct hid_device_id winwing_devices[] = {
	{ HID_USB_DEVICE(0x4098, 0xbe62),
		.driver_data = 0 },
	{ }
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

MODULE_LICENSE("GPL");

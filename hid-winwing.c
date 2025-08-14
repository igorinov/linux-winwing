// SPDX-License-Identifier: GPL-2.0

/*
 * HID driver for WinWing Orion 2 throttle
 *
 * Copyright (c) 2023 Ivan Gorinov
 */

#include <linux/device.h>
#include <linux/hid.h>
#include <linux/hidraw.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>

#define MAX_REPORT 16

#define WW_F15E 0xf15e

struct winwing_led {
	struct led_classdev cdev;
	struct hid_device *hdev;
	int number;
};

struct winwing_led_info {
	int number;
	int max_brightness;
	const char *led_name;
};

static const struct winwing_led_info led_info[3] = {
	{ 0, 255, "backlight" },
	{ 1, 1, "a-a" },
	{ 2, 1, "a-g" },
};

/* Button numbers are 1-based to make it easier to find on the diagram. */
static const __u8 remap_f15e[] = {
	50, 12, 51, 13, 52, 14, 53, 15, 54, 16,
	55, 17,	56, 18, 57, 19, 58, 20, 59, 21,
	28, 25, 29, 26, 32, 27, 33, 28, 34, 29,
	35, 32,
	255, 0 };

struct winwing_drv_data {
	struct hid_device *hdev;
	const __u8 *remap;
	__u8 *report_buf;
	struct mutex lock;
	unsigned int num_leds;
	struct winwing_led leds[];
};

static int winwing_led_write(struct led_classdev *cdev, enum led_brightness br)
{
	struct winwing_led *led = (struct winwing_led *) cdev;
	struct winwing_drv_data *data = hid_get_drvdata(led->hdev);
	__u8 *buf = data->report_buf;
	int ret;

	mutex_lock(&data->lock);

	buf[0] = 0x02;
	buf[1] = 0x60;
	buf[2] = 0xbe;
	buf[3] = 0x00;
	buf[4] = 0x00;
	buf[5] = 0x03;
	buf[6] = 0x49;
	buf[7] = led->number;
	buf[8] = br;
	buf[9] = 0x00;
	buf[10] = 0;
	buf[11] = 0;
	buf[12] = 0;
	buf[13] = 0;

	ret = hid_hw_output_report(led->hdev, buf, 14);

	mutex_unlock(&data->lock);

	return ret;
}

static int winwing_init_led(struct hid_device *hdev, struct input_dev *input)
{
	struct winwing_drv_data *data;
	struct winwing_led *led;
	int ret;
	int i;

	data = hid_get_drvdata(hdev);

	if (!data)
		return -EINVAL;

	data->report_buf = devm_kmalloc(&hdev->dev, MAX_REPORT, GFP_KERNEL);

	if (!data->report_buf)
		return -ENOMEM;

	for (i = 0; i < 3; i += 1) {
		const struct winwing_led_info *info = &led_info[i];

		led = &data->leds[i];
		led->hdev = hdev;
		led->number = info->number;
		led->cdev.max_brightness = info->max_brightness;
		led->cdev.brightness_set_blocking = winwing_led_write;
		led->cdev.flags = LED_HW_PLUGGABLE;
		led->cdev.name = devm_kasprintf(&hdev->dev, GFP_KERNEL,
						"%s::%s",
						dev_name(&input->dev),
						info->led_name);

		ret = devm_led_classdev_register(&hdev->dev, &led->cdev);
		if (ret)
			return ret;
	}

	return ret;
}

static int winwing_probe(struct hid_device *hdev,
			const struct hid_device_id *id)
{
	struct winwing_drv_data *data;
	size_t data_size = struct_size(data, leds, 3);
	int ret;

	ret = hid_parse(hdev);
	if (ret) {
		hid_err(hdev, "parse failed\n");
		return ret;
	}


	data = devm_kzalloc(&hdev->dev, data_size, GFP_KERNEL);

	if (!data)
		return -ENOMEM;

	if (id->driver_data == WW_F15E) {
		data->remap = remap_f15e;
	}

	hid_set_drvdata(hdev, data);

	ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
	if (ret) {
		hid_err(hdev, "hw start failed\n");
		return ret;
	}

	return 0;
}

static int winwing_input_configured(struct hid_device *hdev,
			struct hid_input *hidinput)
{
	int ret;

	ret = winwing_init_led(hdev, hidinput->input);

	if (ret)
		hid_err(hdev, "led init failed\n");

	return ret;
}

static const __u8 rdesc_buttons_111_original[] = {
	0x05, 0x09, 0x19, 0x01, 0x29, 0x6F,
	0x15, 0x00, 0x25, 0x01, 0x35, 0x00,
	0x45, 0x01, 0x75, 0x01, 0x95, 0x6F,
	0x81, 0x02, 0x75, 0x01, 0x95, 0x01,
	0x81, 0x01
};

static const __u8 rdesc_buttons_111_modified[] = {
	0x05, 0x09, 0x19, 0x01, 0x29, 0x4F,
	0x15, 0x00, 0x25, 0x01, 0x35, 0x00,
	0x45, 0x01, 0x75, 0x01, 0x95, 0x4F,
	0x81, 0x02, 0x75, 0x01, 0x95, 0x21,
	0x81, 0x01
};

static const __u8 rdesc_buttons_128_original[] = {
	0x05, 0x09, 0x19, 0x01, 0x29, 0x80,
	0x15, 0x00, 0x25, 0x01, 0x35, 0x00,
	0x45, 0x01, 0x75, 0x01,	0x95, 0x80,
	0x81, 0x02
};

static const __u8 rdesc_buttons_128_modified[] = {
	0x05, 0x09, 0x19, 0x01, 0x29, 0x50,
	0x15, 0x00, 0x25, 0x01, 0x35, 0x00,
	0x45, 0x01, 0x75, 0x01,	0x95, 0x50,
	0x81, 0x02, 0x75, 0x01, 0x95, 0x30,
	0x81, 0x01
};

struct descriptor_patch {
	const __u8 *original;
	const __u8 *modified;
	int original_size;
	int modified_size;
	const char *name;
};

static struct descriptor_patch patch_table[] = {
	{ rdesc_buttons_111_original, rdesc_buttons_111_modified, sizeof(rdesc_buttons_111_original), sizeof(rdesc_buttons_111_modified), "111 buttons" },
	{ rdesc_buttons_128_original, rdesc_buttons_128_modified, sizeof(rdesc_buttons_128_original), sizeof(rdesc_buttons_128_modified), "128 buttons" },
	{ NULL, NULL, 0, 0 }
};

/*
 * HID report descriptor shows 111 buttons, which exceeds maximum
 * number of buttons (80) supported by Linux kernel HID subsystem.
 *
 * This module skips numbers 32-63, unused on some throttle grips.
 */

static __u8 *winwing_report_fixup(struct hid_device *hdev, __u8 *rdesc,
		unsigned int *rsize)
{
	struct descriptor_patch *patch = patch_table;
	const int offset_buttons = 8;

	while (patch->original) {
		if (patch->original_size + offset_buttons > (*rsize)) {
			continue;
		}

		if (memcmp(rdesc + offset_buttons, patch->original, patch->original_size) == 0) {
			int src_offset = 0;
			int dst_offset = 0;
			int new_rsize = *rsize;
			__u8 *new_rdesc;

			/* Compute size of new rdesc, where original part is replaced with modified one */
			new_rsize -= patch->original_size;
			new_rsize += patch->modified_size;

			new_rdesc = devm_kzalloc(&hdev->dev, new_rsize, GFP_KERNEL);
			if (!new_rdesc) {
				hid_err(hdev, "unable to allocate new report descriptor\n");
				return rdesc;
			}

			/* Copy the part before button info */
			memcpy(new_rdesc, rdesc, offset_buttons);
			src_offset += offset_buttons;
			dst_offset += offset_buttons;

			/* Copy modified button info instead of original button info */
			memcpy(new_rdesc + dst_offset, patch->modified, patch->modified_size);
			src_offset += sizeof(patch->original_size);
			dst_offset += sizeof(patch->modified_size);

			/* Copy the rest of report descriptor */
			memcpy(new_rdesc + dst_offset, rdesc + src_offset, new_rsize - dst_offset);

			hid_info(hdev, "winwing descriptor (%s) fixed\n", patch->name);

			*rsize = new_rsize;
			return new_rdesc;
		}
	}

	return rdesc;
}

static int winwing_raw_event(struct hid_device *hdev,
		struct hid_report *report, u8 *raw_data, int size)
{
	struct winwing_drv_data *data = hid_get_drvdata(hdev);

	if (!data)
		return -EINVAL;

        if (data->remap) {
		const __u64 one = 1;
		int src_bit, dst_bit;
		__u64 mask_src, mask_dst;
		__u64 grip_button_map = 0;
		int i, k;

		for (k = 0; k < 8; k += 1) {
			__u64 oct = raw_data[k + 1];
			grip_button_map |= oct << (k * 8);
		}

		i = 0;
		while (data->remap[i] <= 64) {
			src_bit = data->remap[i++] - 1;
			dst_bit = data->remap[i++] - 1;
			mask_src = one << src_bit;
			mask_dst = one << dst_bit;
			if ((grip_button_map & mask_src) != 0) {
				grip_button_map |= mask_dst;
			} else {
				grip_button_map &= ~mask_dst;
			}
		}

		grip_button_map &= 0xffffffff;

		for (k = 0; k < 8; k += 1) {
			raw_data[k + 1] = grip_button_map & 0xff;
			grip_button_map >>= 8;
		}
        }

	if (size >= 15) {
		/* Throttle base buttons are remapped from [64 .. 111] to [32 .. 79] */
		memmove(raw_data + 5, raw_data + 9, 6);

		/* Clear the padding */
		memset(raw_data + 11, 0, 4);
	}

	return 0;
}

static const struct hid_device_id winwing_devices[] = {
	{ HID_USB_DEVICE(0x4098, 0xbe62) },  /* TGRIP-18 */
	{ HID_USB_DEVICE(0x4098, 0xbe68) },  /* TGRIP-16EX */
	{ HID_USB_DEVICE(0x4098, 0xbd65), .driver_data = WW_F15E },
	{ HID_USB_DEVICE(0x4098, 0xbd64), .driver_data = WW_F15E /* TGRIP-15EX */ },
	{}
};

MODULE_DEVICE_TABLE(hid, winwing_devices);

static struct hid_driver winwing_driver = {
	.name = "winwing",
	.id_table = winwing_devices,
	.probe = winwing_probe,
	.input_configured = winwing_input_configured,
	.report_fixup = winwing_report_fixup,
	.raw_event = winwing_raw_event,
};
module_hid_driver(winwing_driver);

MODULE_LICENSE("GPL");

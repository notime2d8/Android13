/*
 * Input driver for resistor ladder connected on ADC
 *
 * Copyright (c) 2016 Alexandre Belloni
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include <linux/err.h>
#include <linux/iio/consumer.h>
#include <linux/iio/types.h>
#include <linux/input-polldev.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>

struct adc_joystick_state {
	struct iio_channel *channelx;
	struct iio_channel *channely;
	struct iio_channel *channelhall;
	// struct iio_channel *channelz;
	// struct iio_channel *channelrz;
	int adc_ena_gpios;
	int adc_enb_gpios;
	int adc_hall_gpios;
	int num_joystick;
	unsigned char data[4];
	unsigned char btn_data[2];
};

static unsigned char abs_date[]={128,128,128,128};
//static unsigned char adcval_maxmin[]={2048,2048,2048,2048,0,0,0,0};
//static unsigned char adcval_mid[]={1024,1024,1024,1024};
static unsigned char btn_date[]={128,128};

static int adc_to_abs(int val) {
	int abs_val;
	// if(val<500)val = val-200;
	// if(val>1500)val = val+200;
	val = val+100;
	if(val<0) val = 0;
	if(val>2048) val = 2048;
	abs_val = 255 - val * 255 / 2048;
	if(abs_val >210) abs_val = 255;
	if(abs_val <45) abs_val = 0;
	// if(abs_val > (128 - 25) && abs_val < (128 + 20)) abs_val = 128;
	if(abs_val > (128 - 32) && abs_val < (128 + 37)) abs_val = 128;
	return abs_val;
};

static int adc_to_btn(int val) {
	int btn_val;
	if(val > 450) btn_val = 0;
	else btn_val = 1;
	return btn_val;
};

static void adc_joystick_poll(struct input_polled_dev *dev) {
	struct adc_joystick_state *st = dev->private;
	int value, ret;
	int state = 0;

	if (!gpio_is_valid(st->adc_ena_gpios) ||
		!gpio_is_valid(st->adc_enb_gpios) ||
		!gpio_is_valid(st->adc_hall_gpios)) {
		printk("adc_joystick_poll: No valid gpios\n");
		return;
	}

	gpio_direction_output(st->adc_ena_gpios, 0);
	gpio_direction_output(st->adc_enb_gpios, 0);
	gpio_direction_output(st->adc_hall_gpios, 0);

	// joystick x
	ret = iio_read_channel_processed(st->channelx, &value);
	if (unlikely(ret < 0)) {
		/* Forcibly release key if any was pressed */
		value = 1024;
	}

	st->data[0] = 255 - adc_to_abs(value);
	if(st->data[0] == 127) st->data[0] = 128;
	// printk("adc_joystick_poll: read joystick x_val = %d \n",value);

	// joystick y
	ret = iio_read_channel_processed(st->channely, &value);
	if (unlikely(ret < 0)) {
		/* Forcibly release key if any was pressed */
		value = 1024;
	}

	st->data[1] = adc_to_abs(value);
	// printk("adc_joystick_poll: read joystick y_val = %d \n",value);

	// joystick hall
	ret = iio_read_channel_processed(st->channelhall, &value);
	if (unlikely(ret < 0)) {
		value = 1024;
	}
	st->btn_data[0] = adc_to_btn(value);
	// printk("adc_joystick_poll: read joystick hall = %d \n",value);

	gpio_set_value(st->adc_ena_gpios, 1);
	gpio_set_value(st->adc_enb_gpios, 1);
	gpio_set_value(st->adc_hall_gpios, 1);

	// joystick rz
	ret = iio_read_channel_processed(st->channelx, &value);
	if (unlikely(ret < 0)) {
		/* Forcibly release key if any was pressed */
		value = 1024;
	}

	st->data[2] = 255 - adc_to_abs(value);
	if(st->data[2] == 127) st->data[2] = 128;
	// printk("adc_joystick_poll: read joystick rz_val = %d \n",value);

	// joystick z
	ret = iio_read_channel_processed(st->channely, &value);
	if (unlikely(ret < 0)) {
		/* Forcibly release key if any was pressed */
		value = 1024;
	}

	st->data[3] = 255 - adc_to_abs(value);
	if(st->data[3] == 127) st->data[3] = 128;
	// printk("adc_joystick_poll: read joystick z_val = %d \n",value);

	// joystick hall
	ret = iio_read_channel_processed(st->channelhall, &value);
	if (unlikely(ret < 0)) {
		value = 1024;
	}
	st->btn_data[1] = adc_to_btn(value);
	// printk("adc_joystick_poll: read joystick hall = %d \n",value);

	if(abs_date[0] != st->data[0]) state++;
	if(abs_date[1] != st->data[1]) state++;
	if(abs_date[2] != st->data[2]) state++;
	if(abs_date[3] != st->data[3]) state++;
	if(btn_date[0] != st->btn_data[0]) state++;
	if(btn_date[1] != st->btn_data[1]) state++;

	if(state > 0){
		abs_date[0] = st->data[0];
		abs_date[1] = st->data[1];
		abs_date[2] = st->data[2];
		abs_date[3] = st->data[3];
		btn_date[0] = st->btn_data[0];
		btn_date[1] = st->btn_data[1];

		input_report_abs(dev->input, ABS_X,  abs_date[0]);
		input_report_abs(dev->input, ABS_Y,  abs_date[1]);
		input_report_abs(dev->input, ABS_RZ, abs_date[2]);
		input_report_abs(dev->input, ABS_Z,  abs_date[3]);
		input_report_key(dev->input, BTN_TL2,  btn_date[0]);
		input_report_key(dev->input, BTN_TR2,  btn_date[1]);

		input_sync(dev->input);
		state = 0;
		// printk("adc_joystick_poll: input_report_abs [x=%d,y=%d,z=%d,rz=%d]\n",abs_date[0],abs_date[1],abs_date[2],abs_date[3]);
	}
}

static int adc_joystick_probe(struct platform_device *pdev) {
	struct device *dev = &pdev->dev;
	struct adc_joystick_state *st;
	struct input_polled_dev *poll_dev;
	struct input_dev *input;
	enum iio_chan_type type;
	int value;
	int error;
	printk("adc_joystick_probe start \n");
	st = devm_kzalloc(dev, sizeof(*st), GFP_KERNEL);
	if (!st) return -ENOMEM;

	//joystick x
	printk("adc_joystick_probe joystick_x \n");
	st->channelx = devm_iio_channel_get(dev, "joystick_x");
	if (IS_ERR(st->channelx))
		return PTR_ERR(st->channelx);

	if (!st->channelx->indio_dev)
		return -ENXIO;

	error = iio_get_channel_type(st->channelx, &type);
	if (error < 0)
		return error;
		
	if (type != IIO_VOLTAGE) {
		dev_err(dev, "Incompatible channel type %d\n", type);
		return -EINVAL;
	}

	//joystick y
	printk("adc_joystick_probe joystick_y \n");
	st->channely = devm_iio_channel_get(dev, "joystick_y");
	if (IS_ERR(st->channely))
		return PTR_ERR(st->channely);

	if (!st->channely->indio_dev)
		return -ENXIO;

	error = iio_get_channel_type(st->channely, &type);
	if (error < 0)
		return error;
		
	if (type != IIO_VOLTAGE) {
		dev_err(dev, "Incompatible channel type %d\n", type);
		return -EINVAL;
	}

	//joystick hall
	printk("adc_joystick_probe joystick_hall \n");
	st->channelhall = devm_iio_channel_get(dev, "joystick_hall");
	if (IS_ERR(st->channelhall))
		return PTR_ERR(st->channelhall);

	if (!st->channelhall->indio_dev)
		return -ENXIO;

	error = iio_get_channel_type(st->channelhall, &type);
	if (error < 0)
		return error;

	if (type != IIO_VOLTAGE) {
		dev_err(dev, "Incompatible channel type %d\n", type);
		return -EINVAL;
	}

	st->adc_ena_gpios = of_get_named_gpio(dev->of_node, "adc-ena-gpios", 0);
	if (!gpio_is_valid(st->adc_ena_gpios))
		dev_err(dev, "No valid adc_ena_gpios");

	st->adc_enb_gpios = of_get_named_gpio(dev->of_node, "adc-enb-gpios", 0);
	if (!gpio_is_valid(st->adc_enb_gpios))
		dev_err(dev, "No valid adc_enb_gpios");

	st->adc_hall_gpios = of_get_named_gpio(dev->of_node, "adc-hall-gpios", 0);
	if (!gpio_is_valid(st->adc_hall_gpios))
		dev_err(dev, "No valid adc_hall_gpios");

	poll_dev = devm_input_allocate_polled_device(dev);
	if (!poll_dev) {
		dev_err(dev, "failed to allocate input device\n");
		return -ENOMEM;
	}

	if (!device_property_read_u32(dev, "poll-interval", &value))
		poll_dev->poll_interval = value;

	poll_dev->poll = adc_joystick_poll;
	poll_dev->private = st;

	input = poll_dev->input;

	input->name = pdev->name;
	input->phys = "adc-joysticks/input0";

	input->id.bustype = BUS_HOST;
	input->id.vendor = 0x0810;
	input->id.product = 0x0002;
	input->id.version = 0x0100;
	input->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	__set_bit(EV_ABS, input->evbit);
	__set_bit(EV_KEY, input->evbit);
	__set_bit(KEY_UP, input->keybit);
	__set_bit(KEY_DOWN, input->keybit);
	__set_bit(KEY_LEFT, input->keybit);
	__set_bit(KEY_RIGHT, input->keybit);
	__set_bit(BTN_A, input->keybit);
	__set_bit(BTN_B, input->keybit);
	__set_bit(BTN_X, input->keybit);
	__set_bit(BTN_Y, input->keybit);
	__set_bit(BTN_TL2, input->keybit);
	__set_bit(BTN_TR2, input->keybit);

	set_bit(ABS_X, input->absbit);
	set_bit(ABS_Y, input->absbit);
	set_bit(ABS_RZ, input->absbit);
	set_bit(ABS_Z, input->absbit);
	input_set_abs_params(input, ABS_X, 0, 255, 0, 15);
	input_set_abs_params(input, ABS_Z, 0, 255, 0, 15);
	input_set_abs_params(input, ABS_RZ, 0, 255, 0, 15);
	input_set_abs_params(input, ABS_Y, 0, 255, 0, 15);

	/* Enable auto repeat feature of Linux input subsystem */
	if (device_property_read_bool(dev, "autorepeat"))
		__set_bit(EV_REP, input->evbit);
		
	error = input_register_polled_device(poll_dev);
	if (error) {
		dev_err(dev, "Unable to register input device: %d\n", error);
		return error;
	}
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id adc_joystick_of_match[] = {
	{ .compatible = "adc-joysticks", },
	{ }
};
MODULE_DEVICE_TABLE(of, adc_joystick_of_match);
#endif

static struct platform_driver __refdata adc_joystick_driver = {
	.driver = {
		.name = "adc_joysticks",
		.of_match_table = of_match_ptr(adc_joystick_of_match),
	},
	.probe = adc_joystick_probe,
};
module_platform_driver(adc_joystick_driver);

MODULE_AUTHOR("Alexandre Belloni <alexandre.belloni@free-electrons.com>");
MODULE_DESCRIPTION("Input driver for resistor ladder connected on ADC");
MODULE_LICENSE("GPL v2");

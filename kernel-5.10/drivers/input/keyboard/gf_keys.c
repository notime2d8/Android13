// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for keys on GPIO lines capable of generating interrupts.
 *
 * Copyright 2005 Phil Blundell
 * Copyright 2010, 2011 David Jander <david@protonic.nl>
 */

#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/spinlock.h>
#include <dt-bindings/input/gpio-keys.h>

#include <linux/err.h>
#include <linux/iio/consumer.h>
#include <linux/iio/types.h>
#include <linux/input-polldev.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/property.h>
#include <linux/of_gpio.h>
#include <linux/device/driver.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <linux/atomic.h>

static int dbg_enable = 0;

#define DBG(args...) \
	do { \
		if (dbg_enable) { \
			pr_info(args); \
		} \
	} while(0)


struct gpio_button_data {
	const struct gpio_keys_button *button;
	struct input_dev *input;
	struct gpio_desc *gpiod;

	unsigned short *code;

	struct timer_list release_timer;
	unsigned int release_delay;	/* in msecs, for IRQ-only buttons */

	struct delayed_work work;
	unsigned int software_debounce;	/* in msecs, for GPIO-driven buttons */

	unsigned int irq;
	unsigned int wakeup_trigger_type;
	spinlock_t lock;
	bool disabled;
	bool key_pressed;
	bool suspended;

};

struct gpio_keys_drvdata {
	const struct gpio_keys_platform_data *pdata;
	struct input_dev *input;
	struct mutex disable_lock;
	unsigned short *keymap;
	
	struct iio_channel *channelx;
	struct iio_channel *channely;
	struct iio_channel *channelz;
	struct iio_channel *channelrz;
	int num_joystick;
	
	unsigned char now_data[4];
	int raw_data[4];
	
	int last_bra;
	int last_gas;
	
	struct gpio_button_data data[];
};

static unsigned char abs_date[]={128,128,128,128};

extern atomic_t ads1015_shared_bradata;
extern atomic_t ads1015_shared_gasdata;

static int average_data(struct iio_channel *chan){
	int i,value;
	int sum = 0;
	int ret = 0;
	DBG("**********************\n");
	for(i = 0;i<10;i++){
		ret = iio_read_channel_processed(chan, &value);
//		DBG("average_data value = %d\n",value);
		sum+=value;
	}
	sum = sum/10;
	DBG("average_data**********************sum = %d\n",sum);
	return sum;
}

static int get_raw_data(struct gpio_keys_drvdata *st){

	st->raw_data[0] = average_data(st->channelx);
	DBG("lqblqb get_raw_data x = %d\n",st->raw_data[0]);

	st->raw_data[1] = average_data(st->channely);
	DBG("lqblqb get_raw_data y = %d\n",st->raw_data[1]);
	
	st->raw_data[2] = average_data(st->channelrz);
	DBG("lqblqb get_raw_data rz = %d\n",st->raw_data[2]);

	st->raw_data[3] = average_data(st->channelz);
	DBG("lqblqb get_raw_data z = %d\n",st->raw_data[3]);
	return 0;
}


static int cal_data(int val,int data){
	int ret = 0;
	if(val > (data +100))
	{
		ret = 128 + (val-data) * 127/(1700 -data);		
	}
	else if(val < (data - 100 ))
	{
		ret = val * 127/(data-100);	
	}
	else {
		ret = 128;
	}
	
	return	ret;

}


static int adc_to_abs(int val,int data) {
	int abs_val;
	if(val > 1700) val = 1700;
		else if(val < 100) val = 0;
	abs_val = cal_data(val,data);
	return abs_val;
};

static void adc_joystick_poll(struct input_dev *dev){

	struct gpio_keys_drvdata *st = input_get_drvdata(dev);
	int value, ret;
	int state = 0;
	int data_bra =0;
	int data_gas = 0;
	DBG("lqbjoy *******************************\n");

	// joystick x
	ret = iio_read_channel_processed(st->channelx, &value);
	if (unlikely(ret < 0)) {
		/* Forcibly release key if any was pressed */
		value = 1024;
	}
	
	st->now_data[0] = adc_to_abs(value,st->raw_data[0]);
	DBG("lqbjoy read adc_to_abs[x_val] %d,value = %d\n",adc_to_abs(value,st->raw_data[0]),value);
//	if(st->data[0] == 127) st->data[0] = 128;
	
	// joystick y
	ret = iio_read_channel_processed(st->channely, &value);
	if (unlikely(ret < 0)) {
		value = 1024;
	}
	
	st->now_data[1] = 255 - adc_to_abs(value,st->raw_data[1]);
	 DBG("lqbjoy read adc_to_abs[y_val] %d----value = %d\n",adc_to_abs(value,st->raw_data[1]),value);

	// joystick rz
	ret = iio_read_channel_processed(st->channelrz, &value);
	if (unlikely(ret < 0)) {
		/* Forcibly release key if any was pressed */
		value = 1024;
	}

	st->now_data[2] = adc_to_abs(value,st->raw_data[2]);
	
	DBG("lqbjoy read adc_to_abs[rz_val] %d----value = %d\n",adc_to_abs(value,st->raw_data[2]),value);
//	if(st->data[2] == 127) st->data[2] = 128;
	
	// joystick z
	ret = iio_read_channel_processed(st->channelz, &value);
	if (unlikely(ret < 0)) {
		/* Forcibly release key if any was pressed */
		value = 1024;
	}
	
	st->now_data[3] = adc_to_abs(value,st->raw_data[3]);
	
	DBG("lqbjoy read adc_to_abs[z_val] %d----value = %d\n",adc_to_abs(value,st->raw_data[3]),value);
	
	data_bra = atomic_read(&ads1015_shared_bradata);
	DBG("ABS_BRAKE value = %d\n",data_bra);
	data_gas = atomic_read(&ads1015_shared_gasdata);
	DBG("ABS_GAS value = %d\n",data_gas);

	if(data_bra) input_event(st->input, EV_KEY, BTN_TL2, 1);
	else input_event(st->input, EV_KEY, BTN_TL2, 0);
	
	if(data_gas) input_event(st->input, EV_KEY, BTN_TR2, 1);
	else input_event(st->input, EV_KEY, BTN_TR2, 0);
	
	if(abs_date[0] != st->now_data[0]) state++;
	if(abs_date[1] != st->now_data[1]) state++;
	if(abs_date[2] != st->now_data[2]) state++;
	if(abs_date[3] != st->now_data[3]) state++;
	if(data_gas != st->last_gas) state++;
	if(data_bra != st->last_bra) state++;
	if(state > 0){
		abs_date[0] = st->now_data[0];
		abs_date[1] = st->now_data[1];
		abs_date[2] = st->now_data[2];
		abs_date[3] = st->now_data[3];
		st->last_gas = data_gas;
		st->last_bra = data_bra;
		DBG("lqb datax = %d,datay= %d,datarz = %d,dataz= %d\n",abs_date[0],abs_date[1],abs_date[2],abs_date[3]);
		input_report_abs(st->input, ABS_X,  abs_date[0]);
		input_report_abs(st->input, ABS_Y,  abs_date[1]);

		input_report_abs(st->input, ABS_RZ, abs_date[2]);
		input_report_abs(st->input, ABS_Z,  abs_date[3]);

		input_report_abs(st->input, ABS_GAS,data_gas);	
		input_report_abs(st->input, ABS_BRAKE,data_bra);

		DBG("lqb input_report_abs ABS_GAS= %d,ABS_BRAKE= %d\n",data_gas,data_bra);
		input_sync(st->input);
		state = 0;
	}
	
	DBG("lqbjoy*******************************\n");
}


static ssize_t joystick_calibration_show(struct device *pdev,
			struct device_attribute *attr, char *buf){
	struct gpio_keys_drvdata *st;
	struct platform_device *dev;
		DBG("lqb joystick_calibration_show	\n");
		
		dev = to_platform_device(pdev);
		st = dev_get_drvdata(&dev->dev);		
		DBG("st->raw_data[0] = %d,st->raw_data[0] = %d,st->raw_data[0] = %d,st->raw_data[0] = %d\n",st->raw_data[0],st->raw_data[1],st->raw_data[2],st->raw_data[3]);
	return sprintf(buf, "%d,%d,%d,%d\n", st->raw_data[0],st->raw_data[1],st->raw_data[2],st->raw_data[3]);
}

static ssize_t joystick_calibration_store(struct device *pdev,
			struct device_attribute *attr, const char *buf, size_t count){
	struct platform_device *dev;
	struct gpio_keys_drvdata *st;
	char *token;
	int err;
	int cou = 0;
	char str[30];
	int val[4];
	char *cur = str;
	
	memset(val,0,sizeof(val));
	DBG("joystick_calibration_store lqb buf:%s count:%d\n",buf,count);
	strcpy(str,buf);
	
	dev = to_platform_device(pdev);
	DBG("joystick_calibration_store str =  %s\n",str);
	st = dev_get_drvdata(&dev->dev);
	if(buf == NULL) return -1;
	if(count < 3)
	{
		DBG("joystick_calibration_store buf == 1,get_raw_data");
		get_raw_data(st);	
	}
	else{
		token = strsep(&cur,",");
		while( token && cou < 4){
			err = kstrtoint(token,10,&val[cou]);
			DBG("joystick_calibration_store %d Conversion result is %d\n",cou,val[cou]);
		    if (err) {
        		pr_err("joystick_calibration_store Failed to parse string: %s\n", str);
		        return err;
   			}
			st->raw_data[cou] = val[cou];
			token = strsep(&cur,",");
			cou++;
		}
	}

	return count;
}	


static DEVICE_ATTR_RW(joystick_calibration);


static struct attribute *joystick_calibration_attrs[] = {
        &dev_attr_joystick_calibration.attr,
        NULL
 };


static const struct attribute_group  joystick_calibration_group_attrs = {
	.attrs = joystick_calibration_attrs,
};

/*
 * SYSFS interface for enabling/disabling keys and switches:
 *
 * There are 4 attributes under /sys/devices/platform/gpio-keys/
 *	keys [ro]              - bitmap of keys (EV_KEY) which can be
 *	                         disabled
 *	switches [ro]          - bitmap of switches (EV_SW) which can be
 *	                         disabled
 *	disabled_keys [rw]     - bitmap of keys currently disabled
 *	disabled_switches [rw] - bitmap of switches currently disabled
 *
 * Userland can change these values and hence disable event generation
 * for each key (or switch). Disabling a key means its interrupt line
 * is disabled.
 *
 * For example, if we have following switches set up as gpio-keys:
 *	SW_DOCK = 5
 *	SW_CAMERA_LENS_COVER = 9
 *	SW_KEYPAD_SLIDE = 10
 *	SW_FRONT_PROXIMITY = 11
 * This is read from switches:
 *	11-9,5
 * Next we want to disable proximity (11) and dock (5), we write:
 *	11,5
 * to file disabled_switches. Now proximity and dock IRQs are disabled.
 * This can be verified by reading the file disabled_switches:
 *	11,5
 * If we now want to enable proximity (11) switch we write:
 *	5
 * to disabled_switches.
 *
 * We can disable only those keys which don't allow sharing the irq.
 */

/**
 * get_n_events_by_type() - returns maximum number of events per @type
 * @type: type of button (%EV_KEY, %EV_SW)
 *
 * Return value of this function can be used to allocate bitmap
 * large enough to hold all bits for given type.
 */
static int get_n_events_by_type(int type)
{
	BUG_ON(type != EV_SW && type != EV_KEY);

	return (type == EV_KEY) ? KEY_CNT : SW_CNT;
}

/**
 * get_bm_events_by_type() - returns bitmap of supported events per @type
 * @input: input device from which bitmap is retrieved
 * @type: type of button (%EV_KEY, %EV_SW)
 *
 * Return value of this function can be used to allocate bitmap
 * large enough to hold all bits for given type.
 */
static const unsigned long *get_bm_events_by_type(struct input_dev *dev,
						  int type)
{
	BUG_ON(type != EV_SW && type != EV_KEY);

	return (type == EV_KEY) ? dev->keybit : dev->swbit;
}

/**
 * gpio_keys_disable_button() - disables given GPIO button
 * @bdata: button data for button to be disabled
 *
 * Disables button pointed by @bdata. This is done by masking
 * IRQ line. After this function is called, button won't generate
 * input events anymore. Note that one can only disable buttons
 * that don't share IRQs.
 *
 * Make sure that @bdata->disable_lock is locked when entering
 * this function to avoid races when concurrent threads are
 * disabling buttons at the same time.
 */
static void gpio_keys_disable_button(struct gpio_button_data *bdata)
{
	if (!bdata->disabled) {
		/*
		 * Disable IRQ and associated timer/work structure.
		 */
		disable_irq(bdata->irq);

		if (bdata->gpiod)
			cancel_delayed_work_sync(&bdata->work);
		else
			del_timer_sync(&bdata->release_timer);

		bdata->disabled = true;
	}
}

/**
 * gpio_keys_enable_button() - enables given GPIO button
 * @bdata: button data for button to be disabled
 *
 * Enables given button pointed by @bdata.
 *
 * Make sure that @bdata->disable_lock is locked when entering
 * this function to avoid races with concurrent threads trying
 * to enable the same button at the same time.
 */
static void gpio_keys_enable_button(struct gpio_button_data *bdata)
{
	if (bdata->disabled) {
		enable_irq(bdata->irq);
		bdata->disabled = false;
	}
}

/**
 * gpio_keys_attr_show_helper() - fill in stringified bitmap of buttons
 * @ddata: pointer to drvdata
 * @buf: buffer where stringified bitmap is written
 * @type: button type (%EV_KEY, %EV_SW)
 * @only_disabled: does caller want only those buttons that are
 *                 currently disabled or all buttons that can be
 *                 disabled
 *
 * This function writes buttons that can be disabled to @buf. If
 * @only_disabled is true, then @buf contains only those buttons
 * that are currently disabled. Returns 0 on success or negative
 * errno on failure.
 */
static ssize_t gpio_keys_attr_show_helper(struct gpio_keys_drvdata *ddata,
					  char *buf, unsigned int type,
					  bool only_disabled)
{
	int n_events = get_n_events_by_type(type);
	unsigned long *bits;
	ssize_t ret;
	int i;

	bits = bitmap_zalloc(n_events, GFP_KERNEL);
	if (!bits)
		return -ENOMEM;

	for (i = 0; i < ddata->pdata->nbuttons; i++) {
		struct gpio_button_data *bdata = &ddata->data[i];

		if (bdata->button->type != type)
			continue;

		if (only_disabled && !bdata->disabled)
			continue;

		__set_bit(*bdata->code, bits);
	}

	ret = scnprintf(buf, PAGE_SIZE - 1, "%*pbl", n_events, bits);
	buf[ret++] = '\n';
	buf[ret] = '\0';

	bitmap_free(bits);

	return ret;
}

/**
 * gpio_keys_attr_store_helper() - enable/disable buttons based on given bitmap
 * @ddata: pointer to drvdata
 * @buf: buffer from userspace that contains stringified bitmap
 * @type: button type (%EV_KEY, %EV_SW)
 *
 * This function parses stringified bitmap from @buf and disables/enables
 * GPIO buttons accordingly. Returns 0 on success and negative error
 * on failure.
 */
static ssize_t gpio_keys_attr_store_helper(struct gpio_keys_drvdata *ddata,
					   const char *buf, unsigned int type)
{
	int n_events = get_n_events_by_type(type);
	const unsigned long *bitmap = get_bm_events_by_type(ddata->input, type);
	unsigned long *bits;
	ssize_t error;
	int i;

	bits = bitmap_zalloc(n_events, GFP_KERNEL);
	if (!bits)
		return -ENOMEM;

	error = bitmap_parselist(buf, bits, n_events);
	if (error)
		goto out;

	/* First validate */
	if (!bitmap_subset(bits, bitmap, n_events)) {
		error = -EINVAL;
		goto out;
	}

	for (i = 0; i < ddata->pdata->nbuttons; i++) {
		struct gpio_button_data *bdata = &ddata->data[i];

		if (bdata->button->type != type)
			continue;

		if (test_bit(*bdata->code, bits) &&
		    !bdata->button->can_disable) {
			error = -EINVAL;
			goto out;
		}
	}

	mutex_lock(&ddata->disable_lock);

	for (i = 0; i < ddata->pdata->nbuttons; i++) {
		struct gpio_button_data *bdata = &ddata->data[i];

		if (bdata->button->type != type)
			continue;

		if (test_bit(*bdata->code, bits))
			gpio_keys_disable_button(bdata);
		else
			gpio_keys_enable_button(bdata);
	}

	mutex_unlock(&ddata->disable_lock);

out:
	bitmap_free(bits);
	return error;
}

#define ATTR_SHOW_FN(name, type, only_disabled)				\
static ssize_t gpio_keys_show_##name(struct device *dev,		\
				     struct device_attribute *attr,	\
				     char *buf)				\
{									\
	struct platform_device *pdev = to_platform_device(dev);		\
	struct gpio_keys_drvdata *ddata = platform_get_drvdata(pdev);	\
									\
	return gpio_keys_attr_show_helper(ddata, buf,			\
					  type, only_disabled);		\
}

ATTR_SHOW_FN(keys, EV_KEY, false);
ATTR_SHOW_FN(switches, EV_SW, false);
ATTR_SHOW_FN(disabled_keys, EV_KEY, true);
ATTR_SHOW_FN(disabled_switches, EV_SW, true);

/*
 * ATTRIBUTES:
 *
 * /sys/devices/platform/gpio-keys/keys [ro]
 * /sys/devices/platform/gpio-keys/switches [ro]
 */
static DEVICE_ATTR(keys, S_IRUGO, gpio_keys_show_keys, NULL);
static DEVICE_ATTR(switches, S_IRUGO, gpio_keys_show_switches, NULL);

#define ATTR_STORE_FN(name, type)					\
static ssize_t gpio_keys_store_##name(struct device *dev,		\
				      struct device_attribute *attr,	\
				      const char *buf,			\
				      size_t count)			\
{									\
	struct platform_device *pdev = to_platform_device(dev);		\
	struct gpio_keys_drvdata *ddata = platform_get_drvdata(pdev);	\
	ssize_t error;							\
									\
	error = gpio_keys_attr_store_helper(ddata, buf, type);		\
	if (error)							\
		return error;						\
									\
	return count;							\
}

ATTR_STORE_FN(disabled_keys, EV_KEY);
ATTR_STORE_FN(disabled_switches, EV_SW);

/*
 * ATTRIBUTES:
 *
 * /sys/devices/platform/gpio-keys/disabled_keys [rw]
 * /sys/devices/platform/gpio-keys/disables_switches [rw]
 */
static DEVICE_ATTR(disabled_keys, S_IWUSR | S_IRUGO,
		   gpio_keys_show_disabled_keys,
		   gpio_keys_store_disabled_keys);
static DEVICE_ATTR(disabled_switches, S_IWUSR | S_IRUGO,
		   gpio_keys_show_disabled_switches,
		   gpio_keys_store_disabled_switches);

static struct attribute *gf_keys_attrs[] = {
	&dev_attr_keys.attr,
	&dev_attr_switches.attr,
	&dev_attr_disabled_keys.attr,
	&dev_attr_disabled_switches.attr,
	NULL,
};
ATTRIBUTE_GROUPS(gf_keys);

static void other_key_report(struct input_dev *input,unsigned int code ,int state,int value){
	if(state)
	{
		printk("11111111111other_key_report code = %d,state = %d,value = %d\n",code,state,value);
		input_report_abs(input, code,value);
	}
	else{
		input_report_abs(input, code,127);		
		printk("00000000000other_key_report code = %d,state = %d,value = %d\n",code,state,value);
	}
}

static void gpio_keys_gpio_report_event(struct gpio_button_data *bdata)
{
	const struct gpio_keys_button *button = bdata->button;
	struct input_dev *input = bdata->input;
	unsigned int type = button->type ?: EV_KEY;
	int state;
	unsigned short code = 0;
	state = gpiod_get_value_cansleep(bdata->gpiod);
	
	printk("func = %s lqbirq state = %d,code = %d\n",__func__,state,*bdata->code);
	if (state < 0) {
		dev_err(input->dev.parent,
			"failed to get gpio state: %d\n", state);
		return;
	}

	if (type == EV_ABS) {
		if (state)
			input_event(input, type, button->code, button->value);	
		printk("lqbirq type == EV_ABS");
	} else {
		code = *bdata->code;
		if(code < KEY_UP || code >KEY_DOWN){
			input_event(input, type, *bdata->code, state);
			printk("other_key_report key state = %d\n",state);
		}
		else if( code == KEY_UP ){			
			other_key_report(input,ABS_HAT0Y,state,0);
		}else if(code == KEY_DOWN){
			other_key_report(input,ABS_HAT0Y,state,255);
		}else if(code == KEY_LEFT){
			other_key_report(input,ABS_HAT0X,state,0);
		}else if(code == KEY_RIGHT){
			other_key_report(input,ABS_HAT0X,state,255);
		}
	}
	input_sync(input);
}

static void gpio_keys_gpio_work_func(struct work_struct *work)
{
	struct gpio_button_data *bdata =
		container_of(work, struct gpio_button_data, work.work);

	gpio_keys_gpio_report_event(bdata);

	if (bdata->button->wakeup)
		pm_relax(bdata->input->dev.parent);
}

static irqreturn_t gpio_keys_gpio_isr(int irq, void *dev_id)
{
	struct gpio_button_data *bdata = dev_id;

	BUG_ON(irq != bdata->irq);
	DBG("lqbirq func = %s\n,irq = %d\n",__func__,irq);
	if (bdata->button->wakeup) {
		const struct gpio_keys_button *button = bdata->button;

		pm_stay_awake(bdata->input->dev.parent);
		if (bdata->suspended  &&
		    (button->type == 0 || button->type == EV_KEY)) {
			/*
			 * Simulate wakeup key press in case the key has
			 * already released by the time we got interrupt
			 * handler to run.
			 */
			input_report_key(bdata->input, button->code, 1);
		}
	}

	mod_delayed_work(system_wq,
			 &bdata->work,
			 msecs_to_jiffies(bdata->software_debounce));

	return IRQ_HANDLED;
}

static void gpio_keys_irq_timer(struct timer_list *t)
{
	struct gpio_button_data *bdata = from_timer(bdata, t, release_timer);
	struct input_dev *input = bdata->input;
	unsigned long flags;
	spin_lock_irqsave(&bdata->lock, flags);
	if (bdata->key_pressed) {
		input_event(input, EV_KEY, *bdata->code, 0);
		input_sync(input);
		bdata->key_pressed = false;
	}
	spin_unlock_irqrestore(&bdata->lock, flags);
}

static irqreturn_t gpio_keys_irq_isr(int irq, void *dev_id)
{
	struct gpio_button_data *bdata = dev_id;
	struct input_dev *input = bdata->input;
	unsigned long flags;

	BUG_ON(irq != bdata->irq);
	DBG("func = %s,lqbjoystick irq = %d\n",__func__,irq);

	spin_lock_irqsave(&bdata->lock, flags);
	if (!bdata->key_pressed) {
		if (bdata->button->wakeup)
			pm_wakeup_event(bdata->input->dev.parent, 0);

		input_event(input, EV_KEY, *bdata->code, 1);
		input_sync(input);

		if (!bdata->release_delay) {
			input_event(input, EV_KEY, *bdata->code, 0);
			input_sync(input);
			goto out;
		}

		bdata->key_pressed = true;
	}

	if (bdata->release_delay)
		mod_timer(&bdata->release_timer,
			jiffies + msecs_to_jiffies(bdata->release_delay));
out:
	spin_unlock_irqrestore(&bdata->lock, flags);
	return IRQ_HANDLED;
}

static void gpio_keys_quiesce_key(void *data)
{
	struct gpio_button_data *bdata = data;
	if (bdata->gpiod)
		cancel_delayed_work_sync(&bdata->work);
	else
		del_timer_sync(&bdata->release_timer);
}

static int gpio_keys_setup_key(struct platform_device *pdev,
				struct input_dev *input,
				struct gpio_keys_drvdata *ddata,
				const struct gpio_keys_button *button,
				int idx,
				struct fwnode_handle *child)
{
	const char *desc = button->desc ? button->desc : "gpio_keys";
	struct device *dev = &pdev->dev;
	struct gpio_button_data *bdata = &ddata->data[idx];
	irq_handler_t isr;
	unsigned long irqflags;
	int irq;
	int error;

	bdata->input = input;
	bdata->button = button;
	spin_lock_init(&bdata->lock);

	if (child) {
		bdata->gpiod = devm_fwnode_gpiod_get(dev, child,
						     NULL, GPIOD_IN, desc);
		if (IS_ERR(bdata->gpiod)) {
			error = PTR_ERR(bdata->gpiod);
			if (error == -ENOENT) {
				/*
				 * GPIO is optional, we may be dealing with
				 * purely interrupt-driven setup.
				 */
				bdata->gpiod = NULL;
			} else {
				if (error != -EPROBE_DEFER)
					dev_err(dev, "failed to get gpio: %d\n",
						error);
				return error;
			}
		}
	} else if (gpio_is_valid(button->gpio)) {
		/*
		 * Legacy GPIO number, so request the GPIO here and
		 * convert it to descriptor.
		 */
		unsigned flags = GPIOF_IN;

		if (button->active_low)
			flags |= GPIOF_ACTIVE_LOW;

		error = devm_gpio_request_one(dev, button->gpio, flags, desc);
		if (error < 0) {
			dev_err(dev, "Failed to request GPIO %d, error %d\n",
				button->gpio, error);
			return error;
		}

		bdata->gpiod = gpio_to_desc(button->gpio);
		if (!bdata->gpiod)
			return -EINVAL;
	}

	if (bdata->gpiod) {
		bool active_low = gpiod_is_active_low(bdata->gpiod);

		if (button->debounce_interval) {
			error = gpiod_set_debounce(bdata->gpiod,
					button->debounce_interval * 1000);
			/* use timer if gpiolib doesn't provide debounce */
			if (error < 0)
				bdata->software_debounce =
						button->debounce_interval;
		}

		if (button->irq) {
			bdata->irq = button->irq;
		} else {
			irq = gpiod_to_irq(bdata->gpiod);
			
			if (irq < 0) {
				error = irq;
				dev_err(dev,
					"Unable to get irq number for GPIO %d, error %d\n",
					button->gpio, error);
				return error;
			}
			bdata->irq = irq;
		}

		INIT_DELAYED_WORK(&bdata->work, gpio_keys_gpio_work_func);

		isr = gpio_keys_gpio_isr;
		irqflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;

		switch (button->wakeup_event_action) {
		case EV_ACT_ASSERTED:
			bdata->wakeup_trigger_type = active_low ?
				IRQ_TYPE_EDGE_FALLING : IRQ_TYPE_EDGE_RISING;
			break;
		case EV_ACT_DEASSERTED:
			bdata->wakeup_trigger_type = active_low ?
				IRQ_TYPE_EDGE_RISING : IRQ_TYPE_EDGE_FALLING;
			break;
		case EV_ACT_ANY:
		default:
			/*
			 * For other cases, we are OK letting suspend/resume
			 * not reconfigure the trigger type.
			 */
			break;
		}
	} else {
		if (!button->irq) {
			dev_err(dev, "Found button without gpio or irq\n");
			return -EINVAL;
		}

		bdata->irq = button->irq;

		if (button->type && button->type != EV_KEY) {
			dev_err(dev, "Only EV_KEY allowed for IRQ buttons.\n");
			return -EINVAL;
		}

		bdata->release_delay = button->debounce_interval;
		timer_setup(&bdata->release_timer, gpio_keys_irq_timer, 0);

		isr = gpio_keys_irq_isr;
		irqflags = 0;

		/*
		 * For IRQ buttons, there is no interrupt for release.
		 * So we don't need to reconfigure the trigger type for wakeup.
		 */
	}

	bdata->code = &ddata->keymap[idx];
	*bdata->code = button->code;
	input_set_capability(input, button->type ?: EV_KEY, *bdata->code);

	/*
	 * Install custom action to cancel release timer and
	 * workqueue item.
	 */
	error = devm_add_action(dev, gpio_keys_quiesce_key, bdata);
	if (error) {
		dev_err(dev, "failed to register quiesce action, error: %d\n",
			error);
		return error;
	}

	/*
	 * If platform has specified that the button can be disabled,
	 * we don't want it to share the interrupt line.
	 */
	if (!button->can_disable)
		irqflags |= IRQF_SHARED;

	error = devm_request_any_context_irq(dev, bdata->irq, isr, irqflags,
					     desc, bdata);
	if (error < 0) {
		dev_err(dev, "Unable to claim irq %d; error %d\n",
			bdata->irq, error);
		return error;
	}

	return 0;
}

static void gpio_keys_report_state(struct gpio_keys_drvdata *ddata)
{
	struct input_dev *input = ddata->input;
	int i;

	for (i = 0; i < ddata->pdata->nbuttons; i++) {
		struct gpio_button_data *bdata = &ddata->data[i];
		if (bdata->gpiod)
			gpio_keys_gpio_report_event(bdata);
	}
	input_sync(input);
}

static int gpio_keys_open(struct input_dev *input)
{
	struct gpio_keys_drvdata *ddata = input_get_drvdata(input);
	const struct gpio_keys_platform_data *pdata = ddata->pdata;
	int error;

	if (pdata->enable) {
		error = pdata->enable(input->dev.parent);
		if (error)
			return error;
	}

	/* Report current state of buttons that are connected to GPIOs */
	gpio_keys_report_state(ddata);

	return 0;
}

static void gpio_keys_close(struct input_dev *input)
{
	struct gpio_keys_drvdata *ddata = input_get_drvdata(input);
	const struct gpio_keys_platform_data *pdata = ddata->pdata;

	if (pdata->disable)
		pdata->disable(input->dev.parent);
}

/*
 * Handlers for alternative sources of platform_data
 */

/*
 * Translate properties into platform_data
 */
static struct gpio_keys_platform_data *
gpio_keys_get_devtree_pdata(struct device *dev)
{
	struct gpio_keys_platform_data *pdata;
	struct gpio_keys_button *button;
	struct fwnode_handle *child;
	int nbuttons;

	nbuttons = device_get_child_node_count(dev);
	if (nbuttons == 0)
		return ERR_PTR(-ENODEV);

	pdata = devm_kzalloc(dev,
			     sizeof(*pdata) + nbuttons * sizeof(*button),
			     GFP_KERNEL);
	if (!pdata)
		return ERR_PTR(-ENOMEM);

	button = (struct gpio_keys_button *)(pdata + 1);

	pdata->buttons = button;
	pdata->nbuttons = nbuttons;

	pdata->rep = device_property_read_bool(dev, "autorepeat");

	device_property_read_string(dev, "label", &pdata->name);

	device_for_each_child_node(dev, child) {
		if (is_of_node(child))
			button->irq =
				irq_of_parse_and_map(to_of_node(child), 0);

		if (fwnode_property_read_u32(child, "linux,code",
					     &button->code)) {
			dev_err(dev, "Button without keycode\n");
			fwnode_handle_put(child);
			return ERR_PTR(-EINVAL);
		}

		fwnode_property_read_string(child, "label", &button->desc);

		if (fwnode_property_read_u32(child, "linux,input-type",
					     &button->type))
			button->type = EV_KEY;

		button->wakeup =
			fwnode_property_read_bool(child, "wakeup-source") ||
			/* legacy name */
			fwnode_property_read_bool(child, "gpio-key,wakeup");

		fwnode_property_read_u32(child, "wakeup-event-action",
					 &button->wakeup_event_action);

		button->can_disable =
			fwnode_property_read_bool(child, "linux,can-disable");

		if (fwnode_property_read_u32(child, "debounce-interval",
					 &button->debounce_interval))
			button->debounce_interval = 5;

		button++;
	}

	return pdata;
}

static const struct of_device_id gpio_keys_of_match[] = {
	{ .compatible = "gf-keys", },
	{ },
};
MODULE_DEVICE_TABLE(of, gpio_keys_of_match);

static int gpio_keys_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	const struct gpio_keys_platform_data *pdata = dev_get_platdata(dev);
	struct fwnode_handle *child = NULL;
	struct gpio_keys_drvdata *ddata;
	struct input_dev *input;
	
	int value;
	enum iio_chan_type type;
	int i, error;
	int wakeup = 0;
	DBG("lqb func = %s\n",__func__);
	if (!pdata) {
		pdata = gpio_keys_get_devtree_pdata(dev);
		if (IS_ERR(pdata))
			return PTR_ERR(pdata);
	}

	ddata = devm_kzalloc(dev, struct_size(ddata, data, pdata->nbuttons),
			     GFP_KERNEL);
	if (!ddata) {
		dev_err(dev, "failed to allocate state\n");
		return -ENOMEM;
	}
	dev_set_drvdata(&pdev->dev,ddata);

	ddata->last_bra = 0;
	ddata->last_gas = 0;
	
	        //joystick x
	        ddata->channelx = devm_iio_channel_get(dev, "joystick_x");
	        if (IS_ERR(ddata->channelx))
		        return PTR_ERR(ddata->channelx);

	        if (!ddata->channelx->indio_dev)
		        return -ENXIO;

	        error = iio_get_channel_type(ddata->channelx, &type);
	        if (error < 0)
		        return error;
		        
	        if (type != IIO_VOLTAGE) {
		        dev_err(dev, "Incompatible channel type %d\n", type);
		        return -EINVAL;
	        }
	        
	////error = sysfs_create_group(&pdev->dev.kobj,&joystick_calibration_group_attrs);
	
	        //joystick y
		ddata->channely = devm_iio_channel_get(dev, "joystick_y");
		if (IS_ERR(ddata->channely))
			return PTR_ERR(ddata->channely);
	
		if (!ddata->channely->indio_dev)
			return -ENXIO;
	
		error = iio_get_channel_type(ddata->channely, &type);
		if (error < 0)
			return error;
			
		if (type != IIO_VOLTAGE) {
			dev_err(dev, "Incompatible channel type %d\n", type);
			return -EINVAL;
		}
	
		//joystick z
		ddata->channelz = devm_iio_channel_get(dev, "joystick_z");
		if (IS_ERR(ddata->channelz))
			return PTR_ERR(ddata->channelz);
	
		if (!ddata->channelz->indio_dev)
			return -ENXIO;
		
		error = iio_get_channel_type(ddata->channelz, &type);
		if (error < 0)
			return error;
			
		if (type != IIO_VOLTAGE) {
			dev_err(dev, "Incompatible channel type %d\n", type);
			return -EINVAL;
		}
	
	
		//joystick rz
		ddata->channelrz = devm_iio_channel_get(dev, "joystick_rz");
		if (IS_ERR(ddata->channelrz))
			return PTR_ERR(ddata->channelrz);
	
		if (!ddata->channelrz->indio_dev)
			return -ENXIO;
	
		error = iio_get_channel_type(ddata->channelrz, &type);
		if (error < 0)
			return error;
			
		if (type != IIO_VOLTAGE) {
			dev_err(dev, "Incompatible channel type %d\n", type);
			return -EINVAL;
		}



	ddata->keymap = devm_kcalloc(dev,
				     pdata->nbuttons, sizeof(ddata->keymap[0]),
				     GFP_KERNEL);
	if (!ddata->keymap)
		return -ENOMEM;

	input = devm_input_allocate_device(dev);
	if (!input) {
		dev_err(dev, "failed to allocate input device\n");
		return -ENOMEM;
	}
	error = input_setup_polling(input, adc_joystick_poll);
	if (error) {
		dev_err(dev, "Unable to set up polling: %d\n", error);
		return error;
	}
	if (!device_property_read_u32(dev, "poll-interval", &value))
	{
		input_set_poll_interval(input, value);
	}
	
	ddata->pdata = pdata;
	ddata->input = input;
	mutex_init(&ddata->disable_lock);

	platform_set_drvdata(pdev, ddata);
	input_set_drvdata(input, ddata);

//	input->name = "Microsoft X-Box 360 pad";
	input->name = "GameForce Controller";

	input->dev.parent = dev;
	input->open = gpio_keys_open;
	input->close = gpio_keys_close;

	input->id.bustype = BUS_HOST;
//	input->id.bustype = BUS_USB;

	input->id.vendor = 0x0003;
	input->id.product = 0x0003;

//	input->id.vendor = 0x045e;
//	input->id.product = 0x0032;
//	input->id.product = 0x0b13;

	input->id.version = 0x0110;

	input->keycode = ddata->keymap;
	input->keycodesize = sizeof(ddata->keymap[0]);
	input->keycodemax = pdata->nbuttons;
	
	__set_bit(EV_ABS, input->evbit);
	__set_bit(ABS_BRAKE, input->absbit); 
	__set_bit(ABS_GAS, input->absbit);
	__set_bit(ABS_HAT0Y, input->absbit);
	__set_bit(ABS_HAT0X, input->absbit);
	__set_bit(ABS_X, input->absbit);
	__set_bit(ABS_Y, input->absbit);
	__set_bit(ABS_RZ, input->absbit);
	__set_bit(ABS_Z, input->absbit);

	__set_bit(BTN_TL2, input->keybit);
	__set_bit(BTN_TR2, input->keybit);
	
	input_set_abs_params(input, ABS_X, 0, 255, 0, 15);
	input_set_abs_params(input, ABS_Y, 0, 255, 0, 15);
	input_set_abs_params(input, ABS_Z, 0, 255, 0, 15);
	input_set_abs_params(input, ABS_RZ, 0, 255, 0, 15);
	input_set_abs_params(input, ABS_BRAKE, 0, 255, 0, 15);
	input_set_abs_params(input, ABS_GAS, 0, 255, 0, 15);
	//0xff :  up  , 0x01 : down
	input_set_abs_params(input, ABS_HAT0Y, 0, 255, 0, 0);
	//0xff : left , 0x01 : right
	input_set_abs_params(input, ABS_HAT0X, 0, 255, 0, 0);

	/* Enable auto repeat feature of Linux input subsystem */
	if (pdata->rep)
		__set_bit(EV_REP, input->evbit);

	for (i = 0; i < pdata->nbuttons; i++) {
		const struct gpio_keys_button *button = &pdata->buttons[i];

		if (!dev_get_platdata(dev)) {
			child = device_get_next_child_node(dev, child);
			if (!child) {
				dev_err(dev,
					"missing child device node for entry %d\n",
					i);
				return -EINVAL;
			}
		}

		error = gpio_keys_setup_key(pdev, input, ddata,
					    button, i, child);
		if (error) {
			fwnode_handle_put(child);
			return error;
		}

		if (button->wakeup)
			wakeup = 1;
	}

	fwnode_handle_put(child);

	error = input_register_device(input);
	if (error) {
		dev_err(dev, "Unable to register input device, error: %d\n",
			error);
		return error;
	}

	device_init_wakeup(dev, wakeup);

	return 0;
}

static int __maybe_unused
gpio_keys_button_enable_wakeup(struct gpio_button_data *bdata)
{
	int error;

	error = enable_irq_wake(bdata->irq);
	if (error) {
		dev_err(bdata->input->dev.parent,
			"failed to configure IRQ %d as wakeup source: %d\n",
			bdata->irq, error);
		return error;
	}

	if (bdata->wakeup_trigger_type) {
		error = irq_set_irq_type(bdata->irq,
					 bdata->wakeup_trigger_type);
		if (error) {
			dev_err(bdata->input->dev.parent,
				"failed to set wakeup trigger %08x for IRQ %d: %d\n",
				bdata->wakeup_trigger_type, bdata->irq, error);
			disable_irq_wake(bdata->irq);
			return error;
		}
	}

	return 0;
}

static void __maybe_unused
gpio_keys_button_disable_wakeup(struct gpio_button_data *bdata)
{
	int error;

	/*
	 * The trigger type is always both edges for gpio-based keys and we do
	 * not support changing wakeup trigger for interrupt-based keys.
	 */
	if (bdata->wakeup_trigger_type) {
		error = irq_set_irq_type(bdata->irq, IRQ_TYPE_EDGE_BOTH);
		if (error)
			dev_warn(bdata->input->dev.parent,
				 "failed to restore interrupt trigger for IRQ %d: %d\n",
				 bdata->irq, error);
	}

	error = disable_irq_wake(bdata->irq);
	if (error)
		dev_warn(bdata->input->dev.parent,
			 "failed to disable IRQ %d as wake source: %d\n",
			 bdata->irq, error);
}

static int __maybe_unused
gpio_keys_enable_wakeup(struct gpio_keys_drvdata *ddata)
{
	struct gpio_button_data *bdata;
	int error;
	int i;

	for (i = 0; i < ddata->pdata->nbuttons; i++) {
		bdata = &ddata->data[i];
		if (bdata->button->wakeup) {
			error = gpio_keys_button_enable_wakeup(bdata);
			if (error)
				goto err_out;
		}
		bdata->suspended = true;
	}

	return 0;

err_out:
	while (i--) {
		bdata = &ddata->data[i];
		if (bdata->button->wakeup)
			gpio_keys_button_disable_wakeup(bdata);
		bdata->suspended = false;
	}

	return error;
}

static void __maybe_unused
gpio_keys_disable_wakeup(struct gpio_keys_drvdata *ddata)
{
	struct gpio_button_data *bdata;
	int i;

	for (i = 0; i < ddata->pdata->nbuttons; i++) {
		bdata = &ddata->data[i];
		bdata->suspended = false;
		if (irqd_is_wakeup_set(irq_get_irq_data(bdata->irq)))
			gpio_keys_button_disable_wakeup(bdata);
	}
}

static int __maybe_unused gpio_keys_suspend(struct device *dev)
{
	struct gpio_keys_drvdata *ddata = dev_get_drvdata(dev);
	struct input_dev *input = ddata->input;
	int error;

	if (device_may_wakeup(dev)) {
		error = gpio_keys_enable_wakeup(ddata);
		if (error)
			return error;
	} else {
		mutex_lock(&input->mutex);
		if (input->users)
			gpio_keys_close(input);
		mutex_unlock(&input->mutex);
	}

	return 0;
}

static int __maybe_unused gpio_keys_resume(struct device *dev)
{
	struct gpio_keys_drvdata *ddata = dev_get_drvdata(dev);
	struct input_dev *input = ddata->input;
	int error = 0;

	if (device_may_wakeup(dev)) {
		gpio_keys_disable_wakeup(ddata);
	} else {
		mutex_lock(&input->mutex);
		if (input->users)
			error = gpio_keys_open(input);
		mutex_unlock(&input->mutex);
	}

	if (error)
		return error;

	gpio_keys_report_state(ddata);
	return 0;
}

static SIMPLE_DEV_PM_OPS(gpio_keys_pm_ops, gpio_keys_suspend, gpio_keys_resume);

static void gpio_keys_shutdown(struct platform_device *pdev)
{
	int ret;

	ret = gpio_keys_suspend(&pdev->dev);
	if (ret)
		dev_err(&pdev->dev, "failed to shutdown\n");
}

static struct platform_driver gf_keys_device_driver = {
	.probe		= gpio_keys_probe,
	.shutdown	= gpio_keys_shutdown,
	.driver		= {
		.name	= "gf-keys",
		.pm	= &gpio_keys_pm_ops,
		.of_match_table = gpio_keys_of_match,
		.dev_groups	= gf_keys_groups,
	}
};

static int __init gf_keys_init(void)
{
	return platform_driver_register(&gf_keys_device_driver);
}

static void __exit gf_keys_exit(void)
{
	platform_driver_unregister(&gf_keys_device_driver);
}

late_initcall(gf_keys_init);
module_exit(gf_keys_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Phil Blundell <pb@handhelds.org>");
MODULE_DESCRIPTION("Keyboard driver for GPIOs");
MODULE_ALIAS("platform:gpio-keys");

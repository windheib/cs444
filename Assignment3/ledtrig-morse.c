/*
 * LED Morse Trigger
 *
 * Written by Ben Windheim, Kyle Baldes, Burton Jaursch
 *
 * Oregon State University, Operating Systems II, Fall 2018
 *
 * Based on Richard Purdie's ledtrig-timer.c, ledtrig-heartbeat.c and some arch's
 * CONFIG_morse code.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/sched/loadavg.h>
#include <linux/leds.h>
#include <linux/reboot.h>
#include "../leds.h"

static int panic_morses;

struct morse_trig_data {
	unsigned int phase;
	unsigned int period;
	struct timer_list timer;
	unsigned int speed;
	int count;
	char* morse_str;
};

static const char* start_str = "HELLO";

static const char* hard_coded_str[28] = { 
			"10111000", "111010101000", "11101011101000",	/* A-C */
			"1110101000", "1000", "101011101000", /* D-F */
			"111011101000", "1010101000", "101000", "1011101110111000", /* F-J */
			"11101011100", "101110101000", "1110111000", "11101000", "11101110111000", /* K-O */
			"10111011101000", "1110111010111000", "1011101000", "10101000", /* P-S */
			"111000", "1010111000", "101010111000", "101110111000", /* T-W */
			"11101010111000", "1110101110111000", "11101110101000", /* X-Z */
			"0000", /* space */ "000000000002" /* end transmission */
};


// Converting string into concatenated string of 1's and zeros


static char* convert_str(const char* input_str){
	int len_input, i, new_str_len; 
	char* entire_str; 

	len_input = strlen(input_str);
	
	// Loop through the input string 
	new_str_len = 0;
	for(i = 0; i < len_input; i++)
	{
		// Sums up the number of characters in each letter of the string 
		new_str_len = new_str_len + strlen(hard_coded_str[(input_str[i]-65)]);
	}
	// Need to use kmalloc() to allocate space for long string here
	// allocate the length of all the characters plus 1 (NULL terminator)
	entire_str =(char*)kmalloc(new_str_len+1, GFP_KERNEL);
	
	// The next loop actually creates the string by concatenating the next letter
	for(i = 0; i < len_input; i++)
	{
		strcat(entire_str, hard_coded_str[(input_str[i]-65)]/*, strlen(hard_coded_str[(input_str[i]-65)])*/);
	}
	strcat(entire_str, hard_coded_str[27]);
	
	return entire_str;
}


static void led_morse_function(unsigned long data)
{
	struct led_classdev *led_cdev = (struct led_classdev *) data;
	struct morse_trig_data *morse_data = led_cdev->trigger_data;
	unsigned long brightness = LED_OFF;
	unsigned long delay = 0;
	
	delay =  msecs_to_jiffies(140) / morse_data->speed;
	printk(KERN_DEBUG "Morse_str: %s", morse_data->morse_str);
	printk(KERN_DEBUG "Iteration num: %d", morse_data->count);
	if(morse_data->morse_str[morse_data->count] == '1') {
		brightness = 255;
	} else if(morse_data->morse_str[morse_data->count] == '2') {
		morse_data->count = 0;	// reset
	} else {
		brightness = LED_OFF;
	}
	
	morse_data->count++;
	led_set_brightness_nosleep(led_cdev, brightness);
	mod_timer(&morse_data->timer, jiffies + delay);
}
/*
* 
*
*
*
*/
static ssize_t led_speed_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct morse_trig_data *morse_data = led_cdev->trigger_data;

	return sprintf(buf, "%u\n", morse_data->speed);
}



/*
* 
*
*
*
*/
static ssize_t led_speed_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct morse_trig_data *morse_data = led_cdev->trigger_data;
	unsigned long state;
	int ret;

	ret = kstrtoul(buf, 0, &state);
	if (ret)
		return ret;

	morse_data->speed = ((state - 1) % 4) + 1;
	return size;
}

static DEVICE_ATTR(speed, 0644, led_speed_show, led_speed_store);

static void morse_trig_activate(struct led_classdev *led_cdev)
{
	struct morse_trig_data *morse_data;
	int rc;

	morse_data = kzalloc(sizeof(*morse_data), GFP_KERNEL);
	if (!morse_data)
		return;

	led_cdev->trigger_data = morse_data;
	rc = device_create_file(led_cdev->dev, &dev_attr_speed);
	if (rc) {
		kfree(led_cdev->trigger_data);
		return;
	}

	setup_timer(&morse_data->timer,
		    led_morse_function, (unsigned long) led_cdev);
	morse_data->phase = 0;
	// Create string with the provided value
	// Provided value should come from morse "file" or hardcoded global  
	// Strings must be ALL CAPS
	
	morse_data->count = 0;
	morse_data->morse_str = convert_str(start_str);
	morse_data->speed = 1; 
	
	if (!led_cdev->blink_brightness)
		led_cdev->blink_brightness = led_cdev->max_brightness;
	led_morse_function(morse_data->timer.data);
	set_bit(LED_BLINK_SW, &led_cdev->work_flags);
	led_cdev->activated = true;
}


static void morse_trig_deactivate(struct led_classdev *led_cdev)
{
	struct morse_trig_data *morse_data = led_cdev->trigger_data;

	if (led_cdev->activated) {
		del_timer_sync(&morse_data->timer);
		device_remove_file(led_cdev->dev, &dev_attr_speed);
		if(morse_data->morse_str) {
			kfree(morse_data->morse_str);
		}
		kfree(morse_data);
		clear_bit(LED_BLINK_SW, &led_cdev->work_flags);
		led_cdev->activated = false;
	}
}

static struct led_trigger morse_led_trigger = {
	.name     = "morse",
	.activate = morse_trig_activate,
	.deactivate = morse_trig_deactivate,
};

static int morse_reboot_notifier(struct notifier_block *nb,
				     unsigned long code, void *unused)
{
	led_trigger_unregister(&morse_led_trigger);
	return NOTIFY_DONE;
}

static int morse_panic_notifier(struct notifier_block *nb,
				     unsigned long code, void *unused)
{
	panic_morses = 1;
	return NOTIFY_DONE;
}

static struct notifier_block morse_reboot_nb = {
	.notifier_call = morse_reboot_notifier,
};

static struct notifier_block morse_panic_nb = {
	.notifier_call = morse_panic_notifier,
};

static int __init morse_trig_init(void)
{
	int rc = led_trigger_register(&morse_led_trigger);
		
	if (!rc) {
		atomic_notifier_chain_register(&panic_notifier_list,
					       &morse_panic_nb);
		register_reboot_notifier(&morse_reboot_nb);
	}
	return rc;
}

static void __exit morse_trig_exit(void)
{
	unregister_reboot_notifier(&morse_reboot_nb);
	atomic_notifier_chain_unregister(&panic_notifier_list,
					 &morse_panic_nb);
	led_trigger_unregister(&morse_led_trigger);
}

module_init(morse_trig_init);
module_exit(morse_trig_exit);

MODULE_AUTHOR("Ben Windheim, Kyle Baldes, Burton Jaursch");
MODULE_DESCRIPTION("Morse LED trigger");
MODULE_LICENSE("GPL");

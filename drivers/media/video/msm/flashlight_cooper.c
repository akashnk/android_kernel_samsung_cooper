/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 * Copyright (c) 2011, Ketut P. Kumajaya <ketut.kumajaya@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */
 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <asm/uaccess.h>
#include <linux/unistd.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/major.h>
#include <linux/input.h>
#include <mach/gpio.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/leds.h>

#define CAM_FLASH_ENSET 1
#define CAM_FLASH_FLEN 2

static int state = 0;

static void cooper_flashlight_set(struct led_classdev *led_cdev,
	enum led_brightness value)
{
	int i = 0;

	printk("[FLASHLIGHT] %s: %d\n", __func__, value);

	if (value != state)
	{
		gpio_set_value(CAM_FLASH_ENSET,0); 
		gpio_set_value(CAM_FLASH_FLEN,0);
		mdelay(1);
		if (value)
		{
			for(i=0;i<8;i++)
			{
				udelay(1);
				gpio_set_value(CAM_FLASH_ENSET,1);
				udelay(1);
				gpio_set_value(CAM_FLASH_ENSET,0);
			}
			gpio_set_value(CAM_FLASH_ENSET,1);
		}
		state = value;
	}
}

static struct led_classdev cooper_flashlight = {
	.name			= "flashlight",
	.brightness_set		= cooper_flashlight_set,
	.brightness		= 0,
	.max_brightness		= 16,
};

static int cooper_flashlight_probe(struct platform_device *pdev)
{
	int rc;

	printk("[FLASHLIGHT] %s\n", __func__);
	rc = led_classdev_register(&pdev->dev, &cooper_flashlight);
	if (rc)
	{
		dev_err(&pdev->dev, "unable to register flashlight class driver\n");
		return rc;
	}

	cooper_flashlight_set(&cooper_flashlight, 0);
	return rc;
}

static int __devexit cooper_flashlight_remove(struct platform_device *pdev)
{
	led_classdev_unregister(&cooper_flashlight);

	return 0;
}

#ifdef CONFIG_PM
static int cooper_flashlight_suspend(struct platform_device *dev,
		pm_message_t pm_state)
{
	if (!state)
	{
		// cooper_flashlight_set(&cooper_flashlight, 0);
		led_classdev_suspend(&cooper_flashlight);
	}

	return 0;
}

static int cooper_flashlight_resume(struct platform_device *dev)
{
	led_classdev_resume(&cooper_flashlight);

	return 0;
}
#else
#define cooper_flashlight_suspend NULL
#define cooper_flashlight_resume NULL
#endif

static struct platform_driver cooper_flashlight_driver = {
	.probe		= cooper_flashlight_probe,
	.remove		= __devexit_p(cooper_flashlight_remove),
	.suspend	= cooper_flashlight_suspend,
	.resume		= cooper_flashlight_resume,
	.driver		= {
		.name	= "flashlight-leds",
		.owner	= THIS_MODULE,
	},
};

static int cooper_flashlight_init(void)
{
	printk("[FLASHLIGHT] %s\n", __func__);
        return platform_driver_register(&cooper_flashlight_driver);
}

static void cooper_flashlight_exit(void)
{
        platform_driver_unregister(&cooper_flashlight_driver);
}

module_init(cooper_flashlight_init);
module_exit(cooper_flashlight_exit);

MODULE_DESCRIPTION("MSM flashlight driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:flashlight-leds");


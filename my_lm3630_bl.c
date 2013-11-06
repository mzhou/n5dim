/*
 * Copyright (c) 2013 LGE Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "my_lm3630_bl.h"

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/platform_data/lm3630_bl.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/backlight.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/of_gpio.h>
#include <linux/debugfs.h>

#ifdef CONFIG_MACH_LGE
/* HACK: disable fb notifier unless off-mode charge */
#include <mach/board_lge.h>
#endif

#define I2C_BL_NAME   "lm3630"

/* Register definitions */
#define CONTROL_REG        0x00
#define CONFIG_REG         0x01
#define BOOST_CTL_REG      0x02
#define BRIGHTNESS_A_REG   0x03
#define BRIGHTNESS_B_REG   0x04
#define CURRENT_A_REG      0x05
#define CURRENT_B_REG      0x06
#define ON_OFF_RAMP_REG    0x07
#define RUN_RAMP_REG       0x08
#define IRQ_STATUS_REG     0x09
#define IRQ_ENABLE_REG     0x0A
#define FAULT_STAT_REG     0x0B
#define SOFT_RESET_REG     0x0F
#define PWM_OUT_REG        0x12
#define REVISION_REG       0x1F

#define SLEEP_CMD_MASK     0x80
#define LINEAR_A_MASK      0x10
#define LINEAR_B_MASK      0x08
#define LED_A_EN_MASK      0x04
#define LED_B_EN_MASK      0x02
#define LED2_ON_A_MASK     0x01
#define FEEDBACK_B_MASK    0x10
#define FEEDBACK_A_MASK    0x08
#define PWM_EN_B_MASK      0x02
#define PWM_EN_A_MASK      0x01

#define DEFAULT_CTRL_REG   0xC0
#define DEFAULT_CFG_REG    0x18

#define BL_OFF 0x00

enum {
	LED_BANK_A,
	LED_BANK_B,
	LED_BANK_AB,
};

struct lm3630_device {
	struct i2c_client *client;
	struct backlight_device *bl_dev;
	struct dentry  *dent;
	int en_gpio;
	int boost_ctrl_reg;
	int ctrl_reg;
	int bank_sel;
	int cfg_reg;
	int linear_map;
	int max_current;
	int min_brightness;
	int max_brightness;
	int default_brightness;
	int pwm_enable;
	int blmap_size;
	char *blmap;
};

struct debug_reg {
	char  *name;
	u8  reg;
};

#include "lm3630_bl_ksyms.h"

static void my_lm3630_hw_reset(struct lm3630_device *dev)
{
	if (gpio_is_valid(dev->en_gpio)) {
		ksym_gpio_set_value_cansleep(dev->en_gpio, 1);
		mdelay(1);
	} else {
		pr_err("%s: en_gpio is not valid !!\n", __func__);
	}
}

static int my_lm3630_write_reg(struct i2c_client *client, u8 reg, u8 val)
{
	int ret;

	pr_debug("%s: reg=0x%x\n", __func__, reg);
	ret = i2c_smbus_write_byte_data(client, reg, val);
	if (ret < 0)
		pr_err("%s: i2c error! addr = 0x%x\n", __func__, reg);

	return ret;
}

static void my_lm3630_set_brightness_reg(struct lm3630_device *dev, int level)
{
	if (dev->bank_sel == LED_BANK_A) {
		my_lm3630_write_reg(dev->client, BRIGHTNESS_A_REG, level);
	} else if (dev->bank_sel == LED_BANK_B) {
		my_lm3630_write_reg(dev->client, BRIGHTNESS_B_REG, level);
	} else {
		my_lm3630_write_reg(dev->client, BRIGHTNESS_A_REG, level);
		my_lm3630_write_reg(dev->client, BRIGHTNESS_B_REG, level);
	}
}

static void my_lm3630_set_max_current_reg(struct lm3630_device *dev, int val)
{
	if (dev->bank_sel == LED_BANK_A) {
		my_lm3630_write_reg(dev->client, CURRENT_A_REG, val);
	} else if (dev->bank_sel == LED_BANK_B) {
		my_lm3630_write_reg(dev->client, CURRENT_B_REG, val);
	} else {
		my_lm3630_write_reg(dev->client, CURRENT_A_REG, val);
		my_lm3630_write_reg(dev->client, CURRENT_B_REG, val);
	}
}

static void my_lm3630_set_main_current_level(struct i2c_client *client, int level)
{
	struct lm3630_device *dev = i2c_get_clientdata(client);

	mutex_lock(ksym_backlight_mtx);
	dev->bl_dev->props.brightness = level;
	if (level == 0) {
		my_lm3630_write_reg(client, CONTROL_REG, BL_OFF);
	} else if (level == 1) {
		my_lm3630_set_max_current_reg(dev, 0);
		my_lm3630_set_brightness_reg(dev, 1);
	} else {
		if (level < dev->min_brightness) {
			level = dev->min_brightness;
		} else if (level > dev->max_brightness) {
			level = dev->max_brightness;
		}

		my_lm3630_set_max_current_reg(dev, dev->max_current);

		if (dev->blmap) {
			if (level < dev->blmap_size) {
				my_lm3630_set_brightness_reg(dev,
						dev->blmap[level]);
			} else {
				pr_err("%s: invalid index %d:%d\n",
						__func__,
						dev->blmap_size,
						level);
			}
		} else {
			my_lm3630_set_brightness_reg(dev, level);
		}
	}
	mutex_unlock(ksym_backlight_mtx);
	pr_debug("%s: level=%d\n", __func__, level);
}

static void my_lm3630_hw_init(struct lm3630_device *dev)
{
	my_lm3630_hw_reset(dev);
	my_lm3630_write_reg(dev->client, BOOST_CTL_REG, dev->boost_ctrl_reg);
	my_lm3630_write_reg(dev->client, CONFIG_REG, dev->cfg_reg);
	my_lm3630_write_reg(dev->client, CONTROL_REG, dev->ctrl_reg);
	mdelay(1);
}

static void my_lm3630_backlight_on(struct lm3630_device *dev, int level)
{
	if (dev->bl_dev->props.brightness == 0 && level != 0) {
		my_lm3630_hw_init(dev);
		pr_info("%s\n", __func__);
	}
	my_lm3630_set_main_current_level(dev->client, level);
}

static void my_lm3630_backlight_off(struct lm3630_device *dev)
{
	if (dev->bl_dev->props.brightness == 0)
		return;

	my_lm3630_set_main_current_level(dev->client, 0);
	ksym_gpio_set_value_cansleep(dev->en_gpio, 0);
	udelay(1);
	pr_info("%s\n", __func__);
}

void my_lm3630_lcd_backlight_set_level(int level)
{
	if (!ksym_v_lm3630_dev) {
		pr_warn("%s: No device\n", __func__);
		return;
	}

	pr_debug("%s: level=%d\n", __func__, level);

	if (level)
		my_lm3630_backlight_on(ksym_v_lm3630_dev, level);
	else
		my_lm3630_backlight_off(ksym_v_lm3630_dev);
}

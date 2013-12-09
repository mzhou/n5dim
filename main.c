/*
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

#include <linux/module.h>
#include <linux/kallsyms.h>

#include "hook.h"
#include "my_lm3630_bl.h"

// Lookups
unsigned long ksym_backlight_mtx;
unsigned long ksym_gpio_set_value_cansleep;
unsigned long ksym_lm3630_set_main_current_level;
unsigned long ksym_mem_text_address_restore;
unsigned long ksym_mem_text_address_writeable;
unsigned long ksym_mem_text_writeable_spinlock;
unsigned long ksym_mem_text_writeable_spinunlock;

// Local
static uintptr_t backup_lm3630_set_main_current_level[2];

static void set_main_current_level_proxy(struct i2c_client *client, int level);

static int __init n5dim_init(void)
{
	pr_info("n5dim: Hello!\n");

	ksym_gpio_set_value_cansleep = kallsyms_lookup_name(
			"gpio_set_value_cansleep");
	pr_info("n5dim: ksym_gpio_set_value_cansleep == 0x%08lx\n",
			ksym_gpio_set_value_cansleep);
	if (!ksym_gpio_set_value_cansleep) return -EFAULT;

	ksym_lm3630_set_main_current_level = kallsyms_lookup_name(
			"lm3630_set_main_current_level");
	pr_info("n5dim: ksym_lm3630_set_main_current_level == 0x%08lx\n",
			ksym_lm3630_set_main_current_level);
	if (!ksym_lm3630_set_main_current_level) return -EFAULT;

	ksym_mem_text_address_restore = kallsyms_lookup_name(
			"mem_text_address_restore");
	pr_info("n5dim: ksym_mem_text_address_restore == 0x%08lx\n",
			ksym_mem_text_address_restore);
	ksym_mem_text_address_writeable = kallsyms_lookup_name(
			"mem_text_address_writeable");
	pr_info("n5dim: ksym_mem_text_address_writeable == 0x%08lx\n",
			ksym_mem_text_address_writeable);
	ksym_mem_text_writeable_spinlock = kallsyms_lookup_name(
			"mem_text_writeable_spinlock");
	pr_info("n5dim: ksym_mem_text_writeable_spinlock == 0x%08lx\n",
			ksym_mem_text_writeable_spinlock);
	ksym_mem_text_writeable_spinunlock = kallsyms_lookup_name(
			"mem_text_writeable_spinunlock");
	pr_info("n5dim: ksym_mem_text_writeable_spinunlock == 0x%08lx\n",
			ksym_mem_text_writeable_spinunlock);

	ksym_backlight_mtx = kallsyms_lookup_name("backlight_mtx");
	pr_info("n5dim: ksym_backlight_mtx == 0x%08lx\n", ksym_backlight_mtx);
	if (!ksym_backlight_mtx) {
		pr_warn("n5dim: kallsyms_lookup_name(\"backlight_mtx\")"
				" failed\n");
	}

	if (hook_and_backup((void*) ksym_lm3630_set_main_current_level,
			backup_lm3630_set_main_current_level,
			set_main_current_level_proxy)) return -EFAULT;

	pr_info("n5dim: Done!\n");

	return 0;
}

static void __exit n5dim_exit(void)
{
        unhook_and_restore((void*) ksym_lm3630_set_main_current_level,
                        backup_lm3630_set_main_current_level);
        pr_info("n5dim: Bye!\n");
}

module_init(n5dim_init);
module_exit(n5dim_exit);

static bool enable = true;
MODULE_PARM_DESC(enable, "Change any behaviour at all");
module_param(enable, bool, 0644);

static bool old = false;
MODULE_PARM_DESC(old, "Only change minimum, not other levels");
module_param(old, bool, 0644);

static void set_main_current_level_proxy(struct i2c_client *client, int level)
{
	if (!enable) my_lm3630_orig_set_main_current_level(client, level);
	else if (old) my_lm3630_set_main_current_level(client, level);
	else my_lm3630_new_set_main_current_level(client, level);
}

MODULE_DESCRIPTION("Nexus 5 Screen Dimmer");
MODULE_AUTHOR("Michael Zhou <mzhou@cse.unsw.edu.au>");
MODULE_LICENSE("GPL");

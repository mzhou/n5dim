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

// Comment to debug
#undef pr_emerg
#define pr_emerg(fmt, ...) \
         no_printk(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)

static int __init n5dim_init(void)
{
	pr_emerg("Hello!\n");

	ksym_gpio_set_value_cansleep = kallsyms_lookup_name(
			"gpio_set_value_cansleep");
	if (!ksym_gpio_set_value_cansleep) return -EFAULT;

	ksym_lm3630_set_main_current_level = kallsyms_lookup_name(
			"lm3630_set_main_current_level");
	if (!ksym_lm3630_set_main_current_level) return -EFAULT;

	ksym_mem_text_address_restore = kallsyms_lookup_name(
			"mem_text_address_restore");
	ksym_mem_text_address_writeable = kallsyms_lookup_name(
			"mem_text_address_writeable");
	ksym_mem_text_writeable_spinlock = kallsyms_lookup_name(
			"mem_text_writeable_spinlock");
	ksym_mem_text_writeable_spinunlock = kallsyms_lookup_name(
			"mem_text_writeable_spinunlock");

	ksym_backlight_mtx = kallsyms_lookup_name("backlight_mtx");
	if (!ksym_backlight_mtx) {
		pr_warn("kallsyms_lookup_name(\"backlight_mtx\") failed\n");
	}

	if (hook_and_backup((void*) ksym_lm3630_set_main_current_level,
			backup_lm3630_set_main_current_level,
			my_lm3630_set_main_current_level)) return -EFAULT;

	pr_emerg("Done!\n");

	return 0;
}

static void __exit n5dim_exit(void)
{
	unhook_and_restore((void*) ksym_lm3630_set_main_current_level,
			backup_lm3630_set_main_current_level);
	pr_emerg("Bye!\n");
}

module_init(n5dim_init);
module_exit(n5dim_exit);

MODULE_DESCRIPTION("Nexus 5 Screen Dimmer");
MODULE_AUTHOR("Michael Zhou <mzhou@cse.unsw.edu.au>");
MODULE_LICENSE("GPL");

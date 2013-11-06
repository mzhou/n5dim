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

static int __init n5dim_init(void)
{
	return 0;
}

static void __exit n5dim_exit(void)
{
}

module_init(n5dim_init);
module_exit(n5dim_exit);

MODULE_DESCRIPTION("Nexus 5 Screen Dimmer");
MODULE_AUTHOR("Michael Zhou <mzhou@cse.unsw.edu.au>");
MODULE_LICENSE("GPL");

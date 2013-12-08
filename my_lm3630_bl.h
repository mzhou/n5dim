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

#ifndef MY_LM3630_BL_H_
#define MY_LM3630_BL_H_

struct i2c_client;

void my_lm3630_set_main_current_level(struct i2c_client *client, int level);
void my_lm3630_new_set_main_current_level(
		struct i2c_client *client, int level);
void my_lm3630_orig_set_main_current_level(
		struct i2c_client *client, int level);

#endif // MY_LM3630_BL_H_

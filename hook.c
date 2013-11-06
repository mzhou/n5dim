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

#include "hook.h"

#include <asm/cacheflush.h>

#include "mm_ksyms.h"

// 0:	e51ff004	ldr	pc, [pc, #-4]	; 4 <trampoline+0x4>
#define TRAMPOLINE 0xe51ff004

int hook_and_backup(void *orig, uintptr_t *backup, void *target)
{
	unsigned long flags;

	if (ksym_mem_text_address_restore) {
		if (!ksym_mem_text_address_writeable
				|| !ksym_mem_text_writeable_spinlock
				|| !ksym_mem_text_writeable_spinunlock) {
			return -1;
		}
		ksym_mem_text_writeable_spinlock(&flags);
		ksym_mem_text_address_writeable((unsigned long) orig);
	}

	backup[0] = ((uintptr_t*) orig)[0];
	backup[1] = ((uintptr_t*) orig)[1];
	((uintptr_t*) orig)[0] = TRAMPOLINE;
	((uintptr_t*) orig)[1] = (uintptr_t) target;
	flush_icache_range((uintptr_t) orig,
			2 * sizeof(uintptr_t) + (uintptr_t) orig);

	if (ksym_mem_text_address_restore) {
		ksym_mem_text_address_restore();
		ksym_mem_text_writeable_spinunlock(&flags);
	}

	return 0;
}

void unhook_and_restore(void *orig, uintptr_t *backup)
{
	unsigned long flags;

	if (ksym_mem_text_address_restore) {
		ksym_mem_text_writeable_spinlock(&flags);
		ksym_mem_text_address_writeable((unsigned long) orig);
	}

	((uintptr_t*) orig)[0] = backup[0];
	((uintptr_t*) orig)[1] = backup[1];
	flush_icache_range((uintptr_t) orig,
			2 * sizeof(uintptr_t) + (uintptr_t) orig);

	if (ksym_mem_text_address_restore) {
		ksym_mem_text_address_restore();
		ksym_mem_text_writeable_spinunlock(&flags);
	}
}

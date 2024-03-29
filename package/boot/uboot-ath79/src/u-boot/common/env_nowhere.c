/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>

 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

#if defined(CFG_ENV_IS_NOWHERE) /* Environment is nowhere */

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>

DECLARE_GLOBAL_DATA_PTR;

env_t *env_ptr = NULL;
const char *env_name_spec = "NOWHERE";

extern uchar default_environment[];
extern int default_environment_size;

void env_relocate_spec(void){
}

int saveenv(void) {
    return 0;
}

uchar env_get_char_spec(int index){
	return(*((uchar *) (gd->env_addr + index)));
}

/************************************************************************
 * Initialize Environment use
 *
 * We are still running from ROM, so data use is limited
 */
int env_init(void){
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 0;

	return(0);
}

#endif /* CFG_ENV_IS_NOWHERE) */

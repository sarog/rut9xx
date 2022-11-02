/*
 * (C) Copyright 2016 JSC Teltonika <gpl@teltonika.lt>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crc32.c"

int main(int argc, char *argv[]) {
	char *line;
	int i, c;
	ulong size, crc;
	FILE *f;

	if (argc == 2) {
		f = fopen(argv[1], "r");
		if (f == NULL) {
			perror ("Error opening file");
			exit(1);
		}

		fseek(f, 0L, SEEK_END);
		size = ftell(f);
		fseek(f, 0L, SEEK_SET);

		line = (char *) malloc(size+1);

		i=0;
		c = getc(f);
		while (c!= EOF) {
			line[i++] = c;
			c = getc(f);
		}

		fclose(f);

		crc = crc32(0, line, i);

		printf("%010lu %lu %s\n", crc, size, argv[1]);
		free(line);

		exit(0);
	} else {
		printf("usage: crc32 <filename>\n");
		exit(1);
	}


}

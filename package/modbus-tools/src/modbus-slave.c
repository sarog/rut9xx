/*
 * Copyright © 2008-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>

#include "modbus.h"

#define APPNAME "modbus-slave"

enum {
	TCP,
	TCP_PI,
	RTU
};

void help() {
	printf("%s (server) v%s for testing puroses\n\n", APPNAME, VERSION);
	printf("Usage: %s [OPTIONS]\n", APPNAME);
	printf("\t-h\t\tDisplay this help message.\n");
	printf("\t-p [tcp|rtu]\tProtocol. Default - TCP.\n");
	printf("\t-o [1 .. 65535]\tPort. TCP mode only.\n");
	printf("\t-d [dev]\tSpecify serial device. RTU mode only.\n");
	printf("\t-s [0 .. 247]\tSpecify slave ID. RTU mode only.\n");
	printf("\t-v\t\tDisplay version.\n");
}

int main(int argc, char*argv[])
{
	int socket, rc, i, opt, header_length, port = 502;
	modbus_t *ctx;
	modbus_mapping_t *mb_mapping;
	uint8_t *query;
	int use_backend = TCP;
	int slave_id = SERVER_ID;
	char device[256] = "/dev/rs232";

	// Get options.
	while ((opt = getopt(argc, argv, "hp:o:d:s:v")) != -1) {
		switch (opt) {
			case 'h': // help
				help();
				exit(EXIT_SUCCESS);

			case 'p': // protocol
				if (strcmp(optarg, "tcp") == 0) {
					use_backend = TCP;
				} else if (strcmp(optarg, "tcppi") == 0) {
					use_backend = TCP_PI;
				} else if (strcmp(optarg, "rtu") == 0) {
					use_backend = RTU;
				}
				break;

			case 'o': // tcp port
				port = atoi(optarg);
				if (port < 1 || port > 65535) {
					help();
					exit(EXIT_FAILURE);
				}
				break;

			case 'd': // device
				strncpy(device, optarg, sizeof(device));
				break;

			case 's': // slave id
				slave_id = atoi(optarg);
				if (slave_id < 0 || slave_id > 247) {
					help();
					exit(EXIT_FAILURE);
				}
				break;

			case 'v': // version
				printf("%s (server) version: %s\n", APPNAME, VERSION);
				exit(EXIT_SUCCESS);

			case '?': // any other option
				printf("exiting\n");
				exit(EXIT_FAILURE);
		}
	}

	// Print any remaining command line arguments (not options).
	if (optind < argc) {
		printf("unknown arguments: ");
		while (optind < argc)
			printf ("%s ", argv[optind++]);
		printf("\n\n");
		help();
		exit(EXIT_FAILURE);
	}

	if (use_backend == TCP) {
		ctx = modbus_new_tcp("0.0.0.0", port);
		query = malloc(MODBUS_TCP_MAX_ADU_LENGTH);
	} else if (use_backend == TCP_PI) {
		char *sport = malloc(6);
		sprintf(sport, "%d", port);
		ctx = modbus_new_tcp_pi("::0", sport);
		free(sport);
		query = malloc(MODBUS_TCP_MAX_ADU_LENGTH);
	} else {
		ctx = modbus_new_rtu(device, 115200, 'N', 8, 1);
		modbus_set_slave(ctx, slave_id);
		query = malloc(MODBUS_RTU_MAX_ADU_LENGTH);
	}
	header_length = modbus_get_header_length(ctx);

	modbus_set_debug(ctx, TRUE);

	mb_mapping = modbus_mapping_new(
		UT_BITS_ADDRESS + UT_BITS_NB,
		UT_INPUT_BITS_ADDRESS + UT_INPUT_BITS_NB,
		UT_REGISTERS_ADDRESS + UT_REGISTERS_NB,
		UT_INPUT_REGISTERS_ADDRESS + UT_INPUT_REGISTERS_NB);
	if (mb_mapping == NULL) {
		fprintf(stderr, "Failed to allocate the mapping: %s\n",
				modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}

	/* Examples from PI_MODBUS_300.pdf.
	   Only the read-only input values are assigned. */

	/** INPUT STATUS **/
	modbus_set_bits_from_bytes(mb_mapping->tab_input_bits,
							   UT_INPUT_BITS_ADDRESS, UT_INPUT_BITS_NB,
							   UT_INPUT_BITS_TAB);

	/** INPUT REGISTERS **/
	for (i=0; i < UT_INPUT_REGISTERS_NB; i++) {
		mb_mapping->tab_input_registers[UT_INPUT_REGISTERS_ADDRESS+i] =
			UT_INPUT_REGISTERS_TAB[i];;
	}
start:
	if (use_backend == TCP) {
		socket = modbus_tcp_listen(ctx, 1);
		modbus_tcp_accept(ctx, &socket);
	} else if (use_backend == TCP_PI) {
		socket = modbus_tcp_pi_listen(ctx, 1);
		modbus_tcp_pi_accept(ctx, &socket);
	} else {
		rc = modbus_connect(ctx);
		if (rc == -1) {
			fprintf(stderr, "Unable to connect %s\n", modbus_strerror(errno));
			modbus_free(ctx);
			return -1;
		}
	}

	for (;;) {
		rc = modbus_receive(ctx, query);
		if (rc == -1) {
			/* Connection closed by the client or error */
			if (use_backend == TCP) {
				close(socket);
				goto start;
			}
			break;
		}

		/* Read holding registers */
		if (query[header_length] == 0x03) {
			if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 3)
				== UT_REGISTERS_NB_SPECIAL) {
				printf("Set an incorrect number of values\n");
				MODBUS_SET_INT16_TO_INT8(query, header_length + 3,
										 UT_REGISTERS_NB_SPECIAL - 1);
			} else if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 1)
				== UT_REGISTERS_ADDRESS_SPECIAL) {
				printf("Reply to this special register address by an exception\n");
				modbus_reply_exception(ctx, query,
									   MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY);
				continue;
			}
		}

		rc = modbus_reply(ctx, query, rc, mb_mapping);
		if (rc == -1) {
			if (use_backend == TCP) {
				close(socket);
				goto start;
			}
			break;
		}
	}

	printf("Quit the loop: %s\n", modbus_strerror(errno));

	if (use_backend == TCP) {
		close(socket);
	}
	modbus_mapping_free(mb_mapping);
	free(query);
	modbus_free(ctx);

	return 0;
}

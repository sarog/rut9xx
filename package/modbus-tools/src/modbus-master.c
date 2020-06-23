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

#define APPNAME "modbus-master"

enum {
	TCP,
	TCP_PI,
	RTU
};

enum {
	TEST,
	ECHO
};

void help() {
	printf("%s (client) v%s for testing puroses\n\n", APPNAME, VERSION);
	printf("Usage: %s [OPTIONS]\n", APPNAME);
	printf("\t-h\t\tDisplay this help message.\n");
	printf("\t-p [tcp|rtu]\tProtocol. Default - TCP.\n");
	printf("\t-m [test|echo]\tClient mode. Default - test.\n");
	printf("\t-i [ip addr]\tIP address. TCP mode only. Default - 192.168.1.1.\n");
	printf("\t-o [1 .. 65535]\tPort. TCP mode only.\n");
	printf("\t-d [dev]\tSpecify serial device. RTU mode only.\n");
	printf("\t-s [0 .. 247]\tSpecify slave ID. RTU mode only.\n");
	printf("\t-v\t\tDisplay version.\n");
}

/* return 1 if string contain only digits, else return 0 */
int valid_digit(char *ip) {
	while (*ip) {
		if (*ip >= '0' && *ip <= '9')
			++ip;
		else
			return 0;
	}

	return 1;
}

/* return 1 if IP string is valid, else return 0 */
int valid_ip(char *ip) {
	int num, dots = 0;
	char *ptr, ip_str[256];

	strncpy(ip_str, ip, sizeof(ip_str));

	if (ip_str == NULL)
		return 0;

	ptr = strtok(ip_str, ".");

	if (ptr == NULL)
		return 0;

	while (ptr) {
		// After parsing string, it must contain only digits.
		if (!valid_digit(ptr))
			return 0;

		num = atoi(ptr);

		// Check for valid IP.
		if (num >= 0 && num <= 255) {
			// Parse remaining string.
			ptr = strtok(NULL, ".");
			if (ptr != NULL)
				++dots;
		} else
			return 0;
	}

	// Valid IP string must contain 3 dots.
	if (dots != 3)
		return 0;

	return 1;
}

int main(int argc, char *argv[])
{
	uint8_t *tab_rp_bits;
	uint16_t *tab_rp_registers;
	uint16_t *tab_rp_registers_bad;
	modbus_t *ctx;
	int i, rc, nb_points, opt;
	uint8_t value;
	float real;
	uint32_t ireal;
	struct timeval old_response_timeout;
	struct timeval response_timeout;
	int use_backend = TCP, mode = TEST;
	char ip[16] = "192.168.1.1";
	int port = 502;
	int slave_id = SERVER_ID;
	char device[256] = "/dev/rs232";

	// Get options.
	while ((opt = getopt(argc, argv, "hp:m:i:o:d:s:v")) != -1) {
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

			case 'm': // mode
				if (strcmp(optarg, "test") == 0) {
					mode = TEST;
				} else if (strcmp(optarg, "echo") == 0) {
					mode = ECHO;
				}
				break;

			case 'i': // ip address
				strncpy(ip, optarg, sizeof(ip));
				if (!valid_ip(ip)) {
					help();
					exit(EXIT_FAILURE);
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
				printf("%s (client) version: %s\n", APPNAME, VERSION);
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
		ctx = modbus_new_tcp(ip, port);
	} else if (use_backend == TCP_PI) {
		char *sport = malloc(6);
		sprintf(sport, "%d", port);
		ctx = modbus_new_tcp_pi("::1", sport);
		free(sport);
	} else {
		ctx = modbus_new_rtu(device, 115200, 'N', 8, 1);
	}
	if (ctx == NULL) {
		fprintf(stderr, "Unable to allocate libmodbus context\n");
		return -1;
	}
	modbus_set_debug(ctx, TRUE);
	modbus_set_error_recovery(ctx,
							  MODBUS_ERROR_RECOVERY_LINK |
							  MODBUS_ERROR_RECOVERY_PROTOCOL);

	if (use_backend == RTU) {
		  modbus_set_slave(ctx, slave_id);
	}

	if (modbus_connect(ctx) == -1) {
		fprintf(stderr, "Connection failed: %s\n",
				modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}

	/* Allocate and initialize the memory to store the bits */
	nb_points = (UT_BITS_NB > UT_INPUT_BITS_NB) ? UT_BITS_NB : UT_INPUT_BITS_NB;
	tab_rp_bits = (uint8_t *) malloc(nb_points * sizeof(uint8_t));
	memset(tab_rp_bits, 0, nb_points * sizeof(uint8_t));

	/* Allocate and initialize the memory to store the registers */
	nb_points = (UT_REGISTERS_NB > UT_INPUT_REGISTERS_NB) ?
		UT_REGISTERS_NB : UT_INPUT_REGISTERS_NB;
	tab_rp_registers = (uint16_t *) malloc(nb_points * sizeof(uint16_t));
	memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));

	if (mode == TEST) {
		printf("** UNIT TESTING **\n");

		printf("\nTEST WRITE/READ:\n");

		/** COIL BITS **/

		/* Single */
		rc = modbus_write_bit(ctx, UT_BITS_ADDRESS, ON);
		printf("1/2 modbus_write_bit: ");
		if (rc == 1) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}

		rc = modbus_read_bits(ctx, UT_BITS_ADDRESS, 1, tab_rp_bits);
		printf("2/2 modbus_read_bits: ");
		if (rc != 1) {
			printf("FAILED (nb points %d)\n", rc);
			goto close;
		}

		if (tab_rp_bits[0] != ON) {
			printf("FAILED (%0X = != %0X)\n", tab_rp_bits[0], ON);
			goto close;
		}
		printf("OK\n");
		/* End single */

		/* Multiple bits */
		{
			uint8_t tab_value[UT_BITS_NB];

			modbus_set_bits_from_bytes(tab_value, 0, UT_BITS_NB, UT_BITS_TAB);
			rc = modbus_write_bits(ctx, UT_BITS_ADDRESS,
								   UT_BITS_NB, tab_value);
			printf("1/2 modbus_write_bits: ");
			if (rc == UT_BITS_NB) {
				printf("OK\n");
			} else {
				printf("FAILED\n");
				goto close;
			}
		}

		rc = modbus_read_bits(ctx, UT_BITS_ADDRESS, UT_BITS_NB, tab_rp_bits);
		printf("2/2 modbus_read_bits: ");
		if (rc != UT_BITS_NB) {
			printf("FAILED (nb points %d)\n", rc);
			goto close;
		}

		i = 0;
		nb_points = UT_BITS_NB;
		while (nb_points > 0) {
			int nb_bits = (nb_points > 8) ? 8 : nb_points;

			value = modbus_get_byte_from_bits(tab_rp_bits, i*8, nb_bits);
			if (value != UT_BITS_TAB[i]) {
				printf("FAILED (%0X != %0X)\n", value, UT_BITS_TAB[i]);
				goto close;
			}

			nb_points -= nb_bits;
			i++;
		}
		printf("OK\n");
		/* End of multiple bits */

		/** DISCRETE INPUTS **/
		rc = modbus_read_input_bits(ctx, UT_INPUT_BITS_ADDRESS,
									UT_INPUT_BITS_NB, tab_rp_bits);
		printf("1/1 modbus_read_input_bits: ");

		if (rc != UT_INPUT_BITS_NB) {
			printf("FAILED (nb points %d)\n", rc);
			goto close;
		}

		i = 0;
		nb_points = UT_INPUT_BITS_NB;
		while (nb_points > 0) {
			int nb_bits = (nb_points > 8) ? 8 : nb_points;

			value = modbus_get_byte_from_bits(tab_rp_bits, i*8, nb_bits);
			if (value != UT_INPUT_BITS_TAB[i]) {
				printf("FAILED (%0X != %0X)\n", value, UT_INPUT_BITS_TAB[i]);
				goto close;
			}

			nb_points -= nb_bits;
			i++;
		}
		printf("OK\n");

		/** HOLDING REGISTERS **/

		/* Single register */
		rc = modbus_write_register(ctx, UT_REGISTERS_ADDRESS, 0x1234);
		printf("1/2 modbus_write_register: ");
		if (rc == 1) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}

		rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
								   1, tab_rp_registers);
		printf("2/2 modbus_read_registers: ");
		if (rc != 1) {
			printf("FAILED (nb points %d)\n", rc);
			goto close;
		}

		if (tab_rp_registers[0] != 0x1234) {
			printf("FAILED (%0X != %0X)\n",
				   tab_rp_registers[0], 0x1234);
			goto close;
		}
		printf("OK\n");
		/* End of single register */

		/* Many registers */
		rc = modbus_write_registers(ctx, UT_REGISTERS_ADDRESS,
									UT_REGISTERS_NB, UT_REGISTERS_TAB);
		printf("1/5 modbus_write_registers: ");
		if (rc == UT_REGISTERS_NB) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}

		rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
								   UT_REGISTERS_NB, tab_rp_registers);
		printf("2/5 modbus_read_registers: ");
		if (rc != UT_REGISTERS_NB) {
			printf("FAILED (nb points %d)\n", rc);
			goto close;
		}

		for (i=0; i < UT_REGISTERS_NB; i++) {
			if (tab_rp_registers[i] != UT_REGISTERS_TAB[i]) {
				printf("FAILED (%0X != %0X)\n",
					   tab_rp_registers[i],
					   UT_REGISTERS_TAB[i]);
				goto close;
			}
		}
		printf("OK\n");

		rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
								   0, tab_rp_registers);
		printf("3/5 modbus_read_registers (0): ");
		if (rc != -1 && errno == EMBMDATA) {
			printf("FAILED (nb points %d)\n", rc);
			goto close;
		}
		printf("OK\n");

		nb_points = (UT_REGISTERS_NB >
					 UT_INPUT_REGISTERS_NB) ?
			UT_REGISTERS_NB : UT_INPUT_REGISTERS_NB;
		memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));

		/* Write registers to zero from tab_rp_registers and store read registers
		   into tab_rp_registers. So the read registers must set to 0, except the
		   first one because there is an offset of 1 register on write. */
		rc = modbus_write_and_read_registers(ctx,
											 UT_REGISTERS_ADDRESS + 1, UT_REGISTERS_NB - 1,
											 tab_rp_registers,
											 UT_REGISTERS_ADDRESS,
											 UT_REGISTERS_NB,
											 tab_rp_registers);
		printf("4/5 modbus_write_and_read_registers: ");
		if (rc != UT_REGISTERS_NB) {
			printf("FAILED (nb points %d != %d)\n", rc, UT_REGISTERS_NB);
			goto close;
		}

		if (tab_rp_registers[0] != UT_REGISTERS_TAB[0]) {
			printf("FAILED (%0X != %0X)\n",
				   tab_rp_registers[0], UT_REGISTERS_TAB[0]);
		}

		for (i=1; i < UT_REGISTERS_NB; i++) {
			if (tab_rp_registers[i] != 0) {
				printf("FAILED (%0X != %0X)\n",
					   tab_rp_registers[i], 0);
				goto close;
			}
		}
		printf("OK\n");

		/* End of many registers */


		/** INPUT REGISTERS **/
		rc = modbus_read_input_registers(ctx, UT_INPUT_REGISTERS_ADDRESS,
										 UT_INPUT_REGISTERS_NB,
										 tab_rp_registers);
		printf("1/1 modbus_read_input_registers: ");
		if (rc != UT_INPUT_REGISTERS_NB) {
			printf("FAILED (nb points %d)\n", rc);
			goto close;
		}

		for (i=0; i < UT_INPUT_REGISTERS_NB; i++) {
			if (tab_rp_registers[i] != UT_INPUT_REGISTERS_TAB[i]) {
				printf("FAILED (%0X != %0X)\n",
					   tab_rp_registers[i], UT_INPUT_REGISTERS_TAB[i]);
				goto close;
			}
		}
		printf("OK\n");

		printf("\nTEST FLOATS\n");
		/** FLOAT **/
		printf("1/2 Set float: ");
		modbus_set_float(UT_REAL, tab_rp_registers);
		if (tab_rp_registers[1] == (UT_IREAL >> 16) &&
			tab_rp_registers[0] == (UT_IREAL & 0xFFFF)) {
			printf("OK\n");
		} else {
			/* Avoid *((uint32_t *)tab_rp_registers)
			 * https://github.com/stephane/libmodbus/pull/104 */
			ireal = (uint32_t) tab_rp_registers[0] & 0xFFFF;
			ireal |= (uint32_t) tab_rp_registers[1] << 16;
			printf("FAILED (%x != %x)\n", ireal, UT_IREAL);
			goto close;
		}

		printf("2/2 Get float: ");
		real = modbus_get_float(tab_rp_registers);
		if (real == UT_REAL) {
			printf("OK\n");
		} else {
			printf("FAILED (%f != %f)\n", real, UT_REAL);
			goto close;
		}

		printf("\nAt this point, error messages doesn't mean the test has failed\n");

		/** ILLEGAL DATA ADDRESS **/
		printf("\nTEST ILLEGAL DATA ADDRESS:\n");

		/* The mapping begins at 0 and ends at address + nb_points so
		 * the addresses are not valid. */

		rc = modbus_read_bits(ctx, UT_BITS_ADDRESS,
							  UT_BITS_NB + 1, tab_rp_bits);
		printf("* modbus_read_bits: ");
		if (rc == -1 && errno == EMBXILADD) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}

		rc = modbus_read_input_bits(ctx, UT_INPUT_BITS_ADDRESS,
									UT_INPUT_BITS_NB + 1, tab_rp_bits);
		printf("* modbus_read_input_bits: ");
		if (rc == -1 && errno == EMBXILADD)
			printf("OK\n");
		else {
			printf("FAILED\n");
			goto close;
		}

		rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
								   UT_REGISTERS_NB + 1, tab_rp_registers);
		printf("* modbus_read_registers: ");
		if (rc == -1 && errno == EMBXILADD)
			printf("OK\n");
		else {
			printf("FAILED\n");
			goto close;
		}

		rc = modbus_read_input_registers(ctx, UT_INPUT_REGISTERS_ADDRESS,
										 UT_INPUT_REGISTERS_NB + 1,
										 tab_rp_registers);
		printf("* modbus_read_input_registers: ");
		if (rc == -1 && errno == EMBXILADD)
			printf("OK\n");
		else {
			printf("FAILED\n");
			goto close;
		}

		rc = modbus_write_bit(ctx, UT_BITS_ADDRESS + UT_BITS_NB, ON);
		printf("* modbus_write_bit: ");
		if (rc == -1 && errno == EMBXILADD) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}

		rc = modbus_write_bits(ctx, UT_BITS_ADDRESS + UT_BITS_NB,
							   UT_BITS_NB, tab_rp_bits);
		printf("* modbus_write_coils: ");
		if (rc == -1 && errno == EMBXILADD) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}

		rc = modbus_write_registers(ctx, UT_REGISTERS_ADDRESS + UT_REGISTERS_NB,
									UT_REGISTERS_NB, tab_rp_registers);
		printf("* modbus_write_registers: ");
		if (rc == -1 && errno == EMBXILADD) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}


		/** TOO MANY DATA **/
		printf("\nTEST TOO MANY DATA ERROR:\n");

		rc = modbus_read_bits(ctx, UT_BITS_ADDRESS,
							  MODBUS_MAX_READ_BITS + 1, tab_rp_bits);
		printf("* modbus_read_bits: ");
		if (rc == -1 && errno == EMBMDATA) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}

		rc = modbus_read_input_bits(ctx, UT_INPUT_BITS_ADDRESS,
									MODBUS_MAX_READ_BITS + 1, tab_rp_bits);
		printf("* modbus_read_input_bits: ");
		if (rc == -1 && errno == EMBMDATA) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}

		rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
								   MODBUS_MAX_READ_REGISTERS + 1,
								   tab_rp_registers);
		printf("* modbus_read_registers: ");
		if (rc == -1 && errno == EMBMDATA) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}

		rc = modbus_read_input_registers(ctx, UT_INPUT_REGISTERS_ADDRESS,
										 MODBUS_MAX_READ_REGISTERS + 1,
										 tab_rp_registers);
		printf("* modbus_read_input_registers: ");
		if (rc == -1 && errno == EMBMDATA) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}

		rc = modbus_write_bits(ctx, UT_BITS_ADDRESS,
							   MODBUS_MAX_WRITE_BITS + 1, tab_rp_bits);
		printf("* modbus_write_bits: ");
		if (rc == -1 && errno == EMBMDATA) {
			printf("OK\n");
		} else {
			goto close;
			printf("FAILED\n");
		}

		rc = modbus_write_registers(ctx, UT_REGISTERS_ADDRESS,
									MODBUS_MAX_WRITE_REGISTERS + 1,
									tab_rp_registers);
		printf("* modbus_write_registers: ");
		if (rc == -1 && errno == EMBMDATA) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}

		/** SLAVE REPLY **/
		printf("\nTEST SLAVE REPLY:\n");
		modbus_set_slave(ctx, INVALID_SERVER_ID);
		rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
								   UT_REGISTERS_NB, tab_rp_registers);
		if (use_backend == RTU) {
			const int RAW_REQ_LENGTH = 6;
			uint8_t raw_req[] = { INVALID_SERVER_ID, 0x03, 0x00, 0x01, 0xFF, 0xFF };
			uint8_t rsp[MODBUS_TCP_MAX_ADU_LENGTH];

			/* No response in RTU mode */
			printf("1/4-A No response from slave %d: ", INVALID_SERVER_ID);

			if (rc == -1 && errno == ETIMEDOUT) {
				printf("OK\n");
			} else {
				printf("FAILED\n");
				goto close;
			}

			/* Send an invalid query with a wrong slave ID */
			modbus_send_raw_request(ctx, raw_req,
									RAW_REQ_LENGTH * sizeof(uint8_t));
			rc = modbus_receive_confirmation(ctx, rsp);

			printf("1/4-B No response from slave %d with invalid request: ",
				   INVALID_SERVER_ID);

			if (rc == -1 && errno == ETIMEDOUT) {
				printf("OK\n");
			} else {
				printf("FAILED (%d)\n", rc);
				goto close;
			}

		} else {
			/* Response in TCP mode */
			printf("1/4 Response from slave %d: ", 18);

			if (rc == UT_REGISTERS_NB) {
				printf("OK\n");
			} else {
				printf("FAILED\n");
				goto close;
			}
		}

		rc = modbus_set_slave(ctx, MODBUS_BROADCAST_ADDRESS);
		if (rc == -1) {
			printf("Invalid broacast address\n");
			goto close;
		}

		rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
								   UT_REGISTERS_NB, tab_rp_registers);
		printf("2/4 Reply after a broadcast query: ");
		if (rc == UT_REGISTERS_NB) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}

		/* Restore slave */
		if (use_backend == RTU) {
			modbus_set_slave(ctx, slave_id);
		} else {
			modbus_set_slave(ctx, MODBUS_TCP_SLAVE);
		}

		printf("3/4 Report slave ID: \n");
		/* tab_rp_bits is used to store bytes */
		rc = modbus_report_slave_id(ctx, tab_rp_bits);
		if (rc == -1) {
			printf("FAILED\n");
			goto close;
		}

		/* Slave ID is an arbitraty number for libmodbus */
		if (rc > 0) {
			printf("OK Slave ID is %d\n", tab_rp_bits[0]);
		} else {
			printf("FAILED\n");
			goto close;
		}

		/* Run status indicator */
		if (rc > 1 && tab_rp_bits[1] == 0xFF) {
			printf("OK Run Status Indicator is %s\n", tab_rp_bits[1] ? "ON" : "OFF");
		} else {
			printf("FAILED\n");
			goto close;
		}

		/* Print additional data as string */
		if (rc > 2) {
			printf("Additional data: ");
			for (i=2; i < rc; i++) {
				printf("%c", tab_rp_bits[i]);
			}
			printf("\n");
		}

		/* Save original timeout */
		modbus_get_response_timeout(ctx, &old_response_timeout);

		/* Define a new and too short timeout */
		response_timeout.tv_sec = 1;
		response_timeout.tv_usec = 0;
		modbus_set_response_timeout(ctx, &response_timeout);

		rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
								   UT_REGISTERS_NB, tab_rp_registers);
		printf("4/4 Too short timeout: ");
		if (rc == -1 && errno == ETIMEDOUT) {
			printf("OK\n");
		} else {
			printf("FAILED (can fail on slow systems or Windows)\n");
		}

		/* Restore original timeout */
		modbus_set_response_timeout(ctx, &old_response_timeout);

		/* A wait and flush operation is done by the error recovery code of
		 * libmodbus */

		/** BAD RESPONSE **/
		printf("\nTEST BAD RESPONSE ERROR:\n");

		/* Allocate only the required space */
		tab_rp_registers_bad = (uint16_t *) malloc(
			UT_REGISTERS_NB_SPECIAL * sizeof(uint16_t));
		rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
								   UT_REGISTERS_NB_SPECIAL, tab_rp_registers_bad);
		printf("* modbus_read_registers: ");
		//if (rc == -1 && (errno == EMBBADDATA || errno == EAGAIN)) {
		if (rc == -1 && errno == EMBBADDATA) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}

		free(tab_rp_registers_bad);

		/** MANUAL EXCEPTION **/
		printf("\nTEST MANUAL EXCEPTION:\n");

		rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS_SPECIAL,
								   UT_REGISTERS_NB, tab_rp_registers);
		printf("* modbus_read_registers at special address: ");
		if (rc == -1 && errno == EMBXSBUSY) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}

		/** RAW REQUEST */
		printf("\nTEST RAW REQUEST:\n");
		{
			const int RAW_REQ_LENGTH = 6;
			uint8_t raw_req[] = { (use_backend == RTU) ? slave_id : 0xFF,
								  0x03, 0x00, 0x01, 0x0, 0x05 };
			int req_length;
			uint8_t rsp[MODBUS_TCP_MAX_ADU_LENGTH];

			req_length = modbus_send_raw_request(ctx, raw_req,
												 RAW_REQ_LENGTH * sizeof(uint8_t));

			printf("* modbus_send_raw_request: ");
			if ((use_backend == RTU && req_length == (RAW_REQ_LENGTH + 2)) ||
				((use_backend == TCP || use_backend == TCP_PI) &&
				 req_length == (RAW_REQ_LENGTH + 6))) {
				printf("OK\n");
			} else {
				printf("FAILED (%d)\n", req_length);
				goto close;
			}

			printf("* modbus_receive_confirmation: ");
			rc  = modbus_receive_confirmation(ctx, rsp);
			if ((use_backend == RTU && rc == 15) ||
				((use_backend == TCP || use_backend == TCP_PI) &&
				 rc == 19)) {
				printf("OK\n");
			} else {
				printf("FAILED (%d)\n", rc);
				goto close;
			}
		}

		printf("\nALL TESTS PASS WITH SUCCESS.\n");
	} else {
		while (1) {
			rc = modbus_read_registers(ctx, MB_REG_BASE, MB_REG_COUNT, tab_rp_registers+MB_REG_BASE);

			if (rc <= 0) {
				fprintf(stderr, "Unable to read modbus registers\n");
				errno = -1;
			}

			for ( i=MB_REG_BASE; i<MB_REG_BASE+rc; i++ ) {
				printf ( "[%d]: %d\n", i, tab_rp_registers[i]);
			}

			sleep(5);
		}
	}

close:
	/* Free the memory */
	free(tab_rp_bits);
	free(tab_rp_registers);

	/* Close the connection */
	modbus_close(ctx);
	modbus_free(ctx);

	return 0;
}

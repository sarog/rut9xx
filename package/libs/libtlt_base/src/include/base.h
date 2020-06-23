#ifndef BASE_H
#define BASE_H

#include <string.h> // String function definitions (strcmp...)
#include <stdio.h> // Standard input/output definitions (sscanf()...)
#include <stdlib.h> // (EXIT_FAILURE)
#include <signal.h> // (SIGALARM)
#include <syslog.h>

#define OK_ERR			 0
#define NO_ERR			-1
#define READ_ERR		-2
#define PACKET_ERR	-3
#define PARSE_ERR		-4
#define SYSCALL_ERR	-5
#define OPTION_ERR	-6
#define UCICALL_ERR	-7
#define IFADDRCALL_ERR	-8

int check_output_errors(char *output);
void check_alocate(void *name);
void quit(int error_code);

#endif

#pragma once

/***************************************************************************
 *
 * Functions for converting between an ISO-8859-1 ASCII string and a
 * PDU-coded string as described in ETSI GSM 03.38 and ETSI GSM 03.40.
 *
 * This code is released to the public domain in 2003 by Mats Engstrom,
 * Nerdlabs Consulting. ( matseng at nerdlabs dot org )
 *
 **************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <malloc.h>

/* Define Non-Printable Characters as a question mark */
#define NPC7    63
#define NPC8    '?'

/****************************************************************************
	This lookup table converts from ISO-8859-1 8-bit ASCII to the
	7 bit "default alphabet" as defined in ETSI GSM 03.38

	ISO-characters that don't have any correspondning character in the
	7-bit alphabet is replaced with the NPC7-character.  If there's
	a close match between the ISO-char and a 7-bit character (for example
	the letter i with a circumflex and the plain i-character) a substitution
	is done. These "close-matches" are marked in the lookup table by
	having its value negated.

	There are some character (for example the curly brace "}") that must
	be converted into a 2 byte 7-bit sequence.  These characters are
	marked in the table by having 256 added to its value.
****************************************************************************/

int convert_7bit_to_ascii(char *a7bit, int length, char **ascii, int start);
int convert_ascii_to_7bit(char *ascii, char **a7bit, int sms_num);
int pdu_to_ascii(unsigned char *pdu, int pdulength, char **ascii, int start, int data_length);
int ascii_to_pdu(char *ascii, unsigned char **pdu, int *cnt_7bit, int sms_num);

int lookup_8to7(unsigned char text);

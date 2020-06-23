/* this is a small program for testing the IsWildcardMatch function
   by Julian Robichaux -- http://www.nsftools.com */

#include <stdio.h>
#include <string.h>

#include "wildcard.h"


int main( int argc, char *argv[] )
{
	char string[256];	// you can use either an initialized char array...
	char *pattern;		// ...or a pointer for the IsWildcardMatch function

	if (argc < 3)
	{
		printf("USAGE: wcTest <pattern> <string>\n\n");
		printf("This will return TRUE or FALSE, depending on whether or not\n");
		printf("the string matches the wildcard pattern (which can contain\n");
		printf("* or ? as wildcard characters).\n\n");
		printf("For example: 'wcTest win*.??? windows.bat' will return TRUE.\n\n");
		return 1;
	}

	pattern = argv[1];
	strcpy(string, argv[2]);
	printf("pattern = %s\nstring = %s\n\n", pattern, string);

	if (IsWildcardMatch(pattern, string, FALSE))
		printf("match = TRUE\n");
	else
		printf("match = FALSE\n");

	return 0;
}



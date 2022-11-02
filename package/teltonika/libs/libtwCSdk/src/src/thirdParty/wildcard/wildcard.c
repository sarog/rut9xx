/**********************************************************************
 * wildcard.c
 *
 * written by Julian Robichaux, http://www.nsftools.com
 * please see the wildcard.h header file for more information
 **********************************************************************/

/*
	ThingWorx - added ctype.h include  for MicroSoft.
*/

#include <stdlib.h>
#include <string.h>
#if defined(_WIN32) || defined(OS_IOS)
#include <ctype.h>
#endif

#include "wildcard.h"


/*  See if a string matches a wildcard specification that uses * or ?
    (like "*.txt"), and return TRUE or FALSE, depending on the result.
    There's also a TRUE/FALSE parameter you use to indicate whether
    the match should be case-sensitive or not.  */
int IsWildcardMatch (const char *wildcardString, const char *stringToCheck, int caseSensitive)
{
	char wcChar;
	char strChar;
	// use the starMatchesZero variable to determine whether an asterisk
	// matches zero or more characters (TRUE) or one or more characters
	// (FALSE)
	int starMatchesZero = TRUE;


	while ((strChar = *stringToCheck) && (wcChar = *wildcardString))
	{
		// we only want to advance the pointers if we successfully assigned
		// both of our char variables, so we'll do it here rather than in the
		// loop condition itself
		*stringToCheck++;
		*wildcardString++;

		// if this isn't a case-sensitive match, make both chars uppercase
		// (thanks to David John Fielder (Konan) at http://innuendo.ev.ca
		// for pointing out an error here in the original code)
		if (!caseSensitive)
		{
			wcChar = toupper(wcChar);
			strChar = toupper(strChar);
		}

		// check the wcChar against our wildcard list
		switch (wcChar)
		{
			// an asterisk matches zero or more characters
			case '*' :
				// do a recursive call against the rest of the string,
				// until we've either found a match or the string has
				// ended
				if (starMatchesZero)
					*stringToCheck--;

				while (*stringToCheck)
				{
					if (IsWildcardMatch(wildcardString, stringToCheck++, caseSensitive))
						return TRUE;
				}

				break;

			// a question mark matches any single character
			case '?' :
				break;

			// if we fell through, we want an exact match
			default :
				if (wcChar != strChar)
					return FALSE;
				break;
		}

	}

	// if we have any asterisks left at the end of the wildcard string, we can
	// advance past them if starMatchesZero is TRUE (so "blah*" will match "blah")
	while ((*wildcardString) && (starMatchesZero))
	{
		if (*wildcardString == '*')
			wildcardString++;
		else
			break;
	}
	
	// if we got to the end but there's still stuff left in either of our strings,
	// return false; otherwise, we have a match
	if ((*stringToCheck) || (*wildcardString))
		return FALSE;
	else
		return TRUE;

}


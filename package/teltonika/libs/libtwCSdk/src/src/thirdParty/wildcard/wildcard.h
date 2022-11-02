/**********************************************************************
 * This is a function that allows you to check to see if a string
 * matches a wildcard pattern (with * or ?). It should work in much
 * the same way as the wildcard patterns work in a DOS shell: for
 * example, *.bat would match a string that ends with .bat, and ?th
 * would match 4th, 5th, 6th, etc.
 *
 * Written by Julian Robichaux
 * http://www.nsftools.com
 *
 * You may use this code in any way you'd like, as long as you don't
 * hold me liable for anything, and you don't tell people that you 
 * wrote it yourself. If you're feeling really generous, please include
 * my name and website whenever you use this code in your programs.
 * Thanks!
 **********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* make sure the boolean TRUEs and FALSEs are defined */
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE !FALSE
#endif

/* Function prototypes */
/* This function checks to see if a string (*stringToCheck) matches a
 * wildcard pattern (*wildcardString). There is also a boolean parameter
 * (caseSensitive) to indicate whether the match should be case-sensitive
 * or not. It will return TRUE if there is a match, or FALSE if there's
 * not a match.
 *
 * A ? in a wildcard string will match any one character, and a * in a
 * wildcard string will match zero or more characters (although you can 
 * easily modify the function code to make a * match one or more characters,
 * if that's what you want).
 *
 * As an example, IsWildcardMatch("c*c?s", "circus", FALSE) should return
 * TRUE.
 */
int IsWildcardMatch (const char *wildcardString, const char *stringToCheck, int caseSensitive);

#ifdef __cplusplus
}
#endif

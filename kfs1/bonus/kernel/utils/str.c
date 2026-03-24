/* ************************************************************************** */
/*                                                                            */
/*   str.c - String utility functions (currently unused)                      */
/*                                                                            */
/*   Contains string manipulation functions for kernel use.                   */
/*   Most functions are currently commented out but preserved for             */
/*   future implementation.                                                   */
/*                                                                            */
/* ************************************************************************** */

/*
 * String comparison function
 *
 * int kstrcmp(char *s1, char *s2)
 * {
 *     while (*s1 && *s2)
 *     {
 *         if ((unsigned char)*s1 != (unsigned char)*s2)
 *         {
 *             return ((unsigned char)*s1 - (unsigned char)*s2);
 *         }
 *         *s1++;
 *         *s2++;
 *     }
 *
 *     if ((unsigned char)*s1 == '\0' && (unsigned char)*s2 == '\0')
 *     {
 *         return (0);
 *     }
 *     else if ((unsigned char)*s1 == '\0')
 *     {
 *         return (-1);
 *     }
 *     else
 *     {
 *         return (1);
 *     }
 * }
 */

/*
 * String length function
 *
 * int kstrlen(char *s)
 * {
 *     char *new = s;
 *
 *     while (*new)
 *     {
 *         new++;
 *     }
 *
 *     return new - s;
 * }
 */

/*
 * String copy function (from index)
 *
 * char* kstrcpy(char* source, int start_index)
 * {
 *     char destination[10000];
 *
 *     int length = strlen(source);
 *     if (start_index >= length)
 *     {
 *         if (destination)
 *         {
 *             destination[0] = '\0';
 *         }
 *         return destination;
 *     }
 *
 *     int i = 0;
 *     while (source[start_index] != '\0')
 *     {
 *         destination[i++] = source[start_index++];
 *     }
 *     destination[i] = '\0';
 *
 *     return destination;
 * }
 */

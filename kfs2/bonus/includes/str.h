/*
 * =============================================================================
 *                              KFS2 BONUS - STRING HEADER
 * =============================================================================
 * Kernel string manipulation functions header file
 * Provides basic string operations for the kernel
 * =============================================================================
 */

#ifndef STR_H
# define STR_H

/*
 * k_strcmp - Compare two strings
 * @s1: First string to compare
 * @s2: Second string to compare
 * Returns: 0 if equal, negative if s1 < s2, positive if s1 > s2
 */
int k_strcmp(const char *s1, const char *s2);

/*
 * k_strlen - Calculate the length of a string
 * @s: String to measure
 * Returns: Number of characters before the null terminator
 */
int k_strlen(const char *s);

/*
 * k_strcpy - Copy a string starting from a given index
 * @source: Source string to copy from
 * @start_index: Starting index in source string
 * @destination: Destination buffer
 * Returns: Pointer to destination string
 */
char* k_strcpy(char* source, int start_index, char *destination);

#endif

/* ************************************************************************** */
/*                                                                            */
/*   str.c - Kernel string utility functions                                  */
/*                                                                            */
/* ************************************************************************** */

/*
** Compare two null-terminated strings lexicographically
** Returns:
**   0  if strings are equal
**   <0 if s1 is less than s2
**   >0 if s1 is greater than s2
*/
int
k_strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s2)
    {
        if ((unsigned char)*s1 != (unsigned char)*s2)
        {
            return ((unsigned char)*s1 - (unsigned char)*s2);
        }
        *s1++;
        *s2++;
    }
    if ((unsigned char)*s1 == '\0' && (unsigned char)*s2 == '\0')
    {
        return (0);
    }
    else if ((unsigned char)*s1 == '\0')
    {
        return (-1);
    }
    else
    {
        return (1);
    }
}

/*
** Calculate the length of a null-terminated string
** Returns the number of characters before the null terminator
*/
int
k_strlen(const char *s)
{
    char *ptr = s;

    while (*ptr)
    {
        ptr++;
    }
    return ptr - s;
}

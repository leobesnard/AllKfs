int kstrcmp(const char *s1, const char *s2)
{
    while (*s1 && *s2)
    {
        if ((unsigned char)*s1 != (unsigned char)*s2)
            return ((unsigned char)*s1 - (unsigned char)*s2);
        *s1++;
        *s2++;
    }

    if ((unsigned char)*s1 == '\0' && (unsigned char)*s2 == '\0')
        return (0);
    else if ((unsigned char)*s1 == '\0')
        return (-1);
    else
        return (1);
}

int kstrlen(const char *s)
{
    char *new = s;
    while (*new)
    {
        new++;
    }
    return new - s;
}
static int valid_mask(const char *mask, const char *str)
{
    /*
     *  \\  next character is a literal (not a function) (required)
     *  \?  next character is a literal (not a function) (optional)
     *  +   repeat while next intruction match
     *  0   isdigit (required)
     *  9   isdigit (optional)
     *  A   isalpha (required)
     *  a   isalpha (optional)
     *  T   isalnum (required)
     *  t   isalnum (optional)
     *  X   isxdigit (required)
     *  x   isxdigit (optional)
     *  *   end (returns the position if there is more text to scan or 1)
     */

    const char *ptr = str;
    int required = 1;
    int repeat = 0;

    while (*mask != '\0')
    {
        int (*func)(int) = NULL;

        switch (*mask)
        {
            case '\\':
                mask++;
                break;
            case '\?':
                required = 0;
                mask++;
                break;
            case '+':
                repeat = 1;
                mask++;
                continue;
            case '0':
                func = isdigit;
                break;
            case '9':
                func = isdigit;
                required = 0;
                break;
            case 'A':
                func = isalpha;
                break;
            case 'a':
                func = isalpha;
                required = 0;
                break;
            case 'T':
                func = isalnum;
                break;
            case 't':
                func = isalnum;
                required = 0;
                break;
            case 'X':
                func = isxdigit;
                break;
            case 'x':
                func = isxdigit;
                required = 0;
                break;
            case '*':
                return (*str == '\0') ? 1 : (int)(str - ptr);
            default:
                break;
        }

        int match = 0;

        if (func != NULL)
        {
            match = func((unsigned char)*str);
        }
        else
        {
            match = (*mask == *str);
        }
        if (match)
        {
            if (*str != '\0')
            {
                str++;
            }
            else
            {
                break;
            }
            if (repeat && func)
            {
                while (func((unsigned char)*str))
                {
                    str++;
                }
            }
        }
        else if (required)
        {
            return 0;
        }
        required = 1;
        repeat = 0;
        mask++;
    }
    return (*str == *mask);
}

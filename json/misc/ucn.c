/* Returns the codepoint of an UCN (Universal character name) "\uxxxx" */
static unsigned ucn_code(const char *str, int *error)
{
    if (error != NULL)
    {
        *error = 0;
    }

    int code = 0;

    for (int len = 0; len < 4; len++)
    {
        int c = *(++str);

        switch (*str)
        {
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                code = code * 16 + c - '0';
                break;
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                code = code * 16 + 10 + c - 'A';
                break;
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                code = code * 16 + 10 + c - 'a';
                break;
            default:
                if (error != NULL)
                {
                    *error = 1;
                }
                return 0;
        }
    }
    return (unsigned)code;
}

/* Check wether a character is an UCN */
static int is_ucn(const char *str)
{
    if (*str == 'u')
    {
        int error;

        ucn_code(str, &error);
        return !error;
    }
    return 0;
}

/*
 * Converts UCN to multibyte
 * Returns the length of the multibyte in bytes
 */
static size_t ucn_to_mb(const char *str, char *buf)
{
    unsigned code = ucn_code(str, NULL);

    if (code <= 0x7f)
    {
        buf[0] = (char)code;
        return 1;
    }
    else if (code <= 0x7ff)
    {
        buf[0] = (char)(0xc0 | ((code >> 6) & 0x1f));
        buf[1] = (char)(0x80 | ((code & 0x3f)));
        return 2;
    }
    else if (code <= 0xffff)
    {
        buf[0] = (char)(0xe0 | ((code >> 12) & 0x0f));
        buf[1] = (char)(0x80 | ((code >> 6) & 0x3f));
        buf[2] = (char)(0x80 | ((code & 0x3f)));
        return 3;
    }
    else
    {
        buf[0] = (char)(0xf0 | ((code >> 18) & 0x07));
        buf[1] = (char)(0x80 | ((code >> 12) & 0x3f));
        buf[2] = (char)(0x80 | ((code >> 6) & 0x3f));
        buf[3] = (char)(0x80 | ((code & 0x3f)));
        return 4;
    }
}

/*
 * Encodes a multibyte as UCN "\uxxxx"
 * Returns the length of the string in bytes (6)
 */
size_t json_ucn_encode(char *buf, const char *str)
{
    unsigned int code = 0;

    while (*str != 0)
    {
        unsigned char c = (unsigned char)*str;

        if (c <= 0x7f)
        {
            code = c;
        }
        else if (c <= 0xbf)
        {
            code = (code << 6) | (c & 0x3f);
        }
        else if (c <= 0xdf)
        {
            code = c & 0x1f;
        }
        else if (c <= 0xef)
        {
            code = c & 0x0f;
        }
        else
        {
            code = c & 0x07;
        }
        str++;
    }
    return (size_t)sprintf(buf, "\\u%04x", code);
}


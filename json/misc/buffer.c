#include <stdio.h>
#include <stdlib.h>

#define MIN_SIZE 16

typedef struct
{
    size_t length;
    size_t size;
    char *value;
} json_buffer;

/* Round to next power of 2 (>= size) */
static size_t aligned_size(size_t size)
{
    size--;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    if (sizeof(size) >= 8)
    {
        size |= size >> 32;
    }
    size++;
    return size;
}

static json_buffer *json_create_buffer(size_t size)
{
    json_buffer *buffer = calloc(1, sizeof *buffer);

    if (buffer != NULL)
    {
        if (size < MIN_SIZE)
        {
            buffer->size = MIN_SIZE;
        }
        else
        {
            buffer->size = aligned_size(size);
        }
        buffer->value = malloc(buffer->size);
        if (buffer->value == NULL)
        {
            free(buffer);
            return NULL;
        }
        buffer->value[0] = '\0';
    }
    return buffer;
}

static int buffer_resize(json_buffer *buffer)
{
    char *temp = realloc(buffer->value, buffer->size * 2);

    if (temp == NULL)
    {
        return 0;
    }
    buffer->value = temp;
    buffer->size *= 2;
    return 1;
}

#define ESCAPE(buffer, c)                   \
do                                          \
{                                           \
    if (buffer->length + 2 >= buffer->size) \
    {                                       \
        if (buffer_resize(buffer) == 0)     \
        {                                   \
            return 0;                       \
        }                                   \
    }                                       \
    buffer->value[buffer->length++] = '\\'; \
    buffer->value[buffer->length++] = c;    \
} while (0)

#define CONCAT(buffer, c)                   \
do                                          \
{                                           \
    if (buffer->length + 1 >= buffer->size) \
    {                                       \
        if (buffer_resize(buffer) == 0)     \
        {                                   \
            return 0;                       \
        }                                   \
    }                                       \
    buffer->value[buffer->length++] = c;    \
} while (0)

static int json_encode(json_buffer *buffer, const char *str)
{
    while (*str != '\0')
    {
        int escape = 0;

        switch (*str)
        {
            case '\\':
                escape = '\\';
                break;
            case '/':
                escape = '/';
                break;
            case '"':
                escape = '"';
                break;
            case '\b':
                escape = 'b';
                break;
            case '\f':
                escape = 'f';
                break;
            case '\n':
                escape = 'n';
                break;
            case '\r':
                escape = 'r';
                break;
            case '\t':
                escape = 't';
                break;
            default:
                break;
        }
        if (escape == 0)
        {
            CONCAT(buffer, *str);
        }
        else
        {
            ESCAPE(buffer, escape);
        }
        str++;
    }
    buffer->value[buffer->length] = '\0';
    return 1;
}

static int json_quote(json_buffer *buffer, const char *str)
{
    CONCAT(buffer, '"');

    int result = json_encode(buffer, str);

    if (result != 0)
    {
        CONCAT(buffer, '"');
        buffer->value[buffer->length] = '\0';
    }
    return result;
}

static void json_free_buffer(json_buffer *buffer)
{
    if (buffer != NULL)
    {
        free(buffer->value);
        free(buffer);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        return 0;
    }

    json_buffer *buffer = json_create_buffer(0);

    if (buffer == NULL)
    {
        perror("json_buffer_create");
        exit(EXIT_FAILURE);
    }
    printf("length = %zu | size = %zu\n", buffer->length, buffer->size); 
    if (json_quote(buffer, argv[1]))
    {
        puts(buffer->value);
    }
    printf("length = %zu | size = %zu\n", buffer->length, buffer->size); 
    json_free_buffer(buffer);
    return 0;
}


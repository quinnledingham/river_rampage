//
// string
//

inline b32
equal(const char* a, const char *b)
{
    if (a == 0 && b == 0) return true;
    if (a == 0 || b == 0) return false;
    //printf("%s == %s\n", a, b);
    
    int i = 0;
    do
    {
        if (a[i] != b[i])
            return false;
    } while(a[i] != 0 && b[i++] != 0);
    
    return true;
}

inline u32
get_length(const char *string)
{
    if (string == 0)
        return 0;
    
    u32 length = 0;
    const char *ptr = string;
    while(*ptr != 0)
    {
        length++;
        ptr++;
    }
    return length;
}

inline char*
string_malloc(const char *string)
{
    if (string == 0) return 0;
    u32 length = get_length(string);
    char* result = (char*)malloc(length + 1);
    for (u32 i = 0; i < length; i++) result[i] = string[i];
    result[length] = 0;
    return result;
}

inline const char*
string_malloc_length(const char *string, u32 length)
{
    if (string == 0) return 0;
    char* result = (char*)malloc(length + 1);
    for (u32 i = 0; i < length; i++) result[i] = string[i];
    result[length] = 0;
    return result;
}

// converts n number of chars to a string
// ex) chtos(3, a, b, c) returns "abc"
inline char*
chtos(int n, ...)
{
    char* s = (char*)malloc(n + 1);
    memset(s, 0, n + 1);
    
    va_list ptr;
    va_start(ptr, n);
    for (int i = 0; i < n; i++)
    {
        s[i] = va_arg(ptr, int);
    }
    
    return s;
}

inline char*
ftos(f32 f)
{
    u32 size = 64;
    char *buffer = (char*)malloc(size);
    memset(buffer, 0, size);
    u32 ret = snprintf(buffer, size, "%f", f);
    if (ret < 0)
    {
        error(0, "ftos(): failed");
        return 0;
    }
    if (ret >= size) warning(0, "ftos(): result was truncated");
    return buffer;
}

// ptr must point to first char of int
inline const char*
char_array_to_s32(const char *ptr, s32 *result)
{
    u32 sign = 1;
    s32 num = 0;

    if (*ptr == '-')
    {
        sign = -1;
        ptr++;
    }

    while (isdigit(*ptr)) num = 10 * num + (*ptr++ - '0');
    *result = sign * num;

    return ptr;
}

inline const char*
char_array_to_u32(const char *ptr, u32 *result)
{   
    s32 num = 0;
    ptr = char_array_to_s32(ptr, &num);
    *result = (u32)num;
    return ptr;
}

// char_array_to_float

#define MAX_POWER 20

global const
double POWER_10_POS[MAX_POWER] =
{
    1.0e0,  1.0e1,  1.0e2,  1.0e3,  1.0e4,  1.0e5,  1.0e6,  1.0e7,  1.0e8,  1.0e9,
    1.0e10, 1.0e11, 1.0e12, 1.0e13, 1.0e14, 1.0e15, 1.0e16, 1.0e17, 1.0e18, 1.0e19,
};

global const
double POWER_10_NEG[MAX_POWER] =
{
    1.0e0,   1.0e-1,  1.0e-2,  1.0e-3,  1.0e-4,  1.0e-5,  1.0e-6,  1.0e-7,  1.0e-8,  1.0e-9,
    1.0e-10, 1.0e-11, 1.0e-12, 1.0e-13, 1.0e-14, 1.0e-15, 1.0e-16, 1.0e-17, 1.0e-18, 1.0e-19,
};

inline b8
is_exponent(char c)
{
    return (c == 'e' || c == 'E');
}

// returns the point where it stopped reading chars
// reads up until it no longer looks like a number
// writes the result it got to where result pointer
inline const char*
char_array_to_f32(const char *ptr, f32 *result)
{
    r64 sign = 1.0;
    r64 num  = 0.0;
    r64 fra  = 0.0;
    r64 div  = 1.0;
    u32 eval = 0;
    const r64* powers = POWER_10_POS;

    switch (*ptr)
    {
        case '+': sign =  1.0; ptr++; break;
        case '-': sign = -1.0; ptr++; break;
    }

    while (isdigit(*ptr)) num = 10.0 * num + (double)(*ptr++ - '0');

    if (*ptr == '.') ptr++;

    while (isdigit(*ptr))
    {
        fra  = 10.0 * fra + (double)(*ptr++ - '0');
        div *= 10.0;
    }

    num += fra / div;

    if (is_exponent(*ptr))
    {
        ptr++;

        switch (*ptr)
        {
            case '+': powers = POWER_10_POS; ptr++; break;
            case '-': powers = POWER_10_NEG; ptr++; break;
        }

        while (isdigit(*ptr)) eval = 10 * eval + (*ptr++ - '0');

        num *= (eval >= MAX_POWER) ? 0.0 : powers[eval];
    }

    *result = (f32)(sign * num);

    return ptr;
}

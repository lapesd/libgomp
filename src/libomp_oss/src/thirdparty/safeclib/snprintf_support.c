/*------------------------------------------------------------------
 * snprintf_support.c
 *
 * August 2014, D Wheeler
 *
 * Copyright (c) 2014 by Intel Corp
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *------------------------------------------------------------------
 */
#include "safeclib_private.h"
#include "safe_str_constraint.h"
#include "safe_str_lib.h"
#include "snprintf_s.h"

#define FMT_CHAR    'c'
#define FMT_WCHAR   'C'
#define FMT_SHORT   'h'
#define FMT_INT     'd'
#define FMT_LONG    'l'
#define FMT_STRING  's'
#define FMT_WSTRING 'S'
#define FMT_DOUBLE  'g'
#define FMT_LDOUBLE 'G'
#define FMT_VOID    'p'
#define FMT_PCHAR   '1'
#define FMT_PSHORT  '2'
#define FMT_PINT    '3'
#define FMT_PLONG   '4'



#define MAX_FORMAT_ELEMENTS    16

#define CHK_FORMAT(X,Y)   (((X)==(Y))?1:0)

//
// Changes by OMP team
// 1. Enabled parsing of variable width/precision
// 2. Enabled parsing of adjacent format expression
//
unsigned int
parse_format(const char *format, char pformatList[], unsigned int maxFormats)
{
    unsigned int  numFormats = 0;
    unsigned int  index = 0;
    unsigned int  start = 0;
    char          lmod = 0;

    while (index < RSIZE_MAX_STR && format[index] != '\0' && numFormats < maxFormats)
    {
        if (format[index] == '%') {
            start = index; // remember where the format string started
            // Check for flags
            switch( format[++index]) {
            case '\0': continue; // skip - end of format string
            case '%' : continue; // skip - actually a percent character
            case '#' : // convert to alternate form
            case '0' : // zero pad
            case '-' : // left adjust
            case ' ' : // pad with spaces
            case '+' : // force a sign be used
                index++; // skip the flag character
                break;
            }
            // check for and skip the optional field width, including variable width
            while ( format[index] != '\0' &&
                    ((format[index] >= '0' && format[index] <= '9') || format[index] == '*') ) {
                index++;
            }
            // Check for and skip the optional precision, including variable precision
            if ( format[index] != '\0' && format[index] == '.') {
                index++; // skip the period
                while ( format[index] != '\0' &&
                        ((format[index] >= '0' && format[index] <= '9') || format[index] == '*') ) {
                    index++;
                }
            }
            // Check for and skip the optional length modifiers
            lmod = ' ';
            switch( format[index]) {
            case 'h' : if ( format[++index] == 'h') {
                           ++index; //also recognize the 'hh' modifier
                           lmod = 'H'; // for char
                       } else {
                           lmod = 'h'; // for short
                       }
                       break;
            case 'l' : if ( format[++index] == 'l') {
                           ++index; //also recognize the 'll' modifier
                           lmod = 'd'; // for long long
                       } else {
                           lmod = 'l'; // for long
                       }
                       break;
            case 'L' : lmod = 'L'; break;
            case 'j' :
            case 'z' :
            case 't' : index++;
                       break;
            }

            // Recognize and record the actual modifier
            switch( format[index]) {
            case 'c' :
                 if ( lmod == 'l') {
                     pformatList[numFormats] = FMT_WCHAR; // store the format character
                 } else {
                     pformatList[numFormats] = FMT_CHAR;
                 }
                 numFormats++;
                 index++; // skip the format character
                 break;

            case 'd' : case 'i' : // signed
            case 'o' : case 'u' : // unsigned
            case 'x' : case 'X' : // unsigned
                 if ( lmod == 'H') {
                     pformatList[numFormats] = FMT_CHAR; // store the format character
                 } else if ( lmod == 'l') {
                      pformatList[numFormats] = FMT_LONG; // store the format character
                 } else if ( lmod == 'h') {
                     pformatList[numFormats] = FMT_SHORT; // store the format character
                 } else{
                     pformatList[numFormats] = FMT_INT;
                 }
                 numFormats++;
                 index++; // skip the format character
                 break;

            case 'e' : case 'E' :
            case 'f' : case 'F' :
            case 'g' : case 'G' :
            case 'a' : case 'A' :
                 if ( lmod == 'L') {
                     pformatList[numFormats] = FMT_LDOUBLE; // store the format character
                 } else{
                     pformatList[numFormats] = FMT_DOUBLE;
                 }
                 numFormats++;
                 index++; // skip the format character
                 break;

            case 's' :
                 if ( lmod == 'l' || lmod == 'L') {
                     pformatList[numFormats] = FMT_WSTRING; // store the format character
                 } else {
                     pformatList[numFormats] = FMT_STRING;
                 }
                 numFormats++;
                 index++; // skip the format character
                 break;

            case 'p' :
                 pformatList[numFormats] = FMT_VOID;
                 numFormats++;
                 index++; // skip the format character
                 break;

            case 'n' :
                 if ( lmod == 'H') {
                     pformatList[numFormats] = FMT_PCHAR; // store the format character
                 } else if ( lmod == 'l') {
                     pformatList[numFormats] = FMT_PLONG; // store the format character
                 } else if ( lmod == 'h') {
                     pformatList[numFormats] = FMT_PSHORT; // store the format character
                 } else{
                     pformatList[numFormats] = FMT_PINT;
                 }
                 numFormats++;
                 index++; // skip the format character
                 break;
            case 'm' :
                 // Does not represent an argument in the call stack
                 index++; // skip the format character
                 continue;
            default:
                printf("failed to recognize format string [");
                for (;start<index; start++) { printf("%c", format[start]); }
                puts("]");
                break;
            }
        }
        // don't skip over this prefix.
        if (format[index] != '%')
            index++; // move past this character
    }

    return numFormats;
}


//
// snprintf helpers/wrappers for OMP RTL - calls snprintf eventaully after
// checking the format and arguments, and eliminating variadic calls
//

#define CHECK_FORMAT(num)                                                      \
    char pformatList[MAX_FORMAT_ELEMENTS];                                     \
    unsigned int index = 0;                                                    \
    unsigned int nfo = parse_format(format, pformatList, MAX_FORMAT_ELEMENTS); \
    if (nfo != (num)) {                                                        \
        dest[0] = '\0';                                                        \
        return SNPRFNEGATE(ESBADFMT);                                          \
    }

#define CHECK_TYPE(type)                                                       \
    if (FMT_##type != pformatList[index]) {                                    \
        dest[0] = '\0';                                                        \
        return SNPRFNEGATE(ESFMTTYP);                                          \
    }                                                                          \
    index++;

// Suffixes
// cN - N character arguments
// iN - N integer arguments
// sN - N string arguments
// lN - N long arguments
// *w - has a width argument

int
snprintf_c1(char *dest, rsize_t dmax, const char *format, char c)
{
    CHECK_FORMAT(1);
    CHECK_TYPE(CHAR);
    return snprintf(dest, dmax, format, c);
}

int
snprintf_i1(char *dest, rsize_t dmax, const char *format, int i)
{
    CHECK_FORMAT(1);
    CHECK_TYPE(INT);
    return snprintf(dest, dmax, format, i);
}

int
snprintf_i2(char *dest, rsize_t dmax, const char *format, int i1, int i2)
{
    CHECK_FORMAT(2);
    CHECK_TYPE(INT); CHECK_TYPE(INT);
    return snprintf(dest, dmax, format, i1, i2);
}

int
snprintf_i1s1(char *dest, rsize_t dmax, const char *format, int i, const char *s)
{
    CHECK_FORMAT(2);
    CHECK_TYPE(INT); CHECK_TYPE(STRING);
    return snprintf(dest, dmax, format, i, s);
}

int
snprintf_i1s1i1(char *dest, rsize_t dmax, const char *format, int i1, const char *s, int i2)
{
    CHECK_FORMAT(3);
    CHECK_TYPE(INT); CHECK_TYPE(STRING); CHECK_TYPE(INT);
    return snprintf(dest, dmax, format, i1, s, i2);
}

int
snprintf_i1s1i2(char *dest, rsize_t dmax, const char *format, int i1, const char *s, int i2, int i3)
{
    CHECK_FORMAT(4);
    CHECK_TYPE(INT); CHECK_TYPE(STRING); CHECK_TYPE(INT); CHECK_TYPE(INT);
    return snprintf(dest, dmax, format, i1, s, i2, i3);
}

int
snprintf_i1w(char *dest, rsize_t dmax, const char *format, int width, int i)
{
    CHECK_FORMAT(1);
    CHECK_TYPE(INT);
    return snprintf(dest, dmax, format, width, i);
}

int
snprintf_i4p1i4l1(char *dest, rsize_t dmax, const char *format, int i1, int i2, int i3, int i4, void *p,
                  int i5, int i6, int i7, int i8, long l)
{
    CHECK_FORMAT(10);
    CHECK_TYPE(INT); CHECK_TYPE(INT); CHECK_TYPE(INT); CHECK_TYPE(INT); CHECK_TYPE(VOID);
    CHECK_TYPE(INT); CHECK_TYPE(INT); CHECK_TYPE(INT); CHECK_TYPE(INT); CHECK_TYPE(LONG);
    return snprintf(dest, dmax, format, i1, i2, i3, i4, p, i5, i6, i7, i8, l);
}

int
snprintf_l1(char *dest, rsize_t dmax, const char *format, long l)
{
    CHECK_FORMAT(1);
    CHECK_TYPE(LONG);
    return snprintf(dest, dmax, format, l);
}

int
snprintf_l1i1p1(char *dest, rsize_t dmax, const char *format, long l, int i, void *p)
{
    CHECK_FORMAT(3);
    CHECK_TYPE(LONG); CHECK_TYPE(INT); CHECK_TYPE(VOID);
    return snprintf(dest, dmax, format, l, i, p);
}

int
snprintf_p1(char *dest, rsize_t dmax, const char *format, void *p)
{
    CHECK_FORMAT(1);
    CHECK_TYPE(VOID);
    return snprintf(dest, dmax, format, p);
}

int
snprintf_p2l1s1(char *dest, rsize_t dmax, const char *format, void *p1, void *p2, long l, const char *s)
{
    CHECK_FORMAT(4);
    CHECK_TYPE(VOID); CHECK_TYPE(VOID); CHECK_TYPE(LONG); CHECK_TYPE(STRING);
    return snprintf(dest, dmax, format, p1, p2, l, s);
}

int
snprintf_s1(char *dest, rsize_t dmax, const char *format, const char *s)
{
    CHECK_FORMAT(1);
    CHECK_TYPE(STRING);
    return snprintf(dest, dmax, format, s);
}


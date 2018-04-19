/*------------------------------------------------------------------
 * sprintf_s.h -- Safe Sprintf Interfaces
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
#ifndef SPRINTF_S_H_
#define SPRINTF_S_H_

#include <safe_lib_errno.h>

#define SNPRFNEGATE(x) (-1*(x))

#ifdef __cplusplus
extern "C" {
#endif

int snprintf_c1(char *dest, rsize_t dmax, const char *format, char c);
int snprintf_i1(char *dest, rsize_t dmax, const char *format, int i);
int snprintf_i2(char *dest, rsize_t dmax, const char *format, int i1, int i2);
int snprintf_i1s1(char *dest, rsize_t dmax, const char *format, int i, const char *s);
int snprintf_i1s1i1(char *dest, rsize_t dmax, const char *format, int i1, const char *s, int i2);
int snprintf_i1s1i2(char *dest, rsize_t dmax, const char *format, int i1, const char *s, int i2, int i3);
int snprintf_i1w(char *dest, rsize_t dmax, const char *format, int width, int i);
int snprintf_i4p1i4l1(char *dest, rsize_t dmax, const char *format, int i1, int i2, int i3, int i4, void *p,
                      int i5, int i6, int i7, int i8, long l);
int snprintf_l1(char *dest, rsize_t dmax, const char *format, long l);
int snprintf_l1i1p1(char *dest, rsize_t dmax, const char *format, long l, int i, void *p);
int snprintf_p1(char *dest, rsize_t dmax, const char *format, void *p);
int snprintf_p2l1s1(char *dest, rsize_t dmax, const char *format, void *p1, void *p2, long l, const char *s);
int snprintf_s1(char *dest, rsize_t dmax, const char *format, const char *s);

#ifdef __cplusplus
}
#endif

#endif /* SPRINTF_S_H_ */

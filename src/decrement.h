/*
 * Copyright (c) 2013-2014 Jean Niklas L'orange. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

// decrement hack: Cannot decrement in CPP stage directly.
// Insert value into DECREMENT, which will then be decremented once when you
// include this file

#if DECREMENT == 1
# undef  DECREMENT
# define DECREMENT 0
#elif DECREMENT == 2
# undef  DECREMENT
# define DECREMENT 1
#elif DECREMENT == 3
# undef  DECREMENT
# define DECREMENT 2
#elif DECREMENT == 4
# undef  DECREMENT
# define DECREMENT 3
#elif DECREMENT == 5
# undef  DECREMENT
# define DECREMENT 4
#elif DECREMENT == 6
# undef  DECREMENT
# define DECREMENT 5
#elif DECREMENT == 7
# undef  DECREMENT
# define DECREMENT 6
#elif DECREMENT == 8
# undef  DECREMENT
# define DECREMENT 7
#elif DECREMENT == 9
# undef  DECREMENT
# define DECREMENT 8
#elif DECREMENT == 10
# undef  DECREMENT
# define DECREMENT 9
#elif DECREMENT == 11
# undef  DECREMENT
# define DECREMENT 10
#elif DECREMENT == 12
# undef  DECREMENT
# define DECREMENT 11
#elif DECREMENT == 13
# undef  DECREMENT
# define DECREMENT 12
#elif DECREMENT == 14
# undef  DECREMENT
# define DECREMENT 13
#elif DECREMENT == 15
# undef  DECREMENT
# define DECREMENT 14
#elif DECREMENT == 16
# undef  DECREMENT
# define DECREMENT 15
#elif DECREMENT == 17
# undef  DECREMENT
# define DECREMENT 16
#elif DECREMENT == 18
# undef  DECREMENT
# define DECREMENT 17
#elif DECREMENT == 19
# undef  DECREMENT
# define DECREMENT 18
#elif DECREMENT == 20
# undef  DECREMENT
# define DECREMENT 19
#elif DECREMENT == 21
# undef  DECREMENT
# define DECREMENT 20
#elif DECREMENT == 22
# undef  DECREMENT
# define DECREMENT 21
#elif DECREMENT == 23
# undef  DECREMENT
# define DECREMENT 22
#elif DECREMENT == 24
# undef  DECREMENT
# define DECREMENT 23
#elif DECREMENT == 25
# undef  DECREMENT
# define DECREMENT 24
#elif DECREMENT == 26
# undef  DECREMENT
# define DECREMENT 25
#elif DECREMENT == 27
# undef  DECREMENT
# define DECREMENT 26
#elif DECREMENT == 28
# undef  DECREMENT
# define DECREMENT 27
#elif DECREMENT == 29
# undef  DECREMENT
# define DECREMENT 28
#elif DECREMENT == 30
# undef  DECREMENT
# define DECREMENT 29
#elif DECREMENT == 31
# undef  DECREMENT
# define DECREMENT 30
#elif DECREMENT == 32
# undef  DECREMENT
# define DECREMENT 31
#else
# error "Cannot decrement this value"
#endif

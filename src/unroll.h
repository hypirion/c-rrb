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

// Unroll hack:
// To unroll a piece of code, define WANTED_ITERATIONS and LOOP_BODY(i).
// Then include this file (unroll.h), and your deed is done.
// Supports up to 32 iterations.

#ifndef ITERATION
#	define ITERATION 0
#endif

#if ITERATION < WANTED_ITERATIONS
	LOOP_BODY(ITERATION)

// can't increment variable directly
#if ITERATION == 0
# undef  ITERATION
# define ITERATION 1
#elif ITERATION == 1
# undef  ITERATION
# define ITERATION 2
#elif ITERATION == 2
# undef  ITERATION
# define ITERATION 3
#elif ITERATION == 3
# undef  ITERATION
# define ITERATION 4
#elif ITERATION == 4
# undef  ITERATION
# define ITERATION 5
#elif ITERATION == 5
# undef  ITERATION
# define ITERATION 6
#elif ITERATION == 6
# undef  ITERATION
# define ITERATION 7
#elif ITERATION == 7
# undef  ITERATION
# define ITERATION 8
#elif ITERATION == 8
# undef  ITERATION
# define ITERATION 9
#elif ITERATION == 9
# undef  ITERATION
# define ITERATION 10
#elif ITERATION == 10
# undef  ITERATION
# define ITERATION 11
#elif ITERATION == 11
# undef  ITERATION
# define ITERATION 12
#elif ITERATION == 12
# undef  ITERATION
# define ITERATION 13
#elif ITERATION == 13
# undef  ITERATION
# define ITERATION 14
#elif ITERATION == 14
# undef  ITERATION
# define ITERATION 15
#elif ITERATION == 15
# undef  ITERATION
# define ITERATION 16
#elif ITERATION == 16
# undef  ITERATION
# define ITERATION 17
#elif ITERATION == 17
# undef  ITERATION
# define ITERATION 18
#elif ITERATION == 18
# undef  ITERATION
# define ITERATION 19
#elif ITERATION == 19
# undef  ITERATION
# define ITERATION 20
#elif ITERATION == 20
# undef  ITERATION
# define ITERATION 21
#elif ITERATION == 21
# undef  ITERATION
# define ITERATION 22
#elif ITERATION == 22
# undef  ITERATION
# define ITERATION 23
#elif ITERATION == 23
# undef  ITERATION
# define ITERATION 24
#elif ITERATION == 24
# undef  ITERATION
# define ITERATION 25
#elif ITERATION == 25
# undef  ITERATION
# define ITERATION 26
#elif ITERATION == 26
# undef  ITERATION
# define ITERATION 27
#elif ITERATION == 27
# undef  ITERATION
# define ITERATION 28
#elif ITERATION == 28
# undef  ITERATION
# define ITERATION 29
#elif ITERATION == 29
# undef  ITERATION
# define ITERATION 30
#elif ITERATION == 30
# undef  ITERATION
# define ITERATION 31
#elif ITERATION == 31
# undef  ITERATION
# define ITERATION 32
#else
# error "unroll.h doesn't support that many iterations"
#endif

#include "unroll.h"

#else
#	undef ITERATION
#	undef WANTED_ITERATIONS
#	undef LOOP_BODY
#endif

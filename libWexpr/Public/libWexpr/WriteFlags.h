//
/// \file libWexpr/WriteFlags.h
/// \brief Writing flags for Wexpr
//
// #LICENSE_BEGIN:MIT#
// 
// Copyright (c) 2017-2020, Kenneth Perry (thothonegan)
// 
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// 
// SPDX-License-Identifier: MIT
// #LICENSE_END#
//

#ifndef LIBWEXPR_WRITEFLAGS_H
#define LIBWEXPR_WRITEFLAGS_H

#include "Macros.h"

#include <stdint.h>

LIBWEXPR_EXTERN_C_BEGIN()

//
/// \brief These flags alter writing of wexpr strings.
//
typedef int8_t WexprWriteFlags;

enum
{
	WexprWriteFlagNone = 0, ///< No special flags
	WexprWriteFlagHumanReadable = (1 << 0U), ///< Instead of trying to compress down, will add newlines and indentation to make it more readable.
};

LIBWEXPR_EXTERN_C_END()

#endif // LIBWEXPR_WRITEFLAGS_H

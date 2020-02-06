//
/// \file libWexpr/ParseFlags.h
/// \brief Parsing flags for Wexpr
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

#ifndef LIBWEXPR_PARSEFLAGS_H
#define LIBWEXPR_PARSEFLAGS_H

#include "Macros.h"

#include <stdint.h>

LIBWEXPR_EXTERN_C_BEGIN()

//
/// \brief These flags alter parsing of wexpr on load.
//
typedef uint8_t WexprParseFlags;

enum
{
	WexprParseFlagNone = 0, ///< No special flags
	// flags are bitflags (0 << 1), (0 << 2), etc
};

LIBWEXPR_EXTERN_C_END()

#endif // LIBWEXPR_PARSEFLAGS_H

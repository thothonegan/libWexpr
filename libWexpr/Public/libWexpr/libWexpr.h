//
/// \file libWexpr/libWexpr.h
/// \brief Includes all Wexpr files
//
// #LICENSE_BEGIN:MIT#
// 
// Copyright (c) 2017-2019, Kenneth Perry (thothonegan)
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

#ifndef LIBWEXPR_LIBWEXPR_H
#define LIBWEXPR_LIBWEXPR_H

#include "Endian.h"
#include "Error.h"
#include "Expression.h"
#include "ExpressionType.h"
#include "Macros.h"
#include "ParseFlags.h"
#include "UVLQ64.h"
#include "WriteFlags.h"

#define LIBWEXPR_VERSION_MAJOR 1
#define LIBWEXPR_VERSION_MINOR 0
#define LIBWEXPR_VERSION_PATCH 0

LIBWEXPR_EXTERN_C_BEGIN()

LIBWEXPR_PUBLIC int wexpr_Version_major ();
LIBWEXPR_PUBLIC int wexpr_Version_minor ();
LIBWEXPR_PUBLIC int wexpr_Version_patch ();

LIBWEXPR_EXTERN_C_END()

#endif // LIBWEXPR_LIBWEXPR_H

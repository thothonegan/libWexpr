//
/// \file libWexpr/ExpressionType.h
/// \brief A type of expression
//
// #LICENSE_BEGIN:MIT#
// 
// Copyright (c) 2017-2018, Kenneth Perry (thothonegan)
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
// #LICENSE_END#
//

#ifndef LIBWEXPR_EXPRESSIONTYPE_H
#define LIBWEXPR_EXPRESSIONTYPE_H

#include "Macros.h"

#include <stdint.h>

LIBWEXPR_EXTERN_C_BEGIN()

//
/// \brief An expression type. Marks the type of an expression
//
typedef uint8_t WexprExpressionType;

// These are the different types an expression can be.
// These numbers are also used in binary formats as needed.
//
enum
{
	WexprExpressionTypeNull  = 0x00, ///< Null expression (null / nil)
	WexprExpressionTypeValue = 0x01, ///< A value. Can be a number, quoted, or token. Must be UTF-8 safe.
	WexprExpressionTypeArray = 0x02, ///< An array of items where order matters.
	WexprExpressionTypeMap   = 0x03,  ///< An array of items where each pair is a key/value pair. Not ordered. Keys are required to be values currently.
	WexprExpressionTypeBinaryData = 0x04, ///< A set of binary data, which is encoded in text format as base64.
	
	WexprExpressionTypeInvalid = 0xFF ///< Invalid expression - not filled in or usable
};

//
/// \brief Return an expressiontype as a string
/// Will return "Null", "Value", "Map", "Array", etc as needed. Returns NULL if no entry found.
//
LIBWEXPR_PUBLIC const char* wexpr_ExpressionType_toString (WexprExpressionType self);

LIBWEXPR_EXTERN_C_END()

#endif // LIBWEXPR_EXPRESSIONTYPE_H

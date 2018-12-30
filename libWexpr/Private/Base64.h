//
/// \file libWexpr/Base64.h
/// \brief Base64 support
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

#ifndef LIBWEXPR_BASE64_H
#define LIBWEXPR_BASE64_H

#include <stddef.h>

typedef struct Base64IBuffer
{
	const void* buffer; // the start of the buffer. nullptr if invalid.
	size_t size; // the size of the buffer in bytes
} Base64IBuffer;

typedef struct Base64Buffer
{
	void* buffer; // the start of the buffer. nullptr if invalid. Owned.
	size_t size; // the size of the buffer in bytes
} Base64Buffer;

//
/// \brief Decode the given string encoded in Base64. You own the new buffer.
//
Base64Buffer base64_decode (Base64IBuffer buf);

//
/// \brief Encode the given buffer as a Base64 string. You own the new buffer.
//
Base64Buffer base64_encode (Base64IBuffer buf);

#endif // LIBWEXPR_BASE64_H

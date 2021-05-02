//
/// \file libWexpr/UVLQ64.h
/// \brief UVLQ64 helpers
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

#ifndef LIBWEXPR_UVLQ64_H
#define LIBWEXPR_UVLQ64_H

#include "Endian.h"

#include <stdint.h>

//
/// \brief Return the number of bytes which is needed to store a value in the UVLQ64.
//
static inline size_t wexpr_uvlq64_bytesize(uint64_t value)
{
	// we get 7 bits per byte. 2^7 for each
	static const uint64_t v2_to_7  = 2*2*2*2*2*2*2;      // 2^7
	static const uint64_t v2_to_14 = v2_to_7 * v2_to_7;  // 2^14
	static const uint64_t v2_to_21 = v2_to_14 * v2_to_7; // 2^21
	static const uint64_t v2_to_28 = v2_to_21 * v2_to_7; // 2^28
	static const uint64_t v2_to_35 = v2_to_28 * v2_to_7; // 2^35
	static const uint64_t v2_to_42 = v2_to_35 * v2_to_7; // 2^42
	static const uint64_t v2_to_49 = v2_to_42 * v2_to_7; // 2^49
	static const uint64_t v2_to_56 = v2_to_49 * v2_to_7; // 2^56
	static const uint64_t v2_to_63 = v2_to_56 * v2_to_7; // 2^63
	
	if (value < v2_to_7)  { return 1; } // NOLINT: reasonable magic number
	if (value < v2_to_14) { return 2; } // NOLINT: reasonable magic number
	if (value < v2_to_21) { return 3; } // NOLINT: reasonable magic number
	if (value < v2_to_28) { return 4; } // NOLINT: reasonable magic number
	if (value < v2_to_35) { return 5; } // NOLINT: reasonable magic number
	if (value < v2_to_42) { return 6; } // NOLINT: reasonable magic number
	if (value < v2_to_49) { return 7; } // NOLINT: reasonable magic number
	if (value < v2_to_56) { return 8; } // NOLINT: reasonable magic number
	if (value < v2_to_63) { return 9; } // NOLINT: reasonable magic number
	
	return 10; // NOLINT: reasonable magic number // 2^64+
}

//
/// \brief Write a UVLQ64 (big endian) to the given buffer.
/// \param buffer The buffer to read from
/// \param bufferSize The number of bytes we're allowed to read from the buffer (at max).
/// \param value The value to write
/// \return 1 on success, 0 on failure (generally invalid buffer).
//
static inline int wexpr_uvlq64_write (uint8_t* buffer, size_t bufferSize, uint64_t value)
{
	size_t bytesNeeded = wexpr_uvlq64_bytesize(value);
	if (bufferSize < bytesNeeded) { return 0; }
	
	size_t i = bytesNeeded - 1;
	for (size_t j=0; j <= i; ++j)
	{
		buffer[j] = ((value >> ((i - j) * 7)) & 127) | 128;
	}
	
	buffer[i] ^= 128;
	return 1;
}

//
/// \brief Read a UVLQ64 (big endian) from the given buffer.
/// \param buffer The buffer to read from
/// \param bufferSize The number of bytes we're allowed to read from the buffer (at max).
/// \param outValue (not null) The place to store the value read.
/// \return Pointer to first byte in the buffer we didnt read. Or NULL if failed.
//
static inline const uint8_t* wexpr_uvlq64_read (const uint8_t* buffer, size_t bufferSize, uint64_t* outValue)
{
	uint64_t r = 0;
	
	do {
		if (bufferSize == 0) return LIBWEXPR_NULLPTR;
		r = (r << 7) | LIBWEXPR_STATICCAST(uint64_t, (*buffer & 127));
		--bufferSize;
	} while (*buffer++ & 128);
	
	*outValue = r;
	return buffer;
}

#endif // LIBWEXPR_UVLQ64_H

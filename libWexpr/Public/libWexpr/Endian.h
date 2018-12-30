//
/// \file libWexpr/Endian.h
/// \brief Simple endian helpers
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

#ifndef LIBWEXPR_ENDIAN_H
#define LIBWEXPR_ENDIAN_H

#include <stdint.h>

#define LIBWEXPR_ENDIAN_ISBIG 0

#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__)
	#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		#undef LIBWEXPR_ENDIAN_ISBIG
		#define LIBWEXPR_ENDIAN_ISBIG 1
	#endif
#endif

#ifdef __cplusplus
	#define WEXPR_REINTERP_CAST(Type, val) reinterpret_cast<Type>(val)
#else
	#define WEXPR_REINTERP_CAST(Type, val) ((Type)val)
#endif

#ifdef _MSC_VER
	#include <stdlib.h>
#endif

//
/// \brief Swap a uint16 bytes
/// \param v The value to swap
/// \return The swapped value
//
static inline uint16_t wexpr_uint16Swap(uint16_t v)
{
	typedef union swap { uint16_t v; uint8_t b[2]; } swap;
	
	swap* s = WEXPR_REINTERP_CAST(swap*, &v);
	swap r;
	r.b[0] = s->b[1];
	r.b[1] = s->b[0];
	
	return r.v; 
}

//
/// \brief Convert a native uint16 to big
/// \param v The value to swap
/// \return The swapped value
//
static inline uint16_t wexpr_uint16ToBig(uint16_t v)
{
	#if LIBWEXPR_ENDIAN_ISBIG
		return v;
	#else
		return wexpr_uint16Swap(v);
	#endif
}

//
/// \brief Convert a big uint16 to native
/// \param v The value to swap
/// \return The swapped value
//
static inline uint16_t wexpr_bigUInt16ToNative (uint16_t v)
{
	#if LIBWEXPR_ENDIAN_ISBIG
		return v;
	#else
		return wexpr_uint16Swap(v);
	#endif
}

//
/// \brief Swap a uint32 bytes
/// \param v The value to swap
/// \return The swapped value
//
static inline uint32_t wexpr_uint32Swap (uint32_t v)
{
	// NOTE: msvc's 32bit swap isn't guaranteed to be 32bit
	#if defined(__clang__) || defined(__GNUC__)
		return __builtin_bswap32(v);
	#else // c method
		return
			( v >> 24) |
			((v << 8) & 0x00FF0000) |
			((v >> 8) & 0x0000FF00) |
			( v << 24)
		;
	#endif
}

//
/// \brief Convert a native uint32 to big
/// \param v The value to swap
/// \return The swapped value
//
static inline uint32_t wexpr_uint32ToBig(uint32_t v)
{
	#if LIBWEXPR_ENDIAN_ISBIG
		return v;
	#else
		return wexpr_uint32Swap(v);
	#endif
}

//
/// \brief Convert a big uint32 to native
/// \param v The value to swap
/// \return The swapped value
//
static inline uint32_t wexpr_bigUInt32ToNative (uint32_t v)
{
	#if LIBWEXPR_ENDIAN_ISBIG
		return v;
	#else
		return wexpr_uint32Swap(v);
	#endif
}


//
/// \brief Swap a uint64 bytes
/// \param v The value to swap
/// \return The swapped value
//
static inline uint64_t wexpr_uint64Swap (uint64_t v)
{
	#if defined(__clang__) || defined(__GNUC__)
		return __builtin_bswap64(v);
	#elif defined(_MSC_VER)
		return _byteswap_uint64 (v);
	#else // c method
		return
			( v >> 56) ||
			((v << 40) & 0x00FF000000000000ull) |
			((v << 24) & 0x0000FF0000000000ull) |
			((v << 8 ) & 0x000000FF00000000ull) |
			((v >> 8 ) & 0x00000000FF000000ull) |
			((v >> 24) & 0x0000000000FF0000ull) |
			((v >> 40) & 0x000000000000FF00ull) |
			( v << 56)
		;
	#endif
}

//
/// \brief Convert a native uint64 to big
/// \param v The value to convert as native
/// \return The value converted as big endian.
//
static inline uint64_t wexpr_uint64ToBig(uint64_t v)
{
	#if LIBWEXPR_ENDIAN_ISBIG
		return v;
	#else
		return wexpr_uint64Swap(v);
	#endif
}

//
/// \brief Convert a big uint64 to native
/// \param v The value to convert, in big endian.
/// \return The converted value as native
//
static inline uint64_t wexpr_bigUInt64ToNative (uint64_t v)
{
	#if LIBWEXPR_ENDIAN_ISBIG
		return v;
	#else
		return wexpr_uint64Swap(v);
	#endif
}

#undef WEXPR_REINTERP_CAST

#endif // LIBWEXPR_ENDIAN_H

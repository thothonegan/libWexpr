//
/// \file libWexpr/Macros.h
/// \brief Wexpr macros
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

#ifndef LIBWEXPR_MACROS_H
#define LIBWEXPR_MACROS_H

#ifdef __cplusplus
	#define LIBWEXPR_EXTERN_C_BEGIN() extern "C" {
	#define LIBWEXPR_EXTERN_C_END() }
	
	#define LIBWEXPR_STATICCAST(type, val) static_cast<type>(val)
	
	#if __cplusplus >= 201103L
		#define LIBWEXPR_NULLPTR nullptr
	#else
		#define LIBWEXPR_NULLPTR NULL
	#endif
#else
	#define LIBWEXPR_EXTERN_C_BEGIN()
	#define LIBWEXPR_EXTERN_C_END()
	#define LIBWEXPR_STATICCAST(type, val) ((type) (val))
	#define LIBWEXPR_NULLPTR NULL
#endif

// similar to wolf macros
#if defined(_WIN32)
	#define LIBWEXPR_EXPORT __declspec(dllexport)
	#define LIBWEXPR_IMPORT __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
	#define LIBWEXPR_EXPORT __attribute__ ((visibility ("default") ))
	#define LIBWEXPR_IMPORT
#else // unknown/unneeded
	#define LIBWEXPR_EXPORT
	#define LIBWEXPR_IMPORT
#endif

#if defined(_MSC_VER)
	#define LIBWEXPR_STRDUP(var) _strdup(var)
#else
	#define LIBWEXPR_STRDUP(var) strdup(var)
#endif

// LIBWEXPR_PUBLIC will export/import as needed

#if defined(CATALYST_libWexpr_IS_SHARED_LIBRARY)
	#if defined(CATALYST_libWexpr_IS_BUILDING)
		#define LIBWEXPR_PUBLIC LIBWEXPR_EXPORT
	#else
		#define LIBWEXPR_PUBLIC LIBWEXPR_IMPORT
	#endif
#else // not shared
	#define LIBWEXPR_PUBLIC
#endif

#endif // LIBWEXPR_MACROS_H

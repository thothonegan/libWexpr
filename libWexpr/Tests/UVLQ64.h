//
/// \file UVLQ64.h
/// \brief ExpressionType tests
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

#ifndef WEXPR_TESTS_UVLQ64_H
#define WEXPR_TESTS_UVLQ64_H

#include <libWexpr/UVLQ64.h>

#include "UnitTest.h"
WEXPR_UNITTEST_BEGIN(UVLQ64CanEncodeDecode)
	// simple tests
	uint8_t tempBuffer[10];
	
	uint64_t x[] = { 0x7f, 0x4000, 0, 0x3ffffe, 0x1fffff, 0x200000, 0x3311a1234df31413ULL};
	for (int j=0; j < sizeof(x)/sizeof(uint64_t); ++j)
	{
		int writeResult = wexpr_uvlq64_write (tempBuffer, sizeof(tempBuffer), x[j]);
		WEXPR_UNITTEST_ASSERT (writeResult, "Unable to write");
		
		uint64_t out;
		const uint8_t* readResult = wexpr_uvlq64_read (tempBuffer, sizeof(tempBuffer), &out);
		WEXPR_UNITTEST_ASSERT (readResult != NULL, "Unable to read");
		WEXPR_UNITTEST_ASSERT (out == x[j], "Not correct");
	}
	
WEXPR_UNITTEST_END()

WEXPR_UNITTEST_SUITE_BEGIN (UVLQ64)
	WEXPR_UNITTEST_SUITE_ADDTEST (UVLQ64, UVLQ64CanEncodeDecode);
WEXPR_UNITTEST_SUITE_END ()

#endif // WEXPR_TESTS_UVLQ64_H

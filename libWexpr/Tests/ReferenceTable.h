//
/// \file ReferenceTable.h
/// \brief ReferenceTable tests
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

#ifndef WEXPR_TESTS_REFERENCETABLE_H
#define WEXPR_TESTS_REFERENCETABLE_H

#include <libWexpr/ReferenceTable.h>

#include "UnitTest.h"

WEXPR_UNITTEST_BEGIN (ReferenceTableCanCreate)

	WexprReferenceTable* table = wexpr_ReferenceTable_create ();
	
	WEXPR_UNITTEST_ASSERT (wexpr_ReferenceTable_count(table) == 0, "Default table should have 0 items");
	
	WEXPR_UNITTEST_ASSERT (wexpr_ReferenceTable_expressionForKey(table, "unknown") == LIBWEXPR_NULLPTR, "Invalid key should return null");
	wexpr_ReferenceTable_destroy (table);
	
WEXPR_UNITTEST_END ()

WEXPR_UNITTEST_BEGIN (ReferenceTableCanSetKey)

	WexprReferenceTable* table = wexpr_ReferenceTable_create ();
	
	WexprExpression* val = wexpr_Expression_createValue ("asdf");
	
	wexpr_ReferenceTable_setExpressionForKey (table, "key", val); // transfer ownership
	
	WexprExpression* valFromRefTable = wexpr_ReferenceTable_expressionForKey (table, "key");
	
	WEXPR_UNITTEST_ASSERT (valFromRefTable, "Got back from table");
	WEXPR_UNITTEST_ASSERT (strcmp(wexpr_Expression_value(valFromRefTable), "asdf") == 0, "value is correct");
	
	wexpr_ReferenceTable_destroy (table);
	
WEXPR_UNITTEST_END ()

WEXPR_UNITTEST_SUITE_BEGIN (ReferenceTable)
	WEXPR_UNITTEST_SUITE_ADDTEST (ReferenceTable, ReferenceTableCanCreate);
	WEXPR_UNITTEST_SUITE_ADDTEST (ReferenceTable, ReferenceTableCanSetKey);
WEXPR_UNITTEST_SUITE_END ()

#endif // WEXPR_TESTS_REFERENCETABLE_H

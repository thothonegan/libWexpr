//
/// \file ExpressionErrors.h
/// \brief Expression tests for Errors
///
/// #LICENSE_BEGIN:MIT#
/// 
/// Copyright (c) 2017-2018, Kenneth Perry (thothonegan)
/// 
/// Permission is hereby granted, free of charge, to any person obtaining
/// a copy of this software and associated documentation files (the
/// "Software"), to deal in the Software without restriction, including
/// without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to
/// permit persons to whom the Software is furnished to do so, subject to
/// the following conditions:
/// 
/// The above copyright notice and this permission notice shall be
/// included in all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
/// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
/// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
/// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
/// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
/// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
/// 
/// #LICENSE_END#
//

#ifndef WEXPR_TESTS_EXPRESSIONERRORS_H
#define WEXPR_TESTS_EXPRESSIONERRORS_H

#include <libWexpr/Expression.h>

#include <stdbool.h>

#include "UnitTest.h"

// Testing parsing errors being reported properly
// See Expression.h for normal testing

WEXPR_UNITTEST_BEGIN (ExpressionErrorsEmptyIsInvalid)

	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* valueExpr = wexpr_Expression_createFromString("", WexprParseFlagNone, &err);
	
	WEXPR_UNITTEST_ASSERT (!valueExpr, "Shouldnt generate expression");
	WEXPR_UNITTEST_ASSERT (err.code == WexprErrorCodeEmptyString, "Empty string error");
	WEXPR_UNITTEST_ASSERT (err.line == 1 && err.column == 1, "Position should be right");
	
	WEXPR_ERROR_FREE (err);
	
WEXPR_UNITTEST_END ()

WEXPR_UNITTEST_BEGIN (ExpressionErrorsExtraDataAfterExpression)

	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* valueExpr = wexpr_Expression_createFromString("#(1) 1", WexprParseFlagNone, &err);
	
	WEXPR_UNITTEST_ASSERT (!valueExpr, "Shouldnt generate expression");
	WEXPR_UNITTEST_ASSERT (err.code == WexprErrorCodeExtraDataAfterParsingRoot, "Extra data after root");
	WEXPR_UNITTEST_ASSERT (err.line == 1 && err.column == 6, "Position should be right");
	
	WEXPR_ERROR_FREE (err);
	
WEXPR_UNITTEST_END ()

WEXPR_UNITTEST_BEGIN (ExpressionErrorsArrayMissingEndParen)

	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* valueExpr = wexpr_Expression_createFromString("#(", WexprParseFlagNone, &err);
	
	WEXPR_UNITTEST_ASSERT (!valueExpr, "Shouldnt generate expression");
	WEXPR_UNITTEST_ASSERT (err.code == WexprErrorCodeArrayMissingEndParen, "Missing an ending paren");
	WEXPR_UNITTEST_ASSERT (err.line == 1 && err.column == 3, "Position should be right");

	WEXPR_ERROR_FREE (err);
	
WEXPR_UNITTEST_END ()

WEXPR_UNITTEST_BEGIN (ExpressionErrorsMapMissingEndParen)

	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* valueExpr = wexpr_Expression_createFromString("@(", WexprParseFlagNone, &err);
	
	WEXPR_UNITTEST_ASSERT (!valueExpr, "Shouldnt generate expression");
	WEXPR_UNITTEST_ASSERT (err.code == WexprErrorCodeMapMissingEndParen, "Missing an ending paren");
	WEXPR_UNITTEST_ASSERT (err.line == 1 && err.column == 3, "Position should be right");
	
	WEXPR_ERROR_FREE (err);
WEXPR_UNITTEST_END ()

WEXPR_UNITTEST_BEGIN (ExpressionErrorsMapKeysMustBeValues)

	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* valueExpr = wexpr_Expression_createFromString("@(#() a)", WexprParseFlagNone, &err);
	
	WEXPR_UNITTEST_ASSERT (!valueExpr, "Shouldnt generate expression");
	WEXPR_UNITTEST_ASSERT (err.code == WexprErrorCodeMapKeyMustBeAValue, "Keys must be values");
	WEXPR_UNITTEST_ASSERT (err.line == 1 && err.column == 3, "Position should be right");
	
	WEXPR_ERROR_FREE (err);
WEXPR_UNITTEST_END ()

WEXPR_UNITTEST_BEGIN (ExpressionErrorsReferenceMissingItsEndingBracket)

	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* valueExpr = wexpr_Expression_createFromString("[", WexprParseFlagNone, &err);
	
	WEXPR_UNITTEST_ASSERT (!valueExpr, "Shouldnt generate expression");
	WEXPR_UNITTEST_ASSERT (err.code == WexprErrorCodeReferenceMissingEndBracket, "Ref needs an ending bracket");
	WEXPR_UNITTEST_ASSERT (err.line == 1 && err.column == 1, "Position should be right");
	
	WEXPR_ERROR_FREE (err);
WEXPR_UNITTEST_END ()

WEXPR_UNITTEST_BEGIN (ExpressionErrorsReferenceInvalid)
	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* valueExpr = wexpr_Expression_createFromString("*[asdf]", WexprParseFlagNone, &err);
	
	WEXPR_UNITTEST_ASSERT (!valueExpr, "Shouldnt generate expression");
	WEXPR_UNITTEST_ASSERT (err.code == WexprErrorCodeReferenceUnknownReference, "Invalid ref");
	WEXPR_UNITTEST_ASSERT (err.line == 1 && err.column == 8, "Position should be right");
	
	WEXPR_ERROR_FREE (err);
WEXPR_UNITTEST_END ()

WEXPR_UNITTEST_BEGIN (ExpressionErrorsBlankIsError)
	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* valueExpr = wexpr_Expression_createFromString("", WexprParseFlagNone, &err);
	
	WEXPR_UNITTEST_ASSERT (!valueExpr, "Shouldnt generate expression");
	WEXPR_UNITTEST_ASSERT (err.code == WexprErrorCodeEmptyString, "Empty string");
	WEXPR_UNITTEST_ASSERT (err.line == 1 && err.column == 1, "Position should be right");
	
	WEXPR_ERROR_FREE (err);
WEXPR_UNITTEST_END ()

WEXPR_UNITTEST_BEGIN (ExpressionErrorsJustCommentIsError)
	WexprError err = WEXPR_ERROR_INIT();
	WexprExpression* valueExpr = wexpr_Expression_createFromString(" ;(-- asdf --)  ", WexprParseFlagNone, &err);
	
	WEXPR_UNITTEST_ASSERT (!valueExpr, "Shouldnt generate expression");
	WEXPR_UNITTEST_ASSERT (err.code == WexprErrorCodeEmptyString, "Empty string");
	WEXPR_UNITTEST_ASSERT (err.line == 1 && err.column == 17, "Position should be right");
	
	WEXPR_ERROR_FREE (err);
WEXPR_UNITTEST_END ()

WEXPR_UNITTEST_SUITE_BEGIN (ExpressionErrors)
	WEXPR_UNITTEST_SUITE_ADDTEST (ExpressionErrors, ExpressionErrorsEmptyIsInvalid);
	WEXPR_UNITTEST_SUITE_ADDTEST (ExpressionErrors, ExpressionErrorsExtraDataAfterExpression);
	WEXPR_UNITTEST_SUITE_ADDTEST (ExpressionErrors, ExpressionErrorsArrayMissingEndParen);
	WEXPR_UNITTEST_SUITE_ADDTEST (ExpressionErrors, ExpressionErrorsMapMissingEndParen);
	WEXPR_UNITTEST_SUITE_ADDTEST (ExpressionErrors, ExpressionErrorsMapKeysMustBeValues);
	WEXPR_UNITTEST_SUITE_ADDTEST (ExpressionErrors, ExpressionErrorsReferenceMissingItsEndingBracket);
	WEXPR_UNITTEST_SUITE_ADDTEST (ExpressionErrors, ExpressionErrorsReferenceInvalid);
	WEXPR_UNITTEST_SUITE_ADDTEST (ExpressionErrors, ExpressionErrorsBlankIsError);
	WEXPR_UNITTEST_SUITE_ADDTEST (ExpressionErrors, ExpressionErrorsJustCommentIsError);
WEXPR_UNITTEST_SUITE_END ()

#endif // WEXPR_TESTS_EXPRESSIONERRORS_H

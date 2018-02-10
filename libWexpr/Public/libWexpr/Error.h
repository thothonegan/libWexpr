//
/// \file libWexpr/Error.h
/// \brief An error
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

#ifndef LIBWEXPR_ERROR_H
#define LIBWEXPR_ERROR_H

#include "Macros.h"

#include <stdint.h>

LIBWEXPR_EXTERN_C_BEGIN()

//
/// \brief A numeric code to represent the class of error.
/// This offers a basic level of detail.
//
typedef uint8_t WexprErrorCode;

// Possible error codes
enum
{
	WexprErrorCodeNone, ///< Not an error
	WexprErrorCodeStringMissingEndingQuote, ///< A string with a quote is missing the end quote
	WexprErrorCodeInvalidUTF8, ///< UTF8 was invalid.
	WexprErrorCodeExtraDataAfterParsingRoot, ///< Got extra data after we parsed the first object from the wexpr.
	WexprErrorCodeEmptyString, ///< An empty string was given when we require one.
	WexprErrorCodeMapMissingEndParen, ///< Parsing a map its missing the ending paren
	WexprErrorCodeMapKeyMustBeAValue, ///< Map keys must be a value
	WexprErrorCodeReferenceMissingEndBracket, ///< A reference is missing an end bracket
	WexprErrorCodeReferenceInsertMissingEndBracket, ///< A reference we tried to insert is missing an end bracket
	WexprErrorCodeReferenceUnknownReference ///< Tried to look for a reference, but it didn't exist.
};

//
/// \brief The actual error.
/// An error consists of two parts - a code which is generally the failure, and a string which can pinpoint the specific reason for the failure.
//
typedef struct WexprError
{
	WexprErrorCode code; ///< The general code for the error.
	char* message; ///< Must be freed if set. See WEXPR_ERROR_FREE()
} WexprError;

//
/// \brief Macro which creates a default non-error
/// Use this to create your errors:
///   WexprError err = WEXPR_ERROR_INIT();
//
#define WEXPR_ERROR_INIT() { WexprErrorCodeNone, NULL }

//
/// \brief Macro which frees an error. Call when done with the error variable, will cleanup as needed or not.
//
#define WEXPR_ERROR_FREE(err) \
	if (err.message) { free(err.message); err.message = NULL; }

LIBWEXPR_EXTERN_C_END()

#endif // LIBWEXPR_ERROR_H

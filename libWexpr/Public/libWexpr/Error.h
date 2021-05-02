//
/// \file libWexpr/Error.h
/// \brief An error
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

//
/// Possible error codes
/// \see WexprErrorCode
//
enum
{
	WexprErrorCodeNone, ///< Not an error
	WexprErrorCodeStringMissingEndingQuote, ///< A string with a quote is missing the end quote
	WexprErrorCodeInvalidUTF8, ///< UTF8 was invalid.
	WexprErrorCodeExtraDataAfterParsingRoot, ///< Got extra data after we parsed the first object from the wexpr.
	WexprErrorCodeEmptyString, ///< An empty string was given when we require one.
	WexprErrorCodeInvalidStringEscape, ///< A string contained an invalid escape
	WexprErrorCodeMapMissingEndParen, ///< Parsing a map its missing the ending paren
	WexprErrorCodeMapKeyMustBeAValue, ///< Map keys must be a value
	WexprErrorCodeMapNoValue, ///< A key had no value before the map ended
	WexprErrorCodeReferenceMissingEndBracket, ///< A reference is missing an end bracket
	WexprErrorCodeReferenceInsertMissingEndBracket, ///< A reference we tried to insert is missing an end bracket
	WexprErrorCodeReferenceUnknownReference, ///< Tried to look for a reference, but it didn't exist.
	WexprErrorCodeArrayMissingEndParen, ///< Tried to find the ending paren, but it didn't exist.
	WexprErrorCodeReferenceInvalidName, ///< A reference has an invalid character in it
	WexprErrorCodeBinaryDataNoEnding, ///< Binary data had no ending <
	WexprErrorCodeBinaryDataInvalidBase64, ///< Unable to parse the base64 data
	
	WexprErrorCodeBinaryInvalidHeader, ///< The binary header didn't make sense
	WexprErrorCodeBinaryUnknownVersion, ///< The version was unknown
	WexprErrorCodeBinaryMultipleExpressions, ///< Found multiple expression chunks
	WexprErrorCodeBinaryChunkBiggerThanData, ///< The chunk size said to expand past the buffer size
	WexprErrorCodeBinaryChunkNotBigEnough, ///< The length of buffer given wasnt't big enough for a valid chunk.
	WexprErrorCodeBinaryUnknownCompression ///< Unknown compression method received
};

typedef uint32_t WexprLineNumber;
typedef uint32_t WexprColumnNumber;

//
/// \brief The actual error.
/// An error consists of two parts - a code which is generally the failure, and a string which can pinpoint the specific reason for the failure.
/// To use this:
/// \code{.c}
///
/// WexprError err = WEXPR_ERROR_INIT();
/// /* use */
/// WEXPR_ERROR_FREE (err);
///
/// \endcode
//
typedef struct WexprError
{
	WexprErrorCode code; ///< The general code for the error.
	char* message; ///< Must be freed if set. See WEXPR_ERROR_FREE()
	WexprLineNumber line; ///< Line number of the error. 0 if unknown.
	WexprColumnNumber column; ///< Column number of the error. 0 if unknown.
} WexprError;

//
/// \brief Move an error to another error (c++ style)
/// \relates WexprError
//
#define WEXPR_ERROR_MOVE(dest, source) do { \
	(dest)->code = (source)->code; \
	(dest)->message = (source)->message; (source)->message = LIBWEXPR_NULLPTR; \
	(dest)->line = (source)->line; \
	(dest)->column = (source)->column; \
} while (0)

//
/// \brief Macro which creates a default non-error
/// Use this to create your errors:
///   WexprError err = WEXPR_ERROR_INIT();
/// \relates WexprError
//
#define WEXPR_ERROR_INIT() { WexprErrorCodeNone, LIBWEXPR_NULLPTR, 0, 0 }

//
/// \brief Macro which frees an error. Call when done with the error variable, will cleanup as needed or not.
/// \relates WexprError
//
#define WEXPR_ERROR_FREE(err) \
	do { \
		if ( (err).message) { free( (err).message); (err).message = LIBWEXPR_NULLPTR; } \
	} while (0)

LIBWEXPR_EXTERN_C_END()

#endif // LIBWEXPR_ERROR_H

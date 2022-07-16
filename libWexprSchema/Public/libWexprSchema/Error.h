//
/// \file libWexprSchema/Error.h
/// \brief A schema error
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

#ifndef LIBWEXPRSCHEMA_ERROR_H
#define LIBWEXPRSCHEMA_ERROR_H

#include "Macros.h"

#include <stdint.h>

LIBWEXPR_EXTERN_C_BEGIN()

//
/// \brief A numeric code to represent the error type.
//
typedef uint8_t WexprSchemaErrorCode;

//
/// \brief Possible error codes
/// \see WexprSchemaErrorCode
//
enum
{
	WexprSchemaErrorCodeNone, ///< Not an error

	WexprSchemaErrorInternal, ///< An internal/unspecified error occurred.

};

//
/// \brief The schema error
//
typedef struct WexprSchemaError WexprSchemaError;

/// \name Construction/Destruction
/// \relates WexprSchemaError
/// \{


//
/// \brief Create a new schema error containing the given code, objectPath, and message.
//
WexprSchemaError* wexprSchema_Error_create(
	WexprSchemaErrorCode code,
	const char* objectPath,
	const char* message,
	WexprSchemaError* nextErrorIfAny
);

//
/// \brief Destroy an existing schema error
//
void wexprSchema_Error_destroy(WexprSchemaError* self);

/// \}

/// \name Properties
/// \relates WexprSchemaError
/// \{

//
/// \brief Return the error code
//
WexprSchemaErrorCode wexprSchema_Error_code(WexprSchemaError* self);

//
/// \brief Return the object path for the error, if any
//
const char* wexprSchema_Error_objectPath(WexprSchemaError* self);

//
/// \brief Return the message for the error, if any
//
const char* wexprSchema_Error_message(WexprSchemaError* self);

//
/// \brief Return the next error in the chain, if any
//
WexprSchemaError* wexprSchema_Error_nextError(WexprSchemaError* self);

/// \}

/// \name Error Chain
/// \relates WexprSchemaError
/// \{

//
/// \brief Append the given error to the end of the chain. Will take ownership.
//
void wexprSchema_Error_appendError(WexprSchemaError* self, WexprSchemaError* errorToAppend);

/// \}

LIBWEXPR_EXTERN_C_END()

#endif // LIBWEXPRSCHEMA_ERROR_H

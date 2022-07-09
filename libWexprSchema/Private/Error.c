//
/// \file libWexprSchema/Error.c
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

#include <libWexprSchema/Error.h>

#include <stdlib.h>
#include <string.h>

// --- structures

struct WexprSchemaError
{
	WexprSchemaErrorCode m_code; ///< The general code for the error
	char* m_objectPath; ///< Must be freed if set
	char* m_message; ///< Must be freed if set.
	WexprSchemaError* m_nextError; ///< If set, the next error in the chain and is owned by this error
};

// --- public Construction/Destruction

WexprSchemaError* wexprSchema_Error_create(
	WexprSchemaErrorCode code,
	const char* objectPath,
	const char* message
)
{
	WexprSchemaError* self = malloc(sizeof(WexprSchemaError));
	self->m_code = code;
	self->m_objectPath = strdup(objectPath);
	self->m_message = strdup(message);
	self->m_nextError = LIBWEXPR_NULLPTR;

	return self;
}

void wexprSchema_Error_destroy(WexprSchemaError* self)
{
	if (self->m_nextError)
	{
		wexprSchema_Error_destroy(self->m_nextError);
		self->m_nextError = LIBWEXPR_NULLPTR;
	}

	if (self->m_objectPath)
	{
		free(self->m_objectPath);
		self->m_objectPath = LIBWEXPR_NULLPTR;
	}

	if (self->m_message)
	{
		free(self->m_message);
		self->m_message = LIBWEXPR_NULLPTR;
	}

	free(self);
}

// --- public Properties

WexprSchemaErrorCode wexprSchema_Error_code (WexprSchemaError* self)
{
	return self->m_code;
}

const char* wexprSchema_Error_objectPath (WexprSchemaError* self)
{
	return self->m_objectPath;
}

const char* wexprSchema_Error_message (WexprSchemaError* self)
{
	return self->m_message;
}

WexprSchemaError* wexprSchema_Error_nextError(WexprSchemaError* self)
{
	return self->m_nextError;
}

// --- public Error Chain

void wexprSchema_Error_appendError(WexprSchemaError* self, WexprSchemaError* errorToAppend)
{
	if (self->m_nextError)
	{
		return wexprSchema_Error_appendError(self->m_nextError, errorToAppend);
	}

	self->m_nextError = errorToAppend;
}

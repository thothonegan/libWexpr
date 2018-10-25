//
/// \file libWexpr/ReferenceTable.h
/// \brief A table of expressions given names
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

#ifndef LIBWEXPR_REFERENCETABLE_H
#define LIBWEXPR_REFERENCETABLE_H

#include "Macros.h"

#include <stddef.h>

LIBWEXPR_EXTERN_C_BEGIN()

// Expression.h
struct WexprExpression;

//
/// \struct WexprReferenceTable
/// \brief A table of expressions given names
///
/// Stores a list of expressions with a given name, allowing you to pull them out.
/// Generally used as a list of references, which allows '*[asdf]' in wexpr to pull out
/// an expression.
//
struct WexprReferenceTable;

typedef struct WexprReferenceTable WexprReferenceTable;

/// \name Construction/Destruction
/// \relates WexprReferenceTable
/// \{

//
/// \brief Creates an empty reference table
/// \return The newly created table
//
LIBWEXPR_PUBLIC WexprReferenceTable* wexpr_ReferenceTable_create();

//
/// \brief Destroy a reference table
/// \param self The referencetable to destroy
//
LIBWEXPR_PUBLIC void wexpr_ReferenceTable_destroy (WexprReferenceTable* self);

/// \}

/// \name Keys/Values
/// \{

//
/// \brief Set the expression for the given key
/// \param self The reference table
/// \param key The key to assign to (cstring)
/// \param expression The expression to assign it. We will take ownership of it.
//
LIBWEXPR_PUBLIC void wexpr_ReferenceTable_setExpressionForKey (
	WexprReferenceTable* self,
	const char* key,
	struct WexprExpression* expression
);

//
/// \brief Set the expression for the given key (with length)
/// \param self The reference table
/// \param key The key to assign to
/// \param keyLength The size of key
/// \param expression The expression to assign it. We will take ownership of it.
//
LIBWEXPR_PUBLIC void wexpr_ReferenceTable_setExpressionForLengthKey (
	WexprReferenceTable* self,
	const char* key, size_t keyLength,
	struct WexprExpression* expression
);

//
/// \brief Return the expression for the given key if found
/// \param self The reference table
/// \param key The key to fetch (cstring)
/// \return The expression found, or null if not.
//
LIBWEXPR_PUBLIC struct WexprExpression* wexpr_ReferenceTable_expressionForKey (
	WexprReferenceTable* self,
	const char* key
);

//
/// \brief Return the expression for the given key if found
/// \param self The reference table
/// \param key The key to fetch
/// \param keyLength The size of key
/// \return The expression found, or null if not.
//
LIBWEXPR_PUBLIC struct WexprExpression* wexpr_ReferenceTable_expressionForLengthKey (
	WexprReferenceTable* self,
	const char* key, size_t keyLength
);

//
/// \brief Remove a key from the reference table
/// \param self The reference table
/// \param key The key to remove
//
LIBWEXPR_PUBLIC void wexpr_ReferenceTable_removeKey (
	WexprReferenceTable* self,
	const char* key
);

//
/// \brief Remove a key from the reference table
/// \param self The reference table
/// \param key The key to remove
/// \param keyLength The length of the key
//
LIBWEXPR_PUBLIC void wexpr_ReferenceTable_removeLengthKey (
	WexprReferenceTable* self,
	const char* key, size_t keyLength
);

//
/// \brief Count the number of keys in the table
/// \param self The reference table
/// \return The number of keys in the table
//
LIBWEXPR_PUBLIC size_t wexpr_ReferenceTable_count (
	WexprReferenceTable* self
);

//
/// \brief Return the index of the requested key
/// \param self The reference table
/// \param key The key to lookup
/// \return The index the key was found, or wepxr_ReferenceTable_count() if not found.
//
LIBWEXPR_PUBLIC size_t wexpr_ReferenceTable_indexOfKey (
	WexprReferenceTable* self,
	const char* key
);

//
/// \brief Get the key at the given index in the table
/// \param self The reference table
/// \param index The index in the table
/// \return The key at the given index, or NULL if invalid index.
//
LIBWEXPR_PUBLIC const char* wexpr_ReferenceTable_keyAtIndex (
	WexprReferenceTable* self,
	size_t index
);

//
/// \brief Get the expression at the given index in the table
/// \param self The reference table
/// \param index The index in the table
/// \return The expression at the given index, or NULL if invalid index.
//
LIBWEXPR_PUBLIC struct WexprExpression* wexpr_ReferenceTable_expressionAtIndex (
	WexprReferenceTable* self,
	size_t index
);

/// \}

LIBWEXPR_EXTERN_C_END()

#endif // LIBWEXPR_REFERENCETABLE_H

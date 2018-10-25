//
/// \file libWexpr/Expression.h
/// \brief A wexpr expression
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

#ifndef LIBWEXPR_EXPRESSION_H
#define LIBWEXPR_EXPRESSION_H

#include "Error.h"
#include "ExpressionType.h"
#include "Macros.h"
#include "ParseFlags.h"
#include "WriteFlags.h"

#include <stddef.h> // size_t

LIBWEXPR_EXTERN_C_BEGIN()

// ReferenceTable.h
struct WexprReferenceTable;

//
/// \struct WexprExpression
/// \brief A wexpr expression
///
/// An expression represents any specific type in Wexpr. It can be:
/// - null/none - means the expression is invalid or nothing.
/// - a value in the form of:
///     alphanumeric characters: asdf
///     a quoted string: "asdf"
///     a number: 2.3
/// - an array: #(a b c)
/// - a map \@(key1 value1 key2 value2)
/// - a binary data as Base64: \<SGlzdG9yeSBtYXkgbm90IHJlcGVhdCwgYnV0IGl0IHJoeW1lcy4=\>
///
/// Comments ;[endofline] or ;(--...--) are not stored and are stripped on import.
/// References [asdf] *[asdf] are also only interpreted on import, and thrown away. (? we might be able to keep it if we're storing the tree anyways).
//
struct WexprExpression;

typedef struct WexprExpression WexprExpression;

//
/// \brief A buffer containing a piece of memory (writeable).
/// Refer to the specific usage about ownership or not
//
typedef struct WexprMutableBuffer
{
	void* data; ///< Pointer to the buffer
	size_t byteSize; ///< Size of the buffer
} WexprMutableBuffer;

//
/// \brief A buffer containing a piece of memory (readonly).
/// Refer to the specific usage about ownership or not
//
typedef struct WexprBuffer
{
	const void* data; ///< Pointer to the buffer
	size_t byteSize; ///< Size of the buffer
} WexprBuffer;

/// \name Construction/Destruction
/// \relates WexprExpression
/// \{

//
/// \brief Creates an expression from a string. You own and must destroy.
/// \param str The string, must be UTF-8 safe/compatible.
/// \param flags Flags about parsing.
/// \param error Will store error information if any occurs.
/// \return The created expression, or nullptr if none/error occurred.
//
LIBWEXPR_PUBLIC WexprExpression* wexpr_Expression_createFromString (
	const char* str, WexprParseFlags flags,
	WexprError* error
);

//
/// \brief Creates an expression from a string. You own and must destroy.
/// \param str The string, must be UTF-8 safe/compatible.
/// \param flags Flags about parsing.
/// \param referenceTable The table to use for pulling references after ones in the file. Will not take ownership.
/// \param error Will store error information if any occurs.
/// \return The created expression, or nullptr if none/error occurred.
//
LIBWEXPR_PUBLIC WexprExpression* wexpr_Expression_createFromStringWithExternalReferenceTable (
	const char* str, WexprParseFlags flags,
	struct WexprReferenceTable* referenceTable,
	WexprError* error
);

//
/// \brief Creates an expression from a string. You own and must destroy.
/// \param str The string, must be UTF-8 safe/compatible.
/// \param length The length of str in bytes
/// \param flags Flags about parsing.
/// \param error Will store error information if any occurs.
/// \return The created expression, or nullptr if none/error occurred.
//
LIBWEXPR_PUBLIC WexprExpression* wexpr_Expression_createFromLengthString (
	const char* str, size_t length, WexprParseFlags flags,
	WexprError* error
);

//
/// \brief Creates an expression from a string. You own and must destroy.
/// \param str The string, must be UTF-8 safe/compatible.
/// \param length The length of str in bytes
/// \param flags Flags about parsing.
/// \param referenceTable The table to use for pulling references after ones in the file. Will not take ownership.
/// \param error Will store error information if any occurs.
/// \return The created expression, or nullptr if none/error occurred.
//
LIBWEXPR_PUBLIC WexprExpression* wexpr_Expression_createFromLengthStringWithExternalReferenceTable (
	const char* str, size_t length, WexprParseFlags flags,
	struct WexprReferenceTable* referenceTable,
	WexprError* error
);

//
/// \brief Creates an expression from a binary chunk. You own and must destroy.
/// \param data The data
/// \param length The length of the data
/// \param error Error information if any occurs.
/// \return The created expression, or nullptr if none/error occurred.
//
LIBWEXPR_PUBLIC WexprExpression* wexpr_Expression_createFromBinaryChunk (
	const void* data, size_t length, WexprError* error
);

//
/// \brief Creates an empty invalid expression. You own and must destroy.
/// \return A newly created invalid expression, or null if it fails.
//
LIBWEXPR_PUBLIC WexprExpression* wexpr_Expression_createInvalid (void);

//
/// \brief Creates an empty null expression. You own and must destroy.
/// \return A newly created null expression, or null if it fails.
//
LIBWEXPR_PUBLIC WexprExpression* wexpr_Expression_createNull (void);

//
/// \brief Create a value expression with the given string being the value.
/// \return a newly created value expression, or null if it fails.
//
LIBWEXPR_PUBLIC WexprExpression* wexpr_Expression_createValue (const char* val);

//
/// \brief Create a value expression from a length string.
/// \return The newly created expression, or null if it fails.
//
LIBWEXPR_PUBLIC WexprExpression* wexpr_Expression_createValueFromLengthString (const char* val, size_t length);

//
/// \brief Create a copy of an expression. You own the copy - deep copy.
/// \param rhs The expression to copy
/// \return The newly created expression, or null if it fails.
//
LIBWEXPR_PUBLIC WexprExpression* wexpr_Expression_createCopy (WexprExpression* rhs);

//
/// \brief Destroy an expression that was created by a create* function.
/// \param self The expression to destroy
//
LIBWEXPR_PUBLIC void wexpr_Expression_destroy (WexprExpression* self);

/// \}

/// \name Information
/// \{

//
/// \brief Return the type of the given expression.
/// \param self The expression to get the type of
/// \return The type of the expression.
//
LIBWEXPR_PUBLIC WexprExpressionType wexpr_Expression_type (WexprExpression* self);

//
/// \brief Change the type of the expression. Invalidates all data currently in the expression.
/// \param self The expression to operate on
/// \param type The new type of the expression
//
LIBWEXPR_PUBLIC void wexpr_Expression_changeType (WexprExpression* self, WexprExpressionType type);

//
/// \brief Create a string which represents the expression. Owned by you, must be destroyed with free.
/// \param self The expression to operate on
/// \param indent The starting indent level, generally 0. Will use tabs to indent.
/// \param flags Flags to use when writing the string
/// \return String with the representation in wexpr text format. You own and must free().
//
LIBWEXPR_PUBLIC char* wexpr_Expression_createStringRepresentation (WexprExpression* self, size_t indent, WexprWriteFlags flags);

//
/// \brief Create binary data which represents the expression. This contains of an expression chunk and all of its child chunks, but NOT the file header. Owned by you, must be destroyed with free.
/// \param self The expression to operate on
/// \return Binary chunk in bwexpr format. Will return a null buffer on errors.
//
LIBWEXPR_PUBLIC WexprMutableBuffer wexpr_Expression_createBinaryRepresentation (WexprExpression* self);

/// \}

/// \name Values
/// \{

//
/// \brief Return the value of the expression. Will return null if not a value.
/// \param self The expression to operate on
/// \return The value of the expression, or null if not found.
//
LIBWEXPR_PUBLIC const char* wexpr_Expression_value (WexprExpression* self);

//
/// \brief Set the value of the expression.
/// \param self The expression to operate on
/// \param str The value to set it to. (null terminated)
//
LIBWEXPR_PUBLIC void wexpr_Expression_valueSet (WexprExpression* self, const char* str);

//
/// \brief Set the value of the expression using a string with a length.
/// \param self The expression to operate on
/// \param str The value to set it to.
/// \param length The length of the str.
//
LIBWEXPR_PUBLIC void wexpr_Expression_valueSetLengthString (WexprExpression* self, const char* str, size_t length);

/// \}

/// \name Binary Data
/// \{

//
/// \brief Return the data of the expression. Will return null if not a binary data.
/// \param self The expression to operate on
/// \return A pointer to the binary data. See  wexpr_Expression_binaryData_size() for how many bytes you can read from it
//
LIBWEXPR_PUBLIC const void* wexpr_Expression_binaryData_data (WexprExpression* self);

//
/// \brief Return the buffer size of the expression. Will return 0 if not binary data.
/// \param self The expression to operate on
/// \return The size of the binary data.
//
LIBWEXPR_PUBLIC size_t wexpr_Expression_binaryData_size (WexprExpression* self);

//
/// \brief Set the binary data to use. Will copy it in.
/// \param self The expression to operate on
/// \param buffer The buffer to read from
/// \param byteSize The size to read in the buffer
//
LIBWEXPR_PUBLIC void wexpr_Expression_binaryData_setValue (WexprExpression* self, const void* buffer, size_t byteSize);

/// \}

/// \name Array
/// \{

//
/// \brief Return the number of expressions in the array. Returns 0 if not an array.
/// \param self The expression to operate on
/// \return The number of items in the array, or 0 if not an array.
//
LIBWEXPR_PUBLIC size_t wexpr_Expression_arrayCount (WexprExpression* self);

//
/// \brief Return the expression at the given index. [0 .. arrayCount-1]
/// \param self The expression to operate on
/// \param index The index in the array to fetch
/// \return The expression or NULL if invalid.
//
LIBWEXPR_PUBLIC WexprExpression* wexpr_Expression_arrayAt (WexprExpression* self, size_t index);

//
/// \brief Add an element to the end of the array.
/// \param self The expression to operate on
/// \param element The element to add. You MUST own, and we'll take ownership from you. Use wexpr_Expression_createCopy() if you need to add an un-owned pointer.
//
LIBWEXPR_PUBLIC void wexpr_Expression_arrayAddElementToEnd (WexprExpression* self, WexprExpression* element);

/// \}

/// \name Map
/// \{

//
/// \brief Return the number of key-value pairs in the map. Returns 0 if not a map.
/// \param self The expression to operate on
/// \return The number of key-value pairs in the map, or 0 if not a map.
//
LIBWEXPR_PUBLIC size_t wexpr_Expression_mapCount (WexprExpression* self);

//
/// \brief Return the key at a given index within the map.
/// \param self The expression to operate on
/// \param index The index in the map to fetch the key of
/// \return The key at the given index, or null if none.
//
LIBWEXPR_PUBLIC const char* wexpr_Expression_mapKeyAt (WexprExpression* self, size_t index);

//
/// \brief Return the value at a given index within the map.
/// \param self The expression to operate on
/// \param index The index in the map to fetch the value of
/// \return The value, or NULL if an invalid index is given.
//
LIBWEXPR_PUBLIC WexprExpression* wexpr_Expression_mapValueAt (WexprExpression* self, size_t index);

//
/// \brief Return the value for a given key within the map, or NULL if not found.
/// \param self The expression to operate on
/// \param key The key to fetch the value of (null terminated)
/// \return The value, or NULL if not found.
//
LIBWEXPR_PUBLIC WexprExpression* wexpr_Expression_mapValueForKey (WexprExpression* self, const char* key);

//
/// \brief Return the value for a given key (w/length) within the map, or NULL if not found.
/// \param self The expression to operate on
/// \param key The key string pointer.
/// \param length The length of the key in bytes
/// \return The value for the given key, or null if not found.
//
LIBWEXPR_PUBLIC WexprExpression* wexpr_Expression_mapValueForLengthKey (WexprExpression* self, const char* key, size_t length);

//
/// \brief Set the value for a given key in the map
/// \param self The expression to operate on
/// \param key The key to assign the value to.
/// \param value The value to use. You MUST own, and we'll take ownership from you.
//
LIBWEXPR_PUBLIC void wexpr_Expression_mapSetValueForKey (WexprExpression* self, const char* key, WexprExpression* value);

//
/// \brief Set the value for a given key (lengthstr) in the map
/// \param self The expression to operate on
/// \param key The key to assign the value to.
/// \param length The length of the key
/// \param value The value to use. You MUST own, and we'll take ownership from you.
//
LIBWEXPR_PUBLIC void wexpr_Expression_mapSetValueForKeyLengthString (WexprExpression* self, const char* key, size_t length, WexprExpression* value);

/// \}

LIBWEXPR_EXTERN_C_END()

#endif // LIBWEXPR_EXPRESSION_H

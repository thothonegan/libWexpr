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

//
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
typedef struct WexprExpression WexprExpression;

/// \name Construction/Destruction
/// \{

//
/// \brief Creates an expression from a string. You own and must destroy.
/// \param str The string, must be UTF-8 safe/compatible.
/// \param flags Flags about parsing.
/// \param error Will store error information if any occurs.
/// \return The created expression, or nullptr if none/error occurred.
//
WexprExpression* wexpr_Expression_createFromString (
	const char* str, WexprParseFlags flags,
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
WexprExpression* wexpr_Expression_createFromLengthString (
	const char* str, size_t length, WexprParseFlags flags,
	WexprError* error
);

//
/// \brief Creates an empty invalid expression. You own and must destroy.
/// \return A newly created invalid expression, or null if it fails.
//
WexprExpression* wexpr_Expression_createInvalid (void);

//
/// \brief Creates an empty null expression. You own and must destroy.
/// \return A newly created null expression, or null if it fails.
//
WexprExpression* wexpr_Expression_createNull (void);

//
/// \brief Create a value expression with the given string being the value.
/// \return a newly created value expression, or null if it fails.
//
WexprExpression* wexpr_Expression_createValue (const char* val);

//
/// \brief Create a value expression from a length string.
//
WexprExpression* wexpr_Expression_createValueFromLengthString (const char* val, size_t length);

//
/// \brief Create a copy of an expression. You own the copy - deep copy.
//
WexprExpression* wexpr_Expression_createCopy (WexprExpression* rhs);

//
/// \brief Destroy an expression that was created by a create* function.
/// \param self The expression to destroy
//
void wexpr_Expression_destroy (WexprExpression* self);

/// \}

/// \name Information
/// \{

//
/// \brief Return the type of the given expression.
/// \param self The expression to get the type of
/// \return The type of the expression.
//
WexprExpressionType wexpr_Expression_type (WexprExpression* self);

//
/// \brief Change the type of the expression. Invalidates all data currently in the expression.
//
void wexpr_Expression_changeType (WexprExpression* self, WexprExpressionType type);

//
/// \brief Create a string which represents the expression. Owned by you, must be destroyed with free.
/// \param indent The starting indent level, generally 0. Will use tabs to indent.
//
char* wexpr_Expression_createStringRepresentation (WexprExpression* self, size_t indent, WexprWriteFlags flags);

/// \}

/// \name Values
/// \{

//
/// \brief Return the value of the expression. Will return null if not a value.
//
const char* wexpr_Expression_value (WexprExpression* self);

//
/// \brief Set the value of the expression.
//
void wexpr_Expression_valueSet (WexprExpression* self, const char* str);

//
/// \brief Set the value of the expression using a string with a length.
//
void wexpr_Expression_valueSetLengthString (WexprExpression* self, const char* str, size_t length);

/// \}

/// \name Binary Data
/// \{

//
/// \brief Return the data of the expression. Will return null if not a binary data.
//
const void* wexpr_Expression_binaryData_data (WexprExpression* self);

//
/// \brief Return the buffer size of the expression. Will return 0 if not binary data.
//
size_t wexpr_Expression_binaryData_size (WexprExpression* self);

//
/// \brief Set the binary data to use. Will copy it in.
//
void wexpr_Expression_binaryData_setValue (WexprExpression* self, void* buffer, size_t byteSize);

/// \}

/// \name Array
/// \{

//
/// \brief Return the number of expressions in the array. Returns 0 if not an array.
//
size_t wexpr_Expression_arrayCount (WexprExpression* self);

//
/// \brief Return the expression at the given index. [0 .. arrayCount-1]
/// \return The expression or NULL if invalid.
//
WexprExpression* wexpr_Expression_arrayAt (WexprExpression* self, size_t index);

//
/// \brief Add an element to the end of the array.
/// \param element The element to add. You MUST own, and we'll take ownership from you. Use wexpr_Expression_createCopy() if you need to add an un-owned pointer.
//
void wexpr_Expression_arrayAddElementToEnd (WexprExpression* self, WexprExpression* element);

/// \}

/// \name Map
/// \{

//
/// \brief Return the number of key-value pairs in the map. Returns 0 if not a map.
//
size_t wexpr_Expression_mapCount (WexprExpression* self);

//
/// \brief Return the key at a given index within the map.
//
const char* wexpr_Expression_mapKeyAt (WexprExpression* self, size_t index);

//
/// \brief Return the value at a given index within the map.
//
WexprExpression* wexpr_Expression_mapValueAt (WexprExpression* self, size_t index);

//
/// \brief Return the value for a given key within the map, or NULL if not found.
//
WexprExpression* wexpr_Expression_mapValueForKey (WexprExpression* self, const char* key);

//
/// \brief Return the value for a given key (w/length) within the map, or NULL if not found.
//
WexprExpression* wexpr_Expression_mapValueForLengthKey (WexprExpression* self, const char* key, size_t length);

//
/// \brief Set the value for a given key in the map
/// \param key The key to assign the value to.
/// \param value The value to use. You MUST own, and we'll take ownership from you.
//
void wexpr_Expression_mapSetValueForKey (WexprExpression* self, const char* key, WexprExpression* value);

//
/// \brief Set the value for a given key (lengthstr) in the map
/// \param key The key to assign the value to.
/// \param value The value to use. You MUST own, and we'll take ownership from you.
//
void wexpr_Expression_mapSetValueForKeyLengthString (WexprExpression* self, const char* key, size_t length, WexprExpression* value);

/// \}

LIBWEXPR_EXTERN_C_END()

#endif // LIBWEXPR_EXPRESSION_H

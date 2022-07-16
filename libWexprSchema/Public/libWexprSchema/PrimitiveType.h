//
/// \file libWexprSchema/PrimitiveType.h
/// \brief Primitive types for schemas
//
// #LICENSE_BEGIN:MIT#
// #LICENSE_END#
//

#ifndef LIBWEXPRSCHEMA_PRIMITIVETYPE_H
#define LIBWEXPRSCHEMA_PRIMITIVETYPE_H

#include <libWexpr/Macros.h>
#include <libWexpr/ExpressionType.h>

#include <stdbool.h>

#include "Twine.h"

LIBWEXPR_EXTERN_C_BEGIN()

//
/// \brief The list of possible primitive types. Can be flagged.
//
enum WexprSchemaPrimitiveType
{
	//
	/// \brief An unknown primitive type
	//
	WexprSchemaPrimitiveTypeUnknown = 0x00,

	//
	/// \brief A null value
	//
	WexprSchemaPrimitiveTypeNull = 0x01,

	//
	/// \brief A single value
	//
	WexprSchemaPrimitiveTypeValue = 0x02,

	//
	/// \brief An array - has an ordered list of elements
	//
	WexprSchemaPrimitiveTypeArray = 0x04,

	//
	/// \brief A map/object - has keyed properties in an arbitrary order
	//
	WexprSchemaPrimitiveTypeMap = 0x08,

	//
	/// \brief Arbitrary binary data
	//
	WexprSchemaPrimitiveTypeBinaryData = 0x10,
};

typedef enum WexprSchemaPrimitiveType WexprSchemaPrimitiveType;

//
/// \brief Return the equivilant primitive type for the given string
//
WexprSchemaPrimitiveType wexprSchema_PrimitiveType_fromString (const char* str);

//
/// \brief Does the given primitive types match the given wexpr expression type?
//
bool wexprSchema_PrimitiveType_matchesExpressionType (WexprSchemaPrimitiveType self, WexprExpressionType type);

//
/// \brief Return a string which represents the primitive type.
/// Will be the primitive type, or 'type1|type2'
//
WexprSchemaTwine wexprSchema_PrimitiveType_toTwine(WexprSchemaPrimitiveType self);

LIBWEXPR_EXTERN_C_END()

#endif // LIBWEXPRSCHEMA_PRIMITIVETYPE_H

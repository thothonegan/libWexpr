//
/// \file libWexprSchema/Type.h
/// \brief A single type we validate with the schema
//
// #LICENSE_BEGIN:MIT#
// #LICENSE_END#
//

#ifndef LIBWEXPRSCHEMA_TYPE_H
#define LIBWEXPRSCHEMA_TYPE_H

#include <libWexpr/Macros.h>
#include <libWexpr/Expression.h>

#include "Error.h"
#include "PrimitiveType.h"
#include "Twine.h"

#include <stdbool.h>

LIBWEXPR_EXTERN_C_BEGIN()

struct WexprSchemaSchema;

//
/// \brief A type within a schema to validate (SchemaTypeDefinition)
//
typedef struct WexprSchemaType WexprSchemaType;

/// \name Construction/Destruction
/// \relates WexprSchemaType
/// \{

//
/// \brief Create the type from a wexpr expression. Will set it up, but not fully resolve its types yet.
/// Call wexprSchema_Type_resolveWithSchema() after your schemais setup.
//
WexprSchemaType* wexprSchema_Type_createFromExpression (const char* name, WexprExpression* expression);

//
/// \brief Destroy the type
//
void wexprSchema_Type_destroy(WexprSchemaType* self);

/// \}

/// \name Loading
/// \{

//
/// \brief Resolve any dependent types in the given schema. Will link the type instead of the name if possible.
/// \return Success or failure if its unable to resolve.
//
bool wexprSchema_Type_resolveWithSchema(WexprSchemaType* self, struct WexprSchemaSchema* schema, WexprSchemaError** error);

/// \}

/// \}

/// \name Properties
/// \relates WexprSchemaType
/// \{

//
/// \brief Returns the name of the type
//
const char* wexprSchema_Type_name(WexprSchemaType* self);

//
/// \brief The description of the type
//
const char* wexprSchema_Type_description(WexprSchemaType* self);

//
/// \brief The number of possible base types this type has. The value is expected to meet at least one of them.
//
unsigned int wexprSchema_Type_possibleTypesCount(WexprSchemaType* self);

//
/// \brief The base type at the given index. Must be between 0 and count-1.
//
WexprSchemaType* wexprSchema_Type_typeAt(WexprSchemaType* self, unsigned int index);

//
/// \brief The primitive types of this type - will define what types of rules can be applied to it.
/// The primitive types will either be set on this type, or derived off our base type.
//
WexprSchemaPrimitiveType wexprSchema_Type_primitiveTypes(WexprSchemaType* self);

// ---- Value specific properties

// ---- Map specific properties

// ---- Array specific properties

/// \}

/// \name Validation
/// \relates WexprSchemaType
/// \{

//
/// \brief Valiate the given object against this Type
/// Expression can be null if theirs no expression on the other side
/// \return Success or failure. If failure, the reason should be within error (if provided).
//
bool wexprSchema_Type_validateObject (
	WexprSchemaType* self,
	WexprSchemaTwine* objectPath,
	WexprExpression* expression,
	WexprSchemaError** error
);

/// \}

LIBWEXPR_EXTERN_C_END()

#endif // LIBWEXPRSCHEMA_TYPE_H

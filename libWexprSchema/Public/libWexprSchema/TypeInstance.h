//
/// \file libWexprSchema/TypeInstance.h
/// \brief An instance of a type (usually for a given property)
//
// #LICENSE_BEGIN:MIT#
// #LICENSE_END#
//

#ifndef LIBWEXPRSCHEMA_TYPEINSTANCE_H
#define LIBWEXPRSCHEMA_TYPEINSTANCE_H

#include <libWexpr/Macros.h>

LIBWEXPR_EXTERN_C_BEGIN()

//
/// \brief A type instance within a schema to validate, usually a property. (SchemaTypeInstanceDefinition)
//
typedef struct WexprSchemaTypeInstance WexprSchemaTypeInstance;

/// \name Construction/Destruction
/// \relates WexprSchemaTypeInstance
/// \{

//
/// \brief Create the instance from a wexpr expression. Will set it up, but not fully resolve its types yet.
/// Call wexprSchema_TypeInstance_resolveWithSchema() as part of the schema resolveWithSchema.
//
WexprSchemaTypeInstance* wexprSchema_TypeInstance_createFromExpression (WexprExpression* expression);

//
/// \brief Destroy the type
//
void wexprSchema_TypeInstance_destroy (WexprSchemaTypeInstance* self);

/// \}

/// \name Resolution
/// \relates WexprSchemaTypeInstance
/// \{

//
/// \brief Resolve its type against the given schema if possible
//
bool wexprSchema_TypeInstance_resolveWithSchema(WexprSchemaTypeInstance* self, struct WexprSchemaSchema* schema, WexprSchemaError** error);

/// \}

/// \name Validation
/// \relates WexprSchemaTypeInstance
/// \{

//
/// \brief Valiate the given object against this TypeInstance. Will validate against the type we're registered.
/// Expression can be null if theirs no expression on the other side
/// \return Success or failure. If failure, the reason should be within error (if provided).
//
bool wexprSchema_TypeInstance_validateObject (
	WexprSchemaTypeInstance* self,
	WexprSchemaTwine* objectPath,
	WexprExpression* expression,
	WexprSchemaError** error
);

/// \}

LIBWEXPR_EXTERN_C_END()

#endif // LIBWEXPRSCHEMA_TYPEINSTANCE_H

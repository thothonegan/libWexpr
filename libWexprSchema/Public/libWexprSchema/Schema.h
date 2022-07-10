//
/// \file libWexprSchema/Schema.h
/// \brief A wexpr schema
//
// #LICENSE_BEGIN:MIT#
// #LICENSE_END#
//

#ifndef LIBWEXPRSCHEMA_SCHEMA_H
#define LIBWEXPRSCHEMA_SCHEMA_H

#include <libWexpr/Expression.h>

#include "Error.h"
#include "Macros.h"

#include <stdbool.h>
#include <stdint.h>

LIBWEXPR_EXTERN_C_BEGIN()

//
/// \brief A wexpr schema - can be used to validate wexpr epxressions.
/// May contain other child schemas.
//
typedef struct WexprSchemaSchema WexprSchemaSchema;

//
/// \brief Callbacks to customize the behavior of schema validation
/// Define callbacks as null that you want to use defaults.
//
typedef struct WexprSchemaSchema_Callbacks
{
	//
	/// \brief Allocate memory of the given size/alignment.
	/// If alloc is null, we'll use malloc().
	//
	void* (*alloc) (void* allocatorUserData, size_t size);

	//
	/// \brief Ree the memory previously allocated with the alloc handler.
	/// If dealloc is null, we'll use free().
	//
	void (*dealloc) (void* allocatorUserData, void* ptr);

	//
	/// \brief Callback to get the path or url for a given schemaID.
	/// If it returns null, or pathForSchemaID is null - it will use the schemaID itself as the path/url.
	//
	const char* (*pathForSchemaID) (void* pathForSchemaIDUserData, const char* schemaID);

	//
	/// \brief The user data pointer to provide to the allocator functions.
	//
	void* allocatorUserData;

	//
	/// \brief The userdata pointer for pathForSchemaID
	//
	void* pathForSchemaIDUserData;

} WexprSchemaSchema_Callbacks;

/// \name Construction/Destruction
/// \see WexprSchemaSchema
/// \{

//
/// \brief Create a new schema by loading it from the requested id, using the given helpers.
/// \param schemaID The Id of the schema to load
/// \param callbacks The callbacks to use.
/// \return nullptr if an error occurs and cannot create the schema, or the schema was invalid.
//
WexprSchemaSchema* wexprSchema_Schema_createFromSchemaID(const char* schemaID, WexprSchemaSchema_Callbacks* callbacks, WexprSchemaError** error);

//
/// \brief Destroy the schema
//
void wexprSchema_Schema_destroy(WexprSchemaSchema* self);

/// \}

/// \name Validation
/// \{


//
/// \brief Validate the given expression with the loaded schema. Will return true/false, and will store
/// the failures if we are given a location to put the resulting error.
//
bool wexprSchema_Schema_validateExpression(WexprExpression* expression, WexprSchemaError** error);

/// \}

LIBWEXPR_EXTERN_C_END()

#endif // LIBWEXPRSCHEMA_SCHEMA_H

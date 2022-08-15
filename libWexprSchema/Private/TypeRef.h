//
/// \file libWexprSchema/TypeRef.h
/// \brief A reference to another type
//
// #LICENSE_BEGIN:MIT#
// #LICENSE_END#
//

#ifndef LIBWEXPRSCHEMA_TYPEREF_H
#define LIBWEXPRSCHEMA_TYPEREF_H

#include <libWexpr/Macros.h>
#include <libWexprSchema/Type.h>

#include "../../libWexpr/Private/ThirdParty/sglib/sglib.h"

LIBWEXPR_EXTERN_C_BEGIN()

//
/// \brief A reference to a type
//
typedef struct WexprSchemaPrivateTypeRef
{
	char* typeName; // we own if exists, will only exist temporarily till we fill out type.
	WexprSchemaType* type;  // we dont own - will be part of the schema that set us up.
	struct WexprSchemaPrivateTypeRef* next;
} WexprSchemaPrivateTypeRef;

#define WEXPRSCHEMAPRIVATETYPEREF_COMPARATOR(e1, e2) ( (char*)(e1->type) - (char*)(e2->type))

SGLIB_DEFINE_LIST_PROTOTYPES (WexprSchemaPrivateTypeRef, WEXPRSCHEMAPRIVATETYPEREF_COMPARATOR, next)


/// \name Construction/Destruction
/// \relates WexprSchemaPrivateTypeRef
/// \{

//
/// \brief Create the ref with the given name. Not resolved yet.
//
WexprSchemaPrivateTypeRef* wexprSchema_PrivateTypeRef_createWithName(const char* name);

//
/// \brief Destroy the ref
//
void wexprSchema_PrivateTypeRef_destroy(WexprSchemaPrivateTypeRef* self);

/// \}

/// \name Resolution
/// \relates WexprSchemaPrivateTypeRef
/// \{

//
/// \brief Resolve with the given type - will remove the name.
/// \note We dont take ownership of the type
//
void wexprSchema_PrivateTypeRef_resolveWith(WexprSchemaPrivateTypeRef* self, WexprSchemaType* type);

/// \}

/// \name Properties
/// \relates WexprSchemaPrivateTypeRef
/// \{

//
/// \brief Returns the name of the type
//
const char* wexprSchema_PrivateTypeRef_name (WexprSchemaPrivateTypeRef* self);

//
/// \brief Is it resolved?
//
bool wexprSchema_PrivateTypeRef_isResolved (WexprSchemaPrivateTypeRef* self);

//
/// \brief Return the type if its resolved, returns null if not.
//
WexprSchemaType* wexprSchema_PrivateTypeRef_type (WexprSchemaPrivateTypeRef* self);

/// \}

LIBWEXPR_EXTERN_C_END()

#endif // LIBWEXPRSCHEMA_TYPEREF_H


//
/// \file libWexprSchema/libWexprSchema.h
/// \brief Includes all WexprSchema files
//
// #LICENSE_BEGIN:MIT#
// #LICENSE_END#
//

#ifndef LIBWEXPRSCHEMA_LIBWEXPRSCHEMA_H
#define LIBWEXPRSCHEMA_LIBWEXPRSCHEMA_H

#include "Macros.h"

#define LIBWEXPRSCHEMA_VERSION_MAJOR 1
#define LIBWEXPRSCHEMA_VERSION_MINOR 0
#define LIBWEXPRSCHEMA_VERSION_PATCH 0

LIBWEXPR_EXTERN_C_BEGIN()

LIBWEXPRSCHEMA_PUBLIC int wexprSchema_Version_major ();
LIBWEXPRSCHEMA_PUBLIC int wexprSchema_Version_minor ();
LIBWEXPRSCHEMA_PUBLIC int wexprSchema_Version_patch ();

LIBWEXPR_EXTERN_C_END()

#endif // LIBWEXPRSCHEMA_LIBWEXPRSCHEMA_H

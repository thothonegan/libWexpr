//
/// \file libWexprSchema/Global.h
/// \brief Global functions for schema 
//
// #LICENSE_BEGIN:MIT#
// #LICENSE_END#
//

#ifndef LIBWEXPRSCHEMA_GLOBAL_H
#define LIBWEXPRSCHEMA_GLOBAL_H

#include <libWexpr/Expression.h>

LIBWEXPR_EXTERN_C_BEGIN()

//
/// \brief Init the library - call before you use the library
//
void wexprSchema_Global_init();

//
/// \brief Free the library - call once done with the library
//
void wexprSchema_Global_free();

LIBWEXPR_EXTERN_C_END()

#endif // LIBWEXPRSCHEMA_GLOBAL_H


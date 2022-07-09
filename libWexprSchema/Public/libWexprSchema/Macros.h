//
/// \file libWexprSchema/Macros.h
/// \brief Wexpr schema macros
//
// #LICENSE_BEGIN:MIT#
// #LICENSE_END#
//

#ifndef LIBWEXPRSCHEMA_MACROS_H
#define LIBWEXPRSCHEMA_MACROS_H

#include "libWexpr/Macros.h"

// LIBWEXPRSCHEMA_PUBLIC will export/import as needed

#if defined(CATALYST_libWexprSchema_IS_SHARED_LIBRARY)
	#if defined(CATALYST_libWexprSchema_IS_BUILDING)
		#define LIBWEXPRSCHEMA_PUBLIC LIBWEXPR_EXPORT
	#else
		#define LIBWEXPRSCHEMA_PUBLIC LIBWEXPR_IMPORT
	#endif
#else // not shared
	#define LIBWEXPRSCHEMA_PUBLIC
#endif

#endif // LIBWEXPRSCHEMA_MACROS_H

//
/// \file libWexprSchema/Global.c
/// \brief Global functions for schema 
//
// #LICENSE_BEGIN:MIT#
// #LICENSE_END#
//

#include <libWexprSchema/Global.h>

#include "ExternalOnigmo.h"

void wexprSchema_Global_init()
{
	onig_init();
}

void wexprSchema_Global_free()
{
	onig_end();
}

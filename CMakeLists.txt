#
# libWexpr/CMakeLists.txt
# Reference W-Expressions C library 
#
# Can build with normal cmake or use Catalyst.
#

cmake_minimum_required (VERSION 3.3)

project (libWexpr
	#VERSION master
	DESCRIPTION "Library for handling W Expressions"
)

set (CMAKE_C_STANDARD 99)
set (CMAKE_C_STANDARD_REQUIRED YES)

# NOTE: If change, must change in libWexpr.h too. Someday should use configure or something.
set (LIBWEXPR_VERSION_MAJOR 1)
set (LIBWEXPR_VERSION_MINOR 0)
set (LIBWEXPR_VERSION_PATCH 0)

if (CATALYST_INSTALL_PREFIX) # use catalyst on top of cmake
	list (INSERT CMAKE_MODULE_PATH 0
		${CATALYST_INSTALL_PREFIX}/Catalyst/Modules/
	)
	
	include (Catalyst)
	
	catalyst_module_declare (libWexpr)

endif ()

# Documentation
add_subdirectory (Documentation/Doxygen)

add_subdirectory (libWexpr)
add_subdirectory (WexprTool)

#
# libWexpr/CMakeLists.txt
# Reference W-Expressions C library 
#
# Can build with normal cmake or use Catalyst.
#

cmake_minimum_required (VERSION 3.3)

set (CMAKE_C_STANDARD 99)
set (CMAKE_C_STANDARD_REQUIRED YES)

if (CATALYST_INSTALL_PREFIX) # use catalyst on top of cmake
	list (INSERT CMAKE_MODULE_PATH 0
		${CATALYST_INSTALL_PREFIX}/Catalyst/Modules/
	)
	
	include (Catalyst)
	
	catalyst_module_declare (libWexpr)

endif ()

add_subdirectory (libWexpr)
add_subdirectory (WexprTool)

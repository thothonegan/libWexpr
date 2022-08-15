#
# libWexpr/WexprTool/CMakeLists.txt
# Tool for performing things on wexpr expressions
#

find_package (Curl)

project (WexprTool)

	set (WexprTool_PRIVATE_HEADERS
		${WexprTool_SOURCE_DIR}/Private/CommandLineParser.hpp
	)

	set (WexprTool_SOURCES
		${WexprTool_SOURCE_DIR}/Private/Application.cpp
		${WexprTool_SOURCE_DIR}/Private/CommandLineParser.cpp
	)

	if (CATALYST_INSTALL_PREFIX)
		catalyst_begin_console_executable (WexprTool ${WexprTool_PRIVATE_HEADERS} ${WexprTool_SOURCES})
			catalyst_module_use (WexprTool libWexpr libWexprSchema)
			catalyst_target_require_cxx_standard (WexprTool 11)

			catalyst_append_target_property (WexprTool COMPILE_DEFINITIONS "ENABLE_CURL")
			catalyst_target_link_libraries (WexprTool PRIVATE ${curl_TARGETS})
		catalyst_end_console_executable (WexprTool)

		catalyst_install_target (WexprTool Tools/)
	else ()
		add_executable (WexprTool ${WexprTool_PRIVATE_HEADERS} ${WexprTool_SOURCES})

		set_property (TARGET WexprTool PROPERTY CXX_STANDARD 11)
		set_property (TARGET WexprTool PROPERTY CXX_STANDARD_REQUIRED YES)
		set_property (TARGET WexprTool APPEND PROPERTY COMPILE_DEFINITIONS "ENABLE_CURL")

		set_property (TARGET WexprTool APPEND PROPERTY INCLUDE_DIRECTORIES
			"${WexprTool_SOURCE_DIR}/../libWexpr/Public"
		)

		target_link_libraries (WexprTool libWexpr libWexprSchema)

		if (Curl_FOUND)
			target_link_libraries (WexprTool CURL::libcurl)
		endif ()

		install (TARGETS WexprTool
			DESTINATION bin/
		)
	endif ()

#
# libWexprSchema/Tests/CMakeLists.txt
# Self tests for libWexprSchema
#

if (CATALYST_INSTALL_PREFIX)
	catalyst_project (libWexprSchemaTests DEPENDS libWexprSchema)
else ()
	project (libWexprSchemaTests)
	set (CatalystProject_libWexprSchemaTests_ENABLE ON)
endif ()

if (CatalystProject_libWexprSchemaTests_ENABLE)

	set (libWexprSchemaTests_HEADERS
		#	${CMAKE_CURRENT_SOURCE_DIR}/Expression.h
		#	${CMAKE_CURRENT_SOURCE_DIR}/ExpressionErrors.h
		#	${CMAKE_CURRENT_SOURCE_DIR}/ExpressionType.h
		#	${CMAKE_CURRENT_SOURCE_DIR}/ReferenceTable.h
		#	${CMAKE_CURRENT_SOURCE_DIR}/UnitTest.h
		#	${CMAKE_CURRENT_SOURCE_DIR}/UVLQ64.h
	)

	set (libWexprSchemaTests_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/Main.c
	)

	# MSVC gets annoyed with our POSIX functions
	set (libWexprSchemaTests_DEFINES
		_CRT_NONSTDC_NO_DEPRECATE=1
		_CRT_SECURE_NO_WARNINGS=1
	)

	if (CATALYST_INSTALL_PREFIX)

		catalyst_begin_test (libWexprSchemaTests ${libWexprSchemaTests_HEADERS} ${libWexprSchemaTests_SOURCES})
			catalyst_module_use (libWexprSchemaTests libWexprSchema)

			catalyst_append_target_property (libWexprSchemaTests COMPILE_DEFINITIONS
				${libWexprSchemaTests_DEFINES}
			)
		catalyst_end_test (libWexprSchemaTests)

	else ()

		add_executable (libWexprSchemaTests ${libWexprSchemaTests_HEADERS} ${libWexprSchemaTests_SOURCES})
		target_link_libraries (libWexprSchemaTests libWexprSchema)

		set_property (TARGET libWexprSchemaTests APPEND PROPERTY INCLUDE_DIRECTORIES
			"${CMAKE_CURRENT_SOURCE_DIR}/../Public"
		)

	set_property (TARGET libWexprSchemaTests APPEND PROPERTY COMPILE_DEFINITIONS ${libWexprSchemaTests_DEFINES})

		if (NOT TESTS_DISABLE)
			add_custom_command (TARGET libWexprSchemaTests POST_BUILD COMMAND ${TEST_RUNNER} $<TARGET_FILE:libWexprSchemaTests>)
		endif ()
	endif ()

endif ()

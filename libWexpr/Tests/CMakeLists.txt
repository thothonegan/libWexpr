#
# libWexpr/Tests/CMakeLists.txt
# Self tests for libWexpr
#

if (CATALYST_INSTALL_PREFIX)
	catalyst_project (libWexprTests DEPENDS libWexpr)
else ()
	project (libWexprTests)
	set (CatalystProject_libWexprTests_ENABLE ON)
endif ()

if (CatalystProject_libWexprTests_ENABLE)

	set (libWexprTests_HEADERS
		${libWexprTests_SOURCE_DIR}/Expression.h
		${libWexprTests_SOURCE_DIR}/ExpressionErrors.h
		${libWexprTests_SOURCE_DIR}/ExpressionType.h
		${libWexprTests_SOURCE_DIR}/UnitTest.h
	)

	set (libWexprTests_SOURCES
		${libWexprTests_SOURCE_DIR}/Main.c
	)

	# MSVC gets annoyed with our POSIX functions
	set (libWexprTests_DEFINES
		_CRT_NONSTDC_NO_DEPRECATE=1
		_CRT_SECURE_NO_WARNINGS=1
	)

	if (CATALYST_INSTALL_PREFIX)

		catalyst_begin_test (libWexprTests ${libWexprTests_HEADERS} ${libWexprTests_SOURCES})
			catalyst_module_use (libWexprTests libWexpr)

			catalyst_append_target_property (libWexprTests COMPILE_DEFINITIONS
				${libWexprTests_DEFINES}
			)
		catalyst_end_test (libWexprTests)

	else ()

		add_executable (libWexprTests ${libWexprTests_HEADERS} ${libWexprTests_SOURCES})
		target_link_libraries (libWexprTests libWexpr)

		set_property (TARGET libWexprTests APPEND PROPERTY INCLUDE_DIRECTORIES
			"${libWexprTests_SOURCE_DIR}/../Public"
		)

	set_property (TARGET libWexprTests APPEND PROPERTY COMPILE_DEFINITIONS ${libWexprTests_DEFINES})

		if (NOT TESTS_DISABLE)
			add_custom_command (TARGET libWexprTests POST_BUILD COMMAND ${TEST_RUNNER} $<TARGET_FILE:libWexprTests>)
		endif ()
	endif ()

endif ()

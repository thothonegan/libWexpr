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
		${libWexprTests_SOURCE_DIR}/ExpressionType.h
		${libWexprTests_SOURCE_DIR}/UnitTest.h
	)
	
	set (libWexprTests_SOURCES
		${libWexprTests_SOURCE_DIR}/Main.c
	)
	
	if (CATALYST_INSTALL_PREFIX)
	
		catalyst_begin_test (libWexprTests ${libWexprTests_HEADERS} ${libWexprTests_SOURCES})
			catalyst_module_use (libWexprTests libWexpr)
		catalyst_end_test (libWexprTests)
	
	else ()
	
		add_executable (libWexprTests ${libWexprTests_HEADERS} ${libWexprTests_SOURCES})
		target_link_libraries (libWexprTests libWexpr)
		
		set_property (TARGET libWexprTests APPEND PROPERTY INCLUDE_DIRECTORIES
			"${libWexprTests_SOURCE_DIR}/../Public"
		)
		
		if (NOT TESTS_DISABLE)
			add_custom_command (TARGET libWexprTests POST_BUILD COMMAND ${TEST_RUNNER} $<TARGET_FILE:libWexprTests>)
		endif ()
	endif ()
	
endif ()

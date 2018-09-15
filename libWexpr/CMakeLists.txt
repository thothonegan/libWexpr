#
# libWexpr/libWexpr/CMakeLists.txt
# Reference W-Expressions C library
#

project (libWexpr)

	set (libWexpr_HEADERS
		${libWexpr_SOURCE_DIR}/Public/libWexpr/libWexpr.h

		${libWexpr_SOURCE_DIR}/Public/libWexpr/Endian.h
		${libWexpr_SOURCE_DIR}/Public/libWexpr/Error.h
		${libWexpr_SOURCE_DIR}/Public/libWexpr/Expression.h
		${libWexpr_SOURCE_DIR}/Public/libWexpr/ExpressionType.h
		${libWexpr_SOURCE_DIR}/Public/libWexpr/Macros.h
		${libWexpr_SOURCE_DIR}/Public/libWexpr/ParseFlags.h
		${libWexpr_SOURCE_DIR}/Public/libWexpr/UVLQ64.h
		${libWexpr_SOURCE_DIR}/Public/libWexpr/WriteFlags.h
	)

	set (libWexpr_PRIVATE_HEADERS
		${libWexpr_SOURCE_DIR}/Private/ThirdParty/sglib/sglib.h
		${libWexpr_SOURCE_DIR}/Private/ThirdParty/c_hashmap/hashmap.h
		
		${libWexpr_SOURCE_DIR}/Private/Base64.h
	)

	set (libWexpr_SOURCES
		${libWexpr_SOURCE_DIR}/Private/Base64.c
		${libWexpr_SOURCE_DIR}/Private/Expression.c
		${libWexpr_SOURCE_DIR}/Private/ExpressionType.c
		${libWexpr_SOURCE_DIR}/Private/libWexpr.c

		${libWexpr_SOURCE_DIR}/Private/ThirdParty/c_hashmap/hashmap.c
	)

	# MSVC gets annoyed with our POSIX functions
	# we duplicate some catalyst macros just in case you're not using catalyst
	set (libWexpr_DEFINES
		_CRT_NONSTDC_NO_DEPRECATE=1
		_CRT_SECURE_NO_WARNINGS=1
		
		# similar to catalyst macros
		CATALYST_libWexpr_IS_BUILDING=1
	)

	set (libWexpr_SHAREDLIB_DEFINES
		# similar to catalyst macros
		CATALYST_libWexprIS_SHARED_LIBRARY=1
	)
	
	# if building for catalyst, build it as a module
	if (CATALYST_INSTALL_PREFIX)

		catalyst_begin_module (libWexpr ${libWexpr_HEADERS} ${libWexpr_PRIVATE_HEADERS} ${libWexpr_SOURCES})

			catalyst_module_copy_licenses (libWexpr ${libWexpr_SOURCE_DIR}/../LICENSE.txt)

			catalyst_append_target_property (libWexpr COMPILE_DEFINITIONS
				${libWexpr_DEFINES}
			)
			
			catalyst_module_append_target_property_shared (libWexpr COMPILE_DEFINITIONS
				${libWexpr_SHAREDLIB_DEFINES}
			)
			
		catalyst_end_module (libWexpr Modules/)

		catalyst_install_target (libWexpr Modules/)

	else () # otherwise build it as a normal static library

		add_library (libWexpr STATIC ${libWexpr_HEADERS} ${libWexpr_PRIVATE_HEADERS} ${libWexpr_SOURCES})

		set_property (TARGET libWexpr APPEND PROPERTY INCLUDE_DIRECTORIES "${libWexpr_SOURCE_DIR}/Public")
		set_property (TARGET libWexpr PROPERTY PREFIX "") # no prefix - we added the lib already
		set_property (TARGET libWexpr APPEND PROPERTY COMPILE_DEFINITIONS ${libWexpr_DEFINES})
		install (TARGETS libWexpr
			ARCHIVE DESTINATION lib/
		)

		install (FILES ${libWexpr_HEADERS}
			DESTINATION include/libWexpr/
		)

	endif ()

	add_subdirectory (Tests)

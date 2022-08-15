#
# libWexpr/libWexprSchema/CMakeLists.txt
# Schemas for W-Expressions
#

project (libWexprSchema)

	set (libWexprSchema_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/Public/libWexprSchema/Error.h
		${CMAKE_CURRENT_SOURCE_DIR}/Public/libWexprSchema/Global.h
		${CMAKE_CURRENT_SOURCE_DIR}/Public/libWexprSchema/libWexprSchema.h
		${CMAKE_CURRENT_SOURCE_DIR}/Public/libWexprSchema/Macros.h
		${CMAKE_CURRENT_SOURCE_DIR}/Public/libWexprSchema/PrimitiveType.h
		${CMAKE_CURRENT_SOURCE_DIR}/Public/libWexprSchema/Schema.h
		${CMAKE_CURRENT_SOURCE_DIR}/Public/libWexprSchema/Twine.h
		${CMAKE_CURRENT_SOURCE_DIR}/Public/libWexprSchema/Type.h
		${CMAKE_CURRENT_SOURCE_DIR}/Public/libWexprSchema/TypeInstance.h
	)
	
	set (libWexprSchema_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/Private/Error.c
		${CMAKE_CURRENT_SOURCE_DIR}/Private/ExternalOnigmo.h
		${CMAKE_CURRENT_SOURCE_DIR}/Private/Global.c
		${CMAKE_CURRENT_SOURCE_DIR}/Private/libWexprSchema.c
		${CMAKE_CURRENT_SOURCE_DIR}/Private/PrimitiveType.c
		${CMAKE_CURRENT_SOURCE_DIR}/Private/Schema.c
		${CMAKE_CURRENT_SOURCE_DIR}/Private/Twine.c
		${CMAKE_CURRENT_SOURCE_DIR}/Private/Type.c
		${CMAKE_CURRENT_SOURCE_DIR}/Private/TypeInstance.c
		${CMAKE_CURRENT_SOURCE_DIR}/Private/TypeRef.h
		${CMAKE_CURRENT_SOURCE_DIR}/Private/TypeRef.c
	)
	
	# MSVC gets annoyed with our POSIX functions
	# we duplicate some catalyst macros just in case you're not using catalyst
	set (libWexprSchema_DEFINES
		_CRT_NONSTDC_NO_DEPRECATE=1
		_CRT_SECURE_NO_WARNINGS=1
		
		# similar to catalyst macros
		CATALYST_libWexprSchema_IS_BUILDING=1
	)

	set (libWexprSchema_SHAREDLIB_DEFINES
		# similar to catalyst macros
		CATALYST_libWexprSchema_IS_SHARED_LIBRARY=1
	)
	
	# if building for catalyst, build it as a module
	if (CATALYST_INSTALL_PREFIX)

		catalyst_begin_module (libWexprSchema ${libWexprSchema_HEADERS} ${libWexprSchema_PRIVATE_HEADERS} ${libWexprSchema_SOURCES})

			catalyst_module_copy_licenses (libWexprSchema ${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE.txt)

			catalyst_append_target_property (libWexprSchema COMPILE_DEFINITIONS
				${libWexpr_DEFINES}
			)
			
			catalyst_module_append_target_property_shared (libWexprSchema COMPILE_DEFINITIONS
				${libWexpr_SHAREDLIB_DEFINES}
			)
			
			catalyst_module_use (libWexprSchema libWexpr Onigmo)
		catalyst_end_module (libWexprSchema Modules/)

		catalyst_install_target (libWexprSchema Modules/)

	else () # otherwise build it as a normal static library

		add_library (libWexprSchema STATIC ${libWexprSchema_HEADERS} ${libWexprSchema_PRIVATE_HEADERS} ${libWexprSchema_SOURCES})

		target_link_libraries (libWexprSchema libWexpr)
		
		set_property (TARGET libWexprSchema APPEND PROPERTY INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/Public")
		set_property (TARGET libWexprSchema PROPERTY PREFIX "") # no prefix - we added the lib already
		set_property (TARGET libWexprSchema APPEND PROPERTY COMPILE_DEFINITIONS ${libWexpr_DEFINES})
		
		install (TARGETS libWexprSchema
			ARCHIVE DESTINATION lib/
		)

		install (FILES ${libWexprSchema_HEADERS}
			DESTINATION include/libWexprSchema/
		)

	endif ()

	add_subdirectory (Tests)

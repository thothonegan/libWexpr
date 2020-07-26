#
# Documentation/Doxygen/CMakeLists.txt
# Builds Doxygen documentation
#

# Toplevel dir for source -- up 2 dirs
get_filename_component (cmakeSourceDirParent ${CMAKE_SOURCE_DIR} PATH)

string(TIMESTAMP currentDate "%Y-%m-%d")

# Write override file
file (WRITE ${CMAKE_BINARY_DIR}/Documentation/Doxygen/Doxyfile.override
	"STRIP_FROM_PATH = ${CMAKE_SOURCE_DIR}/libWexpr/Public/\n"
	"STRIP_FROM_INC_PATH = ${CMAKE_SOURCE_DIR}/libWexpr/Public/\n"
	"PREDEFINED += CMAKE_BUILD_TYPE_${CMAKE_BUILD_TYPE} \n"
	"HTML_HEADER = ${CMAKE_CURRENT_LIST_DIR}/html/header.html\n"
	"HTML_FOOTER = ${CMAKE_CURRENT_LIST_DIR}/html/footer.html\n"
	"HTML_EXTRA_STYLESHEET = ${CMAKE_CURRENT_LIST_DIR}/html/stylesheet.css\n"
	
	# Override version number
	"PROJECT_NUMBER = \"${LIBWEXPR_VERSION_MAJOR}.${LIBWEXPR_VERSION_MINOR}.${LIBWEXPR_VERSION_PATCH} - ${currentDate}\"\n"
)

# Add the libWexpr module
file (APPEND ${CMAKE_BINARY_DIR}/Documentation/Doxygen/Doxyfile.override
	"INPUT += \"${CMAKE_SOURCE_DIR}/libWexpr/Public/\"\n"
)

# Add our documentation folder - possibly at some point have a specific public folder or something
file (APPEND ${CMAKE_BINARY_DIR}/Documentation/Doxygen/Doxyfile.override
	"INPUT += \"${CMAKE_SOURCE_DIR}/Documentation/\"\n"
)

# Add our README.md
file (APPEND ${CMAKE_BINARY_DIR}/Documentation/Doxygen/Doxyfile.override
	"INPUT += \"${CMAKE_SOURCE_DIR}/README.md\"\n"
)

# Set readme as our index
file (APPEND ${CMAKE_BINARY_DIR}/Documentation/Doxygen/Doxyfile.override
	"USE_MDFILE_AS_MAINPAGE = \"${CMAKE_SOURCE_DIR}/README.md\"\n"
)

if (CATALYST_INSTALL_PREFIX)
	catalyst_doxygen_target (
		doxygen "wexpr" "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile"
		${CMAKE_CURRENT_BINARY_DIR}/html
		"Documentation/Doxygen/"
	)
else ()
	# roughly equivilant for non-catalyst
	add_custom_target (doxygen
		COMMAND doxygen ${CMAKE_SOURCE_DIR}/Documentation/Doxygen/Doxyfile

		# https://stackoverflow.com/questions/14678025/how-do-i-specify-a-keyword-for-dash-with-doxygen
		# set our keyword for searching in dash/zeal
		COMMAND perl -pi -w -e "s@<string>doxygen</string>@<string>wexpr</string>@s" ${CMAKE_BINARY_DIR}/Documentation/Doxygen/html/Info.plist

		DEPENDS ${CMAKE_SOURCE_DIR}/Documentation/Doxygen/Doxyfile
		COMMENT "Running Doxygen"
		USES_TERMINAL VERBATIM
	)
endif ()

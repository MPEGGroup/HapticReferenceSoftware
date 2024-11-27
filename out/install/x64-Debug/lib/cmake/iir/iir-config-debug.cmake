#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "iir::iir" for configuration "Debug"
set_property(TARGET iir::iir APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(iir::iir PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/iir.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS iir::iir )
list(APPEND _IMPORT_CHECK_FILES_FOR_iir::iir "${_IMPORT_PREFIX}/lib/iir.lib" )

# Import target "iir::iir_static" for configuration "Debug"
set_property(TARGET iir::iir_static APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(iir::iir_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/iir_static.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS iir::iir_static )
list(APPEND _IMPORT_CHECK_FILES_FOR_iir::iir_static "${_IMPORT_PREFIX}/lib/iir_static.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)

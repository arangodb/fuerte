# FindVelocypack
# --------
#
# Find ArangoDB Velocypack
#
# ::
#
#   VELOCYPACK_INCLUDE_DIRS   - where to find velocypack/vpack.h, etc.
#   VELOCYPACK_LIBRARIES      - List of libraries when using velocypack.
#   VELOCYPACK_FOUND          - True if velocypack found.
#   VELOCYPACK_VERSION_STRING - the version of velocypack found (since CMake 2.8.8)

# Look for the header file.
find_path(VELOCYPACK_INCLUDE_DIR NAMES velocypack/vpack.h)
mark_as_advanced(VELOCYPACK_INCLUDE_DIR)

# Look for the library (sorted from most current/relevant entry to least).
find_library(VELOCYPACK_LIBRARY NAMES
    libvelocypack
    libvelocypack.a
)
mark_as_advanced(VELOCYPACK_LIBRARY)

if(VELOCYPACK_INCLUDE_DIR)
  if(EXISTS "${VELOCYPACK_INCLUDE_DIR}/velocypack/velocypack-version-number.h")
    file(STRINGS "${VELOCYPACK_INCLUDE_DIR}/velocypack/velocypack-version-number.h" velocypack_version_str REGEX "^#define[\t ]+VELOCYPACK_VERSION[\t ]+\".*\"")

    string(REGEX REPLACE "^#define[\t ]+VELOCYPACK_VERSION[\t ]+\"([^\"]*)\".*" "\\1" VELOCYPACK_VERSION_STRING "${velocypack_version_str}")
    unset(velocypack_version_str)
  endif()
endif()

# handle the QUIETLY and REQUIRED arguments and set VELOCYPACK_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(VELOCYPACK
                                  REQUIRED_VARS VELOCYPACK_LIBRARY VELOCYPACK_INCLUDE_DIR
                                  VERSION_VAR VELOCYPACK_VERSION_STRING)

if(VELOCYPACK_FOUND)
  set(VELOCYPACK_LIBRARIES ${VELOCYPACK_LIBRARY})
  set(VELOCYPACK_INCLUDE_DIRS ${VELOCYPACK_INCLUDE_DIR})
endif()

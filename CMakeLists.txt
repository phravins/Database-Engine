cmake_minimum_required(VERSION 3.10)
project(mydb)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static linking for standalone binaries
if(WIN32 AND MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
elseif(UNIX)
    # Linux and macOS portable linking
    if(APPLE)
        # macOS doesn't support -static, but we can ensure dependencies are handled
        # Most system libs are standard, but we avoid non-standard dynamic linking where possible.
    else()
        # Linux static linking for portability
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    endif()
endif()

include_directories(include)

# Add source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create executable
add_executable(mydb ${SOURCES})

# Install
install(TARGETS mydb DESTINATION bin)

# Packaging
set(CPACK_PACKAGE_NAME "mydb-v2v")
set(CPACK_PACKAGE_VENDOR "V2V Systems")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "V2V Database - Educational C++ DB")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt") # Placeholder license

set(CPACK_GENERATOR "ZIP")
include(CPack)

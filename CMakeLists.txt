# CMakeList.txt : CMake project for HTTPSServer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("HTTPSServer")

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find OpenSSL
set(OPENSSL_ROOT_DIR "C:/Users/black/vcpkg/installed/x64-windows")
find_package(OpenSSL REQUIRED)

#Add sources
set(SOURCES "src/HTTPSServer.cpp" "src/HTTPSServer.h" "src/main.cpp")

# Add source to this project's executable.
add_executable (HTTPSServer ${SOURCES})

# Link OpenSSL
target_link_libraries(HTTPSServer PRIVATE OpenSSL::SSL OpenSSL::Crypto)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET HTTPSServer PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

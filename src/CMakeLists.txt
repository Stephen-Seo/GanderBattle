cmake_minimum_required(VERSION 3.25.1)
project(GanderBattle CXX)

set(GanderBattle_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/screen.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/shared_data.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/screen_debug.cc"
)

add_executable(GanderBattle ${GanderBattle_SOURCES})

if(NOT DEFINED CMAKE_BUILD_TYPE OR "${CMAKE_BUILD_TYPE}" STREQUAL "")
    message("Defaulting to \"Debug\" build type.")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
else()
    message("Using build type \"${CMAKE_BUILD_TYPE}\".")
endif()

target_compile_options(GanderBattle PUBLIC
$<IF:$<CONFIG:Debug>,-Og,-fno-delete-null-pointer-checks -fno-strict-overflow -fno-strict-aliasing -ftrivial-auto-var-init=zero>
-Wall -Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough
-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3
-D_GLIBCXX_ASSERTIONS
-fstrict-flex-arrays=3
-fstack-clash-protection -fstack-protector-strong
-Wl,-z,nodlopen -Wl,-z,noexecstack
-Wl,-z,relro -Wl,-z,now
-fPIE
)

target_link_options(GanderBattle PUBLIC
$<IF:$<CONFIG:Debug>,-Og,-fno-delete-null-pointer-checks -fno-strict-overflow -fno-strict-aliasing -ftrivial-auto-var-init=zero>
-Wall -Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough
-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3
-D_GLIBCXX_ASSERTIONS
-fstrict-flex-arrays=3
-fstack-clash-protection -fstack-protector-strong
-Wl,-z,nodlopen -Wl,-z,noexecstack
-Wl,-z,relro -Wl,-z,now
-fPIE
-pie
)

target_compile_features(GanderBattle PUBLIC cxx_std_20)

find_package(raylib 5.0 REQUIRED)

target_link_libraries(GanderBattle PUBLIC raylib)
target_include_directories(GanderBattle PUBLIC ${raylib_INCLUDE_DIRS})

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/3d_collision_helpers/CMakeLists.txt")
  message(FATAL_ERROR "3d_collision_helpers not found, please run \"git submodule update --init\"!")
else()
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../third_party/3d_collision_helpers" "${CMAKE_CURRENT_BINARY_DIR}/3d_collision_helpers")
  target_link_libraries(GanderBattle PUBLIC SC_3D_CollisionDetectionHelpers)
  target_include_directories(GanderBattle PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/3d_collision_helpers/src")
endif()

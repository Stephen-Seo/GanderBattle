cmake_minimum_required(VERSION 3.25.1)
project(GanderBattle C CXX)

set(GanderBattle_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/ems.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/screen.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/shared_data.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/screen_debug.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/screen_blank.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/screen_battle.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/resource_handler.cc"
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

target_compile_features(GanderBattle PUBLIC cxx_std_23)

find_package(raylib 5.0 REQUIRED)

target_link_libraries(GanderBattle PUBLIC raylib)
target_include_directories(GanderBattle PUBLIC ${raylib_INCLUDE_DIRS})

target_link_libraries(GanderBattle PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/lua/liblua.a")
target_include_directories(GanderBattle PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/lua")

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/3d_collision_helpers/CMakeLists.txt")
  message(FATAL_ERROR "3d_collision_helpers not found, please run \"git submodule update --init\"!")
else()
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../third_party/3d_collision_helpers" "${CMAKE_CURRENT_BINARY_DIR}/3d_collision_helpers")
  target_link_libraries(GanderBattle PUBLIC SC_3D_CollisionDetectionHelpers)
  target_include_directories(GanderBattle PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/3d_collision_helpers/src")
endif()

add_library(duktape "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/duktape/src/duktape.c")
target_link_libraries(GanderBattle PUBLIC duktape)
target_include_directories(GanderBattle
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/duktape/src")

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/ResourcePacker/src/CMakeLists.txt")
  message(WARNING "ResourcePacker not found, skipping... (ran \"git submodule update --init\"?)")
elseif(DEFINED DO_NOT_USE_RESOURCE_PACKER)
  message(WARNING "ResourcePacker will not be used as DO_NOT_USE_RESOURCE_PACKER is defined.")
  set(DO_NOT_USE_RESOURCE_PACKER "${DO_NOT_USE_RESOURCE_PACKER}")
else()
  message(NOTICE "Building with ResourcePacker support...")
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../third_party/ResourcePacker/src" "${CMAKE_CURRENT_BINARY_DIR}/rp")
  target_compile_definitions(ResourcePacker-s PRIVATE NDEBUG)
  target_link_libraries(GanderBattle PUBLIC ResourcePacker-s)
  target_include_directories(GanderBattle PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/ResourcePacker/src")
  target_compile_definitions(GanderBattle PRIVATE SEODISPARATE_RESOURCE_PACKER_AVAILABLE)

  if (DEFINED DO_NOT_CREATE_PACKFILE)
    message(NOTICE "Not creating packfile \"data\"...")
    set(DO_NOT_CREATE_PACKFILE "${DO_NOT_CREATE_PACKFILE}")
  else()
    message(NOTICE "Creating packfile \"data\" and making executable depend on it...")
    add_executable(ResourcePack "${CMAKE_CURRENT_SOURCE_DIR}/../src_build/rp_main.cpp")
    target_link_libraries(ResourcePack PUBLIC ResourcePacker-s)
    target_include_directories(ResourcePack PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/ResourcePacker/src")

    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/data" COMMAND "${CMAKE_CURRENT_BINARY_DIR}/ResourcePack" ARGS "${CMAKE_CURRENT_BINARY_DIR}/data" "${CMAKE_CURRENT_BINARY_DIR}/../res" DEPENDS ResourcePack)
    add_custom_target(ResourcePackData DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/data")
    add_dependencies(GanderBattle ResourcePackData)
  endif()
endif()

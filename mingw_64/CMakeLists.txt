cmake_minimum_required(VERSION 3.25.1)
project(GanderBattle CXX)

set(GanderBattle_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/../src/main.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/../src/ems.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/../src/screen.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/../src/shared_data.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/../src/screen_debug.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/../src/screen_battle.cc"
  "${CMAKE_CURRENT_BINARY_DIR}/resource_handler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/3d_collision_helpers/src/sc_sacd.cpp"
)

add_executable(GanderBattle ${GanderBattle_SOURCES})
target_include_directories(GanderBattle
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../src"
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/3d_collision_helpers/src"
)

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/resource_handler.cc"
  COMMAND cp
    ARGS
      "${CMAKE_CURRENT_SOURCE_DIR}/../src/resource_handler.cc"
      "${CMAKE_CURRENT_BINARY_DIR}/"
  COMMAND sed
    ARGS
      "-i" "-e" "/.*raylib.h/d"
      "-e" "/.*TraceLog/d" "${CMAKE_CURRENT_BINARY_DIR}/resource_handler.cc"
)
add_custom_target(PatchedResourceHandler DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/resource_handler.cc")
add_dependencies(GanderBattle PatchedResourceHandler)

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
-fPIE
-mwindows
)

target_link_options(GanderBattle PUBLIC
$<IF:$<CONFIG:Debug>,-Og,-fno-delete-null-pointer-checks -fno-strict-overflow -fno-strict-aliasing -ftrivial-auto-var-init=zero>
-Wall -Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough
-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3
-D_GLIBCXX_ASSERTIONS
-fstrict-flex-arrays=3
-fstack-clash-protection -fstack-protector-strong
-fPIE
-pie
-mwindows
)

target_compile_features(GanderBattle PUBLIC cxx_std_20)

find_package(raylib 4.5 REQUIRED)

target_link_libraries(GanderBattle PUBLIC raylib)
target_include_directories(GanderBattle PUBLIC ${raylib_INCLUDE_DIRS})

if(LINUX)
  target_link_libraries(GanderBattle PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/lua/liblua.a")
elseif(MINGW OR WIN32)
  target_link_libraries(GanderBattle PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lua/liblua.a")
else()
  message(FATAL_ERROR "Unsupported Platform!")
endif()

target_include_directories(GanderBattle PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/lua")

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/3d_collision_helpers/src/sc_sacd.cpp")
  message(FATAL_ERROR "3d_collision_helpers not found, please run \"git submodule update --init\"!")
endif()

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

    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/rpacker"
      COMMAND g++
        ARGS
          "${CMAKE_CURRENT_SOURCE_DIR}/../src_build/rp_main.cpp"
          "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/ResourcePacker/src/ResourcePacker.cpp"
          "-o" "${CMAKE_CURRENT_BINARY_DIR}/rpacker"
          "-I${CMAKE_CURRENT_SOURCE_DIR}/../third_party/ResourcePacker/src"
          "-DNDEBUG"
    )
    add_custom_target(RPacker DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/rpacker")
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/data"
      COMMAND "${CMAKE_CURRENT_BINARY_DIR}/rpacker"
        ARGS "${CMAKE_CURRENT_BINARY_DIR}/data"
        "${CMAKE_CURRENT_BINARY_DIR}/../res"
      DEPENDS RPacker
    )
    add_custom_target(ResourcePackData DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/data")
    add_dependencies(GanderBattle ResourcePackData)
  endif()
endif()

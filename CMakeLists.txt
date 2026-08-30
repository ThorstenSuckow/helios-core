cmake_minimum_required(VERSION 4.0)
project(helios-core LANGUAGES CXX)

if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

include(CTest)

set(HELIOS_USE_DEVTOOLS_DEFAULT OFF)
if(PROJECT_IS_TOP_LEVEL)
    set(HELIOS_USE_DEVTOOLS_DEFAULT ON)
endif()

option(HELIOS_USE_DEVTOOLS "Load shared tooling from helios-devtools" ${HELIOS_USE_DEVTOOLS_DEFAULT})
set(HELIOS_DEVTOOLS_GIT_REPOSITORY "https://github.com/thorstensuckow/helios-devtools.git" CACHE STRING "helios-devtools git repository")
set(HELIOS_DEVTOOLS_GIT_TAG "main" CACHE STRING "helios-devtools git tag or commit")
set(HELIOS_DEVTOOLS_LOCAL_PATH "" CACHE PATH "Optional local helios-devtools checkout")

if(HELIOS_USE_DEVTOOLS)
    include(FetchContent)

    if(HELIOS_DEVTOOLS_LOCAL_PATH)
        FetchContent_Declare(
                helios_devtools
                SOURCE_DIR "${HELIOS_DEVTOOLS_LOCAL_PATH}"
        )
    else()
        FetchContent_Declare(
                helios_devtools
                GIT_REPOSITORY "${HELIOS_DEVTOOLS_GIT_REPOSITORY}"
                GIT_TAG "${HELIOS_DEVTOOLS_GIT_TAG}"
        )
    endif()

    FetchContent_MakeAvailable(helios_devtools)

    include("${helios_devtools_SOURCE_DIR}/cmake/HeliosDevtools.cmake")
endif()

if(NOT TARGET thirdparty::stb)
    include(FetchContent)
    FetchContent_Declare(stb
            GIT_REPOSITORY https://github.com/nothings/stb.git
            GIT_TAG master
    )
    FetchContent_MakeAvailable(stb)

    add_library(thirdparty_stb INTERFACE)
    target_include_directories(thirdparty_stb INTERFACE ${stb_SOURCE_DIR})
    add_library(thirdparty::stb ALIAS thirdparty_stb)
endif()

if(CMAKE_IFC_OUTPUT_DIRECTORY)
    file(MAKE_DIRECTORY "${CMAKE_IFC_OUTPUT_DIRECTORY}")
endif()

file(GLOB_RECURSE CPP_FILES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

file(GLOB_RECURSE MODULE_INTERFACE_FILES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.ixx")

list(FILTER CPP_FILES EXCLUDE REGEX "/CMakeFiles/|/CMakeTmp/|/cmake_build_|/build/")
list(FILTER MODULE_INTERFACE_FILES EXCLUDE REGEX "/CMakeFiles/|/CMakeTmp/|/cmake_build_|/build/")

add_library(helios_core STATIC)
add_library(helios::core ALIAS helios_core)
set_property(TARGET helios_core PROPERTY CXX_SCAN_FOR_MODULES ON)
target_compile_features(helios_core PUBLIC cxx_std_23)
target_link_libraries(helios_core PRIVATE thirdparty::stb)

target_sources(helios_core PRIVATE ${CPP_FILES})

if(MODULE_INTERFACE_FILES)
    target_sources(helios_core PUBLIC
            FILE_SET modules TYPE CXX_MODULES
            BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src"
            FILES ${MODULE_INTERFACE_FILES}
    )
endif()


target_include_directories(helios_core PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

if(HELIOS_USE_DEVTOOLS)
    helios_configure_target(helios_core)
endif()

if(PROJECT_IS_TOP_LEVEL AND BUILD_TESTING)
    include(FetchContent)

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.17.0
    )
    FetchContent_MakeAvailable(googletest)

    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.test.cpp")

    add_executable(helios-core-tests ${TEST_SOURCES})
    target_link_libraries(helios-core-tests PRIVATE helios_core GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(helios-core-tests)
endif()

set(HELIOS_BUILD_BENCHMARKS_DEFAULT OFF)
if(PROJECT_IS_TOP_LEVEL)
    set(HELIOS_BUILD_BENCHMARKS_DEFAULT ON)
endif()

option(HELIOS_BUILD_BENCHMARKS "Build helios-core benchmarks" ${HELIOS_BUILD_BENCHMARKS_DEFAULT})
if(HELIOS_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()


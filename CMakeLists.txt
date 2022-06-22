cmake_minimum_required(VERSION 3.10.0)

# Extract package name and version from package.xml
find_package(ros_industrial_cmake_boilerplate REQUIRED)
extract_package_metadata(pkg)
project(${pkg_extracted_name} VERSION ${pkg_extracted_version} LANGUAGES CXX)

if(WIN32)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

find_package(Boost REQUIRED COMPONENTS filesystem)

option(BUILD_SHARED_LIBS "Enables build shared libraries" ON)
option(ENABLE_CLANG_TIDY "Enables compilation with clang-tidy" OFF)
option(ENABLE_CODE_COVERAGE "Enables compilation with code coverage" OFF)
option(BUILD_TESTING "Enables compilation of unit tests" OFF)
option(ENABLE_RUN_TESTING "Enables running of unit tests as a part of the build" OFF)
option(ENABLE_CPACK "Enable cpack to generate debian or nuget packages" OFF)

set(COMPILE_DEFINITIONS "")
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(COMPILE_DEFINITIONS "_USE_MATH_DEFINES=ON")
endif()

set(COVERAGE_EXCLUDE
    /usr/*
    /opt/*
    ${CMAKE_CURRENT_LIST_DIR}/test/*
    /*/gtest/*)

initialize_code_coverage(ENABLE ${ENABLE_CODE_COVERAGE})
add_code_coverage_all_targets(EXCLUDE ${COVERAGE_EXCLUDE} ENABLE ${ENABLE_CODE_COVERAGE})

add_library(${PROJECT_NAME} src/utils.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
                                                  "$<INSTALL_INTERFACE:include>")
target_link_libraries(${PROJECT_NAME} PUBLIC Boost::boost Boost::filesystem ${CMAKE_DL_LIBS})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${COMPILE_DEFINITIONS})
target_cxx_version(${PROJECT_NAME} PUBLIC VERSION 17)
target_clang_tidy(${PROJECT_NAME} ENABLE ${ENABLE_CLANG_TIDY})
target_code_coverage(${PROJECT_NAME} ALL ENABLE ${ENABLE_CODE_COVERAGE})

# Build examples
add_subdirectory(examples)

# Install headers
install(DIRECTORY include/${PROJECT_NAME}/ DESTINATION include/${PROJECT_NAME})

# Unit tests
if(BUILD_TESTING)
  enable_testing()
  add_run_tests_target(ENABLE ${ENABLE_RUN_TESTING})
  add_subdirectory(test)
endif()

# Package configuration
configure_package(NAMESPACE ${PROJECT_NAME} DEPENDENCIES "Boost REQUIRED COMPONENTS filesystem" TARGETS ${PROJECT_NAME})

if(ENABLE_CPACK)
  cpack(
    VERSION
    ${pkg_extracted_version}
    MAINTAINER
    <https://github.com/tesseract-robotics/boost_plugin_loader>
    VENDOR
    "ROS-Industrial"
    DESCRIPTION
    "${PROJECT_NAME}"
    LICENSE_FILE
    ${CMAKE_CURRENT_LIST_DIR}/LICENSE.Apache-2.0
    README_FILE
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    LINUX_DEPENDS
    "libboost-filesystem1.71.0"
    WINDOWS_DEPENDS
    "boost_filesystem")
endif()

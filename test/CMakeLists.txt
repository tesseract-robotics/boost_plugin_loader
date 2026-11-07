find_gtest()

add_library(${PROJECT_NAME}_test_plugin INTERFACE)
target_compile_definitions(${PROJECT_NAME}_test_plugin INTERFACE SECTION_MULTIPLY=mult SECTION_ADD=add
                                                                 SYMBOL_NAME=plugin)

add_library(${PROJECT_NAME}_test_plugin_multiply test_plugin_multiply.cpp)
target_link_libraries(${PROJECT_NAME}_test_plugin_multiply PUBLIC ${PROJECT_NAME} ${PROJECT_NAME}_test_plugin
                                                                  Boost::boost)
target_compile_definitions(${PROJECT_NAME}_test_plugin_multiply PUBLIC ${COMPILE_DEFINITIONS})
target_clang_tidy(${PROJECT_NAME}_test_plugin_multiply ENABLE ${ENABLE_CLANG_TIDY})
target_cxx_version(${PROJECT_NAME}_test_plugin_multiply PUBLIC VERSION 17)

add_library(${PROJECT_NAME}_test_plugin_add test_plugin_add.cpp)
target_link_libraries(${PROJECT_NAME}_test_plugin_add PUBLIC ${PROJECT_NAME} ${PROJECT_NAME}_test_plugin Boost::boost)
target_compile_definitions(${PROJECT_NAME}_test_plugin_add PUBLIC ${COMPILE_DEFINITIONS})
target_clang_tidy(${PROJECT_NAME}_test_plugin_add ENABLE ${ENABLE_CLANG_TIDY})
target_cxx_version(${PROJECT_NAME}_test_plugin_add PUBLIC VERSION 17)

add_executable(${PROJECT_NAME}_plugin_loader_unit plugin_loader_unit.cpp)
target_link_libraries(
  ${PROJECT_NAME}_plugin_loader_unit
  PRIVATE GTest::GTest
          GTest::Main
          ${PROJECT_NAME}
          ${PROJECT_NAME}_test_plugin)
target_compile_definitions(${PROJECT_NAME}_plugin_loader_unit PRIVATE ${COMPILE_DEFINITIONS})
target_compile_definitions(
  ${PROJECT_NAME}_plugin_loader_unit
  PRIVATE PLUGIN_DIR="${CMAKE_CURRENT_BINARY_DIR}" PLUGINS_MULTIPLY="${PROJECT_NAME}_test_plugin_multiply"
          PLUGINS_ADD="${PROJECT_NAME}_test_plugin_add")
target_clang_tidy(${PROJECT_NAME}_plugin_loader_unit ENABLE ${ENABLE_CLANG_TIDY})
target_cxx_version(${PROJECT_NAME}_plugin_loader_unit PUBLIC VERSION 17)
target_code_coverage(
  ${PROJECT_NAME}_plugin_loader_unit
  PRIVATE
  ALL
  ENABLE ${ENABLE_CODE_COVERAGE})
add_gtest_discover_tests(${PROJECT_NAME}_plugin_loader_unit)
add_dependencies(${PROJECT_NAME}_plugin_loader_unit ${PROJECT_NAME})
add_dependencies(run_tests ${PROJECT_NAME}_plugin_loader_unit)

add_executable(${PROJECT_NAME}_plugin_loader_anchor_unit plugin_loader_anchor_unit.cpp)
target_link_libraries(
  ${PROJECT_NAME}_plugin_loader_anchor_unit
  PRIVATE GTest::GTest
          GTest::Main
          ${PROJECT_NAME}
          ${PROJECT_NAME}_test_plugin
          ${PROJECT_NAME}_test_plugin_multiply)
target_compile_definitions(${PROJECT_NAME}_plugin_loader_anchor_unit PRIVATE ${COMPILE_DEFINITIONS})
target_clang_tidy(${PROJECT_NAME}_plugin_loader_anchor_unit ENABLE ${ENABLE_CLANG_TIDY})
target_cxx_version(${PROJECT_NAME}_plugin_loader_anchor_unit PUBLIC VERSION 17)
target_code_coverage(
  ${PROJECT_NAME}_plugin_loader_anchor_unit
  PRIVATE
  ALL
  ENABLE ${ENABLE_CODE_COVERAGE})
add_gtest_discover_tests(${PROJECT_NAME}_plugin_loader_anchor_unit)
add_dependencies(${PROJECT_NAME}_plugin_loader_anchor_unit ${PROJECT_NAME})
add_dependencies(run_tests ${PROJECT_NAME}_plugin_loader_anchor_unit)

install(
  TARGETS ${PROJECT_NAME}_test_plugin_multiply
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

# Not a test: prints createInstance throughput for 1 vs N threads (#45). Run by hand.
add_executable(${PROJECT_NAME}_plugin_loader_concurrency_bench plugin_loader_concurrency_bench.cpp)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME}_plugin_loader_concurrency_bench
                      PRIVATE ${PROJECT_NAME} ${PROJECT_NAME}_test_plugin Threads::Threads)
target_compile_definitions(${PROJECT_NAME}_plugin_loader_concurrency_bench PRIVATE ${COMPILE_DEFINITIONS})
target_compile_definitions(
  ${PROJECT_NAME}_plugin_loader_concurrency_bench PRIVATE PLUGIN_DIR="${CMAKE_CURRENT_BINARY_DIR}"
                                                          PLUGINS_MULTIPLY="${PROJECT_NAME}_test_plugin_multiply")
target_cxx_version(${PROJECT_NAME}_plugin_loader_concurrency_bench PUBLIC VERSION 17)
add_dependencies(${PROJECT_NAME}_plugin_loader_concurrency_bench ${PROJECT_NAME}_test_plugin_multiply)

# Not a test: prints the cost of the first createInstance through a new loader against the number of loaded images.
# macOS only: there boost::dll resolves a handle's path by scanning every loaded image. Run by hand, once per count.
if(APPLE)
  add_executable(${PROJECT_NAME}_plugin_loader_image_count_bench plugin_loader_image_count_bench.cpp)
  target_link_libraries(${PROJECT_NAME}_plugin_loader_image_count_bench PRIVATE ${PROJECT_NAME}
                                                                                ${PROJECT_NAME}_test_plugin)
  target_compile_definitions(${PROJECT_NAME}_plugin_loader_image_count_bench PRIVATE ${COMPILE_DEFINITIONS})
  target_compile_definitions(
    ${PROJECT_NAME}_plugin_loader_image_count_bench PRIVATE PLUGIN_DIR="${CMAKE_CURRENT_BINARY_DIR}"
                                                            PLUGINS_MULTIPLY="${PROJECT_NAME}_test_plugin_multiply")
  target_cxx_version(${PROJECT_NAME}_plugin_loader_image_count_bench PUBLIC VERSION 17)
  add_dependencies(${PROJECT_NAME}_plugin_loader_image_count_bench ${PROJECT_NAME}_test_plugin_multiply)
endif()

add_executable(${PROJECT_NAME}_plugin_loader_image_location_unit plugin_loader_image_location_unit.cpp)
target_link_libraries(${PROJECT_NAME}_plugin_loader_image_location_unit PRIVATE GTest::GTest GTest::Main
                                                                                ${PROJECT_NAME})
target_compile_definitions(${PROJECT_NAME}_plugin_loader_image_location_unit PRIVATE ${COMPILE_DEFINITIONS})
target_compile_definitions(
  ${PROJECT_NAME}_plugin_loader_image_location_unit
  PRIVATE PLUGIN_DIR="${CMAKE_CURRENT_BINARY_DIR}" PLUGINS_MULTIPLY="${PROJECT_NAME}_test_plugin_multiply"
          PLUGINS_ADD="${PROJECT_NAME}_test_plugin_add")
target_clang_tidy(${PROJECT_NAME}_plugin_loader_image_location_unit ENABLE ${ENABLE_CLANG_TIDY})
target_cxx_version(${PROJECT_NAME}_plugin_loader_image_location_unit PUBLIC VERSION 17)
add_gtest_discover_tests(${PROJECT_NAME}_plugin_loader_image_location_unit)
add_dependencies(${PROJECT_NAME}_plugin_loader_image_location_unit ${PROJECT_NAME}_test_plugin_multiply
                 ${PROJECT_NAME}_test_plugin_add)
add_dependencies(run_tests ${PROJECT_NAME}_plugin_loader_image_location_unit)

# The resolver must name the file dyld mapped, not the path requested: DYLD_LIBRARY_PATH points dyld at a copy of a test
# library with the same file name. macOS only; the test is run with the variable set, in its own process.
if(APPLE)
  set(REMAP_DIR "${CMAKE_CURRENT_BINARY_DIR}/remap")
  add_executable(${PROJECT_NAME}_plugin_loader_image_location_remap_unit plugin_loader_image_location_remap_unit.cpp)
  target_link_libraries(${PROJECT_NAME}_plugin_loader_image_location_remap_unit PRIVATE GTest::GTest GTest::Main
                                                                                        ${PROJECT_NAME})
  target_compile_definitions(${PROJECT_NAME}_plugin_loader_image_location_remap_unit PRIVATE ${COMPILE_DEFINITIONS})
  target_compile_definitions(
    ${PROJECT_NAME}_plugin_loader_image_location_remap_unit
    PRIVATE PLUGIN_DIR="${CMAKE_CURRENT_BINARY_DIR}" PLUGINS_MULTIPLY="${PROJECT_NAME}_test_plugin_multiply"
            REMAP_DIR="${REMAP_DIR}")
  target_cxx_version(${PROJECT_NAME}_plugin_loader_image_location_remap_unit PUBLIC VERSION 17)
  add_dependencies(${PROJECT_NAME}_plugin_loader_image_location_remap_unit ${PROJECT_NAME}_test_plugin_multiply)
  add_custom_command(
    TARGET ${PROJECT_NAME}_plugin_loader_image_location_remap_unit
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${REMAP_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}_test_plugin_multiply> ${REMAP_DIR}/)
  add_test(NAME ${PROJECT_NAME}_plugin_loader_image_location_remap_unit
           COMMAND ${PROJECT_NAME}_plugin_loader_image_location_remap_unit)
  set_tests_properties(${PROJECT_NAME}_plugin_loader_image_location_remap_unit
                       PROPERTIES ENVIRONMENT "DYLD_LIBRARY_PATH=${REMAP_DIR}")
  add_dependencies(run_tests ${PROJECT_NAME}_plugin_loader_image_location_remap_unit)
endif()

set(TESTS_SOURCE_FILES main.test.cpp parser.test.cpp utility.test.hpp)

target_sources_local(tests PRIVATE ${TESTS_SOURCE_FILES})
set_target_properties(tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/bin)
target_include_directories(tests PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(tests PRIVATE "${DAVISBASE_SOURCE_DIR}")
target_link_libraries(tests ${Boost_LIBRARIES})

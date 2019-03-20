set(DAVISBASE_SOURCE_FILES main.cpp parser.hpp parser.cpp ast.hpp ast.cpp
    util.hpp boost_test.hpp)

target_sources_local(davisbase PRIVATE ${DAVISBASE_SOURCE_FILES})
set_target_properties(davisbase PROPERTIES RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/bin)
target_include_directories(davisbase PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(davisbase ${Boost_LIBRARIES})

set(TIDY_TESTS
    # from tokenizer
    tests/test_tokenizer.cpp

    # from parser
    tests/test_parser.cpp

    # from basic
    tests/test_boolean.cpp
    tests/test_eval.cpp
    tests/test_integer.cpp
    tests/test_list.cpp
    tests/test_fuzzing_2.cpp

    tests/test_symbol.cpp
    tests/test_pair_mut.cpp
    tests/test_control_flow.cpp
    tests/test_lambda.cpp)

add_catch(test_scheme_tidy
    ${TIDY_TESTS})

include(sources.cmake)

target_include_directories(scheme_tidy PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SCHEME_COMMON_DIR})

target_link_libraries(test_scheme_tidy
    scheme_tidy
    allocations_checker)

add_executable(scheme_tidy_repl repl/main.cpp)
target_link_libraries(scheme_tidy_repl scheme_tidy)

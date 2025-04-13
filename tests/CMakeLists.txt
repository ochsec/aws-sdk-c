add_executable(aws-sdk-c-tests
    sigv4_test.c
)

target_link_libraries(aws-sdk-c-tests
    aws-sdk-c-core
    aws-c-common
    aws-c-auth
)

target_include_directories(aws-sdk-c-tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

add_test(NAME sigv4_tests COMMAND aws-sdk-c-tests)

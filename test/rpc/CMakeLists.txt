
file(GLOB_RECURSE ALL_TEST_FILES "*.cc" "*.h")

link_directories(third_party)

add_executable(test_proto ${ALL_TEST_FILES})

target_link_libraries(test_proto PRIVATE co tit_base tit_log ${PROTOBUF_LIBRARY})


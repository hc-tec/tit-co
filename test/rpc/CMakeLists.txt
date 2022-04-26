
file(GLOB_RECURSE ALL_TEST_FILES "*.cc" "*.h")

link_directories(third_party)

add_executable(rpc_server rpc_server.cc)

target_link_libraries(rpc_server PRIVATE co tit_base tit_log ${PROTOBUF_LIBRARY})

add_executable(rpc_client rpc_client.cc)
target_link_libraries(rpc_client PRIVATE co tit_base tit_log ${PROTOBUF_LIBRARY})


file(GLOB_RECURSE CO_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

set(ASM_FILES context/context.S)

list(APPEND CO_SRC_FILES ${ASM_FILES})

add_library(co ${CO_SRC_FILES})
#add_library(cocoyaxi::co ALIAS co)

target_include_directories(co
        PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

install(
        TARGETS co
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

#file(GLOB HEADERS "${PROJECT_SOURCE_DIR}/include/*.h")
#install(FILES ${HEADERS} DESTINATION co)

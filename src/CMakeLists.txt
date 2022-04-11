
file(GLOB_RECURSE CO_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

if(MSVC)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(ASM_FILES context/context_x86.asm)
    else()
        set(ASM_FILES context/context_x64.asm)
    endif()
    set_property(SOURCE ${ASM_FILES} PROPERTY LANGUAGE ASM_MASM)
else()
    set(ASM_FILES context/context.S)
endif()

list(APPEND CO_SRC_FILES ${ASM_FILES})

add_library(co ${CO_SRC_FILES})
#add_library(cocoyaxi::co ALIAS co)

target_include_directories(co
        PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>
)

install(
        TARGETS co
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

#install(
#        DIRECTORY ${PROJECT_SOURCE_DIR}/include
#        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/include
#)

#file(GLOB HEADERS "${PROJECT_SOURCE_DIR}/include/*.h")
#install(FILES ${HEADERS} DESTINATION co)

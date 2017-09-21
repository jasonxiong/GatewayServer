file(GLOB SRC_FILES *.hpp *.h *.cpp *.cxx *.c )

add_executable(${GATEWAY_SVR_EXE} ${SRC_FILES})

add_definitions(-D_GATEWAY_USE_64BIT_)
target_link_libraries(${GATEWAY_SVR_EXE} ${SYSTEM_LAS})

install(TARGETS ${GATEWAY_SVR_EXE} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})

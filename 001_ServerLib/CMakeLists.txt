file(GLOB SRC_FILES BaseLibs/*)

add_library(${SERVERLIB_LAS}  STATIC  ${SRC_FILES})
target_link_libraries(${SERVERLIB_LAS} ${SYSTEM_LAS})

add_library(${SERVERLIB_MTLAS}  STATIC  ${SRC_FILES})
set_target_properties(${SERVERLIB_MTLAS}  PROPERTIES COMPILE_FLAGS "-D_POSIX_MT_")
target_link_libraries(${SERVERLIB_MTLAS} ${SYSTEM_LAS})


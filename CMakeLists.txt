cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{DOLCESDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{DOLCESDK}/share/dolce.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define DOLCESDK to point to your SDK path!")
  endif()
endif()

project(PIBConfig)

include("${DOLCESDK}/share/dolce.cmake" REQUIRED)

set(DOLCE_APP_NAME "PIB Configuration Tool")
set(DOLCE_TITLEID  "PIGS00001")

set(DOLCE_VERSION  "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(DOLCE_MKSFOEX_FLAGS "${DOLCE_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")

function(ADD_RESOURCES out_var)
  set(result)
  foreach(in_f ${ARGN})
    set(out_f "${CMAKE_CURRENT_BINARY_DIR}/${in_f}.o")
    get_filename_component(out_dir ${out_f} DIRECTORY)
    add_custom_command(OUTPUT ${out_f}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${out_dir}
      COMMAND ${CMAKE_LINKER} -r -b binary -o ${out_f} ${in_f}
      DEPENDS ${in_f}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Building resource ${out_f}"
      VERBATIM
      )
    list(APPEND result ${out_f})
  endforeach()
  set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

file(GLOB res_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} resources/*.suprx)
add_resources(suprx_res ${res_files})

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${PROJECT_NAME}
  ${suprx_res}
  src/main.c
  src/ctrl.c
  src/gxm.c
  common/debugScreen.c
)

target_link_libraries(${PROJECT_NAME}
  SceLibKernel_stub
  SceIofilemgr_stub
  SceAppMgr_stub
  SceCtrl_stub
  SceDisplay_stub
  SceDisplayUser_stub
  SceCommonDialog_stub
  SceGxm_stub
  SceVshBridge_stub 
)

dolce_create_self(eboot.bin ${PROJECT_NAME} UNSAFE)

dolce_create_vpk(${PROJECT_NAME}.vpk ${DOLCE_TITLEID} eboot.bin
  VERSION ${DOLCE_VERSION}
  NAME ${DOLCE_APP_NAME}

  #FILE sce_sys/icon0.png sce_sys/icon0.png
  #FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  #FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)
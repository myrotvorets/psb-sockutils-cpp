get_filename_component(PSB_SOCKUTILS_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

list(APPEND CMAKE_MODULE_PATH ${PSB_SOCKUTILS_CMAKE_DIR})

if(NOT TARGET psb-sockutils)
    include("${PSB_SOCKUTILS_CMAKE_DIR}/psb-sockutils-target.cmake")
    add_library(psb::sockutils ALIAS psb-sockutils)
endif()

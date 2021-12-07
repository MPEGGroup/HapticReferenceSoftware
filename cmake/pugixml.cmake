include(FetchContent)
FetchContent_Declare(pugixml
        GIT_REPOSITORY "https://github.com/zeux/pugixml"
        GIT_TAG "origin/master"
        GIT_SHALLOW ON
        )
set(PUGIXML_NO_XPATH ON CACHE INTERNAL "")
FetchContent_MakeAvailable(pugixml)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_BINARY_DIR}/_deps/pugixml-src/")
include_directories(${CMAKE_BINARY_DIR}/_deps/pugixml-src/src/)
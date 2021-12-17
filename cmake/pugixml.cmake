include(FetchContent)
foreach (lang IN ITEMS C CXX)
    set("CMAKE_${lang}_CLANG_TIDY_save" "${CMAKE_${lang}_CLANG_TIDY}")
    set("CMAKE_${lang}_CLANG_TIDY" "")
endforeach ()
FetchContent_Declare(pugixml
        GIT_REPOSITORY "https://github.com/zeux/pugixml"
        GIT_TAG "origin/master"
        GIT_SHALLOW ON
        )
set(PUGIXML_NO_XPATH ON CACHE INTERNAL "")
FetchContent_MakeAvailable(pugixml)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_BINARY_DIR}/_deps/pugixml-src/")
include_directories(${CMAKE_BINARY_DIR}/_deps/pugixml-src/src/)
foreach (lang IN ITEMS C CXX)
    set("CMAKE_${lang}_CLANG_TIDY" "${CMAKE_${lang}_CLANG_TIDY_save}")
endforeach ()

cmake_minimum_required(VERSION 4.3)
project(HistoVis LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(HISTOVIS_BUILD_CLI "CLI" ON)
option(HISTOVIS_BUILD_GUI "GUI" ON)
add_library(histovis_core STATIC
        src/core/Image.h
        src/core/Image.cpp
        src/core/StbImpl.cpp
        src/core/Histogram.h
        src/core/Histogram.cpp
        src/core/ColorSpace.cpp
        src/core/ColorSpace.h
        src/core/Equalization.cpp
        src/core/Equalization.h
        src/core/LUT.h
        src/core/LUT.cpp
        src/core/Matching.h
        src/core/Matching.cpp
)
target_include_directories(histovis_core
        PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/src
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external
)
target_compile_options(histovis_core PRIVATE -Wall -Wextra -Wpedantic)
set_source_files_properties(src/core/StbImpl.cpp PROPERTIES COMPILE_OPTIONS "-w")

if (HISTOVIS_BUILD_CLI)
    add_executable(histovis
            src/cli/main.cpp
            src/cli/ArgParser.h
            src/cli/Commands.h
    )
    target_link_libraries(histovis PRIVATE histovis_core)
    target_compile_options(histovis PRIVATE -Wall -Wextra -Wpedantic)
endif ()

if (HISTOVIS_BUILD_GUI)
    add_subdirectory(src/gui)
endif ()
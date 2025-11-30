cmake_minimum_required(VERSION 3.16)
project(bgp_simulator_wasm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Emscripten settings
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".js")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SIMULATOR_SOURCES
    src/AS.cpp
    src/ASGraph.cpp
    src/Announcement.cpp
    src/Policy.cpp
    src/ROV.cpp
    src/Community.cpp
    src/Aggregation.cpp
    src/Statistics.cpp
    src/Csvoutput.cpp
    src/CSVInput.cpp
)

# WASM executable
add_executable(bgp_simulator_wasm
    src/wasm_interface.cpp
    ${SIMULATOR_SOURCES}
)

# Emscripten-specific compilation flags
if(EMSCRIPTEN)
    target_link_options(bgp_simulator_wasm PRIVATE
        "SHELL:-lembind"
        "SHELL:-s WASM=1"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s INITIAL_MEMORY=256MB"
        "SHELL:-s MAXIMUM_MEMORY=512MB"
    )

    # Output to dist directory
    set_target_properties(bgp_simulator_wasm PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dist"
    )
endif()

# project info
cmake_minimum_required(VERSION 3.28)
project(master VERSION 0.1.0 LANGUAGES C CXX)

# set some option
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SOEM_BUILD_SAMPLES ON)

# libs
add_subdirectory(lib/SOEM)
include(lib/imgui.cmake)
include(lib/implot.cmake)

add_executable(master master.cpp imgui_main.cpp fieldbus.c)

# link libraries
target_link_libraries(master PRIVATE soem)
target_link_libraries(master PRIVATE imgui)
target_link_libraries(master PRIVATE implot)
target_link_libraries(master PRIVATE modbus)
target_link_libraries(master PRIVATE pthread)

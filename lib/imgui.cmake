project(imgui)

message("imgui cmake..")

add_library(imgui
    lib/imgui/imgui.cpp
    lib/imgui/imgui_demo.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/backends/imgui_impl_sdl2.cpp
    lib/imgui/backends/imgui_impl_opengl3.cpp
)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

target_link_libraries(imgui PUBLIC SDL2::SDL2)
target_link_libraries(imgui PUBLIC OpenGL::GL)

include_directories(lib/imgui)
include_directories(lib/imgui/backends)
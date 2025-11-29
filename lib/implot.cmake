project(implot)

message("implot cmake..")

add_library(implot
    lib/implot/implot.cpp
    lib/implot/implot_demo.cpp
    lib/implot/implot_items.cpp
)

include_directories(lib/implot)
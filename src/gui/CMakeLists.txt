find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(histovis_gui
        main.cpp
        MainWindow.h
        MainWindow.cpp
        PlotWidget.h
        PlotWidget.cpp
)
target_link_libraries(histovis_gui PRIVATE histovis_core Qt6::Widgets)
target_compile_options(histovis_gui PRIVATE ${HISTOVIS_WARNINGS})
set_target_properties(histovis_gui PROPERTIES WIN32_EXECUTABLE ON)
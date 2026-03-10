cmake_minimum_required(VERSION 3.20)
project(Autoclick LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Autoclick WIN32
        main.cpp
        icon/resource.rc

        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_tables.cpp
        imgui/backends/imgui_impl_win32.cpp
        imgui/backends/imgui_impl_dx11.cpp
        module/module.h
        module/impl/click_input.hpp
        icon/resource.h
        icon/resource.rc
)

set_source_files_properties(icon/resource.rc PROPERTIES
        LANGUAGE RC
)

target_include_directories(Autoclick PRIVATE
        imgui
        imgui/backends
)

target_link_libraries(Autoclick PRIVATE
        kernel32
        user32
        gdi32
        d3d11
        dxgi
        d3dcompiler
        dwmapi
        winmm
)

target_compile_definitions(Autoclick PRIVATE
        UNICODE
        _UNICODE
)

if (MINGW)
    target_link_options(Autoclick PRIVATE
            -static
            -static-libgcc
            -static-libstdc++
    )
endif()
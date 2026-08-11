cmake_minimum_required(VERSION 3.16)
project(cache_simulator_wxwidgets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cache_simulator WIN32 main.cpp gui.cpp simulator.cpp)

find_package(wxWidgets CONFIG QUIET COMPONENTS core base)

if(wxWidgets_FOUND AND TARGET wx::core)
    target_link_libraries(cache_simulator PRIVATE wx::core wx::base)
else()
    find_package(wxWidgets REQUIRED COMPONENTS core base)
    include(${wxWidgets_USE_FILE})
    target_link_libraries(cache_simulator PRIVATE ${wxWidgets_LIBRARIES})
endif()

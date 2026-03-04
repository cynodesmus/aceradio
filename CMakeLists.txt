cmake_minimum_required(VERSION 3.14)
project(MusicGeneratorGUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt packages
find_package(Qt6 COMPONENTS Core Gui Widgets Multimedia REQUIRED)

# Include acestep.cpp as a subdirectory
#add_subdirectory(acestep.cpp)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Add executable
add_executable(MusicGeneratorGUI
    main.cpp
    MainWindow.ui
    MainWindow.cpp
    SongListModel.cpp
    AudioPlayer.cpp
    AceStepWorker.cpp
    ${MusicGeneratorGUI_H}
)

# UI file
target_include_directories(MusicGeneratorGUI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link libraries
target_link_libraries(MusicGeneratorGUI PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    acestep-core
)

# Include directories
target_include_directories(MusicGeneratorGUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/acestep.cpp/src
    ${CMAKE_CURRENT_SOURCE_DIR}/acestep.cpp/ggml/include
)

# Copy models directory if it exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/acestep.cpp/models")
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/acestep.cpp/models" DESTINATION "${CMAKE_BINARY_DIR}")
endif()

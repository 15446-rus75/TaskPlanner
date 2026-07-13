cmake_minimum_required(VERSION 3.16)
project(TaskPlanner VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core Test)

set(STORAGE_SOURCES
    src/storage/task.hpp
    src/storage/task.cpp
    src/storage/istorage.hpp
    src/storage/memorystorage.hpp
    src/storage/memorystorage.cpp
    src/storage/serial-utils.hpp
    src/storage/serial-utils.cpp
)

set(VIEW_SOURCES
    src/view/iview.hpp
    src/view/igamification_view.hpp
    src/view/taskplannerview.hpp
    src/view/taskplannerview.cpp
    src/view/taskplanner.ui
    src/view/gamification_view.hpp
    src/view/gamification_view.cpp
)

set(CONTROLLER_SOURCES
    src/controller/icontroller.hpp
    src/controller/controller.hpp
    src/controller/controller.cpp
)

set(UTILS_SOURCES
    src/utils/achievements.hpp
    src/utils/locations.hpp
    src/utils/sdo_links.hpp
)

set(RESOURCES
    resources.qrc
)

set(MAIN_SOURCES
    src/main/main.cpp
)

set(PROJECT_SOURCES
    ${MAIN_SOURCES}
    ${STORAGE_SOURCES}
    ${VIEW_SOURCES}
    ${CONTROLLER_SOURCES}
    ${UTILS_SOURCES}
    ${RESOURCES}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_executable(TaskPlanner
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(TaskPlanner SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(TaskPlanner
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(TaskPlanner PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)

if(QT_VERSION VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.TaskPlanner)
endif()

set_target_properties(TaskPlanner PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS TaskPlanner
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TaskPlanner)
endif()

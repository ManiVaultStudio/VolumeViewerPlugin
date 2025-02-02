cmake_minimum_required(VERSION 3.17.0)

set(PROJECT "VolumeViewerPlugin")

PROJECT(${PROJECT})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /EHsc /MP /permissive- /Zc:__cplusplus")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG /NODEFAULTLIB:LIBCMT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------
find_package(OpenGL REQUIRED)
find_package(Qt6 COMPONENTS Core Widgets WebEngineWidgets Xml OpenGL OpenGLWidgets REQUIRED)
find_package(OpenMP)

find_package(ManiVault COMPONENTS Core PointData ClusterData ImageData CONFIG REQUIRED)

set(PLUGIN
    src/Common.h
    src/VolumeViewerPlugin.h
    src/VolumeViewerPlugin.cpp
)

set(RENDERERS
    src/Renderer/OpenGL/VolumeRenderer.h
    src/Renderer/OpenGL/VolumeRenderer.cpp
    src/Renderer/OpenGL/OpenGLRendererWidget.h
    src/Renderer/OpenGL/OpenGLRendererWidget.cpp
)

set(ACTIONS
    src/ColoringAction.h
    src/ColoringAction.cpp
    src/DimensionAction.h
    src/DimensionAction.cpp
    src/PositionAction.h
    src/PositionAction.cpp
    src/RendererSettingsAction.h
    src/RendererSettingsAction.cpp
    src/SelectedPointsAction.h
    src/SelectedPointsAction.cpp
    src/SettingsAction.h
    src/SettingsAction.cpp
    src/SlicingAction.h
    src/SlicingAction.cpp
    src/ThresholdAction.h
    src/ThresholdAction.cpp
    src/Actions/PickRendererAction.h
    src/Actions/PickRendererAction.cpp
)

set(WIDGETS
    src/Widgets/VolumeViewerWidget.h
    src/Widgets/VolumeViewerWidget.cpp
)

set(QRESOURCES
    res/ViewerResources.qrc
)

QT6_ADD_RESOURCES(RESOURCE_FILES ${QRESOURCES})

file(GLOB JSON src/*.json)

set(SOURCES ${PLUGIN} ${RENDERERS} ${WIDGETS} ${ACTIONS})
set(AUX ${JSON})

source_group(Plugin FILES ${PLUGIN})
source_group(Renderers FILES ${RENDERERS})
source_group(Widgets FILES ${WIDGETS})
source_group(Actions FILES ${ACTIONS})
source_group(Resources FILES ${QRESOURCES})

add_library(${PROJECT} SHARED ${SOURCES} ${AUX} ${QRESOURCES} ${RESOURCE_FILES})

target_include_directories(${PROJECT} PRIVATE "${ManiVault_INCLUDE_DIR}")
target_include_directories(${PROJECT} PRIVATE src)

target_compile_features(${PROJECT} PRIVATE cxx_std_17)

target_link_libraries(${PROJECT} PRIVATE Qt6::Widgets)
target_link_libraries(${PROJECT} PRIVATE Qt6::WebEngineWidgets)
target_link_libraries(${PROJECT} PRIVATE Qt6::OpenGL)
target_link_libraries(${PROJECT} PRIVATE Qt6::OpenGLWidgets)
target_link_libraries(${PROJECT} PRIVATE ${OPENGL_LIBRARIES})

set(MV_LINK_PATH "${ManiVault_INSTALL_DIR}/$<CONFIGURATION>/lib")
set(PLUGIN_LINK_PATH "${ManiVault_INSTALL_DIR}/$<CONFIGURATION>/$<IF:$<CXX_COMPILER_ID:MSVC>,lib,Plugins>")
set(MV_LINK_SUFFIX $<IF:$<CXX_COMPILER_ID:MSVC>,${CMAKE_LINK_LIBRARY_SUFFIX},${CMAKE_SHARED_LIBRARY_SUFFIX}>)

# Link to ManiVault and data plugins
target_link_libraries(${PROJECT} PRIVATE ManiVault::Core)
target_link_libraries(${PROJECT} PRIVATE ManiVault::PointData)
target_link_libraries(${PROJECT} PRIVATE ManiVault::ClusterData)
target_link_libraries(${PROJECT} PRIVATE ManiVault::ImageData)

install(TARGETS ${PROJECT}
    RUNTIME DESTINATION Plugins COMPONENT PLUGINS # Windows .dll
    LIBRARY DESTINATION Plugins COMPONENT PLUGINS # Linux/Mac .so
)

add_custom_command(TARGET ${PROJECT} POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
    --install ${CMAKE_CURRENT_BINARY_DIR}
    --config $<CONFIGURATION>
    --prefix ${ManiVault_INSTALL_DIR}/$<CONFIGURATION>
    --verbose
)

# Automatically set the debug environment (command + working directory) for MSVC
if(MSVC)
    set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${ManiVault_INSTALL_DIR}/debug,${ManiVault_INSTALL_DIR}/release>)
    set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,${ManiVault_INSTALL_DIR}/debug/ManiVault\ Studio.exe,${ManiVault_INSTALL_DIR}/release/ManiVault\ Studio.exe>)
endif()

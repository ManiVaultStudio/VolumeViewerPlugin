cmake_minimum_required(VERSION 3.17.0)

PROJECT("VolumeViewerPlugin")

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
# Set install directory
# -----------------------------------------------------------------------------
# Check if the directory to the ManiVault installation has been provided
if(NOT DEFINED PS_TECH_INSTALL_DIR)
    set(PS_TECH_INSTALL_DIR "" CACHE PATH "Directory where PSTech is installed")
    message(FATAL_ERROR "Please set PS_TECH_INSTALL_DIR to the directory where the PS tech SDK is installed")
endif()
file(TO_CMAKE_PATH ${PS_TECH_INSTALL_DIR} PS_TECH_INSTALL_DIR)

set(PST_DEPENDENCIES_BIN ${PS_TECH_INSTALL_DIR}/Redist)
file(TO_CMAKE_PATH ${PST_DEPENDENCIES_BIN} PST_DEPENDENCIES_BIN)
message(STATUS "PST dependency dir :" ${PST_DEPENDENCIES_BIN})
if(NOT EXISTS ${PST_DEPENDENCIES_BIN})
    message(FATAL_ERROR "There is an issue with the PS-Tech SDK path provided. It must contain a valid Redist directory")
endif()


if(NOT DEFINED SDL_BUILD_DIR)
    set(SDL_BUILD_DIR "" CACHE PATH "Directory where SDL is installed")
    message(FATAL_ERROR "Please set SDL_BUILD_DIR to the directory where the SDL library is installed")
endif()
file(TO_CMAKE_PATH ${SDL_BUILD_DIR} SDL_BUILD_DIR)
if(NOT EXISTS ${SDL_BUILD_DIR})
    message(FATAL_ERROR "There is an issue with the SDL path provided.")
endif()



# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------
find_package(Qt6 COMPONENTS Widgets WebEngineWidgets Xml OpenGL OpenGLWidgets REQUIRED)

find_package(ManiVault COMPONENTS Core PointData CONFIG REQUIRED)
find_package(ManiVault COMPONENTS Core PointData ClusterData CONFIG)


# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------
# Define the plugin sources
set(PLUGIN
    src/Common.h
    src/VolumeViewerPlugin.h
    src/VolumeViewerPlugin.cpp
    src/Controllers/Tracker.h
    src/Controllers/Tracker.cpp
    src/Controllers/Pedal.h
    src/Controllers/Pedal.cpp
    src/Controls.h
    src/Controls.cpp
    src/ReferenceSetup.h
    src/ReferenceSetup.cpp
)

set(RENDERERS
    src/Renderer/OpenGL/VolumeRenderer.h
    src/Renderer/OpenGL/VolumeRenderer.cpp
    src/Renderer/OpenGL/OpenGLRendererWidget.h
    src/Renderer/OpenGL/OpenGLRendererWidget.cpp
)

set(ACTIONS
    src/Actions/ColoringAction.h
    src/Actions/ColoringAction.cpp
    src/Actions/DimensionAction.h
    src/Actions/DimensionAction.cpp
    src/Actions/PositionAction.h
    src/Actions/PositionAction.cpp
    src/Actions/RendererSettingsAction.h
    src/Actions/RendererSettingsAction.cpp
    src/Actions/SelectedPointsAction.h
    src/Actions/SelectedPointsAction.cpp
    src/Actions/SettingsAction.h
    src/Actions/SettingsAction.cpp
    src/Actions/SlicingAction.h
    src/Actions/SlicingAction.cpp
    src/Actions/ThresholdAction.h
    src/Actions/ThresholdAction.cpp
    src/Actions/PickRendererAction.h
    src/Actions/PickRendererAction.cpp
    src/Actions/SelectModeAction.h
    src/Actions/SelectModeAction.cpp
)

set(WIDGETS
    src/Widgets/VolumeViewerWidget.h
    src/Widgets/VolumeViewerWidget.cpp
    src/Widgets/FullScreenWidget.h
    src/Widgets/FullScreenWidget.cpp
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

# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------
# Create dynamic library for the plugin
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${AUX} ${QRESOURCES} ${RESOURCE_FILES})

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------
# Include ManiVault headers, including system data plugins
target_include_directories(${PROJECT_NAME} PRIVATE "${ManiVault_INCLUDE_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PRIVATE "${PS_TECH_INSTALL_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${SDL_BUILD_DIR}/include")

# -----------------------------------------------------------------------------
# Target properties
# -----------------------------------------------------------------------------
# Request C++17
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# -----------------------------------------------------------------------------
# Target library linking
# -----------------------------------------------------------------------------
# Link to Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::WebEngineWidgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::OpenGL)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::OpenGLWidgets)
target_link_libraries(${PROJECT_NAME} PRIVATE "${PS_TECH_INSTALL_DIR}/lib/pstsdk_cpp.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE "${SDL_BUILD_DIR}/lib/x64/SDL3.lib")

# Link to ManiVault and data plugins
target_link_libraries(${PROJECT_NAME} PRIVATE ManiVault::Core)
target_link_libraries(${PROJECT_NAME} PRIVATE ManiVault::PointData)
target_link_libraries(${PROJECT_NAME} PRIVATE ManiVault::ClusterData)

# -----------------------------------------------------------------------------
# Target installation
# -----------------------------------------------------------------------------
# Install the shared plugin libary to the "Plugins" folder in the ManiVault install directory
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION Plugins COMPONENT PLUGINS # Windows .dll
    LIBRARY DESTINATION Plugins COMPONENT PLUGINS # Linux/Mac .so
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
        --install ${CMAKE_CURRENT_BINARY_DIR}
        --config $<CONFIGURATION>
        --prefix ${ManiVault_INSTALL_DIR}/$<CONFIGURATION>
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${PST_DEPENDENCIES_BIN} ${ManiVault_INSTALL_DIR}/$<CONFIGURATION>)


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${SDL_BUILD_DIR}/lib/x64/SDL3.dll" ${ManiVault_INSTALL_DIR}/$<CONFIGURATION>)

# -----------------------------------------------------------------------------
# Miscellaneous
# -----------------------------------------------------------------------------
# Automatically set the debug environment (command + working directory) for MSVC
if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${ManiVault_INSTALL_DIR}/debug,${ManiVault_INSTALL_DIR}/release>)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,${ManiVault_INSTALL_DIR}/debug/ManiVault\ Studio.exe,${ManiVault_INSTALL_DIR}/release/ManiVault\ Studio.exe>)
endif()

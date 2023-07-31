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

# vtk Dir build needs to be in the same mode as hdps is to be run. (for release hdps release vtk is needed)
#set(VTK_DIR $ENV{VTK_DIR})
find_package(VTK CONFIG REQUIRED)

find_package(OpenGL REQUIRED)
find_package(Qt6 6.3.2 COMPONENTS Core Widgets WebEngineWidgets Xml OpenGL OpenGLWidgets REQUIRED)
find_package(OpenMP)
# Normalize the incoming install path
file(TO_CMAKE_PATH $ENV{HDPS_INSTALL_DIR} INSTALL_DIR)

set(PLUGIN
    src/Common.h
    src/VolumeViewerPlugin.h
    src/VolumeViewerPlugin.cpp
    src/VolumeRenderer.h
    src/VolumeRenderer.cpp
    src/VolumeRendererWidget.h
    src/VolumeRendererWidget.cpp
)

set(QRESOURCES
    res/ViewerResources.qrc
)

QT6_ADD_RESOURCES(RESOURCE_FILES ${QRESOURCES})

file(GLOB ACTIONS src/*Action.*)
file(GLOB WIDGETS src/*Widget.*)
file(GLOB JSON src/*.json)

set(SOURCES ${PLUGIN} ${WIDGETS} ${ACTIONS})
set(AUX ${JSON})

source_group(Plugin FILES ${PLUGIN})
source_group(Widgets FILES ${WIDGETS})
source_group(Actions FILES ${ACTIONS})
source_group(Resources FILES ${QRESOURCES})

add_library(${PROJECT} SHARED ${SOURCES} ${AUX} ${QRESOURCES} ${RESOURCE_FILES})

target_include_directories(${PROJECT} PRIVATE "${INSTALL_DIR}/$<CONFIGURATION>/include/")
target_include_directories(${PROJECT} PRIVATE src)

target_compile_features(${PROJECT} PRIVATE cxx_std_17)

target_link_libraries(${PROJECT} PRIVATE Qt6::Widgets)
target_link_libraries(${PROJECT} PRIVATE Qt6::WebEngineWidgets)
target_link_libraries(${PROJECT} PRIVATE Qt6::OpenGL)
target_link_libraries(${PROJECT} PRIVATE Qt6::OpenGLWidgets)
target_link_libraries(${PROJECT} PRIVATE ${OPENGL_LIBRARIES})
target_link_libraries(${PROJECT} PRIVATE ${VTK_LIBRARIES})
vtk_module_autoinit(TARGETS ${PROJECT} MODULES ${VTK_LIBRARIES})

set(HDPS_LINK_PATH "${INSTALL_DIR}/$<CONFIGURATION>/lib")
set(PLUGIN_LINK_PATH "${INSTALL_DIR}/$<CONFIGURATION>/$<IF:$<CXX_COMPILER_ID:MSVC>,lib,Plugins>")
set(HDPS_LINK_SUFFIX $<IF:$<CXX_COMPILER_ID:MSVC>,${CMAKE_LINK_LIBRARY_SUFFIX},${CMAKE_SHARED_LIBRARY_SUFFIX}>)

set(HDPS_LINK_LIBRARY "${HDPS_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}HDPS_Public${HDPS_LINK_SUFFIX}")
set(POINTDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}PointData${HDPS_LINK_SUFFIX}") 
set(IMAGEDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}ImageData${HDPS_LINK_SUFFIX}") 
set(CLUSTERDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}ClusterData${HDPS_LINK_SUFFIX}") 

target_link_libraries(${PROJECT} PRIVATE "${HDPS_LINK_LIBRARY}")
target_link_libraries(${PROJECT} PRIVATE "${POINTDATA_LINK_LIBRARY}")
target_link_libraries(${PROJECT} PRIVATE "${IMAGEDATA_LINK_LIBRARY}")
target_link_libraries(${PROJECT} PRIVATE "${CLUSTERDATA_LINK_LIBRARY}")

install(TARGETS ${PROJECT}
    RUNTIME DESTINATION Plugins COMPONENT PLUGINS # Windows .dll
    LIBRARY DESTINATION Plugins COMPONENT PLUGINS # Linux/Mac .so
)

add_custom_command(TARGET ${PROJECT} POST_BUILD
	COMMAND "${CMAKE_COMMAND}"
	--install ${CMAKE_CURRENT_BINARY_DIR}
	--config $<CONFIGURATION>
	--prefix ${INSTALL_DIR}/$<CONFIGURATION>
	--verbose
)

# Automatically set the debug environment (command + working directory) for MSVC
if(MSVC)
	set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${INSTALL_DIR}/debug,${INSTALL_DIR}/release>)
    set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,${INSTALL_DIR}/debug/HDPS.exe,${INSTALL_DIR}/release/HDPS.exe>)
endif()

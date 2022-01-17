cmake_minimum_required(VERSION 3.12.0)

set(PROJECT "VolumeViewerPlugin")

PROJECT(${PROJECT})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /EHsc /MP" )
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG /NODEFAULTLIB:LIBCMT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)

# vtk Dir build needs to be in the same mode as hdps is to be run. (for release hdps release vtk is needed)
#set(VTK_DIR $ENV{VTK_DIR})
find_package(VTK REQUIRED)



find_package(OpenGL REQUIRED)
find_package(Qt5 COMPONENTS Widgets WebEngineWidgets REQUIRED)
find_package(OpenMP)
# Normalize the incoming install path
file(TO_CMAKE_PATH $ENV{HDPS_INSTALL_DIR} INSTALL_DIR)

set(PLUGIN
	src/Common.h
	src/VolumeViewerPlugin.h
	src/VolumeViewerPlugin.cpp
)



file(GLOB ACTIONS src/*Action.*)
file(GLOB WIDGETS src/*Widget.*)
file(GLOB MODEL src/*Model.*)


file(GLOB JSON src/*.json)


set(SOURCES ${PLUGIN} ${MODEL} ${RENDERING} ${WIDGETS})
set(AUX ${JSON} ${QRC})

source_group(Plugin FILES ${PLUGIN})
source_group(Widgets FILES ${WIDGETS})
source_group(Actions FILES ${ACTIONS})
source_group(Model FILES ${MODEL})

include_directories("${INSTALL_DIR}/$<CONFIGURATION>/include/" src)


add_library(${PROJECT} SHARED ${SOURCES} ${UIS} ${SHADERS} ${AUX} ${ACTIONS})

target_compile_features(${PROJECT} PRIVATE cxx_std_17)

if(MSVC)
	set(LIB_SUFFIX "${CMAKE_STATIC_LIBRARY_SUFFIX}")
else()
	set(LIB_SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
endif()

target_link_libraries(${PROJECT} ${OPENGL_LIBRARIES})
target_link_libraries(${PROJECT} Qt5::Widgets)
target_link_libraries(${PROJECT} Qt5::WebEngineWidgets)
target_link_libraries(${PROJECT} ${VTK_LIBRARIES})
vtk_module_autoinit(TARGETS ${PROJECT} MODULES ${VTK_LIBRARIES})


target_link_libraries(${PROJECT} "${INSTALL_DIR}/$<CONFIGURATION>/lib/${CMAKE_SHARED_LIBRARY_PREFIX}HDPS_Public${LIB_SUFFIX}")
target_link_libraries(${PROJECT} "${INSTALL_DIR}/$<CONFIGURATION>/lib/${CMAKE_SHARED_LIBRARY_PREFIX}PointData${LIB_SUFFIX}")
target_link_libraries(${PROJECT} "${INSTALL_DIR}/$<CONFIGURATION>/lib/${CMAKE_SHARED_LIBRARY_PREFIX}ImageData${LIB_SUFFIX}")

install(TARGETS ${PROJECT}
   RUNTIME DESTINATION Plugins COMPONENT SHAREDLIB
   LIBRARY DESTINATION lib COMPONENT LINKLIB
   ARCHIVE DESTINATION lib COMPONENT LINKLIB
)

add_custom_command(TARGET ${PROJECT} POST_BUILD
	COMMAND "${CMAKE_COMMAND}"
	--install ${CMAKE_BINARY_DIR}
	--config $<CONFIGURATION>
	--prefix ${INSTALL_DIR}/$<CONFIGURATION>
	--verbose
)

# add_custom_command(TARGET ${PROJECT} POST_BUILD
#     COMMAND "${CMAKE_COMMAND}" -E copy_if_different
# 	"$<TARGET_FILE:${PROJECT}>"
# 	"$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/Plugins/$<TARGET_FILE_NAME:${PROJECT}>"
# )
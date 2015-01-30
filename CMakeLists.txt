cmake_minimum_required(VERSION 3.0)

project( FlowBase )

find_package( OpenCV REQUIRED )


FIND_PACKAGE(wxWidgets REQUIRED)
INCLUDE(${wxWidgets_USE_FILE})

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable( FlowBase 
"${PROJECT_SOURCE_DIR}/src/main.cpp" 
"${PROJECT_SOURCE_DIR}/src/reactionDiffusion.cpp" 
"${PROJECT_SOURCE_DIR}/src/postProcessing.cpp" 
"${PROJECT_SOURCE_DIR}/src/gui.cpp" 
"${PROJECT_SOURCE_DIR}/include/reactionDiffusion.h" 
"${PROJECT_SOURCE_DIR}/include/postProcessing.h" 
"${PROJECT_SOURCE_DIR}/include/gui.h" 
)

target_link_libraries( FlowBase ${OpenCV_LIBS} ${wxWidgets_LIBRARIES} )
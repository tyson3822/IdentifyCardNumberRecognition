# cmake needs this line
cmake_minimum_required(VERSION 2.8)

# Define project name
#camshift
#surf_matcher
#main
project(main)

# Find OpenCV, you may need to set OpenCV_DIR variable
# to the absolute path to the directory containing OpenCVConfig.cmake file
# via the command line or GUI
find_package(OpenCV REQUIRED)

if(CMAKE_VERSION VERSION_LESS "2.8.11")
  # Add OpenCV headers location to your include paths
  include_directories(${OpenCV_INCLUDE_DIRS})
endif()

# Declare the executable target built from your sources
add_executable(main main.cpp)

# Link your application with OpenCV libraries
target_link_libraries(main ${OpenCV_LIBS})
target_link_libraries(main tesseract)
target_link_libraries(main lept)

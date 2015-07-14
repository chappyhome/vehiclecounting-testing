cmake_minimum_required(VERSION 2.8)
project(VehicleTrackingCounting)

#find and link OpenCV
find_package(OpenCV REQUIRED)
IF(OpenCV_FOUND)
	MESSAGE(STATUS "OpenCV found")
ELSE()
	MESSAGE("***OpenCV not found")
ENDIF()

#find cvBLobs
find_package(cvBlob REQUIRED)
IF(cvBlobs_FOUND)
	MESSAGE(STATUS "cvBlobs found")
ELSE()
	MESSAGE("***cvBlobs not found")
ENDIF()

#build executable
add_executable(bsgmmtracker main.cpp)

#link libraries
target_link_libraries(bsgmmtracker ${OpenCV_LIBS} ${cvBlob_LIBS})

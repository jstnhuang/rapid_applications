cmake_minimum_required(VERSION 2.8.3)
project(object_search)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  cmake_modules
  mongo_msg_db
  mongo_msg_db_msgs
  object_search_msgs
  pcl_conversions
  pcl_ros
  rapid_db
  rapid_perception
  rapid_utils
  rapid_viz
  roscpp
  rospy
  rospy_message_converter
  sensor_msgs
  static_cloud_db_msgs
  tf
  tf_conversions
  transform_graph
)

find_package(Boost REQUIRED)
find_package(Eigen REQUIRED)
find_package(PCL REQUIRED)
include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

## System dependencies are found with CMake's conventions
# find_package(Boost REQUIRED COMPONENTS system)


## Uncomment this if the package has a setup.py. This macro ensures
## modules and global scripts declared therein get installed
## See http://ros.org/doc/api/catkin/html/user_guide/setup_dot_py.html
# catkin_python_setup()

################################################
## Declare ROS messages, services and actions ##
################################################

## To declare and build messages, services or actions from within this
## package, follow these steps:
## * Let MSG_DEP_SET be the set of packages whose message types you use in
##   your messages/services/actions (e.g. std_msgs, actionlib_msgs, ...).
## * In the file package.xml:
##   * add a build_depend tag for "message_generation"
##   * add a build_depend and a run_depend tag for each package in MSG_DEP_SET
##   * If MSG_DEP_SET isn't empty the following dependency has been pulled in
##     but can be declared for certainty nonetheless:
##     * add a run_depend tag for "message_runtime"
## * In this file (CMakeLists.txt):
##   * add "message_generation" and every package in MSG_DEP_SET to
##     find_package(catkin REQUIRED COMPONENTS ...)
##   * add "message_runtime" and every package in MSG_DEP_SET to
##     catkin_package(CATKIN_DEPENDS ...)
##   * uncomment the add_*_files sections below as needed
##     and list every .msg/.srv/.action file to be processed
##   * uncomment the generate_messages entry below
##   * add every package in MSG_DEP_SET to generate_messages(DEPENDENCIES ...)

## Generate messages in the 'msg' folder
# add_message_files(
#   FILES
#   Message1.msg
#   Message2.msg
# )

## Generate services in the 'srv' folder
# add_service_files(
#   FILES
#   Service1.srv
#   Service2.srv
# )

## Generate actions in the 'action' folder
# add_action_files(
#   FILES
#   Action1.action
#   Action2.action
# )

## Generate added messages and services with any dependencies listed here
# generate_messages(
#   DEPENDENCIES
#   std_msgs  # Or other packages containing msgs
# )

################################################
## Declare ROS dynamic reconfigure parameters ##
################################################

## To declare and build dynamic reconfigure parameters within this
## package, follow these steps:
## * In the file package.xml:
##   * add a build_depend and a run_depend tag for "dynamic_reconfigure"
## * In this file (CMakeLists.txt):
##   * add "dynamic_reconfigure" to
##     find_package(catkin REQUIRED COMPONENTS ...)
##   * uncomment the "generate_dynamic_reconfigure_options" section below
##     and list every .cfg file to be processed

## Generate dynamic reconfigure parameters in the 'cfg' folder
# generate_dynamic_reconfigure_options(
#   cfg/DynReconf1.cfg
#   cfg/DynReconf2.cfg
# )

###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## INCLUDE_DIRS: uncomment this if you package contains header files
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
catkin_package(
  INCLUDE_DIRS include
  LIBRARIES
    object_search
    object_search_capture_roi
    object_search_cloud_database
    object_search_commands
    object_search_experiment
    object_search_experiment_commands
  CATKIN_DEPENDS
    mongo_msg_db
    mongo_msg_db_msgs
    object_search_msgs
    pcl_conversions
    pcl_ros
    rapid_db
    rapid_perception
    roscpp
    rospy
    rospy_message_converter
    sensor_msgs
    static_cloud_db_msgs
    tf
    tf_conversions
  DEPENDS
    Boost
    Eigen
    PCL
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
# include_directories(include)
include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${PCL_INCLUDE_DIRS}
)

add_library(object_search
  src/object_search.cpp)
add_dependencies(object_search
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${catkin_EXPORTED_TARGETS})
target_link_libraries(object_search
  ${catkin_LIBRARIES}
  ${pcl_LIBRARIES})

add_library(object_search_capture_roi
  src/capture_roi.cpp)
add_dependencies(object_search_capture_roi
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${catkin_EXPORTED_TARGETS})
target_link_libraries(object_search_capture_roi
  ${catkin_LIBRARIES}
  ${pcl_LIBRARIES})

add_library(object_search_cloud_database
  src/cloud_database.cpp)
add_dependencies(object_search_cloud_database
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${catkin_EXPORTED_TARGETS})
target_link_libraries(object_search_cloud_database
  ${catkin_LIBRARIES}
  ${pcl_LIBRARIES})

add_library(object_search_commands
  src/commands.cpp)
add_dependencies(object_search_commands
  object_search
  object_search_cloud_database
  object_search_capture_roi
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${catkin_EXPORTED_TARGETS})
target_link_libraries(object_search_commands
  object_search
  object_search_cloud_database
  object_search_capture_roi
  ${catkin_LIBRARIES}
  ${pcl_LIBRARIES})

add_library(object_search_experiment
  src/experiment.cpp)
add_dependencies(object_search_experiment
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${catkin_EXPORTED_TARGETS})
target_link_libraries(object_search_experiment
  ${catkin_LIBRARIES}
  ${pcl_LIBRARIES})

add_library(object_search_experiment_commands
  src/experiment_commands.cpp)
add_dependencies(object_search_experiment_commands
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${catkin_EXPORTED_TARGETS})
target_link_libraries(object_search_experiment_commands
  readline
  ${catkin_LIBRARIES}
  ${pcl_LIBRARIES})

add_executable(object_search_main
  src/object_search_main.cpp)
add_dependencies(object_search_main
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${catkin_EXPORTED_TARGETS}
  object_search_capture_roi
  object_search_cloud_database
  object_search_commands)
target_link_libraries(object_search_main
  ${catkin_LIBRARIES}
  ${pcl_LIBRARIES}
  object_search_capture_roi
  object_search_cloud_database
  object_search_commands)

add_executable(object_search_experiment_cli_main
  src/experiment_cli_main.cpp)
add_dependencies(object_search_experiment_cli_main
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${catkin_EXPORTED_TARGETS}
  object_search_experiment
  object_search_experiment_commands)
target_link_libraries(object_search_experiment_cli_main
  ${catkin_LIBRARIES}
  ${pcl_LIBRARIES}
  object_search_experiment
  object_search_experiment_commands)

add_executable(object_search_experiment_main
  src/experiment_main.cpp)
add_dependencies(object_search_experiment_main
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${catkin_EXPORTED_TARGETS}
  object_search
  object_search_experiment)
target_link_libraries(object_search_experiment_main
  ${catkin_LIBRARIES}
  ${pcl_LIBRARIES}
  object_search
  object_search_experiment)

add_executable(object_search_service_node
  src/object_search_node.cpp)
add_dependencies(object_search_service_node
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${catkin_EXPORTED_TARGETS}
  object_search_capture_roi
  object_search_cloud_database
  object_search_commands)
target_link_libraries(object_search_service_node
  ${catkin_LIBRARIES}
  ${pcl_LIBRARIES}
  object_search_capture_roi
  object_search_cloud_database
  object_search_commands)

#############
## Install ##
#############

# all install targets should use catkin DESTINATION variables
# See http://ros.org/doc/api/catkin/html/adv_user_guide/variables.html

## Mark executable scripts (Python etc.) for installation
## in contrast to setup.py, you can choose the destination
# install(PROGRAMS
#   scripts/my_python_script
#   DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
# )

## Mark executables and/or libraries for installation
# install(TARGETS object_search object_search_node
#   ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
#   LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
#   RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
# )

## Mark cpp header files for installation
# install(DIRECTORY include/${PROJECT_NAME}/
#   DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
#   FILES_MATCHING PATTERN "*.h"
#   PATTERN ".svn" EXCLUDE
# )

## Mark other files for installation (e.g. launch and bag files, etc.)
# install(FILES
#   # myfile1
#   # myfile2
#   DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
# )

#############
## Testing ##
#############

## Add gtest based cpp test target and link libraries
# catkin_add_gtest(${PROJECT_NAME}-test test/test_object_search.cpp)
# if(TARGET ${PROJECT_NAME}-test)
#   target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME})
# endif()

## Add folders to be run by python nosetests
# catkin_add_nosetests(test)

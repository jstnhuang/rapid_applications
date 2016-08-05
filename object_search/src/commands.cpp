#include "object_search/commands.h"

#include <iostream>
#include <string>
#include <vector>

#include "Eigen/Core"
#include "pcl/common/time.h"
#include "pcl/filters/crop_box.h"
#include "pcl/filters/extract_indices.h"
#include "pcl/filters/filter.h"
#include "pcl/filters/voxel_grid.h"
#include "pcl/point_cloud.h"
#include "pcl/point_types.h"
#include "pcl/PointIndices.h"
#include "pcl_conversions/pcl_conversions.h"
#include "pcl_ros/transforms.h"
#include "rapid_msgs/StaticCloud.h"
#include "rapid_msgs/StaticCloudInfo.h"
#include "rapid_perception/pose_estimation.h"
#include "rapid_perception/random_heat_mapper.h"
#include "ros/ros.h"
#include "sensor_msgs/PointCloud2.h"
#include "tf/transform_datatypes.h"
#include "tf/transform_listener.h"
#include "tf_conversions/tf_eigen.h"

#include "object_search/capture_roi.h"
#include "object_search/cloud_database.h"

using pcl::FPFHSignature33;
using pcl::PointCloud;
using pcl::PointXYZRGB;
using pcl::PointXYZRGBNormal;
using sensor_msgs::PointCloud2;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using rapid_msgs::StaticCloud;
using rapid_msgs::StaticCloudInfo;
using rapid::perception::PoseEstimationMatch;

namespace object_search {
ListCommand::ListCommand(Database* db, const string& type)
    : db_(db), type_(type) {}

void ListCommand::Execute(vector<string>& args) {
  vector<StaticCloudInfo> clouds;
  db_->List(&clouds);
  if (type_ == "object") {
    cout << "Objects:" << endl;
  } else {
    cout << "Scenes:" << endl;
  }
  if (clouds.size() == 0) {
    cout << "  None." << endl;
  }
  for (size_t i = 0; i < clouds.size(); ++i) {
    cout << "  " << clouds[i].id << "\t" << clouds[i].name << endl;
  }
}

RecordObjectCommand::RecordObjectCommand(Database* db, CaptureRoi* capture)
    : db_(db), capture_(capture), tf_listener_() {}

void RecordObjectCommand::Execute(std::vector<std::string>& args) {
  // Start server and wait for input
  capture_->ShowMarker();
  cout << "Adjust the ROI marker in rviz." << endl;
  cout << "Type \"save\" or anything else to cancel: ";
  string input;
  std::getline(std::cin, input);
  capture_->HideMarker();
  if (input != "save") {
    return;
  }

  // Read cloud and saved region
  PointCloud2::ConstPtr cloud_in =
      ros::topic::waitForMessage<PointCloud2>("cloud_in", ros::Duration(10));
  capture_->set_cloud(cloud_in);
  StaticCloud static_cloud;
  capture_->Capture(&static_cloud.cloud);

  // Get transform
  static_cloud.parent_frame_id = "base_footprint";
  tf::transformTFToMsg(capture_->cloud_to_base().inverse(),
                       static_cloud.base_to_camera);
  static_cloud.roi = capture_->roi();

  // Save static cloud
  static_cloud.name = args[0];
  string id = db_->Save(static_cloud);
  std::cout << "Saved " << static_cloud.name << " with ID " << id << std::endl;
}

RecordSceneCommand::RecordSceneCommand(Database* db)
    : db_(db), tf_listener_() {}

void RecordSceneCommand::Execute(std::vector<std::string>& args) {
  // Read cloud and saved region
  PointCloud2::ConstPtr cloud_in =
      ros::topic::waitForMessage<PointCloud2>("cloud_in", ros::Duration(10));
  StaticCloud static_cloud;
  static_cloud.cloud = *cloud_in;

  // Get transform
  static_cloud.parent_frame_id = "base_footprint";
  try {
    tf::StampedTransform base_to_camera_tf;
    tf_listener_.lookupTransform(
        static_cloud.cloud.header.frame_id, "base_footprint",
        static_cloud.cloud.header.stamp, base_to_camera_tf);
    tf::transformTFToMsg(base_to_camera_tf, static_cloud.base_to_camera);
  } catch (tf::TransformException e) {
    ROS_WARN("%s", e.what());
  }

  // Save static cloud
  static_cloud.name = args[0];
  db_->Save(static_cloud);
}

DeleteCommand::DeleteCommand(Database* db) : db_(db) {}

void DeleteCommand::Execute(std::vector<std::string>& args) {
  bool success = db_->Remove(args[0]);
  if (!success) {
    cout << "Invalid ID " << args[0] << ", nothing deleted." << endl;
  }
}

UseCommand::UseCommand(Database* db,
                       rapid::perception::PoseEstimator* estimator,
                       const std::string& type)
    : db_(db), estimator_(estimator), type_(type) {}

void UseCommand::Execute(std::vector<std::string>& args) {
  StaticCloud cloud;
  bool success = db_->Get(args[0], &cloud);
  if (!success) {
    cout << "Invalid ID " << args[0] << endl;
    return;
  }

  PointCloud<PointXYZRGB>::Ptr pcl_cloud_unfiltered(
      new PointCloud<PointXYZRGB>);
  PointCloud<PointXYZRGB>::Ptr pcl_cloud(new PointCloud<PointXYZRGB>);
  pcl::fromROSMsg(cloud.cloud, *pcl_cloud_unfiltered);

  pcl::VoxelGrid<PointXYZRGB> vox;
  vox.setInputCloud(pcl_cloud_unfiltered);
  vox.setLeafSize(0.005, 0.005, 0.005);
  vox.filter(*pcl_cloud);
  ROS_INFO("Downsampled to %ld points", pcl_cloud->size());

  if (type_ == "scene") {
    // Filter NaNs
    std::vector<int> mapping;
    pcl_cloud->is_dense = false;  // Force check for NaNs
    pcl::removeNaNFromPointCloud(*pcl_cloud, *pcl_cloud, mapping);
    ROS_INFO("Filtered NaNs, there are now %ld points", pcl_cloud->size());
  }
  if (estimator_->heat_mapper()->name() == "cnn" && type_ == "object") {
    ROS_ERROR("CNN heat mapper not enabled, update the code.");
    return;
    // static_cast<rapid::perception::CnnHeatMapper*>(estimator_->heat_mapper())
    //    ->set_object_camera(pcl_cloud);
  }

  // Transform into base_footprint
  tf::Transform base_to_camera;
  tf::transformMsgToTF(cloud.base_to_camera, base_to_camera);
  PointCloud<PointXYZRGB>::Ptr pcl_cloud_base(new PointCloud<PointXYZRGB>);
  pcl_ros::transformPointCloud(*pcl_cloud, *pcl_cloud_base,
                               base_to_camera.inverse());
  pcl_cloud_base->header.frame_id = cloud.parent_frame_id;

  ROS_INFO("Loaded point cloud with %ld points", pcl_cloud_base->size());

  if (type_ == "scene") {
    vector<int> indices;
    CropScene(pcl_cloud_base, &indices);
    pcl::PointIndicesPtr indices_ptr(new pcl::PointIndices);
    indices_ptr->indices = indices;

    // Get cropped versions of scene in both frames
    pcl::ExtractIndices<pcl::PointXYZRGB> extract;
    extract.setInputCloud(pcl_cloud);
    extract.setIndices(indices_ptr);
    extract.filter(*pcl_cloud);
    if (estimator_->heat_mapper()->name() == "cnn") {
      ROS_ERROR("CNN heat mapper not enabled, update the code.");
      return;
      // static_cast<rapid::perception::CnnHeatMapper*>(estimator_->heat_mapper())
      //    ->set_scene_camera(pcl_cloud);
    }

    // Get cropped version of camera frame image
    extract.setInputCloud(pcl_cloud_base);
    extract.setIndices(indices_ptr);
    extract.filter(*pcl_cloud_base);
  }

  if (type_ == "object") {
    estimator_->set_object(pcl_cloud_base, cloud.roi);
  } else {
    estimator_->set_scene(pcl_cloud_base);
  }
}

void UseCommand::CropScene(PointCloud<PointXYZRGB>::Ptr scene,
                           vector<int>* indices) {
  double min_x, min_y, min_z, max_x, max_y, max_z;
  ros::param::param<double>("min_x", min_x, 0.2);
  ros::param::param<double>("min_y", min_y, -1);
  ros::param::param<double>("min_z", min_z, 0.3);
  ros::param::param<double>("max_x", max_x, 1.2);
  ros::param::param<double>("max_y", max_y, 1);
  ros::param::param<double>("max_z", max_z, 1.7);
  ROS_INFO(
      "Cropping:\n"
      "  min_x: %f\n"
      "  min_y: %f\n"
      "  min_z: %f\n"
      "  max_x: %f\n"
      "  max_y: %f\n"
      "  max_z: %f\n",
      min_x, min_y, min_z, max_x, max_y, max_z);

  pcl::CropBox<PointXYZRGB> crop;
  crop.setInputCloud(scene);
  Eigen::Vector4f min;
  min << min_x, min_y, min_z, 1;
  Eigen::Vector4f max;
  max << max_x, max_y, max_z, 1;
  crop.setMin(min);
  crop.setMax(max);
  crop.filter(*indices);
  // crop.filter(*scene);
  ROS_INFO("Cropped to %ld points", indices->size());
}

RunCommand::RunCommand(rapid::perception::PoseEstimator* estimator)
    : estimator_(estimator) {}

void RunCommand::Execute(std::vector<std::string>& args) {
  UpdateParams();
  pcl::ScopeTime timer("Running algorithm");
  vector<PoseEstimationMatch> matches;
  estimator_->Find(&matches);
}

void RunCommand::UpdateParams() {
  double sample_ratio;
  int max_samples;
  double max_sample_radius;
  int max_neighbors;
  double feature_threshold;
  int num_candidates;
  double fitness_threshold;
  double sigma_threshold;
  double nms_radius;
  int min_results;
  ros::param::param<double>("sample_ratio", sample_ratio, 0.01);
  ros::param::param<int>("max_samples", max_samples, 1000);
  ros::param::param<double>("max_sample_radius", max_sample_radius, 0.1);
  ros::param::param<int>("max_neighbors", max_neighbors, 400);
  ros::param::param<double>("feature_threshold", feature_threshold, 1500);
  ros::param::param<int>("num_candidates", num_candidates, 100);
  ros::param::param<double>("fitness_threshold", fitness_threshold, 0.0035);
  ros::param::param<double>("sigma_threshold", sigma_threshold, 2);
  ros::param::param<double>("nms_radius", nms_radius, 0.03);
  ros::param::param<int>("min_results", min_results, 0);
  ROS_INFO(
      "Parameters:\n"
      "sample_ratio: %f\n"
      "max_samples: %d\n"
      "max_sample_radius: %f\n"
      "max_neighbors: %d\n"
      "feature_threshold: %f\n"
      "num_candidates: %d\n"
      "fitness_threshold: %f\n"
      "sigma_threshold: %f\n"
      "nms_radius: %f\n"
      "min_results: %d\n",
      sample_ratio, max_samples, max_sample_radius, max_neighbors,
      feature_threshold, num_candidates, fitness_threshold, sigma_threshold,
      nms_radius, min_results);

  if (estimator_->heat_mapper()->name() == "cnn") {
    ROS_ERROR("CNN heat mapper not enabled, update the code.");
    return;
    // rapid::perception::CnnHeatMapper* mapper =
    //    static_cast<rapid::perception::CnnHeatMapper*>(
    //        estimator_->heat_mapper());
    // mapper->set_sample_ratio(sample_ratio);
    // mapper->set_max_samples(max_samples);
    // mapper->set_max_sample_radius(max_sample_radius);
    // mapper->set_max_neighbors(max_neighbors);
    // mapper->set_cnn_layer(cnn_layer);
  } else if (estimator_->heat_mapper()->name() == "fpfh") {
    ROS_ERROR("FPFH heat mapper not enabled, update the code.");
    return;
    // rapid::perception::FpfhHeatMapper* mapper =
    //    static_cast<rapid::perception::FpfhHeatMapper*>(
    //        estimator_->heat_mapper());
    // mapper->set_sample_ratio(sample_ratio);
    // mapper->set_max_samples(max_samples);
    // mapper->set_max_sample_radius(max_sample_radius);
    // mapper->set_max_neighbors(max_neighbors);
    // mapper->set_feature_threshold(feature_threshold);
  } else if (estimator_->heat_mapper()->name() == "template_matching") {
    ROS_ERROR("Template matching heat mapper not enabled, update the code.");
    return;
    // rapid::perception::TemplateMatchingHeatMapper* mapper =
    //    static_cast<rapid::perception::TemplateMatchingHeatMapper*>(
    //        estimator_->heat_mapper());
    // mapper->set_sample_ratio(sample_ratio);
    // mapper->set_max_samples(max_samples);
  } else if (estimator_->heat_mapper()->name() == "random") {
    rapid::perception::RandomHeatMapper* mapper =
        static_cast<rapid::perception::RandomHeatMapper*>(
            estimator_->heat_mapper());
    mapper->set_sample_ratio(sample_ratio);
    mapper->set_max_samples(max_samples);
  }
  estimator_->set_num_candidates(num_candidates);
  estimator_->set_fitness_threshold(fitness_threshold);
  estimator_->set_sigma_threshold(sigma_threshold);
  estimator_->set_nms_radius(nms_radius);
  estimator_->set_min_results(min_results);
}

SetDebugCommand::SetDebugCommand(rapid::perception::PoseEstimator* estimator)
    : estimator_(estimator) {}

void SetDebugCommand::Execute(std::vector<std::string>& args) {
  if (args[0] == "on") {
    estimator_->set_debug(true);
  } else {
    estimator_->set_debug(false);
  }
}
}  // namespace object_search

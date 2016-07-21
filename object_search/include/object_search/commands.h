#ifndef _OBJECT_SEARCH_COMMANDS_H_
#define _OBJECT_SEARCH_COMMANDS_H_

#include <string>
#include <vector>

#include "pcl/point_cloud.h"
#include "pcl/point_types.h"
#include "rapid_msgs/StaticCloud.h"
#include "tf/transform_listener.h"

namespace rapid {
namespace perception {
class PoseEstimator;
}
}

namespace object_search {
class CaptureRoi;
class Database;  // Forward declaration

class Command {
 public:
  virtual ~Command() {}
  virtual void Execute(std::vector<std::string>& args) = 0;
};

class ListCommand : public Command {
 public:
  ListCommand(Database* db, const std::string& type);
  void Execute(std::vector<std::string>& args);

 private:
  Database* db_;
  std::string type_;
};

class RecordObjectCommand : public Command {
 public:
  RecordObjectCommand(Database* db, CaptureRoi* capture);
  void Execute(std::vector<std::string>& args);

 private:
  Database* db_;
  CaptureRoi* capture_;
  tf::TransformListener tf_listener_;
};

class RecordSceneCommand : public Command {
 public:
  RecordSceneCommand(Database* db);
  void Execute(std::vector<std::string>& args);

 private:
  Database* db_;
  tf::TransformListener tf_listener_;
};

class DeleteCommand : public Command {
 public:
  DeleteCommand(Database* db);
  void Execute(std::vector<std::string>& args);

 private:
  Database* db_;
};

class UseCommand : public Command {
 public:
  UseCommand(Database* db, rapid::perception::PoseEstimator* estimator,
             const std::string& type, const ros::Publisher& pub);
  void Execute(std::vector<std::string>& args);

 private:
  void CropScene(pcl::PointCloud<pcl::PointXYZRGBNormal>::Ptr scene);
  void ComputeNormals(pcl::PointCloud<pcl::PointXYZRGBNormal>::Ptr cloud);
  void ComputeFeatures(pcl::PointCloud<pcl::PointXYZRGBNormal>::Ptr in,
                       pcl::PointCloud<pcl::FPFHSignature33>::Ptr out);

  Database* db_;
  rapid::perception::PoseEstimator* estimator_;
  std::string type_;
  ros::Publisher pub_;
};

class RunCommand : public Command {
 public:
  RunCommand(rapid::perception::PoseEstimator* estimator,
             const ros::Publisher& heatmap_pub,
             const ros::Publisher& candidates_pub,
             const ros::Publisher& alignment_pub,
             const ros::Publisher& output_pub);
  void Execute(std::vector<std::string>& args);

 private:
  void UpdateParams();
  rapid::perception::PoseEstimator* estimator_;
};

class SetDebugCommand : public Command {
 public:
  SetDebugCommand(rapid::perception::PoseEstimator* estimator);
  void Execute(std::vector<std::string>& args);

 private:
  rapid::perception::PoseEstimator* estimator_;
};
}  // namespace object_search

#endif  // _OBJECT_SEARCH_COMMANDS_H_

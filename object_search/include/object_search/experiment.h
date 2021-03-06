#ifndef _OBJECT_SEARCH_EXPERIMENT_H_
#define _OBJECT_SEARCH_EXPERIMENT_H_

#include "rapid_db/name_db.hpp"

namespace object_search {
struct ExperimentDbs {
  rapid::db::NameDb* task_db;
  rapid::db::NameDb* scene_db;
  rapid::db::NameDb* scene_cloud_db;
  rapid::db::NameDb* landmark_db;
  rapid::db::NameDb* landmark_cloud_db;
};

class ConfusionMatrix {
 public:
  ConfusionMatrix();

  int tp;
  int tn;
  int fp;
  int fn;

  double Precision() const;
  double Recall() const;
  double F1() const;

  void Merge(const ConfusionMatrix& other);
};
}  // namespace object_search

#endif  // _OBJECT_SEARCH_EXPERIMENT_H_

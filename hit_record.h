#pragma once

#include "vec3.h"
#include "material.h"

struct HitRecord {
  Vec3 point;
  Vec3 normal;
  double t;
  bool outside_face;
  Material material;
};

ScatterInfo as_scatter_info(const HitRecord& hit_record) {
    return {hit_record.point, hit_record.normal};
}
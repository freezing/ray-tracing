#pragma once

#include <vector>
#include <optional>
#include <tuple>
#include <variant>

#include "material.h"
#include "sphere.h"

using Object = std::variant<Sphere>;

class World {
public:
  World() {}
  
  void clear() {
    objects_.clear();
  }

  void add(Object&& object, Material material) {
    objects_.push_back({std::move(object), std::move(material)});
  }

  const std::vector<std::tuple<Object, Material>>& objects() const {
      return objects_;
  }

private:
  std::vector<std::tuple<Object, Material>> objects_;

};

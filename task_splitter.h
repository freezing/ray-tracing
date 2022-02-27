#pragma once

#include <vector>
#include <math.h>

using CoreId = int;

struct RenderTask {
    int start_x;
    int end_x;
    int start_y;
    int end_y;
};

std::string to_debug(const RenderTask& task) {
    std::stringstream ss;
    ss 
        << "(start_x = " 
        << task.start_x 
        << ", end_x = " 
        << task.end_x 
        << ", start_y = " 
        << task.start_y 
        << ", end_y = " 
        << task.end_y 
        << ")";
    return ss.str();
}

struct RenderTasksPerCore {
    CoreId core_id;
    std::vector<RenderTask> tasks;
};

int next_core_id(int core_id, int num_cores) {
    assert(core_id < num_cores);
    if (core_id == num_cores - 1) {
        return 0;
    }
    return core_id + 1;
}

std::vector<RenderTasksPerCore> split_tasks(int image_height, int image_width, int num_cores) {
    int k = num_cores;
    int grid_height = image_height / k + 1;
    int grid_width = image_width / k + 1;

    std::vector<RenderTasksPerCore> tasks;
    tasks.resize(num_cores);

    for (CoreId core_id = 0; core_id < num_cores; core_id++) {
        RenderTasksPerCore& per_core = tasks[core_id];
        per_core.core_id = core_id;
    }

    CoreId core_id = 0;
    for (int offset_x = 0; offset_x < image_width; offset_x += grid_width) {
        for (int offset_y = 0; offset_y < image_height; offset_y += grid_height) {
            int end_x = std::min(offset_x + grid_width, image_width) - 1;
            int end_y = std::min(offset_y + grid_height, image_height) - 1;
            RenderTask task = {
                .start_x = offset_x,
                .end_x = end_x,
                .start_y = offset_y,
                .end_y = end_y,
            };
            tasks[core_id].tasks.emplace_back(std::move(task));
            core_id = next_core_id(core_id, num_cores);
        }
    }
    return tasks;
}
#pragma once

#include <string>
#include <sstream>

#include "camera.h"
#include "engine.h"
#include "task_splitter.h"
#include "world.h"

struct PixelLocation {
    int x;
    int y;

    bool operator <(const PixelLocation& other) const {
        if (x < other.x) {
            return true;
        } else if (x > other.x) {
            return false;
        } else {
            return y < other.y;
        }
    }
};

std::string to_debug(const PixelLocation& location) {
    std::stringstream ss;
    ss << "(x = " << location.x << ", y = " << location.y << ")";
    return ss.str();
}

struct RenderTaskResult {
    RenderTask task;
    std::vector<Vec3> pixels;

    PixelLocation location_of(int index) {
        int width = task.end_x - task.start_x + 1;
        int offset_y = index / width;
        int offset_x = index % width;
        return {task.start_x + offset_x, task.start_y + offset_y};
    }
};

RenderTaskResult render_task(CoreId core_id, RenderTask task, const Renderer& renderer, bool log_progress = false) {
    std::cerr << "Render task: " << to_debug(task) << std::endl;
    RenderTaskResult result{.task = task};
    for (int y = task.start_y; y <= task.end_y; y++) {
        if (log_progress) {
            int remaining_lines = task.end_y - y + 1;
            std::cerr << "\rRemaining lines for core " << core_id << ": " << remaining_lines << std::endl << std::flush;
        }
        for (int x = task.start_x; x <= task.end_x; x++) {
            int row = y;
            int col = x;
            result.pixels.emplace_back(renderer.color_at(row, col));
        }
    }
    return result;
}
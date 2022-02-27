#pragma once

#include "renderer.h"
#include "task_splitter.h"
#include "task_renderer.h"

struct RenderedImage {
    std::map<PixelLocation, Vec3> pixels;

    void add(const RenderedImage& image) {
        for (const auto& entry : image.pixels) {
        pixels.insert(entry);
        }
    }
};

class ParallelRenderer {
public:
    const Renderer& renderer;

    RenderedImage render(int num_cores) const {
        auto tasks = split_tasks(renderer.image_height(), renderer.image_width(), num_cores);

        for (const auto& tasks_per_core : tasks) {
            std::cerr 
                << "  Num tasks[" << tasks_per_core.core_id << "]: "
                << tasks_per_core.tasks.size()
                << std::endl
                << std::flush;
        }

        std::vector<RenderedImage> rendered_images;
        rendered_images.resize(num_cores);

        std::vector<std::thread> threads;
        for (const auto& tasks_per_core : tasks) {
            std::thread thread([this, &tasks_per_core, &rendered_images]() mutable {
            auto& rendered_image = rendered_images[tasks_per_core.core_id];
            for (const auto& task : tasks_per_core.tasks) {
                auto result = render_task(tasks_per_core.core_id, task, renderer, tasks_per_core.core_id == 0);
                // Populate result in the rendered_image map.
                for (std::size_t i = 0; i < result.pixels.size(); i++) {
                auto location = result.location_of(i);
                rendered_image.pixels[location] = result.pixels[i];
                }
            }
            });
            threads.push_back(std::move(thread));
        }

        // Merge image parts into one image.
        for (auto& thread : threads) {
            thread.join();
        }

        RenderedImage rendered_image = std::move(rendered_images[0]);
        for (std::size_t i = 1; i < rendered_images.size(); i++) {
            rendered_image.add(rendered_images[i]);
        }
        return rendered_image;
    }
};
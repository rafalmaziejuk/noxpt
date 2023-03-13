#pragma once

#include "kernel_types.h"

#include <nox/compute/compute_object.h>

#include <nox/graphics/image.h>
#include <nox/graphics/light.h>
#include <nox/graphics/model.h>

#include <nox/renderer/texture.h>

namespace NOXPT {

    class Scene {
      public:
        const std::vector<KernelTypes::Triangle> &getTriangles() const { return m_triangles; }
        std::vector<KernelTypes::Triangle> &getTriangles() { return m_triangles; }
        const std::vector<KernelTypes::Light> &getLights() const { return m_lights; }
        const std::vector<KernelTypes::Material> &getMaterials() const { return m_materials; }

        void addRectangleLight(const NOX::RectangleLight &light);
        void addModel(const std::shared_ptr<NOX::Model> &model);

      private:
        std::vector<KernelTypes::Triangle> m_triangles{};
        std::vector<KernelTypes::Light> m_lights{};
        std::vector<KernelTypes::Material> m_materials{};
    };

} // namespace NOXPT

#pragma once

#include "bvh.h"
#include "scene.h"

#include <nox/compute/compute_buffer.h>
#include <nox/compute/compute_image.h>
#include <nox/compute/compute_program.h>

#include <nox/graphics/camera.h>

#include <nox/renderer/texture.h>

namespace NOXPT {

    class PathTracer {
      public:
        PathTracer(const NOX::Camera &camera, const Scene &scene);

        const std::shared_ptr<NOX::Texture2D> &getOutputTexture() const { return m_outputTexture; }

        void initialize();
        void reset();

        void onUpdate();

      private:
        void initializeImages();
        void initializeBuffers();
        void initializeGeneratePrimaryRayKernel();
        void initializeTracePathKernel();
        void initializeComputePixelKernel();

      private:
        void updateCameraData();
        void updateSampleCount();

      private:
        const NOX::Camera *m_camera{nullptr};
        const Scene *m_scene{nullptr};
        BVH m_bvh{};
        uint32_t m_sampleCount = 1u;

        std::shared_ptr<NOX::ComputeProgram> m_pathTracingProgram{nullptr};
        std::shared_ptr<NOX::Texture2D> m_outputTexture{nullptr};

        NOX::ComputeKernel *m_generatePrimaryRayKernel{nullptr};
        NOX::ComputeKernel *m_tracePathKernel{nullptr};
        NOX::ComputeKernel *m_computePixelKernel{nullptr};

        std::shared_ptr<NOX::ComputeImage> m_outputImage{nullptr};
        std::shared_ptr<NOX::ComputeBuffer> m_primaryRaysBuffer{nullptr};
        std::shared_ptr<NOX::ComputeBuffer> m_radianceBuffer{nullptr};
        std::shared_ptr<NOX::ComputeBuffer> m_bvhNodesBuffer{nullptr};
        std::shared_ptr<NOX::ComputeBuffer> m_trianglesBuffer{nullptr};
        std::shared_ptr<NOX::ComputeBuffer> m_lightsBuffer{nullptr};
        std::shared_ptr<NOX::ComputeBuffer> m_materialsBuffer{nullptr};
    };

} // namespace NOXPT

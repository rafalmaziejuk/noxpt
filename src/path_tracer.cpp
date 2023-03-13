#include "path_tracer.h"

#include <nox/application.h>
#include <nox/window.h>

#include <nox/assets/asset_manager.h>

#include <nox/compute/compute.h>

namespace NOXPT {

    namespace {

        constexpr size_t s_globalWorkSize1D = 1280 * 720;
        constexpr size_t s_globalWorkSize2D[2] = {1280, 720};
        constexpr uint32_t s_maxBounces = 3u;

        constexpr size_t s_radianceValueSize = sizeof(cl_float3);
        constexpr cl_float3 s_radianceFillPattern = {0.0f, 0.0f, 0.0f};

    } // namespace

    PathTracer::PathTracer(const NOX::Camera &camera, const Scene &scene) : m_camera(&camera),
                                                                            m_scene(&scene) {
        auto &application = *NOX::Application::get();
        auto &assetManager = application.getAssetManager();
        const auto &window = application.getWindow();

        m_outputTexture = assetManager.loadAssetImmediate<NOX::Texture2D>("outputTexture", window.getWidth(), window.getHeight());

        m_pathTracingProgram = assetManager.loadAssetImmediate<NOX::ComputeProgram>("pathTracingProgram", "assets/kernels/path_tracing.cl");
        m_generatePrimaryRayKernel = &m_pathTracingProgram->getKernel("generate_primary_ray");
        m_tracePathKernel = &m_pathTracingProgram->getKernel("trace_path");
        m_computePixelKernel = &m_pathTracingProgram->getKernel("compute_pixel");
    }

    void PathTracer::initialize() {
        m_bvh.build(m_scene->getTriangles());

        initializeImages();
        initializeBuffers();
        initializeGeneratePrimaryRayKernel();
        initializeTracePathKernel();
        initializeComputePixelKernel();
    }

    void PathTracer::reset() {
        m_sampleCount = 1u;
        NOX::Compute::enqueueFillBuffer(*m_radianceBuffer, &s_radianceFillPattern, s_radianceValueSize, s_globalWorkSize1D * s_radianceValueSize);
    }

    void PathTracer::initializeImages() {
        m_outputImage = NOX::Compute::createImage(NOX::MemoryUsage::READ_WRITE, *m_outputTexture);
    }

    void PathTracer::initializeBuffers() {
        constexpr size_t raySize = sizeof(cl_float3) * 2;
        m_primaryRaysBuffer = NOX::Compute::createBuffer(NOX::MemoryUsage::READ_WRITE, s_globalWorkSize1D * raySize);

        m_radianceBuffer = NOX::Compute::createBuffer(NOX::MemoryUsage::READ_WRITE, s_globalWorkSize1D * s_radianceValueSize);
        NOX::Compute::enqueueFillBuffer(*m_radianceBuffer, &s_radianceFillPattern, s_radianceValueSize, s_globalWorkSize1D * s_radianceValueSize);

        const auto &bvhNodes = m_bvh.getBvhNodes();
        m_bvhNodesBuffer = NOX::Compute::createBuffer(NOX::MemoryUsage::READ_ONLY | NOX::MemoryUsage::COPY_HOST_PTR, bvhNodes.size() * sizeof(KernelTypes::BVHNode), bvhNodes.data());

        const auto &triangles = m_bvh.getOrderedTriangles();
        m_trianglesBuffer = NOX::Compute::createBuffer(NOX::MemoryUsage::READ_ONLY | NOX::MemoryUsage::COPY_HOST_PTR, triangles.size() * sizeof(KernelTypes::Triangle), triangles.data());

        const auto &lights = m_scene->getLights();
        m_lightsBuffer = NOX::Compute::createBuffer(NOX::MemoryUsage::READ_ONLY | NOX::MemoryUsage::COPY_HOST_PTR, lights.size() * sizeof(KernelTypes::Light), lights.data());

        const auto &materials = m_scene->getMaterials();
        m_materialsBuffer = NOX::Compute::createBuffer(NOX::MemoryUsage::READ_ONLY | NOX::MemoryUsage::COPY_HOST_PTR, materials.size() * sizeof(KernelTypes::Material), materials.data());
    }

    void PathTracer::initializeGeneratePrimaryRayKernel() {
        const auto &position = m_camera->getPosition();
        const auto &forward = m_camera->getForwardVector();
        const auto &right = m_camera->getRightVector();
        const auto &up = m_camera->getUpVector();
        const auto &cameraSpecification = m_camera->getSpecification();
        const auto &fov = glm::tan(glm::radians(cameraSpecification.fov) * 0.5f);
        const auto &width = static_cast<cl_float>(m_outputTexture->getWidth());
        const auto &height = static_cast<cl_float>(m_outputTexture->getHeight());
        const auto &aspectRatio = cameraSpecification.aspectRatio;

        m_generatePrimaryRayKernel->setArg(0, &position, sizeof(cl_float3));
        m_generatePrimaryRayKernel->setArg(1, &forward, sizeof(cl_float3));
        m_generatePrimaryRayKernel->setArg(2, &right, sizeof(cl_float3));
        m_generatePrimaryRayKernel->setArg(3, &up, sizeof(cl_float3));
        m_generatePrimaryRayKernel->setArg(4, &fov, sizeof(cl_float));
        m_generatePrimaryRayKernel->setArg(5, &width, sizeof(cl_float));
        m_generatePrimaryRayKernel->setArg(6, &height, sizeof(cl_float));
        m_generatePrimaryRayKernel->setArg(7, &aspectRatio, sizeof(cl_float));
        m_generatePrimaryRayKernel->setArg(8, &m_sampleCount, sizeof(cl_uint));
        m_generatePrimaryRayKernel->setArg(9, *m_primaryRaysBuffer);
    }

    void PathTracer::initializeTracePathKernel() {
        const auto &width = m_outputTexture->getWidth();
        const auto &lightsCount = static_cast<cl_uint>(m_scene->getLights().size());

        m_tracePathKernel->setArg(0, *m_primaryRaysBuffer);
        m_tracePathKernel->setArg(1, *m_bvhNodesBuffer);
        m_tracePathKernel->setArg(2, *m_trianglesBuffer);
        m_tracePathKernel->setArg(3, *m_lightsBuffer);
        m_tracePathKernel->setArg(4, &lightsCount, sizeof(cl_uint));
        m_tracePathKernel->setArg(5, &s_maxBounces, sizeof(cl_uint));
        m_tracePathKernel->setArg(6, &m_sampleCount, sizeof(cl_uint));
        m_tracePathKernel->setArg(7, &width, sizeof(cl_uint));
        m_tracePathKernel->setArg(8, *m_materialsBuffer);
        m_tracePathKernel->setArg(9, *m_radianceBuffer);
    }

    void PathTracer::initializeComputePixelKernel() {
        m_computePixelKernel->setArg(0, *m_outputImage);
        m_computePixelKernel->setArg(1, *m_radianceBuffer);
        m_computePixelKernel->setArg(2, &m_sampleCount, sizeof(cl_uint));
    }

    void PathTracer::updateCameraData() {
        const auto &position = m_camera->getPosition();
        const auto &forward = m_camera->getForwardVector();
        const auto &right = m_camera->getRightVector();
        const auto &up = m_camera->getUpVector();

        m_generatePrimaryRayKernel->setArg(0, &position, sizeof(cl_float3));
        m_generatePrimaryRayKernel->setArg(1, &forward, sizeof(cl_float3));
        m_generatePrimaryRayKernel->setArg(2, &right, sizeof(cl_float3));
        m_generatePrimaryRayKernel->setArg(3, &up, sizeof(cl_float3));
    }

    void PathTracer::updateSampleCount() {
        m_sampleCount++;

        m_generatePrimaryRayKernel->setArg(8, &m_sampleCount, sizeof(cl_uint));
        m_tracePathKernel->setArg(6, &m_sampleCount, sizeof(cl_uint));
        m_computePixelKernel->setArg(2, &m_sampleCount, sizeof(cl_uint));
    }

    void PathTracer::onUpdate() {
        updateCameraData();
        updateSampleCount();

        NOX::Compute::enqueueNDRangeKernel(*m_generatePrimaryRayKernel, 2, s_globalWorkSize2D);
        NOX::Compute::enqueueNDRangeKernel(*m_tracePathKernel, 2, s_globalWorkSize2D);

        NOX::Compute::enqueueAcquireGLObject(*m_outputImage);
        NOX::Compute::enqueueNDRangeKernel(*m_computePixelKernel, 2, s_globalWorkSize2D);
        NOX::Compute::enqueueReleaseGLObject(*m_outputImage);
    }

} // namespace NOXPT

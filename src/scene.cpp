#include "scene.h"

#include <nox/graphics/image.h>

#include <nox/renderer/texture.h>

#include <glm/gtc/type_ptr.hpp>

namespace NOXPT {

    void Scene::addModel(const std::shared_ptr<NOX::Model> &model) {
        for (const auto &mesh : model->getMeshes()) {
            m_triangles.reserve(m_triangles.size() + mesh.vertices.size());
            for (auto i = 0; i < mesh.vertices.size(); i += 3) {
                KernelTypes::Triangle triangle;

                std::memcpy(triangle.v0.position.s, glm::value_ptr(glm::vec4(mesh.vertices[i + 0].position, 0.0f)), sizeof(cl_float4));
                std::memcpy(triangle.v0.normal.s, glm::value_ptr(glm::vec4(mesh.vertices[i + 0].normal, 0.0f)), sizeof(cl_float4));

                std::memcpy(triangle.v1.position.s, glm::value_ptr(glm::vec4(mesh.vertices[i + 1].position, 0.0f)), sizeof(cl_float4));
                std::memcpy(triangle.v1.normal.s, glm::value_ptr(glm::vec4(mesh.vertices[i + 1].normal, 0.0f)), sizeof(cl_float4));

                std::memcpy(triangle.v2.position.s, glm::value_ptr(glm::vec4(mesh.vertices[i + 2].position, 0.0f)), sizeof(cl_float4));
                std::memcpy(triangle.v2.normal.s, glm::value_ptr(glm::vec4(mesh.vertices[i + 2].normal, 0.0f)), sizeof(cl_float4));

                triangle.materialIndex = mesh.materialIndex;

                m_triangles.push_back(triangle);
            }
        }

        m_materials.reserve(model->getMaterials().size());
        for (const auto &material : model->getMaterials()) {
            KernelTypes::Material newMaterial;
            newMaterial.diffuse = {material.diffuse.x, material.diffuse.y, material.diffuse.z};
            newMaterial.emissive = {material.emissive.x, material.emissive.y, material.emissive.z};
            m_materials.push_back(newMaterial);
        }
    }

    void Scene::addRectangleLight(const NOX::RectangleLight &light) {
        KernelTypes::Light newLight;
        std::memcpy(newLight.position.s, glm::value_ptr(glm::vec4(light.getPosition(), 0.0f)), sizeof(cl_float4));
        std::memcpy(newLight.emission.s, glm::value_ptr(glm::vec4(light.getEmission(), 0.0f)), sizeof(cl_float4));
        std::memcpy(newLight.u.s, glm::value_ptr(glm::vec4(light.getU(), 0.0f)), sizeof(cl_float4));
        std::memcpy(newLight.v.s, glm::value_ptr(glm::vec4(light.getV(), 0.0f)), sizeof(cl_float4));
        newLight.area = light.getArea();

        m_lights.push_back(newLight);
    }

} // namespace NOXPT

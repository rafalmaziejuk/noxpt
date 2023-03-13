#pragma once

#include "path_tracer.h"
#include "scene.h"

#include <nox/application.h>

#include <nox/graphics/camera_controller.h>

namespace NOXPT {

    class Application : public NOX::Application {
      public:
        explicit Application(const NOX::ApplicationSpecification &specification);
        ~Application() override;

        [[nodiscard]] static Application *createApplication(NOX::ApplicationCommandLineArguments arguments);

        void onUpdate(float timestep) override;

      private:
        NOX::CameraController m_cameraController{};
        float m_cameraMovementSpeed{2.5f};
        float m_cameraSensitivity{0.05f};

        Scene m_scene{};
        PathTracer m_pathTracer;
    };

} // namespace NOXPT

#include "application.h"

#include <nox/common_types.h>
#include <nox/input.h>
#include <nox/window.h>

#include <nox/renderer/renderer.h>

namespace NOXPT {

    Application::Application(const NOX::ApplicationSpecification &specification) : NOX::Application(specification),
                                                                                   m_pathTracer(m_cameraController.getCamera(), m_scene) {
        m_eventDispatcher.getKeyEventDelegate().subscribe([this](const NOX::KeyEvent &event) {
            if (event.getAction() == NOX::Action::PRESSED) {
                switch (event.getKey()) {
                case NOX::Key::ESCAPE:
                    m_cameraController.focus();
                    break;
                }
            }
        });

        m_scene.addModel(m_assetManager.loadAssetImmediate<NOX::Model>("cornellBox", "assets/models/cornell_box/cornell_box.obj"));
        m_scene.addRectangleLight(NOX::RectangleLight({0.0f, 1.985f, 0.0f}, 0.5f, 0.5f, {17.0f, 12.0f, 4.0f}));

        m_pathTracer.initialize();
    }

    Application::~Application() {}

    Application *Application::createApplication(NOX::ApplicationCommandLineArguments arguments) {
        NOX::ApplicationSpecification specification;
        specification.arguments = arguments;
        specification.name = "NOXPT";
        specification.workingDirectoryPath = arguments[0];

        return new Application(specification);
    }

    void Application::onUpdate(float timestep) {
        m_cameraController.rotate(m_cameraSensitivity);
        if (NOX::Input::isKeyPressed(NOX::Key::W)) {
            m_cameraController.move(NOX::CameraController::Direction::FORWARDS, m_cameraMovementSpeed * timestep);
        }
        if (NOX::Input::isKeyPressed(NOX::Key::A)) {
            m_cameraController.move(NOX::CameraController::Direction::LEFT, m_cameraMovementSpeed * timestep);
        }
        if (NOX::Input::isKeyPressed(NOX::Key::S)) {
            m_cameraController.move(NOX::CameraController::Direction::BACKWARDS, m_cameraMovementSpeed * timestep);
        }
        if (NOX::Input::isKeyPressed(NOX::Key::D)) {
            m_cameraController.move(NOX::CameraController::Direction::RIGHT, m_cameraMovementSpeed * timestep);
        }
        if (NOX::Input::isKeyPressed(NOX::Key::SPACE)) {
            m_cameraController.move(NOX::CameraController::Direction::UP, m_cameraMovementSpeed * timestep);
        }
        if (NOX::Input::isKeyPressed(NOX::Key::LCTRL)) {
            m_cameraController.move(NOX::CameraController::Direction::DOWN, m_cameraMovementSpeed * timestep);
        }

        if (m_cameraController.isFocused()) {
            m_pathTracer.reset();
        }

        m_pathTracer.onUpdate();

        NOX::Renderer::clear();
        NOX::Renderer::drawFullscreenTexture(*m_pathTracer.getOutputTexture());
    }

} // namespace NOXPT

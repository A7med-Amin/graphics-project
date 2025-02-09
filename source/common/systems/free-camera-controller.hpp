#pragma once

#include "../ecs/world.hpp"
#include "../components/camera.hpp"
#include "../components/free-camera-controller.hpp"

#include "../application.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

namespace our
{

    // TODO: (Phase 2) free camera controller
    enum JumpState
    {
        JUMPING,
        FALLING,
        GROUNDED
    };
    // The free camera controller system is responsible for moving every entity which contains a FreeCameraControllerComponent.
    // This system is added as a slightly complex example for how use the ECS framework to implement logic.
    // For more information, see "common/components/free-camera-controller.hpp"
    class FreeCameraControllerSystem
    {
        Application *app;                                    // The application in which the state runs
        bool mouse_locked = true;                            // Is the mouse locked
                                                             // The state of jumping and falling
        our::JumpState jumpState = our::JumpState::GROUNDED; // The state of jumping
    public:
        // When a state enters, it should call this function and give it the pointer to the application
        void enter(Application *app)
        {
            this->app = app;
        }

        void objectCollide()
        {
        }

        // This should be called every frame to update all entities containing a FreeCameraControllerComponent
        void update(World *world, float deltaTime, bool didCollide)
        {
            // First of all, we search for an entity containing both a CameraComponent and a FreeCameraControllerComponent
            // As soon as we find one, we break
            CameraComponent *camera = nullptr;
            FreeCameraControllerComponent *controller = nullptr;
            int collisionFactor = 1;

            float jumpSpeed = 8;
            float jumpMaxHeight = 5;
            for (auto entity : world->getEntities())
            {
                camera = entity->getComponent<CameraComponent>();
                controller = entity->getComponent<FreeCameraControllerComponent>();
                if (camera && controller)
                    break;
            }
            // If there is no entity with both a CameraComponent and a FreeCameraControllerComponent, we can do nothing so we return
            if (!(camera && controller))
                return;
            // Get the entity that we found via getOwner of camera (we could use controller->getOwner())
            Entity *entity = camera->getOwner();

            // If the left mouse button is pressed, we lock and hide the mouse. This common in First Person Games.
            // if (app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1) && !mouse_locked)
            // {
            //     app->getMouse().lockMouse(app->getWindow());
            //     mouse_locked = true;
            //     // If the left mouse button is released, we unlock and unhide the mouse.
            // }
            // else if (!app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1) && mouse_locked)
            // {
            //     app->getMouse().unlockMouse(app->getWindow());
            //     mouse_locked = false;
            // }

            // We get a reference to the entity's position and rotation
            glm::vec3 &position = entity->localTransform.position;
            glm::vec3 &rotation = entity->localTransform.rotation;

            // We update the camera fov based on the mouse wheel scrolling amount
            // float fov = camera->fovY + app->getMouse().getScrollOffset().y * controller->fovSensitivity;
            // fov = glm::clamp(fov, glm::pi<float>() * 0.01f, glm::pi<float>() * 0.99f); // We keep the fov in the range 0.01*PI to 0.99*PI
            // camera->fovY = fov;

            // We get the camera model matrix (relative to its parent) to compute the front, up and right directions
            glm::mat4 matrix = entity->localTransform.toMat4();

            glm::vec3 front = glm::vec3(matrix * glm::vec4(0, 0, -1, 0)),
                      up = glm::vec3(matrix * glm::vec4(0, 1, 0, 0)),
                      right = glm::vec3(matrix * glm::vec4(1, 0, 0, 0));

            glm::vec3 current_sensitivity = controller->positionSensitivity;

            // if (app->getKeyboard().isPressed(GLFW_KEY_LEFT_SHIFT))
            //     current_sensitivity *= controller->speedupFactor;

            // We change the camera position based on the keys WASD/QE
            // S & W moves the player back and forth
            if (app->getKeyboard().isPressed(GLFW_KEY_W) || app->getKeyboard().isPressed(GLFW_KEY_UP))
                position += front * (deltaTime * current_sensitivity.z * collisionFactor);
            // if (app->getKeyboard().isPressed(GLFW_KEY_S) || app->getKeyboard().isPressed(GLFW_KEY_DOWN))
            //     position -= front * (deltaTime * current_sensitivity.z * collisionFactor);
            // Q & E moves the player up and down
            if(app->getKeyboard().isPressed(GLFW_KEY_Q)) position += up * (deltaTime * current_sensitivity.y);
            if (app->getKeyboard().isPressed(GLFW_KEY_E))
                position -= up * (deltaTime * current_sensitivity.y);
            // A & D moves the player left or right
            if (app->getKeyboard().isPressed(GLFW_KEY_D) || app->getKeyboard().isPressed(GLFW_KEY_RIGHT))
            {
                // limit the player movement to the left and right
                if (position.x <= 9)
                    position += right * (deltaTime * current_sensitivity.x * collisionFactor);
            }
            if (app->getKeyboard().isPressed(GLFW_KEY_A) || app->getKeyboard().isPressed(GLFW_KEY_LEFT))
            {
                // limit the player movement to the left and right
                if (position.x >= -9)
                    position -= right * (deltaTime * current_sensitivity.x * collisionFactor);
            }

            // Jump logic
            if (app->getKeyboard().isPressed(GLFW_KEY_SPACE))
            {
                if (jumpState == our::JumpState::GROUNDED)
                {
                    // We set the jump state to JUMPING
                    jumpState = our::JumpState::JUMPING;
                    // Start the jump
                    position.y += (deltaTime * jumpSpeed * collisionFactor);
                }
            }
            // If the player jumps higher than the max height, we set the jump state to FALLING
            if (position.y >= jumpMaxHeight)
            {
                jumpState = our::JumpState::FALLING;
            }
            else if (position.y <= 1)
            {
                // If the player was falling, we set the jump state to GROUNDED
                jumpState = our::JumpState::GROUNDED;
            }

            // We update the player position based on the jump state
            if (jumpState == our::JumpState::JUMPING)
            {
                position.y += (deltaTime * jumpSpeed);
            }
            else if (jumpState == our::JumpState::FALLING)
            {
                // We update the player position based on the jump state
                position.y -= (deltaTime * jumpSpeed);
            }
            else
            {
                // We make sure the player is grounded
                position.y = 1;
            }
        }

        // When the state exits, it should call this function to ensure the mouse is unlocked
        void exit()
        {
            if (mouse_locked)
            {
                mouse_locked = false;
                app->getMouse().unlockMouse(app->getWindow());
            }
        }
    };

}

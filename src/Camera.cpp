
#include "Camera.hpp"

Camera::Camera()
{
    updating = true;

    camUp = glm::vec3(0.0f,1.0f,0.0f);

    pos = glm::vec3(0,1,0);
    pitch = 0.0f;
    yaw = 90.0f;

    speed = 5.0f;
    sensitivity = 0.5f;
}

Camera::Camera(float yaw_, float pitch_, glm::vec3 pos_, glm::vec3 camUp_ = glm::vec3(0.0f,1.0f,0.0f))
{
    updating = true;

    yaw = yaw_;
    pitch = pitch_;
    pos = pos_;

    speed = 5.0f;
    sensitivity = 0.5f;
}

Camera::~Camera()
{

}

void Camera::setAngle(float yaw_, float pitch_)
{
    yaw = yaw_;
    pitch = pitch_;

    setLookDir();//Change the look direction
}

void Camera::setPosition(glm::vec3& newPos)
{
    pos = newPos;

    setLookDir();//Change the look direction
}

void Camera::adaptToEvents(bool toFront, bool toBack, bool toLeft, bool toRight, glm::vec2 mousePos, bool applyChanges)
{
    glm::vec2 offset;

    offset.x = mousePos.x - oldMousePos.x;
    offset.y = oldMousePos.y - mousePos.y;

    oldMousePos = mousePos;

    if (applyChanges && updating)
    {
        yaw += offset.x * sensitivity;
        pitch += offset.y * sensitivity;

        if (pitch >= 89.0f)
            pitch = 89.0f;
        else if (pitch <= -89.0f)
            pitch = -89.0f;

        setLookDir();

        glm::vec3 lookDirXZ;
        lookDirXZ = glm::normalize(glm::cross(rightDir, camUp));

        if (toFront)
            pos += lookDir * speed * speedClock.getElapsedTime().asSeconds();
        if (toBack)
            pos -= lookDir * speed * speedClock.getElapsedTime().asSeconds();
        if (toRight)
            pos += rightDir * speed * speedClock.getElapsedTime().asSeconds();
        if (toLeft)
            pos -= rightDir * speed * speedClock.getElapsedTime().asSeconds();

    }

    speedClock.restart();
}

void Camera::setLookDir()
{
    lookDir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    lookDir.y = sin(glm::radians(pitch));
    lookDir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    lookDir = glm::normalize(lookDir);

    rightDir = glm::normalize(glm::cross(lookDir, camUp));
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(pos, pos + lookDir, camUp);
}

float Camera::getX() const
{
    return pos.x;
}

float Camera::getY() const
{
    return pos.y;
}

float Camera::getZ() const
{
    return pos.z;
}

float Camera::getYaw() const
{
    return yaw;
}

float Camera::getPitch() const
{
    return pitch;
}

glm::vec2 Camera::getOldMousePos() const
{
    return oldMousePos;
}

glm::vec3 Camera::getLookDir() const
{
    return glm::normalize(lookDir);
}

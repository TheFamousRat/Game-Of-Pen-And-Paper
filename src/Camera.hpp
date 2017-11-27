#ifndef CAMERA_HPP_INCLUDED
#define CAMERA_HPP_INCLUDED

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Graphics.hpp>

#include <iostream>

class Camera{
public:
    Camera();
    Camera(float yaw_, float pitch_, glm::vec3 pos_, glm::vec3 camUp_);
    ~Camera();

    void setAngle(float yaw_, float pitch_);
    void setPosition(glm::vec3& newPos);
    void adaptToEvents(bool toFront, bool toBack, bool toLeft, bool toRight, glm::vec2 mousePos, bool applyChanges = true);//That is, mouse and keyboard
    void allowUpdating(bool allow) {updating = allow;};

    glm::mat4 getViewMatrix() const;

    float getX() const;
    float getY() const;
    float getZ() const;
    float getYaw() const;
    float getPitch() const;
    glm::vec2 getOldMousePos() const;
    glm::vec3 getLookDir() const;

private:
    glm::vec3 pos;
    float yaw;
    float pitch;
    float speed;
    float sensitivity;

    bool updating;

    sf::Clock speedClock;

    glm::vec3 lookDir;//Will be modified by the class, so the user cannot access it
    glm::vec3 rightDir;
    glm::vec3 camUp;

    glm::vec2 oldMousePos;
private:

    void setLookDir();
};

#endif // CAMERA_HPP_INCLUDED

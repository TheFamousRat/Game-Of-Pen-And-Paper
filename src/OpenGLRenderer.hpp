#ifndef OPENGLRENDERER_HPP_INCLUDED
#define OPENGLRENDERER_HPP_INCLUDED

#include <glad/glad.h>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <iostream>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.hpp"
#include "Model.hpp"

class OpenGLRenderer{
public:
    OpenGLRenderer();
    ~OpenGLRenderer();

    void Init(sf::RenderTarget& obj);

    void render(sf::RenderTarget& target, Camera& camObj, sf::Window& window);
private:
    bool init;

    Model dummyModel;
    Mesh dummyMesh;

    sf::Shader modelShader;
    sf::Shader hitboxShader;

    sf::Clock testClock;
    bool firstPointSet;
    glm::vec3 firstPoint;
    glm::vec3 secondPoint;
    glm::mat4 blockedViewMatrix;
};



#endif // OPENGLRENDERER_HPP_INCLUDED

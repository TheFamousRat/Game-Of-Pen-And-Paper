#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <assimp/Importer.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <cmath>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 BiTangent;

    Vertex() {};
    Vertex(glm::vec3 Position_,
           glm::vec3 Normal_ = glm::vec3(0,0,0),
           glm::vec2 TexCoords_ = glm::vec2(0,0),
           glm::vec3 Tangent_ = glm::vec3(0,0,0),
           glm::vec3 BiTangent_ = glm::vec3(0,0,0))
    {
        Position = Position_;
        Normal = Normal_;
        TexCoords = TexCoords_;
        Tangent = Tangent_;
        BiTangent = BiTangent_;
    };

    friend bool operator==(Vertex& a1, Vertex& a2) {return ((a1.Position == a2.Position) &&
                                                            (a1.Normal == a2.Normal) &&
                                                            (a1.TexCoords == a2.TexCoords) &&
                                                            (a1.Tangent == a2.Tangent) &&
                                                            (a1.BiTangent == a2.BiTangent));};

    friend Vertex operator+(Vertex& a1, Vertex& a2)
    {
        Vertex toReturn;

        toReturn.Position = a1.Position + a2.Position;
        toReturn.Normal = a1.Normal + a2.Normal;
        toReturn.TexCoords = a1.TexCoords + a2.TexCoords;
        toReturn.Tangent = a1.Tangent + a2.Tangent;
        toReturn.BiTangent = a1.BiTangent + a2.BiTangent;

        return toReturn;
    };

    friend Vertex operator*(Vertex& a1, float toMul)
    {
        Vertex toReturn;

        toReturn.Position = a1.Position * toMul;
        toReturn.Normal = a1.Normal * toMul;
        toReturn.TexCoords = a1.TexCoords * toMul;
        toReturn.Tangent = a1.Tangent * toMul;
        toReturn.BiTangent = a1.BiTangent * toMul;

        return toReturn;
    };

    friend Vertex operator*(float toMul, Vertex& a1)
    {
        return a1 * toMul;
    };
};

struct Texture {
    unsigned int id;
    std::string type;
    aiString path;
};

class Mesh {
public:

    Mesh();
    Mesh(std::vector<Vertex>& vertices_, std::vector<unsigned int>& indices_, std::vector<Texture>& textures_);
    Mesh(Mesh const& copied)
    {
        vertices = copied.vertices;
        indices = copied.indices;
        textures = copied.textures;

        setupMesh();
    };

    ~Mesh();

    void Draw(sf::Shader& shader, bool wireframe = false);

    void setupMesh();

    unsigned int getVerticesNumber() const {return vertices.size();};

    Mesh cutMeshInTwo(float a, float b, float c, float d, glm::vec3 newMeshVector = glm::vec3(0,0,0));//Effectively cut the triangles, adding new vertices

    std::vector<Vertex> getVertexArray() {return vertices;};
private:
    GLuint VBO, EBO;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    unsigned int VAO;
public:

    Mesh& operator=(const Mesh& a)
    {
        vertices = a.vertices;
        indices = a.indices;
        textures = a.textures;

        setupMesh();

        return *this;
    };

};

float calculateTriangleArea(glm::vec3 pointA, glm::vec3 pointB, glm::vec3 pointC);
float putPlaneOnPoint(glm::vec3 point, float a, float b, float c);

#endif

#ifndef MODEL_HPP_INCLUDED
#define MODEL_HPP_INCLUDED

#include <glad/glad.h>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <iostream>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.hpp"

GLuint TextureFromFile(const char *path, const std::string &directory, bool gamma = false);
GLuint TextureFromFile(const std::string &path, bool gamma = false);

class Model
{
    public:
        /*  Functions   */
        Model();
        Model(std::string path);
        ~Model();

        bool loadModel(std::string path);
        void processNode(aiNode *node, const aiScene *scene);
        Mesh processMesh(aiMesh *mesh, const aiScene *scene);
        void loadMaterialTextures(std::vector<Texture>& target, aiMaterial *mat, aiTextureType type,
                                             std::string typeName);

        void Draw(sf::Shader& shader);

        void cutModelAccordingToPlane(float a, float b, float c, float d, glm::vec3 newMeshVec = glm::vec3(0,0,0));

        unsigned int giveNumberOfMeshes() const {return meshes.size();};
    private:
        std::vector<Mesh> meshes;
        std::string directory;
        static std::vector<Texture> textures_loaded;

};




#endif // MODEL_HPP_INCLUDED

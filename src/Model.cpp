
#include "Model.hpp"

std::vector<Texture> Model::textures_loaded;

Model::Model()
{

}

Model::Model(std::string path)
{
    loadModel(path);
}

Model::~Model()
{

}

bool Model::loadModel(std::string path)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << '\n';
        return false;
    }
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);

    return true;
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal = vector;

        if (mesh->mBitangents != NULL)
        {
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.BiTangent = vector;

            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
        }

        if(mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);


        vertices.push_back(vertex);
    }

    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps;
        loadMaterialTextures(diffuseMaps, material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<Texture> specularMaps;
        loadMaterialTextures(specularMaps, material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        std::vector<Texture> normalMaps;
        loadMaterialTextures(normalMaps, material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        std::vector<Texture> heightMaps;
        loadMaterialTextures(heightMaps, material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    }

    return Mesh(vertices, indices, textures);
}

void Model::loadMaterialTextures(std::vector<Texture>& target, aiMaterial *mat, aiTextureType type,
                                     std::string typeName)
{
    if (!target.empty())
        target.clear();

    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for(unsigned int j = 0; j < Model::textures_loaded.size(); j++)
        {
            if(std::strcmp(Model::textures_loaded[j].path.C_Str(), str.C_Str()) == 0)
            {
                target.push_back(Model::textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if(!skip)
        {
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str;
            target.push_back(texture);
            Model::textures_loaded.push_back(texture);
        }
    }
}

void Model::Draw(sf::Shader& shader)
{

    for(unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);


}

void Model::cutModelAccordingToPlane(float a, float b, float c, float d, glm::vec3 newMeshVec)
{
    std::vector<Mesh> newMeshes;

    for (int i(0) ; i < meshes.size() ; i++)
    {
        newMeshes.push_back(meshes[i].cutMeshInTwo(a,b,c,d, newMeshVec));
    }

    unsigned int numVert(0);

    for (int i(0) ; i < newMeshes.size() ; i++)
    {
        if (newMeshes[i].getVerticesNumber() > 0)
        {
            meshes.push_back(newMeshes[i]);
        }
    }

    for (int i(0) ; i < meshes.size() ; i++)
        numVert += meshes[i].getVerticesNumber();


    std::cout << numVert << " vertices so far\n";
}

GLuint TextureFromFile(const char *path, const std::string &directory, bool gamma)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    sf::Image loadTex;
    if (loadTex.loadFromFile(filename))
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, loadTex.getSize().x, loadTex.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, loadTex.getPixelsPtr());
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Loading failed (path = '" << filename  << "', charging an empty image\n";
        loadTex.create(1, 1, sf::Color(255,0,255));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, loadTex.getSize().x, loadTex.getSize().y, 0, GL_RGB, GL_UNSIGNED_BYTE, loadTex.getPixelsPtr());
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    return textureID;
}

GLuint TextureFromFile(const std::string &path, bool gamma)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    sf::Image loadTex;

    if (loadTex.loadFromFile(path))
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, loadTex.getSize().x, loadTex.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, loadTex.getPixelsPtr());
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Loading failed (path = '" << path  << "'), charging an empty image\n";
        loadTex.create(1, 1, sf::Color(255,0,255));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, loadTex.getSize().x, loadTex.getSize().y, 0, GL_RGB, GL_UNSIGNED_BYTE, loadTex.getPixelsPtr());
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    return textureID;
}


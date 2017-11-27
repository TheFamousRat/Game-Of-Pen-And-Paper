
#include "OpenGLRenderer.hpp"

OpenGLRenderer::OpenGLRenderer()
{
    sf::RenderTexture misc;
    misc.create(1,1);//Create a context

    init = false;

    if(!gladLoadGL() ) {
            printf("Something went wrong!\n");
            exit(-1);
        }

    if (!sf::Shader::isAvailable())
    {
        std::cerr << "Can't use shaders. Maybe your graphic card is too old ? Check your drivers too !\n";
    }
}

OpenGLRenderer::~OpenGLRenderer()
{

}

void OpenGLRenderer::Init(sf::RenderTarget& obj)
{
    obj.pushGLStates();



    dummyModel.loadModel("resources/objects/cube.obj");

float vertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
    };

    std::vector<Vertex> verticesVec;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (int i(0) ; i < 36 ; i++)
    {
        verticesVec.push_back(Vertex(glm::vec3(vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2])));
        indices.push_back(i);
    }

    dummyMesh = Mesh(verticesVec, indices, textures);

    modelShader.loadFromFile("src/shaders/model_loading_vertex.txt", "src/shaders/model_loading_frag.txt");
    hitboxShader.loadFromFile("src/shaders/hitbox_vertex.txt", "src/shaders/hitbox_frag.txt");

    firstPointSet = false;
    init = true;

    obj.popGLStates();
}

void OpenGLRenderer::render(sf::RenderTarget& target, Camera& camObj, sf::Window& window)
{
    if (!init)
    {
        Init(target);
    }

    if (init)
    {
        /*sf::Shader::bind(&modelShader);

        sf::Vector3f lightPos(camObj.getX(), camObj.getY(), camObj.getZ());

        glm::mat4 model, view, projection;
        glm::mat3 normalMat;

        model = glm::mat4();
        model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));


        view = camObj.getViewMatrix();

        projection = glm::perspective(1.0f, (float)target.getSize().x / target.getSize().y, 0.1f, 100.0f);

        normalMat = glm::transpose(glm::inverse(glm::mat3(model)));

        modelShader.setUniform("model", sf::Glsl::Mat4(glm::value_ptr(model)));
        modelShader.setUniform("view", sf::Glsl::Mat4(glm::value_ptr(view)));
        modelShader.setUniform("projection", sf::Glsl::Mat4(glm::value_ptr(projection)));
        modelShader.setUniform("normalMat", sf::Glsl::Mat4(glm::value_ptr(normalMat)));

        modelShader.setUniform("viewPos", sf::Glsl::Vec3(camObj.getX(), camObj.getY(), camObj.getZ()));
        modelShader.setUniform("lightPos", sf::Glsl::Vec3(camObj.getX(), camObj.getY(), camObj.getZ()));


        dummyModel.Draw(modelShader);

        for (int i(0) ; i < 10 ; i++)
        {
            model = glm::mat4();
            model = glm::translate(model, glm::vec3(0.0f, -1.75f, i));
            model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

            modelShader.setUniform("model", sf::Glsl::Mat4(glm::value_ptr(model)));

            dummyModel.Draw(modelShader);
        }

        sf::Shader::bind(nullptr);*/

        sf::Shader::bind(&hitboxShader);

            glm::mat4 model, view, projection;

            model = glm::mat4();
            model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
            model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

            view = camObj.getViewMatrix();

            projection = glm::perspective(1.0f, (float)target.getSize().x / target.getSize().y, 0.1f, 100.0f);

            hitboxShader.setUniform("model", sf::Glsl::Mat4(glm::value_ptr(model)));
            hitboxShader.setUniform("view", sf::Glsl::Mat4(glm::value_ptr(view)));
            hitboxShader.setUniform("projection", sf::Glsl::Mat4(glm::value_ptr(projection)));

            dummyMesh.Draw(hitboxShader);

        sf::Shader::bind(nullptr);
    }
    else
    {
        std::cout << "Can't draw the OpenGLRenderer : its initialization failed.\n";
        exit(-1);
    }
}


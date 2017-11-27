
#include "Mesh.hpp"

Mesh::Mesh()
{

}

Mesh::Mesh(std::vector<Vertex>& vertices_, std::vector<unsigned int>& indices_, std::vector<Texture>& textures_)
{
    VAO = 0;
    VBO = 0;
    EBO = 0;

    vertices = vertices_;
    indices = indices_;
    textures = textures_;

    setupMesh();
}

Mesh::~Mesh()
{
    /*std::cout << "Deleted VAO is " << VAO << '\n';*/
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VAO);
}

void Mesh::Draw(sf::Shader& shader, bool wireframe)
{
    sf::Shader::bind(&shader);

    unsigned int diffuseNr  = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;

    for(unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);

        std::stringstream ss;
        std::string number;
        std::string name = textures[i].type;

        if(name == "texture_diffuse")
            ss << diffuseNr++;
        else if(name == "texture_specular")
            ss << specularNr++;
        else if(name == "texture_normal")
            ss << normalNr++;
        else if(name == "texture_height")
            ss << heightNr++;

        number = ss.str();

        shader.setUniform(("material." + name + number), (float)i);

        glUniform1i(glGetUniformLocation(shader.getNativeHandle(), (name + number).c_str()), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    if (wireframe)
    {
        glPolygonMode(GL_FRONT, GL_LINE);
        glPolygonMode(GL_BACK, GL_LINE);
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);

    if (wireframe)
    {
        glPolygonMode(GL_FRONT, GL_FILL);
        glPolygonMode(GL_BACK, GL_FILL);
    }

    sf::Shader::bind(nullptr);
}


void Mesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);

    /*std::cout << "Created VAO is " << VAO << '\n';*/
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);


        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, BiTangent));

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &EBO);

}

#define EQUALITY_THRESHOLD 0.0000001

Mesh Mesh::cutMeshInTwo(float a, float b, float c, float d, glm::vec3 newMeshVector)
{
    if (vertices.size() % 3 == 0)
    {
        /**Adding the necessary vertices*/

        int interNumber(0);
        Vertex newVertices[2];
        int edgeIntersected[2];

        for (int iii(0) ; iii < vertices.size() ; iii+=3)
        {
            interNumber = 0;

            for (int jjj(0) ; jjj < 3 ; jjj++)
            {
                float e,f,g,h,i,j;//Just recopying my calculations, too bothered to use theStruct.whatev
                e = vertices[iii + jjj].Position.x;//First point
                f = vertices[iii + jjj].Position.y;
                g = vertices[iii + jjj].Position.z;
                h = vertices[iii + ((jjj + 1) % 3)].Position.x;//Second point
                i = vertices[iii + ((jjj + 1) % 3)].Position.y;
                j = vertices[iii + ((jjj + 1) % 3)].Position.z;

                //Tests are done edge by edge
                float t = (-(a * e + b * f + c * g + d))/(a * (h - e) + b * (i - f) + c * (j - g));

                Vertex smallX, highX;
                Vertex smallY, highY;
                Vertex smallZ, highZ;

                if (e < h)//X
                {
                    smallX = vertices[iii + jjj];
                    highX = vertices[iii + ((jjj + 1) % 3)];
                }
                else
                {
                    smallX = vertices[iii + ((jjj + 1) % 3)];
                    highX = vertices[iii + jjj];
                }

                if (f < i)//Y
                {
                    smallY = vertices[iii + jjj];
                    highY = vertices[iii + ((jjj + 1) % 3)];
                }
                else
                {
                    smallY = vertices[iii + ((jjj + 1) % 3)];
                    highY = vertices[iii + jjj];
                }

                if (g < j)//Z
                {
                    smallZ = vertices[iii + jjj];
                    highZ = vertices[iii + ((jjj + 1) % 3)];
                }
                else
                {
                    smallZ = vertices[iii + ((jjj + 1) % 3)];
                    highZ = vertices[iii + jjj];
                }

                float xInter(e + t * (h - e));
                float yInter(f + t * (i - f));
                float zInter(g + t * (j - g));

                if ((smallX.Position.x < xInter && xInter < highX.Position.x))//If yes, we found an intersection
                {

                    float factor = (xInter - smallX.Position.x) / (highX.Position.x - smallX.Position.x);

                    //Doing weighted averages to get new vertices data
                    newVertices[interNumber].Position = factor * highX.Position + (1 - factor) * smallX.Position;
                    newVertices[interNumber].BiTangent = factor * highX.BiTangent + (1 - factor) * smallX.BiTangent;
                    newVertices[interNumber].Normal = factor * highX.Normal + (1 - factor) * smallX.Normal;
                    newVertices[interNumber].Tangent = factor * highX.Tangent + (1 - factor) * smallX.Tangent;
                    newVertices[interNumber].TexCoords = factor * highX.TexCoords + (1 - factor) * smallX.TexCoords;

                    edgeIntersected[interNumber] = jjj;
                    interNumber++;
                }
                else if ((smallY.Position.y < yInter && yInter < highY.Position.y))//If yes, we found an intersection
                {

                    float factor = (yInter - smallY.Position.y) / (highY.Position.y - smallY.Position.y);

                    //Doing weighted averages to get new vertices data
                    newVertices[interNumber].Position = factor * highY.Position + (1 - factor) * smallY.Position;
                    newVertices[interNumber].BiTangent = factor * highY.BiTangent + (1 - factor) * smallY.BiTangent;
                    newVertices[interNumber].Normal = factor * highY.Normal + (1 - factor) * smallY.Normal;
                    newVertices[interNumber].Tangent = factor * highY.Tangent + (1 - factor) * smallY.Tangent;
                    newVertices[interNumber].TexCoords = factor * highY.TexCoords + (1 - factor) * smallY.TexCoords;

                    edgeIntersected[interNumber] = jjj;
                    interNumber++;
                }
                else if ((smallZ.Position.z < zInter && zInter < highZ.Position.z))//If yes, we found an intersection
                {

                    float factor = (zInter - smallZ.Position.z) / (highZ.Position.z - smallZ.Position.z);

                    //Doing weighted averages to get new vertices data
                    newVertices[interNumber].Position = factor * highZ.Position + (1 - factor) * smallZ.Position;
                    newVertices[interNumber].BiTangent = factor * highZ.BiTangent + (1 - factor) * smallZ.BiTangent;
                    newVertices[interNumber].Normal = factor * highZ.Normal + (1 - factor) * smallZ.Normal;
                    newVertices[interNumber].Tangent = factor * highZ.Tangent + (1 - factor) * smallZ.Tangent;
                    newVertices[interNumber].TexCoords = factor * highZ.TexCoords + (1 - factor) * smallZ.TexCoords;

                    edgeIntersected[interNumber] = jjj;
                    interNumber++;
                }

            }

            if (interNumber == 1)
            {
                Vertex layout[4];
                Vertex finalLayout[6];

                layout[0] = vertices[iii +  edgeIntersected[0]];
                layout[1] = newVertices[0];
                layout[2] = vertices[iii + ((edgeIntersected[0] + 1) % 3)];
                layout[3] = vertices[iii + ((edgeIntersected[0] + 2) % 3)];

                finalLayout[0] = layout[2];
                finalLayout[1] = layout[0];
                finalLayout[2] = layout[1];
                finalLayout[3] = layout[3];
                finalLayout[4] = layout[0];
                finalLayout[5] = layout[2];

                //Now lets finally insert the new vertices
                vertices.insert(vertices.begin() + iii, finalLayout, finalLayout+3);


                for (int jjj(0) ; jjj < 6 ; jjj++)
                    vertices[iii + jjj] = finalLayout[jjj];

                iii += 3;//We skip the two added triangles

            }
            else if (interNumber == 2)
            {
                Vertex layout[5];
                Vertex finalLayout[9];

                if (edgeIntersected[1] == ((edgeIntersected[0] + 1)%3))
                {
                    layout[0] = vertices[iii + edgeIntersected[0]];
                    layout[1] = newVertices[0];
                    layout[2] = vertices[iii + edgeIntersected[1]];
                    layout[3] = newVertices[1];
                    layout[4] = vertices[iii + ((edgeIntersected[1] + 1)%3)];
                }
                else if (((edgeIntersected[1] + 1)%3) == edgeIntersected[0])
                {
                    layout[0] = vertices[iii + edgeIntersected[1]];
                    layout[1] = newVertices[1];
                    layout[2] = vertices[iii + edgeIntersected[0]];
                    layout[3] = newVertices[0];
                    layout[4] = vertices[iii + ((edgeIntersected[0] + 1)%3)];
                }
                else
                {
                    std::cout << "NOOOOOOOOOO\n";
                    exit(-1);
                }

                finalLayout[0] = layout[4];
                finalLayout[1] = layout[0];
                finalLayout[2] = layout[1];

                finalLayout[3] = layout[3];
                finalLayout[4] = layout[2];
                finalLayout[5] = layout[1];

                finalLayout[6] = layout[4];
                finalLayout[7] = layout[1];
                finalLayout[8] = layout[3];

                //Now lets finally insert the new vertices

                vertices.insert(vertices.begin() + iii, finalLayout, finalLayout+6);

                for (int jjj(0) ; jjj < 9 ; jjj++)
                    vertices[iii + jjj] = finalLayout[jjj];

                iii += 6;//We skip the two added triangles
            }

        }
        while (indices.size() < vertices.size())
        {
            indices.push_back(0);
            indices[indices.size() - 1] = indices[indices.size() - 2] + 1;
        }

        std::vector<Vertex> verticesS1;
        std::vector<unsigned int> indicesS1;
        std::vector<Texture> texturesS1;

        std::vector<Vertex> verticesS2;
        std::vector<unsigned int> indicesS2;
        std::vector<Texture> texturesS2;
        texturesS1 = textures;
        texturesS2 = textures;

        ///Now we spread the triangles around the plane
        for (int iii(0) ; iii < vertices.size() ; iii+=3)
        {
            float dAvg(0.0f);
            short numOfD(0);

            for (int jjj(0) ; jjj < 3 ; jjj++)
            {
                if (std::abs(a * vertices[iii + jjj].Position.x + b * vertices[iii + jjj].Position.y + c * vertices[iii + jjj].Position.z + d) > 10e-7)
                {
                    //if (std::abs(a * vertices[iii + jjj].Position.x + b * vertices[iii + jjj].Position.y + c * vertices[iii + jjj].Position.z + d) < 1)
                    //std::cout << std::abs(a * vertices[iii + jjj].Position.x + b * vertices[iii + jjj].Position.y + c * vertices[iii + jjj].Position.z + d) << '\n';
                    numOfD++;
                    dAvg += putPlaneOnPoint(vertices[iii + jjj].Position, a, b, c);
                }
            }

            if (numOfD > 0)
            {
                dAvg /= numOfD;

                if (dAvg > d)
                {
                    for (int jjj(0) ; jjj < 3 ; jjj++)
                    {
                        verticesS1.push_back(vertices[iii + jjj]);
                        indicesS1.push_back(indices[iii + jjj]);
                    }
                }
                else
                {
                    for (int jjj(0) ; jjj < 3 ; jjj++)
                    {
                        verticesS2.push_back(vertices[iii + jjj]);
                        indicesS2.push_back(indices[iii + jjj]);
                    }
                }
            }
        }

        for (int i(0) ; i < indicesS1.size() ; i++)
            indicesS1[i] = i;

        for (int i(0) ; i < indicesS2.size() ; i++)
            indicesS2[i] = i;

        vertices = verticesS1;
        indices = indicesS1;
        textures = texturesS1;

        if (newMeshVector != glm::vec3(0,0,0))
            for (int i(0) ; i < verticesS2.size() ; i++)
                verticesS2[i].Position += newMeshVector;

        setupMesh();
        return Mesh(verticesS2, indicesS2, texturesS2);
    }
    else
    {
        std::cout << "Mesh has non-triangular faces, see to fix it.\n";
        return;
    }
}

float calculateTriangleArea(glm::vec3 pointA, glm::vec3 pointB, glm::vec3 pointC)
{
    if (pointA != pointB && pointA != pointC && pointB != pointC)
    {
        float AB = glm::distance(pointA, pointB);
        float BC = glm::distance(pointB, pointC);
        float AC = glm::distance(pointA, pointC);
        float s = (AB + BC + AC)/2;

        return sqrtf(s * (s - AB) * (s - BC) * (s - AC));
    }
    else
        return 0.0f;
}

float putPlaneOnPoint(glm::vec3 point, float a, float b, float c)
{
    return (-(a * point.x + b * point.y + c * point.z));
}

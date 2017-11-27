#ifndef CLASSES_H_INCLUDED
#define CLASSES_H_INCLUDED

#include "OpenGLRenderer.hpp"
#include "Camera.hpp"

#include <SFML/Graphics.hpp>
#include <queue>
#include <memory>

#include "Nodes.hpp"
#include <TGUI/TGUI.hpp>
#include "NodeSetup.hpp"
#include "Input.hpp"

#define ID_TYPE unsigned int

class System{
public:
    System();
    ~System();

    void manageWindowEvent(sf::Event event, sf::RenderWindow& window, int FPS);//Processes event in the window (zooms, moving...)
    void writeDebugInfo(sf::RenderWindow& window, int FPS) const;//Used for debug. Write info on the screen about the current system, window...

    bool isDebugDisplayed() const;

    bool pointInWidgetAbsolutePos(sf::Vector2f point, tgui::Widget::Ptr widget) const
    {
        return (point.x >= widget.get()->getAbsolutePosition().x &&
                point.x <= widget.get()->getAbsolutePosition().x + widget.get()->getFullSize().x &&
                point.y >= widget.get()->getAbsolutePosition().y &&
                point.y <= widget.get()->getAbsolutePosition().y + widget.get()->getFullSize().y);
    };

    sf::RenderWindow* returnRenderWindowPointer() const;

    void blockFPS(sf::Clock &fpsClock, int &currentFrameNumber, const int wishedFPS) const;
    void editMenu();

    void console();

    void updateWidgets();
    void renderWidgets();

    void renderOpenGL(sf::RenderTarget& target);

    void Update();
    void Render();

private:
    std::map<sf::Vector2i, Node> allNodes;

    tgui::Gui windowGui;

    Camera cameraRender;
    OpenGLRenderer oglpart;

    sf::Font debug_font;

    sf::RenderWindow windowMain;

    bool displayConsole;
    bool displayDebug;

    sf::Event event;

    int FPS;
    int numberOfFrames;

    sf::Clock clockFPS;
};

//A simple class to symbolize and perform operation on vectors
class Vector{
public:
    Vector() {x=0; y=0;};
    Vector(float xToGive, float yToGive) {x=xToGive; y=yToGive;};
    ~Vector() {};

    Vector normalize() const {double length(sqrt((x*x) + (y*y))); return Vector(x / length, y / length);};
    static Vector sumVector(Vector *data, int dataSize)
    {
        Vector dummy(0,0);

        for (int i(0) ; i < dataSize ; i++)
            dummy += *(data + i);

        return dummy;
    };

    friend Vector operator+(const Vector& A, const Vector& B) {return Vector(A.x + B.x, A.y + B.y);};
    friend void operator+=(Vector& A, const Vector& B) {A.x += B.x; A.y += B.y;};
    friend std::ostream& operator<<(std::ostream& out, const Vector A)
    {
        out << '(' << A.x << ';' << A.y << ')';
        return out;
    };

private:
    float x, y;
};

#endif // CLASSES_H_INCLUDED

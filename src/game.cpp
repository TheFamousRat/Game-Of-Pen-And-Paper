
#include "game.hpp"
#include "Nodes.hpp"
#include "OpenGLRenderer.hpp"
#include "WorldGen.hpp"

#include "Entity.hpp"

unsigned int wishedFPS = 60;
unsigned int Path::currentID = 0;

void gameLoop()
{
    genMap("result.png");

    /*System mainSystem;
    Input::init();

    do
    {
        mainSystem.Update();

        mainSystem.Render();

    }while (1);*/
}

void System::Update()
{
    if (clockFPS.getElapsedTime().asSeconds() >= 1.0)
    {
        FPS = numberOfFrames;

        //std::cout << FPS << '\n';

        if (numberOfFrames == 0)
            FPS = 1;

        numberOfFrames = 0;
        clockFPS.restart();
    }

    Input::updateButtons();

    while (windowMain.pollEvent(event))
    {
        manageWindowEvent(event, windowMain, FPS);

        Input::updateEvents(event);
        windowGui.handleEvent(event);
    }

    cameraRender.adaptToEvents(Input::checkKeyboardButtonInteracted(sf::Keyboard::Up) || Input::checkKeyboardButtonInteracted(sf::Keyboard::Z),
                                Input::checkKeyboardButtonInteracted(sf::Keyboard::Down) || Input::checkKeyboardButtonInteracted(sf::Keyboard::S),
                                Input::checkKeyboardButtonInteracted(sf::Keyboard::Left) || Input::checkKeyboardButtonInteracted(sf::Keyboard::Q),
                                Input::checkKeyboardButtonInteracted(sf::Keyboard::Right) || Input::checkKeyboardButtonInteracted(sf::Keyboard::D),
                                glm::vec2(sf::Mouse::getPosition().x, sf::Mouse::getPosition().y), true);
}

void System::Render()
{


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    windowMain.clear(sf::Color(255,0,255));

    renderOpenGL(windowMain);

    windowMain.pushGLStates();

        //renderWidgets();
        windowGui.draw();

    windowMain.popGLStates();

    windowMain.display();

    numberOfFrames++;
}

void System::blockFPS(sf::Clock &fpsClock, int &currentFrameNumber, const int wishedFPS) const
{
    while (fpsClock.getElapsedTime().asSeconds() < static_cast<float>(currentFrameNumber)/static_cast<float>(wishedFPS));
}

sf::RenderWindow* System::returnRenderWindowPointer() const
{
    return &windowMain;
}

void System::writeDebugInfo(sf::RenderWindow& window, int FPS) const
{
    if (displayDebug)
    {
        sf::Text debug_text;
        debug_text.setCharacterSize(16);
        debug_text.setColor(sf::Color(255,0,0));
        debug_text.setFont(debug_font);
        debug_text.setPosition(0,0);

        std::stringstream myStringStream;

        myStringStream << "Welcome to the debug menu (press F12 to leave ;) )\n";

            myStringStream << "Current FPS count : " << FPS << '\n';

        myStringStream << "That's all I'm allowed to say...\n";

        debug_text.setString(myStringStream.str());
        window.draw(debug_text);
    }
}

bool System::isDebugDisplayed() const
{
    return displayDebug;
}

void System::editMenu()
{

}

void System::console()
{
    if (displayConsole)
    {
        std::cout << "MEH";
    }
}

void System::manageWindowEvent(sf::Event event, sf::RenderWindow& window, int FPS)
{
    double speed((double)SCROLL_SPEED / FPS);

    if (Input::checkKeyboardButtonPushed(sf::Keyboard::F12))
        displayDebug = !displayDebug;

    if (Input::checkKeyboardButtonPushed(sf::Keyboard::T))
        displayConsole = !displayConsole;
}

System::System() :  windowMain(sf::VideoMode(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT),
                               "Game", sf::Style::Default, sf::ContextSettings(24,8,4))
{
    //windowMain.setFramerateLimit(60);

    glViewport(0, 0, windowMain.getSize().x, windowMain.getSize().y);
    windowMain.setVerticalSyncEnabled(true);
    windowMain.setActive(true);

    oglpart.Init(windowMain);

    glEnable(GL_DEPTH_TEST);

    debug_font.loadFromFile("arial.ttf");

    displayConsole = false;
    displayDebug = false;//Pretty self-explanatory : if true, debug infos are displayed

    clockFPS.restart();

    numberOfFrames = 0;
    FPS = wishedFPS;

    windowGui.setTarget(windowMain);

    try
    {
        loadWidgets(windowGui);
    }
    catch (const tgui::Exception& e)
    {
        std::cerr << "Failed to load TGUI widgets: " << e.what() << std::endl;
    }
}

void login(tgui::EditBox::Ptr username, tgui::EditBox::Ptr password)
{
    std::cout << "Username: " << username->getText().toAnsiString() << std::endl;
    std::cout << "Password: " << password->getText().toAnsiString() << std::endl;
}

void loadWidgets( tgui::Gui& gui )
{
    // Create the background image
    // The picture is of type tgui::Picture::Ptr which is actually just a typedef for std::shared_widget<Picture>
    // The picture will fit the entire window and will scale with it
    tgui::Theme theme{"themes/Black.txt"};

    auto picture = tgui::Picture::create("xubuntu_bg_aluminium.jpg");
    picture->setSize({"100%", "100%"});
    gui.add(picture);

    auto panel = tgui::NodeSetup::create({"50%", "100%"});
        //panel.get()->setPosition("0%", "5%");
    //tgui::ScrollablePanel::

    auto checkbox1 = tgui::CheckBox::create();
        checkbox1->setRenderer(theme.getRenderer("CheckBox"));
        checkbox1->setPosition(0, 0);
        checkbox1->setSize(25, 25);
        panel->add(checkbox1, "checkbox1");

    auto WorldMap = tgui::Picture::create();

    gui.add(panel, "panel");
}

void System::renderOpenGL(sf::RenderTarget& target)
{
    oglpart.render(target, cameraRender, windowMain);
}

System::~System(){};


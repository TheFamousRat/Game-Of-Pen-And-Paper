#ifndef INPUT_HPP_INCLUDED
#define INPUT_HPP_INCLUDED

#include <SFML/Graphics.hpp>
#include <iostream>

class Input{
public:

    static void init();

    static void updateButtons();
    static void updateEvents(sf::Event const event);

    static bool checkKeyboardButtonInteracted(sf::Keyboard::Key key);//Pushed or maintained
    static bool checkKeyboardButtonMaintained(sf::Keyboard::Key key);
    static bool checkKeyboardButtonPushed(sf::Keyboard::Key key);
    static void nullifyPush(sf::Keyboard::Key key);//For example, when the key's push has already been used (less useful than click)

    static bool checkMouseButtonInteracted(sf::Mouse::Button button);//Clicked or maintained
    static bool checkMouseButtonMaintained(sf::Mouse::Button button);
    static bool checkMouseButtonClicked(sf::Mouse::Button button);
    static void nullifyClick(sf::Mouse::Button button);//For example, when the click has already been registered and used, or used on something forefront (interfaces...)
    static void nullifyClickMaintained(sf::Mouse::Button button);

    static sf::Vector2i getMouseVector();
    static sf::Vector2i getWheelScrollPosition();
    static float getWheelDelta();

private:
    static sf::Vector2i wheelScrollPosition;
    static float wheelDelta;

    static bool mouseButtonsMaintained[sf::Mouse::ButtonCount];
    static bool ignoreMouseMaintained[sf::Mouse::ButtonCount];
    static bool mouseButtonClick[sf::Mouse::ButtonCount];
    static bool ignoreMouseClick[sf::Mouse::ButtonCount];

    static bool keyboardButtonsMaintained[sf::Keyboard::KeyCount];
    static bool keyboardButtonPushed[sf::Keyboard::KeyCount];

    static sf::Vector2i currentMouseCoords;
    static sf::Vector2i previousMouseCoords;
};

#endif // INPUT_HPP_INCLUDED

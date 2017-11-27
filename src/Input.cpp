
#include "Input.hpp"

sf::Vector2i Input::wheelScrollPosition;
float Input::wheelDelta;

bool Input::mouseButtonsMaintained[sf::Mouse::ButtonCount];
bool Input::ignoreMouseMaintained[sf::Mouse::ButtonCount];
bool Input::mouseButtonClick[sf::Mouse::ButtonCount];
bool Input::ignoreMouseClick[sf::Mouse::ButtonCount];

bool Input::keyboardButtonsMaintained[sf::Keyboard::KeyCount];
bool Input::keyboardButtonPushed[sf::Keyboard::KeyCount];

sf::Vector2i Input::currentMouseCoords;
sf::Vector2i Input::previousMouseCoords;

void Input::init()
{
    for (int i(0) ; i < sf::Mouse::ButtonCount ; i++)
    {
        ignoreMouseClick[i] = false;
        ignoreMouseMaintained[i] = false;
    }

    for (int i(0) ; i<sf::Mouse::ButtonCount ; i++)//Initialize states of the mouse buttons
    {
        mouseButtonsMaintained[i] = false;
        mouseButtonClick[i] = false;
    }

    for (int i(0) ; i<sf::Keyboard::KeyCount ; i++)//And of the keyboard buttons
    {
        keyboardButtonPushed[i] = false;
        keyboardButtonsMaintained[i] = false;
    }

    currentMouseCoords = sf::Mouse::getPosition();
    previousMouseCoords = sf::Mouse::getPosition();
}

void Input::updateButtons()
{
    //Update mouse
    for (int i(0);i<sf::Mouse::ButtonCount;i++)
    {
        if (sf::Mouse::isButtonPressed(i))//If the button is pressed...
        {
            if (mouseButtonClick[i] == false && mouseButtonsMaintained[i] == false)//We first for once set it on clicked...
            {
                mouseButtonClick[i] = true;
                ignoreMouseClick[i] = false;
            }
            else if (mouseButtonClick[i] == true && mouseButtonsMaintained[i] == false)//Then block it on maintained
            {
                mouseButtonClick[i] = false;
                mouseButtonsMaintained[i] = true;
                ignoreMouseMaintained[i] = false;
            }
        }
        else if (mouseButtonClick[i] || mouseButtonsMaintained[i])//Release the button
        {
            mouseButtonClick[i] = false;
            mouseButtonsMaintained[i] = false;
            ignoreMouseClick[i] = false;
            ignoreMouseMaintained[i] = false;
        }
    }

    previousMouseCoords = currentMouseCoords;
    currentMouseCoords = sf::Mouse::getPosition();

    //Update keyboard
    for (int i(0); i<sf::Keyboard::KeyCount ; i++)
    {
        if (sf::Keyboard::isKeyPressed(i))//If the button is pressed...
        {
            if (keyboardButtonPushed[i] == false && keyboardButtonsMaintained[i] == false)//We first for once set it on clicked...
            {
                keyboardButtonPushed[i] = true;
            }
            else if (keyboardButtonPushed[i] == true && keyboardButtonsMaintained[i] == false)//Then block it on maintained
            {
                keyboardButtonPushed[i] = false;
                keyboardButtonsMaintained[i] = true;
            }
        }
        else if (keyboardButtonPushed[i] || keyboardButtonsMaintained[i])//Release the button
        {

            keyboardButtonPushed[i] = false;
            keyboardButtonsMaintained[i] = false;
        }
    }

    Input::wheelDelta = 0;
    Input::wheelScrollPosition.x = 0;
    Input::wheelScrollPosition.y = 0;
}

void Input::updateEvents(sf::Event const event)
{
    if (event.type == sf::Event::MouseWheelScrolled)
    {
        Input::wheelDelta = event.mouseWheelScroll.delta;
        Input::wheelScrollPosition.x = event.mouseWheelScroll.x;
        Input::wheelScrollPosition.y = event.mouseWheelScroll.y;
    }
}

bool Input::checkKeyboardButtonInteracted(sf::Keyboard::Key key)
{
    return (Input::checkKeyboardButtonMaintained(key) || Input::checkKeyboardButtonPushed(key));
}

bool Input::checkKeyboardButtonMaintained(sf::Keyboard::Key key)
{
    return keyboardButtonsMaintained[key];
}

bool Input::checkKeyboardButtonPushed(sf::Keyboard::Key key)
{
    return keyboardButtonPushed[key];
}

void Input::nullifyPush(sf::Keyboard::Key key)
{
    keyboardButtonPushed[key] = false;
    if (sf::Keyboard::isKeyPressed(key))
        keyboardButtonsMaintained[key] = true;
    else
        keyboardButtonsMaintained[key] = false;
}

bool Input::checkMouseButtonInteracted(sf::Mouse::Button button)
{
    return (Input::checkMouseButtonMaintained(button) || Input::checkMouseButtonClicked(button));
}

bool Input::checkMouseButtonMaintained(sf::Mouse::Button button)
{
    if (ignoreMouseMaintained[button])
        return false;
    else
        return mouseButtonsMaintained[button];
}

bool Input::checkMouseButtonClicked(sf::Mouse::Button button)
{
    if (ignoreMouseClick[button])
        return false;
    else
        return mouseButtonClick[button];
}

void Input::nullifyClick(sf::Mouse::Button button)
{
    ignoreMouseClick[button] = true;
}

void Input::nullifyClickMaintained(sf::Mouse::Button button)
{
    ignoreMouseMaintained[button] = true;
}

sf::Vector2i Input::getMouseVector()
{
    return (currentMouseCoords - previousMouseCoords);
}

sf::Vector2i Input::getWheelScrollPosition()
{
    return wheelScrollPosition;
}

float Input::getWheelDelta()
{
    return wheelDelta;
}

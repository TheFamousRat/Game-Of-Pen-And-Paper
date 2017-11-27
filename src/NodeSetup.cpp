/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TGUI - Texus' Graphical User Interface
// Copyright (C) 2012-2017 Bruno Van de Velde (vdv_b@tgui.eu)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "NodeSetup.hpp"
#include <TGUI/Vector2f.hpp>
#include <TGUI/Clipping.hpp>
#include <cmath>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    NodeSetup::NodeSetup(const Layout2d& size, sf::Vector2f contentSize) :
        Panel{size}
    {
        m_type = "NodeSetup";

        m_renderer = aurora::makeCopied<NodeSetupRenderer>();
        setRenderer(Theme::getDefault()->getRendererNoThrow(m_type));

        // Rotate the horizontal scrollbar
        m_horizontalScrollbar.setSize(m_horizontalScrollbar.getSize().y, m_horizontalScrollbar.getSize().x);

        setContentSize(contentSize);

        zoomLevel = 1.0f;
        zoomSpeed = 2.0f;

        minPosition.x = 0.0f;
        minPosition.y = 0.0f;
        maxPosition.x = 0.0f;
        maxPosition.y = 0.0f;

        upLeftMark = tgui::Canvas::create();
            upLeftMark.get()->setSize(1,1);
            upLeftMark.get()->setPosition(0,0);
            upLeftMark.get()->clear(sf::Color(255,0,0));
            add(upLeftMark);

        downRightMark = tgui::Canvas::create();
            downRightMark.get()->setSize(1,1);
            downRightMark.get()->setPosition(0,0);
            downRightMark.get()->clear(sf::Color(255,0,0));
            add(downRightMark);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    NodeSetup::Ptr NodeSetup::create(Layout2d size, sf::Vector2f contentSize)
    {
        return std::make_shared<NodeSetup>(size, contentSize);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    NodeSetup::Ptr NodeSetup::copy(NodeSetup::ConstPtr panel)
    {
        if (panel)
            return std::static_pointer_cast<NodeSetup>(panel->clone());
        else
            return nullptr;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    NodeSetupRenderer* NodeSetup::getSharedRenderer()
    {
        return aurora::downcast<NodeSetupRenderer*>(Widget::getSharedRenderer());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const NodeSetupRenderer* NodeSetup::getSharedRenderer() const
    {
        return aurora::downcast<const NodeSetupRenderer*>(Widget::getSharedRenderer());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    NodeSetupRenderer* NodeSetup::getRenderer()
    {
        return aurora::downcast<NodeSetupRenderer*>(Widget::getRenderer());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const NodeSetupRenderer* NodeSetup::getRenderer() const
    {
        return aurora::downcast<const NodeSetupRenderer*>(Widget::getRenderer());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NodeSetup::setSize(const Layout2d& size)
    {
        Panel::setSize(size);
        updateScrollbars();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NodeSetup::add(const tgui::Widget::Ptr& widget, const sf::String& widgetName)
    {
        Panel::add(widget, widgetName);

        const sf::Vector2f bottomRight = widget->getPosition() + widget->getFullSize();

        //If the space was empty
        if (m_contentSize == sf::Vector2f{0, 0})
        {
            if (bottomRight.x > m_mostBottomRightPosition.x)
                m_mostBottomRightPosition.x = bottomRight.x;
            if (bottomRight.y > m_mostBottomRightPosition.y)
                m_mostBottomRightPosition.y = bottomRight.y;

            updateScrollbars();
        }

        if (widget != upLeftMark && widget != downRightMark)
        {
            recalculateWidgetBounds();

            downRightMark.get()->setPosition(std::max((unsigned int)getFullSize().x + 1, maxPosition.x) - 1,
                                            std::max((unsigned int)getFullSize().y + 1, maxPosition.y) - 1);
        }


    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NodeSetup::moveWidgetsWithMouse(sf::Vector2f relativeMousePos)
    {
        if (widgetMoving != nullptr)
        {
            if (!Input::checkMouseButtonInteracted(sf::Mouse::Left))
            {
                widgetMoving = nullptr;
            }

        }
        else if (Input::checkMouseButtonInteracted(sf::Mouse::Left))
        {
            for (auto it = m_widgets.begin() ; it != m_widgets.end() ; it++)
            {
                if ((*it) != upLeftMark && (*it) != downRightMark)
                {
                    if (mouseHoveredWidget(relativeMousePos, *it))
                    {

                        widgetMoving = *it;
                        break;
                    }
                }
            }
        }

        if (widgetMoving != nullptr)
        {
            moveWidgetWithMouse();

            recalculateWidgetBounds();

            upLeftMark.get()->setPosition(std::min(minPosition.x, m_horizontalScrollbar.getValue()),
                                          std::min(minPosition.y, m_verticalScrollbar.getValue()));

            if (widgetMoving.get()->getPosition().x != upLeftMark.get()->getPosition().x || mouseVec.x > 0)
            {
                downRightMark.get()->setPosition(std::max((unsigned int)m_horizontalScrollbar.getValue() + m_horizontalScrollbar.getLowValue(), maxPosition.x),
                                                 downRightMark.get()->getPosition().y);
            }

            if (widgetMoving.get()->getPosition().y != upLeftMark.get()->getPosition().y || mouseVec.y > 0)
            {
                downRightMark.get()->setPosition(downRightMark.get()->getPosition().x,
                                                std::max((unsigned int)(m_verticalScrollbar.getValue() + m_verticalScrollbar.getLowValue() + m_horizontalScrollbar.getFullSize().y), maxPosition.y));
            }

            std::cout << m_verticalScrollbar.getLowValue() << ' ' << m_verticalScrollbar.getMaximum() << '\n';

            sf::Vector2u toRemove(upLeftMark.get()->getPosition().x, upLeftMark.get()->getPosition().y);

            for (auto it = getWidgets().begin() ; it != getWidgets().end() ; it++)
            {
                tgui::Widget* widg = (*it).get();

                widg->setPosition(widg->getPosition().x - toRemove.x, widg->getPosition().y - toRemove.y);
            }

            unsigned int horizontalGap(m_horizontalScrollbar.getMaximum() - m_horizontalScrollbar.getValue());
            unsigned int verticalGap(m_verticalScrollbar.getMaximum() - m_verticalScrollbar.getValue());

            m_horizontalScrollbar.setValue(m_horizontalScrollbar.getValue() - toRemove.x);
            m_verticalScrollbar.setValue(m_verticalScrollbar.getValue() - toRemove.y);

            recalculateMostBottomRightPosition();
            updateScrollbars();

            if (repositionHorizontal)
            {
                m_horizontalScrollbar.setValue(m_horizontalScrollbar.getMaximum() - horizontalGap);
            }

            if (repositionVertical)
            {
                m_verticalScrollbar.setValue(m_verticalScrollbar.getMaximum() - verticalGap);
            }



        }


    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NodeSetup::recalculateWidgetBounds()
    {
        bool firstSet(false);

        for (auto it = getWidgets().begin() ; it != getWidgets().end() ; it++)
        {
            if ((*it) != upLeftMark && (*it) != downRightMark)
            {
                tgui::Widget* widg = (*it).get();

                if (firstSet)
                {
                    if (widg->getPosition().x < minPosition.x)
                        minPosition.x = widg->getPosition().x;
                    if (widg->getPosition().y < minPosition.y)
                        minPosition.y = widg->getPosition().y;
                    if (widg->getPosition().x + widg->getFullSize().x > maxPosition.x)
                        maxPosition.x = widg->getPosition().x + widg->getFullSize().x;
                    if (widg->getPosition().y + widg->getFullSize().y > maxPosition.y)
                        maxPosition.y = widg->getPosition().y + widg->getFullSize().y;
                }
                else
                {
                    firstSet = true;
                    minPosition.x = widg->getPosition().x;
                    minPosition.y = widg->getPosition().y;
                    maxPosition.x = widg->getPosition().x + widg->getFullSize().x;
                    maxPosition.y = widg->getPosition().y + widg->getFullSize().y;
                }
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NodeSetup::moveWidgetWithMouse()
    {
        tgui::Widget* widget = widgetMoving.get();

        if (widget->getPosition().x + mouseVec.x < 0)
        {
            int gap = widget->getPosition().x + mouseVec.x;

            std::vector<tgui::Widget::Ptr> vec = getWidgets();

            for (auto it = vec.begin() ; it != vec.end() ; it++)
            {

                //Hopefully removing some hideous syntax
                tgui::Widget* temp = (*it).get();

                temp->setPosition(temp->getPosition().x - gap, temp->getPosition().y);

            }

            widget->setPosition(0, widget->getPosition().y);

            repositionHorizontal = true;
        }
        else
        {
            widget->setPosition(widget->getPosition().x + mouseVec.x, widget->getPosition().y);

            repositionHorizontal = false;
        }

        if (widget->getPosition().y + mouseVec.y < 0)
        {
            int gap = widget->getPosition().y + mouseVec.y;

            std::vector<tgui::Widget::Ptr> vec = getWidgets();

            for (auto it = vec.begin() ; it != vec.end() ; it++)
            {

                //Hopefully removing some hideous syntax
                tgui::Widget* temp = (*it).get();

                temp->setPosition(temp->getPosition().x, temp->getPosition().y - gap);

            }

            widget->setPosition(widget->getPosition().x, 0);

            repositionVertical = true;
        }
        else
        {
            widget->setPosition(widget->getPosition().x, widget->getPosition().y + mouseVec.y);

            repositionVertical = false;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NodeSetup::updateWheelScroll()
    {
        if (Input::getWheelDelta())
        {
            if (Input::getWheelDelta() > 0 && zoomLevel < 8)
            {
                zoomLevel *= zoomSpeed;

                auto vec = getWidgets();

                for (auto it = vec.begin() ; it != vec.end() ; it++)
                {
                    tgui::Widget* widg = (*it).get();

                    widg->setPosition(widg->getPosition().x * zoomSpeed, widg->getPosition().y * zoomSpeed);

                    if ((*it) != upLeftMark && (*it) != downRightMark)
                    {
                        widg->setSize(widg->getSize().x * zoomSpeed, widg->getSize().y * zoomSpeed);
                    }
                }
            }
            else if (zoomLevel > 0.125 && Input::getWheelDelta() < 0)//Negative, wheel scrolled backwards
            {
                zoomLevel /= zoomSpeed;

                auto vec = getWidgets();

                for (auto it = vec.begin() ; it != vec.end() ; it++)
                {
                    tgui::Widget* widg = (*it).get();

                    widg->setPosition(widg->getPosition().x / zoomSpeed, widg->getPosition().y / zoomSpeed);

                    if ((*it) != upLeftMark && (*it) != downRightMark)
                    {
                        widg->setSize(widg->getSize().x / zoomSpeed, widg->getSize().y / zoomSpeed);
                    }
                }
            }

            float previousHorizontalPercent((float)m_horizontalScrollbar.getValue() / (m_horizontalScrollbar.getMaximum() - m_horizontalScrollbar.getLowValue()));
            float previousVerticalPercent((float)m_verticalScrollbar.getValue() / (m_verticalScrollbar.getMaximum() - m_verticalScrollbar.getLowValue()));

            recalculateMostBottomRightPosition();
            updateScrollbars();

            m_horizontalScrollbar.setValue(previousHorizontalPercent * (m_horizontalScrollbar.getMaximum() - m_horizontalScrollbar.getLowValue()));
            m_verticalScrollbar.setValue(previousVerticalPercent * (m_verticalScrollbar.getMaximum() - m_verticalScrollbar.getLowValue()));
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool NodeSetup::remove(const Widget::Ptr& widget)
    {
        const sf::Vector2f bottomRight = widget->getPosition() + widget->getFullSize();

        const bool ret = Panel::remove(widget);

        if (m_contentSize == sf::Vector2f{0, 0})
        {
            if ((bottomRight.x == m_mostBottomRightPosition.x) || (bottomRight.y == m_mostBottomRightPosition.y))
            {
                recalculateMostBottomRightPosition();
                updateScrollbars();
            }
        }

        recalculateWidgetBounds();

        downRightMark.get()->setPosition(std::max((unsigned int)getFullSize().x + 1, maxPosition.x) - 1,
                                         std::max((unsigned int)getFullSize().y + 1, maxPosition.y) - 1);

        return ret;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NodeSetup::removeAllWidgets()
    {
        Panel::removeAllWidgets();

        if (m_contentSize == sf::Vector2f{0, 0})
        {
            recalculateMostBottomRightPosition();
            updateScrollbars();
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NodeSetup::setContentSize(sf::Vector2f size)
    {
        m_contentSize = size;

        if (m_contentSize == sf::Vector2f{0, 0})
            recalculateMostBottomRightPosition();

        updateScrollbars();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    sf::Vector2f NodeSetup::getContentSize() const
    {
        if (m_contentSize != sf::Vector2f{0, 0})
            return m_contentSize;
        else if (m_widgets.empty())
            return getInnerSize();
        else
            return m_mostBottomRightPosition;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    sf::Vector2f NodeSetup::getContentOffset() const
    {
        return {static_cast<float>(m_horizontalScrollbar.getValue()), static_cast<float>(m_verticalScrollbar.getValue())};
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NodeSetup::leftMousePressed(sf::Vector2f pos)
    {
        m_mouseDown = true;

        if (m_verticalScrollbar.mouseOnWidget(pos - getPosition()))
            m_verticalScrollbar.leftMousePressed(pos - getPosition());
        else if (m_horizontalScrollbar.mouseOnWidget(pos - getPosition()))
            m_horizontalScrollbar.leftMousePressed(pos - getPosition());
        else if (sf::FloatRect{getPosition().x + getChildWidgetsOffset().x, getPosition().y + getChildWidgetsOffset().y, getInnerSize().x, getInnerSize().y}.contains(pos))
        {
            Panel::leftMousePressed({pos.x + static_cast<float>(m_horizontalScrollbar.getValue()),
                                     pos.y + static_cast<float>(m_verticalScrollbar.getValue())});
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NodeSetup::leftMouseReleased(sf::Vector2f pos)
    {
        if (m_verticalScrollbar.mouseOnWidget(pos - getPosition()))
            m_verticalScrollbar.leftMouseReleased(pos - getPosition());
        else if (m_horizontalScrollbar.mouseOnWidget(pos - getPosition()))
            m_horizontalScrollbar.leftMouseReleased(pos - getPosition());
        else if (sf::FloatRect{getPosition().x + getChildWidgetsOffset().x, getPosition().y + getChildWidgetsOffset().y, getInnerSize().x, getInnerSize().y}.contains(pos))
        {
            Panel::leftMouseReleased({pos.x + static_cast<float>(m_horizontalScrollbar.getValue()),
                                      pos.y + static_cast<float>(m_verticalScrollbar.getValue())});
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NodeSetup::mouseMoved(sf::Vector2f pos)
    {
        mouseVec = pos - lastMousePos;
        lastMousePos = pos;


        // Check if the mouse event should go to the scrollbar
        if ((m_verticalScrollbar.isMouseDown() && m_verticalScrollbar.isMouseDownOnThumb()) || m_verticalScrollbar.mouseOnWidget(pos - getPosition()))
        {
            m_verticalScrollbar.mouseMoved(pos - getPosition());
        }
        else if ((m_horizontalScrollbar.isMouseDown() && m_horizontalScrollbar.isMouseDownOnThumb()) || m_horizontalScrollbar.mouseOnWidget(pos - getPosition()))
        {
            m_horizontalScrollbar.mouseMoved(pos - getPosition());
        }
        else // Mouse not on scrollbar or dragging the scrollbar thumb
        {
            if (sf::FloatRect{getPosition().x + getChildWidgetsOffset().x, getPosition().y + getChildWidgetsOffset().y, getInnerSize().x, getInnerSize().y}.contains(pos))
            {
                Panel::mouseMoved({pos.x + static_cast<float>(m_horizontalScrollbar.getValue()),
                                   pos.y + static_cast<float>(m_verticalScrollbar.getValue())});
            }

            m_verticalScrollbar.mouseNoLongerOnWidget();
            m_horizontalScrollbar.mouseNoLongerOnWidget();
        }

        moveWidgetsWithMouse(pos);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NodeSetup::mouseWheelScrolled(float delta, sf::Vector2f pos)
    {
        updateWheelScroll();

        if (m_horizontalScrollbar.isShown() && m_horizontalScrollbar.mouseOnWidget(pos - getPosition()))
        {
            m_horizontalScrollbar.mouseWheelScrolled(delta, pos - getPosition());
            mouseMoved(pos);
        }
        else if (m_verticalScrollbar.isShown())
        {
            m_verticalScrollbar.mouseWheelScrolled(delta, pos - getPosition());
            mouseMoved(pos);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NodeSetup::mouseNoLongerOnWidget()
    {
        Panel::mouseNoLongerOnWidget();
        m_verticalScrollbar.mouseNoLongerOnWidget();
        m_horizontalScrollbar.mouseNoLongerOnWidget();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NodeSetup::mouseNoLongerDown()
    {
        Panel::mouseNoLongerDown();
        m_verticalScrollbar.mouseNoLongerDown();
        m_horizontalScrollbar.mouseNoLongerDown();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NodeSetup::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        states.transform.translate(getPosition());

        const auto oldStates = states;

        // Draw the borders
        if (m_bordersCached != Borders{0})
        {
            drawBorders(target, states, m_bordersCached, getSize(), m_borderColorCached);
            states.transform.translate({m_bordersCached.getLeft(), m_bordersCached.getTop()});
        }

        // Draw the background
        const sf::Vector2f innerSize = {getSize().x - m_bordersCached.getLeft() - m_bordersCached.getRight(),
                                        getSize().y - m_bordersCached.getTop() - m_bordersCached.getBottom()};
        drawRectangleShape(target, states, innerSize, m_backgroundColorCached);

        states.transform.translate(m_paddingCached.getLeft(), m_paddingCached.getTop());
        sf::Vector2f contentSize = {innerSize.x - m_paddingCached.getLeft() - m_paddingCached.getRight(),
                                    innerSize.y - m_paddingCached.getTop() - m_paddingCached.getBottom()};

        if (m_verticalScrollbar.getMaximum() > m_verticalScrollbar.getLowValue())
            contentSize.x -= m_verticalScrollbar.getSize().x;
        if (m_horizontalScrollbar.getMaximum() > m_horizontalScrollbar.getLowValue())
            contentSize.y -= m_horizontalScrollbar.getSize().y;

        // Draw the child widgets
        {
            const Clipping clipping{target, states, {}, contentSize};

            states.transform.translate(-static_cast<float>(m_horizontalScrollbar.getValue()),
                                       -static_cast<float>(m_verticalScrollbar.getValue()));

            drawWidgetContainer(&target, states);
        }

        m_verticalScrollbar.draw(target, oldStates);
        m_horizontalScrollbar.draw(target, oldStates);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NodeSetup::rendererChanged(const std::string& property)
    {
        if (property == "scrollbar")
        {
            m_verticalScrollbar.setRenderer(getSharedRenderer()->getScrollbar());
            m_horizontalScrollbar.setRenderer(getSharedRenderer()->getScrollbar());
        }
        else
            Panel::rendererChanged(property);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::unique_ptr<DataIO::Node> NodeSetup::save(SavingRenderersMap& renderers) const
    {
        auto node = Panel::save(renderers);
        node->propertyValuePairs["ContentSize"] = make_unique<DataIO::ValueNode>("(" + to_string(m_contentSize.x) + ", " + to_string(m_contentSize.y) + ")");
        return node;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NodeSetup::load(const std::unique_ptr<DataIO::Node>& node, const LoadingRenderersMap& renderers)
    {
        Panel::load(node, renderers);

        if (node->propertyValuePairs["contentsize"])
            setContentSize(Vector2f{node->propertyValuePairs["contentsize"]->value});
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NodeSetup::updateScrollbars()
    {
        const sf::Vector2f visibleSize = getInnerSize();
        m_horizontalScrollbar.setLowValue(static_cast<unsigned int>(visibleSize.x));
        m_verticalScrollbar.setLowValue(static_cast<unsigned int>(visibleSize.y));

        const sf::Vector2f contentSize = getContentSize();
        m_horizontalScrollbar.setMaximum(static_cast<unsigned int>(contentSize.x));
        m_verticalScrollbar.setMaximum(static_cast<unsigned int>(contentSize.y));

        const bool horizontalScrollbarVisible = m_horizontalScrollbar.getMaximum() > m_horizontalScrollbar.getLowValue();
        if (horizontalScrollbarVisible)
        {
            m_verticalScrollbar.setSize(m_verticalScrollbar.getSize().x, getInnerSize().y - m_horizontalScrollbar.getSize().y);
            m_verticalScrollbar.setLowValue(static_cast<unsigned int>(m_verticalScrollbar.getLowValue() - m_horizontalScrollbar.getSize().y));

            const bool verticalScrollbarVisible = m_verticalScrollbar.getMaximum() > m_verticalScrollbar.getLowValue();
            if (verticalScrollbarVisible)
                m_horizontalScrollbar.setSize(getInnerSize().x - m_verticalScrollbar.getSize().x, m_horizontalScrollbar.getSize().y);
            else
                m_horizontalScrollbar.setSize(getInnerSize().x, m_horizontalScrollbar.getSize().y);
        }
        else
        {
            m_verticalScrollbar.setSize(m_verticalScrollbar.getSize().x, getInnerSize().y);

            const bool verticalScrollbarVisible = m_verticalScrollbar.getMaximum() > m_verticalScrollbar.getLowValue();
            if (verticalScrollbarVisible)
            {
                m_horizontalScrollbar.setSize(getInnerSize().x - m_verticalScrollbar.getSize().x, m_horizontalScrollbar.getSize().y);
                m_horizontalScrollbar.setLowValue(static_cast<unsigned int>(m_horizontalScrollbar.getLowValue() - m_verticalScrollbar.getSize().x));

                if (m_horizontalScrollbar.getMaximum() > m_horizontalScrollbar.getLowValue())
                    m_verticalScrollbar.setSize(m_verticalScrollbar.getSize().x, getInnerSize().y - m_horizontalScrollbar.getSize().y);
            }
            else
                m_horizontalScrollbar.setSize(getInnerSize().x, m_horizontalScrollbar.getSize().y);
        }

        m_verticalScrollbar.setPosition(getChildWidgetsOffset().x + getInnerSize().x - m_verticalScrollbar.getSize().x, getChildWidgetsOffset().y);
        m_horizontalScrollbar.setPosition(getChildWidgetsOffset().x, getChildWidgetsOffset().y + getInnerSize().y - m_horizontalScrollbar.getSize().y);

        const float verticalSpeed = 40.f * (static_cast<float>(m_verticalScrollbar.getMaximum() - m_verticalScrollbar.getLowValue()) / m_verticalScrollbar.getLowValue());
        m_verticalScrollbar.setScrollAmount(static_cast<unsigned int>(std::ceil(std::sqrt(verticalSpeed))));

        const float horizontalSpeed = 40.f * (static_cast<float>(m_horizontalScrollbar.getMaximum() - m_horizontalScrollbar.getLowValue()) / m_horizontalScrollbar.getLowValue());
        m_horizontalScrollbar.setScrollAmount(static_cast<unsigned int>(std::ceil(std::sqrt(horizontalSpeed))));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void NodeSetup::recalculateMostBottomRightPosition()
    {
        m_mostBottomRightPosition = {0, 0};

        for (const auto& widget : m_widgets)
        {
            const sf::Vector2f bottomRight = widget->getPosition() + widget->getFullSize();
            if (bottomRight.x > m_mostBottomRightPosition.x)
                m_mostBottomRightPosition.x = bottomRight.x;
            if (bottomRight.y > m_mostBottomRightPosition.y)
                m_mostBottomRightPosition.y = bottomRight.y;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef NODES_HPP_INCLUDED
#define NODES_HPP_INCLUDED

#include <iostream>
#include <SFML/Graphics.hpp>

struct RoadType{
    double avgSpeed;
    sf::Color roadColor;
};

enum RoadID{
    SMALL_ROAD,
    AVERAGE_ROAD,
    HUGE_ROAD,
    BOAT
};
//A set of vertex, which design a road
class Path{
public:
    Path(bool putID = false);
    Path(sf::Vector2i *pathData, int pathDataSize, bool putID = false);
    ~Path();

    int getPathLength() const;
    sf::Vector2i getPointCoordinates(int pointIndex) const;
    void editPointCoordinates(int index, sf::Vector2i newCoordinates);
private:
    static std::map<std::string, RoadType> allRoads;
    //Contains all roads types. For example, we can have
    //road by foot, by boat... Each different road
    //type has a color, and an estimated average speed. Boat,
    // for example, if faster than foot

    static unsigned int currentID;

    int ID;
    RoadID roadIdentity;//The type of the road (maritime way, horse etc.)
    std::string pathName;//The name of the road
    std::vector<sf::Vector2i> pathPoints;//The key points who define the shape of the road
};

class Node{
public:

private:
    sf::Color nodeColor;//Just for testing purposes

    std::string nodeName;
    sf::Vector2i nodeCoordinates;
    std::vector<int> pathConnectedTo;//List of ID's of paths to which the Node is connected
};

#endif // NODES_HPP_INCLUDED

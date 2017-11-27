#ifndef ENTITY_HPP_INCLUDED
#define ENTITY_HPP_INCLUDED

#include <iostream>
#include <sstream>

#include "Python.hpp"
#include "Model.hpp"

class Entity{
public:
    Entity() {};
    Entity(std::string className, double x_, double y_, double z_, std::string modelLocation);
    ~Entity() {};

    Position returnPos() {return Position(1,47,52);};

private:
    double x, y, z;

    pythonEntity pyEntity;
    Model entityModel;
};

#endif // ENTITY_HPP_INCLUDED

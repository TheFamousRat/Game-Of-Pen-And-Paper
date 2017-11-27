
#include "Nodes.hpp"

Path::Path(bool putID /*= false*/)
{
    ID = Path::currentID;
    currentID++;


}

Path::Path(sf::Vector2i *pathData, int pathDataSize, bool putID /*= false*/)
{
    ID = Path::currentID;
    currentID++;

    for (int i(0) ; i < pathDataSize ; i++)
        pathPoints.push_back(*(pathData + i));
}

Path::~Path()
{

}

sf::Vector2i Path::getPointCoordinates(int pointIndex) const
{
    return pathPoints[pointIndex];
}

void Path::editPointCoordinates(int index, sf::Vector2i newCoordinates)
{
    pathPoints[index] = newCoordinates;
}

int Path::getPathLength() const
{
    double lengthToReturn(0);

    for (int i(0) ; i < pathPoints.size() - 1 ; i++)
    {
        lengthToReturn += sqrt( (pathPoints[i+1].x-pathPoints[i].x)*(pathPoints[i+1].x-pathPoints[i].x) + (pathPoints[i+1].y-pathPoints[i].y)*(pathPoints[i+1].x-pathPoints[i].x) );
    }

    return static_cast<int>(lengthToReturn);
}

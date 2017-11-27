
#include "Entity.hpp"

Entity::Entity(std::string className, double x_, double y_, double z_, std::string modelLocation) :
    pyEntity(className, x_, y_, z_)
{
    x = x_;
    y = y_;
    z = z_;

    std::stringstream strm;
    std::string pyVarName = pyEntity.getVariablePythonName();

    entityModel.loadModel(modelLocation);

    for (int i(0) ; i < entityModel.giveNumberOfMeshes() ; i++)
    {

        //Each new member is initialized with a life point count of 100 by default
        pyEntity.addModelMember(i, 100);
    }

    strm << pyVarName << ".printTab()\n";
    boost::python::exec(strm.str().c_str(), pythonInterpreter::global);
}

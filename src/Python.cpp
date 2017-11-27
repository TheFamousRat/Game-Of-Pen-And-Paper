
#include "Python.hpp"

unsigned long long pythonEntity::instanceNumber = 0;
bool pythonInterpreter::initialized = false;
bool pythonInterpreter::somethingWentWrong = false;
boost::python::object pythonInterpreter::main;
boost::python::object pythonInterpreter::global;

BOOST_PYTHON_MODULE(gameFunc)
{
    boost::python::class_<Position>("Position")
        .def_readwrite("x", &Position::x)
        .def_readwrite("y", &Position::y)
        .def_readwrite("z", &Position::z)
    ;

    boost::python::def("thisFunc", thisFunc);
}

static bool pythonInterpreter::init()
{
    try
    {
        Py_Initialize();

        pythonInterpreter::main = boost::python::import("__main__");
        pythonInterpreter::global = boost::python::object(pythonInterpreter::main.attr("__dict__"));

        initgameFunc();

        initialized = true;
        somethingWentWrong = false;
    }
    catch (const boost::python::error_already_set&)
    {
        PyErr_Print();
        initialized = false;
        somethingWentWrong = true;
    }

    return initialized;
}

pythonEntity::pythonEntity(std::string className, double x, double y, double z)
{
    if (!pythonInterpreter::didSomethingGoWrong())
    {
        try
        {
            if (!pythonInterpreter::wasInitialized())
            {
                pythonInterpreter::init();
            }

            if (pythonInterpreter::wasInitialized())
            {
                std::stringstream strm;
                strm << className << pythonEntity::instanceNumber++;
                variableName = strm.str();
                strm.str(std::string());

                strm << "import imp\n";
                strm << "temp = imp.load_source('" << className << "', 'scripts/entities/" << className << ".py')\n";
                strm << variableName << " = temp." << className << "(" << x << ',' << y << ',' << z << ")\n";

                boost::python::exec(strm.str().c_str(), pythonInterpreter::global);


                pythonClass = pythonInterpreter::global[variableName.c_str()];
            }
            else
            {
                std::cout << "Could not create an instance of pythonEntity : something went wrong\n";
            }
        }
        catch (const boost::python::error_already_set&)
        {
            PyErr_Print();
        }
    }
}

void pythonEntity::addModelMember(unsigned long long id, double lifePoint)
{
    //We prepare a python line of code, adding a new mesh to the "tab" list.
    std::stringstream strm;

    strm << variableName << ".tab.append(" << variableName << ".ModelData(" << id << ',' << lifePoint << "))\n";

    boost::python::exec(strm.str().c_str(), pythonInterpreter::global);
}

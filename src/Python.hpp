#ifndef PYTHON_HPP_INCLUDED
#define PYTHON_HPP_INCLUDED

#include <cmath>
#include <iostream>
#include <boost/python.hpp>
#include <sstream>
#include "gameFunc.hpp"

class pythonInterpreter{
private:

    pythonInterpreter() {initialized = false;};
    ~pythonInterpreter() {};

public:

    static bool wasInitialized()  {return initialized;};
    static bool didSomethingGoWrong()  {return somethingWentWrong;};

    static bool init();

    static void stopInterpreter()
    {
        Py_Finalize();
        pythonInterpreter::initialized = false;
        pythonInterpreter::somethingWentWrong = false;
    };

    static boost::python::object main;
    static boost::python::object global;

private:

    static bool initialized;
    static bool somethingWentWrong;
};

class pythonEntity{
public:

    pythonEntity() {};//Nothing
    pythonEntity(std::string className, double x, double y, double z);
    ~pythonEntity() {};

    void __init__()
    {
        pythonClass.attr("___init__")();
    };

    void addModelMember(unsigned long long id, double lifePoint);

    std::string getVariablePythonName() const {return variableName;};

private:

    boost::python::object pythonClass;
    std::string variableName;

    static unsigned long long instanceNumber;
    static bool initialized;

};


#endif // PYTHON_HPP_INCLUDED

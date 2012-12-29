#ifndef INCLUDED_HPP_OBJECTS_CONTAINER
#define INCLUDED_HPP_OBJECTS_CONTAINER
#include "object.hpp"
struct objects_container
{

    double pos[3];
    double rot[3];

    std::vector<object> objs;

};


#endif

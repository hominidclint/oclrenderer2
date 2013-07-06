#ifndef INCLUDED_HPP_OBJECTS_CONTAINER
#define INCLUDED_HPP_OBJECTS_CONTAINER
#include "object.hpp"
struct objects_container
{
    int id;
    static int gid;

    bool isactive;
    double pos[3];
    double rot[3];

    std::vector<object> objs;

    objects_container();

    void set_active(bool param);
};


#endif

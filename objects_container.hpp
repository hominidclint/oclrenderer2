#ifndef INCLUDED_HPP_OBJECTS_CONTAINER
#define INCLUDED_HPP_OBJECTS_CONTAINER
#include "object.hpp"
#include <boost/function.hpp>

struct objects_container
{
    cl_uint id;
    static cl_uint gid;

    cl_uint arrange_id; ///ie position in desc

    std::string file;

    boost::function<void (objects_container*)> fp;

    bool isactive;
    bool isloaded;

    cl_float4 pos;
    cl_float4 rot;

    std::vector<object> objs;
    static std::vector<objects_container> obj_container_list;

    objects_container();
    cl_uint push();

    void    set_pos(cl_float4);
    void    set_file(std::string);
    cl_uint set_active(bool param);
    void    set_active_subobjs(bool);
    void    unload_tris();
    void    set_load_func  (boost::function<void (objects_container*)>);
    void    call_load_func (objects_container*);

    void g_flush_objects(); ///calls g_flush for all objects
};


#endif

#include "objects_container.hpp"
#include <iostream>
#include <boost/bind.hpp>
#include "obj_load.hpp"

cl_uint objects_container::gid = 0;
std::vector<objects_container> objects_container::obj_container_list;

objects_container::objects_container()
{
    isactive = false;
    isloaded = false;
    pos = (cl_float4){0,0,0,0};
    rot = (cl_float4){0,0,0,0};
    set_load_func(boost::bind(obj_load, _1));
}

cl_uint objects_container::push()
{
    obj_container_list.push_back(*this);
    return obj_container_list.size() - 1;
}

void objects_container::set_pos(cl_float4 _pos) ///both remote and local
{
    pos = _pos;
    for(unsigned int i=0; i<objs.size(); i++)
    {
        objs[i].pos = _pos;
    }

    if(isactive)
    {
        obj_container_list[id].pos = _pos;
        for(unsigned int i=0; i<obj_container_list[id].objs.size(); i++)
        {
            obj_container_list[id].objs[i].pos = _pos;
        }
    }
}

void objects_container::set_file(std::string f)
{
    file = f;
}

    ///push objs to thing? Manage from here?
void objects_container::set_active_subobjs(bool param)
{
    for(unsigned int i=0; i<objs.size(); i++)
    {
        objs[i].set_pos(pos);
        objs[i].set_rot(rot);
        objs[i].set_active(param);
    }
}

cl_uint objects_container::set_active(bool param)
{
    if(!isactive && param)
    {
        isactive = param;
        id = push();
        return id;
    }

    if(isactive && !param)
    {
        std::vector<objects_container>::iterator it = objects_container::obj_container_list.begin();
        for(unsigned int i=0; i<id; i++)
        {
            it++;
        }
        objects_container::obj_container_list.erase(it);
        id = -1;
    }

    isactive = param;
    return id;
}

void objects_container::unload_tris()
{
    for(unsigned int i=0; i<objs.size(); i++)
    {
        std::vector<triangle>().swap(objs[i].tri_list);
    }
}


void objects_container::set_load_func(boost::function<void (objects_container*)> func)
{
    fp = func;
}

void objects_container::call_load_func(objects_container* c)
{
    fp(c);
}


void objects_container::g_flush_objects()
{
    if(isactive)
    {
        objects_container *T = &objects_container::obj_container_list[id];

        for(unsigned int i=0; i<T->objs.size(); i++)
        {
            T->objs[i].object_g_id = id;
            T->objs[i].object_sub_position = i;
        }

        for(unsigned int i=0; i<T->objs.size(); i++)
        {
            T->objs[i].g_flush(arrange_id);
        }
    }
    else
    {
        std::cout << "Warning (objects_container.hpp) " << __LINE__ << " g_flush_objects called on object not pushed to global storage" << std::endl;
    }
}

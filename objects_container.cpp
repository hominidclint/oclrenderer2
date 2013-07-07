#include "objects_container.hpp"
#include <iostream>

cl_uint objects_container::gid = 0;
std::vector<objects_container> objects_container::obj_container_list;

objects_container::objects_container()
{
    isactive = false;
    isloaded = false;
}

cl_uint objects_container::push()
{
    obj_container_list.push_back(*this);
    return obj_container_list.size() - 1;
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
        objs[i].set_pos_rot(pos, rot);
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

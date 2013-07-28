#include "object.hpp"
#include "clstate.h"
#include "texture.hpp"
#include "obj_mem_manager.hpp"
#include "objects_container.hpp"
#include <iostream>

object::object()
{
    pos.x=0, pos.y=0, pos.z=0;
    rot.x=0, rot.y=0, rot.z=0;
    tid = 0;
    atid = 0;
    isactive = false;
    has_bump = 0;
}

///activate the textures in an object
void object::set_active(bool param)
{
    if(param)
    {
        if(!isactive)
        {
            ///if object ! initialised, error
            isactive = param;
            texture::texturelist[tid].type = 0;
            atid = texture::texturelist[tid].set_active(true);

            if(has_bump)
            {
                texture::texturelist[bid].type = 1;
                abid = texture::texturelist[bid].set_active(true);
            }
        }
        else
        {
            return;
        }
    }
    else
    {
        if(isactive)
        {
            isactive = param;

            std::vector<cl_uint>::iterator it = texture::active_textures.begin();
            for(int i=0; i<atid; i++)
            {
                it++;
            }
            ///remove from active texturelist
            texture::active_textures.erase(it);
            atid = 0;

            if(has_bump)
            {
                std::vector<cl_uint>::iterator it2 = texture::active_textures.begin();
                for(int i=0; i<abid; i++)
                {
                    it2++;
                }
                ///remove from active texturelist
                texture::active_textures.erase(it2);
                abid = 0;
            }
        }
        else
        {
            return;
        }
    }
}

void object::set_pos(cl_float4 _pos)
{
    pos = _pos;
}

void object::set_rot(cl_float4 _rot)
{
    rot = _rot;
}

void object::g_flush(cl_uint arrange_id)
{
    ///get id
    //objects_container *T = &objects_container::obj_container_list[object_g_id];

    //object *Tobj = &T->objs[object_sub_position];

    int cumulative = 0;

    for(int i=0; i<arrange_id; i++)
    {
        cumulative+=obj_mem_manager::obj_sub_nums[i];
    }

    ///need cumulative sub object position

    clEnqueueWriteBuffer(cl::cqueue, obj_mem_manager::g_obj_desc, CL_TRUE, sizeof(obj_g_descriptor)*(cumulative + object_sub_position), sizeof(cl_float4), &pos, 0, NULL, NULL);
}

#include "object.hpp"
#include "clstate.h"
#include "texture.hpp"

object::object()
{
    pos.x=0, pos.y=0, pos.z=0;
    rot.x=0, rot.y=0, rot.z=0;
    tid = 0;
    atid = 0;
    isactive = false;
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
            atid = texture::texturelist[tid].set_active(true);
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
        }
        else
        {
            return;
        }
    }
}

void object::set_pos_rot(cl_float4 _pos, cl_float4 _rot)
{
    pos = _pos;
    rot = _rot;
}

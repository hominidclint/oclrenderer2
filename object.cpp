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

void object::alloc(int num)
{
    tri_list.reserve(num);
    tri_num=(unsigned int)num;
}

void object::set_active(bool param)
{
    if(param)
    {
        if(!isactive)
        {
            isactive = param;
            ///if object ! initialised, error
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
            atid = 0;
            ///remove from active texturelist
        }
        else
        {
            return;
        }
    }
}

#include "object.hpp"
#include "clstate.h"

object::object()
{
    pos.x=0, pos.y=0, pos.z=0;
    rot.x=0, rot.y=0, rot.z=0;
    tid=0;
    onvector=false;
}

void object::alloc(int num)
{
    tri_list.reserve(num);
    tri_num=(unsigned int)num;
}

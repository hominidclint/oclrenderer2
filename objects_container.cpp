#include "objects_container.hpp"

int objects_container::gid = 0;

objects_container::objects_container()
{
    isactive = false;
}

void objects_container::set_active(bool param)
{
    isactive = param;
    for(unsigned int i=0; i<objs.size(); i++)
    {
        objs[i].set_active(param);
    }
}

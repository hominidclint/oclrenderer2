#include "object.hpp"
#include "clstate.h"

object::object()
{
    //x=0, y=0, z=0;
    pos.x=0, pos.y=0, pos.z=0;
    rot.x=0, rot.y=0, rot.z=0;

    tid=0;
    onvector=false;
}

void object::alloc(int num)
{
    tri_list=new triangle[num];
    tri_num=(unsigned int)num;
}

void object::g_alloc()
{

    g_mem     = clCreateBuffer(cl::context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(triangle)*tri_num, tri_list, &cl::error);
    g_tri_num = clCreateBuffer(cl::context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint), &tri_num, &cl::error);

}

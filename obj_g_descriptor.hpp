#ifndef INCLUDED_H_OBJ_G_DESCRIPTOR
#define INCLUDED_H_OBJ_G_DESCRIPTOR
#include <CL/CL.h>

struct obj_g_descriptor
{
    cl_float4 world_pos; ///w is blaenk
    cl_float4 world_rot; ///w is blaenk
    cl_uint start; ///internally used value
    cl_uint tri_num;
    cl_uint tid;///texture id
    cl_uint size;
};

#endif // INCLUDED_H_OBJ_G_DESCRIPTOR

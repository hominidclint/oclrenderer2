#include "light.hpp"

std::vector<light> light::lightlist;

void light::set_pos(cl_float4 p)
{
    pos=p;
}

void light::set_col(cl_float4 c)
{
    col=c;
}

void light::set_shadow_bright(cl_uint isshadowcasting, cl_float bright)
{
    shadow=isshadowcasting, brightness=bright;
}

light::light()
{
    shadow=0;
    col = (cl_float4){1.0, 1.0, 1.0, 0.0};
}

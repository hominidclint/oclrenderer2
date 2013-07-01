#ifndef INCLUDED_HPP_TEXTURE
#define INCLUDED_HPP_TEXTURE
#include <sfml/graphics.hpp>
#include <cl/cl.h>
#include <vector>

#include "obj_g_descriptor.hpp"

struct texture
{

    cl_mem image;
    sf::Image c_image;
    std::string location;

    static std::vector<texture> texturelist;

    bool loaded;

    bool g_pushed;

    cl_uint id; ///starts from 1
    static cl_uint gidc;

    cl_uint mip_level_ids[MIP_LEVELS];

    texture();


    static bool t_compare(texture one, texture two);

    static cl_uint idquerystring(std::string);

    //static void generate_mipmaps();

    cl_uint loadtomaster(std::string);

    cl_uint init();

    void unload();
};



#endif

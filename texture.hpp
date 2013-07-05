#ifndef INCLUDED_HPP_TEXTURE
#define INCLUDED_HPP_TEXTURE
#include <sfml/graphics.hpp>
#include <cl/cl.h>
#include <vector>

#include "obj_g_descriptor.hpp"

struct texture
{
    sf::Image c_image;
    ///location is unique string that is texture location, used to check if textures refer to the same thing
    std::string location;

    static std::vector<texture> texturelist;

    bool loaded;

    cl_uint id; ///starts from 0
    static cl_uint gidc;

    cl_uint mip_level_ids[MIP_LEVELS];

    texture();

    static bool t_compare(texture one, texture two);

    static cl_uint idquerystring(std::string);

    //static void generate_mipmaps();

    cl_uint get_largest_dimension();

    cl_uint loadtomaster(std::string);

    cl_uint init();

    void unload();
};



#endif

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
    static std::vector<cl_uint> active_textures;

    bool loaded;
    bool isactive;

    cl_uint id; ///starts from 0
    static cl_uint gidc;

    cl_uint mip_level_ids[MIP_LEVELS];

    texture();

    static bool t_compare(texture one, texture two);

    static cl_uint idquerystring(std::string);

    static cl_uint idqueryisactive(cl_uint);

    static cl_uint idquerytexture(cl_uint);

    //static void generate_mipmaps();

    cl_uint get_largest_dimension();

    void set_texture_location(std::string);

    cl_uint loadtomaster();

    void init();

    cl_uint get_id();

    cl_uint push();

    cl_uint set_active(bool);

    void unload();
};



#endif

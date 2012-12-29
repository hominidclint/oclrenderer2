#ifndef INCLUDED_HPP_TEXTURE
#define INCLUDED_HPP_TEXTURE
#include <sfml/graphics.hpp>
#include <cl/cl.h>
#include <vector>

struct texture
{

    cl_mem image;
    //cl_float4 ** c_gcompatible_image;
    sf::Image c_image;
    std::string location;

    static std::vector<texture> texturelist;

    bool loaded;

    bool g_pushed;


    cl_uint id; ///starts from 1
    static cl_uint gidc;


    texture();

    static bool t_compare(texture one, texture two);

    static cl_uint idquerystring(std::string);

    cl_uint loadtomaster(std::string);

    static void generate_mipmaps();

    void unload();

    void g_push();
    void g_pull();




};



#endif

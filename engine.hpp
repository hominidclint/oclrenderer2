#ifndef INCLUDED_HPP_ENGINE
#define INCLUDED_HPP_ENGINE
#include <SFML/graphics.hpp>
#include "object.hpp"
#include <vector>
#include "objects_container.hpp"

struct engine
{


    cl_uint width, height, depth;
    cl_uint g_size; /// height > width rounded up to nearest power of 2
    cl_float4 c_pos;
    cl_float4 c_rot;

    cl_mem g_screen;

    cl_mem g_c_pos;
    cl_mem g_c_rot;

    cl_mem g_triangle_id_framebuffer;
    cl_mem g_triangle_depth_framebuffer;

    cl_mem depth_buffer;

    cl_mem g_depth_screen;

    cl_mem g_id_screen;

    cl_mem g_normals_screen;

    cl_mem g_texture_screen;

    //cl_mem g_obj_descriptors;

    static unsigned int gl_screen_id;
    static unsigned int gl_framebuffer_id;




    sf::RenderWindow window;

    std::vector<object*> objects;

    void load(cl_uint, cl_uint, cl_uint, std::string);

    void draw_poor_objs(objects_container&);

    void draw_obj(object &obj);

    void draw_bulk_objs();

    void draw_bulk_objs_new();

    void draw_bulk_objs_test();

    void draw_tile_objs();

    void render_buffers();

    void input();






};



#endif

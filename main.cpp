#include <cstdio>
#include <string>
#include <iostream>


#include <windows.h>
#include <gl/gl.h>

#include "clstate.h"
#include "obj_load.hpp"
#include "objects_container.hpp"
#include "texture.hpp"
#include "ocl.h"
#include "engine.hpp"
#include "obj_mem_manager.hpp"
#include "obj_g_descriptor.hpp"
#include <math.h>
#include <limits.h>

///todo
///modify textures to return largest dimension

int main(int argc, char *argv[])
{
    sf::Clock clo;

    //objects_container *sponza=obj_load("Sp2/sp2.obj");

    /*if(sponza == NULL)
    {
        std::cout << "could not load file" << std::endl;
        exit(1);
    }*/

    objects_container sponza;

    sponza.set_file("Sp2/sp2.obj");
    sponza.pos = (cl_float4){0,0,0,0};
    sponza.rot = (cl_float4){0,0,0,0};
    sponza.set_active(true);


    //cl_uint id = sponza.push();
    ///pushes textures


    obj_mem_manager g_manage;

    engine window;
    oclstuff();
    window.load(800,600,1000, "turtles");

    g_manage.g_arrange_mem();
    g_manage.g_changeover();

    g_manage.g_arrange_mem();
    g_manage.g_changeover();

    sf::Event Event;

    light l;
    l.set_col((cl_float4){1.0, 1.0, 1.0, 0});
    l.set_shadow_bright(1, 1);
    l.set_pos((cl_float4){0, 1000, 0, 0});
    l.set_pos((cl_float4){-800, 150, -800, 0});
    window.add_light(l);

    l.set_pos((cl_float4){0, 300, 800, 0});
    l.shadow=0;
    window.add_light(l);


    //l.shadow=0;
    //window.add_light(l);

    window.construct_shadowmaps();

    while(window.window.isOpen())
    {
        sf::Clock c;

        if(window.window.pollEvent(Event))
        {
            if(Event.type == sf::Event::Closed)
                window.window.close();

        }


        window.input();

        window.draw_bulk_objs_n();

        window.render_buffers();

        {
            int mx = window.get_mouse_x();
            int my = window.get_mouse_y();
            //std::cout << mx << std::endl << my << std::endl;

            //if(mx > 0 && mx < 800 && my > 0 && my < 600)
            //    std::cout << (float)window.d_depth_buf[(600-my)*800 + (mx)]/UINT_MAX << std::endl;
                //std::cout << window.d_depth_buf[(600-my)*800 + (mx)] << std::endl;
        }

        std::cout << c.getElapsedTime().asMicroseconds() << std::endl;
    }
}

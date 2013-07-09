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

///texturing
///shadowing
///moving models

int main(int argc, char *argv[])
{
    ///remember to make g_arrange_mem run faster!
    objects_container sponza;
    objects_container sponza2;

    sponza.set_file("Sp2/sp2.obj");

    //sponza.set_file("Sp2/player.obj");
    sponza.pos = (cl_float4){0,0,0,0};
    sponza.rot = (cl_float4){0,0,0,0};
    sponza.set_active(true);

    sponza2.set_file("Sp2/boringroom.obj");
    sponza2.pos = (cl_float4){0,0,0,0};
    sponza2.rot = (cl_float4){0,0,0,0};
    //sponza2.set_active(true);

    obj_mem_manager g_manage;

    engine window;
    oclstuff();
    window.load(800,600,1000, "turtles");

    g_manage.g_arrange_mem();
    g_manage.g_changeover();

    sf::Event Event;

    light l;
    l.set_col((cl_float4){1.0, 1.0, 1.0, 0});
    l.set_shadow_bright(1, 1);
    //l.set_pos((cl_float4){0, 1000, 0, 0});
    l.set_pos((cl_float4){-800, 150, -800, 0});
    window.add_light(l);

    l.set_pos((cl_float4){0, 300, 800, 0});
    l.shadow=0;
    window.add_light(l);

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

        std::cout << c.getElapsedTime().asMicroseconds() << std::endl;
    }
}

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

float interpolate_i(float f1, float f2, float f3, int x, int y, int x1, int x2, int x3, int y1, int y2, int y3)
{
    float rconstant=1.0/(x2*y3+x1*(y2-y3)-x3*y2+(x3-x2)*y1);
    float A=((f2*y3+f1*(y2-y3)-f3*y2+(f3-f2)*y1) * rconstant);
    float B=(-(f2*x3+f1*(x2-x3)-f3*x2+(f3-f2)*x1) * rconstant);
    float C=f1-A*x1 - B*y1;

    return (float)(A*x + B*y + C);
}


int main(int argc, char *argv[])
{
    ///the next thing to do is to shrink too-large triangles

    //std::cout << interpolate_i(-10, 20, 30, 25, 6, 15, 40, 50, 0, 34, 45);

    sf::Clock clo;
    //objects_container *sponza=obj_load("Sponza/testspz.obj");
    objects_container *sponza=obj_load("Sp2/sp2.obj");
    //std::cout << clo.getElapsedTime().asMilliseconds() << std::endl;


    obj_mem_manager g_manage;


    engine window;
    oclstuff();
    window.load(800,600,1000, "turtles");




    for(std::vector<object>::iterator it=sponza->objs.begin(); it!=sponza->objs.end(); it++)
    {
        g_manage.obj_list.push_back(&(*it));
    }

    g_manage.g_arrange_mem();





    GLint texSize=0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);


    sf::Event Event;

    light l;
    l.set_pos((cl_float4){175, 135, 815, 0});
    l.set_pos((cl_float4){-800, 150, -800, 0});
    l.set_col((cl_float4){1.0, 1.0, 1.0, 0});
    l.set_shadow_bright(1, 1);
    //for(int i=0; i<10; i++)
    window.add_light(l);

    l.set_pos((cl_float4){0, 300, 800, 0});
    l.shadow=0;
    window.add_light(l);

    l.set_pos((cl_float4){-750, 0, -700, 0});
    l.brightness=0.4;

    //window.add_light(l);


    //sf::Mouse mouse;

    /*while(window.window.isOpen())
    {

        sf::Clock c;

        if(window.window.pollEvent(Event))
        {
            if(Event.type == sf::Event::Closed)
                window.window.close();

        }

        cl_float4 centre = {300.0f, 300.0f, 0.0, 0.0};

        cl_float4 m = {mouse.getPosition(window.window).x, mouse.getPosition(window.window).y, 0.0f, 0.0f};

        if(mouse.isButtonPressed(sf::Mouse::Left))

            std::cout << ret_cubeface(m, centre) << std::endl;



        window.window.display();
        window.window.clear();
    }*/

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
                //std::cout << (float)window.d_depth_buf[my*800 + mx]/UINT_MAX << std::endl;
        }

        //std::cout << c.getElapsedTime().asMilliseconds() << std::endl;


    }



    ///success
    ///have subobjects under objects be grouped texture triangles or some shit


    ///sfml context creation etc in engine





}

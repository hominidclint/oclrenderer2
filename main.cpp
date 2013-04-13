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

float ret_cubeface(cl_float4 point, cl_float4 light)
{
    cl_float4 r_pl;
    r_pl.x=point.x-light.x;
    r_pl.y=point.y-light.y;
    r_pl.z=point.z-light.z;

    float angle = atan2(r_pl.y, r_pl.x);

    if(angle < 0)
    {
        angle = M_PI - fabs(angle) + M_PI;
    }

    angle = angle - M_PI/4.0f;

    if(angle < 0)
    {
        angle = angle + 2.0f*M_PI;
    }

    angle = angle/(2.0f*M_PI);
    angle = angle*4;
    //angle +=0.5;

    int wp1 = floor(angle);




    return wp1;
}




int main(int argc, char *argv[])
{

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
    l.set_pos((cl_float4){-800, 150, -600, 0});
    l.set_col((cl_float4){1.0, 1.0, 1.0, 0});
    l.set_shadow_bright(1, 1);
    int lid=window.add_light(l);

    l.set_pos((cl_float4){0, 300, 800, 0});
    l.shadow=0;
    window.add_light(l);

    l.set_pos((cl_float4){-750, 0, -700, 0});
    l.brightness=0.4;

    window.add_light(l);


    sf::Mouse mouse;

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

        std::cout << c.getElapsedTime().asMilliseconds() << std::endl;


    }



    ///success
    ///have subobjects under objects be grouped texture triangles or some shit


    ///sfml context creation etc in engine





}

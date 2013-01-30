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




int main(int argc, char *argv[])
{

    sf::Clock clo;
    objects_container *sponza=obj_load("Sponza/testspz.obj");
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

    while(window.window.isOpen())
    {

        sf::Clock c;
        if (window.window.pollEvent(Event))
        {
            if (Event.type == sf::Event::Closed)
                window.window.close();

        }


        window.input();

        window.draw_bulk_objs();

        window.render_buffers();

        std::cout << c.getElapsedTime().asMilliseconds() << std::endl;


    }



    ///success
    ///have subobjects under objects be grouped texture triangles or some shit


    ///sfml context creation etc in engine





}

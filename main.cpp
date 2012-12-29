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

    //std::cout << sizeof(obj_g_descriptor);
    sf::Clock clo;
    objects_container *sponza=obj_load("sponza/spz.obj");
    //std::cout << clo.getElapsedTime().asMilliseconds() << std::endl;


    obj_mem_manager g_manage;


    //std::cout << sponza->objs.size();

    //for(std::vector<texture>::iterator it=texture::texturelist.begin(); it!=texture::texturelist.end(); it++)
    //{
        //std::cout << (*it).location << std::endl;
    //} ///textures load in successfully


    //std::cout << sizeof(vertex);

    engine window;
    oclstuff();
    window.load(800,600,1000, "turtles");




    for(std::vector<object>::iterator it=sponza->objs.begin(); it!=sponza->objs.end(); it++)
    {
        // (*it).g_alloc();
        g_manage.obj_list.push_back(&(*it));
    }

    g_manage.g_arrange_mem();


    GLint texSize=0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);

    //std::cout << texSize << std::endl;

    //sf::Keyboard key;
    //sf::Event event;




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

    //system("pause");



    //std::cout << texture::gidc;


    ///success
    ///have subobjects under objects be grouped texture triangles or some shit


    ///sfml context creation etc in engine





}

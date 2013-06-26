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


float interpolate_i(float f1, float f2, float f3, int x, int y, int x1, int x2, int x3, int y1, int y2, int y3, float rconstant)
{
    float A=((f2*y3+f1*(y2-y3)-f3*y2+(f3-f2)*y1) * rconstant);
    float B=(-(f2*x3+f1*(x2-x3)-f3*x2+(f3-f2)*x1) * rconstant);
    float C=f1-A*x1 - B*y1;
    return (float)(A*x + B*y + C);
}

float interpolate_r(float f1, float f2, float f3, int x, int y, int x1, int x2, int x3, int y1, int y2, int y3)
{
    float rconstant=1.0/(x2*y3+x1*(y2-y3)-x3*y2+(x3-x2)*y1);
    return interpolate_i(f1, f2, f3, x, y, x1, x2, x3, y1, y2, y3, rconstant);
}


cl_float4 cross(cl_float4 u, cl_float4 v)
{
    return (cl_float4){u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x, 0};
}

float length(cl_float4 u)
{
    return sqrt(u.x*u.x + u.y*u.y + u.z*u.z);
}

cl_float4 sub(cl_float4 u, cl_float4 v)
{
    return (cl_float4){u.x - v.x, u.y - v.y, u.z - v.z, 0};
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


    /*sf::Mouse mouse;

    cl_float2 a[3] = {{0.1, 0.1}, {0.5, 0.5}, {0.2, 0.7}};
    cl_float4 pos[3] = {{0, 0, 0, 0}, {700, 500, 100, 0}, {20, 400, 35, 0}};

    cl_float4 p1 = {50, 200, 50, 0};

    while(window.window.isOpen())
    {

        sf::Clock c;

        if(window.window.pollEvent(Event))
        {
            if(Event.type == sf::Event::Closed)
                window.window.close();

        }

        cl_float4 centre = {300.0f, 300.0f, 0.0, 0.0};

        cl_float4 m = {mouse.getPosition(window.window).x, mouse.getPosition(window.window).y, 0.0f, 0.0f};

        //cl_float2 p1v, p2v;

        //float nx1 = interpolate_r(a[0].x, a[1].x, a[2].x, m.x, m.y, pos[0].x, pos[1].x, pos[2].x, pos[0].y, pos[1].y, pos[2].y);
        //float ny1 = interpolate_r(a[0].y, a[1].y, a[2].y, m.x, m.y, pos[0].x, pos[1].x, pos[2].x, pos[0].y, pos[1].y, pos[2].y);

        //float nx2 = interpolate_i(vts[0].x, vts[1].x, vts[2].x, p2.x, p2.y, pos[0].x, pos[0].y, pos[1].x, pos[1].y, pos[2].x, pos[2].y);
        //float ny2 = interpolate_i(vts[0].y, vts[1].y, vts[2].y, p2.x, p2.y, pos[0].x, pos[0].y, pos[1].x, pos[1].y, pos[2].x, pos[2].y);

        //p1v = (cl_float2){nx1, ny1};
        //p2v = (cl_float2){nx2, ny2};

        //std::cout << "h " << std::endl << p1v.x << std::endl << p1v.y << std::endl;

        //if(mouse.isButtonPressed(sf::Mouse::Left))

            //std::cout << ret_cubeface(m, centre) << std::endl;




        cl_float4 point = m;

        cl_float4 m1, m2, m3;

        cl_float4 f1, f2, f3;

        float area;
        float a1, a2, a3;

        m1 = pos[0], m2 = pos[1], m3 = pos[2];

        area = length(cross(sub(m1, m2), sub(m1,m3)));



        f1 = sub(m1, point);
        f2 = sub(m2, point);
        f3 = sub(m3, point);

        a1 = length(cross(f2, f3))/area;
        a2 = length(cross(f3, f1))/area;
        a3 = length(cross(f1, f2))/area;

        //p1v = a[0] * a1 + a[1] * a2 + a[2] * a3;
        cl_float2 p1v = (cl_float2){a[0].x * a1 + a[1].x * a2 + a[2].x * a3, a[0].y * a1 + a[1].y * a2 + a[2].y * a3};

        std::cout << "b\n" << p1v.x << std::endl << p1v.y << std::endl;

        //exit(0);



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
            //    std::cout << (float)window.d_depth_buf[(600-my)*800 + (mx)]/UINT_MAX << std::endl;
                //std::cout << window.d_depth_buf[(600-my)*800 + (mx)] << std::endl;
        }

        //std::cout << c.getElapsedTime().asMilliseconds() << std::endl;


    }



    ///success
    ///have subobjects under objects be grouped texture triangles or some shit


    ///sfml context creation etc in engine





}

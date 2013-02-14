#include "engine.hpp"
#include <math.h>
#include <gl/gl3.h>
#include <gl/glext.h>
#include "clstate.h"
#include <iostream>
#include <gl/gl.h>
#include "obj_mem_manager.hpp"
#include <stdio.h>
#include <limits.h>

unsigned int engine::gl_framebuffer_id=0;
unsigned int engine::gl_screen_id=0;

void engine::load(cl_uint pwidth, cl_uint pheight, cl_uint pdepth, std::string name)
{

    width=pwidth;
    height=pheight;
    depth=pdepth;

    window.create(sf::VideoMode(width, height), name.c_str());

    //window.EnableVerticalSync(false);

    cl_uint size = std::max(height, width);
    ///2^x=size;
    g_size=pow(2, ceil(log2(size)));

    //std::cout << g_size << std::endl;

    c_pos.x=0;
    c_pos.y=0;
    c_pos.z=-5000;

    c_rot.x=0;
    c_rot.y=0;
    c_rot.z=0;

    cl_float4 *blank = new cl_float4[g_size*g_size];
    memset(blank, 0, g_size*g_size*sizeof(cl_float4));

    cl_uint *arr = new cl_uint[g_size*g_size];
    memset(arr, UINT_MAX, g_size*g_size*sizeof(cl_uint));






    PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
    PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
    PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffersEXT");
    PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)wglGetProcAddress("glBindRenderbufferEXT");
    PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorageEXT");
    PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbufferEXT");
    //PFNGLBLITFRAMEBUFFEREXTPROC glBlitFramebufferEXT = (PFNGLBLITFRAMEBUFFEREXTPROC)wglGetProcAddress("glBlitFramebufferEXT");
    //PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT");


    glGenRenderbuffersEXT(1, &gl_screen_id);
    glBindRenderbufferEXT(GL_RENDERBUFFER, gl_screen_id);

    glRenderbufferStorageEXT(GL_RENDERBUFFER, GL_RGBA, g_size, g_size); ///find nearest power of two to screen size and use that.






    glGenFramebuffersEXT(1, &gl_framebuffer_id);
    glBindFramebufferEXT(GL_FRAMEBUFFER, gl_framebuffer_id);


    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, gl_screen_id);


    g_screen=clCreateFromGLRenderbuffer(cl::context, CL_MEM_WRITE_ONLY, gl_screen_id, &cl::error);

    if(cl::error!=0)
    {
        std::cout << "Error: CL/GL interop memory alloc in engine.cpp/engine::load" << std::endl;
        exit(cl::error);
    }




    cl_image_format fermat;
    fermat.image_channel_order=CL_RGBA;
    fermat.image_channel_data_type=CL_FLOAT;

    //g_triangle_id_framebuffer=      clCreateImage2D(cl::context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, &fermat, g_size, g_size, 0, NULL, &cl::error);
    //g_triangle_depth_framebuffer=   clCreateImage2D(cl::context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, &fermat, g_size, g_size, 0, NULL, &cl::error);

    if(cl::error!=0)
    {
        std::cout << "image creation (Engine.cpp uvw_coords)" << std::endl;
        exit(cl::error);
    }

    c_pos.x=-800;
    c_pos.y=150;
    c_pos.z=-570;
    ///700

    g_c_pos=clCreateBuffer(cl::context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float4), &c_pos, &cl::error);
    g_c_rot=clCreateBuffer(cl::context, CL_MEM_READ_ONLY, sizeof(cl_float4), NULL, &cl::error);

    depth_buffer=       clCreateBuffer(cl::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint)*g_size*g_size, arr, &cl::error);
    g_id_screen=        clCreateBuffer(cl::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint)*g_size*g_size, arr, &cl::error);
    g_normals_screen=   clCreateBuffer(cl::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_float4)*g_size*g_size, blank, &cl::error);
    g_texture_screen=   clCreateBuffer(cl::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint4)*g_size*g_size, blank, &cl::error);


    //g_depth_screen=clCreateImage2D(cl::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, &fermat, g_size, g_size, 0, blank, &cl::error);





}

cl_float4 rot(double x, double y, double z, cl_float4 rotation)
{

    double i0x=x;
    double i0y=y;
    double i0z=z;

    double i1x=i0x;
    double i1y=i0y*cos(rotation.x) - sin(rotation.x)*i0z;
    double i1z=i0y*sin(rotation.x) + cos(rotation.x)*i0z;


    double i2x=i1x*cos(rotation.y) + i1z*sin(rotation.y);
    double i2y=i1y;
    double i2z=-i1x*sin(rotation.y) + i1z*cos(rotation.y);

    double i3x=i2x*cos(rotation.z) - i2y*sin(rotation.z);
    double i3y=i2x*sin(rotation.z) + i2y*cos(rotation.z);
    double i3z=i2z;

    cl_float4 ret;

    ret.x=i3x;
    ret.y=i3y;
    ret.z=i3z;

    return ret;


}

void engine::input()
{
        sf::Keyboard keyboard;



        //double frametime=window.getframetime(); ///function no longer exists
        //window.

        static int distance_multiplier=1;

        if(keyboard.isKeyPressed(sf::Keyboard::LShift))
        {
            distance_multiplier=10;
        }
        else
        {
            distance_multiplier=1;
        }

        double distance=0.04*distance_multiplier*30;

        if(keyboard.isKeyPressed(sf::Keyboard::W))  ///get engine to do this itself. Engine.ProcessInput
        {
            cl_float4 t=rot(0, 0, distance, c_rot);
            c_pos.x+=t.x;
            c_pos.y+=t.y;
            c_pos.z+=t.z;
        }
        if(keyboard.isKeyPressed(sf::Keyboard::S))
        {
            cl_float4 t=rot(0, 0, -distance, c_rot);
            c_pos.x+=t.x;
            c_pos.y+=t.y;
            c_pos.z+=t.z;
        }
        if(keyboard.isKeyPressed(sf::Keyboard::A))
        {
            //c_pos.x-=0.04*frametime*mult;
            cl_float4 t=rot(-distance, 0, 0, c_rot);
            c_pos.x+=t.x;
            c_pos.y+=t.y;
            c_pos.z+=t.z;
        }
        if(keyboard.isKeyPressed(sf::Keyboard::D))
        {
            cl_float4 t=rot(distance, 0, 0, c_rot);
            c_pos.x+=t.x;
            c_pos.y+=t.y;
            c_pos.z+=t.z;
        }
        if(keyboard.isKeyPressed(sf::Keyboard::E))
        {
            c_pos.y-=0.04*distance_multiplier*30;
        }
        if(keyboard.isKeyPressed(sf::Keyboard::Q))
        {
            c_pos.y+=0.04*distance_multiplier*30;
        }

        if(keyboard.isKeyPressed(sf::Keyboard::Left))
        {
            c_rot.y-=0.001*30;
        }
        if(keyboard.isKeyPressed(sf::Keyboard::Right))
        {
            c_rot.y+=0.001*30;
        }
        if(keyboard.isKeyPressed(sf::Keyboard::Up))
        {
            c_rot.x-=0.001*30;
        }
        if(keyboard.isKeyPressed(sf::Keyboard::Down))
        {
            c_rot.x+=0.001*30;
        }

        if(keyboard.isKeyPressed(sf::Keyboard::Escape))
        {
            window.close();
        }

        if(keyboard.isKeyPressed(sf::Keyboard::B))
        {
            std::cout << "rerr: " << c_pos.x << " " << c_pos.y << " " << c_pos.z << std::endl;
        }

        if(keyboard.isKeyPressed(sf::Keyboard::V))
        {
            //std::cout << frametime << std::endl;
        }


        clEnqueueWriteBuffer(cl::cqueue, g_c_pos, true, 0, sizeof(cl_float4), &c_pos, 0, NULL, NULL);
        clEnqueueWriteBuffer(cl::cqueue, g_c_rot, true, 0, sizeof(cl_float4), &c_rot, 0, NULL, NULL);



}




/*void engine::draw_tile_objs()
{
    ///rotate triangles
    ///for every block, split into half and sort triangles
    ///



}*/


void run_kernel_with_args(cl_kernel &kernel, cl_uint *global_ws, cl_uint *local_ws, int d, cl_mem **argv, int argc)
{

    for(int i=0; i<argc; i++)
    {
        cl::error |= clSetKernelArg(kernel, i, sizeof(cl_mem), argv[i]);
        if(cl::error!=0)
        {
            std::cout << "Error in kernel setargs " << i << " " << cl::error << std::endl;
            exit(1);
        }
    }


    //size_t local=*local_ws;
    //size_t local=*local_ws;

    /*if(*global_ws % local!=0)
    {
        int rem=global_ws % local;
        *global_ws-=(rem);
        *global_ws+=local;
    }*/

    cl::error = clEnqueueNDRangeKernel(cl::cqueue, kernel, d, NULL, global_ws, local_ws, 0, NULL, NULL);
    clFinish(cl::cqueue);

    //std::cout << "c1 " << c.getElapsedTime().asMilliseconds() << std::endl;

    if(cl::error!=0){
        std::cout << "Error In kernel 1" << std::endl;
        exit(cl::error);
    }



}


void engine::draw_bulk_objs_n()
{
    int p0=0;

    glFinish();
    clEnqueueAcquireGLObjects(cl::cqueue, 1, &g_screen, 0, NULL, NULL);
    clFinish(cl::cqueue);

    cl_uint p1global_ws = obj_mem_manager::tri_num;
    cl_uint local=256;

    if(p1global_ws % local!=0)
    {
        int rem=p1global_ws % local;
        p1global_ws-=(rem);
        p1global_ws+=local;
    }

    clEnqueueWriteBuffer(cl::cqueue, obj_mem_manager::g_tri_anum, CL_TRUE, 0, sizeof(cl_uint), &p0, 0, NULL, NULL);
    cl_mem *p1arglist[]={&obj_mem_manager::g_tri_mem, &obj_mem_manager::g_tri_smem, &obj_mem_manager::g_tri_num, &obj_mem_manager::g_tri_anum, &g_c_pos, &g_c_rot,  &depth_buffer};
    run_kernel_with_args(cl::kernel, &p1global_ws, &local, 1, p1arglist, 7);


    int atom_count=0;

    clEnqueueReadBuffer(cl::cqueue, obj_mem_manager::g_tri_anum, CL_TRUE, 0, sizeof(cl_uint), &atom_count, 0, NULL, NULL);



    cl_uint p2global_ws=atom_count;
    cl_uint local2=256;
    if(p2global_ws % local2!=0)
    {
        int rem=p2global_ws % local2;
        p2global_ws-=(rem);
        p2global_ws+=local2;
    }

    //std::cout << p1global_ws << " " << atom_count << std::endl;

    cl_mem *p2arglist[]={&obj_mem_manager::g_tri_smem, &obj_mem_manager::g_tri_anum, &depth_buffer, &g_id_screen};
    run_kernel_with_args(cl::kernel2, &p2global_ws, &local2, 1, p2arglist, 4);


    cl_uint p3global_ws[]={g_size, g_size};
    cl_uint p3local_ws[]={32, 32};

    cl_mem *p3arglist[]={&obj_mem_manager::g_tri_mem, &obj_mem_manager::g_tri_smem, &obj_mem_manager::g_tri_num, &obj_mem_manager::g_tri_anum, &depth_buffer, &g_id_screen, &obj_mem_manager::g_texture_array, &g_screen};
    run_kernel_with_args(cl::kernel3, p3global_ws, p3local_ws, 2, p3arglist, 8);



    clEnqueueReleaseGLObjects(cl::cqueue, 1, &g_screen, 0, NULL, NULL);
    clFinish(cl::cqueue);
    glFinish();
}

void engine::draw_bulk_objs()
{

    glFinish();
    clEnqueueAcquireGLObjects(cl::cqueue, 1, &g_screen, 0, NULL, NULL);
    clFinish(cl::cqueue);




    cl::error |= clSetKernelArg(cl::kernel, 0, sizeof(cl_mem), &obj_mem_manager::g_tri_mem);
    //cl::error |= clSetKernelArg(cl::kernel, 1, sizeof(cl_mem), &g_screen);
    cl::error |= clSetKernelArg(cl::kernel, 1, sizeof(cl_mem), &g_c_pos);
    cl::error |= clSetKernelArg(cl::kernel, 2, sizeof(cl_mem), &g_c_rot);
    cl::error |= clSetKernelArg(cl::kernel, 3, sizeof(cl_mem), &obj_mem_manager::g_tri_num);
    cl::error |= clSetKernelArg(cl::kernel, 4, sizeof(cl_mem), &depth_buffer);
    cl::error |= clSetKernelArg(cl::kernel, 5, sizeof(cl_mem), &g_normals_screen);
    cl::error |= clSetKernelArg(cl::kernel, 6, sizeof(cl_mem), &g_id_screen);
    cl::error |= clSetKernelArg(cl::kernel, 7, sizeof(cl_mem), &g_texture_screen);
    cl::error |= clSetKernelArg(cl::kernel, 8, sizeof(cl_mem), &obj_mem_manager::g_obj_desc);
    cl::error |= clSetKernelArg(cl::kernel, 9, sizeof(cl_mem), &obj_mem_manager::g_obj_num);

    ///pass obj descriptor and size to kernel
    //cl::error |= clSetKernelArg(cl::kernel, 7, sizeof(cl_mem), &g_screen);

    /*cl_uint *idb=new cl_uint[1024*1024];

    clEnqueueReadBuffer(cl::cqueue, g_id_screen, true, 0, sizeof(cl_uint)*1024*1024, idb, 0, NULL, NULL);

    for(int i=0; i<800; i++)
    {
        for(int j=0; j<600; j++)
        {
            if(idb[j*1024 + i]!=4294967295)
                std::cout << idb[j*1024 + i] << std::endl;
        }

    }*/ //id buffer is fucked

    //sf::Clock t;

    size_t global_ws = obj_mem_manager::tri_num;



    int global_mult=1;

    size_t local=(128*global_mult);

    if(global_ws % local!=0)
    {
        int rem=global_ws % local;

        global_ws-=(rem);
        global_ws+=local;

    }



    //exit(1);


    sf::Clock c;
    cl::error = clEnqueueNDRangeKernel(cl::cqueue, cl::kernel, 1, NULL, &global_ws, &local, 0, NULL, NULL);
    clFinish(cl::cqueue);

    //std::cout << "c1 " << c.getElapsedTime().asMilliseconds() << std::endl;

    if(cl::error!=0){
        std::cout << "Error In kernel 1" << std::endl;
        exit(cl::error);
    }




    size_t work_dim[2]={g_size, g_size};

    size_t local_r[2]={32, 32};

    //cl::error |= clSetKernelArg(cl::kernel2, 0, sizeof(cl_mem), &obj_mem_manager::g_tri_mem);
    cl::error |= clSetKernelArg(cl::kernel2, 0, sizeof(cl_mem), &obj_mem_manager::g_tri_mem);
    cl::error |= clSetKernelArg(cl::kernel2, 1, sizeof(cl_mem), &obj_mem_manager::g_tri_num);
    cl::error |= clSetKernelArg(cl::kernel2, 2, sizeof(cl_mem), &g_c_pos);
    cl::error |= clSetKernelArg(cl::kernel2, 3, sizeof(cl_mem), &g_c_rot);
    cl::error |= clSetKernelArg(cl::kernel2, 4, sizeof(cl_mem), &depth_buffer);
    cl::error |= clSetKernelArg(cl::kernel2, 5, sizeof(cl_mem), &g_id_screen);
    cl::error |= clSetKernelArg(cl::kernel2, 6, sizeof(cl_mem), &g_normals_screen);
    cl::error |= clSetKernelArg(cl::kernel2, 7, sizeof(cl_mem), &g_texture_screen);
    cl::error |= clSetKernelArg(cl::kernel2, 8, sizeof(cl_mem), &obj_mem_manager::g_obj_desc);
    cl::error |= clSetKernelArg(cl::kernel2, 9, sizeof(cl_mem), &obj_mem_manager::g_obj_num);
    cl::error |= clSetKernelArg(cl::kernel2, 10, sizeof(cl_mem), &g_screen);
    cl::error |= clSetKernelArg(cl::kernel2, 11, sizeof(cl_mem),&obj_mem_manager::g_texture_nums);
    cl::error |= clSetKernelArg(cl::kernel2, 12, sizeof(cl_mem),&obj_mem_manager::g_texture_sizes);
    cl::error |= clSetKernelArg(cl::kernel2, 13, sizeof(cl_mem),&obj_mem_manager::g_texture_array);
    //cl::error |= clSetKernelArg(cl::kernel2, 10, sizeof(cl_mem), &obj_mem_manager::i256);
    //cl::error |= clSetKernelArg(cl::kernel2, 11, sizeof(cl_mem), &obj_mem_manager::i512);
    //cl::error |= clSetKernelArg(cl::kernel2, 12, sizeof(cl_mem), &obj_mem_manager::i1024);
    //cl::error |= clSetKernelArg(cl::kernel2, 13, sizeof(cl_mem), &obj_mem_manager::i2048);

    c.restart();

    cl::error = clEnqueueNDRangeKernel(cl::cqueue, cl::kernel2, 2, NULL, work_dim, local_r, 0, NULL, NULL);
    clFinish(cl::cqueue);
    //std::cout << "c2 " << c.getElapsedTime().asMilliseconds() << std::endl;
    if(cl::error!=0){
        std::cout << "Error In kernel 2" << std::endl;
        exit(cl::error);
    }



    clEnqueueReleaseGLObjects(cl::cqueue, 1, &g_screen, 0, NULL, NULL);
    clFinish(cl::cqueue);
    glFinish();
    //std::cout << "t " <<  t.getElapsedTime().asMilliseconds() << std::endl;


}



void engine::render_buffers()
{



    PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");

    PFNGLBLITFRAMEBUFFEREXTPROC glBlitFramebufferEXT = (PFNGLBLITFRAMEBUFFEREXTPROC)wglGetProcAddress("glBlitFramebufferEXT");

    glBindFramebufferEXT(GL_READ_FRAMEBUFFER, gl_framebuffer_id);
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, 0);


    glBlitFramebufferEXT(0 , 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);


    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, gl_framebuffer_id);
    glClear(GL_COLOR_BUFFER_BIT);

    window.display();
    window.clear();



}




/*void engine::draw_poor_objs(objects_container &container)
{

    glFinish();
    clEnqueueAcquireGLObjects(cl::cqueue, 1, &g_screen, 0, NULL, NULL);
    clFinish(cl::cqueue);


    for(std::vector<object>::iterator it=container.objs.begin(); it!=container.objs.end(); it++)
    {
        draw_obj((*it));
    }


    clEnqueueReleaseGLObjects(cl::cqueue, 1, &g_screen, 0, NULL, NULL);
    clFinish(cl::cqueue);
    glFinish();


}*/

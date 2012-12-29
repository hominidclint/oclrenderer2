#include "obj_mem_manager.hpp"
#include <CL/CL.h>
#include "clstate.h"
#include "triangle.hpp"
#include <iostream>
#include <windows.h>
#include <algorithm>
#include "obj_g_descriptor.hpp"
#include "texture.hpp"

std::vector<object*> obj_mem_manager::obj_list;

cl_uint obj_mem_manager::tri_num;

cl_mem obj_mem_manager::g_tri_mem;
cl_mem obj_mem_manager::g_tri_num;
cl_mem obj_mem_manager::g_tri_fstorage;
cl_mem obj_mem_manager::g_obj_desc;
cl_mem obj_mem_manager::g_obj_num;

cl_mem obj_mem_manager::i256;
cl_mem obj_mem_manager::i512;
cl_mem obj_mem_manager::i1024;
cl_mem obj_mem_manager::i2048;



int num_to_divide(int target, int tsize)
{
    int f=0;

    while(tsize!=target)
    {
        tsize/=target;
        f++;
    }
    return f;

}

cl_uchar4 * gen_3d(int tstart, int tnum, int size)
{
    texture *c_tex=&texture::texturelist[tstart];

    cl_uchar4 *r=(cl_uchar4*)malloc(size*size*tnum*4);


    for(int n=0; n<tnum; n++)
    {
        std::cout << c_tex[n].location << std::endl;
        for(int i=0; i<size; i++)
        {
            for(int j=0; j<size; j++)
            {
                sf::Color c=c_tex[n].c_image.getPixel(i, j);
                r[i + j*size + n*size*size].x=c.r;
                r[i + j*size + n*size*size].y=c.g;
                r[i + j*size + n*size*size].z=c.b;
            }
        }

    }
    return r;

}


void obj_mem_manager::g_arrange_mem()//arrange textures here and update texture ids
{

    cl_uint trianglecount=0;

    obj_g_descriptor *desc=new obj_g_descriptor[obj_list.size()];
    unsigned int n=0;


    std::sort(texture::texturelist.begin(), texture::texturelist.end(), texture::t_compare);

    //texture::generate_mipmaps();

    /*for(std::vector<texture>::iterator it=texture::texturelist.begin(); it!=texture::texturelist.end(); it++)
    {
        std::cout << (*it).id << " ";
    }

    std::cout << "hi" << std::endl;*/


    for(std::vector<object*>::iterator it=obj_list.begin(); it!=obj_list.end(); it++)
    {
        desc[n].tri_num=(*it)->tri_num;
        desc[n].start=trianglecount;
        desc[n].tid=(*it)->tid;
        //std::cout << (*it)->tid << " ";
        desc[n].world_pos=(*it)->pos;
        desc[n].world_rot=(*it)->rot;

        trianglecount+=(*it)->tri_num;
        n++;
    }

    //std::cout << std::endl;

    bool *terrible = new bool[n];
    memset(terrible, false, sizeof(bool)*n);

    int icount=0;

    for(std::vector<texture>::iterator it=texture::texturelist.begin(); it!=texture::texturelist.end(); it++)
    {
        for(unsigned int i=0; i<n; i++)
        {
            if(desc[i].tid==(*it).id && terrible[i]!=true)
            {
                desc[i].tid=icount;
                terrible[i]=true;
            }
        }
        icount++;
    }

    delete [] terrible;

    /*for(int i=0; i<n; i++)
    {
        std::cout << desc[i].tid << " ";
    }*/

    int tcount2=0;
    for(std::vector<texture>::iterator it=texture::texturelist.begin(); it!=texture::texturelist.end(); it++)
    {
        (*it).id=tcount2;
        tcount2++;
    }

    int tcount=0;
    for(std::vector<object*>::iterator it=obj_list.begin(); it!=obj_list.end(); it++)
    {
        (*it)->tid=desc[tcount].tid;
        tcount++;
    } ///going to assume desc is right for the moment ///pretty sure its correct

    int i2=0, i5=0, i10=0, i20=0;
    std::cout << "hi" << std::endl;
    for(unsigned int i=0; i<n; i++)
    {
        desc[i].size=texture::texturelist[desc[i].tid].c_image.getSize().x;

        //std::cout << desc[i].tid << std::endl << desc[i].size << std::endl << "n " << std::endl;
        //sf::Sleep(400);
        /*if(desc[i].size==256)
        {
            i2++;
        }
        if(desc[i].size==512)
        {
            i5++;
        }
        if(desc[i].size==1024)
        {
            i10++;
        }
        if(desc[i].size==2048)
        {
            i20++;
        }*/
    }
    for(unsigned int i=0; i<texture::texturelist.size(); i++)
    {
        //std::cout << texture::texturelist[i].c_image.GetWidth() << std::endl;

        if(texture::texturelist[i].c_image.getSize().x==256)
        {
            i2++;
        }if(texture::texturelist[i].c_image.getSize().x==512)
        {
            i5++;
        }if(texture::texturelist[i].c_image.getSize().x==1024)
        {
            i10++;
        }if(texture::texturelist[i].c_image.getSize().x==2048)
        {
            i20++;
        }
    }

    ///if textures.arechanged or some shit, but anyway

    //int i2048depth=2;

    /*cl_uint4*** d2048 = new cl_uint4**[i2048depth];

    for(int i=0; i<i2048depth; i++)
    {
        d2048[i]=new cl_uint4*[2048];
        for(int j=0; j<2048; j++)
        {
            d2048[i][j]=new cl_uint4[2048];
        }

    }

    for(int i=0; i<i2048depth; i++)
    {
        //cl_uint4 **pp=(cl_uint4**)texture::texturelist[i].c_image.GetPixelsPtr();
        //d2048[i]=new cl_uint4*[2048];
        for(int j=0; j<texture::texturelist[desc[i].tid].c_image.GetWidth(); j++)
        {
            //d2048[i][j]=new cl_uint4[2048];
            for(int n=0; n<texture::texturelist[desc[i].tid].c_image.GetWidth(); n++)
            {
                sf::Color col;
                col=texture::texturelist[desc[i].tid].c_image.GetPixel(j, n);
                d2048[i][j][n].x=col.r; ///whoops
                d2048[i][j][n].y=col.g; ///whoops
                d2048[i][j][n].z=col.b; ///whoops
            }
        }
    }*/



    /*cl_mem initTexture3D(uchar *h_volume, const size_t volumeSize[3])
    {
        cl_int ciErrNum;

        // create 3D array and copy data to device
        cl_image_format volume_format;
        volume_format.image_channel_order = CL_RGBA;
        volume_format.image_channel_data_type = CL_UNORM_INT8;
        uchar* h_tempVolume =(uchar*)malloc(volumeSize[0] * volumeSize[1] * volumeSize[2] * 4);
        for(unsigned int i = 0; i<(volumeSize[0] * volumeSize[1] * volumeSize[2]); i++)
        {
            h_tempVolume[4 * i] = h_volume[i];
        }
        cl_mem d_volume = clCreateImage3D(cxGPUContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &volume_format,
                                            volumeSize[0], volumeSize[1], volumeSize[2],
                                            (volumeSize[0] * 4), (volumeSize[0] * volumeSize[1] * 4),
                                            h_tempVolume, &ciErrNum);
        oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
        free (h_tempVolume);
        return d_volume;
    }*/

    //cl_uchar4* d2048=(cl_uchar4*)malloc(2048*2048*2*4);

    /*for(int n=0; n<i2048depth; n++)
    {
        for(int i=0; i<texture::texturelist[n].c_image.GetWidth(); i++)
        {
            for(int j=0; j<texture::texturelist[n].c_image.GetHeight(); j++)
            {
                sf::Color col=texture::texturelist[n].c_image.GetPixel(i, j);

                d2048[i + j*2048 + n*2048*2048].x=col.r;
                d2048[i + j*2048 + n*2048*2048].y=col.g;
                d2048[i + j*2048 + n*2048*2048].z=col.b;
            }
        }
        //std::cout << texture::texturelist[n].location << std::endl;
        //break;
    }*/

    //cl_uchar4 *d2048=gen_3d(1, 2, 256);
    std::cout << texture::texturelist.size() << std::endl;
    cl_uchar4* d256= gen_3d(0,           i2,     256);
    cl_uchar4* d512= gen_3d(i2,          i5,     512);
    cl_uchar4* d1024=gen_3d(i5+i2,       i10,    1024);
    cl_uchar4* d2048=gen_3d(i5+i2+i10,   i20,    2048);



    ///right, we are going to load
    cl_image_format fermat;
    fermat.image_channel_order=CL_RGBA;
    fermat.image_channel_data_type=CL_UNSIGNED_INT8;

    //i2048=clCreateImage2D(cl::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, &fermat, texture::texturelist[1].c_image.GetWidth(), texture::texturelist[1].c_image.GetWidth(), 0, (void*)texture::texturelist[1].c_image.GetPixelsPtr(), &cl::error);
    //

    //i2048=clCreateImage3D(cl::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, &fermat, texture::texturelist[1].c_image.GetWidth(), texture::texturelist[1].c_image.GetWidth(), i2048depth, 0,0, d2048, &cl::error);
    i256=clCreateImage3D(cl::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, &fermat, 256, 256, i2, 256*4, (256*256*4), d256, &cl::error);
    i512=clCreateImage3D(cl::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, &fermat, 512, 512, i5, 512*4, (512*512*4), d512, &cl::error);
    i1024=clCreateImage3D(cl::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, &fermat, 1024, 1024, i10, 1024*4, (1024*1024*4), d1024, &cl::error);
    i2048=clCreateImage3D(cl::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, &fermat, 2048, 2048, i20, 2048*4, (2048*2048*4), d2048, &cl::error);

    //std::cout << texture::texturelist[1].location << std::endl;




    ///now, we need to lump texture sizes into catagories

    /*int *ar=new int[n];

    for(int i=0; i<texture::texturelist.size(); i++)
    {
        int size=texture::texturelist[i].c_image.GetWidth();
        ar[i]=size;
    }*/
    ///so, now we have an array of sizes;





    /*for(unsigned int i=0; i<n; i++)
    {
        std::cout << desc[i].start << std::endl;
    }*/

    //std::cout << sizeof (obj_g_descriptor) << std::endl;

    clReleaseMemObject(g_obj_desc);
    clReleaseMemObject(g_obj_num);

    cl_mem g_obj_d=  clCreateBuffer(cl::context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(obj_g_descriptor)*n, desc, &cl::error);
    cl_mem g_obj_n=  clCreateBuffer(cl::context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint),              &n, &cl::error);

    g_obj_desc=g_obj_d;
    g_obj_num=g_obj_n;


    delete [] desc;


    //std::cout << trianglecount << std::endl;

    clReleaseMemObject(g_tri_mem);
    clReleaseMemObject(g_tri_num);
    g_tri_mem = clCreateBuffer(cl::context, CL_MEM_READ_ONLY, sizeof(triangle)*trianglecount, NULL, &cl::error);
    g_tri_fstorage = clCreateBuffer(cl::context, CL_MEM_READ_WRITE, sizeof(cl_float4)*trianglecount*3, NULL, &cl::error);

    if(cl::error!=0)
    {
        std::cout << "g_tri_mem create" << std::endl;
        exit(cl::error);
    }

    g_tri_num = clCreateBuffer(cl::context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR , sizeof(cl_uint), &trianglecount, &cl::error);

    if(cl::error!=0)
    {
        std::cout << "g_tri_num create" << std::endl;
        exit(cl::error);
    }
    //std::cout << trianglecount << std::endl;
    std::cout << trianglecount << std::endl;

    cl_uint running=0;

    for(std::vector<object*>::iterator it=obj_list.begin(); it!=obj_list.end(); it++)
    {

        clEnqueueWriteBuffer(cl::cqueue, g_tri_mem, CL_TRUE, sizeof(triangle)*running, sizeof(triangle)*(*it)->tri_num, (*it)->tri_list, 0, NULL, NULL);
        running+=(*it)->tri_num;


    }

    //std::cout << running << std::endl;

    tri_num=trianglecount;

    clFinish(cl::cqueue);





}

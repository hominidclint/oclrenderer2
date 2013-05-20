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
cl_mem obj_mem_manager::g_tri_smem;
cl_mem obj_mem_manager::g_tri_num;
cl_mem obj_mem_manager::g_tri_anum;
cl_mem obj_mem_manager::g_tri_fstorage;
cl_mem obj_mem_manager::g_obj_desc;
cl_mem obj_mem_manager::g_obj_num;
cl_mem obj_mem_manager::g_light_mem;
cl_mem obj_mem_manager::g_light_num;
cl_mem obj_mem_manager::g_light_buf;


cl_mem obj_mem_manager::g_texture_array;
cl_mem obj_mem_manager::g_texture_sizes;
cl_mem obj_mem_manager::g_texture_nums;

cl_uchar4* obj_mem_manager::c_texture_array;

struct texture_array_descriptor
{

    std::vector<int> texture_nums;
    std::vector<int> texture_sizes;

} tad;

texture_array_descriptor obj_mem_manager::tdescrip;


//cl_uint max_tex_size=2048;

cl_uint return_max_num(int size)
{

    return (max_tex_size/size) *(max_tex_size/size);

}


cl_uchar4 * return_first_free(int size, int &num) ///texture ids need to be embedded in texture
{
    texture_array_descriptor *T=&obj_mem_manager::tdescrip;

    int maxnum=return_max_num(size);

    //std::cout << "mnum " << maxnum << std::endl << "size: " << size << std::endl;


    for(unsigned int i=0; i<obj_mem_manager::tdescrip.texture_nums.size(); i++)
    {

        if(T->texture_nums[i] < maxnum && T->texture_sizes[i]==size)
        {
            ///so, T->texture_nums[i] is the position of the new element to return
            num=i;
            //return &obj_mem_manager::c_texture_array[0*1 + 0*max_tex_size + T->texture_nums[i]*size*size + i*max_tex_size*max_tex_size]; ///so, i is the layer, T->texture_nums[i]*size*size is the texture number
            ///this is wrong
            /*int slicesize=i*max_tex_size*max_tex_size; ///slice bits

            int ialong=T->texture_nums[i] % (max_tex_size/size);
            ialong*=size;
            int jalong=T->texture_nums[i] / (max_tex_size/size);
            std::cout << "j" << jalong << std::endl;
            jalong*=max_tex_size;

            return &obj_mem_manager::c_texture_array[slicesize + ialong + jalong];*/

            return &obj_mem_manager::c_texture_array[i*max_tex_size*max_tex_size];
        }


    }

    ///we didn't find a suitable texture array, which means create a new one! Realloc array and return pointer, as well as update both new buffers. That means all we have to do now is actually write the textures

    int length=T->texture_nums.size();
    length++;

    cl_uchar4 *newarray=new cl_uchar4[max_tex_size*max_tex_size*length];

    memcpy(newarray, obj_mem_manager::c_texture_array, sizeof(cl_uchar4)*max_tex_size*max_tex_size*(length-1));

    delete [] obj_mem_manager::c_texture_array;
    obj_mem_manager::c_texture_array=newarray;

    T->texture_sizes.push_back(size);
    T->texture_nums.push_back(0);


    return return_first_free(size, num);



}


void setpixel(cl_uchar4 *buf, sf::Color &col, int x, int y, int lx, int ly)
{
    buf[x + y*lx].x=col.r;
    buf[x + y*lx].y=col.g;
    buf[x + y*lx].z=col.b;
}

sf::Color pixel4(sf::Color &p0, sf::Color &p1, sf::Color &p2, sf::Color &p3)
{
    sf::Color ret;
    ret.r=(float)(p0.r + p1.r + p2.r + p3.r)/4.0;
    ret.g=(float)(p0.g + p1.g + p2.g + p3.g)/4.0;
    ret.b=(float)(p0.b + p1.b + p2.b + p3.b)/4.0;
    //ret.w=0;

    return ret;
}

void gen_miplevel(texture &tex, texture &gen, int level) ///call from main mem_alloc func?
{
    int size=tex.c_image.getSize().x;
    int newsize=size >> level;

    gen.c_image.create(newsize, newsize);

    for(int i=0; i<newsize; i++)
    {
        for(int j=0; j<newsize; j++)
        {
            sf::Color p4[4];
            p4[0]=tex.c_image.getPixel(i*2, j*2);
            p4[1]=tex.c_image.getPixel(i*2+1, j*2);
            p4[2]=tex.c_image.getPixel(i*2, j*2+1);
            p4[3]=tex.c_image.getPixel(i*2+1, j*2+1);
            sf::Color m=pixel4(p4[0], p4[1], p4[2], p4[3]);
            gen.c_image.setPixel(i, j, m);
        }
    }

}


void add_texture(texture &tex, int &newid)
{
    int size=tex.c_image.getSize().x;
    int num=0;
    cl_uchar4 *firstfree=return_first_free(size, num);


    sf::Image *T=&tex.c_image;
    texture_array_descriptor *Td=&obj_mem_manager::tdescrip;

    int which = Td->texture_nums[num];
    int blockalongy = which / (max_tex_size/size);
    int blockalongx = which % (max_tex_size/size);

    int ti=0, tj=0;

    for(int i=blockalongx*size; i<(blockalongx+1)*size; i++)
    {
        for(int j=blockalongy*size; j<(blockalongy+1)*size; j++)
        {
            sf::Color c=T->getPixel(ti, tj);
            setpixel(firstfree, c, i, j, max_tex_size, max_tex_size);
            tj++;
        }

        ti++;
        tj=0;

    }


    ///so, num represents which slice its in
    ///Td->texture_nums[i] represents which position it is in within the slice;

    int mod=(num << 16) | Td->texture_nums[num];
    newid=mod;
    ///so, now the upper half represents which slice its in, and the lower half, the number within the slice

    obj_mem_manager::tdescrip.texture_nums[num]++;



}


void add_texture_and_mipmaps(texture &tex, cl_uint4 &newmips, int &newid)
{
    //std::cout << "hi" << std::endl;
    add_texture(tex, newid);
    //std::cout << "hello" << std::endl;

    texture mip[MIP_LEVELS];

    for(int n=0; n<MIP_LEVELS; n++)
    {
        if(n==0)
            gen_miplevel(tex, mip[n], 1);
        else
            gen_miplevel(mip[n-1], mip[n], 1);

        mip[n].init();
        add_texture(mip[n], ((int*)&newmips)[n]); ///totally legit
    }
}


/*void gen_tile_textures()
{
    for(std::vector<texture>::iterator it=texture::texturelist.begin(); it!=texture::texturelist.end(); it++)
    {
        sf::Image *T=&(*it).c_image;

        //add_texture(T->getSize().x, *T);
    }
}*/




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

/*cl_uchar4 * gen_3d(int tstart, int tnum, int size)
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

}*/


void obj_mem_manager::g_arrange_mem()//arrange textures here and update texture ids
{

    ///http://kayru.org/articles/dssdo/

    ///texture::generate_mipmaps()



    cl_uint trianglecount=0;

    obj_g_descriptor *desc=new obj_g_descriptor[obj_list.size()];
    unsigned int n=0;


    std::vector<int> newtexid;

    int osize=texture::texturelist.size();




    std::vector<int> mtexids;

    //exit(0);
    for(int i=0; i<osize; i++)
    {
        int t=0;
        //add_texture(texture::texturelist[i], t);
        cl_uint4 mipmaps;
        //std::cout << "hello" << std::endl;
        add_texture_and_mipmaps(texture::texturelist[i], mipmaps, t); ///is trying to push new mipmaps to texturelist, while also iterating through texture list
        //std::cout << "hi" << std::endl;
        ///in essence, uh oh
        ///textures are only added to the end, existing textures already.. well, exist. So, we only iterate through the ones
        ///that previously existed
        newtexid.push_back(t);
        mtexids.push_back(mipmaps.x);
        mtexids.push_back(mipmaps.y);
        mtexids.push_back(mipmaps.z);
        mtexids.push_back(mipmaps.w);

    }


    int mipbegin=newtexid.size();

    for(unsigned int i=0; i<mtexids.size(); i++)
    {
        newtexid.push_back(mtexids[i]);
    }



    for(std::vector<object*>::iterator it=obj_list.begin(); it!=obj_list.end(); it++) ///if you call this more than once, it will break. Need to store how much it has already done, and start it again from there to prevent issues with mipmaps
    {
        desc[n].tri_num=(*it)->tri_num;
        desc[n].start=trianglecount;
        desc[n].tid=(*it)->tid;

        for(int i=0; i<MIP_LEVELS; i++)
        {
            desc[n].mip_level_ids[i]=texture::texturelist[mipbegin + desc[n].tid*MIP_LEVELS + i].id;
        }

        desc[n].world_pos=(*it)->pos;
        desc[n].world_rot=(*it)->rot;

        trianglecount+=(*it)->tri_num;
        n++;
    }


    clReleaseMemObject(g_texture_sizes);
    clReleaseMemObject(g_texture_nums);


    g_texture_sizes  =  clCreateBuffer(cl::context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int)*obj_mem_manager::tdescrip.texture_sizes.size(), obj_mem_manager::tdescrip.texture_sizes.data(), &cl::error);
    g_texture_nums   =  clCreateBuffer(cl::context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,                                sizeof(int)*newtexid.size(),                                newtexid.data(), &cl::error);

    clReleaseMemObject(g_texture_array);

    cl_image_format fermat;
    fermat.image_channel_order=CL_RGBA;
    fermat.image_channel_data_type=CL_UNSIGNED_INT8;

    g_texture_array=clCreateImage3D(cl::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, &fermat, 2048, 2048, obj_mem_manager::tdescrip.texture_sizes.size(), 2048*4, (2048*2048*4), obj_mem_manager::c_texture_array, &cl::error);



    ///now, we need to lump texture sizes into catagories



    clReleaseMemObject(g_obj_desc);
    clReleaseMemObject(g_obj_num);

    cl_mem g_obj_d  =  clCreateBuffer(cl::context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(obj_g_descriptor)*n, desc, &cl::error);
    cl_mem g_obj_n  =  clCreateBuffer(cl::context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint),              &n, &cl::error);

    g_obj_desc =  g_obj_d;
    g_obj_num  =  g_obj_n;


    delete [] desc;


    clReleaseMemObject(g_tri_mem); ///allocate triangle buffers and number buffer
    clReleaseMemObject(g_tri_num);
    g_tri_mem = clCreateBuffer(cl::context, CL_MEM_READ_ONLY, sizeof(triangle)*trianglecount, NULL, &cl::error);
    g_tri_smem = clCreateBuffer(cl::context, CL_MEM_READ_WRITE, sizeof(triangle)*trianglecount, NULL, &cl::error);
    g_tri_fstorage = clCreateBuffer(cl::context, CL_MEM_READ_WRITE, sizeof(cl_float4)*trianglecount*3, NULL, &cl::error);

    if(cl::error!=0)
    {
        std::cout << "g_tri_mem create" << std::endl;
        exit(cl::error);
    }

    int p0=0;

    g_tri_num = clCreateBuffer(cl::context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR , sizeof(cl_uint), &trianglecount, &cl::error);
    g_tri_anum = clCreateBuffer(cl::context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR , sizeof(cl_uint), &p0, &cl::error);


    if(cl::error!=0)
    {
        std::cout << "g_tri_num create" << std::endl;
        exit(cl::error);
    }

    std::cout << trianglecount << std::endl;

    cl_uint running=0;

    int obj_id=0;

    for(std::vector<object*>::iterator it=obj_list.begin(); it!=obj_list.end(); it++)
    {
        for(int i=0; i<(*it)->tri_num; i++)
        {
            (*it)->tri_list[i].vertices[0].pad[1]=obj_id;
        }

        clEnqueueWriteBuffer(cl::cqueue, g_tri_mem, CL_TRUE, sizeof(triangle)*running, sizeof(triangle)*(*it)->tri_num, (*it)->tri_list.data(), 0, NULL, NULL);
        running+=(*it)->tri_num;
        obj_id++;
    }


    tri_num=trianglecount;

    clFinish(cl::cqueue);
}

#include "texture.hpp"
#include "clstate.h"
#include <iostream>
#include <math.h>
cl_uint texture::gidc=0;

std::vector<texture> texture::texturelist;


texture::texture()
{

    loaded=false;
    g_pushed=false;
}

cl_uint texture::idquerystring(std::string name)
{
    cl_uint id=-1;

    for(std::vector<texture>::iterator it=texturelist.begin(); it!=texturelist.end(); it++)
    {
        id++;

        if((*it).location==name)
        {
            return id;
        }

    }

    return -1;
}

bool texture::t_compare(texture one, texture two)
{

    return one.c_image.getSize().x < two.c_image.getSize().x;

}

cl_uint texture::init()
{
    id=gidc++;
    loaded=true;
    texturelist.push_back(*this);
    return id;
}

cl_uint texture::loadtomaster(std::string loc)
{
    //std::cout << loc << std::endl;

    cl_uint idq=-1;

    if((idq=idquerystring(loc))==-1)
    {
        //std::cout << "hi";

        location=loc;
        c_image.loadFromFile(loc);

        if(c_image.getSize().x!=c_image.getSize().x)
        {
            std::cout << "you loaded a non square texture, god damned you " << loc << std::endl;
            //exit(12);
        }

        if(c_image.getSize().x>2048)
        {
            std::cout << "maxsize limit " << loc << std::endl;
        }

        if(c_image.getSize().x<256)
        {
            std::cout << "minsize limit " << loc << std::endl;
        }

        if(c_image.getSize().x!=256 && c_image.getSize().x!=512 && c_image.getSize().x!=1024 && c_image.getSize().x!=2048)
        {
            std::cout << "not between 256 and 2048 " << loc << std::endl;
        }

        //c_gcompatible_image = new cl_float4*[c_image.GetWidth()];
        /*for(unsigned int i=0; i<c_image.GetWidth(); i++)
        {
            c_gcompatible_image[i]=new cl_float4[c_image.GetHeight()];
        }*/

        /*for(unsigned int i=0; i<c_image.GetWidth(); i++)
        {

            for(unsigned int j=0; j<c_image.GetHeight(); j++)
            {

                sf::Color sfml=c_image.GetPixel(i, j);
                c_gcompatible_image[i][j].x=sfml.r;
                c_gcompatible_image[i][j].y=sfml.g;
                c_gcompatible_image[i][j].z=sfml.b;

            }

        }*/

        id=gidc++;
        loaded=true;
        texturelist.push_back(*this);

        return id;

    }

    id=idq;
    return idq;

}

void texture::g_push()
{


    clReleaseMemObject(image);

    cl_image_format fermat;
    fermat.image_channel_order=CL_RGBA;
    fermat.image_channel_data_type=CL_UNSIGNED_INT8;

    //void *pointer = c_image.GetPixelsPtr();

    size_t origin[3]= {0,0,0};
    size_t region[3]= {c_image.getSize().x,c_image.getSize().x,0};

    image=clCreateImage2D(cl::context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, &fermat, c_image.getSize().x, c_image.getSize().x, 0, NULL, NULL);
    clEnqueueWriteImage(cl::cqueue, image, true, origin, region, 0, 0, c_image.getPixelsPtr(), 0, NULL, NULL);
    //cl_float4 **image; ///this is unfortunate



}


cl_uchar4 **generate_mip_level(int level, texture *T)
{

    ///mip level 0 is original, 1 is half, 2 is half of a half, so basically size=size/(2^level)

    unsigned int mipsize=T->c_image.getSize().x / (pow(2, level));

    cl_uchar4 **mip=new cl_uchar4*[mipsize];

    for(unsigned int i=0; i<mipsize; i++)
    {
        mip[i]=new cl_uchar4[mipsize];
    }

    for(unsigned int i=0; i<mipsize; i++)
    {
        for(unsigned int j=0; j<mipsize; j++)
        {

            cl_float4 aggregate= {0,0,0,0};

            for(int n=0; n<level+1; n++)
            {
                for(int k=0; k<level+1; k++)
                {
                    sf::Color pix=T->c_image.getPixel(n+i, j+k);
                    aggregate.x+=pix.r;
                    aggregate.y+=pix.g;
                    aggregate.z+=pix.b;
                }
            }

            //aggregate=
            aggregate.x/=(float)(level+1)*(level+1);
            aggregate.y/=(float)(level+1)*(level+1);
            aggregate.z/=(float)(level+1)*(level+1);
            mip[i][j].x=aggregate.x;
            mip[i][j].y=aggregate.y;
            mip[i][j].z=aggregate.z;

        }

    }



    return mip;

}


void ssfmlimage(sf::Image &img, cl_uchar4 **cimg, int size)
{

    img.create(size, size);

    for(int i=0; i<size; i++)
    {
        for(int j=0; j<size; j++)
        {

            sf::Color col;
            col.r=cimg[i][j].x;
            col.g=cimg[i][j].y;
            col.b=cimg[i][j].z;

            img.setPixel(i, j, col);

        }
    }

}

void tile_4_1(sf::Image *img, sf::Image *c1, sf::Image *c2, sf::Image *c3, sf::Image *c4)
{

    sf::Image *p[4]= {c1, c2, c3, c4};
    ///sizes must all be the same

    int size=c1->getSize().x;

    for(int n=0; n<4; n++)
    {
        if(p[n]==NULL)
        {
            continue;
        }

        for(int i=0; i<size; i++)
        {
            for(int j=0; j<size; j++)
            {
                sf::Image *which=p[n];
                sf::Color col=which->getPixel(i, j);
                int tx=i, ty=j;
                int nx, ny;
                nx=n%2;
                nx*=img->getSize().x >> 1;
                ny=floor(n/2);
                ny*=img->getSize().x >> 1;

                tx+=nx;
                ty+=ny;
                img->setPixel(nx, ny, col);




            }

        }


    }

}


void texture::generate_mipmaps() ///move this to graphics card?
{


    //std::vector<texture> tmipmap;

    texture *l=&texture::texturelist[0];
    unsigned int length=texture::texturelist.size();

    texture *pl=l;

    int numberofchanges=0;

    for(unsigned int i=0; i<length-1; i++, pl++)
    {
        if(pl->c_image.getSize().x != (pl+1)->c_image.getSize().x)
        {
            numberofchanges++; ///this + 1 is the number of different texture levels
        }
    }

    numberofchanges++;


    //std::vector<texture> *texturearray=new std::vector<texture>[numberofchanges];
    std::vector<sf::Image> miptextures;

    int tlistcount=0;
    //for(int i=0; i<numberofchanges; i++)
    // {



    /*for(unsigned int i=0; i<length; i++)
    {

        sf::Image img;
        int miplevel=3;
        //img.loadFromPixels(l[i].c_image.getSize().x >> miplevel, l[i].c_image.getSize().x >> 1, (uint8_t*)generate_mip_level(miplevel, &l[i]));
        ssfmlimage(img, generate_mip_level(miplevel, &l[i]),  l[i].c_image.getSize().x >> miplevel);
        miptextures.push_back(img);
        //miptextures.push_back(img);
        //std::cout << l[i].c_image.getSize().x << std::endl;
        //texture::texturelist[i].c_image=img;

    }*/





    // }

}

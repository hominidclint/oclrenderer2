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

        if(c_image.getSize().x!=c_image.getSize().y)
        {
            //std::cout << "you loaded a non square texture, god damned you " << loc << std::endl;
            //exit(12);
        }

        if(c_image.getSize().x>2048)
        {
            std::cout << "maxsize limit " << loc << std::endl;
        }

        if(c_image.getSize().x<256)
        {
            //std::cout << "minsize limit " << loc << std::endl;
        }

        if(c_image.getSize().x!=256 && c_image.getSize().x!=512 && c_image.getSize().x!=1024 && c_image.getSize().x!=2048)
        {
            //std::cout << "not between 256 and 2048 " << loc << std::endl;
        }

        id=gidc++;
        loaded=true;
        texturelist.push_back(*this);

        return id;

    }

    id=idq;
    return idq;

}

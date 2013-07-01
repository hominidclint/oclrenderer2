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

///this really needs to be changed

cl_uint texture::loadtomaster(std::string loc)
{
    cl_uint idq=-1;

    if((idq=idquerystring(loc))==-1)
    {
        location=loc;
        c_image.loadFromFile(loc);

        if(c_image.getSize().x>2048)
        {
            std::cout << "maxsize limit " << loc << std::endl;
        }

        id=gidc++;
        loaded=true;
        texturelist.push_back(*this);

        return id;
    }

    id=idq;
    return idq;
}

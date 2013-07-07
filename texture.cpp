#include "texture.hpp"
#include "clstate.h"
#include <iostream>
#include <math.h>
cl_uint texture::gidc=0;

std::vector<texture> texture::texturelist;
std::vector<cl_uint> texture::active_textures;


texture::texture()
{
    loaded=false;
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

cl_uint texture::idqueryisactive(cl_uint pid)
{
    cl_uint id=-1;

    for(std::vector<cl_uint>::iterator it=active_textures.begin(); it!=active_textures.end(); it++)
    {
        id++;

        if((*it)==pid)
        {
            return id;
        }
    }
    return -1;
}

cl_uint texture::idquerytexture(cl_uint id)
{
    if(id < texturelist.size())
    {
        return id;
    }
    else
    {
        return -1;
    }
}

bool texture::t_compare(texture one, texture two)
{
    return one.get_largest_dimension() < two.get_largest_dimension();
}

cl_uint texture::get_largest_dimension()
{
    return c_image.getSize().x > c_image.getSize().y ? c_image.getSize().x : c_image.getSize().y;
}

void texture::init()
{
    //location = name;
    //id = gidc++;
    loaded = false;
    isactive = false;
    //texturelist.push_back(*this);
    //return id;
}

cl_uint texture::get_id()
{
    cl_uint temp_id = idquerystring(location);

    if(temp_id == -1)
        id = gidc++;
    else
        id = temp_id;

    return id;
}

cl_uint texture::push()
{
    texturelist.push_back(*this);
    return id;
}

cl_uint texture::set_active(bool param)
{
    if(param)
    {
        if(!isactive)
        {
            active_textures.push_back(id);
            isactive = param;
            return active_textures.size() - 1;
        }
        else
        {
            return idqueryisactive(id);
        }
    }
    else
    {
        if(isactive)
        {
            cl_uint a_id = idqueryisactive(id);
            std::vector<cl_uint>::iterator it = active_textures.begin();
            for(int i=0; i<a_id; i++)
            {
                it++;
            }
            active_textures.erase(it);
            return -1;
        }
        else
        {
            return -1;
        }
    }
}


void texture::set_texture_location(std::string loc)
{
    location = loc;
}

///this really needs to be changed

cl_uint texture::loadtomaster()
{
    c_image.loadFromFile(location);

    if(get_largest_dimension()>max_tex_size)
    {
        std::cout << "maxsize limit " << location << std::endl;
    }

    loaded=true;

    return id;
}

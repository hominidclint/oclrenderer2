#define MIP_LEVELS 4

#define FOV_CONST 700.0f

#define IDENTIFICATION_BITS 10

#define SCREENWIDTH 800
#define SCREENHEIGHT 600

__constant float depth_far=140000;
__constant uint mulint=UINT_MAX;
__constant float depth_cutoff=0.22f;


struct interp_container;



float min3(float x, float y, float z)
{
    return min(min(x,y),z);
}

float max3( float x,  float y,  float z)
{
    return max(max(x,y),z);
}

///triangle equations

float form(int x1, int y1, int x2, int y2, int x3, int y3, int x, int y, int which)
{

    if(which==1)
    {
        return fabs((float)((x2*y-x*y2)+(x3*y2-x2*y3)+(x*y3-x3*y))/2.0);
    }
    else if(which==2)
    {
        return fabs((float)((x*y1-x1*y)+(x3*y-x*y3)+(x1*y3-x3*y1))/2.0);
    }
    else if(which==3)
    {
        return fabs((float)((x2*y1-x1*y2)+(x*y2-x2*y)+(x1*y-x*y1))/2.0);
    }

    return fabs((float)((x2*y1-x1*y2)+(x3*y2-x2*y3)+(x1*y3-x3*y1)))/2.0;
}



struct obj_g_descriptor
{
    float4 world_pos;   ///w is blaenk
    float4 world_rot;   ///w is blaenk
    uint start;         ///internally used value
    uint tri_num;
    uint tid;           ///texture id
    uint size;
    uint mip_level_ids[MIP_LEVELS];
};



struct vertex
{
    float4 pos;
    float4 normal;
    float2 vt;
    float2 pad;
};

struct triangle
{
    struct vertex vertices[3];
};

struct interp_container
{
    int x[3];
    int y[3];
    int xbounds[2];
    int ybounds[2];
    int rconstant;
    int area;
};


float calc_third_areas_i(int x1, int x2, int x3, int y1, int y2, int y3, int x, int y)
{
    return (fabs((float)((x2*y-x*y2)+(x3*y2-x2*y3)+(x*y3-x3*y))/2.0) + fabs((float)((x*y1-x1*y)+(x3*y-x*y3)+(x1*y3-x3*y1))/2.0) + fabs((float)((x2*y1-x1*y2)+(x*y2-x2*y)+(x1*y-x*y1))/2.0));
    ///form was written for this, i think
}

float calc_third_areas(struct interp_container *C, int x, int y)
{
    return calc_third_areas_i(C->x[0], C->x[1], C->x[2], C->y[0], C->y[1], C->y[2], x, y);
}



float4 rot(float4 point, float4 c_pos, float4 c_rot)
{
    float4 ret;
    ret.x=      native_cos(c_rot.y)*(native_sin(c_rot.z)+native_cos(c_rot.z)*(point.x-c_pos.x)) - native_sin(c_rot.y)*(point.z-c_pos.z);
    ret.y=      native_sin(c_rot.x)*(native_cos(c_rot.y)*(point.z-c_pos.z)+native_sin(c_rot.y)*(native_sin(c_rot.z)*(point.y-c_pos.y)+native_cos(c_rot.z)*(point.x-c_pos.x)))+native_cos(c_rot.x)*(native_cos(c_rot.z)*(point.y-c_pos.y)-native_sin(c_rot.z)*(point.x-c_pos.x));
    ret.z=      native_cos(c_rot.x)*(native_cos(c_rot.y)*(point.z-c_pos.z)+native_sin(c_rot.y)*(native_sin(c_rot.z)*(point.y-c_pos.y)+native_cos(c_rot.z)*(point.x-c_pos.x)))-native_sin(c_rot.x)*(native_cos(c_rot.z)*(point.y-c_pos.y)-native_sin(c_rot.z)*(point.x-c_pos.x));
    return ret;
}




float dcalc(float value)
{
    value=(log(value + 1)/(log(depth_far + 1)));

    return value;
}

float interpolate_i(float f1, float f2, float f3, int x, int y, int x1, int x2, int x3, int y1, int y2, int y3, int rconstant)
{
    float A=native_divide((f2*y3+f1*(y2-y3)-f3*y2+(f3-f2)*y1),rconstant);
    float B=native_divide(-(f2*x3+f1*(x2-x3)-f3*x2+(f3-f2)*x1),rconstant);
    float C=f1-A*x1 - B*y1;

    float cdepth=(A*x + B*y + C);
    return cdepth;
}

float interpolate (float f[3], struct interp_container *c, int x, int y)
{
    return interpolate_i(f[0], f[1], f[2], x, y, c->x[0], c->x[1], c->x[2], c->y[0], c->y[1], c->y[2], c->rconstant);
}

int out_of_bounds(float val, float min, float max)
{
    if(val >= min && val < max)
    {
        return false;
    }
    return true;
}


struct triangle full_rotate(__global struct triangle *triangle, __global float4 *c_pos, __global float4 *c_rot, struct interp_container *container)
{

    __global struct triangle *T=triangle;

    float4 rotpoints[3];
    rotpoints[0]=rot(T->vertices[0].pos, *c_pos, *c_rot);
    rotpoints[1]=rot(T->vertices[1].pos, *c_pos, *c_rot);
    rotpoints[2]=rot(T->vertices[2].pos, *c_pos, *c_rot);

    float4 normalrot[3];
    normalrot[0]=rot(T->vertices[0].normal, *c_pos, *c_rot);
    normalrot[1]=rot(T->vertices[1].normal, *c_pos, *c_rot);
    normalrot[2]=rot(T->vertices[2].normal, *c_pos, *c_rot);

    for(int j=0; j<3; j++)
    {
        float rx;
        rx=rotpoints[j].x * (FOV_CONST/rotpoints[j].z);
        float ry;
        ry=rotpoints[j].y * (FOV_CONST/rotpoints[j].z);

        rx+=SCREENWIDTH/2.0f;
        ry+=SCREENHEIGHT/2.0f;


        rotpoints[j].x=rx;
        rotpoints[j].y=ry;
        rotpoints[j].z=dcalc(rotpoints[j].z);
    }


    struct triangle ret;


    for(int i=0; i<3; i++)
    {
        ret.vertices[i]=T->vertices[i];

        ret.vertices[i].pos   =rotpoints[i];

        ret.vertices[i].normal=normalrot[i];
    }


    int y1 = round(rotpoints[0].y);
    int y2 = round(rotpoints[1].y);
    int y3 = round(rotpoints[2].y);


    int x1 = round(rotpoints[0].x);
    int x2 = round(rotpoints[1].x);
    int x3 = round(rotpoints[2].x);

    int miny=min3(y1, y2, y3)-1; ///oh, wow
    int maxy=max3(y1, y2, y3);
    int minx=min3(x1, x2, x3)-1;
    int maxx=max3(x1, x2, x3);

    if(out_of_bounds(miny, 0, SCREENHEIGHT))
    {
        miny=max(miny, 0);
        miny=min(miny, SCREENHEIGHT);
    }
    if(out_of_bounds(maxy, 0, SCREENHEIGHT))
    {
        maxy=max(maxy, 0);
        maxy=min(maxy, SCREENHEIGHT);
    }
    if(out_of_bounds(minx, 0, SCREENWIDTH))
    {
        minx=max(minx, 0);
        minx=min(minx, SCREENWIDTH);
    }
    if(out_of_bounds(maxx, 0, SCREENWIDTH))
    {
        maxx=max(maxx, 0);
        maxx=min(maxx, SCREENWIDTH);
    }

    int rconstant=(x2*y3+x1*(y2-y3)-x3*y2+(x3-x2)*y1);

    int area=form(x1, y1, x2, y2, x3, y3, 0, 0, 0);

    struct interp_container *C=container;

    C->x[0]=x1;
    C->x[1]=x2;
    C->x[2]=x3;

    C->y[0]=y1;
    C->y[1]=y2;
    C->y[2]=y3;

    C->xbounds[0]=minx;
    C->xbounds[1]=maxx;

    C->ybounds[0]=miny;
    C->ybounds[1]=maxy;

    C->rconstant=rconstant;

    C->area=area;



    return ret;
}


struct interp_container construct_interpolation(__global struct triangle *tri)
{
    struct interp_container C;

    int y1 = round(tri->vertices[0].pos.y);
    int y2 = round(tri->vertices[1].pos.y);
    int y3 = round(tri->vertices[2].pos.y);


    int x1 = round(tri->vertices[0].pos.x);
    int x2 = round(tri->vertices[1].pos.x);
    int x3 = round(tri->vertices[2].pos.x);

    int miny=min3(y1, y2, y3)-1; ///oh, wow
    int maxy=max3(y1, y2, y3);
    int minx=min3(x1, x2, x3)-1;
    int maxx=max3(x1, x2, x3);

    if(out_of_bounds(miny, 0, SCREENHEIGHT))
    {
        miny=max(miny, 0);
        miny=min(miny, SCREENHEIGHT);
    }
    if(out_of_bounds(maxy, 0, SCREENHEIGHT))
    {
        maxy=max(maxy, 0);
        maxy=min(maxy, SCREENHEIGHT);
    }
    if(out_of_bounds(minx, 0, SCREENWIDTH))
    {
        minx=max(minx, 0);
        minx=min(minx, SCREENWIDTH);
    }
    if(out_of_bounds(maxx, 0, SCREENWIDTH))
    {
        maxx=max(maxx, 0);
        maxx=min(maxx, SCREENWIDTH);
    }

    int rconstant=(x2*y3+x1*(y2-y3)-x3*y2+(x3-x2)*y1);

    int area=form(x1, y1, x2, y2, x3, y3, 0, 0, 0);

    C.x[0]=x1;
    C.x[1]=x2;
    C.x[2]=x3;

    C.y[0]=y1;
    C.y[1]=y2;
    C.y[2]=y3;

    C.xbounds[0]=minx;
    C.xbounds[1]=maxx;

    C.ybounds[0]=miny;
    C.ybounds[1]=maxy;

    C.rconstant=rconstant;

    C.area=area;


    return C;
}




__kernel void part1(__global struct triangle* triangles, __global struct triangle *screen_triangles, __global uint* tri_num, __global uint *atomic_triangles, __global float4* c_pos, __global float4* c_rot, __global uint* depth_buffer)
{
    ///rotate triangles into screenspace, clip, and probably perform rough hierarchical depth buffering
    ///lighting normals give you backface culling

    uint id = get_global_id(0);

    struct interp_container icontainer;



    int valid_tri=0;

    if(id >= *tri_num)
    {
        return;
    }

    struct triangle tri=full_rotate(&triangles[id], c_pos, c_rot, &icontainer);


    if(icontainer.ybounds[1]-icontainer.ybounds[0] > 50 || icontainer.xbounds[1] - icontainer.xbounds[0] > 50)
    {
        return;
    }


    for(int y=icontainer.ybounds[0]; y<icontainer.ybounds[1]; y++)
    {
        for(int x=icontainer.xbounds[0]; x<icontainer.xbounds[1]; x++)
        {
            float s1=calc_third_areas(&icontainer, x, y);

            if(s1 > icontainer.area - 1 && s1 < icontainer.area + 1)
            {
                __global uint *ft=&depth_buffer[y*SCREENWIDTH + x];

                float depths[3]={tri.vertices[0].pos.z, tri.vertices[1].pos.z, tri.vertices[2].pos.z};
                float fmydepth=interpolate(depths, &icontainer, x, y);

                uint mydepth=fmydepth*mulint;

                if(mydepth==0)
                {
                    continue;
                }

                uint sdepth=atomic_min(ft, mydepth);
                if(!valid_tri && mydepth<sdepth)
                {
                    valid_tri=1;
                }
            }
        }
    }

    if(valid_tri)
    {
        uint at_id=atomic_inc(atomic_triangles);
        __global struct triangle *Ts=&screen_triangles[at_id];

        for(int i=0; i<3; i++)
        {
            Ts->vertices[i]=tri.vertices[i];
        }
    }
}

__kernel void part2(__global struct triangle* screen_triangles, __global uint* anum, __global uint* depth_buffer, __global uint* id_buffer)
{
    ///In screenspace, take triangle and work out if the triangle's depth is that in the pic. If yes, write id
    uint id=get_global_id(0);

    if(id>=*anum)
    {
        return;
    }
    __global struct triangle *tri=&screen_triangles[id];

    struct interp_container icontainer=construct_interpolation(tri);

    if(icontainer.ybounds[1]-icontainer.ybounds[0] > 50 || icontainer.xbounds[1] - icontainer.xbounds[0] > 50)
    {
        return;
    }

    for(int y=icontainer.ybounds[0]; y<icontainer.ybounds[1]; y++)
    {
        for(int x=icontainer.xbounds[0]; x<icontainer.xbounds[1]; x++)
        {
            float s1=calc_third_areas(&icontainer, x, y);

            if(s1 > icontainer.area - 1 && s1 < icontainer.area + 1)
            {
                __global uint *ft=&depth_buffer[y*SCREENWIDTH + x];

                float depths[3]={tri->vertices[0].pos.z, tri->vertices[1].pos.z, tri->vertices[2].pos.z};
                float fmydepth=interpolate(depths, &icontainer, x, y);

                uint mydepth=fmydepth*mulint;

                if(mydepth==0)
                {
                    continue;
                }

                if(mydepth > *ft - 50 && mydepth < *ft + 50)
                {
                    __global uint *fi=&id_buffer[y*SCREENWIDTH + x];
                    *fi=id;
                }
            }
        }
    }


}

__kernel void part3(__global struct triangle *triangles, __global struct triangle *screen_triangles, __global uint *tri_num, __global uint *anum, __global uint* depth_buffer, __global uint* id_buffer, __read_only image3d_t array, __write_only image2d_t screen)
{
    ///widthxheight kernel
    sampler_t sam = CLK_NORMALIZED_COORDS_FALSE |
               CLK_ADDRESS_CLAMP_TO_EDGE        |
               CLK_FILTER_NEAREST;


    uint x=get_global_id(0);
    uint y=get_global_id(1);



    if(x < SCREENWIDTH && y < SCREENHEIGHT)
    {
        __global uint *ft=&depth_buffer[y*SCREENWIDTH + x];
        __global uint *fi=&id_buffer   [y*SCREENWIDTH + x];

        int4 coord={x, y, 0, 0};

        uint4 col;
        col=read_imageui(array, sam, coord);

        uint depth=*ft;
        uint id=*fi;


        float4 col1;
        col1.x=col.x/255.0f;
        col1.y=col.y/255.0f;
        col1.z=col.z/255.0f;
        col1.x = col1.x*0.001;
        col1.y = col1.y*0.001;
        col1.z = col1.z*0.001;

        col1.x=col1.x*0.001 + (float)depth/mulint;
        col1.y=(float)depth/mulint;
        col1.z=(float)depth/mulint;
        //col1.z=(id % 255)/255.0f;

        int2 scoord={x, y};





        write_imagef(screen, scoord, col1);
        *ft=mulint;
        //*fi=0;
    }

}

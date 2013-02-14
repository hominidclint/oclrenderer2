#define MIP_LEVELS 4

#define FOV_CONST 700.0f

#define IDENTIFICATION_BITS 10

#define SCREENWIDTH 800
#define SCREENHEIGHT 600

#define MTRI_SIZE 50

__constant float depth_far=140000;
__constant uint mulint=UINT_MAX;
//__constant float depth_cutoff=0.12f;
__constant int depth_icutoff=50;


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


struct triangle full_rotate(__global struct triangle *triangle, __global float4 *c_pos, __global float4 *c_rot, struct interp_container *container, float odepth[3])
{

    __global struct triangle *T=triangle;

    float4 rotpoints[3];
    rotpoints[0]=rot(T->vertices[0].pos, *c_pos, *c_rot);
    rotpoints[1]=rot(T->vertices[1].pos, *c_pos, *c_rot);
    rotpoints[2]=rot(T->vertices[2].pos, *c_pos, *c_rot);

    float4 nought={0,0,0,0};
    float4 normalrot[3];
    normalrot[0]=rot(T->vertices[0].normal, nought, *c_rot);
    normalrot[1]=rot(T->vertices[1].normal, nought, *c_rot);
    normalrot[2]=rot(T->vertices[2].normal, nought, *c_rot);

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
        odepth[j]=rotpoints[j].z;
        rotpoints[j].z=dcalc(rotpoints[j].z);
    }


    struct triangle ret;


    for(int i=0; i<3; i++)
    {
        ret.vertices[i]=T->vertices[i];

        ret.vertices[i].pos   =rotpoints[i];

        //ret.vertices[i].normal=normalrot[i];
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

////all texture code was not rewritten for time, does not use proper functions
float4 read_tex_array(float4 coords, int tid, global uint *num, global uint *size, __read_only image3d_t array)
{

    sampler_t sam = CLK_NORMALIZED_COORDS_FALSE |
               CLK_ADDRESS_CLAMP_TO_EDGE        |
               CLK_FILTER_NEAREST;

    int d=get_image_depth(array);

    float x=coords.x;
    float y=coords.y;

    int slice = num[tid] >> 16;

    int which = num[tid] & 0x0000FFFF;

    int max_tex_size=2048;

    int width=size[slice];
    int bx=which % (max_tex_size/width);
    int by=which / (max_tex_size/width);

    x=1-x;
    y=1-y;


    x*=width;
    y*=width;

    if(x<0)
        x=0;

    if(x>=width)
        x=width - 1;

    if(y<0)
        y=0;

    if(y>=width)
        y=width - 1;


    int hnum=max_tex_size/width;
    ///max horizontal and vertical nums

    float tnumx=which % hnum;
    float tnumy=which / hnum;

    float tx=tnumx*width;
    float ty=tnumy*width;


    float4 coord={tx + x, ty + y, slice, 0};

    uint4 col;
    col=read_imageui(array, sam, coord);
    float4 t;
    t.x=col.x/255.0f;
    t.y=col.y/255.0f;
    t.z=col.z/255.0f;

    return t;
}


float4 return_bilinear_col(float4 coord, uint tid, global uint *nums, global uint *sizes, __read_only image3d_t array) ///takes a normalised input
{


    float4 mcoord;

    int which=nums[tid];
    float width=sizes[which >> 16];

    mcoord.x=coord.x*width - 0.5;
    mcoord.y=coord.y*width - 0.5;
    mcoord.z=coord.z;

    float4 coords[4];

    int2 pos={floor(mcoord.x), floor(mcoord.y)};

    coords[0].x=pos.x, coords[0].y=pos.y;
    coords[1].x=pos.x+1, coords[1].y=pos.y;
    coords[2].x=pos.x, coords[2].y=pos.y+1;
    coords[3].x=pos.x+1, coords[3].y=pos.y+1;



    float4 colours[4];
    for(int i=0; i<4; i++)
    {
        coords[i].x/=width;
        coords[i].y/=width;
        coords[i].z=coord.z;
        colours[i]=read_tex_array(coords[i], tid, nums, sizes, array);
    }



    float2 uvratio={mcoord.x-pos.x, mcoord.y-pos.y};

    float2 buvr={1.0-uvratio.x, 1.0-uvratio.y};

    float4 result;
    result.x=(colours[0].x*buvr.x + colours[1].x*uvratio.x)*buvr.y + (colours[2].x*buvr.x + colours[3].x*uvratio.x)*uvratio.y;
    result.y=(colours[0].y*buvr.x + colours[1].y*uvratio.x)*buvr.y + (colours[2].y*buvr.x + colours[3].y*uvratio.x)*uvratio.y;
    result.z=(colours[0].z*buvr.x + colours[1].z*uvratio.x)*buvr.y + (colours[2].z*buvr.x + colours[3].z*uvratio.x)*uvratio.y;


    return result;

}

float4 texture_filter(global struct triangle* c_tri, int2 spos, float4 vt, float depth, float4 c_pos, float4 c_rot, int tid2, global uint* mipd , global uint *nums, global uint *sizes, __read_only image3d_t array)
{

    ///find z coord of texture pixel, work out distance of transition between the adjacent 2 texture levels (+ which 2 texture levels), bilinearly interpolate the two pixels, then interpolate the result

    //global struct triangle *tri;
    //tri=&triangles[id];
    global struct triangle *tri=c_tri;

    int width=SCREENWIDTH;
    int height=SCREENHEIGHT;

    float4 rotpoints[3];
    rotpoints[0]=c_tri->vertices[0].pos;
    rotpoints[1]=c_tri->vertices[1].pos;
    rotpoints[2]=c_tri->vertices[2].pos;

    float adepth=(rotpoints[0].z + rotpoints[1].z + rotpoints[2].z)/3.0f;

    ///maths works out to be z = 700*n where n is 1/2, 1/4 of the size etc
    ///so, if z of pixel is between 0-700, use least, then first, then second, etc

    int tids[MIP_LEVELS+1];
    tids[0]=tid2;

    for(int i=1; i<MIP_LEVELS+1; i++)
    {
        tids[i]=mipd[i-1];
    }

    float mipdistance=FOV_CONST;

    float part=0;

    if(adepth<1)
    {
        adepth=1;
    }

    int wc=0;
    int wc1;
    float mdist;
    float fdist;

    int slice=nums[tid2] >> 16;
    int twidth=sizes[slice];



    float minvx=min3(rotpoints[0].x, rotpoints[1].x, rotpoints[2].x); ///these are screenspace coordinates, used relative to each other so +width/2.0 cancels
    float maxvx=max3(rotpoints[0].x, rotpoints[1].x, rotpoints[2].x);

    float minvy=min3(rotpoints[0].y, rotpoints[1].y, rotpoints[2].y);
    float maxvy=max3(rotpoints[0].y, rotpoints[1].y, rotpoints[2].y);

    float mintx=min3(tri->vertices[0].vt.x, tri->vertices[1].vt.x, tri->vertices[2].vt.x);
    float maxtx=max3(tri->vertices[0].vt.x, tri->vertices[1].vt.x, tri->vertices[2].vt.x);

    float minty=min3(tri->vertices[0].vt.y, tri->vertices[1].vt.y, tri->vertices[2].vt.y);
    float maxty=max3(tri->vertices[0].vt.y, tri->vertices[1].vt.y, tri->vertices[2].vt.y);

    float txdif=maxtx-mintx;
    float tydif=maxty-minty;


    float vxdif=maxvx-minvx;
    float vydif=maxvy-minvy;

    float xtexelsperpixel=txdif*twidth/vxdif;
    float ytexelsperpixel=tydif*twidth/vydif;

    float texelsperpixel = xtexelsperpixel > ytexelsperpixel ? xtexelsperpixel : ytexelsperpixel;


    float effectivedistance=mipdistance*texelsperpixel;

    if(effectivedistance<0)
    {
        float4 col;
        col.x=0;
        col.y=0;
        col.z=0;
        return col;
    }

    float corrected_depth=adepth + effectivedistance;




    for(int i=0; i<5; i++) //fundementally broken, using width of polygon when it needs to be using
                            //texture width to calculate miplevel
    {
        wc=i;
        if(i==4)
        {
            mdist=(pow(2.0f, (float)i)-1)*mipdistance;
            fdist=(pow(2.0f, (float)i)-1)*mipdistance;
            break;
        }

        mdist=(pow(2.0f, (float)i)-1)*mipdistance;
        fdist=(pow(2.0f, (float)i+1.0f)-1)*mipdistance;

        if(corrected_depth > mdist && corrected_depth < fdist)
        {
            break;
        }
    }

    wc1=wc+1;
    part=(fdist-corrected_depth)/(fdist-mdist);

    if(wc==4)
    {
        wc1=4;
        part=1;
    }


    ///how far away from the upper mip level are we, between 0 and 1;
    //x, y, tid, num, size, array;

    float4 coord={(float)vt.x, (float)vt.y, 0, 0};

    float4 col1=return_bilinear_col(coord, tids[wc], nums, sizes, array);

    float4 col2=return_bilinear_col(coord, tids[wc1], nums, sizes, array);

    float4 final=col1*(part) + col2*(1-part);

    /*col->x=final.x;
    col->y=final.y;
    col->z=final.z;*/

    return final;
}
///end of unrewritten code

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

    float odepth[3];

    struct triangle tri=full_rotate(&triangles[id], c_pos, c_rot, &icontainer, odepth);


    if(icontainer.ybounds[1]-icontainer.ybounds[0] > MTRI_SIZE || icontainer.xbounds[1] - icontainer.xbounds[0] > MTRI_SIZE)
    {
        return;
    }


    ///backface culling!

    int cont=0;

    if((tri.vertices[0].pos.z) < dcalc(depth_icutoff) && (tri.vertices[1].pos.z) < dcalc(depth_icutoff) && (tri.vertices[2].pos.z) < dcalc(depth_icutoff))
    {
        return;
    }


    float4 p0={tri.vertices[0].pos.x, tri.vertices[0].pos.y, tri.vertices[0].pos.z, 0};
    float4 p1={tri.vertices[1].pos.x, tri.vertices[1].pos.y, tri.vertices[1].pos.z, 0};
    float4 p2={tri.vertices[2].pos.x, tri.vertices[2].pos.y, tri.vertices[2].pos.z, 0};

    float4 p1p0=p1-p0;
    float4 p2p0=p2-p0;

    float4 cr=cross(p1p0, p2p0);



    for(int i=0; i<3; i++)
    {
        float4 blank={0,0,0,0};
        float4 fnormal=cr;

        float4 local_vertex_position={tri.vertices[i].pos.x*tri.vertices[i].pos.z/FOV_CONST, tri.vertices[i].pos.y*tri.vertices[i].pos.z/FOV_CONST, tri.vertices[i].pos.z, 0};

        float4 p2c=local_vertex_position;
        float dotp=dot(fast_normalize(fnormal), fast_normalize(p2c));

        if(dotp < 0)
        {
            cont=1;
        }
    }

    if(cont!=1)
    {
        return;
    }


    float depths[3]={tri.vertices[0].pos.z, tri.vertices[1].pos.z, tri.vertices[2].pos.z};

    for(int y=icontainer.ybounds[0]; y<icontainer.ybounds[1]; y++)
    {
        for(int x=icontainer.xbounds[0]; x<icontainer.xbounds[1]; x++)
        {
            float s1=calc_third_areas(&icontainer, x, y);

            if(s1 > icontainer.area - 1 && s1 < icontainer.area + 1)
            {
                __global uint *ft=&depth_buffer[y*SCREENWIDTH + x];


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
            Ts->vertices[i].pos.z=odepth[i];
        }
        Ts->vertices[0].pad.x=id;
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

    if(icontainer.ybounds[1]-icontainer.ybounds[0] > MTRI_SIZE || icontainer.xbounds[1] - icontainer.xbounds[0] > MTRI_SIZE)
    {
        return;
    }

    float depths[3]={dcalc(tri->vertices[0].pos.z), dcalc(tri->vertices[1].pos.z), dcalc(tri->vertices[2].pos.z)};

    for(int y=icontainer.ybounds[0]; y<icontainer.ybounds[1]; y++)
    {
        for(int x=icontainer.xbounds[0]; x<icontainer.xbounds[1]; x++)
        {
            float s1=calc_third_areas(&icontainer, x, y);

            if(s1 > icontainer.area - 1 && s1 < icontainer.area + 1)
            {
                __global uint *ft=&depth_buffer[y*SCREENWIDTH + x];


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

__kernel void part3(__global struct triangle *triangles, __global struct triangle *screen_triangles, __global uint *tri_num, __global uint *anum, __global float4 *c_pos, __global float4 *c_rot, __global uint* depth_buffer, __global uint* id_buffer, __read_only image3d_t array, __write_only image2d_t screen, __global int *nums, __global int *sizes, __global struct obj_g_descriptor* gobj, __global uint * gnum)
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
        if(*ft==mulint)
        {
            return;
        }
        __global struct triangle *c_tri=&screen_triangles[*fi];

        uint pid=c_tri->vertices[0].pad.x;


        int o_id=c_tri->vertices[0].pad.y;




        int4 coord={x, y, 0, 0};

        //uint4 col;
        //col=read_imageui(array, sam, coord);

        uint depth=*ft;
        uint id=*fi;

        struct interp_container icontainer=construct_interpolation(c_tri);

        float4 vt;

        float xvt[3]={c_tri->vertices[0].vt.x, c_tri->vertices[1].vt.x, c_tri->vertices[2].vt.x};
        vt.x=interpolate(xvt, &icontainer, x, y);

        float yvt[3]={c_tri->vertices[0].vt.y, c_tri->vertices[1].vt.y, c_tri->vertices[2].vt.y};
        vt.y=interpolate(yvt, &icontainer, x, y);

        float normalsx[3]={c_tri->vertices[0].normal.x, c_tri->vertices[1].normal.x, c_tri->vertices[2].normal.x};
        float normalsy[3]={c_tri->vertices[0].normal.y, c_tri->vertices[1].normal.y, c_tri->vertices[2].normal.y};
        float normalsz[3]={c_tri->vertices[0].normal.z, c_tri->vertices[1].normal.z, c_tri->vertices[2].normal.z};

        float4 normal;

        normal.x=interpolate(normalsx, &icontainer, x, y);
        normal.y=interpolate(normalsy, &icontainer, x, y);
        normal.z=interpolate(normalsz, &icontainer, x, y);

        //normal.x=1.0;
        //normal.y=1.0;
        //normal.z=0.0;

        float depths[3]={dcalc(c_tri->vertices[0].pos.z), dcalc(c_tri->vertices[1].pos.z), dcalc(c_tri->vertices[2].pos.z)};

        float fmydepth=interpolate(depths, &icontainer, x, y);


        float naturaldepths[3]={(c_tri->vertices[0].pos.z), (c_tri->vertices[1].pos.z), (c_tri->vertices[2].pos.z)};

        float nmydepth=interpolate(depths, &icontainer, x, y);


        float4 lpos={0, 300, -300, 0};
        //lpos=rot(lpos, *c_pos, *c_rot);

        /*float4 rpos[3];
        for(int i=0; i<3; i++)
        {
            rpos[i].x=c_tri->vertices[i].pos.x*c_tri->vertices[i].pos.z/FOV_CONST;
            rpos[i].y=c_tri->vertices[i].pos.y*c_tri->vertices[i].pos.z/FOV_CONST;
            rpos[i].z=c_tri->vertices[i].pos.z;
        }*/

        float4 local_position={x*nmydepth/FOV_CONST, y*nmydepth/FOV_CONST, nmydepth, 0};



        float4 l2c=lpos-local_position;

        float light=dot(fast_normalize(l2c), fast_normalize(normal));

        light/=2.0f;
        light+=0.5;





        /*float4 col1;
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
        */

        int2 scoord={x, y};

        //float4 col=texture_filter(screen_triangles[])

        ///ftmz=o_id;
        float4 col=texture_filter(c_tri, scoord, vt, depth, *c_pos, *c_rot, gobj[o_id].tid, gobj[o_id].mip_level_ids, nums, sizes, array);





        write_imagef(screen, scoord, col*light);
        *ft=mulint;
        //*fi=0;
    }

}

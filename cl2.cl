#define MIP_LEVELS 4

#define FOV_CONST 400.0f


#define IDENTIFICATION_BITS 10

#define SCREENWIDTH 800
#define SCREENHEIGHT 600

#define LIGHTBUFFERDIM 1024
#define LFOV_CONST (LIGHTBUFFERDIM/2.0f)

#define MTRI_SIZE 50

//#define MAXDEPTH 100000

__constant float depth_far=3500;
__constant uint mulint=UINT_MAX;
//__constant float depth_cutoff=0.12f;
__constant int depth_icutoff=50;


struct interp_container;

float rerror(float a, float b)
{
    return fabs(a-b);
}

float min3(float x, float y, float z)
{
    return min(min(x,y),z);
}

float max3(float x,  float y,  float z)
{
    return max(max(x,y),z);
}

///triangle equations

/*float calc_third_area(int x1, int y1, int x2, int y2, int x3, int y3, int x, int y, int which)
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
}*/


float calc_third_area(int x1, int y1, int x2, int y2, int x3, int y3, int x, int y, int which)
{
    if(which==1)
    {
        return fabs((float)(x1*(y - y3) + x*(y3 - y1) + x3*(y1 - y))/2.0f);
    }
    else if(which==2)
    {
        return fabs((float)(x1*(y2 - y) + x2*(y - y1) + x*(y1 - y2))/2.0f);
    }
    else if(which==3)
    {
        return fabs((float)(x*(y2 - y3) + x2*(y3 - y) + x3*(y - y2))/2.0f);
    }

    //return fabs((float)((x2*y1-x1*y2)+(x3*y2-x2*y3)+(x1*y3-x3*y1)))/2.0;
    // x1*(y2 - y3) + x2*(y3 - y1) + x3*(y1 - y2)

    return abs((x1*(y2 - y3) + x2*(y3 - y1) + x3*(y1 - y2))/2);
}

struct light
{
    float4 pos;
    float4 col;
    uint shadow;
    float brightness;
    float2 pad;
};


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
    float rconstant;
    int area;
    //float4 s1, s2;
    int side;
    int sg[3];
};

struct t_c
{
    struct triangle t[2];
    int c;
};


float calc_third_areas_i(int x1, int x2, int x3, int y1, int y2, int y3, int x, int y)
{
    //return (fabs((float)((x2*y-x*y2)+(x3*y2-x2*y3)+(x*y3-x3*y))/2.0f) + fabs((float)((x*y1-x1*y)+(x3*y-x*y3)+(x1*y3-x3*y1))/2.0f) + fabs((float)((x2*y1-x1*y2)+(x*y2-x2*y)+(x1*y-x*y1))/2.0f));
    return abs(((x2*y-x*y2)+(x3*y2-x2*y3)+(x*y3-x3*y))/2) + abs(((x*y1-x1*y)+(x3*y-x*y3)+(x1*y3-x3*y1))/2) + abs(((x2*y1-x1*y2)+(x*y2-x2*y)+(x1*y-x*y1))/2);
    ///form was written for this, i think
}

float calc_third_areas(struct interp_container *C, int x, int y)
{
    return calc_third_areas_i(C->x[0], C->x[1], C->x[2], C->y[0], C->y[1], C->y[2], x, y);
    //return calc_third_area(C->x[0], C->y[0], C->x[1], C->y[1], C->x[2], C->y[2], x, y, 1) + calc_third_area(C->x[0], C->y[0], C->x[1], C->y[1], C->x[2], C->y[2], x, y, 2) + calc_third_area(C->x[0], C->y[0], C->x[1], C->y[1], C->x[2], C->y[2], x, y, 3);
}





float4 rot(float4 point, float4 c_pos, float4 c_rot)
{
    float4 ret;
    ret.x=      native_cos(c_rot.y)*(native_sin(c_rot.z)+native_cos(c_rot.z)*(point.x-c_pos.x))-native_sin(c_rot.y)*(point.z-c_pos.z);
    ret.y=      native_sin(c_rot.x)*(native_cos(c_rot.y)*(point.z-c_pos.z)+native_sin(c_rot.y)*(native_sin(c_rot.z)*(point.y-c_pos.y)+native_cos(c_rot.z)*(point.x-c_pos.x)))+native_cos(c_rot.x)*(native_cos(c_rot.z)*(point.y-c_pos.y)-native_sin(c_rot.z)*(point.x-c_pos.x));
    ret.z=      native_cos(c_rot.x)*(native_cos(c_rot.y)*(point.z-c_pos.z)+native_sin(c_rot.y)*(native_sin(c_rot.z)*(point.y-c_pos.y)+native_cos(c_rot.z)*(point.x-c_pos.x)))-native_sin(c_rot.x)*(native_cos(c_rot.z)*(point.y-c_pos.y)-native_sin(c_rot.z)*(point.x-c_pos.x));
    return ret;
}




float dcalc(float value)
{
    //value=(log(value + 1)/(log(depth_far + 1)));
    value = value / depth_far;
    return value;
}

float idcalc(float value)
{
    return value * depth_far;
}

float interpolate_2(float vals[3], struct interp_container c, int x, int y)
{
    ///x1, y1, x2, y2, x3, y3, x, y, which

    float a[3];
    for(int i=0; i<3; i++)
    {
        a[i] = calc_third_area(c.x[0], c.y[0], c.x[1], c.y[1], c.x[2], c.y[2], x, y, i+1);
    }

    float area = c.area;

    return (vals[0]*a[2] + vals[1]*a[0] + vals[2]*a[1])/(a[0] + a[1] + a[2]);
}



float interpolate_i(float f1, float f2, float f3, int x, int y, int x1, int x2, int x3, int y1, int y2, int y3, float rconstant)
{
    //float A=native_divide((f2*y3+f1*(y2-y3)-f3*y2+(f3-f2)*y1),rconstant);
    //float B=native_divide(-(f2*x3+f1*(x2-x3)-f3*x2+(f3-f2)*x1),rconstant);

    float A=((f2*y3+f1*(y2-y3)-f3*y2+(f3-f2)*y1) * rconstant);
    float B=(-(f2*x3+f1*(x2-x3)-f3*x2+(f3-f2)*x1) * rconstant);
    float C=f1-A*x1 - B*y1;

    return (float)(A*x + B*y + C);
}

float interpolate(float f[3], struct interp_container *c, int x, int y)
{
    return interpolate_i(f[0], f[1], f[2], x, y, c->x[0], c->x[1], c->x[2], c->y[0], c->y[1], c->y[2], c->rconstant);
    //return interpolate_2(f, *c, x, y);
}

int out_of_bounds(float val, float min, float max)
{
    if(val >= min && val < max)
    {
        return false;
    }
    return true;
}


///triangle plane intersection borrowed off stack overflow

float distance_from_plane(float4 p, float4 pl)
{
    return dot(pl, p) + dcalc(depth_icutoff);
}

bool get_intersection(float4 p1, float4 p2, float4 *r)
{
    float d1 = distance_from_plane(p1, (float4){0,0,1,0});
    float d2 = distance_from_plane(p2, (float4){0,0,1,0});

    if (d1*d2 > 0)
        return false;

    float t = d1 / (d1 - d2);
    *r = p1 + t * (p2 - p1);

    return true;
}


struct interp_container construct_interpolation(struct triangle tri, int width, int height)
{
    struct interp_container C;

    int y1 = round(tri.vertices[0].pos.y);
    int y2 = round(tri.vertices[1].pos.y);
    int y3 = round(tri.vertices[2].pos.y);


    int x1 = round(tri.vertices[0].pos.x);
    int x2 = round(tri.vertices[1].pos.x);
    int x3 = round(tri.vertices[2].pos.x);

    int miny=min3(y1, y2, y3)-1; ///oh, wow
    int maxy=max3(y1, y2, y3);
    int minx=min3(x1, x2, x3)-1;
    int maxx=max3(x1, x2, x3);

    //if(out_of_bounds(miny, 0, SCREENHEIGHT))
    {
        miny=max(miny, 0);
        miny=min(miny, height);
    }
    //if(out_of_bounds(maxy, 0, SCREENHEIGHT))
    {
        maxy=max(maxy, 0);
        maxy=min(maxy, height);
    }
    //if(out_of_bounds(minx, 0, SCREENWIDTH))
    {
        minx=max(minx, 0);
        minx=min(minx, width);
    }
    //if(out_of_bounds(maxx, 0, SCREENWIDTH))
    {
        maxx=max(maxx, 0);
        maxx=min(maxx, width);
    }

    /*int oobw[2] = {-1, -1};
    int nob = -1;
    int pc=0;
    for(int j=0; j<3; j++)
    {
        if(tri.vertices[j].pos.z < 0)
        {
            oobw[pc++] = j;
        }
        else
        {
            nob = j;
        }
    }


    float4 r1;
    float4 r2;

    if(pc==2)
    {
        get_intersection(tri.vertices[oobw[0]].pos, tri.vertices[nob].pos, &r1);
        get_intersection(tri.vertices[oobw[1]].pos, tri.vertices[nob].pos, &r2);
    }
    else if(pc==1)
    {
        get_intersection(tri.vertices[(oobw[0]+1)%3].pos, tri.vertices[oobw[0]].pos, &r1);
        get_intersection(tri.vertices[(oobw[0]+2)%3].pos, tri.vertices[oobw[0]].pos, &r2);
    }*/


    float rconstant=1.0/(x2*y3+x1*(y2-y3)-x3*y2+(x3-x2)*y1);

    int area=calc_third_area(x1, y1, x2, y2, x3, y3, 0, 0, 0);//



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

    ///(Bx - Ax) * (Cy - Ay) - (By - Ay) * (Cx - Ax)





    float x = x1 + x2 + x3;
    x/=3.0f;
    float y = y1 + y2 + y3;
    y/=3.0f;


    int sg[3];

    for(int i=0; i<3; i++)
    {
        float ax = tri.vertices[i % 3].pos.x;
        float ay = tri.vertices[i % 3].pos.y;
        float bx = tri.vertices[(i+1) % 3].pos.x;
        float by = tri.vertices[(i+1) % 3].pos.y;

        sg[i] = sign((bx - ax) * (y - ay) - (by - ay) * (x - ax));
        C.sg[i] = sg[i];
    }


    //float area = calc_third_areas(&C, x, y);


    C.area=area;

    /*C.s1=r1;
    C.s2=r2;*/


    return C;
}

int backface_cull_expanded(float4 p0, float4 p1, float4 p2, int fov, float width, float height)
{
    float4 arr[3];
    arr[0] = p0;
    arr[1] = p1;
    arr[2] = p2;

    float4 p1p0=p1-p0;
    float4 p2p0=p2-p0;

    float4 cr=cross(p1p0, p2p0);

    int cond=0;

    if(cr.z < 0)
    {
        cond=1;
    }

    return cond;
}

int backface_cull(struct triangle *tri, int fov, float w, float h)
{
    return backface_cull_expanded(tri->vertices[0].pos, tri->vertices[1].pos, tri->vertices[2].pos, fov, w, h);
}


struct t_c full_rotate_n_global(__global struct triangle *triangle, float4 c_pos, float4 c_rot, struct interp_container *container, float odepth[3], float fovc, int width, int height)
{

    __global struct triangle *T=triangle;

    float4 rotpoints[3];
    rotpoints[0]=rot(T->vertices[0].pos, c_pos, c_rot);
    rotpoints[1]=rot(T->vertices[1].pos, c_pos, c_rot);
    rotpoints[2]=rot(T->vertices[2].pos, c_pos, c_rot);

    float4 nought={0,0,0,0};
    float4 normalrot[3];
    normalrot[0]=rot(T->vertices[0].normal, nought, c_rot);
    normalrot[1]=rot(T->vertices[1].normal, nought, c_rot);
    normalrot[2]=rot(T->vertices[2].normal, nought, c_rot);

    float4 prerot[3];

    for(int j=0; j<3; j++)
    {
        prerot[j] = rotpoints[j];
    }

    for(int j=0; j<3; j++)
    {
        float rx;
        rx=(rotpoints[j].x) * (fovc/(rotpoints[j].z));
        float ry;
        ry=(rotpoints[j].y) * (fovc/(rotpoints[j].z));

        rx+=width/2;
        ry+=height/2;

        rotpoints[j].x=rx;
        rotpoints[j].y=ry;
        odepth[j]=rotpoints[j].z;
        rotpoints[j].z=dcalc(rotpoints[j].z);
    }



    struct triangle ret;


    for(int i=0; i<3; i++)
    {
        ret.vertices[i]=T->vertices[i];

        ret.vertices[i].pos.x = rotpoints[i].x;
        ret.vertices[i].pos.y = rotpoints[i].y;
        ret.vertices[i].pos.z = rotpoints[i].z;

        ret.vertices[i].pad.x = triangle->vertices[i].pad.x;
        ret.vertices[i].pad.y = triangle->vertices[i].pad.y;

        ret.vertices[i].vt.x  = triangle->vertices[i].vt.x;
        ret.vertices[i].vt.y  = triangle->vertices[i].vt.y;

        ret.vertices[i].normal= triangle->vertices[i].normal; ///I don't think this is correct, however, it seems to run. Investigate this at some point
    }

    ///problem lies here ish



    float4 preperp[3]; ///pre perspective projection

    for(int i=0; i<3; i++)
    {
        /*preperp[i].x = (rotpoints[i].x-width/2)*(odepth[i])/fovc;
        preperp[i].y = (rotpoints[i].y-height/2)*(odepth[i])/fovc;
        preperp[i].z = odepth[i];*/
        preperp[i] = prerot[i];
    }




    int n_behind = 0;
    int w_behind[3]={0,0,0};
    int id_behind=-1;
    int id_behind_2 = -1;
    int id_valid=-1;

    //int table[3];

    for(int i=0; i<3; i++)
    {
        if(preperp[i].z <= depth_icutoff)
        {
            n_behind++;

            if(n_behind==1)
                id_behind = i;
            else if(n_behind==2)
                id_behind_2 = i;
        }
        else
        {
            id_valid = i;
        }
    }

    /*int b = backface_cull_expanded(rotpoints[0], rotpoints[1], rotpoints[2], fovc, width, height);

    if(b!=1 && n_behind==0)
    {
        struct t_c tr;
        tr.c = 4;
        return tr;
    }*/

    float4 p1, p2, c1, c2;

    if(n_behind > 0)
    {
        if(n_behind==1)
        {
            ///find intersections and shit between id and other two id and shit, simply z = dcalc(50) and do a + l*(b-a)
            int n0 = id_behind;
            int v1 = (id_behind + 1) % 3;
            int v2 = (id_behind + 2) % 3;
            ///the two valid coordinates

            ///find intersection of preperp with depth_icutoff

            ///eq 3 = preperp[v1].z + l*(preperp[n0].z - preperp[v1].z) = idepth_cutoff
            /// l = idepth_cutoff -preperp[v1].z / (preperp[n0].z - preperp[v1].z) = l
            ///other coords = preperp[v1] + l*(preperp[n0] - preperp[v1])

            float l1 = (depth_icutoff - preperp[v1].z) / (preperp[n0].z - preperp[v1].z);
            float l2 = (depth_icutoff - preperp[v2].z) / (preperp[n0].z - preperp[v2].z);

            p1 = preperp[v1] + l1*(preperp[n0] - preperp[v1]);
            p2 = preperp[v2] + l2*(preperp[n0] - preperp[v2]);

            c1 = preperp[v1];
            c2 = preperp[v2];
        }
        if(n_behind==2)
        {
            int n0 = id_behind;
            int n1 = id_behind_2;

            //int n0 = (id_valid + 1) % 3;
            //int n1 = (id_valid + 2) % 3;
            int v1 = id_valid;

            float l1, l2;
            l1 = (depth_icutoff - preperp[n0].z) / (preperp[v1].z - preperp[n0].z);
            l2 = (depth_icutoff - preperp[n1].z) / (preperp[v1].z - preperp[n1].z);




            p1 = preperp[n0] + l1*(preperp[v1] - preperp[n0]);
            p2 = preperp[n1] + l2*(preperp[v1] - preperp[n1]);

            //p1.z = (depth_icutoff);
            //p2.z = (depth_icutoff);

            c1 = preperp[v1];

            /*if(isnan(l1)||isnan(l2))
            {
                p1 = preperp[0];
                p2 = preperp[1];
                c1 = preperp[2];

            }*/
        }
    }

    p1.x = (p1.x * fovc / p1.z) + width/2;
    p1.y = (p1.y * fovc / p1.z) + height/2;
    p1.z = dcalc(p1.z);


    p2.x = (p2.x * fovc / p2.z) + width/2;
    p2.y = (p2.y * fovc / p2.z) + height/2;
    p2.z = dcalc(p2.z);


    c1.x = (c1.x * fovc / c1.z) + width/2;
    c1.y = (c1.y * fovc / c1.z) + height/2;
    c1.z = dcalc(c1.z);


    c2.x = (c2.x * fovc / c2.z) + width/2;
    c2.y = (c2.y * fovc / c2.z) + height/2;
    c2.z = dcalc(c2.z);


    struct t_c t;


    if(n_behind==0)
    {
        t.t[0] = ret;
        t.c=1;
    }
    if(n_behind==1)
    {
        t.t[0] = ret;
        t.t[1] = ret;
        t.c=2;

        t.t[0].vertices[0].pos = p1;
        t.t[0].vertices[1].pos = c1;
        t.t[0].vertices[2].pos = c2;

        t.t[1].vertices[0].pos = p1;
        t.t[1].vertices[1].pos = c2;
        t.t[1].vertices[2].pos = p2;

        //t.c=4;
    }
    if(n_behind==2)
    {
        t.t[0] = ret;
        /*t.t[0].vertices[0].pos = p1;
        t.t[0].vertices[1].pos = p2;
        t.t[0].vertices[2].pos = c1;*/

        t.t[0].vertices[id_behind].pos = p1;
        t.t[0].vertices[id_behind_2].pos = p2;
        t.t[0].vertices[id_valid].pos = c1;
        t.c=3;

    }
    if(n_behind>2)
    {
        t.t[0] = ret;
        //t.c=1;
        t.c=4;
    }

    //t.c = 1;
    //t.t[0] = ret;


    *container = construct_interpolation(ret, width, height);

    return t;
}


struct t_c full_rotate(__global struct triangle *triangle, __global float4 *c_pos, __global float4 *c_rot, struct interp_container *container, float odepth[3], float fovc, int width, int height)
{
    return full_rotate_n_global(triangle, *c_pos, *c_rot, container, odepth, fovc, width, height);
}





////all texture code was not rewritten for time, does not use proper functions
float4 read_tex_array(float4 coords, uint tid, global uint *num, global uint *size, __read_only image3d_t array)
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


float return_bilinear_shadf(float2 coord, float values[4])
{
    float mx, my;
    mx = coord.x*1 - 0.5;
    my = coord.y*1 - 0.5;
    float2 uvratio = {mx - floor(mx), my - floor(my)};
    float2 buvr = {1.0-uvratio.x, 1.0-uvratio.y};

    float result;
    result=(values[0]*buvr.x + values[1]*uvratio.x)*buvr.y + (values[2]*buvr.x + values[3]*uvratio.x)*uvratio.y;

    return result;

    //result.y=(values[0].y*buvr.x + values[1].y*uvratio.x)*buvr.y + (values[2].y*buvr.x + values[3].y*uvratio.x)*uvratio.y;
    //result.z=(values[0].z*buvr.x + values[1].z*uvratio.x)*buvr.y + (values[2].z*buvr.x + values[3].z*uvratio.x)*uvratio.y;

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

float4 texture_filter(struct triangle* c_tri, int2 spos, float4 vt, float depth, float4 c_pos, float4 c_rot, int tid2, global uint* mipd , global uint *nums, global uint *sizes, __read_only image3d_t array)
{

    ///find z coord of texture pixel, work out distance of transition between the adjacent 2 texture levels (+ which 2 texture levels), bilinearly interpolate the two pixels, then interpolate the result

    //global struct triangle *tri;
    //tri=&triangles[id];
    struct triangle *tri=c_tri;

    int width=SCREENWIDTH;
    int height=SCREENHEIGHT;

    //depth = 0.01;

    float4 rotpoints[3];
    rotpoints[0]=c_tri->vertices[0].pos;
    rotpoints[1]=c_tri->vertices[1].pos;
    rotpoints[2]=c_tri->vertices[2].pos;

    //float adepth=(rotpoints[0].z + rotpoints[1].z + rotpoints[2].z)/3.0f;

    float adepth = idcalc(depth);

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
            mdist=(pow(2.0f, (float)i) - 1)*mipdistance;
            fdist=(pow(2.0f, (float)i) - 1)*mipdistance;
            break;
        }

        mdist=(pow(2.0f, (float)i) - 1)*mipdistance;
        fdist=(pow(2.0f, (float)i+1.0f) - 1)*mipdistance;

        if(corrected_depth > mdist && corrected_depth < fdist)
        {
            break;
        }
    }

    //every FOV_CONST, the texture size halves


    wc1=wc+1;
    part=(fdist-corrected_depth)/(fdist-mdist);

    if(wc==4)
    {
        wc1=4;
        part=1;
    }


    //vt.x = 0.5;
    //vt.y = 0.5;


    ///how far away from the upper mip level are we, between 0 and 1;
    //x, y, tid, num, size, array;

    float2 vtm = {vt.x, vt.y};

    if(vtm.x > 1)
    {
        while(vtm.x > 1)
        {
            vtm.x -= 1;
        }
        vtm.x = 1.0 - vtm.x;
    }

    if(vtm.x < 0)
    {
        while(vtm.x < 0)
        {
            vtm.x += 1;
        }
        vtm.x = 1.0 - vtm.x;
    }

    if(vtm.y > 1)
    {
        while(vtm.y > 1)
        {
            vtm.y -= 1;
        }
        vtm.y = 1.0 - vtm.y;
    }

    if(vtm.y < 0)
    {
        while(vtm.y < 0)
        {
            vtm.y += 1;
        }
        vtm.y = 1.0 - vtm.y;
    }


    float4 coord={vtm.x, vtm.y, 0, 0};


    float4 col1=return_bilinear_col(coord, tids[wc], nums, sizes, array);

    float4 col2=return_bilinear_col(coord, tids[wc1], nums, sizes, array);

    float4 final=col1*(part) + col2*(1-part);

    //float4 final = read_tex_array(coord, tids[wc], nums, sizes, array);

    return final;
}
///end of unrewritten code

///Shadow mapping works like this
///Draw scene from perspective of light. Build depth buffer for light
///Rotate triangles in part2 around light. If < depth buffer, that means that they are occluded.
///Hmm. Regular projection only gives spotlight, need to project world onto sphere and unravel onto square

///use xz for x coordinates of plane from sphere, use xy for y coordinates of plane from sphere
///normalise coordinates to get the location on sphere
///use old magnitude as depth from sphere

///Actually is cubemap


///If one point loops round the 0 to 2PI, works out this by checking if any points are more thatn 180 away from each other and returns the closest, possible negative or >2pi value. Then we can simply reverse pixel direction
/*float return_180sideclamp(float a, float b, float c)
{
    float p[3]={a, b, c};
    float sorted[3];
    sorted[0]=min3(a, b, c);
    sorted[1]=max3(a, b, c);
    for(int i=0; i<3; i++)
    {
        if(p[i]!=sorted[0] && p[i]!=sorted[1])
        {
            sorted[2]=p[i];
            break;
        }
    }

}*/
///redundant, using cube mapping instead

int ret_cubeface(float4 point, float4 light)
{

    float4 r_struct[6];
    r_struct[0]=(float4){0.0,            0.0,            0.0,0.0};
    r_struct[1]=(float4){M_PI/2.0,       0.0,            0.0,0.0};
    r_struct[2]=(float4){0.0,            M_PI,           0.0,0.0};
    r_struct[3]=(float4){3.0*M_PI/2.0,   0.0,            0.0,0.0};
    r_struct[4]=(float4){0.0,            3.0*M_PI/2.0,   0.0,0.0};
    r_struct[5]=(float4){0.0,            M_PI/2.0,       0.0,0.0};

    float4 r_pl;
    r_pl=point-light;

    float4 pnormal;
    pnormal = (float4){0, 1, 0, 0};


    //return 0;

    float angle = atan2(r_pl.y, r_pl.x);
    ///need to find angle between line to point and thing, not otherwise broken
    ///
    ///acos(m(n.u)/m(n)*m(u))

    angle = acos(length(dot(pnormal, r_pl))/(length(pnormal)*length(r_pl)));

    if(angle < 0)
    {
        angle = M_PI - fabs(angle) + M_PI;
    }

    angle = angle + M_PI/4.0f;

    if(angle < 0)
    {
        angle = angle + 2.0f*M_PI;
    }


    if(angle > 0 && angle < M_PI/2.0f)
    {
        return 1;
    }

    if(angle < 2.0f*M_PI && angle > 3.0f*M_PI/2.0f)
    {
        return 3;
    }

    /*if(angle >= M_PI/2.0f && angle < M_PI)
    {
        return 1;
    }

    if(angle >= 3.0f*M_PI/2.0f && angle < 2.0f*M_PI)
    {
        return 3;
    }*/



    float zangle = atan2(r_pl.z, r_pl.x);

    if(zangle < 0)
    {
        zangle = M_PI - fabs(zangle) + M_PI;
    }

    zangle = zangle + M_PI/4.0f;

    if(zangle < 0)
    {
        zangle = zangle + 2.0f*M_PI;
    }

    //zangle = 2*M_PI-zangle;

    if(zangle >= 0 && zangle < M_PI/2.0f)
    {
        return 5;
    }
    if(zangle >= M_PI/2.0f && zangle < M_PI)
    {
        return 0;
    }
    if(zangle >= M_PI && zangle < 3*M_PI/2.0f)
    {
        return 4;
    }
    else
    {
        return 2;
        //return 0;
    }

}

//screenspace triangles




float get_horizon_direction_depth(int2 start, float2 dir, uint nsamples, __global uint * depth_buffer, float cdepth, float radius)
{
    float h=cdepth;

    uint p = 0;
    uint e = 0;


    //float2 rdir = {1, 1};

    float2 ndir = (normalize(dir)*radius/nsamples);

    float y = start.y + ndir.y, x = start.x + ndir.x;
    p=0;

    for(; p < nsamples; y+=ndir.y, x += ndir.x, p++)
    {
        if ((int)round(x) < 0 || (int)round(x) >= SCREENWIDTH || (int)round(y) < 0 || (int)round(y) >= SCREENHEIGHT)
        {
            return h;
        }

        float dval = (float)depth_buffer[((int)round(y))*SCREENWIDTH + (int)round(x)]/mulint;
        dval = idcalc(dval);

        if(dval < h  && fabs(dval - cdepth) < radius)
        {
            h = dval;
        }
    }


    return h;

}

float generate_hbao(struct triangle* tri, int2 spos, __global uint *depth_buffer)
{

    float depth = (float)depth_buffer[spos.y * SCREENWIDTH + spos.x]/mulint;

    depth = idcalc(depth);
    ///depth is linear between 0 and 1
    //now, instead of taking the horizon because i'm not entirely sure how to calc that, going to use highest point in filtering

    float radius = 8.0; //AO radius

    //radius = radius / (dcalc(depth); ///err?
    //radius = radius / (idcalc(depth));
    //radius = radius * FOV_CONST / (depth);

    if(radius < 1)
    {
        radius = 1;
    }

    ///using 4 uniform sample directions like a heretic, with 4 samples from each direction

    uint ndirsamples = 4;

    int ndirs = 4;

    float2 directions[8] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}, {0, 1}, {0, -1}, {-1, 0}, {1, 0}};

    float distance = radius;


    ///get face normal

    float4 p0={tri->vertices[0].pos.x, tri->vertices[0].pos.y, tri->vertices[0].pos.z, 0};
    float4 p1={tri->vertices[1].pos.x, tri->vertices[1].pos.y, tri->vertices[1].pos.z, 0};
    float4 p2={tri->vertices[2].pos.x, tri->vertices[2].pos.y, tri->vertices[2].pos.z, 0};

    float4 p1p0=p1-p0;
    float4 p2p0=p2-p0;

    float4 cr=cross(p1p0, p2p0);

    ///my sources tell me this is the right thing. Ie stolen from backface culling. Right. the tangent to that is -1.0/it

    float4 tang = -1.0/cr;

    //tang = normalize(tang);

    //tang = -1.0/normal;


    float tangle = atan2(tang.z, length((float2){tang.x, tang.y}));



    float accum=0;
    int t=0;
    for(int i=0; i<ndirs; i++)
    {
        float cdepth = (float)depth_buffer[spos.y*SCREENWIDTH + spos.x]/mulint;
        cdepth = idcalc(cdepth);

        float h = get_horizon_direction_depth(spos, directions[i], ndirsamples, depth_buffer, cdepth, radius);

        //float tangle = atan2(tang.z, );

        float angle = atan2(fabs(h - cdepth), distance);

        if(angle > 0.05)
        {
            accum += max(sin(angle), 0.0);// + max(sin(tangle), 0.0);
            //accum += max(sin(tangle), 0.0);
        }

    }

    accum/=ndirs;

    if(accum > 1)
    {
        accum = 1;
    }

    if(accum < 0)
    {
        accum = 0;
    }

    return accum;

}

///ONLY HARD SHADOWS ONLY ONLY___

float generate_hard_occlusion(float4 spos, float4 normal, float actual_depth, __global struct light* lights, __global uint* light_depth_buffer, __global float4* c_pos, __global float4* c_rot, int i, int shnum)
{

        float4 local_position={((spos.x - SCREENWIDTH/2.0f)*actual_depth/FOV_CONST), ((spos.y - SCREENHEIGHT/2.0f)*actual_depth/FOV_CONST), actual_depth, 0};
        float4 local_position_off={((spos.x + 1 - SCREENWIDTH/2.0f)*actual_depth/FOV_CONST), ((spos.y + 1 - SCREENHEIGHT/2.0f)*actual_depth/FOV_CONST), actual_depth, 0};



        float4 zero = {0,0,0,0};





        float4 lightaccum={0,0,0,0};

        int occnum=0;

        float ddepth=0;

        float occamount=0;


        float4 lpos=lights[i].pos;


        float thisocc=0;

        int smoothskip=0;



        if(lights[i].shadow==1)
        {


            float4 r_struct[6];
            r_struct[0]=(float4){0.0,            0.0,            0.0,0.0};
            r_struct[1]=(float4){M_PI/2.0,       0.0,            0.0,0.0};
            r_struct[2]=(float4){0.0,            M_PI,           0.0,0.0};
            r_struct[3]=(float4){3.0*M_PI/2.0,   0.0,            0.0,0.0};
            r_struct[4]=(float4){0.0,            3.0*M_PI/2.0,   0.0,0.0};
            r_struct[5]=(float4){0.0,            M_PI/2.0,       0.0,0.0};

            float4 global_position = rot(local_position,  zero, (float4){-c_rot->x, 0.0, 0.0, 0.0});
            global_position        = rot(global_position, zero, (float4){0.0, -c_rot->y, 0.0, 0.0});


            global_position.x += c_pos->x;
            global_position.y += c_pos->y;
            global_position.z += c_pos->z;


            float odepth[3];

            struct interp_container icontainer;


            uint ldepth_map_id = ret_cubeface(global_position, lpos);
            float4 *rotation = &r_struct[ldepth_map_id];

            float4 local_pos = rot(global_position, lpos, *rotation);

            float4 postrotate_pos;
            postrotate_pos.x = local_pos.x * LFOV_CONST/local_pos.z;
            postrotate_pos.y = local_pos.y * LFOV_CONST/local_pos.z;
            postrotate_pos.z = local_pos.z;


            ///find the absolute distance as an angle between 0 and 1 that would be required to make it backface, that approximates occlusion
            float err;
            if((err=dot(fast_normalize(rot(normal, zero, *rotation)), fast_normalize(postrotate_pos))) >= 0)
            {
                smoothskip=1; ///we only want hard shadows
            }

            err = fabs(err);


            postrotate_pos.z = dcalc(postrotate_pos.z);

            float dpth=postrotate_pos.z;
            uint idepth = dpth*mulint;

            postrotate_pos.x += LIGHTBUFFERDIM/2.0f;
            postrotate_pos.y += LIGHTBUFFERDIM/2.0f;


            __global uint* ldepth_map = &light_depth_buffer[(ldepth_map_id + shnum*6)*LIGHTBUFFERDIM*LIGHTBUFFERDIM];


            if(postrotate_pos.y < 1 || postrotate_pos.y >= LIGHTBUFFERDIM-1 || postrotate_pos.x < 1 || postrotate_pos.x >= LIGHTBUFFERDIM-1)
            {
                return 0;
            }

            float ldp = ((float)ldepth_map[(int)floor(postrotate_pos.y)*LIGHTBUFFERDIM + (int)floor(postrotate_pos.x)]/mulint);

            float near[4];


            int2 sws[4] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};

            near[0] = (float)ldepth_map[(int)floor(postrotate_pos.y + sws[0].y)*LIGHTBUFFERDIM + (int)floor(postrotate_pos.x + sws[0].x)]/mulint;
            near[1] = (float)ldepth_map[(int)floor(postrotate_pos.y + sws[1].y)*LIGHTBUFFERDIM + (int)floor(postrotate_pos.x + sws[1].x)]/mulint;
            near[2] = (float)ldepth_map[(int)floor(postrotate_pos.y + sws[2].y)*LIGHTBUFFERDIM + (int)floor(postrotate_pos.x + sws[2].x)]/mulint;
            near[3] = (float)ldepth_map[(int)floor(postrotate_pos.y + sws[3].y)*LIGHTBUFFERDIM + (int)floor(postrotate_pos.x + sws[3].x)]/mulint;

            int2 corners[4] = {{1, 1}, {-1, 1}, {-1, -1}, {1, -1}};

            float cnear[4];

            cnear[0] = (float)ldepth_map[(int)floor(postrotate_pos.y + corners[0].y)*LIGHTBUFFERDIM + (int)floor(postrotate_pos.x + corners[0].x)]/mulint;
            cnear[1] = (float)ldepth_map[(int)floor(postrotate_pos.y + corners[1].y)*LIGHTBUFFERDIM + (int)floor(postrotate_pos.x + corners[1].x)]/mulint;
            cnear[2] = (float)ldepth_map[(int)floor(postrotate_pos.y + corners[2].y)*LIGHTBUFFERDIM + (int)floor(postrotate_pos.x + corners[2].x)]/mulint;
            cnear[3] = (float)ldepth_map[(int)floor(postrotate_pos.y + corners[3].y)*LIGHTBUFFERDIM + (int)floor(postrotate_pos.x + corners[3].x)]/mulint;

            float pass_arr[4] = {0,0,0,0};
            float cpass_arr[4] = {0,0,0,0};



            ///change of plan, shadows want to be static and fast at runtime, therefore going to sink the generate time into memory not runtime filtering


            int depthpass=0;
            int cdepthpass=0; //corners
            float len = dcalc(4);

            for(int i=0; i<4; i++)
            {
                pass_arr[i]=0.0;
                if(dpth > near[i] + len)
                {
                    depthpass++;
                    pass_arr[i] = 1;
                }
            }

            for(int i=0; i<4; i++)
            {
                cpass_arr[i] = 0.0;
                if(dpth > cnear[i] + len)
                {
                    cdepthpass++;
                    cpass_arr[i] = 1;
                }
            }


            if((depthpass > 3) && dpth > ldp + len)
            {

                float fx = postrotate_pos.x - floor(postrotate_pos.x);
                float fy = postrotate_pos.y - floor(postrotate_pos.y);

                float dx1 = fx * cpass_arr[3] + (1.0-fx) * cpass_arr[2];
                float dx2 = fx * cpass_arr[0] + (1.0-fx) * cpass_arr[1];
                float fin = fy * dx2 + (1.0-fy) * dx1;


                occamount+=fin;
            }
            else if (depthpass > 0 && dpth > ldp + len)
            {
                float fx = postrotate_pos.x - floor(postrotate_pos.x);
                float fy = postrotate_pos.y - floor(postrotate_pos.y);

                float dx = fx*pass_arr[2] + (1.0-fx)*pass_arr[3];
                float dy = fy*pass_arr[0] + (1.0-fy)*pass_arr[1];

                occamount += dx*dy;
            }
        }

        return occamount;
}



__kernel void construct_smap(__global struct triangle* triangles, __global uint* tri_num, __global uint* light_depth_buffer, __global uint* lnum, __global struct light *lights)
{

    ///there are 6 pieces on my cubemap
    ///6 slices form a light's buffer, each slice is defined in the order up down left foward right back



    ///rotate triangles into screenspace, clip, and probably perform rough hierarchical depth buffering
    ///lighting normals give you backface culling

    uint id = get_global_id(0);

    struct interp_container icontainer;

    __global float4 *l_pos = &lights[*lnum].pos;


    int valid_tri=0;

    if(id >= *tri_num)
    {
        return;
    }


    __global struct triangle *c_tri = &triangles[id];

    float4 r_struct[6];
    r_struct[0]=(float4){0.0,            0.0,            0.0,0.0};
    r_struct[1]=(float4){M_PI/2.0,       0.0,            0.0,0.0};
    r_struct[2]=(float4){0.0,            M_PI,           0.0,0.0};
    r_struct[3]=(float4){3.0*M_PI/2.0,   0.0,            0.0,0.0};
    r_struct[4]=(float4){0.0,            3.0*M_PI/2.0,   0.0,0.0};
    r_struct[5]=(float4){0.0,            M_PI/2.0,       0.0,0.0};


    int bslice=0;




    if(ret_cubeface(c_tri->vertices[0].pos, *l_pos)!=ret_cubeface(c_tri->vertices[1].pos, *l_pos) || ret_cubeface(c_tri->vertices[1].pos, *l_pos)!=ret_cubeface(c_tri->vertices[2].pos, *l_pos) ||ret_cubeface(c_tri->vertices[0].pos, *l_pos)!=ret_cubeface(c_tri->vertices[2].pos, *l_pos))
    {
        return;
    }

    bslice = ret_cubeface(c_tri->vertices[0].pos, *l_pos);

    float4 l_arot = r_struct[bslice];


    float odepth[3];
    int cont=0;

    struct t_c t=full_rotate_n_global(&triangles[id], *l_pos, l_arot, &icontainer, odepth, LFOV_CONST, LIGHTBUFFERDIM, LIGHTBUFFERDIM);
    struct triangle tri = t.t[0];
    //struct triangle tri=full_rotate(&triangles[id], l_pos, l_rot, &icontainer, odepth, LFOV_CONST, LIGHTBUFFERDIM, LIGHTBUFFERDIM);


    if(icontainer.ybounds[1]-icontainer.ybounds[0] > MTRI_SIZE || icontainer.xbounds[1] - icontainer.xbounds[0] > MTRI_SIZE)
    {
        return;
    }


    ///near plane intersection


    if((tri.vertices[0].pos.z) < dcalc(depth_icutoff) && (tri.vertices[1].pos.z) < dcalc(depth_icutoff) && (tri.vertices[2].pos.z) < dcalc(depth_icutoff))
    {
        return;
    }

    ///begin backface culling!



    if(backface_cull(&tri, LFOV_CONST, LIGHTBUFFERDIM, LIGHTBUFFERDIM)!=1)
    {
        return;
    }


    ///end backface

    ///OOB check can go here

    ///begin drawing to depth buffer

    float depths[3]={tri.vertices[0].pos.z, tri.vertices[1].pos.z, tri.vertices[2].pos.z};
    //float depths[3]={odepth[0], odepth[1], odepth[2]};

    for(int y=icontainer.ybounds[0]; y<icontainer.ybounds[1]; y++)
    {
        for(int x=icontainer.xbounds[0]; x<icontainer.xbounds[1]; x++)
        {
            float s1=calc_third_areas(&icontainer, x, y);

            if(s1 > icontainer.area - 2 && s1 < icontainer.area + 2)
            {
                __global uint *ft=&light_depth_buffer[(bslice)*LIGHTBUFFERDIM*LIGHTBUFFERDIM + ((*lnum)*6)*LIGHTBUFFERDIM*LIGHTBUFFERDIM +  y*LIGHTBUFFERDIM + x];

                ///slice*ldbm^2 + lnum*6*lbdm^2 + y*ldbm + x


                float fmydepth=interpolate(depths, &icontainer, x, y);

                //fmydepth = dcalc(fmydepth);

                uint mydepth=fmydepth*mulint;



                /*if(mydepth==0)
                {
                    continue;
                }*/

                if(mydepth!=0)
                {
                    atomic_min(ft, mydepth);
                }
            }
        }
    }
    ///end drawing to depth buffer

}

__constant int op_size = 50;

__kernel void prearrange(__global struct triangle* triangles, __global uint* tri_num, __global float4* c_pos, __global float4* c_rot, __global uint* fragment_id_buffer, __global uint* id_buffer_maxlength, __global uint* id_buffer_atomc)
{
    uint id = get_global_id(0);

    if(id >= *tri_num)
    {
        return;
    }

    __global struct triangle *T=&triangles[id];

    struct interp_container icontainer;

    float odepth[3];

    struct t_c t = full_rotate(T, c_pos, c_rot, &icontainer, odepth, FOV_CONST, SCREENWIDTH, SCREENHEIGHT);

    if(t.c==4)
    {
        return;
    }
    /*struct triangle tri = t.t[0];
    tri.vertices[0].pos.z = odepth[0];
    tri.vertices[1].pos.z = odepth[1];
    tri.vertices[2].pos.z = odepth[2];*/


    int ooany[2];

    ooany[0] = backface_cull(&t.t[0], FOV_CONST, SCREENWIDTH, SCREENHEIGHT);

    if(t.c==2)
        ooany[1] = backface_cull(&t.t[1], FOV_CONST, SCREENWIDTH, SCREENHEIGHT);





    int nct = 1;
    if(t.c==2)
    {
        nct=2;
    }
    for(int j=0; j<nct; j++)
    {
        //ooany[j]=1;
        int oob = 0;
        int ooxmin = 0;
        int ooxmax = 0;
        int ooymin = 0;
        int ooymax = 0;

        for(int i=0; i<3; i++)
        {
            if(t.t[j].vertices[i].pos.z < dcalc(depth_icutoff))
            {
                oob++;
            }
            if(t.t[j].vertices[i].pos.x < 0)
            {
                ooxmin++;
            }
            if(t.t[j].vertices[i].pos.x >= SCREENWIDTH)
            {
                ooxmax++;
            }
            if(t.t[j].vertices[i].pos.y < 0)
            {
                ooymin++;
            }
            if(t.t[j].vertices[i].pos.y >= SCREENHEIGHT)
            {

                ooymax++;
            }
        }
        if(ooxmin >= 3 || ooxmax >= 3 || ooymin >= 3 || ooymax >= 3 || oob >=3)
        {
            ooany[j]=0;
        }
    }


    ///how many pixels does each thread want to render?

    //uint op_size = 1000;

    int loop_c=0;

    if(t.c==1)
    {
        loop_c=1;
    }
    if(t.c==2)
    {
        loop_c=2;
    }
    if(t.c==3)
    {
        loop_c=1;
    }






    for(uint i=0; i<loop_c; i++)
    {
        if(ooany[i]!=1)
        {
            continue;
        }

        int minx, maxx, miny, maxy;

        struct interp_container ic = construct_interpolation(t.t[i], SCREENWIDTH, SCREENHEIGHT);

        minx = ic.xbounds[0];
        maxx = ic.xbounds[1];
        miny = ic.ybounds[0];
        maxy = ic.ybounds[1];

        int area = (maxx-minx+1)*(maxy-miny+1);

        float thread_num = ceil((float)area/op_size);


        if(id == 203279)
        {
            //thread_num *=200;
        }

        uint b = atom_add(id_buffer_atomc, (uint)thread_num);

        int c = 0;

        if(b*3 + thread_num*3 < *id_buffer_maxlength)
        {
            for(uint a = b; a < b + thread_num; a++)
            {
                fragment_id_buffer[a*3] = id;
                fragment_id_buffer[a*3+1] = c;
                fragment_id_buffer[a*3+2] = i;
                c++;
            }

        }
    }

}


__kernel void part1(__global struct triangle* triangles, __global uint* fragment_id_buffer, __global uint* tri_num, __global float4* c_pos, __global float4* c_rot, __global uint* depth_buffer, __global uint* f_len, __global struct triangle* debug)
{
    uint id = get_global_id(0);

    if(id >= *f_len)
    {
        return;
    }

    uint tid = fragment_id_buffer[id*3];

    __global struct triangle *T = &triangles[tid];



    struct interp_container icontainer;


    float odepth[3];

    uint distance = 0;

    distance = fragment_id_buffer[id*3 + 1];

    int wtri = fragment_id_buffer[id*3 + 2];



    struct t_c t = full_rotate(T, c_pos, c_rot, &icontainer, odepth, FOV_CONST, SCREENWIDTH, SCREENHEIGHT);


    struct interp_container ic_b;

    ic_b = construct_interpolation(t.t[wtri], SCREENWIDTH, SCREENHEIGHT);

    struct triangle tri = t.t[wtri];




    int width  = ic_b.xbounds[1] - ic_b.xbounds[0];
    int height = ic_b.ybounds[1] - ic_b.ybounds[0];



    int pixel_along = op_size*distance;


    int xstart = ic_b.xbounds[0];
    int ystart = ic_b.ybounds[0];


    float depths[3]={1.0/tri.vertices[0].pos.z, 1.0/tri.vertices[1].pos.z, 1.0/tri.vertices[2].pos.z};

    int pcount=0;

    int ncount=0;

    while(pcount <= op_size)
    {
        int x = ((pixel_along + pcount) % width) + xstart;
        int y = ((pixel_along + pcount) / width) + ystart;

        if(y < 0 || y >= SCREENHEIGHT)
        {
            pcount++;
            continue;
        }

        //

        /*bool iswithin=true;

        int sg[3]={0};

        for(int i=0; i<3; i++)
        {
            float ax = tri.vertices[i % 3].pos.x;
            float ay = tri.vertices[i % 3].pos.y;
            float bx = tri.vertices[(i+1) % 3].pos.x;
            float by = tri.vertices[(i+1) % 3].pos.y;

            float pb = sign((bx - ax) * ((float)y - ay) - (by - ay) * ((float)x - ax));

            sg[i] = pb;

            if(sg[i]!=ic_b.sg[i] && fabs(pb)>1)
            {
                iswithin=false;
            }
        }

        if(iswithin)*/


        /*bool iswithin = false;

        ///http://www.blackpawn.com/texts/pointinpoly/
        float4 v04 = tri.vertices[2].pos - tri.vertices[0].pos;
        float4 v14 = tri.vertices[1].pos - tri.vertices[0].pos;
        float4 v24 = (float4){x, y, 0, 0} - tri.vertices[0].pos;

        float2 v0, v1, v2;
        v0.x = v04.x;
        v0.y = v04.y;
        v1.x = v14.x;
        v1.y = v14.y;
        v2.x = v24.x;
        v2.y = v24.y;

        // Compute dot products
        float dot00 = dot(v0, v0);
        float dot01 = dot(v0, v1);
        float dot02 = dot(v0, v2);
        float dot11 = dot(v1, v1);
        float dot12 = dot(v1, v2);

        // Compute barycentric coordinates
        float invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
        float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

        // Check if point is in triangle
        iswithin =  ((u >= 0) && (v >= 0) && (u + v < 1));


        if(iswithin)*/

        float s1=calc_third_areas(&ic_b, x, y);

        if(s1 > ic_b.area - 2 && s1 < ic_b.area + 2)
        {

            __global uint *ft=&depth_buffer[y*SCREENWIDTH + x];


            float fmydepth=interpolate(depths, &ic_b, x, y);

            fmydepth = 1.0 / fmydepth;

            if(isnan(fmydepth) || fmydepth < dcalc(depth_icutoff) || fmydepth > 1)
            {
                pcount++;
                continue;
            }

            uint mydepth=fmydepth*mulint;

            if(mydepth==0)
            {
                pcount++;
                continue;
            }



            uint sdepth=atomic_min(ft, mydepth);

        }
        pcount++;
    }

    if(tid == 203279)
    {
        *debug = t.t[0];
        //debug->vertices[0].pos.x = ncount;
    }

}


__kernel void part2(__global struct triangle* triangles, __global uint* fragment_id_buffer, __global uint* tri_num, __global uint* depth_buffer, __global uint* id_buffer, __global float4* c_pos, __global float4* c_rot, __global uint* f_len)
{
    uint id=get_global_id(0);

    if(id>=*f_len)
    {
        return;
    }

    uint tid = fragment_id_buffer[id*3];

    int wtri = fragment_id_buffer[id*3 + 2];



    struct interp_container icontainer;

    float odepth[3];

    __global struct triangle *T = &triangles[tid];

    struct t_c t = full_rotate(T, c_pos, c_rot, &icontainer, odepth, FOV_CONST, SCREENWIDTH, SCREENHEIGHT);


    int distance = 0;

    distance = fragment_id_buffer[id*3 + 1];

    struct interp_container ic_b;


    ic_b = construct_interpolation(t.t[wtri], SCREENWIDTH, SCREENHEIGHT);


    struct triangle tri = t.t[wtri];

    //struct triangle* tri = &tri_; ///ease of code reuse


    int width  = ic_b.xbounds[1] - ic_b.xbounds[0];
    int height = ic_b.ybounds[1] - ic_b.ybounds[0];

    int pixel_along = op_size*distance;

    float depths[3]={1.0/(tri.vertices[0].pos.z), 1.0/(tri.vertices[1].pos.z), 1.0/(tri.vertices[2].pos.z)};

    int xstart = ic_b.xbounds[0];
    int ystart = ic_b.ybounds[0];


    int pcount=0;

    while(pcount <= op_size)
    {
        int x = ((pixel_along + pcount) % width) + xstart;
        int y = ((pixel_along + pcount) / width) + ystart;

        if(y < 0 || y >= SCREENHEIGHT)
        {
            pcount++;
            continue;
        }

        //


        /*bool iswithin=true;

        int sg[3]={0};

        for(int i=0; i<3; i++)
        {
            float ax = tri.vertices[i % 3].pos.x;
            float ay = tri.vertices[i % 3].pos.y;
            float bx = tri.vertices[(i+1) % 3].pos.x;
            float by = tri.vertices[(i+1) % 3].pos.y;

            float pb = sign((bx - ax) * ((float)y - ay) - (by - ay) * ((float)x - ax));

            sg[i] = pb;

            if(sg[i]!=ic_b.sg[i] && fabs(pb) > 1)
            {
                iswithin=false;
            }
        }

        if(iswithin)*/

        /*bool iswithin = false;

        ///http://www.blackpawn.com/texts/pointinpoly/
        float4 v04 = tri.vertices[2].pos - tri.vertices[0].pos;
        float4 v14 = tri.vertices[1].pos - tri.vertices[0].pos;
        float4 v24 = (float4){x, y, 0, 0} - tri.vertices[0].pos;

        float2 v0, v1, v2;
        v0.x = v04.x;
        v0.y = v04.y;
        v1.x = v14.x;
        v1.y = v14.y;
        v2.x = v24.x;
        v2.y = v24.y;

        // Compute dot products
        float dot00 = dot(v0, v0);
        float dot01 = dot(v0, v1);
        float dot02 = dot(v0, v2);
        float dot11 = dot(v1, v1);
        float dot12 = dot(v1, v2);

        // Compute barycentric coordinates
        float invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
        float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

        // Check if point is in triangle
        iswithin =  ((u >= 0) && (v >= 0) && (u + v < 1));


        if(iswithin)*/

        float s1=calc_third_areas(&ic_b, x, y);

        if(s1 > ic_b.area - 2 && s1 < ic_b.area + 2)
        {
            __global uint *ft=&depth_buffer[y*SCREENWIDTH + x];


            float fmydepth=interpolate(depths, &ic_b, x, y);
            fmydepth = 1.0 / fmydepth;

            if(isnan(fmydepth) || fmydepth < dcalc(depth_icutoff) || fmydepth > 1)
            {
                pcount++;
                continue;
            }

            uint mydepth=fmydepth*mulint;


            if(mydepth==0)
            {
                pcount++;
                continue;
            }

            if(mydepth > *ft - 50 && mydepth < *ft + 50)
            {
                __global uint *fi=&id_buffer[y*SCREENWIDTH + x];
                *fi=id;
            }

        }
        pcount++;
    }

}



__kernel void part3(__global struct triangle *triangles, __global struct triangle *screen_triangles, __global uint *tri_num, __global uint *anum, __global float4 *c_pos, __global float4 *c_rot, __global uint* depth_buffer, __global uint* id_buffer,
                    __read_only image3d_t array, __write_only image2d_t screen, __global uint *nums, __global uint *sizes, __global struct obj_g_descriptor* gobj, __global uint * gnum, __global uint *lnum, __global struct light *lights, __global uint* light_depth_buffer, __global uint * to_clear, __global uint* fragment_id_buffer)
                    ///__global uint sacrifice_children_to_argument_god
{
    ///widthxheight kernel
    sampler_t sam = CLK_NORMALIZED_COORDS_FALSE |
               CLK_ADDRESS_CLAMP_TO_EDGE        |
               CLK_FILTER_NEAREST;


    uint x=get_global_id(0);
    uint y=get_global_id(1);





    if(x < SCREENWIDTH && y < SCREENHEIGHT)
    {
        __global uint *ftc=&to_clear[y*SCREENWIDTH + x];
        *ftc = mulint;

        __global uint *ft=&depth_buffer[y*SCREENWIDTH + x];
        __global uint *fi=&id_buffer   [y*SCREENWIDTH + x];

        if(*ft==mulint)
        {
            return;
        }



        float4 r_struct[6];
        r_struct[0]=(float4){0.0,            0.0,            0.0,0.0};
        r_struct[1]=(float4){M_PI/2.0,       0.0,            0.0,0.0};
        r_struct[2]=(float4){0.0,            M_PI,           0.0,0.0};
        r_struct[3]=(float4){3.0*M_PI/2.0,   0.0,            0.0,0.0};
        r_struct[4]=(float4){0.0,            3.0*M_PI/2.0,   0.0,0.0};
        r_struct[5]=(float4){0.0,            M_PI/2.0,       0.0,0.0};


        //__global struct triangle *c_tri=&screen_triangles[*fi];

        struct interp_container icontainer;
        float odepth[3];

        __global struct triangle* T = &triangles[fragment_id_buffer[(*fi)*3]];

        struct t_c t = full_rotate(T, c_pos, c_rot, &icontainer, odepth, FOV_CONST, SCREENWIDTH, SCREENHEIGHT);

        uint wtri = fragment_id_buffer[(*fi)*3 + 2];
        struct triangle c_tri_ = t.t[wtri]; ///
        c_tri_.vertices[0].pos.z = odepth[0];
        c_tri_.vertices[1].pos.z = odepth[1];
        c_tri_.vertices[2].pos.z = odepth[2];


        struct triangle *c_tri = &c_tri_; //ease of code reuse from old screenspace days


        uint pid = fragment_id_buffer[(*fi)*3];

        __global struct triangle *g_tri=&triangles[pid];


        int o_id=c_tri->vertices[0].pad.y;



        int4 coord={x, y, 0, 0};

        //uint4 col;
        //col=read_imageui(array, sam, coord);

        uint depth=*ft;
        //uint id=*fi;


        //float cz = idcalc(((float)*ft / mulint));

        float cz[3] = {c_tri->vertices[0].pos.z, c_tri->vertices[1].pos.z, c_tri->vertices[2].pos.z};
        float icz[3] = {1.0/c_tri->vertices[0].pos.z, 1.0/c_tri->vertices[1].pos.z, 1.0/c_tri->vertices[2].pos.z};


        float ldepth = interpolate(icz, &icontainer, x, y);




        float4 vt;


        float xvt[3]={c_tri->vertices[0].vt.x/cz[0], c_tri->vertices[1].vt.x/cz[1], c_tri->vertices[2].vt.x/cz[2]};
        vt.x=interpolate(xvt, &icontainer, x, y);

        float yvt[3]={c_tri->vertices[0].vt.y/cz[0], c_tri->vertices[1].vt.y/cz[1], c_tri->vertices[2].vt.y/cz[2]};
        vt.y=interpolate(yvt, &icontainer, x, y);

        vt /= ldepth;


        float normalsx[3]={c_tri->vertices[0].normal.x/cz[0], c_tri->vertices[1].normal.x/cz[1], c_tri->vertices[2].normal.x/cz[2]};
        float normalsy[3]={c_tri->vertices[0].normal.y/cz[0], c_tri->vertices[1].normal.y/cz[1], c_tri->vertices[2].normal.y/cz[2]};
        float normalsz[3]={c_tri->vertices[0].normal.z/cz[0], c_tri->vertices[1].normal.z/cz[1], c_tri->vertices[2].normal.z/cz[2]};

        float4 normal;

        normal.x=interpolate(normalsx, &icontainer, x, y);
        normal.y=interpolate(normalsy, &icontainer, x, y);
        normal.z=interpolate(normalsz, &icontainer, x, y);

        normal /= ldepth;



        //float naturaldepths[3]={(c_tri->vertices[0].pos.z), (c_tri->vertices[1].pos.z), (c_tri->vertices[2].pos.z)};

        //float namydepth=interpolate(naturaldepths, &icontainer, x, y);

        float namydepth = idcalc ((float)*ft/mulint);


        float actual_depth = namydepth;


        float4 local_position={((x - SCREENWIDTH/2.0f)*actual_depth/FOV_CONST), ((y - SCREENHEIGHT/2.0f)*actual_depth/FOV_CONST), actual_depth, 0};


        float4 zero = {0,0,0,0};


        float4 global_position = rot(local_position,  zero, (float4){-c_rot->x, 0.0, 0.0, 0.0});
        global_position        = rot(global_position, zero, (float4){0.0, -c_rot->y, 0.0, 0.0});

        global_position.x += c_pos->x;
        global_position.y += c_pos->y;
        global_position.z += c_pos->z;



        float4 lightaccum={0,0,0,0};


        int shnum=0;

        int occnum=0;

        float ddepth=0;

        float occamount=0;

        for(int i=0; i<*(lnum); i++)
        {
            float4 lpos=lights[i].pos;

            float4 l2c=lpos-global_position;

            float light=dot(fast_normalize(l2c), fast_normalize(normal));



            float thisocc=0;


            int skip=0;

            float specialocc=0;

            float occ[5];

            float average_occ = 0;


            if(lights[i].shadow==1)
            {


                average_occ = generate_hard_occlusion((float4){x, y, 0, 0}, normal, namydepth, lights, light_depth_buffer, c_pos, c_rot, i, shnum);




                float odepth[3];

                struct interp_container icontainer;


                uint ldepth_map_id = ret_cubeface(global_position, lpos);
                float4 *rotation = &r_struct[ldepth_map_id];



                //struct triangle lspace = full_rotate_n_global(g_tri, lpos, *rotation, &icontainer, odepth, LFOV_CONST, LIGHTBUFFERDIM, LIGHTBUFFERDIM);


                float4 local_light_pos = rot(global_position, lpos, *rotation);


                float4 postrotate_pos;// = lspace.vertices[0].pos;

                postrotate_pos.x = local_light_pos.x * LFOV_CONST/local_light_pos.z;
                postrotate_pos.y = local_light_pos.y * LFOV_CONST/local_light_pos.z;
                postrotate_pos.z = local_light_pos.z;


                ///find the absolute distance as an angle between 0 and 1 that would be required to make it backface, that approximates occlusion
                float err;
                if((err=dot(fast_normalize(rot(normal, zero, *rotation)), fast_normalize(postrotate_pos))) >= 0) ///still does not quite bloody work ///actually works pretty well
                {
                    skip=1;
                }

                err = fabs(err);


                if(thisocc!=0)
                    thisocc = (thisocc + (1.0-err))/2.0f;
                else
                    thisocc+=1.0-err;


                shnum++;

            }

            float ambient = 0.2;

            if(light>0)
                lightaccum+=(1.0-ambient)*light*lights[i].col*lights[i].brightness*(1.0-thisocc)*(1.0-skip)*(1.0-average_occ) + ambient*1.0f;
            else
                lightaccum+=ambient*1.0f;


        }


        int2 scoord={x, y};

        float4 col=texture_filter(c_tri, scoord, vt, (float)*ft/mulint, *c_pos, *c_rot, gobj[o_id].tid, gobj[o_id].mip_level_ids, nums, sizes, array);

        lightaccum.x=clamp(lightaccum.x, 0.0f, 1.0f/col.x);
        lightaccum.y=clamp(lightaccum.y, 0.0f, 1.0f/col.y);
        lightaccum.z=clamp(lightaccum.z, 0.0f, 1.0f/col.z);

        //float hbao = generate_hbao(c_tri, scoord, depth_buffer);

        float hbao = 0;


        float4 dcol = {dcalc(namydepth), dcalc(namydepth), dcalc(namydepth), 0};


        write_imagef(screen, scoord, col*(lightaccum)*(1.0-hbao));


        //*fi=0;
    }

}

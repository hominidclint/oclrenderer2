#include "obj_load.hpp"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <cstdio>
#include <string.h>
#include "triangle.hpp"
#include "texture.hpp"
#include <math.h>
#include <list>

void face_decompose(std::string face_section, int *vertex, int *tc, int *normal)
{

    size_t pos=face_section.find('/');
    size_t pos2=face_section.find('/', pos+1);


    std::string useful("");

    useful.append(face_section.c_str(), 0, pos);
    *vertex=atoi(useful.c_str());
    useful.clear();
    useful.append(face_section.c_str(), pos+1, pos2-pos-1);

    *tc=atoi(useful.c_str());
    useful.clear();
    useful.append(face_section.c_str(), pos2+1, face_section.size() - pos2);
    *normal=atoi(useful.c_str());
    //std::cout << normal << std::endl;
}

std::string getfollowingword(std::vector<char>::iterator &it)
{
    //char c=*it;

    while(*it != ' ' && *it != '\n' && *it!= '\0')
    {
        it++;
    }

    it++;

    while(*it==' ')
        it++;

    ///first letter of next word.

    std::vector<char>::iterator it2=it;
    std::string ret;

    while(*it2 != ' ' && *it2 != '\n' && *it2!= '\0')
    {
        it2++;
    }



    if(it==it2)
    {
        ret='\n';
    }
    else
    {
        ret.append(it, it2);
    }

    return ret;


}

std::string retrieve_diffuse(std::string fname, std::string name)
{
    ///implement as part of load_textures?

    FILE *pFile=fopen(fname.c_str(), "r");

    if(pFile==NULL)
    {
        std::cout << "Error in obj_load.cpp (retrieve_diffuse)" << std::endl;
        exit(0xDEADBEEF);
    }

    std::vector<char> lines;

    while(!feof(pFile))
    {
        lines.push_back(fgetc(pFile));
    }

    fclose(pFile);

    std::string result;

    bool accept=false;

    for(std::vector<char>::iterator it=lines.begin(); it!=lines.end(); it++)
    {

        if(!accept && strncmp(&(*it), name.c_str(), strlen(name.c_str()))==0)
        {
            accept=true;
        }

        if(accept && strncmp(&(*it), "map_Kd", 6)==0)
        {
            result=getfollowingword((it));
            break;
        }

    }

    //std::cout << result << std::endl;

    return result;

    ///lines now contains newmtl

}


void copy_vertex(vertex &vt, vertex v2)
{

    for(int i=0; i<3; i++)
        vt.pos[i] = v2.pos[i];
    for(int i=0; i<3; i++)
        vt.normal[i] = v2.normal[i];
    for(int i=0; i<2; i++)
        vt.vt[i] = v2.vt[i];
    for(int i=0; i<2; i++)
        vt.pad[i] = v2.pad[i];

}

vertex avg_two(vertex v1, vertex v2)
{
    cl_float apos[4];
    apos[0] = (v1.pos[0] + v2.pos[0])/2.0;
    apos[1] = (v1.pos[1] + v2.pos[1])/2.0;
    apos[2] = (v1.pos[2] + v2.pos[2])/2.0;

    cl_float anorm[4];
    anorm[0] = (v1.normal[0] + v2.normal[0])/2.0;
    anorm[1] = (v1.normal[1] + v2.normal[1])/2.0;
    anorm[2] = (v1.normal[2] + v2.normal[2])/2.0;

    cl_float avt[2];
    avt[0] = (v1.vt[0] + v2.vt[0])/2.0;
    avt[1] = (v1.vt[1] + v2.vt[1])/2.0;
    //apos[2] = (t.vertices[0].pos[2] + t.vertices[1].pos[2] + t.vertices[2].pos[2])/3.0;

    vertex cent; ///need to average positions, normals, and vt
    cent.pos[3]=0;

    for(int i=0; i<3; i++)
        cent.pos[i] = apos[i];
    for(int i=0; i<3; i++)
        cent.normal[i] = anorm[i];
    for(int i=0; i<2; i++)
        cent.vt[i] = avt[i];

    return cent;
}


std::vector<triangle> tesselate_triangles(triangle &t)
{
    float mindsep = 40;
    float cutsep = 200;
    bool exceededsep = false;
    for(int i=0; i<3; i++)
    {
        for(int j=1; j<3; j++)
        {
            float xsep = fabs(t.vertices[i].pos[0] - t.vertices[(i+j) % 3].pos[0]);
            float ysep = fabs(t.vertices[i].pos[1] - t.vertices[(i+j) % 3].pos[1]);
            float zsep = fabs(t.vertices[i].pos[2] - t.vertices[(i+j) % 3].pos[2]);
            if( xsep > cutsep || ysep > cutsep || zsep > cutsep )
            {
                std::vector<triangle> v;
                v.push_back(t);
                return v;
            }
            if( xsep > mindsep || ysep > mindsep || zsep > mindsep )
            {
                ///this triangle dun need be tesselated!
                exceededsep = true;
            }
        }
    }

    ///clockwise winding order, i assume

    if(exceededsep)
    {
        std::vector<triangle> v;
        cl_float apos[4];
        apos[0] = (t.vertices[0].pos[0] + t.vertices[1].pos[0] + t.vertices[2].pos[0])/3.0;
        apos[1] = (t.vertices[0].pos[1] + t.vertices[1].pos[1] + t.vertices[2].pos[1])/3.0;
        apos[2] = (t.vertices[0].pos[2] + t.vertices[1].pos[2] + t.vertices[2].pos[2])/3.0;

        cl_float anorm[4];
        anorm[0] = (t.vertices[0].normal[0] + t.vertices[1].normal[0] + t.vertices[2].normal[0])/3.0;
        anorm[1] = (t.vertices[0].normal[1] + t.vertices[1].normal[1] + t.vertices[2].normal[1])/3.0;
        anorm[2] = (t.vertices[0].normal[2] + t.vertices[1].normal[2] + t.vertices[2].normal[2])/3.0;

        cl_float avt[2];
        avt[0] = (t.vertices[0].vt[0] + t.vertices[1].vt[0] + t.vertices[2].vt[0])/3.0;
        avt[1] = (t.vertices[0].vt[1] + t.vertices[1].vt[1] + t.vertices[2].vt[1])/3.0;
        //apos[2] = (t.vertices[0].pos[2] + t.vertices[1].pos[2] + t.vertices[2].pos[2])/3.0;

        vertex cent; ///need to average positions, normals, and vt
        cent.pos[3]=0;

        for(int i=0; i<3; i++)
            cent.pos[i] = apos[i];
        for(int i=0; i<3; i++)
            cent.normal[i] = anorm[i];
        for(int i=0; i<2; i++)
            cent.vt[i] = avt[i];


        vertex ev1, ev2, ev3;
        ev1 = avg_two(t.vertices[0], t.vertices[1]); ///cracks because these will not be calculated exactly the same for bordering triangles
        ev2 = avg_two(t.vertices[1], t.vertices[2]);
        ev3 = avg_two(t.vertices[2], t.vertices[0]);




        /*triangle t1;
        copy_vertex(t1.vertices[0], t.vertices[0]);
        copy_vertex(t1.vertices[1], t.vertices[1]);
        copy_vertex(t1.vertices[2], cent);

        triangle t2;
        copy_vertex(t2.vertices[0], t.vertices[1]);
        copy_vertex(t2.vertices[1], t.vertices[2]);
        copy_vertex(t2.vertices[2], cent);

        triangle t3;
        copy_vertex(t3.vertices[0], t.vertices[2]);
        copy_vertex(t3.vertices[1], t.vertices[0]);
        copy_vertex(t3.vertices[2], cent);*/

        triangle t1, t2, t3, t4;

        copy_vertex(t1.vertices[0], t.vertices[0]);
        copy_vertex(t1.vertices[1], ev1);
        copy_vertex(t1.vertices[2], ev3);


        copy_vertex(t2.vertices[0], t.vertices[1]);
        copy_vertex(t2.vertices[1], ev2);
        copy_vertex(t2.vertices[2], ev1);

        copy_vertex(t3.vertices[0], t.vertices[2]);
        copy_vertex(t3.vertices[1], ev3);
        copy_vertex(t3.vertices[2], ev2);

        copy_vertex(t4.vertices[0], ev1);
        copy_vertex(t4.vertices[1], ev2);
        copy_vertex(t4.vertices[2], ev3);




        /*t2.vertices[0] = t.vertices[1];
        t2.vertices[1] = t.vertices[2];
        t2.vertices[2] = cent;


        t3.vertices[0] = t.vertices[2];
        t3.vertices[1] = t.vertices[0];
        t3.vertices[2] = cent;*/


        std::vector<triangle> v1, v2, v3, v4;
        v1 = tesselate_triangles(t1);
        v2 = tesselate_triangles(t2);
        v3 = tesselate_triangles(t3);
        v4 = tesselate_triangles(t4);

        for(unsigned int i=0; i<v1.size(); i++)
        {
            v.push_back(v1[i]);
        }
        for(unsigned int i=0; i<v2.size(); i++)
        {
            v.push_back(v2[i]);
        }
        for(unsigned int i=0; i<v3.size(); i++)
        {
            v.push_back(v3[i]);
        }
        for(unsigned int i=0; i<v4.size(); i++)
        {
            v.push_back(v4[i]);
        }

        return v;
    }
    else
    {
        std::vector<triangle> v;
        v.push_back(t);
        return v;
    }

}



objects_container* obj_load(std::string filename)
{

    std::vector<char> file;
    std::vector<float> vertex_coords;
    std::vector<float> vt_coords;
    std::vector<float> normals;

    std::vector<int>         usemtl_pos;
    std::vector<std::string> usemtl_name;

    std::vector<int> faces;
    //std::vector<int> fourface;

    std::string tfname;
    //std::cout << filename << std::endl;
    tfname.append(filename, 0, (filename.size()-4));
    tfname.append(".mtl");

    std::string tdir=filename;

    if(filename[filename.size()-1]=='\\' || filename[filename.size()-1] == '/')
    {
        tdir.append(filename, 0, filename.size()-1);
    }

    std::string dir;
    dir.append(tdir, 0, tdir.find_last_of("\\/"));
    //std::cout << dir << std::endl;

    FILE *pFile=fopen(filename.c_str(), "r");

    if(pFile==NULL)
    {
        std::cout << "invalid file name in obj_load.cpp" << std::endl;
        return NULL;
    }


    int facenum=0;


    while(!feof(pFile))
    {
        char character=fgetc(pFile);
        file.push_back(character);
    }

    fclose(pFile);
    float scale=1; /// /////////////////////////////// ///scale!!!         !!!!!!!!!!!!!!!!!!!!!!
    //bool first=true;
    objects_container *objs = new objects_container;

    for(std::vector<char>::iterator it=file.begin(); it!=file.end(); it++)
    {

        char current=(*it);

        char next= (it == file.end()  ? '\0' : (*(it+1)));
        char supernext= (it == file.end()  ? '\0' : (*(it+2)));
        char prev= (it == file.begin() ? '\0' : (*(it-1)));

        //printf("%c\n", next);
        if(current=='v' && next==' ')
        {

            for(int i=0; i<3; i++)
            {
                std::string str=getfollowingword(it);
                vertex_coords.push_back(atof(str.c_str())*scale);
            }




        }

        else if(current=='v' && next=='n' && supernext == ' ')
        {

            for(int i=0; i<3; i++)
            {
                std::string str=getfollowingword(it);
                normals.push_back(atof(str.c_str()));
            }


        }

        else if(current=='v' && next=='t' && supernext == ' ')
        {
            std::string str;

            for(int i=0; i<2; i++)
            {
                str=getfollowingword(it);
                vt_coords.push_back(atof(str.c_str()));
            }

            if(*(it + str.length())=='\n')
            {

                vt_coords.push_back(0.0); ///cannot handle if space and then nothinng
                //std::cout << "hello" << std::endl;
            }
            else
            {
                str=getfollowingword(it);
                vt_coords.push_back(atof(str.c_str()));
            }

            // ///if start of word after this one is a newline
            // ///push 0


        }

        else if(strncmp("usemtl", &(*it), 6)==0)
        {
            ///OH MY GOD A NEW MATERIAL HAS EMERGED

            //if(!first)
            // {
            //    first=false;
            // }

            usemtl_pos.push_back(facenum);
            //std::cout << facenum << std::endl;
            usemtl_name.push_back(getfollowingword((it)));
            //std::cout << *(usemtl_name.end()-1) << std::endl;
            //facenum=0;


        }

        else if(current == 'f' && next==' ' && prev == '\n')
        {
            int vnum[4], vtnum[4], nnum[4];
            //bool four=false;
            //std::vector<int> temp4;
            /*for(int i=0; i<4; i++)
            {
                vnum[i]=0;
                vtnum[i]=0;
                nnum[i]=0;
                std::string str=getfollowingword(it);
                std::cout << i << " " <<  str << std::endl;
                if(strncmp(str.c_str(), "\n", 1)==0 || strncmp(str.c_str(), " \n", 2)==0)
                {
                    //std::cout << "hi" << std::endl;
                    break;
                }
                if(i==3)
                {
                    four=true;
                }



                //std::cout << str << std::endl;
                //std::cout << i << std::endl;

                //std::cout << str << std::endl;
                face_decompose(str, &vnum[i], &vtnum[i], &nnum[i]);
            }

            //if(four)
            //{
                ///123
                ///134
                faces.push_back(vnum[0]);
                faces.push_back(vtnum[0]);
                faces.push_back(nnum[0]);

                faces.push_back(vnum[1]);
                faces.push_back(vtnum[1]);
                faces.push_back(nnum[1]);

                faces.push_back(vnum[2]);
                faces.push_back(vtnum[2]);
                faces.push_back(nnum[2]);
                facenum++;

            if(four)
            {

                faces.push_back(vnum[0]);
                faces.push_back(vtnum[0]);
                faces.push_back(nnum[0]);

                faces.push_back(vnum[2]);
                faces.push_back(vtnum[2]);
                faces.push_back(nnum[2]);

                faces.push_back(vnum[3]);
                faces.push_back(vtnum[3]);
                faces.push_back(nnum[3]);
                facenum++;
                //std::cout << "hi";


            }*/


            for(int i=0; i<3; i++)
            {
                vnum[i]=0;
                vtnum[i]=0;
                nnum[i]=0;
                std::string str=getfollowingword(it);
                //std::cout << i << " " <<  str << std::endl;


                face_decompose(str, &vnum[i], &vtnum[i], &nnum[i]);
            }


            faces.push_back(vnum[0]);
            faces.push_back(vtnum[0]);
            faces.push_back(nnum[0]);

            faces.push_back(vnum[1]);
            faces.push_back(vtnum[1]);
            faces.push_back(nnum[1]);

            faces.push_back(vnum[2]);
            faces.push_back(vtnum[2]);
            faces.push_back(nnum[2]);
            facenum++;





            //exit(1);
            //std::cout << vnum << std::endl << vtnum << std::endl << nnum << std::endl;
            //std::cout << "hi" << std::endl;

            //system("pause");

        }





    }


    int tri_num=faces.size()/9; ///is always a multiple of 9.

    //std::vector<triangle> tris;
    triangle *tris=new triangle[tri_num];
    int counter=0;
    int excounter=0;
    int faceposc=0;
    bool tempb=false;

    for(std::vector<int>::iterator it=faces.begin(); it<faces.end(); counter++)
    {
        triangle tri;

        //std::cout << usemtl_pos[faceposc] << std::endl;
        if(counter==usemtl_pos[faceposc])
        {


            if(counter!=0)
            {
                ///push all current triangles into a subobject?

                if(false)
                {
                    end_cleanup:
                    tempb=true;

                    // std::cout << "hi";
                }

                object obj;
                //obj.alloc(counter-excounter);
                obj.mtlname=usemtl_name[faceposc-1];
                //std::cout << obj.mtlname << std::endl;
                obj.tex_name=retrieve_diffuse(tfname, usemtl_name[faceposc-1]);

                texture temp;
                std::string full=dir + std::string("\\") + obj.tex_name;
                //std::cout << full << std::endl;
                temp.loadtomaster(full); ///  --------========---------===-=-=-=--=-=_+_************* THIS IS WHERE TEXTURES ARE LOADED!!!

                //std::cout << obj.mtlname << std::endl << obj.tex_name << std::endl;



                //memcpy(obj.tri_list, tris, sizeof(triangle)*(counter-excounter));
                /*for(int i=0; i<counter-excounter; i++)
                {
                    std::vector<triangle> v = tesselate_triangles(tris[i]);

                    for(int j=0; j<v.size(); j++)
                    {
                        obj.tri_list.push_back(v[j]);
                    }
                }*/

                for(int i=0; i<counter-excounter; i++)
                {
                    obj.tri_list.push_back(tris[i]);
                    //obj.tri_list[i] = tris[i];
                }

                //std::cout << objs->objs.size() << std::endl;
                //obj.tri_num=counter-excounter;
                obj.tri_num = obj.tri_list.size();
                obj.tid=temp.id;
                //obj.x=0, obj.y=0, obj.z=0;
                objs->objs.push_back(obj);

                //std::cout << obj.tri_num << std::endl;

                if(tempb)
                {
                    break;
                }

                //std::cout << "hihih";


            }

            //std::cout << usemtl_name[faceposc] << std::endl;

            faceposc++;

            excounter=counter;


        }

        for(int i=0; i<3; i++)
        {

            int vnum=(*it);
            it++;
            int vtnum=(*it);
            it++;
            int nnum=(*it);
            it++;

            vnum-=1; ///wavefront obj is a slag and starts from 1 for no particularly good reason
            vtnum-=1;
            nnum-=1;


            float px=vertex_coords[vnum*3];
            float py=vertex_coords[vnum*3+1];
            float pz=vertex_coords[vnum*3+2];

            float vx=vt_coords[vtnum*3];
            float vy=vt_coords[vtnum*3+1];
            //float vz=vt_coords[vtnum*3+2];

            float nx=normals[nnum*3];
            float ny=normals[nnum*3+1];
            float nz=normals[nnum*3+2];

            tri.vertices[i].pos[0]=px;
            tri.vertices[i].pos[1]=py;
            tri.vertices[i].pos[2]=pz;

            //std::cout << px << std::endl;



            tri.vertices[i].vt[0]=vx;
            tri.vertices[i].vt[1]=vy;
            //tri.vertices[i].vt[2]=vz;

            tri.vertices[i].normal[0]=nx;
            tri.vertices[i].normal[1]=ny;
            tri.vertices[i].normal[2]=nz;

            //std::cout << px << std::endl << py << std::endl << pz << std::endl;

            //system("pause");



        }



        tris[counter-excounter]=tri;

        if(it==faces.end())
        {
            goto end_cleanup; ///..... :OOOOOOOOOOOOOOOOO THE END IS NIGH
        }




    }








    objs->pos[0]=0;
    objs->pos[1]=0;
    objs->pos[2]=0;

    objs->rot[0]=0;
    objs->rot[1]=0;
    objs->rot[2]=0;




    //std::cout << tfname << std::endl;

    /*object *obj=new object;
    obj->tri_list=tris;
    obj->tri_num=tri_num;
    obj->x=0;
    obj->y=0;
    obj->z=0;*/

    return objs;



    //std::cout << "hi";



}

#include "obj_load.hpp"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <cstdio>
#include <string.h>
#include "triangle.hpp"
#include "texture.hpp"

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
                obj.alloc(counter-excounter);
                obj.mtlname=usemtl_name[faceposc-1];
                //std::cout << obj.mtlname << std::endl;
                obj.tex_name=retrieve_diffuse(tfname, usemtl_name[faceposc-1]);

                texture temp;
                std::string full=dir + std::string("\\") + obj.tex_name;
                //std::cout << full << std::endl;
                temp.loadtomaster(full); ///  --------========---------===-=-=-=--=-=_+_************* THIS IS WHERE TEXTURES ARE LOADED!!!

                //std::cout << obj.mtlname << std::endl << obj.tex_name << std::endl;



                memcpy(obj.tri_list, tris, sizeof(triangle)*(counter-excounter));
                obj.tri_num=counter-excounter;
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

/******************************************
* main.c                                  *
******************************************/

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "objload.h"
#include "n64out.h"

//
// Prototypes
//
void strBetween(char * instr, char *outstr, char *start, char *end);


//
// Global variables
//
char filename_obj[256] = {0};     
       
char filename_dl[256] = "NONE"; 

// ambient occlusion
int ao = 0;
object  obj;        // holds EVERYTHING

int vertex_cache_size = 32;

int one_triangle = 0;


/******************************************
* Main method                             *
******************************************/
int main(int argc, char *argv[])
{
 
    
    char last_option = '\0';
    int i; 
    
    printf("\n  OBJN64, version 0.2 - marshallh/retroactive\n  Vertex cache management - spinout\n  Use at your own risk!\n\n");
    
    // initialize obj
    memset(&obj, 0, sizeof(object));
    obj.scale = 1.0;
    
    for (i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-') // looks like an argument option to me (first char is '-')
        {
            switch(argv[i][1]) // explore 2nd char
            {
                case 'c': 
                    break;
                case 'a':
                    ao = 1;
                    break;
                case 'n':
                    obj.cn = 1;
                    break;
                case '1':
					one_triangle = 1;
					break;
                default: 
                    break;
            }
            last_option = argv[i][1];
        }else{
            // this is not a -option argument, so it must be a textual argument        
            switch(last_option) // explore last command
            {   
                // load the file
                case 'f': 
                    strcpy(filename_obj, argv[i]);                      // copy filenames
                    break;
                // set scale
                case 's': 
                    sscanf(argv[i], "%f", &obj.scale);                 // set mesh scale
                    break;
                    
                case 'v':
					sscanf(argv[i], "%d", &vertex_cache_size);
					break;
                
                default: 
                    break;
            }
            last_option = '\0';
        }
    }
    
    if(filename_obj[0] == 0){
        // filename was not given, complain
        printf("  Invalid parameter(s)\n  Parameters:\n");
		printf("\t-f <filename.obj>\t input file (wavefront OBJ/MTL format)\n");
		printf("\t-s <scale>\t\t multiply coords by scale (default 1.0)\n");
		printf("\t-n \t\t\t export normals instead of vertex colors\n");
		printf("\t-a \t\t\t ambient occlusion (preliminary)\n");
		printf("\t-v <16/32/64> \t\t set the vertex cache size (32 for F3DEX2\n\t\t\t\t 64 for F3DEX2.Rej). Default is 32\n");
		printf("\t-1 only use gsSP1Triangle (For Fast3D)\n");
        exit(1);
    }
    
    // now we have all the info we need
    // get the party going
    printf("\n");
    printf("Opening %s\n", filename_obj);
    objLoadObj(filename_obj, &obj);
    

    objSetVertices(&obj);
    
    if(ao == 1){
        if(obj.cn == 1){
            printf("Both normals and AO cannot be used together. Skipping AO\n");
        }else{
            objAmbientOcclusion(&obj);
        }   
    }
    
    printf("Vertex cache of %d specified.\n", vertex_cache_size);
    printf("Two-triangle rendering disabled for Fast3D compatibility\n\n");
    
    strncpy(filename_dl, filename_obj, strlen(filename_obj)-4);
    strcpy(filename_dl+strlen(filename_obj)-4, ".h");
    printf("Scaling by %f\n", obj.scale);
    printf("Writing %s\n", filename_dl);
    writeDL(filename_dl, &obj, vertex_cache_size, one_triangle);
  

    printf("\n");
    printf("Number of meshes: %d\n", obj.numMeshes);
    printf("Number of materials: %d\n", obj.numMaterials);
    printf("Done.\n");	
    
    free(obj.meshes);
    free(obj.materials);
    free(obj.textures);
    
    return 0;
}





/******************************************
* Finds substring between two strings     *
******************************************/
void strBetween(char *instr, char *outstr, char *start, char *end)
{
    char *s;
    char *f;
    
    s = strstr(instr, start);
    if(s == NULL){
        printf("Fail: Couldn't find start '%s' in '%s' (strBetween)\n", start, instr);
        exit(1);
    }
    f = strstr(instr, end);
    if(f == NULL){
        printf("Fail: Couldn't find end '%s' in '%s' (strBetween)\n", start, instr);
        exit(1);
    }
    strncpy(outstr, s+strlen(start), f-(s+strlen(start)));  
    if(outstr[f-(s+strlen(start)) - 1] == '\r')
        outstr[f-(s+strlen(start)) - 1] = 0;
    outstr[  f-(s+strlen(start)) ] = 0;
}

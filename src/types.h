/******************************************
* types.h                                 *
******************************************/


#ifndef __TYPES_H
#define __TYPES_H

#define     MAX_VERTICES    8192*2
#define     MAX_NORMALS     4096*2
#define     MAX_TEXCOORDS   4096*2
#define     MAX_FACES       4096*2


#define     TEX_MODE_RGBA_5551 1
#define     TEX_MODE_CI8       2
#define     TEX_MODE_CI4       3
#define     TEX_MODE_IA8       4
#define     TEX_MODE_IA4       5


typedef struct {
    float   x;
    float   y;
    float   z;   
} point3;

typedef struct {
    float   x;
    float   y;
    float   z; 
    
    float   tu;
    float   tv;
    
    float   nx;  
    float   ny;
    float   nz;
    
    short   b;
    
    float   a;
    float   Kd;
    
} vertex;

typedef struct {
    float   x;
    float   y;
    
} point2;

typedef struct {
    int v[3];     //vertex
    int vt[3];    //vertex tex UV
    int vn[3];    //vertex normals
    
} face;

typedef struct {
    char    name[64];
    char    material[64];
    int     material_num;
    
    int     numVertices;
    int     numNormals;
    int     numTexCoords;
    int     numFaces;
      
    vertex  vertices[MAX_VERTICES];
    point3  normals[MAX_NORMALS];
    point2  texcoords[MAX_TEXCOORDS];
    
    face    faces[MAX_FACES];
    
    
} mesh;




typedef struct {
    char        name[64];
    short       mode;
    unsigned short data[4096];  // 8k of space
    short       w;
    short       h;

} texture;



typedef struct {
    char    name[64];                   // name of material
    
    float   diffuse;
    float   alpha;                      // transparency (0.0 to 1.0)
    char    texture_diff[64];           // filename of diffuse texture
    
    int     texture_num;
    //texture *texptr_diff;               // pointer to the texture struct
    
} material;
    
    
    
    
    

    
    
typedef struct {
    int         numMeshes;
    mesh        *meshes;
    
    int         numMaterials;
    material    *materials;
    
    int         numTextures;
    texture     *textures;
    
    float       scale;
    short       cn;
    char        filename_mtl[256];
    
} object;


#endif

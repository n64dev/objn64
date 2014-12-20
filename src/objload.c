/******************************************
* objload.c                               *
******************************************/

#include <math.h>
#include "types.h"
#include "objload.h"

#define ENDIANFLIP16(x) x = ((x&0xff)<<8) | ((x&0xff00)>>8)


char *TEX_MODES[] = {NULL,
                    "TEX_MODE_RGBA_5551",
                    "TEX_MODE_CI8",
                    "TEX_MODE_CI4",
                    "TEX_MODE_IA8",
                    "TEX_MODE_IA4",
                    NULL};
                    

/******************************************
* Loads OBJ from file                     *
******************************************/
void objLoadObj(char *filename, object *obj)
{
    char instr[128];
    char cmdstr[16];
    char temp[128];
    
    int current_mesh = 0;

    int current_vertex = 0;
    int current_normal = 0;
    int current_texcoord = 0;
    int current_face = 0;
    int added_face = 0;
    
    int accum_vertex = 0;
    int accum_normal = 0;
    int accum_texcoord = 0;

    int vertexcnt;  
    int i;
    int j;
    int t;
    
    face    tempface;
    face    startface;
    mesh    *tempmesh;
    
    char    *tokptr;
    
    FILE *fp;
    fp = fopen(filename, "r");
    
    if(fp == NULL){
        printf("Failed to open OBJ file %s\n", filename);
        exit(1);
    }
    
    while(1)
    {
        fgets(instr, 128, fp);           //get the first part of the line
        if(feof(fp)) break;
        
        cmdstr[0] = 0;
        sscanf(instr, "%s", cmdstr);
        
        // comment
        //*********************
        if(strcmp(cmdstr, "#") == 0)
        {
            //strBetween(instr, temp, "#", "\n");
        }
        
        // mtllib
        //*********************
        if(strcmp(cmdstr, "mtllib") == 0)
        {
            strBetween(instr, temp, "mtllib ", "\n");
            strcpy(obj->filename_mtl, temp);
            //printf("New material library %s\n", temp);
            
            
            printf("Opening %s\n", obj->filename_mtl);
            objLoadMtl(obj->filename_mtl, obj);
        }
        
        // g (new shape)
        //*********************
        if(strcmp(cmdstr, "g") == 0)
        {
            strBetween(instr, temp, "g ", "\n");
            
            obj->numMeshes++;
            tempmesh = realloc(obj->meshes, obj->numMeshes * sizeof(mesh));
            
            if(tempmesh == NULL){
                // fail
                printf("Fail: couldn't realloc memory for new mesh\n");
                exit(1);
            }
            if(tempmesh != obj->meshes){
                //new memory location
                obj->meshes = tempmesh;
            }
            
            memset(&obj->meshes[obj->numMeshes-1], 0, sizeof(mesh));         // zero out the new mesh container
            strcpy(obj->meshes[obj->numMeshes-1].name, temp);                 // copy the name
            
            
            accum_vertex += current_vertex;
            accum_normal += current_normal;
            accum_texcoord += current_texcoord;
            
            current_mesh = obj->numMeshes-1;
            if(current_mesh > 0) obj->meshes[current_mesh-1].numFaces = current_face;
            current_vertex = 0;
            current_normal = 0;
            current_texcoord = 0;
            current_face = 0;
            
            
            //printf("New mesh found %s\n", temp);
        }
        
        
        // v (new vertex)
        //*********************
        if(strcmp(cmdstr, "v") == 0)
        {
            strBetween(instr, temp, "v ", "\n");                    // temp now has the vertex

            sscanf(temp, "%f %f %f", &obj->meshes[current_mesh].vertices[current_vertex].x, &obj->meshes[current_mesh].vertices[current_vertex].y, &obj->meshes[current_mesh].vertices[current_vertex].z);
            
            obj->meshes[current_mesh].vertices[current_vertex].Kd = 1.0;
            obj->meshes[current_mesh].vertices[current_vertex].b = 255;
            
            current_vertex++;
            if(current_vertex >= MAX_VERTICES){
                printf("Fail: Too many vertices, max is %d\n", MAX_VERTICES);
                exit(1);
            }
        }
        
        // vn (new vertex normal)
        //*********************
        if(strcmp(cmdstr, "vn") == 0)
        {
            strBetween(instr, temp, "vn ", "\n");                    // temp now has the vertex

            sscanf(temp, "%f %f %f", &obj->meshes[current_mesh].normals[current_normal].x, &obj->meshes[current_mesh].normals[current_normal].y, &obj->meshes[current_mesh].normals[current_normal].z);
            
            current_normal++;
            if(current_normal >= MAX_NORMALS){
                printf("Fail: Too many normals, max is %d\n", MAX_NORMALS);
                exit(1);
            }
        }
        
        // vt (new texcoord)
        //*********************
        if(strcmp(cmdstr, "vt") == 0)
        {
            strBetween(instr, temp, "vt ", "\n");                    // temp now has the vertex

            sscanf(temp, "%f %f", &obj->meshes[current_mesh].texcoords[current_texcoord].x, &obj->meshes[current_mesh].texcoords[current_texcoord].y);
            
            current_texcoord++;
            if(current_texcoord >= MAX_TEXCOORDS){
                printf("Fail: Too many texcoords, max is %d\n", MAX_TEXCOORDS);
                exit(1);
            }
        }
        
        // usemtl
        //*********************
        if(strcmp(cmdstr, "usemtl") == 0)
        {
            strBetween(instr, temp, "usemtl ", "\n");

            strcpy(obj->meshes[current_mesh].material, temp);                 // copy the name
            
            
            // DANGER WILL ROBINSON
            //  
            // At this point, we will assume that no more vertices/normals/uv 
            // will be loaded for the mesh.
            
            obj->meshes[current_mesh].numVertices = current_vertex;
            obj->meshes[current_mesh].numNormals = current_normal;
            obj->meshes[current_mesh].numTexCoords = current_texcoord;
        
            //printf("Referencing material %s\n", temp);
            
        }
        
        
        // f (new face)
        //*********************
        if(strcmp(cmdstr, "f") == 0)
        {
            strBetween(instr, temp, "f ", "\n");

            j = 0 ;
            for(i = 0; i < strlen(temp); i++)
            {
                if(temp[i] == '/') j++;
                if(temp[i] == ' ') break;
            }
            // sample string: 0/0/0 1/1/1 2/2/2
            // split up the string 
            tokptr = strtok(temp, " ");
            
            if(tokptr == NULL){
                // there were no valid tokens. FAIL
                printf("Fail: No tokens found when parse OBJ 'f' tag\n");
                exit(1);
            }
            
            // starting tokenizing
    
            vertexcnt = 0;
            while(1)
            {
                //printf("printing token: %s    vertexcnt %d   \tcurrentface %d\n", tokptr, vertexcnt, current_face);
                
                // zero it out
                memset(&tempface, 0, sizeof(tempface));
                
                // handle vertex/texcoord/normal
                if(j == 2){
                    // no double slashes
                    if(strstr(tokptr, "//") != NULL){
                        // read in the indices
                        sscanf(tokptr, "%d//%d", &tempface.v[0],
                                             &tempface.vn[0]);    
                                                            
                    }else{
                        // read in the indices
                        sscanf(tokptr, "%d/%d/%d", &tempface.v[0],
                                             &tempface.vt[0],
                                             &tempface.vn[0]);    
                    }
                }
                
                // handle vertex/normal
                if(j == 1){
                    // read in the indices
                    sscanf(tokptr, "%d/%d", &tempface.v[0],
                                          &tempface.vn[0]);     
                    printf("FFUUUUUUUUUUUUUUUCCCCCCCCCCCCCCCCCCCCCCCCCCKKKKKKKKKKKKKKKKKKKKKKKKK\n");                    
                }
                
                // handle vertex
                if(j == 0){
                    // read in the index
                    sscanf(tokptr, "%d/%d", &tempface.v[0]);    
                    printf("FFUUUUUUUUUUUUUUUCCCCCCCCCCCCCCCCCCCCCCCCCCKKKKKKKKKKKKKKKKKKKKKKKKK\n");                      
                }
                
                // if it's the first one, copy temp to startface
                if(vertexcnt == 0){
                    memcpy(&startface, &tempface, sizeof(face));
                }
                
                obj->meshes[current_mesh].faces[current_face].v[vertexcnt]  = tempface.v[0] - accum_vertex;
                obj->meshes[current_mesh].faces[current_face].vt[vertexcnt] = tempface.vt[0] - accum_texcoord;
                obj->meshes[current_mesh].faces[current_face].vn[vertexcnt] = tempface.vn[0] - accum_normal;      
                
                vertexcnt++;
                if(vertexcnt == 3){      
                    vertexcnt = 2;
                    current_face ++;
                    
                    // copy the first two vertex over
                    obj->meshes[current_mesh].faces[current_face].v[0] = startface.v[0] - accum_vertex;
                    obj->meshes[current_mesh].faces[current_face].vt[0] = startface.vt[0] - accum_texcoord;
                    obj->meshes[current_mesh].faces[current_face].vn[0] = startface.vn[0] - accum_normal;
                    
                    obj->meshes[current_mesh].faces[current_face].v[1] = obj->meshes[current_mesh].faces[current_face-1].v[2];
                    obj->meshes[current_mesh].faces[current_face].vt[1] = obj->meshes[current_mesh].faces[current_face-1].vt[2];
                    obj->meshes[current_mesh].faces[current_face].vn[1] = obj->meshes[current_mesh].faces[current_face-1].vn[2];
                }
                
                tokptr = strtok(NULL, " ");               // get the next one
                if(tokptr == NULL) break;                 // no more tokens!
            }
    
            /*
            
                case 6:  // tri with vertices, texcoords, and normals
                    //printf("tri with vertices, texcoords, and normals\n");
                    obj->meshes[current_mesh].faces[current_face].numVerts = 3;
                    sscanf(temp, "%d/%d/%d %d/%d/%d %d/%d/%d", &obj->meshes[current_mesh].faces[current_face].v[0],
                                                       &obj->meshes[current_mesh].faces[current_face].vt[0],
                                                       &obj->meshes[current_mesh].faces[current_face].vn[0],
                                                       
                                                       &obj->meshes[current_mesh].faces[current_face].v[1],
                                                       &obj->meshes[current_mesh].faces[current_face].vt[1],
                                                       &obj->meshes[current_mesh].faces[current_face].vn[1],
                                                       
                                                       &obj->meshes[current_mesh].faces[current_face].v[2],
                                                       &obj->meshes[current_mesh].faces[current_face].vt[2],
                                                       &obj->meshes[current_mesh].faces[current_face].vn[2]);
                    current_face++;
                    added_face = 1;
                    break;
                                                       
                    
                default:
                    current_face+= j/2;
                    added_face = j/2;
                    break;
            }
            
            for(i = 0; i < obj->meshes[current_mesh].faces[current_face-added_face-1].numVerts; i++)
            {
                for(j = 1; j <= added_face; j++)
                {
                    
                    obj->meshes[current_mesh].faces[current_face-j].v[i] -= accum_vertex;
                    obj->meshes[current_mesh].faces[current_face-j].vt[i] -= accum_texcoord;
                    obj->meshes[current_mesh].faces[current_face-j].vn[i] -= accum_normal;
                }
            }
            
            
            for(i = 0; i < 3; i++)
            {
                obj->meshes[current_mesh].faces[current_face].v[i] -= accum_vertex;
                obj->meshes[current_mesh].faces[current_face].vt[i] -= accum_texcoord;
                obj->meshes[current_mesh].faces[current_face].vn[i] -= accum_normal;
            }
            */
            
            if(current_face >= MAX_FACES){
                printf("Fail: Too many faces, max is %d\n", MAX_FACES);
                exit(1);
            }
        }
        
        
    }
    

    fclose(fp);   
    
    obj->meshes[current_mesh].numFaces = current_face;
    obj->meshes[current_mesh].numVertices = current_vertex;
    obj->meshes[current_mesh].numNormals = current_normal;
    obj->meshes[current_mesh].numTexCoords = current_texcoord;
    
    
    // print report
    for(i = 0; i < obj->numMeshes; i++)
    {
        printf("------------------------------------------------------------------\n");
        printf("  Mesh %d \n",i);
        printf("\tName:   \t%s\n", obj->meshes[i].name);
        printf("\tMaterial:\t%s\n", obj->meshes[i].material);
        printf("\tVertices:\t%d\n", obj->meshes[i].numVertices);
        printf("\tNormals:\t%d\n", obj->meshes[i].numNormals);
        printf("\tTexcoords:\t%d\n", obj->meshes[i].numTexCoords);
        printf("\tFaces:  \t%d\n", obj->meshes[i].numFaces);
    }
    printf("------------------------------------------------------------------\n\n");
    
    //printf("Loaded OBJ %s\n", filename);
    
}




/******************************************
* Loads bmp texture from file             *
******************************************/
void objLoadTexture(object *obj, char *filename)
{
    FILE *fp;
    texture *temptex;
    texture *texptr;
    char temp[64];
    long bmp_size;
    long bmp_offset;
    long bmp_headersize;
    long bmp_w;
    long bmp_h;
    short bmp_depth;
    long bmp_imagesize;
    
    int x;
    int y;
    int i;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    
    unsigned char dat[12288];  // for storing a maximum of a 64x64 24-bit bmp
    
    printf("Loading texture %s\n", filename);
    
    // allocate new memory for texture
    obj->numTextures++;
    temptex = realloc(obj->textures, obj->numTextures * sizeof(texture));
    if(temptex == NULL){
        printf("Fail: couldn't realloc memory for new texture\n");
        exit(1);
    }
    if(temptex != obj->textures){
        //new memory location
        obj->textures = temptex;
    }
    memset(  &obj->textures[obj->numTextures-1], 0, sizeof(texture));         // zero out the new texture
    texptr = &obj->textures[obj->numTextures-1];
    strncpy(texptr->name, filename, strlen(filename)-4);
    texptr->name[ strlen(filename)-4 ] = 0;
    

    
    // open the BMP
    fp = fopen(filename, "rb");
    
    if(fp == NULL){
        printf("Failed to open texture BMP file %s\n", filename);
        exit(1);
    }    
    
    // read header magic
    fread(&temp, 2, 1, fp);
    if(strncmp(temp, "BM", 2) != 0){        
        printf("Fail: %s is not a valid windows bitmap file! Must be 24-bit BMP\n", filename);
        exit(1);
    }
    
    // filesize
    fread(&bmp_size, 4, 1, fp);
    // some other bullshit
    fread(&temp, 2, 2, fp);
    // imagedata offset
    fread(&bmp_offset, 4, 1, fp);
    // headersize
    fread(&bmp_headersize, 4, 1, fp);
    if(bmp_headersize != 40){
        printf("Fail: %s does not have a V3 Windows header structure! Must be 24-bit BMP\n", filename);
        exit(1);
    }
    // width and height
    fread(&bmp_w, 4, 1, fp);
    fread(&bmp_h, 4, 1, fp);
    
    texptr->w = bmp_w;
    texptr->h = bmp_h;
    
    // number of colorplanes
    fread(&temp, 2, 1, fp);
    // color depth (bits per pixel)
    fread(&bmp_depth, 2, 1, fp);
    if(bmp_depth != 24){
        printf("Fail: %s is a %d-bit image, Must be 24-bit BMP\n", filename, bmp_depth);
        exit(1);
    }
    // compression method
    fread(&temp, 4, 1, fp);
    if(temp[0] != 0){
        printf("Fail: %s uses some kind of compression. Must be 24-bit uncompressed BMP\n", filename);
        exit(1);
    }
    // size of bitmap data
    fread(&bmp_imagesize, 4, 1, fp);
    // more bullshit
    fread(&temp, 16, 1, fp);
    // now we should be where the image data starts.
    // read in the whole thing to the buffer.
    fread(&dat, bmp_imagesize, 1, fp);
    
    printf("BMP size is %dx%dx%dbits. imagesize %d\n", bmp_w, bmp_h, bmp_depth, bmp_imagesize);
    fclose(fp);
    
    
    
    
    // next step: process the image. currently it is upside down, 24-bit.
    // the data is in BGR order.
    
    // handle bitmaps that can be raw 5551 16-bit and still fit in TMEM (4kb)
    // 32x32, 64x32, 32x64 and everything smaller
    if(bmp_w+bmp_h <= 96)
    {
        texptr->mode = TEX_MODE_RGBA_5551;
        printf("Bitmap is small enough to fit into TMEM in plain 16-bit 5551.\n");
    }
    // now handle 64x64 bitmaps
    else if(bmp_w+bmp_h == 128)
    {
        texptr->mode = TEX_MODE_CI8;
        printf("Bitmap is too big to fit into TMEM in raw 16bit form, but can fit if it's palettized.\n");
        printf("Fail: bitmaps that are 64x64 are too big to fit into TMEM without conversion to CI first. Not supported yet\n");
        exit(1);
    }else{
        printf("Fail: weird bitmap size. All textures must have dimensions that are a power of two, and preferably around 32x32\n");
        exit(1);
    } 
    
    i = 0;
    for(y = 0; y < bmp_h; y++)
    {
        for(x = 0; x < bmp_w; x++)
        {
            b = dat[((y)*bmp_w+x)*3];
            g = dat[((y)*bmp_w+x)*3+1];
            r = dat[((y)*bmp_w+x)*3+2];
            
            texptr->data[i] = (((r / 8)) << 11) | 
                              (((g / 8)) << 6) | 
                              (((b / 8)) << 1) | 1;
                              
                              
            //ENDIANFLIP16(texptr->data[i]);
            
            i++;
        }
    }    

}




/******************************************
* Loads MTL from file                     *
******************************************/
void objLoadMtl(char *filename, object *obj)
{
    FILE *fp;
    char instr[128];
    char cmdstr[16];
    char temp[128];
    
    int current_mat = 0;


    int i;
    int t;
    material *tempmat;
    texture *texp;
    
    // load the file
    fp = fopen(filename, "r");
    
    if(fp == NULL){
        printf("Failed to open MTL file \"%s\"\n", filename);
        exit(1);
    }

    while(1)
    {
        fgets(instr, 128, fp);          
        if(feof(fp)) break;
        
        cmdstr[0] = 0;
        sscanf(instr, "%s", cmdstr);
        
        // comment
        //*********************
        if(strcmp(cmdstr, "#") == 0)
        {
            strBetween(instr, temp, "# ", "\n");
        }
        
        // newmtl
        //*********************
        if(strcmp(cmdstr, "newmtl") == 0)
        {
            strBetween(instr, temp, "newmtl ", "\n");
            
            obj->numMaterials++;
            tempmat = realloc(obj->materials, obj->numMaterials * sizeof(material));
            
            if(tempmat == NULL){
                printf("Fail: couldn't realloc memory for new material\n");
                exit(1);
            }
            if(tempmat != obj->materials){
                //new memory location
                obj->materials = tempmat;
            }
            
            memset(&obj->materials[obj->numMaterials-1], 0, sizeof(material));       // zero out the new container
            strcpy(obj->materials[obj->numMaterials-1].name, temp);                  // copy the name
            
            current_mat = obj->numMaterials-1;
            
            //printf("New material found %s\n", temp);
        }
        
        
        // map_Kd (texture map for diffusion)
        //*********************
        if(strcmp(cmdstr, "map_Kd") == 0)
        {
            strBetween(instr, temp, "map_Kd ", "\n");               // read texture name

            strcpy(obj->materials[current_mat].texture_diff, temp);
            // make sure this texture hasn't already been loaded
            for(i = 0; i < obj->numTextures; i++)
            {
                texp = &obj->textures[i];
            
                if(strncmp(temp, texp->name, strlen(temp)-4) == 0)
                {
                    goto skipTex;
                }
            }
            objLoadTexture(obj, &obj->materials[current_mat].texture_diff);
            
 skipTex:   obj->materials[current_mat].texture_num = i;
            
        }
        
        // Kd (diffuse)
        //*********************
        if(strcmp(cmdstr, "Kd") == 0)
        {
            strBetween(instr, temp, "Kd ", "\n");
            
            sscanf(temp, "%f", &obj->materials[current_mat].diffuse);
            
        }
        
        // d (opacity)
        //*********************
        if(strcmp(cmdstr, "d") == 0)
        {
            strBetween(instr, temp, "d ", "\n");
            
            sscanf(temp, "%f", &obj->materials[current_mat].alpha);
            
        }
        
        
    }
    fclose(fp);   
    
    // load textures
    for(i = 0; i < obj->numMaterials; i++)
    {
        if(strlen(obj->materials[i].texture_diff) > 0)          // is there a diffuse texture?
        {
            //printf("yes there is a diffuse tex\n");
            
            
        }
        
        
    }
    
    
    
    // print report
    for(i = 0; i < obj->numMaterials; i++)
    {
        
        printf("------------------------------------------------------------------\n");
        printf("  Material %d \n",i);
        printf("\tName:   \t%s\n", obj->materials[i].name);
        printf("\tTex(diff):\t%s\n", obj->materials[i].texture_diff);
        printf("\tTex format:\t%s\n", TEX_MODES[obj->textures[obj->materials[i].texture_num].mode]);
        printf("\tAlpha:  \t%f\n", obj->materials[i].alpha);
    }
    printf("------------------------------------------------------------------\n\n");
    

    //printf("Loaded MTL %s\n", filename);
    
}





/******************************************
* Goes through the object and sets params *
******************************************/
void objSetVertices(object *obj)
{
    int i;
    int j;
    int k = 0;
    
    int current_mesh = 0;
    int current_mat = 0;
    int current_vertex = 0;
    int current_normal = 0;
    int current_texcoord = 0;
    int current_face = 0;
    
    mesh        *meshp;
    material    *matp;
    face        *facep;
    vertex      *vertp;
    
    for(current_mesh = 0; current_mesh < obj->numMeshes; current_mesh++)
    {
        meshp = &obj->meshes[current_mesh];
        
        
        // try to find what material this mesh uses
        for(i = 0; i < obj->numMaterials; i++)
        {
            matp = &obj->materials[i];
            
            if(strcmp(meshp->material, matp->name) == 0)
            {
                // found a matching material
                // set all the vertices to use this alpha
                for(current_vertex = 0; current_vertex < meshp->numVertices; current_vertex++)
                {
                    meshp->vertices[current_vertex].a = matp->alpha;
                    meshp->vertices[current_vertex].Kd = matp->diffuse;
                }
                
                meshp->material_num = i;
                
                break;
            }
        }
        
        for(i = 0; i < meshp->numVertices; i++)
        {
            vertp = &meshp->vertices[i];
            
            vertp->tu = 0.65315095261;
            vertp->tv = 0.65315095261;
        }
        
        
        // assign texture coordinates to each vertex
//        for(i = 0; i < meshp->numFaces; i++)
        for(i = meshp->numFaces-1; i >=0 ; i--)
        {
            facep = &meshp->faces[i];
            
            for(j = 0; j < 3; j++)
            {
                meshp->vertices[facep->v[j]-1].tu = meshp->texcoords[facep->vt[j]-1].x;
                meshp->vertices[facep->v[j]-1].tv = meshp->texcoords[facep->vt[j]-1].y;
            
                if(meshp->vertices[facep->v[j]-1].nx == 0 &&
                   meshp->vertices[facep->v[j]-1].ny == 0 &&
                   meshp->vertices[facep->v[j]-1].nz == 0 )
                {
                    meshp->vertices[facep->v[j]-1].nx = meshp->normals[facep->vn[j]-1].x;
                    meshp->vertices[facep->v[j]-1].ny = meshp->normals[facep->vn[j]-1].y;
                    meshp->vertices[facep->v[j]-1].nz = meshp->normals[facep->vn[j]-1].z;
                }
                else{
                    meshp->vertices[facep->v[j]-1].nx = (meshp->vertices[facep->v[j]-1].nx + meshp->normals[facep->vn[j]-1].x)/2.0f;
                    meshp->vertices[facep->v[j]-1].ny = (meshp->vertices[facep->v[j]-1].ny + meshp->normals[facep->vn[j]-1].y)/2.0f;
                    meshp->vertices[facep->v[j]-1].nz = (meshp->vertices[facep->v[j]-1].nz + meshp->normals[facep->vn[j]-1].z)/2.0f;
                }
            }
            

        }
        
    }

    
}

#define EPSILON 0.000001
#define CROSS(dest,v1,v2) \
          dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
          dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
          dest[2]=v1[0]*v2[1]-v1[1]*v2[0];
#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
#define SUB(dest,v1,v2) \
          dest[0]=v1[0]-v2[0]; \
          dest[1]=v1[1]-v2[1]; \
          dest[2]=v1[2]-v2[2]; 

#define TEST_CULL
int
intersect_triangle(double orig[3], double dir[3],
                   double vert0[3], double vert1[3], double vert2[3],
                   double *t, double *u, double *v)
{
   double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
   double det,inv_det;

   /* find vectors for two edges sharing vert0 */
   SUB(edge1, vert1, vert0);
   SUB(edge2, vert2, vert0);

   /* begin calculating determinant - also used to calculate U parameter */
   CROSS(pvec, dir, edge2);

   /* if determinant is near zero, ray lies in plane of triangle */
   det = DOT(edge1, pvec);

   #ifdef TEST_CULL           /* define TEST_CULL if culling is desired */
   if (det < EPSILON)
      return 0;

   /* calculate distance from vert0 to ray origin */
   SUB(tvec, orig, vert0);

   /* calculate U parameter and test bounds */
   *u = DOT(tvec, pvec);
   if (*u < 0.0 || *u > det)
      return 0;

   /* prepare to test V parameter */
   CROSS(qvec, tvec, edge1);

    /* calculate V parameter and test bounds */
   *v = DOT(dir, qvec);
   if (*v < 0.0 || *u + *v > det)
      return 0;

   /* calculate t, scale parameters, ray intersects triangle */
   *t = DOT(edge2, qvec);
   inv_det = 1.0 / det;
   *t *= inv_det;
   *u *= inv_det;
   *v *= inv_det;
#else                    /* the non-culling branch */
   if (det > -EPSILON && det < EPSILON)
     return 0;
   inv_det = 1.0 / det;

   /* calculate distance from vert0 to ray origin */
   SUB(tvec, orig, vert0);

   /* calculate U parameter and test bounds */
   *u = DOT(tvec, pvec) * inv_det;
   if (*u < 0.0 || *u > 1.0)
     return 0;

   /* prepare to test V parameter */
   CROSS(qvec, tvec, edge1);

   /* calculate V parameter and test bounds */
   *v = DOT(dir, qvec) * inv_det;
   if (*v < 0.0 || *u + *v > 1.0)
     return 0;

   /* calculate t, ray intersects triangle */
   *t = DOT(edge2, qvec) * inv_det;
#endif

   return 1;
}





/******************************************
* Performs ambient occlusion on vertices  *
******************************************/
void objAmbientOcclusion(object *obj)
{
    int i;
    int j;
    int k = 0;
    int r;
    
    float ray_num;
    float ray_hit;
    
    double vert1[3];
    double vert2[3];
    double vert3[3];
    
    double orig[3];
    double dir[3];
    
    double norm[3];
    
    float light[] = {4,3,-2};
    float mag;
    
    int current_mesh = 0;
    int current_mat = 0;
    int current_vertex = 0;
    int current_normal = 0;
    int current_face = 0;
    
    int test_mesh;  
    int test_face;
    
    mesh        *meshp;
    mesh        *tmeshp;

    face        *facep;
    face        *tfacep;
    vertex      *vertp;
    point3      *normp;
    
    double b[3];
    
    double ot;
    double ou;
    double ov;
    
    
    // boobs (also, normalize the light vector)
    mag = sqrt(light[0]*light[0]+ light[1]*light[1]+ light[2]*light[2]);
    light[0] /= mag;
    light[1] /= mag;
    light[2] /= mag;
    
    
    printf("Ambient occlusion");
    // perform AO
    for(current_mesh = 0; current_mesh < obj->numMeshes; current_mesh++)
    {
        meshp = &obj->meshes[current_mesh];
        
        
        if(strstr(meshp->name, "_noao") == NULL){
            
        //for(current_face = 0; current_face < meshp->numFaces; current_face++)
        //{
            
            for(current_vertex = 0; current_vertex < meshp->numVertices; current_vertex++)
            {
                if((current_vertex & 2) == 2) printf("\rMesh %d/%d\t\t%d percent completed          ", current_mesh+1, obj->numMeshes, lroundf(((float)(current_vertex)/(float)(meshp->numVertices)) * 100.0f));
                
            //facep = &meshp->faces[current_face];
            //vertp = &meshp->vertices[current_vertex];
            
            //vertp = &meshp->vertices[facep->v[current_vertex]-1];
            //normp = &meshp->normals[facep->vn[current_vertex]-1];
            
            vertp = &meshp->vertices[current_vertex];
            
            // now cast a shitload of rays out randomly from the point, towards the normal(ish)
            ray_num = 0;
            ray_hit = 0;
                
            for(r = 0; r < 300; r++)
            {
                ray_num++;
                
                // make this ray start at the current vertex
                // bias in the direction of the normal
                orig[0] = vertp->x + vertp->nx * 0.0001;   
                orig[1] = vertp->y + vertp->ny * 0.0001;  
                orig[2] = vertp->z + vertp->nz * 0.0001;  
                
                norm[0] = vertp->nx;
                norm[1] = vertp->ny;
                norm[2] = vertp->nz;
                
                // generate a random vector inside the unit sphere
                while(1){
                    dir[0] = (   (double)rand() / ((double)(RAND_MAX)+(double)(1)) )*2.0f-1.0f;
                    dir[1] = (   (double)rand() / ((double)(RAND_MAX)+(double)(1)) )*2.0f-1.0f;
                    dir[2] = (   (double)rand() / ((double)(RAND_MAX)+(double)(1)) )*2.0f-1.0f;
                    
                    if(sqrt(dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2]) <= 1.0f)
                        break;
                }
                    
                if(DOT(norm, dir) < 0.0f){
                    dir[0] *= -1;
                    dir[1] *= -1;
                    dir[2] *= -1;
                }

                // test this ray against every other single goddamn triangle
                for(test_mesh = 0; test_mesh < obj->numMeshes; test_mesh++)
                {
                    tmeshp = &obj->meshes[test_mesh];
                    
                    // go through each triangle
                    for(test_face = 0; test_face < tmeshp->numFaces; test_face++)
                    {
                        tfacep = &tmeshp->faces[test_face];
                        
                        vert1[0] = tmeshp->vertices[tfacep->v[0]-1].x;
                        vert1[1] = tmeshp->vertices[tfacep->v[0]-1].y;
                        vert1[2] = tmeshp->vertices[tfacep->v[0]-1].z;
                        
                        vert2[0] = tmeshp->vertices[tfacep->v[1]-1].x;
                        vert2[1] = tmeshp->vertices[tfacep->v[1]-1].y;
                        vert2[2] = tmeshp->vertices[tfacep->v[1]-1].z;
                        
                        vert3[0] = tmeshp->vertices[tfacep->v[2]-1].x;
                        vert3[1] = tmeshp->vertices[tfacep->v[2]-1].y;
                        vert3[2] = tmeshp->vertices[tfacep->v[2]-1].z;
                        if(intersect_triangle(orig, dir, vert1, vert2, vert3, &ot, &ou, &ov) == 1)
                        {
                            // hit!
                            if(ot > -0.0001) 
                            {
                                ray_hit++;
                                goto next_ray;
                            }
                        }
                        
                    }
                    
                }
                
next_ray: ov = 0;
            }
            
            // done testing all rays
            vertp->b = 255 - (ray_hit / ray_num) * 255 ;
            
            i = DOT(norm, light)*64+128;
            if(i> 255) i = 255;
            if(i <0) i = 0;
            
//            vertp->b = i>vertp->b ? vertp->b : i;
            
        }
        printf("\rMesh %d/%d\t\t100 percent completed          ", current_mesh+1, obj->numMeshes);
        }
    }
    
    printf("\n\n");
}

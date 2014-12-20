/******************************************
* n64out.c                                *
******************************************/

#include "types.h"
#include "n64out.h"


char tmp_verts[64][66];
char verts_pos;
/* declaration to silence warnings */
long lroundf(float);

/* strcatdup
 strcat and strdup combined
*/
char *strcatdup(char *s1, char *s2)
{
    int l1, l2;
    
    l1 = (s1!=NULL) ? strlen(s1) : 0;
    l2 = (s2!=NULL) ? strlen(s2) : 0;
    if(l1 < 1) s1 = NULL;
    if(l2 < 1) s2 = NULL;
    s1 = realloc(s1, l1 + l2 + 1);
    if(s2 != NULL)strcpy(s1 + l1, s2);
    
    return s1;
}

/* interpPoint
 returns the vertex number of a point in tmp_verts
Note: does not bounds check
*/
int interpPoint(char *vert)
{
    int vert_n, addvert = 1, n;
    for(n=0;n<verts_pos;n++)
    {
        if (!strcmp(tmp_verts[n], vert))
            return n;
    }
    strcpy(tmp_verts[verts_pos++], vert);
    return n;
}

void writeVertex(char *sp, object *obj, material *matp, vertex *vertp)
{
    float xscale = ((float)obj->textures[matp->texture_num].w/(float)obj->textures[matp->texture_num].h);
    float yscale = ((float)obj->textures[matp->texture_num].h/(float)obj->textures[matp->texture_num].w);
    
    if(xscale > yscale) yscale = 1.0;
    if(xscale < yscale) xscale = 1.0;
    
    if(obj->cn == 1)
    {
        // export normals
        sprintf(sp, "{%6d, %6d, %6d, 0, %6d, %6d, %-3d, %-3d, %-3d, %-3d},\n",       (int)lroundf(vertp->x*obj->scale),
                                                                                     (int)lroundf(vertp->y*obj->scale),
                                                                                     (int)lroundf(vertp->z*obj->scale),
                                                                                     
                                                                                     (int)lroundf((vertp->tu * 2047) * xscale),
                                                                                     (int)lroundf((vertp->tv * 2047) * yscale),
                                                                                     
                                                                                     (int)lroundf((float)vertp->nx * 127),
                                                                                     (int)lroundf((float)vertp->ny * 127),
                                                                                     (int)lroundf((float)vertp->nz * 127),
                                                                                     
                                                                                     (int)lroundf(vertp->a*255));
        
    }else{
        sprintf(sp, "{%6d, %6d, %6d, 0, %6d, %6d, %-3d, %-3d, %-3d, %-3d},\n",       (int)lroundf(vertp->x*obj->scale),
                                                                                     (int)lroundf(vertp->y*obj->scale),
                                                                                     (int)lroundf(vertp->z*obj->scale),
                                                                                     
                                                                                     (int)lroundf((vertp->tu * 2047) * xscale),
                                                                                     (int)lroundf((vertp->tv * 2047) * yscale),
                                                                                     
                                                                                     (int)lroundf((float)vertp->b * vertp->Kd),
                                                                                     (int)lroundf((float)vertp->b * vertp->Kd),
                                                                                     (int)lroundf((float)vertp->b * vertp->Kd),
                                                                                     
                                                                                     (int)lroundf(vertp->a*255));
        
    }
    
                                                                                     
                                                                                     
    if((obj->textures[matp->texture_num].h/obj->textures[matp->texture_num].w) == 0)
    {
      // printf("FFFFFFFFFUUUUUCKK %d\n", matp->texture_num);
       //getchar();
    }
}

/******************************************
* Writes a given object to a displaylist  *
******************************************/
void writeDL(char *filename, object *obj, int vertex_cache_size, int one_triangle)
{
    FILE *fp;
    int i;
    int j;
    int k;
    int v;
    int n;
    char objname[64];
    
    char *verts;
    
    material  *matp;
    mesh    *meshp;
    face    *facep;
    vertex  *vertp;
    point2  *texcp;
    point3  *normp;
    texture *texp;
    
    int total_face = 0;
    
    int current_mesh = 0;
    int current_mat = 0;
    int current_vertex = 0;
    int current_normal = 0;
    int current_texcoord = 0;
    int current_face = 0;
    int current_texture = 0;
    
    
    
    
    
    // vertex building
    char temp[256];
    char *dl;
    char *tris;
    char a,b,c,d,e,f;
    a=b=c=d=e=f=0;
    int vert_list_count;
    // texture optimization
    int matindex[256]; // assume 256 max textures
    //char meshnames[256][100];  // 256 max meshes
    
    mesh *meshptrs[256];
    
    
    
    
    fp = fopen(filename, "w");
    if(fp == NULL){
        printf("Failed to open file %s for writing\n", filename);
        exit(1);
    }
    
    strncpy(objname, filename, strlen(filename)-2);
    objname[ strlen(filename)-2 ] = 0;
    
    fprintf(fp, "//\n");
    fprintf(fp, "// Displaylist generated by OBJN64 \n");
    fprintf(fp, "// Input: %s\n", filename);
    fprintf(fp, "//\n");
    fprintf(fp, "\n");
    
    // go through each texture
    for(current_texture = 0; current_texture < obj->numTextures; current_texture++)
    {
       
        texp = &obj->textures[current_texture];
    
        // what kind of texture format???
        if(texp->mode == TEX_MODE_RGBA_5551)
        {
            fprintf(fp, "unsigned short Text_%s_%s_diff[] = {\n", objname, texp->name);
            
            i = 0;
            for(j = 0; j < texp->w * texp->h; j++)
            {
                fprintf(fp, "0x%04x, ", texp->data[j]);
                i++;
                if(i == 13) {fprintf(fp, "\n"); i = 0;}
            }
            
            fprintf(fp, "}; \n\n");
        }
    }

    // go through each mesh
    for(current_mesh = 0; current_mesh < obj->numMeshes; current_mesh++)
    {
      meshp = &obj->meshes[current_mesh];
      matp = &obj->materials[meshp->material_num];
      dl = NULL;
      if(strstr(meshp->name, "_aoh") == NULL)
      {
        
        // triangle and vertex loading    
        sprintf(temp, "Gfx Vtx_%s_%s_dl[] = {\n", objname, meshp->name);
        dl = strdup(temp);
        
        current_face = 0;
        vert_list_count = 0;
        verts_pos = 0;
        tris = NULL;
        while(current_face < meshp->numFaces)
        {
            facep = &meshp->faces[current_face];
            
            // get points. interpPoint() will add the point if it is not there
            writeVertex(temp, obj, matp, &meshp->vertices[facep->v[0]-1]);
            a = interpPoint( temp );
            writeVertex(temp, obj, matp, &meshp->vertices[facep->v[1]-1]);
            b = interpPoint( temp );
            writeVertex(temp, obj, matp, &meshp->vertices[facep->v[2]-1]);
            c = interpPoint( temp );
            
            //vertex buffer full / on last triangle in mesh?
            //if(verts_pos > 29 || current_face+1 == meshp->numFaces)
            if(verts_pos > vertex_cache_size-3|| current_face+1 == meshp->numFaces)
            {            /* ^--potential optimization: if (verts_pos-29 < 3) , see
                           if adding one or two more verts completes another triangle */
                //append to vertex buff (write to fp)
                fprintf(fp, "Vtx_tn Vtx_%s_%s_%i[%i] = {\n", objname, meshp->name, vert_list_count, verts_pos);
                for (n = 0; n < verts_pos; n++)
                    fprintf(fp, "%s", tmp_verts[n]);
                fprintf(fp, "};\n\n" );
                //append to display list (add to string)
                sprintf(temp, "\tgsSPVertex(&Vtx_%s_%s_%i[0], %i, 0),\n", objname, meshp->name, vert_list_count, verts_pos);
                dl = strcatdup(dl, temp);
                dl = strcatdup(dl, tris);
                if (!d && !e && !f){
                    sprintf(temp, "\tgsSP1Triangle(%i, %i, %i, 0),\n", a, b, c);
                }else{
                    if(one_triangle){
						sprintf(temp, "\tgsSP1Triangle(%i, %i, %i, 0),\n", d, e, f);
						sprintf(temp, "\tgsSP1Triangle(%i, %i, %i, 0),\n", a, b, c);
					}else{
		                sprintf(temp, "\tgsSP2Triangles(%i, %i, %i, 0, %i, %i, %i, 0),\n", d, e, f, a, b, c);
					}
				}
				
                dl = strcatdup(dl, temp);
                //reset vars
                d=e=f=0;
                tris = NULL;
                vert_list_count++;
                verts_pos=0;
            }
            //second set of points set?
            else if (d|e|f)
            { // Yes - do a triangle, reset them.
            	if(one_triangle){
					sprintf(temp, "\tgsSP1Triangle(%i, %i, %i, 0),\n", d, e, f);
					sprintf(temp, "\tgsSP1Triangle(%i, %i, %i, 0),\n", a, b, c);
				}else{
	                sprintf(temp, "\tgsSP2Triangles(%i, %i, %i, 0, %i, %i, %i, 0),\n", d, e, f, a, b, c);
				}
                tris = strcatdup(tris, temp);
                d=e=f=0;
            }
            else
            { // No - set them.
                d = a;
                e = b;
                f = c;
            }
            
            total_face++;
            current_face++;
        }
        
        
        fprintf(fp, "%s", dl);
        fprintf(fp,"\tgsSPEndDisplayList(),\n");
        fprintf(fp, "};\n\n");
        
        if(tris != NULL) free(tris);
        if(dl != NULL) free(dl);
        tris = dl = NULL;
      }
    }
    
    // go through each mesh
    // apply materials/textures
    fprintf(fp, "Gfx Wtx_%s[] = {\n", objname);
    
    
    // set up material look-up
    for(current_mesh = 0; current_mesh < obj->numMeshes; current_mesh++)
    {
        meshp = &obj->meshes[current_mesh];
        
        // copy meshname to holding array
        //strcpy(meshnames[current_mesh], meshp->name);
        meshptrs[current_mesh] = meshp;
        matindex[current_mesh] = meshp->material_num;
        
        
        //printf("    %s,  %d\n", meshnames[current_mesh], matindex[current_mesh]);
    }
    
    // now sort the materials and all their objects
    for(i = 0; i < obj->numMeshes; i++)
    {
        for(j = 0; j < obj->numMeshes-1; j++)
        {
            if(matindex[j] > matindex[j+1])
            {
                // swap
                k = matindex[j] ;
                matindex[j] = matindex[j+1];
                matindex[j+1] = k;
                
                meshp = meshptrs[j];
                meshptrs[j] = meshptrs[j+1];
                meshptrs[j+1] = meshp;
            }
        }        
    } 
       
    current_mat = -1;
    for(current_mesh = 0; current_mesh < obj->numMeshes; current_mesh++)
    {
        //meshp = &obj->meshes[current_mesh];
        
        if(strstr(meshptrs[current_mesh]->name, "_aoh") == NULL){
            
        // check if the material has changed
        if(matindex[current_mesh] != current_mat){
            current_mat = matindex[current_mesh];
            matp = &obj->materials[current_mat];
            
            fprintf(fp, "\tgsDPLoadTextureBlock(Text_%s_%s_diff, G_IM_FMT_RGBA, G_IM_SIZ_16b, \n\t\t%d,%d, 0, G_TX_WRAP|G_TX_NOMIRROR, G_TX_WRAP|G_TX_NOMIRROR,\n\t\t%d,%d, G_TX_NOLOD, G_TX_NOLOD),\n", 
            objname, 
            
            obj->textures[matp->texture_num].name, 
            obj->textures[matp->texture_num].w, 
            obj->textures[matp->texture_num].h,
            
            (int)lroundf(log((float)obj->textures[matp->texture_num].w) / log(2.0f)),
            (int)lroundf(log((float)obj->textures[matp->texture_num].h) / log(2.0f))
            
            );
        }
        
        
        
        
        
        fprintf(fp, "\tgsSPDisplayList(Vtx_%s_%s_dl),\n", objname, meshptrs[current_mesh]->name); //meshp->name);
        
        }
    }
    fprintf(fp,"\tgsSPEndDisplayList(),\n");
    fprintf(fp, "};\n\n");
    
    printf("Finished writing displaylist\n");
    printf("Total triangles: %d\n", total_face);
    
    fclose(fp);
}

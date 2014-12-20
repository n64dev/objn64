/******************************************
* objload.h                               *
******************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void objLoadObj(char *filename, object *obj);
void objLoadMtl(char *filename, object *obj);
void objSetVertices(object *obj);
void objAmbientOcclusion(object *obj);

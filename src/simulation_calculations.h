#include <glad/glad.h>
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

// Temporary struct for object definition

typedef struct Object {
    char* type;
    vec3 position;
    float radius;
    float speed;
    float bearing;
} Object;

// Function Definitions
void calcCollision(Object *obj1, Object *obj2, float e);
void boundaryCollision(Object *obj, Object *wall, float e);
void objectCollision(Object *obj1, Object *obj2, float e);

// e is the coefficient of restitution between the 2 objects
void calcCollision(Object *obj1, Object *obj2, float e) {
    if (strcmp(obj1->type, "WALL") == 0) {
        printf("1\n");
        boundaryCollision(obj2, obj1, e);
    } else if (strcmp(obj2->type, "WALL") == 0) {
        boundaryCollision(obj1, obj2, e);
        printf("2\n");
    } else {
        objectCollision(obj1, obj2, e);
    }
}

void boundaryCollision(Object *obj, Object *wall, float e) {
    vec3 bearingVector = {sin(obj->bearing), cos(obj->bearing), 0};
    vec3 wallVector = GLM_VEC3_ZERO_INIT;
    char* wallDirection; 

    if (wall->bearing == 0.0f){
        glm_vec3_add(wallVector, (vec3){1, 0, 0}, wallVector);
        wallDirection = "HORIZONTAL";
    } else if (wall->bearing == glm_rad(90.0f)) {
        glm_vec3_add(wallVector, (vec3){0, 1, 0}, wallVector);
        wallDirection = "VERTICAL";
    } else {
        printf("ERROR::BOUNDARY_CALC::WALL_VECTOR\n");
    }
    
    float angleOfApproach = acos( (abs(glm_vec3_dot(bearingVector, wallVector))) / (glm_vec3_norm(bearingVector) * glm_vec3_norm(wallVector)));
    
    float angleOfDeflection = atan(e * tan(angleOfApproach));

    float speed = sqrt( pow((e * obj->speed * sin(angleOfApproach)), 2) + pow((obj->speed * cos(angleOfApproach)), 2) );

    float newBearing;

    
    // BACKUP IF NO OTHER SOL.

    // if (strcmp(wallDirection, "HORIZONTAL") == 0 && (obj->bearing >= 0 && obj->bearing < glm_rad(180.0f))) {
    //     printf("");
    // } else if (strcmp(wallDirection, "HORIZONTAL") == 0 && (obj->bearing >= glm_rad(180.0f) && obj->bearing < glm_rad(360.0f))) {
    //     printf("");
    // } else if (strcmp(wallDirection, "VERTICAL") == 0 && (obj->bearing >= 0 && obj->bearing < glm_rad(180.0f))) {
    //     printf("");
    // } else if (strcmp(wallDirection, "VERTICAL") == 0 && (obj->bearing >= glm_rad(180.0f) && obj->bearing < glm_rad(360.0f))) {
    //     printf("");
    // } else {
    //     printf("ERROR::BOUNDARY_CALC::NEW_BEARING\n");
    // }

     
}

void objectCollision(Object *obj1, Object *obj2, float e) {

}
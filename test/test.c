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
    float velocity;
    float bearing;
} Object;

// e is the coefficient of restitution between the 2 objects
void calcCollision(Object *obj1, Object *obj2, float e) {
    if (strcmp(obj1->type, "WALL")) {
        boundaryCollision(obj2, obj1, e);
    } else if (strcmp(obj2->type, "WALL")) {
        boundaryCollision(obj1, obj2, e);
    } else {
        objectCollision(obj1, obj2, e);
    }
}

void boundaryCollision(Object *obj, Object *wall, float e) {
    vec3 bearingVector = {sin(obj->bearing), cos(obj->bearing), 0};
    vec3 wallVector;
    if (wall->bearing == 0){
        wallVector = {1, 0, 0};
    } else if (wall->bearing == M_PI) {
        wallVector = {0, 1, 1};
    } else {
        printf("ERROR::BOUNDARY_CALC::WALL_VECTOR");
    }
    

    // float angleOfApproach = acos((abs(glm_vec3_dot(bearingVector, wallVector)))/)

    
    
     
}

void objectCollision(Object *obj1, Object *obj2, float e) {

}


int main() {

    struct Object o1; 
    struct Object o2;

    o1.bearing = 130.0f;
    o2.bearing = 0.0f;


    boundaryCollision(&o1, &o2, 0.5);

    return 0;
}
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
    float mass;
} Object;

// Function Definitions
void calcCollision(Object *obj1, Object *obj2, float e);
void boundaryCollision(Object *obj, Object *wall, float e);
void objectCollision(Object *obj1, Object *obj2, float e);
int directionCheck(Object *obj, float lineOfCentresBearing, char* type);
float normaliseBearing(float bearing);

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
    vec3 lineOfCentresVector = {obj2->position[0] - obj1->position[0], obj2->position[1] - obj1->position[1], 0};
    vec3 northVector = {0.0f, 1.0f, 0.0f};
    float lineOfCentresBearing = acos( (abs(glm_vec3_dot(lineOfCentresVector, northVector))) / (glm_vec3_norm(lineOfCentresVector) * glm_vec3_norm(northVector)));

    float obj1ToLineOfCentresAngle = (abs(obj1->bearing - lineOfCentresBearing) > 180.0f) ? 360.0f - abs(obj1->bearing - lineOfCentresBearing) : abs(obj1->bearing - lineOfCentresBearing);
    float obj2ToLineOfCentresAngle = (abs(obj2->bearing - lineOfCentresBearing) > 180.0f) ? 360.0f - abs(obj2->bearing - lineOfCentresBearing) : abs(obj2->bearing - lineOfCentresBearing);;
    
    // The first value is along the line of centres, the second value is perpendicular to the line of centres
    // From obj1 to obj2 should be considered positive, 'below' this line is positive and 'above' this line is negative
    float obj1SpeedResolved[] = {directionCheck(obj1, lineOfCentresBearing, "HORIZONTAL") * abs(obj1->speed * cos(obj1ToLineOfCentresAngle)), 
        directionCheck(obj1, lineOfCentresBearing, "VERTICAL") * abs(obj1->speed * sin(obj1ToLineOfCentresAngle))};
    float obj2SpeedResolved[] = {directionCheck(obj2, lineOfCentresBearing, "HORIZONTAL") * (obj2->speed * cos(obj2ToLineOfCentresAngle)), 
        directionCheck(obj2, lineOfCentresBearing, "VERTICAL") * abs(obj2->speed * sin(obj2ToLineOfCentresAngle))};

    float initialMomentum = obj1->mass * obj1SpeedResolved[0] + obj2->mass * obj2SpeedResolved[0];
    float restitutionCalc = e * (obj1SpeedResolved[0] - obj2SpeedResolved[0]);

    // Solve simultaneous equations using conservation of momentum and restitution to find final velocities along line of centres
    mat2 simultaneous = {{obj1->mass, obj2->mass}, {-1, 1}};
    mat2 inverse;
    glm_mat2_inv(simultaneous, inverse);
    vec2 finalVelocity;
    glm_mat2_mulv(inverse, (vec2){initialMomentum, restitutionCalc}, finalVelocity);

    obj1->speed = sqrt( pow(finalVelocity[0], 2) + pow(obj1SpeedResolved[1], 2));
    obj2->speed = sqrt( pow(finalVelocity[1], 2) + pow(obj2SpeedResolved[1], 2));

    // Find the bearings of objects after collision
    if (finalVelocity[0] == 0.0f) {
        obj1->bearing = normaliseBearing(lineOfCentresBearing + (glm_rad(90.0f) * directionCheck(obj1, lineOfCentresBearing, "VERTICAL")));
    } else {
        float angleFromLineOfCentres = abs(tan(obj1SpeedResolved[1] / finalVelocity[0]));


    }
    if (finalVelocity[1] == 0.0f) {
        obj2->bearing = normaliseBearing(lineOfCentresBearing + (glm_rad(90.0f) * directionCheck(obj2, lineOfCentresBearing, "VERTICAL")));
    } else {
        float angleFromLineOfCentres = abs(tan(obj2SpeedResolved[1] / finalVelocity[1]));
    }
    


}

int directionCheck(Object *obj, float lineOfCentresBearing, char* type) {
    // Find bounds for which direction the object is going relative to line of centres
    float lower, upper;
    if (strcmp(type, "HORIZONTAL") == 0) {
        lower = lineOfCentresBearing - glm_rad(90.0f);
        upper = lineOfCentresBearing + glm_rad(90.0f);
    } else if (strcmp(type, "VERTICAL") == 0) {
        lower = lineOfCentresBearing;
        upper = lineOfCentresBearing + glm_rad(180.0f);
        // 'below' line of centres is modelled as positive, 'above' line of centres is modelled as negative
    } else {
        printf("ERROR::DIRECTION_CHECK::DIRECTION_CHOICE\n");
    }
    
    normaliseBearing(lower);
    normaliseBearing(upper);

    if (lower < upper) {
        if (obj->bearing >= lower && obj->bearing < upper) {
            return 1;
        } else {
            return -1;
        }

    } else {
        if ((obj->bearing >= lower && obj->bearing < glm_rad(360.0f)) || (obj->bearing >= 0.0f && obj->bearing < upper)) {
            return 1;
        } else {
            return -1;
        }
    }
}

float normaliseBearing(float bearing) {
    if (bearing >= glm_rad(360.0f)) {
        return bearing - glm_rad(360.0f);
    } else if (bearing < 0.0f) {
        return glm_rad(360.0f) - abs(bearing); 
    } else {
        return bearing;
    }
}
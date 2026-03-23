#include <glad/glad.h>
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

// Temporary struct for object definition

typedef struct Object{
    char* type;
    vec3 position;
    float radius;
    float speed;
    float bearing;
    float mass;
} Object;

// // Function Definitions
Object initialiseObject(char* type, vec3 position, float radius, float speed, float bearing, float mass);
void createObjectVertices(Object* obj, float* vertices, int size);
bool checkCollision(Object *obj1, Object *obj2);
void calcCollision(Object *obj1, Object *obj2, float e);
void boundaryCollision(Object *obj, Object *wall, float e);
void objectCollision(Object *obj1, Object *obj2, float e);
int directionCheck(Object *obj, float lineOfCentresBearing, char* type);
void moveObject(Object *obj, float deltaTime);
float normaliseBearing(float bearing);
void setBearing(Object *obj, float lineOfCentresBearing, float verticalSpeed, float horizontalSpeed);

// Simulation constants
float accelerationDueToGravity = -9.81;
float scale;

Object initialiseObject(char* type, vec3 position, float radius, float speed, float bearing, float mass) {
    Object obj;
    obj.type = type;
    obj.position[0] = position[0];
    obj.position[1] = position[1];
    obj.position[2] = position[2];
    obj.radius = radius;
    obj.speed = speed;
    obj.bearing = bearing;
    obj.mass = mass;

    return obj;
}

void createObjectVertices(Object* obj, float* vertices, int size) {
    // Centre
    vertices[0] = obj->position[0];
    vertices[1] = obj->position[1];
    vertices[2] = 0.0f;
    for (int i = 3; i < size*3; i+=3) {
        vertices[i] = vertices[0] + (obj->radius * cos(i * glm_rad(360) / size));
        vertices[i+1] = vertices[1] + (obj->radius * sin(i * glm_rad(360) / size));
        vertices[i+2] = vertices[2];
    }
}

bool checkCollision(Object *obj1, Object *obj2) {
    if (strcmp(obj1->type, "WALL") == 0) {
        // the first object is a wall
        if (obj1->bearing == 0.0f) { // Horizontal wall
            float distance = abs(obj1->position[1] - obj2->position[1]);
            if (distance < obj2->radius) {
                return true;
            } else {
                return false;
            }
        } else if (obj1->bearing == glm_rad(90.0f)) { // Vertical wall
            float distance = abs(obj1->position[0] - obj2->position[0]);
            if (distance < obj2->radius) {
                return true;
            } else {
                return false;
            }
        } else {
            printf("ERROR::COLLISION_CHECK::OBJ1_IS_WALL\n");
        }
    } else if (strcmp(obj2->type, "WALL") == 0) {
        // the second object is a wall
        if (obj2->bearing == 0.0f) { // Horizontal wall
            float distance = abs(obj1->position[1] - obj2->position[1]);
            if (distance < obj1->radius) {
                return true;
            } else {
                return false;
            }
        } else if (obj2->bearing == glm_rad(90.0f)) { // Vertical wall
            float distance = abs(obj1->position[0] - obj2->position[0]);
            if (distance < obj1->radius) {
                return true;
            } else {
                return false;
            }
        } else {
            printf("ERROR::COLLISION_CHECK::OBJ2_IS_WALL\n");
        }
    } else {
        float distance = sqrt(pow(obj1->position[0] - obj2->position[0], 2) + pow(obj1->position[1] - obj2->position[1], 2));
        if (distance < obj1->radius + obj2->radius) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

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
    float perpWallBearing; 
    char* wallDirection;

    if (wall->bearing == 0.0f){
        glm_vec3_add(wallVector, (vec3){1, 0, 0}, wallVector);
        if (obj->bearing > glm_rad(90.0f) && obj->bearing < glm_rad(270.0f)) { // Bottom wall
            perpWallBearing = 0.0f;
        } else {
            perpWallBearing = glm_rad(180.0f); // Top wall
        }
        wallDirection = (char*)"HORIZONTAL";
    } else if (wall->bearing == glm_rad(90.0f)) {
        glm_vec3_add(wallVector, (vec3){0, 1, 0}, wallVector);
        if (obj->bearing > 0.0f && obj->bearing < glm_rad(180.0f)) { // Right wall
            perpWallBearing = glm_rad(270.0f);
        } else {
            perpWallBearing = glm_rad(90.0f); // Left wall
        }
        wallDirection = (char*)"VERTICAL";
    } else {
        printf("ERROR::BOUNDARY_COLLISION::WALL_VECTOR\n");
    }
    
    float angleOfApproach = acos( (abs(glm_vec3_dot(bearingVector, wallVector))) / (glm_vec3_norm(bearingVector) * glm_vec3_norm(wallVector)));
    
    float angleOfDeflection = abs(atan(e * tan(angleOfApproach)));
    float angleOfDeflection2 = glm_rad(90.0f) - angleOfDeflection;

    float speed = sqrt( pow((e * obj->speed * sin(angleOfApproach)), 2) + pow((obj->speed * cos(angleOfApproach)), 2) );

    float option1 = normaliseBearing(perpWallBearing + angleOfDeflection2);
    float option2 = normaliseBearing(perpWallBearing + angleOfDeflection2);
    if (strcmp(wallDirection, "HORIZONTAL") == 0) {
        if (obj->bearing >= 0.0f && obj->bearing < glm_rad(180.0f)) {
            // object moving right
            if (option1 >= 0.0f && option1 < glm_rad(180.0f)) {
                obj->bearing = option1;
            } else if (option2 >= 0.0f && option2 < glm_rad(180.0f)) {
                obj->bearing = option2;
            } else {
                printf("ERROR::BOUNDARY_COLLISION::NEW_BEARING::RIGHT");
            }
        } else {
            // object moving left
            if (option1 >= glm_rad(180.0f) && option1 < glm_rad(360.0f)) {
                obj->bearing = option1;
            } else if (option2 >= glm_rad(180.0f) && option2 < glm_rad(360.0f)) {
                obj->bearing = option2;
            } else {
                printf("ERROR::BOUNDARY_COLLISION::NEW_BEARING::LEFT");
            }
        }
    } else if (strcmp(wallDirection, "VERTICAL") == 0) {
        if (obj->bearing >= glm_rad(90.0f) && obj->bearing < glm_rad(270.0f)) {
            // object moving down
            if (option1 >= glm_rad(90.0f) && option1 < glm_rad(270.0f)) {
                obj->bearing = option1;
            } else if (option2 >= glm_rad(90.0f) && option2 < glm_rad(270.0f)) {
                obj->bearing = option2;
            } else {
                printf("ERROR::BOUNDARY_COLLISION::NEW_BEARING::DOWN");
            }
        } else {
            // object moving up
            if ((option1 >= glm_rad(270.0f) && option1 < glm_rad(360.0f)) || (option1 >= 0.0f && option1 < glm_rad(90.0f))) {
                obj->bearing = option1;
            } else if ((option2 >= glm_rad(270.0f) && option2 < glm_rad(360.0f)) || (option2 >= 0.0f && option2 < glm_rad(90.0f))) {
                obj->bearing = option2;
            } else {
                printf("ERROR::BOUNDARY_COLLISION::NEW_BEARING::UP");
            }
        }
    } else {
        printf("ERROR::BOUNDARY_COLLISION::NEW_BEARING");
    }
}

void objectCollision(Object *obj1, Object *obj2, float e) {
    vec3 lineOfCentresVector = {obj2->position[0] - obj1->position[0], obj2->position[1] - obj1->position[1], 0};
    vec3 northVector = {0.0f, 1.0f, 0.0f};
    float lineOfCentresBearing = acos( (abs(glm_vec3_dot(lineOfCentresVector, northVector))) / (glm_vec3_norm(lineOfCentresVector) * glm_vec3_norm(northVector)));

    float obj1ToLineOfCentresAngle = (abs(obj1->bearing - lineOfCentresBearing) > 180.0f) ? 360.0f - abs(obj1->bearing - lineOfCentresBearing) : abs(obj1->bearing - lineOfCentresBearing);
    float obj2ToLineOfCentresAngle = (abs(obj2->bearing - lineOfCentresBearing) > 180.0f) ? 360.0f - abs(obj2->bearing - lineOfCentresBearing) : abs(obj2->bearing - lineOfCentresBearing);;
    
    // The first value is along the line of centres, the second value is perpendicular to the line of centres
    // From obj1 to obj2 should be considered positive, 'below' this line is positive and 'above' this line is negative
    float obj1SpeedResolved[] = {directionCheck(obj1, lineOfCentresBearing, (char*)"HORIZONTAL") * abs(obj1->speed * cos(obj1ToLineOfCentresAngle)), 
        directionCheck(obj1, lineOfCentresBearing, (char*)"VERTICAL") * abs(obj1->speed * sin(obj1ToLineOfCentresAngle))};
    float obj2SpeedResolved[] = {directionCheck(obj2, lineOfCentresBearing, (char*)"HORIZONTAL") * (obj2->speed * cos(obj2ToLineOfCentresAngle)), 
        directionCheck(obj2, lineOfCentresBearing, (char*)"VERTICAL") * abs(obj2->speed * sin(obj2ToLineOfCentresAngle))};

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
    setBearing(obj1, lineOfCentresBearing, obj1SpeedResolved[1], finalVelocity[0]);
    setBearing(obj2, lineOfCentresBearing, obj2SpeedResolved[1], finalVelocity[1]);
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
        // bearing from line of centres 180 degrees in positive direction is modelled as positive
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

void moveObject(Object *obj, float deltaTime) {
    vec3 velocityVector = {obj->speed * sin(obj->bearing), obj->speed * cos(obj->bearing), 0.0f};
    vec3 northVector = {0.0f, 1.0f, 0.0f};
    float angleToVertical =  acos( (abs(glm_vec3_dot(velocityVector, northVector))) / (glm_vec3_norm(velocityVector) * glm_vec3_norm(northVector)));

    float speedResolved[] = {directionCheck(obj, glm_rad(0.0f), (char*)"HORIZONTAL") * abs(obj->speed * sin(angleToVertical)), 
        directionCheck(obj, glm_rad(0.0f), (char*)"VERTICAL") * abs(obj->speed * cos(angleToVertical))};


    // Currently no horizontal acceleration is implemented so this can be modelled as a simple s = ut equation, the horizontal component doesn't change
    // Horizontal
    obj->position[0] += speedResolved[0] * deltaTime;
    float newHorizontalSpeed = speedResolved[0];
    
    // Vertical: s = ut + 0.5at^2   v = u + at
    obj->position[1] += (speedResolved[1] * deltaTime + 0.5 * accelerationDueToGravity * pow(deltaTime, 2));
    float newVerticalSpeed = speedResolved[1] + accelerationDueToGravity * deltaTime;

    
    obj->speed = sqrt(pow(newHorizontalSpeed, 2) + pow(newVerticalSpeed, 2));
    setBearing(obj, glm_rad(0.0f), newVerticalSpeed, newHorizontalSpeed);
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

void setBearing(Object *obj, float lineOfCentresBearing, float verticalSpeed, float horizontalSpeed) {
    if (horizontalSpeed == 0.0f) {
        obj->bearing = normaliseBearing(lineOfCentresBearing + (glm_rad(90.0f) * directionCheck(obj, lineOfCentresBearing, (char*)"VERTICAL")));
    } else {
        float angleFromLineOfCentres = abs(atan(verticalSpeed / horizontalSpeed));
        // if horizontalSpeed > 0, along line of centres
        // if verticalSpeed > 0, from line of centres in positive direction
        if (horizontalSpeed > 0) { 
            if (verticalSpeed > 0) {
                obj->bearing = normaliseBearing(lineOfCentresBearing + angleFromLineOfCentres);
            } else {
                obj->bearing = normaliseBearing(lineOfCentresBearing - angleFromLineOfCentres);
            }
        } else {
            if (verticalSpeed > 0) {
                obj->bearing = normaliseBearing(glm_rad(180.0f) + lineOfCentresBearing + angleFromLineOfCentres);
            } else {
                obj->bearing = normaliseBearing(glm_rad(180.0f) - lineOfCentresBearing - angleFromLineOfCentres);
            }
        }
    }
}   
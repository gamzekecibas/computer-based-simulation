/*
 MECH 534: Computer Based Modeling and Simulation
 Project 1 - Rigid Body Simulation
 Spring 2022
 Gamze Keçibaş
 60211
 */

// Import required global & local libraries
#include <Inventor/Win/SoWin.h>
#include <Inventor/Win/viewers/SoWinExaminerViewer.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/sensors/SoTimerSensor.h>
#include <Inventor/nodes/SoEventCallback.h>
#include <iostream>
#include <cmath>
using namespace std;
// Enter given dimensions (a,b,c) [H,W,D] of box
const float objectHeight = 20; 
const float objectWidth = 10; 
const float objectDepth = 5; 

const float spaceObjectRatio = 4;

// Vertex coordinates around center of the object
SbVec3f absoluteVertexPositions[8] = { 
    {objectWidth / 2, objectHeight / 2, objectDepth / 2},
    {objectWidth / 2, objectHeight / 2, -objectDepth / 2},
    {objectWidth / 2, -objectHeight / 2, objectDepth / 2},
    {objectWidth / 2, -objectHeight / 2, -objectDepth / 2},
    {-objectWidth / 2, objectHeight / 2, objectDepth / 2},
    {-objectWidth / 2, objectHeight / 2, -objectDepth / 2},
    {-objectWidth / 2, -objectHeight / 2, objectDepth / 2},
    {-objectWidth / 2, -objectHeight / 2, -objectDepth / 2},
};

// Mass of the box
float m = 1;
// Gravitational acceleration to keep initial position of the box
float g = 9.8;
// Drag coefficients
float cd = 0.01;
float cdP = 10;

/*Moment of inertia tensor of the rectangular box :
* I_xx = m * 1/12 * (H^2 + D^2)
* I_yy = m * 1/12 * (D^2 + W^2)
* I_zz = m * 1/12 * (H^2 + W^2)
*/ 
float Ixx = m * 1/12 *(pow(objectHeight, 2) + pow(objectDepth, 2));
float Iyy = m * 1/12 *(pow(objectWidth, 2) + pow(objectDepth, 2));
float Izz = m * 1/12 *(pow(objectHeight, 2) + pow(objectWidth, 2));
SbVec3f I = { Ixx, Iyy, Izz }; //Inertia tensor

// Animation speed & Quality
float dt = 0.1; // Time step
float updateInterval = 0.001; // Frequency

// Iterative variables
SbVec3f objectPosition;
SbVec3f objectPosition_prev;
SbVec3f objectAngularPosition;

SbVec3f objectVelocity;
SbVec3f objectAngularVelocity;

SbVec3f objectAcceleration;
SbVec3f objectAngularAcceleration;
SbVec3f objectMoment;
SbVec3f objectForce;

SbVec3f impactForce;

SbVec3f relativeVertexPositions[8];
bool isVertexCollided[8];
int collidingVertexIndex[8];
int numOfCollidingVertices = 0;
SbVec3f collidedSurfaceNormal[8];
bool reverseDirectionFlag[3] = { true, true, true };

SbVec3f calculatedCenter;

SbVec3f userAppliedForce;

SoTransform* objectTransform;
SoTimerSensor* timer = new SoTimerSensor();

using namespace std;

// Constrained the box using boundaries
SbVec3f calculate_transformed_point(SbVec3f pointToBeTransformed, SbVec3f originVector) {
    float result[4][1] = { {0}, {0}, {0}, {0} };
    static int r1 = 4;
    static int c2 = 1;
    static int c1 = 4;

    float alpha = objectAngularPosition[2]; 
    float beta = objectAngularPosition[1]; 
    float gamma = objectAngularPosition[0];

    float transformMatrix[4][4] = {
        {cos(alpha) * cos(beta),
        cos(alpha) * sin(beta) * sin(gamma) - sin(alpha) * cos(gamma),
        cos(alpha) * sin(beta) * cos(gamma) + sin(alpha) * sin(gamma), 
        originVector[0]
        },
        {sin(alpha) * cos(beta),
        sin(alpha) * sin(beta) * sin(gamma) + cos(alpha) * cos(gamma),
        sin(alpha) * sin(beta) * cos(gamma) - cos(alpha) * sin(gamma),
        originVector[1]
        },
        {-sin(beta),
        cos(beta) * sin(gamma),
        cos(beta) * cos(gamma),
        originVector[2]
        },
        {0, 0, 0, 1}
    };
    float P[4][1] = {
        {pointToBeTransformed[0]},
        {pointToBeTransformed[1]},
        {pointToBeTransformed[2]},
        {1}
    };

    for (int i = 0; i < r1; ++i)
        for (int j = 0; j < c2; ++j)
            for (int k = 0; k < c1; ++k)
            {
                result[i][j] += transformMatrix[i][k] * P[k][j];
            }
    return SbVec3f(result[0][0], result[0][1], result[0][2]);
}

// Collision control
void calculate_vertex_positions() { 
    for (int i = 0; i < 8; i++) {
        relativeVertexPositions[i] = calculate_transformed_point(absoluteVertexPositions[i], objectPosition);
    }
}

// Debugging if the updated coordinates are in the boundary space
void check_for_collisions() { 
    static const float spaceBoundary = objectHeight * spaceObjectRatio / 2; 
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 3; j++) {
            if (relativeVertexPositions[i][j] <= -spaceBoundary || relativeVertexPositions[i][j] >= spaceBoundary) {
                isVertexCollided[i] = true;
                collidingVertexIndex[i] = j;
                collidedSurfaceNormal[i][j] = relativeVertexPositions[i][j] <= -spaceBoundary ? 1 : -1;
                if (reverseDirectionFlag[j]) {        
                    objectVelocity[j] *= -0.9;
                    reverseDirectionFlag[j] = false;
                }
            }
        }
        if (isVertexCollided[i]) numOfCollidingVertices++;
    }
}

void calculate_forces() {
    static SbVec3f weight = { 0, m * g, 0 };
    static SbVec3f dragForce;

    // Drag Force
    SbVec3f prevVel = objectVelocity; 
    prevVel.normalize();
    dragForce = (prevVel * objectVelocity.sqrLength() * cd);
    dragForce.negate(); 

    objectForce += dragForce - weight + userAppliedForce;
}

void calculate_moments() {
    static SbVec3f dragMoment;
    static SbVec3f weight = { 0, m * g, 0 };

    // Drag force
    SbVec3f prevAngVel = objectAngularVelocity; 
    prevAngVel.normalize();
    dragMoment = (prevAngVel * objectAngularVelocity.sqrLength() * cdP);
    dragMoment.negate();

    // Force calculation at corners
    if (!(userAppliedForce[0] == 0 && userAppliedForce[1] == 0 && userAppliedForce[2] == 0)) {
        SbVec3f r = relativeVertexPositions[0] - objectPosition;
        objectMoment += r.cross(userAppliedForce / 5);
    }

    for (int i = 0; i < 8; i++) {
        if (isVertexCollided[i]) {
            SbVec3f r = relativeVertexPositions[i] - objectPosition;
            objectMoment += r.cross(collidedSurfaceNormal[i] * objectVelocity.length());
            objectMoment += (-r).cross(-weight);
        }
    }
    objectMoment += dragMoment;
}

void constrain_position() {
    static const float spaceBoundary = objectHeight * spaceObjectRatio / 2 - 0.01; // 20 * 5 / 2
    SbVec3f objectPosition_new;

    for (int i = 0; i < 8; i++) {
        if (isVertexCollided[i]) {
            boolean isBigger = relativeVertexPositions[i][collidingVertexIndex[i]] >= spaceBoundary;
            switch (collidingVertexIndex[i]) {
            case 0:
                objectPosition_new = isBigger ?
                    calculate_transformed_point(-absoluteVertexPositions[i], SbVec3f(spaceBoundary, relativeVertexPositions[i][1], relativeVertexPositions[i][2])) :
                    calculate_transformed_point(-absoluteVertexPositions[i], SbVec3f(-spaceBoundary, relativeVertexPositions[i][1], relativeVertexPositions[i][2]));
                break;
            case 1:
                objectPosition_new = isBigger ?
                    calculate_transformed_point(-absoluteVertexPositions[i], SbVec3f(relativeVertexPositions[i][0], spaceBoundary, relativeVertexPositions[i][2])) :
                    calculate_transformed_point(-absoluteVertexPositions[i], SbVec3f(relativeVertexPositions[i][0], -spaceBoundary, relativeVertexPositions[i][2]));
                break;
            case 2:
                objectPosition_new = isBigger ?
                    calculate_transformed_point(-absoluteVertexPositions[i], SbVec3f(relativeVertexPositions[i][0], relativeVertexPositions[i][1], spaceBoundary)) :
                    calculate_transformed_point(-absoluteVertexPositions[i], SbVec3f(relativeVertexPositions[i][0], relativeVertexPositions[i][1], -spaceBoundary));
                break;
            }
            objectPosition = objectPosition_new;
            objectTransform->translation.setValue(objectPosition);
        }
    }
}

void euler_to_quaternion(float yaw, float pitch, float roll, float(&quaternion_vector)[4]) { // yaw (Z), pitch (Y), roll (X)
    double cy = cos(yaw * 0.5);
    double sy = sin(yaw * 0.5);
    double cp = cos(pitch * 0.5);
    double sp = sin(pitch * 0.5);
    double cr = cos(roll * 0.5);
    double sr = sin(roll * 0.5);

    quaternion_vector[0] = sr * cp * cy - cr * sp * sy; // qx
    quaternion_vector[1] = cr * sp * cy + sr * cp * sy; // qy
    quaternion_vector[2] = cr * cp * sy - sr * sp * cy; // qz
    quaternion_vector[3] = cr * cp * cy + sr * sp * sy; // qw
}

void update_frame_callback(void* data, SoSensor* timer) {
    calculate_vertex_positions();
    check_for_collisions();
    calculate_forces();
    calculate_moments();

    objectAcceleration = objectForce / m; // F = (mass)(acc)
    objectVelocity += objectAcceleration * dt; // Euler's method
    objectPosition += objectVelocity * dt; 
    constrain_position();    
    objectTransform->translation.setValue(objectPosition);

    for (int i = 0; i < 3; i++) objectAngularAcceleration[i] = objectMoment[i] / I[i];
    objectAngularVelocity += objectAngularAcceleration * dt;
    objectAngularPosition += objectAngularVelocity * dt;
    float q[4];
    euler_to_quaternion(objectAngularPosition[2], objectAngularPosition[1], objectAngularPosition[0], q); 
    objectTransform->rotation.setValue(q[0], q[1], q[2], q[3]);


    // Set the reverse direction flag to true if the object has passed the origin
    for (int i = 0; i < 3; i++)
        if (objectPosition_prev[i] * objectPosition[i] <= 0)
            reverseDirectionFlag[i] = true;

#ifdef DEBUG
    printf_s("%.2f\t%.2f\t%.2f\t%d\n", objectPosition[1], objectVelocity[1], objectAcceleration[1], numOfCollidingVertices);
#endif

    // Set initial values as zero
    memset(isVertexCollided, 0, sizeof(isVertexCollided)); 
    numOfCollidingVertices = 0;
    userAppliedForce = SbVec3f(0, 0, 0); 

    objectPosition_prev = objectPosition;

    for (int i = 0; i < 3; i++) { 
        objectForce[i] = 0;
        objectMoment[i] = 0;
    }
}

void key_press_callback(void* userData, SoEventCallback* eventCB) {
    SoSelection* selection = (SoSelection*)userData;
    const SoEvent* event = eventCB->getEvent();

    if (SO_KEY_PRESS_EVENT(event, SoKeyboardEvent::F)) { // User-input random forces in three axes
        cout << "The box is triggered using random force.\n";
        userAppliedForce = SbVec3f(rand() % 100, rand() % 100, rand() % 100);
    }
    if (SO_KEY_PRESS_EVENT(event, SoKeyboardEvent::Z)) { // Freeze the animation
        objectVelocity = { 0, 0, 0 };
        objectAngularVelocity = { 0, 0, 0 };
        cout << "The box is frozen.\n";
    }
    if (SO_KEY_PRESS_EVENT(event, SoKeyboardEvent::R)) { // Restart the animation
        cout << "The simulation is restarted.\n";
        objectPosition = { 0, 0, 0 };
        objectAngularPosition = { 0, 0, 0 };
        objectVelocity = { 0, 0, 0 };
        objectAngularVelocity = { 0, 0, 0 };
    }
    if (SO_KEY_PRESS_EVENT(event, SoKeyboardEvent::SPACE)) { // Pressing spacebar pauses/continues the simulation
        if (!timer->isScheduled()) {
            timer->schedule();
            cout << "Simulation continued.\n";
        }
        else {
            cout << "Simulation paused.\n";
            timer->unschedule();
        }
        eventCB->setHandled();
    }
}

int main(int, char** argv)
{
    srand(time(NULL));
#ifdef START_WITH_RANDOM_MOVEMENT
    for (int i = 0; i < 3; i++) { // Start the movement with random
        objectMoment[i] = rand() % 100 + 100;
        objectForce[i] = rand() % 50;
    }
#endif
    HWND window = SoWin::init(argv[0]);
    if (window == NULL)
        exit(1);

    SoWinExaminerViewer* viewer = new SoWinExaminerViewer(window);

    SoEventCallback* myEventCB = new SoEventCallback;	// Keyboard event listener
    myEventCB->addEventCallback(SoKeyboardEvent::getClassTypeId(), key_press_callback);

    // Scene
    SoSeparator* myScene = new SoSeparator();

    // Initialize the elements of the space
    SoSeparator* spaceSeperator = new SoSeparator();
    SoMaterial* spaceMaterial = new SoMaterial();
    spaceMaterial->ambientColor = { 0.1, 0.4, 0.6 };
    spaceMaterial->diffuseColor = { 0.1, 0.4, 0.6};
    spaceMaterial->transparency = 0.8;
    SoCube* spaceBox = new SoCube();
    spaceBox->height = objectHeight * spaceObjectRatio;
    spaceBox->width = spaceBox->height;
    spaceBox->depth = spaceBox->height;

    // Initialize the elements of the box
    SoSeparator* objectSeperator = new SoSeparator();
    objectTransform = new SoTransform;
    SoMaterial* objectMaterial = new SoMaterial();
    objectMaterial->ambientColor = { 0.3, 0.3, 0.3 };
    objectMaterial->diffuseColor = { 0.3, 0.3, 0.3 };
    objectMaterial->transparency = 0;
    SoCube* objectBox = new SoCube();
    objectBox->height = objectHeight;
    objectBox->width = objectWidth;
    objectBox->depth = objectDepth;

    // Create the space
    spaceSeperator->addChild(spaceMaterial);
    spaceSeperator->addChild(spaceBox);

    // Create the object
    objectSeperator->addChild(objectTransform);
    objectSeperator->addChild(objectMaterial);
    objectSeperator->addChild(objectBox);

    myScene->addChild(myEventCB); // Add the keyboard listener as a child    
    myScene->addChild(objectSeperator); // Add the object to the scene
    myScene->addChild(spaceSeperator); // Add the space to the scene

    cout << "         SIMULATION CONTROLS         \n";
    cout << "-------------------------------------\n";
    cout << "*     F    -> Randomly apply force\n";
    cout << "*     Z    -> Freeze the box.\n";
    cout << "*     R    -> Restart the animation.\n";
    cout << "* Spacebar -> Stop the animation.\n";

    timer->setFunction(update_frame_callback);
    timer->setInterval(updateInterval);
    if (!timer->isScheduled()) timer->schedule();

    // Show the animation
    viewer->setSceneGraph(myScene);
    viewer->setTitle("MECH 534 Project 1");
    viewer->show();

    SoWin::show(window);
    SoWin::mainLoop();

    return 0;
}
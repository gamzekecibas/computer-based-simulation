/* MECH534 - HW7: Bicycling Animation
Gamze Keçibaş
Spring 2022
 */

// Import libraries
#define ANGLES

#include <Inventor/Win/SoWin.h>
#include <Inventor/Win/viewers/SoWinExaminerViewer.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/sensors/SoTimerSensor.h>
#include <Inventor/SoDB.h>
#include <Inventor/SbBasic.h>
#include <Inventor/nodes/SoEventCallback.h>
#include <iostream>
#include <math.h>

constexpr auto pi = 3.14159265359;

#define FILENAME_IN "bycling_data.txt"
#define TO_RADIAN(angle_in_degrees) (angle_in_degrees * pi / 180.0)
#define TO_DEGREES(angle_in_radians) (angle_in_radians * 180.0 / pi)

// Implementation of provided angles
int** angleValues;
int angleIndex = 0;
int numberOfSteps = 0;

float t = 0;
float dt = 0.2; 

// Animation speed
float interval = 0.1;

SoTransform* link0Transform; float theta0 = 0;
SoTransform* link1Transform; float theta1 = 0;
SoTransform* link2Transform; float theta2 = 0;
SoTransform* link3Transform; float theta3 = 0;

SoTimerSensor* timer = new SoTimerSensor();

// Keyboard control with up & down directions
void myKeyPressCB(void* userData, SoEventCallback* eventCB)
{
    SoSelection* selection = (SoSelection*)userData;
    const SoEvent* event = eventCB->getEvent();

    if (SO_KEY_PRESS_EVENT(event, SoKeyboardEvent::UP_ARROW)) {
        if (!timer->isScheduled()) {
            timer->schedule();
            std::cout << "Animation is started. Use DOWN to break..." << std::endl;
        }
        eventCB->setHandled();
    }
    else if (SO_KEY_PRESS_EVENT(event, SoKeyboardEvent::DOWN_ARROW)) {
        if (timer->isScheduled()) {
            std::cout << "Animation is stopped. Please push UP to continue..." << std::endl;
            timer->unschedule();
        }
        eventCB->setHandled();
    }
}

void updateAnglesCB(void* data, SoSensor* timer) {

#ifdef ANGLES
    theta1 = TO_RADIAN((20 * sin(t + pi / 2) + 50));
    theta2 = TO_RADIAN((32.5 * sin(t) + 77.5)) - pi;
    theta3 = TO_RADIAN((10 * sin(t + pi * 0.35) + 10));
#else
    theta1 = TO_RADIAN(angleValues[angleIndex][0]);
    theta2 = -TO_RADIAN(angleValues[angleIndex][1]);
    theta3 = TO_RADIAN(angleValues[angleIndex][2]) - pi / 2;
#endif    

    link1Transform->rotation.setValue(SbVec3f(0, 0, 1), theta1);
    link2Transform->rotation.setValue(SbVec3f(0, 0, 1), theta2);
    link3Transform->rotation.setValue(SbVec3f(0, 0, 1), theta3);

#ifdef ANGLES
    t += dt;
    t = fmod(t, 2 * pi);
#else
    angleIndex++;
    angleIndex %= numberOfSteps;
#endif
}



int main(int, char** argv)
{
#ifndef ANGLES
    errno_t err_in;
    FILE* fp_in = NULL;

    err_in = fopen_s(&fp_in, FILENAME_IN, "r");

    if (!err_in && fp_in != NULL)
        fscanf_s(fp_in, "%d\n", &numberOfSteps);

    angleValues = new int* [numberOfSteps];

    std::cout << "Given angle values are:\n";
    for (int i = 0; i < numberOfSteps; i++) {
        if (!err_in && fp_in != NULL) {
            angleValues[i] = new int[3];
            fscanf_s(fp_in, "%d\t%d\t%d\n", &angleValues[i][0], &angleValues[i][1], &angleValues[i][2]);
            std::cout << angleValues[i][0] << "\t" << angleValues[i][1] << "\t" << angleValues[i][2] << std::endl;
        }
    }
    std::cout << std::endl;
#endif

    SoDB::init();

    HWND window = SoWin::init(argv[0]);
    if (window == NULL)
        exit(1);

    SoWinExaminerViewer* viewer = new SoWinExaminerViewer(window);

    // Pelvis bone
    SoInput pelvisInput;
    pelvisInput.openFile("pelvisModified.iv");
    SoSeparator* link0 = SoDB::readAll(&pelvisInput);
    link0Transform = new SoTransform;
    link0Transform->rotation.setValue(SbVec3f(0, 0, 1), theta0);

    // Femur bone
    SoInput femurInput;
    femurInput.openFile("femurModified.iv");
    SoSeparator* link1 = SoDB::readAll(&femurInput);
    link1Transform = new SoTransform;
    link1Transform->recenter(SbVec3f(-0.0763924, -0.087262, 0.0340157));
    link1Transform->rotation.setValue(SbVec3f(0, 0, 1), theta1);

    // Tibia bone
    SoInput tibiaInput;
    tibiaInput.openFile("tibiaModified.iv");
    SoSeparator* link2 = SoDB::readAll(&tibiaInput);
    link2Transform = new SoTransform;
    link2Transform->recenter(SbVec3f(-0.0599612, -0.493132, 0.0264737));
    link2Transform->rotation.setValue(SbVec3f(0, 0, 1), theta2);

    // Foot bone
    SoInput input4;
    input4.openFile("footModified.iv");
    SoSeparator* link3 = SoDB::readAll(&input4);
    link3Transform = new SoTransform;
    link3Transform->recenter(SbVec3f(-0.0725796, -0.867281, 0.0367676));
    link3Transform->rotation.setValue(SbVec3f(0, 0, 1), theta3);

    // Timer of animation 
    timer->setFunction(updateAnglesCB);
    timer->setInterval(interval);
    if (!timer->isScheduled()) timer->schedule();

    SoSeparator* leg = new SoSeparator;
    SoEventCallback* myEventCB = new SoEventCallback;
    myEventCB->addEventCallback(SoKeyboardEvent::getClassTypeId(), myKeyPressCB);

    leg->addChild(myEventCB);
    leg->addChild(link0Transform);
    leg->addChild(link0);
    leg->addChild(link1Transform);
    leg->addChild(link1);
    leg->addChild(link2Transform);
    leg->addChild(link2);
    leg->addChild(link3Transform);
    leg->addChild(link3);

    viewer->setSceneGraph(leg);
    viewer->setTitle("MECH534 - HW7");
    viewer->show();

    SoWin::show(window);
    SoWin::mainLoop();
    return 0;
}
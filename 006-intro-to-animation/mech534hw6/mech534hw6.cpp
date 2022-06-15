/* MECH534 - HW6: Intro to Open Inventor
PART A:
Create a humanoid robot via Open Inventor

PART B:
Open a cube from archieve & customize it

Gamze Keçibaş
Spring 2022
 */

constexpr auto pi = 3.14159265359;

#include <Inventor/Win/SoWin.h>
#include <Inventor/Win/viewers/SoWinExaminerViewer.h>

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoCylinder.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <iostream>

using namespace std;

// DEFINITIONS OF REQUIREMENTS
// Define spaces

static float vertices[8][3] =
{
   { 0.0,  0.0,  0.0},
   { 1.0,  0.0,  0.0},
   { 1.0,  1.0,  0.0},
   { 0.0,  1.0,  0.0},
   { 0.0,  0.0,  1.0},
   { 1.0,  0.0,  1.0},
   { 1.0,  1.0,  1.0},
   { 0.0,  1.0,  1.0},
};


static int indices[48] =
{
    // Connections should be defined clockwise to create correct normal directions
    // FRONT
    2, 1, 0, -1,
    0, 3, 2, -1, 
    //LEFT
    4, 3, 0, -1, 
    4, 7, 3, -1, 
    //TOP
    7, 2, 3, -1, 
    7, 6, 2, -1,
    //RIGHT
    5, 1, 2, -1, 
    2, 6, 5, -1,
    //BACK
    5, 6, 7, -1, 
    5, 7, 4, -1, 
    //BOTTOM
    5, 4, 0, -1, 
    0, 1, 5, -1, 
};

// Init surface colors
static float colors[12][3] =
{
    // Surface colors of the cube
    // its order is similar with surface init
   {1,0,0}, {1,0,0}, //red
   {1,1,1}, {1,1,1}, //white
   {1,0.65,0}, {1,0.65,0}, //orange
   {0,1,0}, {0,1,0}, //green
   {1,1,0}, {1,1,0}, //yellow
   {0,0,1}, {0,0,1}, //blue
};


SoSeparator* makeIndexedTriangularCube()
{
    SoSeparator* output = new SoSeparator;
    output->ref();

    SoMaterial* myMaterial = new SoMaterial;
    myMaterial->diffuseColor.setValues(0, 12, colors);
    output->addChild(myMaterial);

    SoMaterialBinding* myMaterialBinding = new SoMaterialBinding;
    myMaterialBinding->value = SoMaterialBinding::PER_FACE;
    output->addChild(myMaterialBinding);

    SoCoordinate3* myCoordinate = new SoCoordinate3;
    myCoordinate->point.setValues(0, 8, vertices);
    output->addChild(myCoordinate);

    SoIndexedFaceSet* myFaceSet = new SoIndexedFaceSet;
    myFaceSet->coordIndex.setValues(0, 48, indices);
    output->addChild(myFaceSet);
    output->unrefNoDelete();
    return output;
}

// ASSIGNMENT PART
int main(int, char** argv)
{
/*                  PART A
--------------------------------------------*/ 
    HWND window = SoWin::init(argv[0]);
    if (window == NULL)
        exit(1);

    SoWinExaminerViewer* viewer = new SoWinExaminerViewer(window);

    // MATERIAL DEFINITIONS via COLORS
    // white 
    SoMaterial* whiteMaterial = new SoMaterial;
    whiteMaterial->ambientColor.setValue(.9, .9, .9);
    whiteMaterial->diffuseColor.setValue(9, .9, .9);
    whiteMaterial->specularColor.setValue(.99, .99, .99);
    whiteMaterial->shininess = .75;
    whiteMaterial->transparency = 0.6;
    // pink
    SoMaterial* pinkMaterial = new SoMaterial;
    pinkMaterial->ambientColor.setValue(.33, .22, 0);
    pinkMaterial->diffuseColor.setValue(.1, .1, .1);
    pinkMaterial->specularColor.setValue(.9, .5, .8);
    pinkMaterial->shininess = .1;
    // green
    SoMaterial* greenMaterial = new SoMaterial;
    greenMaterial->ambientColor.setValue(0, 20, .30);
    greenMaterial->diffuseColor.setValue(.1, .1, .1);
    greenMaterial->specularColor.setValue(.50, .5, .50);
    greenMaterial->shininess = .4;

    // Parts for legs 
    SoCube* thigh = new SoCube;
    thigh->width = 1.2;
    thigh->height = 2.2;
    thigh->depth = 1.1;
    SoTransform* calfTransform = new SoTransform;
    calfTransform->translation.setValue(0, -2, 0.0);
    SoCube* calf = new SoCube;
    calf->width = 1;
    calf->height = 2.2;
    calf->depth = 1;
    SoTransform* footTransform = new SoTransform;
    footTransform->translation.setValue(0, -2, .5);
    SoCube* foot = new SoCube;
    foot->width = 0.8;
    foot->height = 0.8;
    foot->depth = 2;

    // Parts of arms
    SoCube* upperArm = new SoCube;
    upperArm->width = 0.8;
    upperArm->height = 3;
    upperArm->depth = 0.8;
    SoTransform* foreArmTransform = new SoTransform;
    foreArmTransform->translation.setValue(0, -3, 0);
    SoCube* forerArm = new SoCube;
    forerArm->width = 0.6;
    forerArm->height = 2.5;
    forerArm->depth = 0.6;

    // ROBOT CONSTRUCTION
    // Create full leg
    SoGroup* leg = new SoGroup;
    leg->addChild(thigh);
    leg->addChild(calfTransform);
    leg->addChild(calf);
    leg->addChild(footTransform);
    leg->addChild(pinkMaterial);
    leg->addChild(foot);
    SoTransform* leftLegTransform = new SoTransform;
    leftLegTransform->translation = SbVec3f(1, -4.25, 0);
    leftLegTransform->rotation.setValue(SbVec3f(1, 0, 0), -pi / 2);

    // Left 
    SoSeparator* leftLeg = new SoSeparator;
    leftLeg->addChild(leftLegTransform);
    leftLeg->addChild(leg);
    SoTransform* rightLegTransform = new SoTransform;
    rightLegTransform->translation.setValue(-1, -4.25, 0);
    rightLegTransform->rotation.setValue(SbVec3f(1, 0, 0), pi / 2);
    // Right 
    SoSeparator* rightLeg = new SoSeparator;
    rightLeg->addChild(rightLegTransform);
    rightLeg->addChild(leg);
    SoTransform* leftArmTransform = new SoTransform;
    leftArmTransform->translation = SbVec3f(3, 2, 0);
    leftArmTransform->rotation.setValue(SbVec3f(0, 0, 1), pi / 2);

    // Create full arm
    SoGroup* arm = new SoGroup;
    arm->addChild(pinkMaterial);
    arm->addChild(upperArm);
    arm->addChild(foreArmTransform);
    arm->addChild(forerArm);

    //Left 
    SoSeparator* leftArm = new SoSeparator;
    leftArm->addChild(leftArmTransform);
    leftArm->addChild(arm);
    SoTransform* rightArmTransform = new SoTransform;
    rightArmTransform->translation = SbVec3f(-3, 2, 0);
    rightArmTransform->rotation.setValue(SbVec3f(0, 0, 1), -pi / 2);
    //Right
    SoSeparator* rightArm = new SoSeparator;
    rightArm->addChild(rightArmTransform);
    rightArm->addChild(arm);

    //Body parts
    SoTransform* bodyTransform = new SoTransform;
    bodyTransform->translation.setValue(0.0, 3.0, 0.0);
    SoCylinder* bodyCylinder = new SoCylinder;
    bodyCylinder->radius = 2;
    bodyCylinder->height = 6;

    //Assembly the body 
    SoSeparator* body = new SoSeparator;
    body->addChild(bodyTransform);
    body->addChild(whiteMaterial);
    body->addChild(bodyCylinder);
    body->addChild(leftArm);
    body->addChild(rightArm);
    body->addChild(greenMaterial);
    body->addChild(leftLeg);
    body->addChild(rightLeg);

    //Head parts
    SoTransform* headTransform = new SoTransform;
    headTransform->translation.setValue(0, 7.5, 0);
    headTransform->scaleFactor.setValue(1.5, 1.5, 1.5);
    SoSphere* headSphere = new SoSphere;

    //Head assembly
    SoSeparator* head = new SoSeparator;
    head->addChild(headTransform);
    head->addChild(pinkMaterial);
    head->addChild(headSphere);

    // Run the complete humanoid robot
    SoSeparator* robot = new SoSeparator;
    robot->addChild(body);
    robot->addChild(head);
    viewer->setSceneGraph(robot);
    viewer->setTitle("MECH534 - Humanoid Robot");
    viewer->show();
    SoWin::show(window);
    SoWin::mainLoop();

/* END OF PART A OF PROGRAMMING ASSIGNMENT      

                    PART B
--------------------------------------------*/

    SoSeparator* rubiksCube = makeIndexedTriangularCube();

    SoWriteAction writeAction;
    writeAction.getOutput()->openFile("CUBE.IV");
    writeAction.getOutput()->setBinary(FALSE);  
    writeAction.apply(rubiksCube);
    writeAction.getOutput()->closeFile();
    SoInput sceneInput;
    sceneInput.openFile("CUBE.IV");
    SoSeparator* inputFile = new SoSeparator;
    inputFile = SoDB::readAll(&sceneInput);
    viewer->setSceneGraph(inputFile);
    viewer->setTitle("MECH534 - Cubic");
    viewer->show();
    SoWin::show(window);
    SoWin::mainLoop();

    return 0;
}
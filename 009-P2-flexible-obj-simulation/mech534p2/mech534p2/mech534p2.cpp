/*
 MECH 534: Computer Based Modeling and Simulation
 Project 2 - 3D Simulation of a Flexible Object
 Spring 2022
 Gamze Keçibaş
 60211
 */

 // Import required global & local libraries
#include <Inventor/Win/SoWin.h>
#include <Inventor/Win/viewers/SoWinExaminerViewer.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoCylinder.h>
#include <Inventor/nodes/SoNurbsCurve.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoComplexity.h>
#include <Inventor/sensors/SoTimerSensor.h>
#include <Inventor/nodes/SoEventCallback.h>
#include <iostream>
#include <cmath>
using namespace std;

// Enter given variables
#define VISIBLE_PARTICLES 
const int PARTICLE_COUNT = 13;
const float ROPE_LENGTH = 30;

float k = 4.6; // stiffness [N/cm]
float b = 2.2; // damper [N.sec/cm]
float m = 1.5; // mass [m] of each particle
float cd = 0.9; // drag coefficient, viscous drag due to air
float g = 9.8; // gravitational acceleration [m/sec^2]

// Initiate time & frequency for Euler method
float dt = 0.1;
float updateInterval = 0.0001;

// Size of particles
#ifndef VISIBLE_PARTICLES
float radius = .01;
#else
float radius = 3; 
#endif

boolean isWindXOn = false;
boolean isWindZOn = false;
float windStrength = 10;
int selectedParticle = 0;

// Initialize variables related to motion -> position, velocity, acceleration, force
SbVec3f particlePositions[PARTICLE_COUNT];
SbVec3f particleVelocities[PARTICLE_COUNT];
SbVec3f particleAccelerations[PARTICLE_COUNT];
SbVec3f particleForces[PARTICLE_COUNT];

SbVec3f ceilingPoint = { 0, 50, 0 };

SoTimerSensor* timer = new SoTimerSensor();

SbVec3f particleAndceilingPositions[PARTICLE_COUNT + 1];
float knots[PARTICLE_COUNT + 5]; 
SoCoordinate3* controlPts;

SoTransform* particleTransforms[PARTICLE_COUNT];
SoMaterial* particleMaterials[PARTICLE_COUNT];

SoCylinder* strengthIndicator;
SoTransform* indicatorTF;
SoMaterial* indicatorMaterial;

SoSeparator* make_curve() {
	SoSeparator* curveSep = new SoSeparator();
	curveSep->ref();


	SoDrawStyle* drawStyle = new SoDrawStyle;
	drawStyle->lineWidth = 5;
	curveSep->addChild(drawStyle);

	SoComplexity* complexity = new SoComplexity;
	SoNurbsCurve* curve = new SoNurbsCurve;
#ifdef VISIBLE_PARTICLES
	complexity->value = 1; 
#endif
	controlPts = new SoCoordinate3;
	controlPts->point.setValues(0, PARTICLE_COUNT + 1, particleAndceilingPositions);
	curve->numControlPoints = PARTICLE_COUNT + 1;
	curve->knotVector.setValues(0, PARTICLE_COUNT + 5, knots);
	curveSep->addChild(complexity);
	curveSep->addChild(controlPts);
	curveSep->addChild(curve);

	curveSep->unrefNoDelete();
	return curveSep;
}

void change_material_color(SoMaterial*& material, SbVec3f color) {
	material->diffuseColor.setValue(color);
}

void key_press_callback(void* userData, SoEventCallback* eventCB) {
	SoSelection* selection = (SoSelection*)userData;
	const SoEvent* event = eventCB->getEvent();

	if (SO_KEY_PRESS_EVENT(event, SoKeyboardEvent::B)) { // Wind blows @ z-axis	
		if (!isWindZOn)
			cout << "The wind blows during negative Z-axis till B is released.\n\n";
		isWindZOn = true;
		eventCB->setHandled();
	}
	if (SO_KEY_RELEASE_EVENT(event, SoKeyboardEvent::B)) { // No wind 	
		isWindZOn = false;
		eventCB->setHandled();
	}
	if (SO_KEY_PRESS_EVENT(event, SoKeyboardEvent::R)) { // Wind blows @ x-axis	
		if (!isWindXOn)
			cout << "The wind blows during positive X-axis till R is released.\n\n";
		isWindXOn = true;
		eventCB->setHandled();
	}
	if (SO_KEY_RELEASE_EVENT(event, SoKeyboardEvent::R)) { // No wind
		isWindXOn = false;
		eventCB->setHandled();
	}
	if (SO_KEY_PRESS_EVENT(event, SoKeyboardEvent::UP_ARROW)) {
		change_material_color(particleMaterials[selectedParticle], { 0.8, 0.8, 0.8 });
		selectedParticle--;
		selectedParticle = selectedParticle == -1 ? selectedParticle + PARTICLE_COUNT : selectedParticle;
		change_material_color(particleMaterials[selectedParticle], { 0.85, 0, 0 });
	}
	if (SO_KEY_PRESS_EVENT(event, SoKeyboardEvent::DOWN_ARROW)) {
		change_material_color(particleMaterials[selectedParticle], { 0.8, 0.8, 0.8 });
		selectedParticle++;
		selectedParticle %= PARTICLE_COUNT;
		change_material_color(particleMaterials[selectedParticle], { 0.85, 0, 0 });
	}

	// Adjust power of wind using up & down arrows
	if (SO_KEY_PRESS_EVENT(event, SoKeyboardEvent::RIGHT_ARROW))
		windStrength = windStrength >= 100 ? 100 : windStrength + 1; 
	if (SO_KEY_PRESS_EVENT(event, SoKeyboardEvent::LEFT_ARROW))
		windStrength = windStrength <= 0 ? 0 : windStrength - 1; 

	if (SO_KEY_PRESS_EVENT(event, SoKeyboardEvent::SPACE)) { // Spacebar -> PAUSE
		if (!timer->isScheduled()) {
			timer->schedule();
			cout << "Simulation is started!\n\n";
		}
		else {
			cout << "Simulation is stopped...";
			timer->unschedule();
		}
		eventCB->setHandled();
	}
}

void forces() {
	static SbVec3f weight = { 0, m * g, 0 };
	static float piecewiseLength = ROPE_LENGTH / PARTICLE_COUNT;

	static SbVec3f upSideSpringForce;
	static SbVec3f downSideSpringForce;
	static SbVec3f upSideDamperForce;
	static SbVec3f downSideDamperForce;
	static SbVec3f dragForce;

	static float stretch;

	for (int i = 0; i < PARTICLE_COUNT; i++) {
		/*Applied streching force: F = k.x @ z-direction
		For the spring is stretched from the resting length
		*/ 
		if (i == 0) upSideSpringForce = particlePositions[i] - ceilingPoint;
		else		upSideSpringForce = particlePositions[i] - particlePositions[i - 1];
		float upSideSpringForceLength = upSideSpringForce.normalize();
		stretch = (piecewiseLength - upSideSpringForceLength);		
		upSideSpringForce = upSideSpringForce * (k * stretch);		

		downSideSpringForce = particlePositions[i + 1] - particlePositions[i];
		float downSideSpringForceLength = downSideSpringForce.normalize(); 
		stretch = (piecewiseLength - downSideSpringForceLength); 
		downSideSpringForce = downSideSpringForce * (k * stretch);

		/*Applied damping force @ z-direction
		For the spring is stretched from the resting length
		*/
		if (i == 0) upSideDamperForce = particleVelocities[i];
		else		upSideDamperForce = particleVelocities[i] - particleVelocities[i - 1];
		upSideDamperForce = upSideDamperForce * b;

		downSideDamperForce = particleVelocities[i + 1] - particleVelocities[i];
		downSideDamperForce = downSideDamperForce * b;

		// Drag force
		SbVec3f dummy = particleVelocities[i]; 
		dummy.normalize();
		dragForce = (dummy * particleVelocities[i].sqrLength() * cd); // F = cd * V^2
		dragForce.negate(); 

		particleForces[i] = dragForce + upSideSpringForce - upSideDamperForce - weight; 
		if (i != PARTICLE_COUNT - 1)	particleForces[i] += (-downSideSpringForce + downSideDamperForce);
	}
	
	if (isWindXOn || isWindZOn) { 
		if (isWindXOn) {
			SbVec3f WindX = { windStrength, 0, 0 };
			particleForces[selectedParticle] += WindX;
		}
		if (isWindZOn) {
			SbVec3f WindZ = { 0, 0, windStrength };
			particleForces[selectedParticle] += WindZ;
		}
	}
}

void update_frame_callback(void* data, SoSensor* timer) {
	forces(); // Calculate the forces acting on each particle using Euler method

	for (int i = 0; i < PARTICLE_COUNT; i++) {
		particleAccelerations[i] = particleForces[i] / m; // F = m.a
		particleVelocities[i] += particleAccelerations[i] * dt; 
		particlePositions[i] += particleVelocities[i] * dt; 
		particleTransforms[i]->translation.setValue(particlePositions[i]);
		particleAndceilingPositions[i + 1] = particlePositions[i]; // Update the positions
	}
	controlPts->point.setValues(0, PARTICLE_COUNT + 1, particleAndceilingPositions); // Update the control points
	indicatorTF->translation.setValue(-50, -30 + (windStrength * 0.75) / 2, 0);
	strengthIndicator->height = windStrength * 0.75;
	change_material_color(indicatorMaterial, { windStrength / 100, 1 - windStrength / 100, 0 });
}

int main(int, char** argv)
{
	srand(1); 

	particleAndceilingPositions[0] = ceilingPoint; 

	for (int i = 0; i < PARTICLE_COUNT; i++) {
		particlePositions[i].setValue(0, 49 - i, 0); 
		particleAndceilingPositions[i + 1] = particlePositions[i];
	}

	for (int i = 0; i < PARTICLE_COUNT + 5; i++) { // Fill the knot vector: [0,0,0,0, ........... ,n,n,n,n] where n is PARTICLE_COUNT - 2
		if (i < 4)						knots[i] = 0;
		else if (i > PARTICLE_COUNT)	knots[i] = PARTICLE_COUNT - 2;
		else							knots[i] = i - 3;
	}

#ifdef PRINT_PARTICLE_POS
	for (int i = 0; i < PARTICLE_COUNT; i++) {
		float a = 0, b = 0, c = 0;
		particlePositions[i].getValue(a, b, c);
		printf_s("%.2f ", b);
	}
	cout << endl;
#endif
	// Animation part
	HWND window = SoWin::init(argv[0]);
	if (window == NULL)
		exit(1);
	SoWinExaminerViewer* viewer = new SoWinExaminerViewer(window);

	SoSeparator* rope = new SoSeparator(); // Parent-most object in the hierarchy

	SoSeparator* particleSeperator[PARTICLE_COUNT]; 
	SoSphere* particleSpheres[PARTICLE_COUNT]; 

	SoSeparator* nurbsSeperator = make_curve(); 

	// Arrange ceiling view
	SoSeparator* ceilingSeperator = new SoSeparator;
	SoMaterial* ceilingMaterial = new SoMaterial;
	ceilingMaterial->ambientColor.setValue(0, 0, .2);
	ceilingMaterial->diffuseColor.setValue(0, 0, .4);
	ceilingMaterial->specularColor.setValue(0, 0, .4);
	SoCube* ceilingBox = new SoCube;
	ceilingBox->height = 2;
	ceilingBox->width = 80;
	ceilingBox->depth = 25;
	SoTransform* ceilingTF = new SoTransform;
	ceilingTF->translation.setValue(0, ceilingPoint.getValue()[1], 0);
	ceilingSeperator->addChild(ceilingTF);
	ceilingSeperator->addChild(ceilingMaterial);
	ceilingSeperator->addChild(ceilingBox);

	indicatorTF = new SoTransform;
	indicatorTF->translation.setValue(-50, -30 + (windStrength * 0.75) / 2, 0);
	SoSeparator* indicatorSeperator = new SoSeparator;
	indicatorMaterial = new SoMaterial;
	strengthIndicator = new SoCylinder;
	strengthIndicator->radius = 5;
	strengthIndicator->height = windStrength;
	indicatorSeperator->addChild(indicatorTF);
	indicatorSeperator->addChild(indicatorMaterial);
	indicatorSeperator->addChild(strengthIndicator);

	rope->addChild(indicatorSeperator);
	rope->addChild(nurbsSeperator); // Add the rope
	rope->addChild(ceilingSeperator); // Add the ceiling


	for (int i = 0; i < PARTICLE_COUNT; i++) {
		particleSpheres[i] = new SoSphere;    // Particle visual
		particleSpheres[i]->radius = radius;

		particleSeperator[i] = new SoSeparator; 
		particleTransforms[i] = new SoTransform; // Particle transformation
		particleMaterials[i] = new SoMaterial; 

		particleSeperator[i]->addChild(particleTransforms[i]); // Transform of a particle
		particleSeperator[i]->addChild(particleMaterials[i]); 
		particleSeperator[i]->addChild(particleSpheres[i]); 
		rope->addChild(particleSeperator[i]); // All particle seperators to the rope
	}
	change_material_color(particleMaterials[selectedParticle], { 0.9, 0, 0 }); // Highlight selected particle as red

	SoEventCallback* myEventCB = new SoEventCallback;	// Keyboard events
	myEventCB->addEventCallback(SoKeyboardEvent::getClassTypeId(), key_press_callback);
	rope->addChild(myEventCB);

	cout << "         SIMULATION CONTROLS         \n";
	cout << "-------------------------------------\n";
	cout << "*     R    -> Apply wind @ +x direction\n";
	cout << "*     B    -> Apply wind @ -z direction\n";
	cout << "*  UP/DOWN -> Change particle to applied wind\n";
	cout << "*RIGHT/LEFT-> Change magnitude of applied wind\n";
	cout << "* Spacebar -> Stop the animation.\n";

	timer->setFunction(update_frame_callback);
	timer->setInterval(updateInterval);
	if (!timer->isScheduled()) timer->schedule();

	// Show the animation
	viewer->setSceneGraph(rope);
	viewer->setTitle("MECH 534 - Project 2");
	viewer->show();
	SoWin::show(window);
	SoWin::mainLoop();
	return 0;
}
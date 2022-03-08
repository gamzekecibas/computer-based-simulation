/***************************************************************
 * MECH 534 - Computer-Based Modeling and Simulation
 * Homework 1
 * Reads the given data file and store its content 
 * into three “structures”: 
 * - one for the vertices (using an array of SbVec3f objects)
 * - one for the polygons (using an array of 4-vectors)
 * - one for the polyhedron (aggregate vertex and polygon info)
 *
 * Gamze Keçibaş
 * 60211
 * SPRING - 2022
 ***************************************************************/

#include <Inventor/Win/SoWin.h>
#include <Inventor/Win/viewers/SoWinExaminerViewer.h>

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <fstream>
#include <string>
#include <iostream>
using namespace std;

// input and output filenames definitions
#define FILENAME_IN "SmallSphere02.txt"
#define FILENAME_OUT "SmallSphere02-out.txt"

// FILL THIS STRUCT TO STORE THE CONTENTS OF THE POLYHEDRON
// DON'T CHANGE THE FIELD NAMES WITHIN THE STRUCT (i.e. those indicated within comments)
typedef struct 
{
	//numVertices --> the number of vertices 
	int numVertices;
	//numPolygons --> the number of polygons
	int numPolygons;
	//vertices --> the vertices (store the x,y, and z coordinates) array of size 3 x numVertices
	double** vertices;
	//polygons --> the polygons (store the connectivity information) array of size 3 x numPolygons
	short** polygons;
} Polyhedron;

FILE* input_txt;       // source = https://www.programiz.com/c-programming/c-file-input-output
int main(int, char ** argv)
{
	Polyhedron p; // USE THIS FOR COMPLETING THE ASSIGNMENT
	int numVertices, numPolygons;

	// read file
	input_txt = fopen(FILENAME_IN, "r");
	if(input_txt == NULL){
		printf("Error! Not opening the file!");
		exit(-1);
	}

	// Process vertices
	{
		// get number of vertices
		int ret = fscanf(input_txt, "%d\n", &numVertices);
		p.numVertices = numVertices;

		// create polygons array in shape 3xNUM_VERTICES
		p.vertices = new double* [3];
		for (int i = 0; i < 3; i++) {
			p.vertices[i] = new double[p.numVertices];
		}

		/*
		* retrieveand assign x, y, z coord values of vertices
		* to previously instantiated vertices array.
		*/
		for (int i = 0; i < p.numVertices; i++) {
			float x, y, z;
			int ret = fscanf(input_txt, "%f\t%f\t%f\n", &x, &y, &z);
			p.vertices[0][i] = x;
			p.vertices[1][i] = y;
			p.vertices[2][i] = z;
		}
	}
		
	// Process polygons
	{
		// get number of polygons
		int ret = fscanf(input_txt, "%d\n", &numPolygons);
		p.numPolygons = numPolygons;

		// create polygons array in shape 3xNUM_POLYGONS
		// short dtype occupies less memory than int type!
		p.polygons = new short* [3];
		for (int i = 0; i < 3; i++) {
			p.polygons[i] = new short[p.numPolygons];
		}

		/*
		* retrieveand assign polygon relationships
		* to previously instantiated polygons array.
		*/
		for (int i = 0; i < p.numPolygons; i++) {
			int p1, p2, p3;
			int ret = fscanf(input_txt, "%d\t%d\t%d\n", &p1, &p2, &p3);
			p.polygons[0][i] = p1;
			p.polygons[1][i] = p2;
			p.polygons[2][i] = p3;
		}
	}

	// write to file
	{
		ofstream out(FILENAME_OUT);
		out << std::showpoint;
		out << p.numVertices << endl;
		for (int i = 0; i < p.numVertices; i++) {
			out << p.vertices[0][i] << "\t" << p.vertices[1][i] << "\t" << p.vertices[2][i] << endl;
		}

		out << p.numPolygons << endl;
		for (int i = 0; i < p.numPolygons; i++) {
			out << p.polygons[0][i] << "\t" << p.polygons[1][i] << "\t" << p.polygons[2][i] << endl;
		}
		out.close();
	}
	
	/***********************************************************************************
	// DON'T EDIT AFTER THIS POINT
	// IF ALL VARIABLE NAMES ARE INTACT AND FILE READING IS CORRECT, 
	// UNCOMMENT THE NECESSARY LINES TO SEE THE BALL ON THE SCREEN!
	************************************************************************************/
  	HWND window = SoWin::init(argv[0]);
  	if ( window == NULL ) 
  		exit(1);

  	SoWinExaminerViewer *viewer = new SoWinExaminerViewer(window);
	SoSeparator *root = new SoSeparator;
	
	// Draw the object read from the file
	// 3D coordinates of the vertices and indices of the vertices of each 
	// triangular polygon used for rendering in OpenInventory
	SbVec3f *vertexCoordinates = new SbVec3f[p.numVertices];
	
	// indices to be read from 
	// data is triangular and a -1 entry is required in the end for SoIndexedFaceSet data structure
	int *vertexIndices = new int[4*p.numPolygons];

	for(int i=0; i < p.numVertices; i++)
	{	
		vertexCoordinates[i] = SbVec3f(p.vertices[0][i],p.vertices[1][i],p.vertices[2][i]);
	}
	
	// form polygon of triangles
	for(int i=0; i < p.numPolygons; i++)
	{	
		vertexIndices[i*4]	 = p.polygons[0][i];
		vertexIndices[i*4+1] = p.polygons[1][i];
		vertexIndices[i*4+2] = p.polygons[2][i];
		vertexIndices[i*4+3] = -1; // last element needs to be -1
	}
	// make a new object
	SoSeparator *polyObject = new SoSeparator;

	// holds the vertex coordinates
	SoCoordinate3 *coordinates = new SoCoordinate3;
	coordinates->point.setValues(0, p.numVertices, vertexCoordinates);
	// holds connectivity information
	SoIndexedFaceSet *faces = new SoIndexedFaceSet;
	faces->coordIndex.setValues(0, 4*p.numPolygons, vertexIndices);
	
	polyObject->addChild(coordinates);
	polyObject->addChild(faces);
	
	root->addChild(polyObject);

	viewer->setSceneGraph(root);
  	viewer->setTitle("MECH 534 - Homework 1");
  	viewer->show();

  	SoWin::show(window);
  	SoWin::mainLoop();
  	return 0;
}
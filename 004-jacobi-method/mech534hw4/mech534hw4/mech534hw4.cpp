/***************************************************************
 * MECH 534 - Computer-Based Modeling and Simulation
 * Homework 4 Jacoby Method Implementation
 *
 * CONTENT
 * a-d) Derivations are available in PDF
 * e) its implementation of k1=k2=k3 = 1 & m1=m2 = 1 
 * f)  For initial conditions -> t = 0 & x1 = 1 & x2=x1_dot=x2_dot = 0 

 *
 * Gamze Keçibaş
 * 60211
 * SPRING - 2022
 ***************************************************************/

// Import required global & local libraries
#include <iostream>
#include "nrutil.h"
#include "nr.h"

using namespace std;

int main()
{
	int n = 2;
	float** A = matrix(1, n, 1, n);
	float** v = matrix(1, n, 1, n);

	// e) Using equation from part b & its values are verified in part d
	cout << "PART E\n";
	for (int row = 1; row <= n; row++) {
		for (int col = 1; col <= n; col++)
		{
			cout << "Enter A[" << row << "," << col << "] value: ";
			cin >> A[row][col];
		}
	}

	cout << "\nA matrix is:\n";

	for (int row = 1; row <= n; row++) {
		for (int col = 1; col <= n; col++)
		{
			cout << A[row][col] << "\t";
		}
		cout << "\n";
	}

	// PART F
	// initial conditions -> t = 0 & x1 = 1 & x2=x1_dot=x2_dot = 0 
	cout << "\nPART F";
	int nrot;
	float* d = vector(1, n);

	jacobi(A, n, d, v, &nrot);

	cout << endl << "The eigenvalues are: " << endl;
	for (int i = 1; i <= n; i++) cout << d[i] << endl;

	cout << endl << "Eigenvector 1 is: " << endl;
	for (int row = 1; row <= n; row++) cout << v[row][1] << "\n";

	cout << endl << "Eigenvector 2 is: " << endl;
	for (int row = 1; row <= n; row++) cout << v[row][2] << "\n";

	free_matrix(A, 1, n, 1, n);
	free_matrix(v, 1, n, 1, n);
	free_vector(d, 1, n);

}
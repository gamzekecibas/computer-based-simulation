// MECH534 - HW5: Non-linear Differential Equations
// Gamze Keçibaş

#include <iostream>
#include <fstream>
#include "nr.h"
#include "nrutil.h"

using namespace std;

float** y, * xx; // Initiate final outputs
float b1, b2, c1, c2; // Initiate coefficients of equation

void deriveFunc(float x, float y[], float dydx[]) {
	dydx[1] = (b1 + -c1 * y[2]) * y[1];
	dydx[2] = (-b2 + c2 * y[1]) * y[2];
}


void LotVolt(float y[], float dydx[], int n, float x, float h, float yout[],
	void (*deriveFunc)(float, float[], float[]))
	/*	Given values for the variables y[1..n] and their derivatives dydx[1..n] known at x, use the
		Lotka - Volterra system to advance the solution over an interval h and return the
		incremented variables as yout[1..n], which need not be a distinct array from y.The user
		supplies the routine deriveFunc(x, y, dydx), which returns derivatives dydx at x. 
	*/
{

	int i;
	float xh, hh, h6, * dym, * dyt, * yt;
	dym = vector(1, n);
	dyt = vector(1, n);
	yt = vector(1, n);
	hh = h * 0.5;
	h6 = h / 6.0;
	xh = x + hh;
	for (i = 1; i <= n; i++) yt[i] = y[i] + hh * dydx[i];	// First derivative
	(*deriveFunc)(xh, yt, dyt);									
	for (i = 1; i <= n; i++) yt[i] = y[i] + hh * dyt[i];	// Second derivative
	(*deriveFunc)(xh, yt, dym);									
	for (i = 1; i <= n; i++) {								// Update derivative elements
		yt[i] = y[i] + h * dym[i];
		dym[i] += dyt[i];
	}
	(*deriveFunc)(x + h, yt, dyt);								
	for (i = 1; i <= n; i++)								// Calculate the result
		yout[i] = y[i] + h6 * (dydx[i] + dyt[i] + 2.0 * dym[i]);
	free_vector(yt, 1, n);
	free_vector(dyt, 1, n);
	free_vector(dym, 1, n);
}


void rkdumb(float vstart[], int nvar, float x1, float x2, int numSteps,
	void (*deriveFunc)(float, float[], float[]))
	/*	Starting from initial values vstart[1..nvar] known at x1 use simplified Lotka - Volterra system
		to advance # steps equal increments to x2.The user - supplied routine deriveFunc(x, v, dvdx)
		evaluates derivatives.Results are stored in the global variables y[1..nvar][1..numSteps + 1]
		and xx[1..numSteps + 1].
	*/
{
	void LotVolt(float y[], float dydx[], int n, float x, float h, float yout[],
		void (*deriveFunc)(float, float[], float[]));

	int i, k;
	float x, h;
	float* v, * vout, * dv;
	v = vector(1, nvar);
	vout = vector(1, nvar);
	dv = vector(1, nvar);
	for (i = 1; i <= nvar; i++) { 		
		v[i] = vstart[i];
		y[i][1] = v[i];
	}
	xx[1] = x1;
	x = x1;
	h = (x2 - x1) / numSteps;
	for (k = 1; k <= numSteps; k++) { // Take numSteps steps.		
		(*deriveFunc)(x, v, dv);
		LotVolt(v, dv, nvar, x, h, vout, deriveFunc);
		if ((float)(x + h) == x) nrerror("Step size too small in routine rkdumb");
		x += h;
		xx[k + 1] = x; // Store intermediate steps.
		for (i = 1; i <= nvar; i++) {
			v[i] = vout[i];
			y[i][k + 1] = v[i];
		}
	}
	free_vector(dv, 1, nvar);
	free_vector(vout, 1, nvar);
	free_vector(v, 1, nvar);
}

int main()
{
	float n = 2; // # variables

	float t_start = 0;
	float t_final;
	cout << "Enter t_final (sec): ";
	cin >> t_final;

	float numSteps; // # steps
	cout << "Enter desired number of steps: ";
	cin >> numSteps;

	cout << "\ndx1/dt = [b1 - (c1)(x2)][x1] -> \n";
	cout << "Enter b1 value: ";
	cin >> b1;
	cout << "Enter c1 value: ";
	cin >> c1;
	cout << "\ndx2/dt = [-b2 + (c2)(x1)][x2] -> \n";
	cout << "Enter b2 value: ";
	cin >> b2;
	cout << "Enter c2 value: ";
	cin >> c2;

	float* initial_values = vector(1, n); // Values given at t=0
	cout << "\nEnter initial conditions ->\n";
	cout << "Enter x1 at t=0: ";
	cin >> initial_values[1];
	cout << "Enter x2 at t=0: ";
	cin >> initial_values[2];

	y = matrix(1, n, 1, numSteps + 1); // Storing output & output time respectively
	xx = vector(1, numSteps + 1); 

	rkdumb(initial_values, n, t_start, t_final, numSteps, &deriveFunc); // Use previous functions

	ofstream out("gkecibas16_hw5_output.txt");
	
	cout << "\#Step		Prey		Predator	Time" << endl;
	cout << "====		====		========	====" << endl;
	for (int i = 1; i <= numSteps + 1; i++) {
		printf_s("%i		%.2f		%.2f		%.2f\n", i - 1, y[1][i], y[2][i], xx[i]); // Print the values
		out << y[1][i] << "\t" << y[2][i] << "\t" << xx[i] << endl;
	}
	out.close();
	cout << "\nOutput file is exported!! The file is closed now!\n";
}
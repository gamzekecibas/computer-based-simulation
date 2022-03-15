/***************************************************************
 * MECH 534 - Computer-Based Modeling and Simulation
 * Homework 2 Vector Operations
 * 
 * CONTENT
 * a) defining dot (inner) product using Numerical Recipes
 * b) defining cross product using Numerical Recipes
 * c) calculating volume of a box using a & b parts using provided vectors as
 * A = [2.0, 1.0, 0.1], B = [0.0, 0.3, -3.6], C = [0.2, 4.6, -0.1]
 * 
 * Gamze Keçibaş
 * 60211
 * SPRING - 2022
 ***************************************************************/

// Import required global & local libraries
#include <iostream>
#include "nr.h"
#include "nrutil.h"

using namespace std;

// a) defining dot(inner) product using Numerical Recipes
double dot_product(double* a, double* b, int n) {
    double sum = 0;
    for (int i = 0; i < n; i++)
        sum += a[i] * b[i];

    return sum;
}

// b) defining cross product using Numerical Recipes
double* cross_product(double* a, double* b) {
    double* result = dvector(1, 3);

    result[0] = (a[1] * b[2] - a[2] * b[1]);
    result[1] = -(a[0] * b[2] - a[2] * b[0]);
    result[2] = (a[0] * b[1] - a[1] * b[0]);

    return result;
}

// c) calculating volume of a box using a & b parts
double calculate_volume(double* a, double* b, double* c, int n) {
    double result = 0;
    result = abs(dot_product(cross_product(a, b), c, n));
    return result;
}

int main()
{
    int n;
    cout << "MECH534 HW2 PART - C:" << "\n";
    cout << "\nEnter dimension of the vectors: ";
    cin >> n;

    double* a;
    a = dvector(1, n);
    cout << "\n";
    for (int i = 0; i < n; i++) {
        cout << "Enter " << i+1 << "th element of Vector A: ";
        cin >> a[i];
    }

    double* b;
    b = dvector(1, n);
    cout << "\n";
    for (int i = 0; i < n; i++) {
        cout << "Enter " << i+1 << "th element of Vector B: ";
        cin >> b[i];
    }

    double* c;
    c = dvector(1, n);
    cout << "\n";
    for (int i = 0; i < n; i++) {
        cout << "Enter " << i+1 << "th element of Vector C: ";
        cin >> c[i];
    }

    double volume = calculate_volume(a, b, c, n);
    cout << "\nDefined Vectors:" << "\n";
    printf("Vector A = [%.3f % .3f % .3f]", a[0], a[1], a[2]);
    printf("\nVector B = [%.3f % .3f % .3f]", b[0], b[1], b[2]);
    printf("\nVector C = [%.3f % .3f % .3f]\n", c[0], c[1], c[2]);

    free_dvector(a, 1, n);
    free_dvector(b, 1, n);
    free_dvector(c, 1, n);

    printf("\nVolume of a box defined by the vectors A, B, C is: %.3f \n", volume);
}

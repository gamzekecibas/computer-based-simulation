/***************************************************************
 * MECH 534 - Computer-Based Modeling and Simulation
 * Homework 3 LU Decomposition Method
 *
 * CONTENT
 * a) Derive linear equations to define potentials at nodes 1 through 6 [in GK-MECH534HW3.pdf]
 * b) Implement LU decomposition method to calculate voltage of each node
 *
 * Gamze Keçibaş
 * SPRING - 2022
 ***************************************************************/

// Import required global & local libraries
#include <iostream>
#include "nrutil.h"
#include "nr.h"

using namespace std;

int main()
{
    int n;
    float x;
    cout << "Enter # equations: ";
    cin >> n;
    cout << "You have " << n << " equations.\nSize of the coefficient matrix is " << n << "x" << n << ".\n";

    float** A = matrix(1, n, 1, n);
    memset(A[1], 0, (n*n + 1) * sizeof(float));
    float* b = vector(1, n);
    memset(b, 0, (n + 1) * sizeof(float));

    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++)
        {
            cout << "Enter A[" << i << "," << j << "] value: ";
            cin >> A[i][j];
        }
        cout << "Enter b[" << i << "] value: ";
        cin >> b[i];
        cout << "\n";
    }

    cout << "A matrix is:\n";

    for (int i = 1; i <= n; i++) {
        for (int j= 1; j <= n; j++)
        {
            cout << A[i][j] << "\t";
        }
        cout << "\n";
    }

    int* ind = ivector(1, n);
    float d;

    ludcmp(A, n, ind, &d);
    lubksb(A, n, ind, b);

    cout << endl << "Potential of each node: " << endl;
    for (int i = 1; i <= n; i++) {
        cout << "V" << i << " = " << b[i] << "\n";
    }
}
// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#pragma once

#include <Math.Matrix.h>

#include <cmath>

// Analytical solution of a system of two linear equation (lines intersection)
inline bool SolveSystemOfTwoLinearEquations( double* p, double a1, double b1, double c1, double a2, double b2, double c2 )
{
	const double det = a1 * b2 - a2 * b1;
	if( fabs( det ) < std::numeric_limits<double>::epsilon() ) {
		return false;
	}
	p[0] = ( b1 * c2 - b2 * c1 ) / det;
	p[1] = ( a2 * c1 - a1 * c2 ) / det;
	return true;
}

const int ANALYTICAL_INVERT_MATRIX_MAX_SIZE = 3;
bool AnalyticalInvertMatrix( CMatrix<double>& I, const CMatrix<double>& A );

// Solution of Ax = y
bool SolveSystemOfLinearEquations( CMatrix<double>& x, const CMatrix<double>& A, const CMatrix<double>& y );

// Cached solution of Ax = y for constant A
struct CSolveSystemOfLinearEquationsCache {
	CMatrix<double> L; // Lower triangular matrix of LU decomposition
	CMatrix<double> U; // Upper triangular matrix of LU decomposition
	std::vector<int> P; // Permutations matrix of LU decomposition
	CMatrix<double> I; // Analytical solution for small matrices

	bool IsValid() const { return P.size() > 0 || I.Size() > 0; }
};
bool SolveSystemOfLinearEquations( CMatrix<double>& x, CSolveSystemOfLinearEquationsCache& cache, const CMatrix<double>& A, const CMatrix<double>& y );

// Least squares solution for a system M * x = y in the form ( M^T * M )* x = M^T * y,
bool SolveLeastSquares( CMatrix<double>& x, const CMatrix<double>& M, const CMatrix<double>& y );

// Cached least squares for constant M
struct CSolveLeastSquaresCache {
	CSolveSystemOfLinearEquationsCache LU;
	CMatrix<double> MT; // M^T
};
bool SolveLeastSquares( CMatrix<double>& x, CSolveLeastSquaresCache& cache, const CMatrix<double>& M, const CMatrix<double>& y );

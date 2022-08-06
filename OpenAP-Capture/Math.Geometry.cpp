// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include <Math.LinearAlgebra.h>

#include <cassert>

bool LeastSquaresAffineTransform( CMatrix<double>& Ax, CMatrix<double>& Ay,
	const std::vector<double>& x1, const std::vector<double>& y1, const std::vector<double>& x2, const std::vector<double>& y2 )
{
	int count = x1.size();
	assert( count > 2 );

	CMatrix<double> M( count, 3 );

	for( int i = 0; i < count; i++ ) {
		M[i][0] = x1[i]; M[i][1] = y1[i]; M[i][2] = 1;
	}

	CMatrix<double> X( count, 1 );
	for( int i = 0; i < count; i++ ) {
		X[i][0] = x2[i];
	}

	CMatrix<double> Y( count, 1 );
	for( int i = 0; i < count; i++ ) {
		Y[i][0] = y2[i];
	}

	CSolveLeastSquaresCache cache;
	if( SolveLeastSquares( Ax, cache, M, X ) && SolveLeastSquares( Ay, cache, M, Y ) ) {
		return true;
	}
	return false;
}

bool InverseAffineTransform( CMatrix<double>& invAx, CMatrix<double>& invAy, const double* Ax, const double* Ay )
{
	double a11 = Ax[0];
	double a12 = Ax[1];
	double a13 = Ax[2];
	double a21 = Ay[0];
	double a22 = Ay[1];
	double a23 = Ay[2];
	double det = a11 * a22  - a21 * a12;
	if( fabs( det ) < std::numeric_limits<double>::epsilon() ) {
		return false;
	}
	invAx[0][0] = a22 / det;
	invAx[1][0] = - a12 / det;
	invAx[2][0] = ( a12 * a23 - a22 * a13 ) / det;
	invAy[0][0] = - a21 / det;
	invAy[1][0] = a11 / det;
	invAy[2][0] = ( a21 * a13 - a11 * a23 ) / det;

	return true;
}

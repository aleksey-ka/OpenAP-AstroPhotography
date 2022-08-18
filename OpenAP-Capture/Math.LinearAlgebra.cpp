// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include <Math.LinearAlgebra.h>

#include <cassert>
#include <cstring>

bool AnalyticalInvertMatrix( CMatrix<double>& I, const CMatrix<double>& A )
{
	switch( A.Rows() ) {
		case 0 : return false;
		case 1 :
			if( fabs( A[0][0] ) < std::numeric_limits<double>::epsilon() ) {
				return false;
			}
			I.SetSize( 1, 1 );
			I[0][0] = 1 / A[0][0];
			break;
		case 2 : {
				auto det = A[0][0] * A[1][1] - A[0][1] * A[1][0];
				if( fabs( det ) < std::numeric_limits<double>::epsilon() ) {
					return false;
				}
				I.SetSize( 2, 2 );
				I[0][0] = A[1][1] / det;
				I[0][1] = -A[0][1] / det;
				I[1][0] = -A[1][0] / det;
				I[1][1] = A[0][0] / det;
			}
			break;
		case ANALYTICAL_INVERT_MATRIX_MAX_SIZE : {
				auto det = A[0][0] * A[1][1] * A[2][2] + A[1][0] * A[2][1] * A[0][2] +
					A[2][0] * A[0][1] * A[1][2] - A[0][0] * A[2][1] * A[1][2] -
					A[2][0] * A[1][1] * A[0][2] - A[1][0] * A[0][1] * A[2][2];
				if( fabs( det ) < std::numeric_limits<double>::epsilon() ) {
					return false;
				}
				I.SetSize( 3, 3 );
				I[0][0] = ( A[1][1] * A[2][2] - A[1][2] * A[2][1] ) / det;
				I[0][1] = ( A[0][2] * A[2][1] - A[0][1] * A[2][2] ) / det;
				I[0][2] = ( A[0][1] * A[1][2] - A[0][2] * A[1][1] ) / det;
				I[1][0] = ( A[1][2] * A[2][0] - A[1][0] * A[2][2] ) / det;
				I[1][1] = ( A[0][0] * A[2][2] - A[0][2] * A[2][0] ) / det;
				I[1][2] = ( A[0][2] * A[1][0] - A[0][0] * A[1][2] ) / det;
				I[2][0] = ( A[1][0] * A[2][1] - A[1][1] * A[2][0] ) / det;
				I[2][1] = ( A[0][1] * A[2][0] - A[0][0] * A[2][1] ) / det;
				I[2][2] = ( A[0][0] * A[1][1] - A[0][1] * A[1][0] ) / det;
			}
			break;
		default:
			assert( false );
	};
	return true;
}

static bool findLUDecomposeOrInverse( CSolveSystemOfLinearEquationsCache& result, const CMatrix<double>& A )
{
	int rows = A.Rows();
	if( rows <= ANALYTICAL_INVERT_MATRIX_MAX_SIZE ) {
		// Analytical solution
		return AnalyticalInvertMatrix( result.I, A );
	}

	// LU-decomposition
	auto& L = result.L;
	auto& U = result.U;
	auto& permutations = result.P;

	int cols = A.Columns();
	assert( rows == cols );

	// Set L to identity
	L.SetSize( rows, cols );
	for( int i = 0; i < rows; i++ ) {
		for( int j = 0; j < rows; j++ ) {
			L[i][j] = ( i == j ? 1.0 : 0.0 );
		}
	}

	// U is a copy of A
	U.SetSize( rows, cols );
	memcpy( U.Ptr(), A.Ptr(), static_cast<size_t>( A.Size() ) * sizeof( double ) );

	// Permutations matrix
	permutations.resize( static_cast<size_t>( rows ) );
	for( int i = 0; i < rows; i++ ) {
		permutations[static_cast<size_t>( i )] = i;
	}

	for( int n = 0; n < cols - 1; n++ ) {
		// Searching for raws with largest pivot
		double max = 0.0;
		int maxn = -1;
		for( int i = n; i < rows; i++ ) {
			double value = fabs( U[i][n] );
			if( value > max ) {
				max = value;
				maxn = i;
			}
		}
		if( maxn == -1 ) {
			permutations.clear();
			return false;
		}

		if( n != maxn ) {
			// Swapping raws
			std::swap( permutations[static_cast<size_t>( n )], permutations[static_cast<size_t>( maxn )] );
			for( int i = 0; i < n; i++ ) {
				// Swapping raws in L
				std::swap( L[n][i], L[maxn][i] );
			}
			for( int i = 0; i < cols; i++ ) {
				// Swapping raws in U
				std::swap( U[n][i], U[maxn][i] );
			}
		}

		double Ukk = U[n][n];
		for( int i = n + 1; i < rows; i++ ) {
			double temp = U[i][n] / Ukk;
			for( int j = n; j < cols; j++ ) {
				U[i][j] -= temp * U[n][j];
			}
			L[i][n] = temp;
		}
	}
	return true;
}

static bool solveSystemOfLinearEquations( CMatrix<double>& x, const CSolveSystemOfLinearEquationsCache& cache, const CMatrix<double>& y )
{
	int rows = y.Rows();
	if( rows <= ANALYTICAL_INVERT_MATRIX_MAX_SIZE ) {
		// Analytical solution
		if( cache.I.Size() > 0 ) {
			CMatrix<double>::Multiply( x, cache.I, y );
			return true;
		}
		return false;
	}

	// Solving Ax = y through LU-decomposition in O( 2/3 n^3)

	auto& L = cache.L;
	auto& U = cache.U;
	auto& permutations = cache.P;

	assert( rows == L.Rows() );

	// Selecting raws in y according to permutations matrix
	CMatrix<double> _y( rows, 1 );
	for( int i = 0; i < rows; i++ ) {
		_y[i][0] = y[permutations[static_cast<size_t>( i )]][0];
	}

	// Solving Lz = _y where L is the lower triangular matrix
	CMatrix<double> z( rows, 1 );
	for( int i = 0; i < rows; i++ ) {
		double temp = _y[i][0];
		for( int j = 0; j < i; j++ ) {
			temp -= L[i][j] * z[j][0];
		}
		z[i][0] = temp / L[i][i];
	}

	// Solving Ux = z where U is the upper triangular matrix
	x.SetSize( rows, 1 );
	for( int i = rows - 1; i > -1; i-- ) {
		double temp = z[i][0];
		for( int j = rows - 1; j > i; j-- ) {
			temp -= U[i][j] * x[j][0];
		}

		double Uii = U[i][i];
		if( fabs( Uii ) < std::numeric_limits<double>::epsilon() ) {
			x.SetSize( 0, 0 );
			return false;
		}

		temp /= Uii;

		if( std::isfinite( temp ) ) {
			x[i][0] = temp;
		} else {
			x.SetSize( 0, 0 );
			return false;
		}
	}

	return true;
}

bool SolveSystemOfLinearEquations( CMatrix<double>& x, CSolveSystemOfLinearEquationsCache& cache, const CMatrix<double>& A, const CMatrix<double>& y )
{
	if( !cache.IsValid() ) {
		if( !findLUDecomposeOrInverse( cache, A ) ) {
			x.SetSize( 0, 0 );
			return false;
		}
	}

	return solveSystemOfLinearEquations( x, cache, y );
}

bool SolveSystemOfLinearEquations( CMatrix<double>& x, const CMatrix<double>& A, const CMatrix<double>& y )
{
	CSolveSystemOfLinearEquationsCache cache;
	return SolveSystemOfLinearEquations( x, cache, A, y );
}

bool SolveLeastSquares( CMatrix<double>& x, const CMatrix<double>& M, const CMatrix<double>& y )
{
	// Solving M * x = y as least squares in the form ( M^T * M )* x = M^T * y

	CMatrix<double> MT( M );
	MT.Transpose(); // M^T

	CMatrix<double> MTM( MT );
	MTM.Multiply( M ); // M^T * M

	CMatrix<double> MTY( MT );
	MTY.Multiply( y ); // M^T * y

	return SolveSystemOfLinearEquations( x, MTM, MTY );
}

bool SolveLeastSquares( CMatrix<double>& x, CSolveLeastSquaresCache& cache, const CMatrix<double>& M, const CMatrix<double>& y )
{
	// Cached least squares
	if( cache.MT.Size() == 0 ) {
		if( M.Rows() > M.Columns() ) {
			// Performing LU-decomposition once for matrix (М*М^T)
			cache.MT = M;
			cache.MT.Transpose();

			CMatrix<double> MTM( cache.MT );
			MTM.Multiply( M );

			findLUDecomposeOrInverse( cache.LU, MTM );
		} else {
			// Optimization for square matrices. Directly solving M * x = y
			findLUDecomposeOrInverse( cache.LU, M );
		}
	}

	// Solving for each vector y
	if( cache.LU.IsValid() ) {
		if( M.Rows() > M.Columns() ) {
			CMatrix<double> MTY( cache.MT );
			MTY.Multiply( y );
			return solveSystemOfLinearEquations( x, cache.LU, MTY );
		} else {
			// Optimization for square matrices. Directly solving M * x = y
			return solveSystemOfLinearEquations( x, cache.LU, y );
		}
	}
	return false;
}

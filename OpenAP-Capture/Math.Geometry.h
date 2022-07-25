// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#pragma once

#include <Math.Matrix.h>

bool LeastSquaresAffineTransform( CMatrix<double>& Ax, CMatrix<double>& Ay,
	const std::vector<double>& x1, const std::vector<double>& y1, const std::vector<double>& x2, const std::vector<double>& y2 );

bool InverseAffineTransform( CMatrix<double>& invAx, CMatrix<double>& invAy, const double* Ax, const double* Ay );

// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef MATH_MATRIX_H
#define MATH_MATRIX_H

#include <vector>

template<typename T>
class CMatrix {
public:
	CMatrix() = default;
	CMatrix( int rows, int columns ) { SetSize( rows, columns ); }

	CMatrix( const CMatrix& ) = default;
	CMatrix( CMatrix&& ) = default;

	CMatrix& operator = ( const CMatrix& ) = default;
	CMatrix& operator =(  CMatrix&& ) = default;

	int Size() const { return rows * columns; }
	void SetSize( int _rows, int _columns, bool onlyGrow = true );

	int Rows() const { return rows; }
	int Columns() const { return columns; }

	const T* operator [] ( int row ) const { return data.data() + row * columns; }
	T* operator [] ( int row ) { return data.data() + row * columns; }

	const T& Last() const { return data[rows * columns - 1]; }
	T& Last() { return data[rows * columns - 1]; }

	const T* Ptr() const { return data.data(); }
	T* Ptr() { return data.data(); }

	void Transpose();
	void Multiply( const CMatrix<T>& );

	static void Multiply( CMatrix<T>&, const CMatrix<T>&, const CMatrix<T>& );

private:
	std::vector<T> data;
	int rows = 0;
	int columns = 0;
};

template<typename T>
inline void CMatrix<T>::SetSize( int _rows, int _columns, bool onlyGrow )
{
	size_t newSize = _rows * _columns;
	if( ( !onlyGrow ) || newSize > data.size() ) {
		// Only grow by default
		data.resize( newSize );
	}
	rows = _rows;
	columns = _columns;
}

template<typename T>
inline void CMatrix<T>::Transpose()
{
	CMatrix<T> temp( *this );
	SetSize( columns, rows );
	for( int i = 0; i < rows; i++ ) {
		for( int j = 0; j < columns; j++ ) {
			(*this)[i][j] = temp[j][i];
		}
	}
}

template<typename T>
inline void CMatrix<T>::Multiply( CMatrix<T>& result, const CMatrix<T>& a, const CMatrix<T>& b )
{
	result.SetSize( a.rows, b.columns );
	for( int i = 0; i < result.rows; i++ ) {
		for( int j = 0; j < result.columns; j++ ) {
			T sum = T();
			for( int n = 0; n < a.columns; n++ ) {
				sum += a[i][n] * b[n][j];
			}
			result[i][j] = sum;
		}
	}
}

template<typename T>
inline void CMatrix<T>::Multiply( const CMatrix<T>& m )
{
	CMatrix<T> temp( *this );
	Multiply( *this, temp, m );
}

#endif // MATH_MATRIX_H

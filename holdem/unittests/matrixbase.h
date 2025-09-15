/***************************************************************************
 *   Copyright (C) 2014 by Joseph Huang                                    *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef HOLDEM_MatrixBase
#define HOLDEM_MatrixBase

#include <stddef.h>
#include <string>

#include "../src/portability.h"
#include "../src/debug_flags.h"


struct SizedArray {
    float64 *data = 0;
    size_t size = 0;
}
;

class Matrix {
public:
    enum Arrangement { ROW_MAJOR, COLUMN_MAJOR };

    Matrix(size_t rows, size_t columns, Arrangement arrangement);
    virtual ~Matrix();

    Matrix(const Matrix &matrix);
    Matrix & operator=(const Matrix &matrix);

    float64& at(size_t row, size_t column);
    float64 get(size_t row, size_t column) const;

    struct SizedArray getRow(size_t rownum);
    struct SizedArray getColumn(size_t colnum);

    /**
     * Set all elements to zero.
     */
    void setZero();

    static Matrix newIdentityMatrix(size_t size, Arrangement arrangement);

    std::string toString() const;

    const size_t fRows;
    const size_t fColumns;
private:


    float64* fData;
    Arrangement fArrangement;

    size_t getIdx(size_t row, size_t column) const;
};


// Define the equation
//   _coefficients[0] x_0 + ... + _coefficients[n] x_n = _val
// _me must be the name of a NormalizedSystemOfEquations object
// _coefficients must be the name of an array on the stack
// _val must be a scalar
#define NormalizedSystemOfEquations_addEquation(_me, _coefficients, _val) do { \
struct SizedArray _ilistarray; \
_ilistarray.data = _coefficients; \
_ilistarray.size = sizeof(_coefficients) / sizeof((_coefficients)[0]); \
(_me).addEquation(_ilistarray, _val); \
} while (0)


/**
 * Discussion:
 *  We will have some homogeneous equations of the form
 *    A  \vec x = \vec 0
 *
 *  Where A is a flat matrix (m x n , m<n)
 *
 *  We can express A^T = QR where R is tall upper triangular (with bottom row(s) zero) and Q is large unitary.
 *
 *  That means:
 *    R^T Q^T \vec x = \vec 0
 *  and the rightmost columns of R^T are zero.
 *  Now, because Q^T is unitary its rows form a basis onto which \vec x is projected.
 *  The rows of Q^T that correspond to the all-zero columns (rightmost) of R^T are the subspace that permits a non-trivial solution to the homogenous equations.
 *
 *  Thus, the solution to the system is the bottom rows of Q^T, or the rightmost columns of Q.
 *
 *  To determine Q, apply successive Householder reflections to A^T.
 *
 *   Q_n ... Q_2 Q_1 A^T = R
 *
 *   In this case, Q = Q_1^T Q_2^T ... Q_n^T and Q^T = Q_n ... Q_2 Q_1 I
 *
 *   So algorithmically, we need only apply the same sequence of Householder transformations to A^T and I, yeilding R and Q^T
 *   When we're done, we return the bottom rows of Q^T that solve our system.
 *
 *   Algorithmically, we need to perform the following basic operations relatively efficiently and in-place if possible:
 *    + 2-norm squared of a subset of a column of a matrix (specifically, the first columns of the diagonal submatrices of A^T)
 *    + Apply a given Householder reflection to a subset of a column of a matrix (specifically, columns of the diagonal submatrices of A^T, and the columns of I and its diagonal submatrices)
 *       + Dot product a matrix column with a different matrix column
 *       + y = (I -  2|v| |v|^T)y
      -->  y = (I -  (2 / v^T v) v v^T)y
      -->  y -= (2 / v^T v) v v^T y
      -->  y -= (2 / v^T v) v (v^T y)
      -->  y -= 2((v^T y) / (v^T v))  v
      -->  y -= 2dot(v, y) / dot(v, v) * v
      -->  y += -2dot(v, y) / dot(v, v) * v
      -->  \vec y += scalar * \vec v
 *         a.k.a. add_equals(src, scalar, v, dim)
 *
 *   Storage:
 *     If A is stored RowMajor, then A^T is conveniently stored ColumnMajor
 *
 */
class NormalizedSystemOfEquations
{
    public:
    /**
     * Express a system of equations where all the variables are expected to add up to 1.0
     *
     *  e.g.
     *   0.5 x + 0.24 y - 0.1 z = 0.01;
     *   0.2 x - 2.0  y + 0.1 z = 0.5;
     *
     *  (There is also one implied constraint)
     *       x +      y +     z = 1.0;
     *
     *  Due to the structure of the implied constraint, we can always express the system of equations as a homogeneous system
     *  (-0.01 + 0.5) x + (-0.01 + 0.24) y + (-0.01 - 0.1) z = 0.0
     *  (-0.01 + 0.2) x + (-0.01 - 2.0 ) y + (-0.01 + 0.1) z = 0.0
     *
     *  Interally, we express this as a flat row-major matrix "A" of the form
     *   [ (-0.01 + 0.5)  ,   (-0.01 + 0.24)  ,   (-0.01 - 0.1) ]
     *   [ (-0.01 + 0.2)  ,   (-0.01 - 2.0 )  ,   (-0.01 + 0.1) ]
     *  In practice, we store A^T directly rather than A.
     *
     *  and we can solve Ax = 0 by finding the nullspace of A by computing the QR factorization of A^T.
     *
     */
    NormalizedSystemOfEquations(size_t variables);
    virtual ~NormalizedSystemOfEquations();

    void addEquation(struct SizedArray coefficients, float64 value);

    void solve();

    void getSolution(struct SizedArray *output);

private:

    const size_t fVariables;
    const size_t fEquations;

    Matrix At; // Before solving, this holds A^T ;  After solving, this holds R
    Matrix I; // Before solving, this holds I ;  After solving, this holds Q^T

    size_t fEquationsAdded;

    // Starts out false. It's set to true once we have solved.
    bool fSolved;
}
;


namespace ElementWise
{
    /**
     * Return the dot product of x[0] * y[0]  +  x[1] * y[1]  +  ...  + x[dim-1] * y[dim-1]
     */
    static float64 dot(float64 *x, float64 *y, size_t dim);

    /**
     * Modify: \vec src += scalar * \vec v
     */
    static void addEquals(float64 *src, float64 scalar, float64 *v, size_t dim);
}

#endif

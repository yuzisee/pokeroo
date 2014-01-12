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

#include "matrixbase.h"

#include <iostream>
#include <math.h>

static void failIf(bool cond, const char *msg) {
    if (cond) {
        std::cerr << msg;
        exit(1);
    }
}



Matrix::Matrix(size_t rows, size_t columns, Arrangement arrangement)
:
fRows(rows)
,
fColumns(columns)
,
fArrangement(arrangement)
{
    fData = new float64[fRows * fColumns];
    for (int a=0; a<fRows*fColumns; ++a) {
        fData[a] = std::numeric_limits<float64>::signaling_NaN();
    }
}

Matrix::~Matrix() {
    delete [] fData;
}


Matrix::Matrix(const Matrix &matrix)
:
fRows(matrix.fRows)
,
fColumns(matrix.fColumns)
,
fArrangement(matrix.fArrangement)
{
    fData = new float64[fRows * fColumns];
    for (int a=0; a<fRows*fColumns; ++a) {
        fData[a] = matrix.fData[a];
    }
}

Matrix & Matrix::operator=(const Matrix &matrix)
{
    failIf (matrix.fColumns != fColumns, "Math error! Check your equations. Dimension mismatch during assignment\n");
    failIf (matrix.fRows != fRows, "Math error! Check your equations. Dimension mismatch during assignment\n");
    failIf (matrix.fArrangement != fArrangement, "Math error! Check your equations. Dimension mismatch during assignment\n");

    for (int a=0; a<fRows*fColumns; ++a) {
        fData[a] = matrix.fData[a];
    }

    return *this;
}

float64& Matrix::at(size_t row, size_t column) {
    failIf(fRows <= row || fColumns <= column, "index out of bounds\n");

    switch (fArrangement) {
        case ROW_MAJOR:
            return fData[row*fColumns + column];
        case COLUMN_MAJOR:
            return fData[column*fRows + row];
    }
}



struct SizedArray Matrix::getRow(size_t rownum) {
    failIf(fArrangement != ROW_MAJOR, "getRow only works on ROW_MAJOR matrices\n");
    failIf(fRows <= rownum, "index out of bounds\n");

    struct SizedArray result;
    result.data = fData + fColumns * rownum;
    result.size = fColumns;
    return result;
}

struct SizedArray Matrix::getColumn(size_t colnum) {
    failIf(fArrangement != COLUMN_MAJOR, "getCol only works on COLUMN_MAJOR matrices\n");
    failIf(fColumns <= colnum, "index out of bounds\n");

    struct SizedArray result;
    result.data = fData + fRows * colnum;
    result.size = fRows;
    return result;
}

void Matrix::setZero() {
    for (int a=0; a<fRows*fColumns; ++a) {
        fData[a] = 0.0;
    }
}

Matrix Matrix::newIdentityMatrix(size_t size, Arrangement arrangement) {
    Matrix result(size, size, arrangement);
    result.setZero();

    for (size_t k = 0; k<size; ++k) {
        result.at(k, k) = 1.0;
    }

    return result;
}

NormalizedSystemOfEquations::NormalizedSystemOfEquations(size_t variables)
:
fVariables(variables)
,
fEquations(variables - 1)
,
At(variables, fEquations, Matrix::Arrangement::COLUMN_MAJOR)
,
I(Matrix::newIdentityMatrix(variables, Matrix::Arrangement::COLUMN_MAJOR))
,
fSolved(false)
{
    failIf(variables <= 1, "Must be at least 2 variables.");
}

NormalizedSystemOfEquations::~NormalizedSystemOfEquations() {

}

void NormalizedSystemOfEquations::setEquation(size_t eqnNum, struct SizedArray coefficients, float64 value) {
    failIf(fSolved, "You already solved. It's too late to set equations");
    failIf(fEquations <= eqnNum, "eqn index out of bounds");
    failIf(coefficients.size != fVariables, "dimension mismatch in equation variable count");

    struct SizedArray dataToSet = At.getColumn(eqnNum);

    failIf(dataToSet.size != fVariables, "(This should be impossible) Internal error with matrix dimensions");

    // e.g.
    //  express
    //    0.5 x + 0.24 y - 0.1 z = 0.01;
    //  as
    //    [ (-0.01 + 0.5)  ,   (-0.01 + 0.24)  ,   (-0.01 - 0.1) ]
    for (size_t k=0; k<fVariables; ++k) {
        dataToSet.data[k] = -value + coefficients.data[k];
    }
}

/**
 * There is one Householder Transform Step for each element along the diagonal of the matrix to be triangularized.
 * The variables here are assigned once per step and define the step that is taking place.
 * v: The Householder reflection plane normal vector (unnormalized)
 * v2: The squared 2-norm of v, e.g. dot(v,v)
 * skipIdx: How many dimensions to skip? Usually the Householder reflection requires only the bottom N elements of the column.
 *          We will skip the first skipIdx elements, leaving only the bottom N
 *          Here, skipIdx + N === dest.fRows === "matrix height"
 */
struct HouseholderTransformStep {
    struct SizedArray v;
    float64 v2 = std::numeric_limits<float64>::signaling_NaN();
    size_t skipIdx = 0;
};

/**
 * dest: The matrix to modify by applying a Householder reflection to one of its columns
 * step: Which step of the Householder Transform are we on?
 * y_idx: Which column of ``dest`` will we modify by applying the Householder reflection?
 */
static void applyHouseholderReflectionImpl(Matrix &dest, const struct HouseholderTransformStep &step, size_t y_idx) {
    struct SizedArray y = dest.getColumn(y_idx);
    y.data += step.skipIdx;
    y.size -= step.skipIdx;

    failIf(step.v.size != y.size, "(Should be impossible, indicates logic error) Internal dimension mismatch");
    const float64 dot_v_y = ElementWise::dot(step.v.data, y.data, y.size);
    const float64 scalar = -2 * dot_v_y / step.v2;
    ElementWise::addEquals(y.data, scalar, step.v.data, y.size);
}

void NormalizedSystemOfEquations::solve() {
    if (!fSolved) {


        // For each column (e.g. each equation)
        for (size_t eqnIdx = 0; eqnIdx < fEquations; ++eqnIdx) {
            // Identify the column to operate on
            struct SizedArray mainColumn = At.getColumn(eqnIdx);
            // ... but we don't operate on the entire column, we will do a lower subset of that column (moving diagonally)
            mainColumn.data += eqnIdx;
            mainColumn.size -= eqnIdx;

            float64 diagonalResult = sqrt(ElementWise::dot(mainColumn.data, mainColumn.data, mainColumn.size));
            // The final value of this column will be: [diagonalResult 0 0 ... 0]^T

            // Store v into the mainColumn so we can use it
            //   v = mainColumn - elementaryBasis
            // i.e.
            //   v = mainColumn
            //   v[elementaryBasis_idx] -= diagonalResult
            mainColumn.data[0] -= diagonalResult;
            // At this point, mainColumn now stores v.

            // Compute the Householder reflection terms:
            //  y += -dot(v, y) / dot(v, v) * v
            const float64 dot_v_v = ElementWise::dot(mainColumn.data, mainColumn.data, mainColumn.size);

            if (dot_v_v > 0.0) {
                struct HouseholderTransformStep spec;
                spec.v = mainColumn;
                spec.v2 = dot_v_v;
                spec.skipIdx = eqnIdx;

                // Apply this reflection to each column of At (after mainColumn)...
                for (size_t y_idx = eqnIdx + 1; y_idx < fEquations; ++y_idx) {
                    applyHouseholderReflectionImpl(At, spec, y_idx);
                }

                // ... and eachcolumn of I
                for (size_t y_idx = 0; y_idx < fVariables; ++y_idx) {
                    applyHouseholderReflectionImpl(I, spec, y_idx);
                }


                // Finish the Householder reflection by applying it to the original mainColumn.
                // By definition, applying this Householder reflection will rotate the mainColumn onto the elementary basis vector (e.g. zero out all the elements below)
                for(size_t k=1; k<mainColumn.size; ++k) {
                    mainColumn.data[k] = 0.0;
                }
            }
            mainColumn.data[0] = diagonalResult;

        }


        fSolved = true;
    }


}

void NormalizedSystemOfEquations::getSolution(struct SizedArray *result) {
    failIf(result->size != fVariables, "Dimension mismatch. Null Space should have one element per variable.");

    // Return the bottom row of Q^T
    for(size_t k=0; k<fVariables; ++k) {
        result->data[k] = I.at(fVariables - 1, k);
    }
}


float64 ElementWise::dot(float64 *x, float64 *y, size_t dim)
{
    float64 result = 0.0;
    for (size_t k = 0; k<dim; ++k)
    {
        result += x[k] * y[k];
    }
    return result;
}

void ElementWise::addEquals(float64 *src, float64 scalar, float64 *v, size_t dim)
{
    for (size_t k = 0; k<dim; ++k)
    {
        src[k] += scalar * v[k];
    }
}



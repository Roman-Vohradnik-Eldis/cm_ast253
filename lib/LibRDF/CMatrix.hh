// ##### Description #####
// Module: CMatrix
// File: CMatrix.h
// Description: Library for matrices and matrix operations
// Version: 0.9.3
// Author: Jiri Tihelka, j.tihelka@seznam.cz
// Date: 01.10.2008
//-------------------------------------------------------------------
// Change history:
//   02.06.1993 - Matthew Miller
//     Created a base this matrix library (it was little bit rewritten for this
//     matrix library)
//   01.10.2008 - Jiri Tihelka, j.tihelka@seznam.cz.
//     The base of the old matix library little bit rewritten and new features
//     added.
//   15.07.2011 - JPl
//     Templates for matrix initiation. No further equirement to explicitly define matrix in a code.
//   30.09.2011 - JPl
//     Removed static variables to avoid troubles in multithreading
//   24.03.2017 - JPl
//     Unified coding style

//-------------------------------------------------------------------

#define CMATRIX_VERSION "0.9.6.20170424"

//-------------------------------------------------------------------
// LONG DESCRIPTION - REFERENCE MANUAL
/*
================================================================================
This is a matrix C++ library.

This library is intended for fast computation of the matrix operations with
matrices with elements of double type. The main goal is speed of calculation,
especially with small matrices.

================================================================================
A. Matrix constructors and destructor

The following constructors are available for the matrix class

1. CMatrix A(m,n);
   Constructs a matrix object of size m x n with all elements initialized to the
   value of 0, where m is number of rows of the matrix and n is number of
	 columns of the matrix.
   Example: CMatrix A(2,2);

2. CMatrix A(CMatrix B);
   Constructs a new matrix object by copying from an existing matrix object B.
   Example: CMatrix B(2,2); CMatrix A(B);

3. CMatrix A();
   Constructs a matrix object with zero element. This constructor is provided
   only for constructing array of matrix objects. The size should always be
   provided for single matrix object because zero size matrix object can't be
   used any matrix operations except assignment operator.
   Example: CMatrix A();

4. ~CMatrix();
   Destroys a matrix object.
   Example: ~A();


================================================================================
B. Accessing elements

1. CMatrix(i,j)
   Elements are accessed by expressions of the form A(i,j) where i and j run
	 from 1 to the appropriate dimension.
	 Example: A(2,1)

2. CMatrix(i)
   Matrices in form of a column vector can accept only one subscripts, it is
	 equivalent to A(i,0).
   Example: CMatrix A(5,1); A(1) = 0;  // equivalent to A(1,0) = 0.


================================================================================
C. Assignment and copying

1. CMatrix = CMatrix
   Assigns the matrix to the left-hand side matrix. The sizes of the matrices
	 must be equal.
   Example: B = A;

2. CMatrix << CMatrix
   Assigns the matrix to the left-hand side matrix. The sizes of the matrices
	 need not be equal. The dimensions of the matrix on the left-hand side are
	 adjusted to those of the matrix or expression on the right-hand side.
   Example: B << A;
            B << A * C;

3. void CMatrix.swap(CMatrix)
   void swap(CMatrix, CMatrix)
   Swaps values of two matrix objects.
   Example: A.swap(B);
            swap(A, B);


================================================================================
D. Entering values

The following possibilities are available for entering of values.

1. CMatrix = double
   Set all the elements in the matrix to a double.
   Example: A = 0.0;

2. void CMatrix.set(double1, double2, ...)
   Set all the values in the matrix. The number of arguments must be exactly
	 equal to the size of the matrix (nrows x ncols). If there are too many
	 arguments, the extras will be ignored. If there are too few, the last few
	 values in the matrix will be filled with garbage. The entries are filled
	 in, starting with row 0, column 0, and proceeding in row-major order. So
	 double1 goes into the entry at row 0, column 0; double2 goes into row 0,
	 column1; etc.
   WARNING: this function is not type-safe.  All the arguments must be doubles.
	 No automatic type-casting or error messages will result from a mistake
	 -- only a bug.
   Example: A.set(1., 2., 3., 4.);

3. void CMatrix.vset(double1, va_list)
   This is the same as set() except that it is given a va_list argument
	 (see stdarg.h). The value to be placed in row 0, column 0 must be given
	 explicitly.
   Example: A.vset(1., vl);

4. CMatrix << *double
   Loads the elements of a matrix from an array. This construction does not
	 check that the numbers of elements match correctly. If there are too many
	 elements in the array, the extras will be ignored. If there are too few,
	 the last few values in the matrix will be filled with garbage. The entries
	 are filled in in row-major order, see set() for details.
   Example: CMatrix A(2,3);
            double a[] = {1., 2., 3., 4., 5., 6.};
            A << a;

5. CMatrix << double1 << double2 ...]
   Loads the elements of a matrix from list using a sequence of numbers
	 separated by <<. It is a stream input operator. This does check for the
	 correct total number of entries, although the message for there being
	 insufficient numbers in the list may be delayed until the end of the block
	 or the next use of this construction. The entries are filled in in row-major
	 order, see set() for details.
   Example: A << 1. << 2. << 3. << 4.;

6. void CMatrix.zero()
   Sets all elements of a matrix equal to zero.
   Example: A.zero();

7. void CMatrix.zero(m, n)
   Resizes the matrix to m x n matrix and sets all elements of a matrix equal
	 to zero.
   Example: A.zero(3,2);

8. void CMatrix.identity()
   Sets the matrix as a identity (unit) matrix, i.e., its main diagonal
	 elements will be equal to 1.0 and the rest zero.
   Example: A.identity();

9. CMatrix.identity(m)
   Resizes the matrix to m x m matrix and sets the matrix as a identity (unit)
	 matrix, i.e., its main diagonal elements will be equal to 1.0 and the rest
	 zero.
   Example: A.identity(m);


================================================================================
E. Unary operations

The following unary operations are supported.

1. +CMatrix
   Returns the same matrix.
   Example: B = +A;

2. -CMatrix
   Returns a matrix with changed sign of all the elements.
   Example: B = -A;

3. CMatrix CMatrix.t()
   ~CMatrix
   Returns the transpose of a square matrix.
   Example: B = A.t();
            B = ~A;

4. CMatrix CMatrix.i()
   !CMatrix
   Returns the inverse of a square matrix. The matrix is inverted using LU
	 decomposition method.
   Example: B = A.i();
            B = !A;

5. CMatrix CMatrix.reverse()
   Returns a matrix with reversed order of the elements of matrix.
   Example: B = A.reverse();

6. CMatrix CMatrix.sum_rows()
   Returns a matrix (a column vector) with sum of elements of each row.
   Example: B = A.sum_rows();

7. CMatrix CMatrix.sum_columns()
   Returns a matrix (a row vector) with sum of elements of each column.
   Example: B = A.sum_rows();

8. CMatrix CMatrix.sum_square_rows()
   Returns a matrix (a column vector) with sum of squares of elements of each
	 row.
   Example: B = A.sum_square_rows();

9. CMatrix CMatrix.sum_square_columns()
   Returns a matrix (a row vector) with sum of squares of elements of each
	 column.
   Example: B = A.sum_square_columns();

10. CMatrix CMatrix.abs()
    CMatrix abs(CMatrix)
    Returns a matrix with absolute values of the elements of matrix.
    Example: B = A.abs();

11. CMatrix CMatrix.pow2()
    CMatrix pow2(CMatrix)
    Returns a result of powering each element of the matrix to the power of 2,
		i.e. all the elements are squared.
    Example: B = A.pow2();

================================================================================
F. Binary operations

The following binary operations are supported.

1. CMatrix + CMatrix
   Returns the result of adding two matrices.
   Example: C = A + B;

2. CMatrix - CMatrix
   Returns the result of subtracting the second matrix from the first.
   Example: C = A - B;

3. CMatrix * CMatrix
   Returns the result of matrix multiplication.
   Example: C = A * B;

4. CMatrix | CMatrix
   Returns a result of concatenation of two matrices horizontally
	 (concatenate the rows).
   Example: C = A | B;

5. CMatrix & CMatrix
   Returns a result of concatenation of two matrices vertically
	 (concatenate the columns).
   Example: C = A & B;

6. CMatrix += CMatrix
   Adds a matrix to the left-hand side matrix. The matrices must be equal in
	 row and column size.
   Example: A += B;    // A = A + B

7. CMatrix -= CMatrix
   Subtracts a matrix from the left-hand side matrix. The matrices must be equal
	 in row and column size.
   Example: A -= B;    // A = A - B

8. CMatrix *= CMatrix
   Multiplies a matrix to the left-hand side matrix. The column number of
	 left-hand side matrix must be equal to row number of right-hand side matrix.
   Example: A *= B;    // A = A * B
   Note: Resulting matrix is resized if a result of the matrix multiplication
   has different size as the matrix itself.
   E.g.: Given matrix A of size 2x3 and matrix B of size 3x2:
         A *= B; creates matrix A of size 2x2 equal to A*B

9. CMatrix |= CMatrix
   Concatenates two matrices horizontally (concatenate the rows).
   Example: A |= B;    // A = A | B

10. CMatrix &= CMatrix
    Concatenates two matrices vertically (concatenate the columns).
    Example: A &= B;    // A = A & B

11. CMatrix CMatrix.array_multiply(CMatrix)
    CMatrix array_multiply(CMatrix, CMatrix)
    Returns the result of array multiplication of two matrices. Array
		multiplication is element by element multiplication.
    Example: C = A.array_multiply(B);   // C = A.*B
             C = array_multiply(A, B);  // C = A.*B

12. CMatrix CMatrix.pow(double)
    Returns a result of powering each element of the matrix to the power of
		a double.
    Example: B = A.pow(3);       // B = A.^3

13. CMatrix CMatrix.solve(CMatrix)
    CMatrix solve(CMatrix, CMatrix)
    Solves a set of linear equations, Ax=b, using LU decomposition method.
		The right hand side vector is the class method argument or the second
		argument of the function. The matrix A must be square and regular.
    Example: x = A.solve(b);
             x = solve(A, b);


================================================================================
G. Boolean operations

Boolean operators
1. CMatrix == CMatrix
   Compares the matrices, and returns true if they are equal; otherwise, it
	 returns false.
   Example: if (A == B) { }

2. CMatrix != CMatrix
   Compares the matrices, and returns true if they are not equal; otherwise,
	 it returns false.
   Example: if (A != B) { }


Boolean methods - Type of matrices
1. bool is_matrix()
   Returns true if the matrix is a valid matrix (both number of rows and
	 columns are positive); otherwise, it returns false.
   Example: if (A.is_matrix()) { }

2. bool is_square()
   Returns true if the matrix is a square matrix; otherwise, it returns false.
   Example: if (A.is_square()) {}

3. bool is_row()
   Returns true if the matrix is a row vector; otherwise, it returns false.
   Example: if (A.is_row()) {}

4. bool is_column()
   Returns true if the matrix is a column vector; otherwise, it returns false.
   Example: if (A.is_column()) {}

5. bool is_zero()
   Returns true if the matrix is a zero (null) matrix; otherwise, it returns
	 false.
   Example: if (A.is_zero()) {}

6. bool is_singular()
   Returns true if the matrix is singular; otherwise, it returns false.
   Example: if (A.is_singular()) {}

7. bool is_diagonal()
   Returns true if the matrix is diagonal; otherwise, it returns false.
   Example: if (A.is_diagonal()) {}

8. bool is_scalar()
   Returns true if the matrix is a scalar matrix; otherwise, it returns false.
   Example: if (A.is_scalar()) {}

9. bool is_identity()
   Returns true if the matrix is an identity (unit) matrix; otherwise, it
	 returns false.
   Example: if (A.is_identity()) {}

10. bool is_symmetric()
    Returns true if the matrix is symmetric; otherwise, it returns false.
    Example: if (A.is_symmetric()) {}

11. bool is_antisymmetric()
    Returns true if the matrix is antisymmetric (skew-symmetric); otherwise,
		it returns false.
    Example: if (A.is_antisymmetric()) {}

12. bool is_upper_triangular()
    Returns true if the matrix is upper triangular; otherwise, it returns false.
    Example: if (A.is_upper_triangular()) {}

13. bool is_lower_triangular()
    Returns true if the matrix is lower triangular; otherwise, it returns false.
    Example: if (A.is_lower_triangular()) {}


================================================================================
H. Matrix and scalar

1. CMatrix + double
   double - CMatrix
   Returns the result of adding a double to each element of the matrix.
   Example: B = A + d; B = d + A;

2. CMatrix - double
   Returns the result of subtracting a double from each element of the matrix.
   Example: B = A - d;

3. double - CMatrix
   Returns the result of subtracting each element of the matrix from a double.
   Example: B = d - A;

4. CMatrix * double
   double * CMatrix
   Returns the result of multiplying each element of the matrix by a double.
   Example: B = d * A; B = A * d;

5. CMatrix / double
   Returns the result of dividing each element of the matrix by a double.
   Example: B = A / d;

6. CMatrix += double
   Adds a double to all elements of the left-hand side matrix.
   Example: A =+ d;

7. CMatrix -= double
   Subtracts a double to all elements of the left-hand side matrix.
   Example: A =- d;

8. CMatrix *= double
   Multiplies all elements of the left-hand side matrix by a double.
   Example: A *= d;

9. CMatrix /= double
   Divides all elements of the left-hand side matrix by a double.
   Example: A /= d;


================================================================================
I. Scalar functions of a matrix - size & shape

1. int CMatrix.nrows()
   Return the number of rows of a matrix.
   Example: m = A.nrows();

2. int CMatrix.ncols()
   Return the number of columns of a matrix.
   Example: n = A.ncols();

3. int CMatrix.size()
   Returns the number of elements of a matrix object.
   Example: mn = A.size();

4. int CMatrix.size_bytes()
   Returns the number of bytes of all elements of a matrix object.
   Example: num_bytes = A.size_bytes();

5. int CMatrix.type_size()
   Returns the type size in bytes of the matrix elements.
   Example: ts = A.type_size();

5. double * CMatrix.data()
   Return a pointer to the element of the matrix at row 0, column 0.
	 The remaining elements are stored after it in row-major order.
   Example: m = A.size()
   WARNING: under some circumstances, using the matrix on the left-hand-side
	 of an "=" can change the address of its data. So something like
   p = m0.data();
   m0 = m1 * m2;
   *p = 0.;
   is a bad idea.


================================================================================
J. Scalar functions of a matrix - numerical

1. double min(CMatrix)
   double CMatrix.min()
   Returns the minimum value of all matrix elements.
   Example: s = A.min();
   Note: For minimum of absolute values use expression min(abs(A)) or min(A.abs()

2. double max(CMatrix)
   double CMatrix.max()
   Returns the maximum value of all matrix elements.
   Example: s = A.max();
   Note: For maximum of absolute values use expression max(abs(A)) or max(A.abs()

3. double CMatrix.sum()
   Returns the sum of all elements of a matrix.
   Example: s = A.sum();

4. double CMatrix.sum_square()
   Returns the sum squares of all elements of a matrix.
   Example: s = A.sum_square();

5. double CMatrix.trace()
   Returns the sum of main diagonal elements of a matrix.
   Example: s = A.trace();

7. double CMatrix.norm1()
   Returns the one norm of a matrix, i.e. maximum of sum of absolute values
	 of elements of a column.
   Example: d = A.norm1();

8. double CMatrix.norm_inf()
   Returns the infinity norm of a matrix, i.e. for matrix: maximum of sum of
	 absolute values of elements of a row, for vector: maximum of absolute values
	 of elements.
   Example: d = A.norm_inf();

9. double CMatrix.norm_frob()
   Returns the Frobenius norm of a matrix, i.e. square root of sum of squares.
   Example: d = A.norm_frob();

10. double CMatrix.norm(double)
    Returns the L_p norm of a vector, i.e. sum(abs(A).^p)^(1/p),
		for any 1 <= p <= inf.
    Example: d = A.norm(3);

11. double CMatrix.norm()
    Returns the L_2 norm of a vector, i.e. sqrt(sum(abs(A).^2)).
    Example: d = A.norm();

12. double CMatrix.det()
    Returns the determinant of a matrix.
    Example: d = A.det();

13. CLogAndSign CMatrix.log_det()
    Returns the natural log of the determinant of a matrix. It returns a value
		of type CLogAndSign.
    Example: ld = A.log_det();
    If ld is of type CLogAndSign use
    ld.value()     to get the value of the determinant
    ld.sign()      to get the sign of the determinant (values 1, 0, -1)
    ld.log_value() to get the log of the absolute value.
    Note that the direct use of the function det() will often cause a floating
		point overflow exception.

14. double dot_product(CMatrix, CMatrix)
    Returns the dot product of two matrices interpreted as vectors.
    Example: d = dot_product(A, B);


================================================================================
K. Submatrices

1. CMatrix CMatrix.submatrix(fr, lr, fc, lc)
   Returns a submatrix from a matrix. The arguments fr, lr, fc, lc of type int
	 are the first row, last row, first column, last column of the submatrix with
	 the numbering beginning at 0.
   Example: B = A.submatrix(1,2,0,3);

2. CMatrix CMatrix.row(r)
   Returns the r-th row of a matrix. The row index starts from zero.
   Example: B = A.row(2);

3. CMatrix CMatrix.column(c)
   Returns the c-th column of a matrix. The column index starts from zero.
   Example: B = A.column(0);

4. CMatrix CMatrix.diag()
   Returns the diagonal matrix of a matrix. The diagonal matrix has the same
	 size as the original one and contains only values at the main diagonal.
   Example: B = A.diag();

5. void CMatrix.assign_submatrix(fr, lr, fc, lc, CMatrix)
   Assignes a submatrix into a matrix. The arguments fr, lr, fc, lc of type int
	 are the first row, last row, first column, last column of the submatrix with
	 the numbering beginning at 0. The sizes of the given submatrix and the matrix
	 given bt the arguments fr, lr, fc, lc must correspond and the submatrix must
	 be inside the matrix.
   Example: A.assign_submatrix(0, 1, 1, 2, B);

6. void CMatrix.assign_submatrix(r, c, CMatrix)
   Assignes a submatrix into a matrix. The arguments r and c of type int are
	 the first row and the first column of the submatrix with the numbering
	 beginning at 0. The submatrix must be inside the matrix.
   Example: A.assign_submatrix(0, 1, B);

7. void CMatrix.assign_row(r, CMatrix)
   Assignes a row into a matrix at r-th row. The row index starts from zero.
	 The assigned row must be inside the matrix.
   Example: A.assign_column(0, B);

8. void CMatrix.assign_column(c, CMatrix)
   Assignes a column into a matrix at c-th column. The column index starts from
	 zero. The assigned column must be inside the matrix.
   Example: A.assign_row(0, B);


================================================================================
L. Change dimensions

The following operations change the dimensions of a matrix.

1. CMatrix CMatrix.reduce(m,n)
   Return the portion of the matrix of size m x n, that contains the element
	 at 0,0. The both number of rows and columns must be less than number of rows
	 and columns of the original matrix.
   Example: B = A.reduce(2,1);

2. void CMatrix.resize(m,n)
   Resize the matrix to size m x n, that contains the element at 0,0.
   The values of the elements are keeped for those that are in the matrix with
	 the old size and zeroed for those that are not in the matrix with the old
	 size but exist in the matrix with the new size.
   Example: B = A.resize(2,1);


================================================================================
M. Change type
The following functions interpret the elements of a matrix (stored row by row)
to be a vector or matrix of a different type.

1. CMatrix CMatrix.as_row()
   Returns a matrix (row vector) with elements interpreted as a row vector. For
	 a matrix of size m x n, a row vector of size m*n x 1 is returned.
   Example: B = A.as_row();

2. CMatrix CMatrix.as_column()
   Returns a matrix (column vector) with elements interpreted as a column
	 vector. For a matrix of size m x n, a column vector of size 1 x m*n is
	 returned.
   Example: B = A.as_column();

3. CMatrix CMatrix.as_diagonal()
   Returns a matrix with elements interpreted as a diagonal matrix.
	 For a matrix of size m x n, a diagonal matrix of size m*n x m*n is returned.
   Example: B = A.as_diagonal();

4. CMatrix CMatrix.as_matrix(m,n)
   Returns a matrix with elements interpreted as a matrix of size m x n.
	 For a matrix of size M x N, a matrix of size m x n is returned. The requested
	 matrix must have the same size as the original one, i.e. M*N == m*n.
   Example: B = A.as_matrix(4,2);

5. CMatrix CMatrix.as_scalar()
   Returns a double with value of the element at 0,0 of a matrix. The matrix
	 size must be 1 x 1.
   Example: B = A.as_scalar();
   Note: The expression A.as_scalar() is used to convert a 1 x 1 matrix to
	 a scalar.


================================================================================
N. Output matrices

For formatting of the matrix output to a output stream, the following settings
can be adjusted. They can be found in the file CMatrix.h. This setting has no
influence to global format of the output stream - it has influence only upon
matrices.
a. #define MATRIX_OS_PRECISION 6
   Number of decimal points - must be specified.
b. #define MATRIX_OS_NUM_SEP   2
   Minimal number of spaces between the matrix columns - must be specified.
c. #define MATRIX_OS_SCIENTIFIC
   Scientific mode of the output, e.g. 1.11e-2 - compiler switch - it can be
	 defined or undefined.
d. #define MATRIX_OS_SHOW_POINT
   Shows decimal point and all digits in the numbers, e.g. 11.000 (not 11)
	 - compiler switch - it can be defined or undefined.
e. #define MATRIX_OS_NEW_LINE
   Adds a new line before the matrix - compiler switch - it can be defined or
	 undefined.
f. #define MATRIX_OS_MATRIX_INDENT 2
   Number of spaces for the matrix indent - must be specified.

1. void CMatrix.print()
   Prints out a matrix to the output stream cout.
   Example: A.print();

2. ostream << CMatrix
   Prints out a matrix to a output stream like cout or a file.
   Example: cout << A << endl;


================================================================================
O. Library version

1. string CMatrix.version()
   Returns a string with the version of the matrix library.
   Example: ver = A.version()


================================================================================
P. Exceptions

The exceptions are supported by the library if the compiler switch
#define MATRIX_EXCEPTIONS
is enabled.
If this compiler switch is not enabled and the library detects an error it
will write appropriate error message with a type of the error to standard
output and call the function exit(). If the exceptions are enabled, it is
important that you catch this exception and print the error message. Otherwise
you will get an unhelpful message like abnormal termination.

Example:
#define Try             try
#define CatchException  catch(exception & e) {                               \
    cout << "(" << __FILE__ << ", " << __LINE__ << ") " << e.what() << endl; \
  }
#define CatchAll        catch(...)

{
  Try {
  {
    ... your code here
  }
  // catch exceptions thrown by my programs
  CatchException;
  // catch exceptions thrown by other people's programs
  CatchAll { cout << "exception caught in main program" << endl; }
}


================================================================================
Q. Implementation notes:

The implementation of matrices in CMatrix.h and CMatrix.c is a bit
unintuitive. Instead of having a single, straightforward CMatrix class, we
have two classes: CMatrix and CtmpMatrix. A CMatrix is created and used in the
ways that one might expect. A CtmpMatrix cannot be created directly. It can
only be created as the result of some operation performed on a CMatrix.

The difference between a CMatrix and a CtmpMatrix is that, when you perform an
operation on a CtmpMatrix, creating another CtmpMatrix, the new CtmpMatrix
might use the same memory as the old CtmpMatrix. In other words, operations on
CtmpMatrix's are destructive. This significantly reduces the number of memory
allocations and deallocations required.

This CtmpMatrix trick works because CtmpMatrix's are only ever constructed as
temporary objects. A temporary object can only ever have one operation
performed on it before it is destroyed (if the program is written properly),
so why not destroy it during that operation?

Here's an example of how CtmpMatrix's work. Suppose M0, M1, M2, and M3 are all
CMatrix's. Consider the following line of code:

  M3 = M0 * M1 + M2

Three operations are performed here:

  1. M0 is multiplied by M1, call the result T0
  2. M2 is added to T0, call the result T1
  3. T1 is copied into M3

In a normal implementation, in which T0 and T1 are just normal CMatrix's, this
would require two allocations and two deallocations of matrix arrays, plus a
memory copy to perform the assignment.

But here, T0 and T1 are CtmpMatrix's. The first step, which creates T0, still
requires the allocation of a new matrix array. But in the second step, the
addition is performed destructively on the array allocated for T0, and that
memory is "stolen" from T0 by T1. In the third step, M3's array is simply
deallocated, and T1's array is stolen by M3. Thus only one new array is
allocated, and no copying of memory is required.


================================================================================
R. Adoption note:

The base for this library was library called "matrix" by Matthew Miller from
2.6.1993 with the main type MATRIX. The main mechanism for fast matrix
calculation, the matrix class, the temporary matrix class and basic matrix
operations, were adopted from the "matrix" library. The exception handling and
input and output part were rewritten to support extended possibilities. New
features (many new matrix operators and methods) have been added to this core.
Many inspirations for the library were found in the Newmat C++ library by
Robert Davies, http://www.robertnz.net/. A detailed list is above. The
original documentation was rewritten, but the implementation notes are preserved.

The matrix library by Matthew Miller have had the following copyright:
 Copyright (c) 1993, NEC Research Institute. All Rights Reserved.

 Permission to use, copy, and modify this software and its documentation is
 hereby granted only under the following terms and conditions. Both the above
 copyright notice and this permission notice must appear in all copies of the
 software, derivative works or modified versions, and any portions thereof, and
 both notices must appear in supporting documentation.

 Correspondence should be directed to NEC at:
 Ingemar J. Cox
 NEC Research Institute
 4 Independence Way
 Princeton
 NJ 08540
 phone:  609 951 2722
 fax:  609 951 2482
 email:  ingemar@research.nj.nec.com (Inet)
================================================================================
*/


//-------------------------------------------------------------------
// ##### Includes #####
#ifndef CMATRIX_H_
#define CMATRIX_H_

#include <stdarg.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <stdlib.h>

// ##### Includes end #####


//-------------------------------------------------------------------
// ##### Definitions #####

// format of matrix output to the output stream, it has no influence to global format of the output stream - it has influence only upon matrices
#define MATRIX_OS_PRECISION 6     // number of decimal points
#define MATRIX_OS_NUM_SEP   2     // minimal number of spaces between the matrix columns

//#define MATRIX_OS_SCIENTIFIC    // scientific mode of the output, e.g. 1.11e-2
//#define MATRIX_OS_SHOW_POINT    // shows decimal point and all digits in the numbers, e.g. 11.000 (not 11)
#define MATRIX_OS_NEW_LINE        // adds a new line before the matrix
#define MATRIX_OS_MATRIX_INDENT 2 // number of spaces for the matrix indent

#define MATRIX_EXCEPTIONS         // if defined, exceptions are used
// ##### Definitions end #####

//-------------------------------------------------------------------

using namespace std;

//-------------------------------------------------------------------
// ##### Exceptions #####

//-------------------------------------------------------------------
#ifdef MATRIX_EXCEPTIONS
class CMatrixError :
  public logic_error
{
  public: CMatrixError(const string &WhatArg) :
    logic_error(WhatArg)
    {
    }
};

#define MATRIX_ASSERT(condition, error_msg) \
  if (condition) {                          \
    MATRIX_THROW(error_msg);                \
  }


/*
//#define MATRIX_THROW(error_msg)               { \
//    ostringstream strstr; \
//    strstr << error_msg; \
//    string message = strstr.str(); \
//    delete &strstr.str(); \
//    throw matrix_error(message); \
//  }
*/

#define MATRIX_THROW(error_msg) {     \
    ostringstream strstr;             \
    strstr << error_msg;              \
    throw CMatrixError(strstr.str()); \
  }

// snad tam neni memory leak!!!

#else // MATRIX_EXCEPTIONS
//-------------------------------------------------------------------

#define MATRIX_ASSERT(condition, error_msg) \
  if (condition) {                          \
    MATRIX_REPORT_ERROR(error_msg);                \
  }

#define MATRIX_REPORT_ERROR(error_msg) {     \
    ostringstream strstr;             \
    strstr << error_msg;              \
    matrix_error_(strstr.str()); \
  }
//#define MATRIX_REPORT_ERROR(ErrormMsg)  matrix_error_(ErrormMsg);

inline void matrix_error_(const string &ErrorMessage)
{
  cout << "CMatrix error: " << ErrorMessage << endl;
  cout << "CMatrix exit call..." << endl;
  exit(1);
}

//#define MATRIX_ASSERT(X, Y)       // nothing
//#define MATRIX_THROW(X)           // nothing
#endif // MATRIX_EXCEPTIONS

// ##### Exceptions end #####

//-------------------------------------------------------------------
// ##### Declarations #####
class CMatrix;
class CtmpMatrix;
class CMatrixInput;
// ##### Declarations end #####


//-------------------------------------------------------------------
// CMatrixInput - Class for reading values into a (small) matrix within a program.
// It is able to detect a mismatch in the number of elements.
class CMatrixInput
{
    int n;      // number values still to be read
    double *r;  // pointer to next location to be read to
  public:
    CMatrixInput(const CMatrixInput &mi) :
    n(mi.n),
    r(mi.r)
    {
    }

    CMatrixInput(int nx, double *rx) :
    n(nx),
    r(rx)
    {
    }

    ~CMatrixInput(void);
    CMatrixInput operator << (double);
    friend class CMatrix;
};


//-------------------------------------------------------------------
// CLogAndSign - Class for return from log_det function.
// Members are the log of the absolute value and the sign (+1, -1 or 0)
class CLogAndSign
{
    double log_val;
    int sign_val;
public:
    CLogAndSign()
    {
      log_val = 0.0;
      sign_val = 1;
    }
    CLogAndSign(double);                         // initialization by a double
    void operator *= (double);                   // multiply by a double
    void change_sign() { sign_val = -sign_val; } // change sign
    double log_value() const { return log_val; } // returns log of the absolute value
    int sign() const { return sign_val; }        // returns sign of the value
    double value() const;                        // returns the value
};

//-------------------------------------------------------------------
// ##### CMatrix class #####

// CMatrix - Basic matrix class
class CMatrix
{
  protected:

    int m_nrows;
    int m_ncols;
    int m_size;
    double *m_data;

    // makes a CMatrix using a matrix array that's already been allocated
    CMatrix(int nrows, int ncols, double *data) :
    m_nrows(nrows),
    m_ncols(ncols),
    m_size(nrows * ncols),
    m_data(data)
    {
    }

  public:

    CMatrix(int nrows = 0, int ncols = 0) :
    m_nrows(nrows),
    m_ncols(ncols),
    m_size(nrows * ncols),
    m_data(new double[nrows * ncols])
    {
      if (m_data != 0) {
        memset(m_data, 0, m_size * sizeof(*m_data));
      }

      MATRIX_ASSERT(((0 > nrows) || (0 > ncols)),
                    "CMatrix::constructor(r,c): " << "Can't correctly create " <<
                    m_nrows << "x" << m_ncols <<
                    " matrix due to negative size. A corrupted matrix is still available.");
    }

    CMatrix(const CMatrix &src) :
    m_nrows(src.m_nrows),
    m_ncols(src.m_ncols),
    m_size(src.m_size),
    m_data(new double[src.m_nrows * src.m_ncols])
    {
      memcpy(m_data, src.m_data, m_size * sizeof(*m_data));
    }

    inline CMatrix(const CtmpMatrix &src);

    virtual~CMatrix(void)
    {
      if (m_data != 0) {
        delete[] m_data;
      }

      m_data = 0;
    }

    // assignement operators
    CMatrix &operator = (double val);
    CMatrix &operator = (const CMatrix &src);
    inline CMatrix &operator = (const CtmpMatrix &src);
    CMatrix &operator << (const CMatrix &src);
    inline CMatrix &operator << (const CtmpMatrix &src);

    // element access
    double &operator () (int row = 0, int col = 0)const;  // returns a value for CMatrix(row, col)

    // entring values
    void operator << (const double *);      // assignment of an array of numbers
    CMatrixInput operator << (double);      // loads the elements of a matrix from list
                                            // using a sequence of numbers separated by <<
    void vset(double firstVal, va_list ap); // fills a matrix with an array of values
    void set(double firstVal, ...);         // fills a matrix with an array of values

    // size and shape methods
    int nrows(void) const { return m_nrows; } // number of rows
    int ncols(void) const { return m_ncols; } // number of columns
    int size(void) const { return m_size; }   // number of stored elements
    int type_size(void) const { return (sizeof(*m_data)); } // type size of elements
    int size_bytes(void) const { return (m_size * sizeof(*m_data)); } // number of bytes of all elements
    double *data(void) const { return m_data; } // pointer to data, i.e. the element
                                                // of the matrix at row 0, column 0

    // submatrices
    CtmpMatrix column(int c) const;   // selects a column of a matrix
    CtmpMatrix row(int r) const;      // selects a row of a matrix
    CtmpMatrix submatrix(int fr, int lr, int fc, int lc) const; // selects a submatrix of a matrix
    CtmpMatrix diag() const;          // selects a submatrix of a matrix

    void assign_column(int col, const CMatrix &C);  // assignes a column to selected one in a matrix
    void assign_row(int row, const CMatrix &R);     // assignes a row to selected one in a matrix
    void assign_submatrix(int row, int col, const CMatrix &M);  // assignes a submatrix to selected one in a matrix
    void assign_submatrix(int fr, int lr, int fc, int lc, const CMatrix &M);  // assignes a submatrix to selected one in a matrix

    // change dimensions
    CtmpMatrix reduce(int nrows, int ncols);  // reduces a matrix to a given number of rows and columns
    void resize(int nrows, int ncols);        // resizes a matrix to a given number of rows and columns

    void zero(void);            // sets zero to all elements of this matrix
    void zero(int nrows, int ncols);  // resizes the matrix to nrows x ncols matrix and sets zero to all elements of this matrix
    void identity(void);        // sets the matrix to the identity matrix
    void identity(int nrows);   // resizes the matrix to nrows x nrows matrix and sets it to the identity matrix

    void swap(const CMatrix &M);      // swaps values of two matrix objects
    CtmpMatrix reverse(void) const;   // reverses order of the elements of matrix

    // change type
    CtmpMatrix as_row(void) const;    // interprets matrix body as a single row
    CtmpMatrix as_column(void) const; // interprets matrix body as a single column
    CtmpMatrix as_diag(void) const;   // interprets matrix body as a diagonal matrix
    CtmpMatrix as_matrix(int nrows, int ncols); // interprets matrix body as a matrix given size
    double as_scalar(void);           // interprets matrix body of matrix of size 1x1 as a scalar,
                                      // i.e. converts matrix of size 1x1 to a double

    CtmpMatrix t(void) const;     // matrix transpose
    CtmpMatrix i(void) const;     // matrix inversion

    CtmpMatrix array_multiply(const CMatrix &m) const;  // array multiplication of two matrices
    CtmpMatrix abs(void) const;       // absolute values of the elements of matrix
    CtmpMatrix pow(double p) const;   // powers each element of a matrix to the power of a double.
    CtmpMatrix pow2(void) const;      // powers each element of a matrix to the power of two,
                                      // i.e. squares all the elements

    double det(void) const;           // determinant of a matrix
    CLogAndSign log_det(void) const;  // natural logarithm of the determinant of a matrix

    double sum(void) const;           // sum of all elements
    double sum_square(void) const;    // sum of squares of all elements
    CtmpMatrix sum_rows(void) const;  	  // sum elements in each row
    CtmpMatrix sum_columns(void) const;   // sum elements in each column
    CtmpMatrix sum_square_rows(void) const;     // sum squares of elements in each row
    CtmpMatrix sum_square_columns(void) const;  // sum squares of elements in each column

    double min(void) const;           // minimum value of elements
    double max(void) const;           // maximum value of elements
    double trace(void) const;         // trace of a matrix, i.e. the sum of the diagonal elements

    double norm1(void) const;      // maximum of sum of absolute values of elements of a column
    double norm_inf(void) const;   // for matrix: maximum of sum of absolute values of elements of a row,
                                   // for vector: maximum of absolute values of elements
    double norm_frob(void) const;  // square root of sum of squares

    double norm(double p = 2.) const; // L_p norm of vector: sum(abs(A).^p)^(1/p), for any 1 <= p <= inf

    CtmpMatrix solve(const CMatrix &b) const; // solves A*x = b via LU decomposition

    void print(void) const; // prints out a matrix to the output stream cout
    friend std::ostream & operator <<(std::ostream & os, const CMatrix & A); // prints out a matrix to a output stream like cout or a file

    void operator += (double f);      // adds a double
    void operator -= (double f) { operator += (-f); }  // subtracts a double
    void operator *= (double f);      // multiplies by a double
    void operator /= (double f) { operator *= (1.0 / f); } // divides by a double

    void operator += (const CMatrix &A);  // adds a matrix
    void operator -= (const CMatrix &A);  // subtracts a matrix
    void operator *= (const CMatrix &A);  // multiplies by a matrix

    void operator |= (const CMatrix &A);  // concatenates horizontally (concatenates the rows)
    void operator &= (const CMatrix &A);  // concatenates vertically (concatenates the columns)

    inline CMatrix operator + (void) {    // unary plus operator
      return *this; // do nothing
    }

    CMatrix operator - (void);            // unary negation operator

    // Type of matrices
    bool is_matrix(void) const { return ((m_nrows > 0) && (m_ncols > 0)); };
    bool is_square(void) const { return (is_matrix() && (m_nrows == m_ncols)); };
    bool is_row (void) const { return (is_matrix() && (m_nrows == 1)); };
    bool is_column(void) const { return (is_matrix() && (m_ncols == 1)); };
    bool is_vector(void) const { return (is_matrix() && ((m_ncols == 1) || (m_nrows == 1)) ); };
    bool is_zero(void) const;
    bool is_singular(void) const;
    bool is_diagonal(void) const;
    bool is_scalar(void) const;
    bool is_identity(void) const;
    bool is_symmetric(void) const;
    bool is_antisymmetric(void) const;
    bool is_upper_triangular(void) const;
    bool is_lower_triangular(void) const;

    const string version(void);  // matrix library version
};
// ##### CMatrix class end #####


//-------------------------------------------------------------------
// ##### CtmpMatrix class #####

// CtmpMatrix - Temporary matrix class
// This class is not intended for use outside of the matrix package
class CtmpMatrix :
  public CMatrix
{
    friend class CMatrix;

  public:

    CtmpMatrix(const CMatrix &m0, const CMatrix &m1);  // m0 * m1

    CtmpMatrix(const CMatrix &src) : CMatrix(src) {}

    CtmpMatrix(const CtmpMatrix &src) : CMatrix(src) {} // copy constructor

    CtmpMatrix(int nrows, int ncols) : CMatrix(nrows, ncols) {}

    CtmpMatrix(int nrows, int ncols, double *data) : CMatrix(nrows, ncols, data) {}

  private:

    CtmpMatrix &add(const CMatrix &src);
    CtmpMatrix &add(double num);
    CtmpMatrix &subtract(const CMatrix &src);
    CtmpMatrix &subtract_from(const CMatrix &src);
    CtmpMatrix &multiply(double num);
    CtmpMatrix &negate(void);
    CtmpMatrix &array_multiply(const CMatrix &src);

    CtmpMatrix &concat_rows(const CMatrix &src);
    CtmpMatrix &concat_columns(const CMatrix &src);

    double *steal_data(void) const;     /* not really const */

  public:

    // element access
    double &operator () (int row = 0, int col = 0) const;

    // operators
    CtmpMatrix operator + (const CMatrix &m)
    {
      add(m);
      return *this;
    }

    CtmpMatrix operator - (const CMatrix &m)
    {
      subtract(m);
      return *this;
    }

    CtmpMatrix operator * (double num)
    {
      multiply(num);
      return *this;
    }

    friend inline CtmpMatrix operator + (const CMatrix &m0, const CMatrix &m1);
  //    friend inline CtmpMatrix operator + (const CMatrix &m0, const CtmpMatrix &m1);

    friend inline CtmpMatrix operator - (const CMatrix &m0, const CMatrix &m1);
    friend inline CtmpMatrix operator - (const CMatrix &m0, const CtmpMatrix &m1);

    friend inline CtmpMatrix operator * (const CMatrix &m0, const CMatrix &m1);
    friend inline CtmpMatrix operator * (const CMatrix &m, double num);
    friend inline CtmpMatrix operator * (double num, const CMatrix &m);

    friend inline CtmpMatrix operator / (const CMatrix &m, double num);

    friend inline CtmpMatrix operator | (const CMatrix &A, const CMatrix &B);
    friend inline CtmpMatrix operator & (const CMatrix &A, const CMatrix &B);

    friend bool operator == (const CMatrix &A, const CMatrix &B);
    friend bool operator != (const CMatrix &A, const CMatrix &B);

    // unary negation operator
    inline CtmpMatrix operator + (void)
    {
      return *this;
    }

    // unary negation operator
    inline CtmpMatrix operator - (void)
    {
      negate();
      return *this;
    }

};
// ##### CtmpMatrix class end #####


//-------------------------------------------------------------------
// How to make a CMatrix out of a CtmpMatrix
// These routines couldn't be defined within the declaration of CMatrix,
// because they refer to members of CtmpMatrix that hadn't been declared yet.

inline CMatrix::CMatrix(const CtmpMatrix &src) :
  m_nrows(src.m_nrows),
  m_ncols(src.m_ncols),
  m_size(src.m_size),
  m_data(src.steal_data())
{
}

//-------------------------------------------------------------------
// CMatrix::operator = (CtmpMatrix) -- assignment operator
inline CMatrix &CMatrix::operator = (const CtmpMatrix &src)
{
  MATRIX_ASSERT((m_nrows != src.m_nrows || m_ncols != src.m_ncols), "CMatrix::operator=: " <<
                "Bad assignment -- " << m_nrows << "x" << m_ncols << " matrix = " << src.m_nrows <<
                "x" << src.m_ncols << " matrix");

  delete[] m_data;
  m_data = src.steal_data();

  return *this;
}

//-------------------------------------------------------------------
// CMatrix::operator << (CtmpMatrix) -- assignment operator
inline CMatrix &CMatrix::operator << (const CtmpMatrix &src)
{
  delete[] m_data;
  m_data = src.steal_data();
  m_nrows = src.m_nrows;
  m_ncols = src.m_ncols;
  m_size = src.m_size;

  return *this;
}

//-------------------------------------------------------------------
// operator + () -- all permutations
inline CtmpMatrix operator + (const CMatrix &m0, const CMatrix &m1)
{
  CtmpMatrix tmp(m0);
  tmp.add(m1);
  return tmp;
}

// inline CtmpMatrix operator + (const CMatrix &m0, const CtmpMatrix &m1)
// {
//   CtmpMatrix tmp(m1);
//   tmp.add(m0);
//   return tmp;
// }

//-------------------------------------------------------------------
// operator - () -- all permutations
inline CtmpMatrix operator - (const CMatrix &m0, const CMatrix &m1)
{
  CtmpMatrix tmp(m0);
  tmp.subtract(m1);
  return tmp;
}

inline CtmpMatrix operator - (const CMatrix &m0, const CtmpMatrix &m1)
{
  CtmpMatrix tmp(m1);
  tmp.subtract_from(m0);
  return tmp;
}

//-------------------------------------------------------------------
// operator * () -- all permutations
inline CtmpMatrix operator * (const CMatrix &m0, const CMatrix &m1)
{
  CtmpMatrix tmp(m0, m1);
  return tmp;
}

inline CtmpMatrix operator * (const CMatrix &m, double num)
{
  CtmpMatrix tmp(m);
  tmp.multiply(num);
  return tmp;
}

inline CtmpMatrix operator * (double num, const CMatrix &m)
{
  CtmpMatrix tmp(m);
  tmp.multiply(num);
  return tmp;
}

//-------------------------------------------------------------------
// operator / ()
inline CtmpMatrix operator / (const CMatrix &m, double num)
{
  CtmpMatrix tmp(m);
  tmp.multiply(1./num);
  return tmp;
}

//-------------------------------------------------------------------
// operator | () -- concatenation of two matrices horizontally
//	                (concatenate the rows)
inline CtmpMatrix operator | (const CMatrix &A, const CMatrix &B)
{
  CtmpMatrix tmpA(A);
  CtmpMatrix tmp(tmpA.concat_rows(B));
  return tmp;
}

//-------------------------------------------------------------------
// operator & () -- concatenation of two matrices vertically
//	                (concatenate the columns)
inline CtmpMatrix operator & (const CMatrix &A, const CMatrix &B)
{
  CtmpMatrix tmpA(A);
  CtmpMatrix tmp(tmpA.concat_columns(B));
  return tmp;
}

//-------------------------------------------------------------------
// operator ~ () -- unary transpose
inline CtmpMatrix operator ~ (const CMatrix &m)
{
   CtmpMatrix tmp(m);
   return tmp.t();
}

//-------------------------------------------------------------------
// operator ~ () -- unary inversion
inline CtmpMatrix operator!(const CMatrix &m)
{
   CtmpMatrix tmp(m);
   return tmp.i();
}

//-------------------------------------------------------------------
// operator == (CMatrix, CMatrix) -- test for exact equality of two matrices
bool operator == (const CMatrix &A, const CMatrix &B);

//-------------------------------------------------------------------
// operator != (CMatrix, CMatrix) -- test for inequality (i.e. not exact equality)
inline bool operator != (const CMatrix &A, const CMatrix &B)
{
  return (A == B) ? false : true;
}


//-------------------------------------------------------------------
// ##### Non-member functions #####

//-------------------------------------------------------------------
// array_multiply(CMatrix, CMatrix) -- array multiplication of two matrices.
// Array multiplication is the element-by-element product of the matrices. (Matlab operator .*)
CtmpMatrix array_multiply(const CMatrix &A, const CMatrix &B);

//-------------------------------------------------------------------
// solve(CMatrix, CMatrix) -- solves a set of linear equations, Ax=b,
// using LU decomposition method.
// The matrix A and b is the first and the second argument of the function, respectivelly.
// The matrix A must be square and regular.
CtmpMatrix solve(const CMatrix &A, const CMatrix &b);


//-------------------------------------------------------------------
// min(CMatrix) -- minimum value of all matrix elements
inline double min(const CMatrix &M)
{
  return M.min();
}

//-------------------------------------------------------------------
// max(CMatrix) -- maximun value of all matrix elements
inline double max(const CMatrix &M)
{
  return M.max();
}

//-------------------------------------------------------------------
// abs(CMatrix) -- makes absolute values of the elements of a matrix
inline CtmpMatrix abs(const CMatrix &M)
{
  return M.abs();
}

//-------------------------------------------------------------------
// pow(CMatrix, double) -- powers each element of a matrix to the power of a double.
inline CtmpMatrix pow(const CMatrix &M, double p)
{
  return M.pow(p);
}

//-------------------------------------------------------------------
// pow2(CMatrix) -- powers each element of a matrix to the power of two,
// i.e. squares all the elements
inline CtmpMatrix pow2(const CMatrix &M)
{
  return M.pow2();
}

//-------------------------------------------------------------------
// swap(CMatrix, CMatrix) -- swaps values of two matrix objects.
inline void swap(CMatrix &A, CMatrix &B)
{
  MATRIX_ASSERT(((A.nrows() != B.nrows()) || (A.ncols() != B.ncols())),
                "CMatrix::swap(): " << "Can't swap matrices -- "
                << A.nrows() << "x" << A.ncols() << "matrix must be the same as " << B.nrows() << "x" << B.ncols() << " matrix");

//  A.swap(B);

  CtmpMatrix tmp(A);

  A = B;
  B = tmp;
}

//-------------------------------------------------------------------
// dot_product(CMatrix, CMatrix) -- dot product of A and B interpreted as vectors
// dot protuct of A and B is sum(A.*B)
double dot_product(const CMatrix &A, const CMatrix &B);

// ##### Non-member functions end #####

//typedef class CMatrix Matrix
#define Matrix CMatrix

// The specific size of matrix,
// don't require explicit definition

template <int rows, int cols> 
  class CMatrixMN : public CMatrix {

 public :
    CMatrixMN() : CMatrix(rows,cols) {};
};

#endif // CMATRIX_H_

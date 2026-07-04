#include <stdarg.h>
#include <math.h>
#include <iostream>

#include "CVector.hh"
#include "CMatrix.hh"


#if defined(__GNUC__)
#define COMPILER_STRING "gcc " __VERSION__
#else
#define COMPILER_STRING "a non-gcc compiler"
#endif

using namespace std;


//-------------------------------------------------------------------
// ##### Constants #####
static const double MATRIX_TINY = 1e-100; // it is used by LU decomposition to indicate numerical stabitily

static const string c_CMatrixVersion = CMATRIX_VERSION; // version of the Matrix library
// ##### Constants end #####


//-------------------------------------------------------------------
// ##### Local functions #####
static void lu_solve(const CMatrix &lu, int *OriginalRow, double *colBuf);

static int lu_decompose(CMatrix &M, int *OriginalRow, bool *NumSwapsIsOdd);
// ##### Local functions end #####


//-------------------------------------------------------------------
// ##### CLogAndSign class #####

//-------------------------------------------------------------------
// CLogAndSign::operator *= (double) -- multiplies by a double
void CLogAndSign::operator *= (double x)
{
  if (x > 0.0) {
    log_val += log(x);
  }
  else if (x < 0.0) {
    log_val += log(-x);
    sign_val = -sign_val;
  }
  else sign_val = 0;
}

//-------------------------------------------------------------------
// CLogAndSign::value() -- returns the value (no log transform)
double CLogAndSign::value() const
{
//  MATRIX_ASSERT((log_val >= log(numeric_limits<double>::max())), "CLogAndSign::value: Overflow in exponential");

  return sign_val * exp(log_val);
}

//-------------------------------------------------------------------
// CLogAndSign::CLogAndSign(double) -- initialization by a double
CLogAndSign::CLogAndSign(double f)
{
  if (f == 0.0) {
    log_val = 0.0;
    sign_val = 0;
    return;
  }
  else if (f < 0.0) {
    sign_val = -1;
    f = -f;
  }
  else sign_val = 1;
  log_val = log(f);
}

// ##### CLogAndSign class end #####


//-------------------------------------------------------------------
// ##### CMatrixInput class #####
// CMatrixInput routines are taken over newmat11 library by R. Davies
// int CMatrixInput::n;         // number values still to be read
// double* CMatrixInput::r;     // pointer to next location to be read to

//-------------------------------------------------------------------
// CMatrixInput::operator << (double) -- loads next value to a matrix
// Purpose: Loads the elements of a matrix from list using a sequence of numbers
// separated by <<
CMatrixInput CMatrixInput::operator << (double f)
{
  MATRIX_ASSERT((n <= 0),
                "CMatrixInput::operator<<: " << "List of values too long in input operator <<");

  *r = f;

  int n1 = n - 1;
  n = 0;  // n=0 so we won't trigger exception
  return CMatrixInput(n1, r + 1);
}

//-------------------------------------------------------------------
// CMatrixInput::~CMatrixInput() -- finishes reading of values to a matrix
CMatrixInput::~CMatrixInput(void)
{

/*

CMatrix.cc: In destructor ‘CMatrixInput::~CMatrixInput()’:
CMatrix.hh:841:36: warning: throw will always call terminate() [-Wterminate]
     throw CMatrixError(strstr.str()); \
                                    ^
CMatrix.hh:824:5: note: in expansion of macro ‘MATRIX_THROW’
     MATRIX_THROW(error_msg);                \
     ^~~~~~~~~~~~
CMatrix.cc:106:3: note: in expansion of macro ‘MATRIX_ASSERT’
   MATRIX_ASSERT((n != 0),
   ^~~~~~~~~~~~~

*/

//  MATRIX_ASSERT((n != 0),
//                "CMatrixInput::~CMatrixInput: " << "A list of values was too short in input operator <<");

}

//-------------------------------------------------------------------
// CMatrixInput::operator << (double) -- loads the first value to a matrix
// Purpose: Loads the elements of a matrix from list using a sequence of numbers
// separated by <<
CMatrixInput CMatrix::operator << (double f)
{
  int n = nrows() * ncols();
  MATRIX_ASSERT((n <= 0),
                "CMatrix::operator<<: " << "Loading data to zero length matrix in input operator <<");

  double *r;
  r = data();
  *r = f;
  n--;
  return CMatrixInput(n, r + 1);
}
// ##### CMatrixInput class end #####


//-------------------------------------------------------------------
// ##### CMatrix class #####

//-------------------------------------------------------------------
// ostream_indent -- indents output stream, e.g. cout, of NumSpaces spaces
inline void ostream_indent(ostream &os, int NumSpaces)
{
  int i;

  for (i = 0; i < NumSpaces; i++) {
    os << " ";
  }
}

//-------------------------------------------------------------------
// CMatrix::operator()(int, int) -- returns a value for CMatrix(row, col)
 double &CMatrix::operator () (int row, int col)const
{
  MATRIX_ASSERT( ((0 > row) || (row >= m_nrows) || (0 > col) || (col >= m_ncols)),
                 "CMatrix::operator(): " << "Matrix index <" << row << "," << col << ">" <<
                 " out of bounds in " << m_nrows << "x" << m_ncols << " matrix");

  return m_data[row * m_ncols + col];
}

//-------------------------------------------------------------------
// CMatrix::operator = (double) -- fills a matrix with one value of a double
CMatrix &CMatrix::operator = (double val)
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::operator=: " << m_nrows << "x" << m_ncols << " matrix is not valid");

  int i;
  double *p = m_data;

  for (i = 0; i < m_size; i++) {
    *p++ = val;
  }

  return *this;
}

//-------------------------------------------------------------------
// CMatrix::operator - () -- unary negation
CMatrix CMatrix::operator - (void)
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::operator-: " << m_nrows << "x" << m_ncols << " matrix is not valid");

  CtmpMatrix tmp(*this);
  tmp.negate();
  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::operator += (CMatrix) -- adds a matrix
void CMatrix::operator += (const CMatrix &A)
{
  MATRIX_ASSERT(( (!is_matrix()) || (!A.is_matrix()) || (m_nrows != A.nrows()) || (m_ncols != A.ncols()) ),
                "CMatrix::operator+=: " << "Bad add -- "
                << m_nrows << "x" << m_ncols << " matrix + " << A.nrows() << "x" << A.ncols() << " matrix");

  CtmpMatrix tmp(*this);
  tmp.add(A);
  *this = tmp;
}

//-------------------------------------------------------------------
// CMatrix::operator += (double) -- adds a double
void CMatrix::operator += (double num)
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::operator+=: " << m_nrows << "x" << m_ncols << " matrix is not valid");

  CtmpMatrix tmp(*this);
  tmp.add(num);
  *this = tmp;
}

//-------------------------------------------------------------------
// CMatrix::operator -= (CMatrix) -- subtracts a matrix
void CMatrix::operator -= (const CMatrix &A)
{
  MATRIX_ASSERT(( (!is_matrix()) || (!A.is_matrix()) || (m_nrows != A.nrows()) || (m_ncols != A.ncols()) ),
                "CMatrix::operator-=: " << "Bad subtract -- "
                << m_nrows << "x" << m_ncols << " matrix + " << A.nrows() << "x" << A.ncols() << " matrix");

  // subtract matrix
  CtmpMatrix tmp(*this);
  tmp.subtract(A);
  *this = tmp;
}

//-------------------------------------------------------------------
// CMatrix::operator *= (CMatrix) -- multiplies by a matrix
// Resulting matrix is resized if a result of the matrix multiplication has different size as the matrix itself
// E.g.: Given A 2x3 matrix and B 3x2 matrix: A *= B creates 2x2 matrix A equal to A*B
void CMatrix::operator *= (const CMatrix &A)
{
  MATRIX_ASSERT(( (!is_matrix()) || (!A.is_matrix()) || (A.nrows() != m_ncols)),
                "CMatrix::operator*=: " << "Bad multiply -- " <<
                m_nrows << "x" << m_ncols << " matrix * " << A.nrows() << "x" << A.ncols() << " matrix");

  CtmpMatrix tmp(*this, A);

  delete[] m_data;
  m_data = tmp.steal_data();
  m_nrows = tmp.nrows();
  m_ncols = tmp.ncols();
  m_size = m_nrows * m_ncols;
}

//-------------------------------------------------------------------
// CMatrix::operator *= (double) -- multiplies by a double
void CMatrix::operator *= (double num)
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::operator*=: " << m_nrows << "x" << m_ncols << " matrix is not valid");

  CtmpMatrix tmp(*this);
  tmp.multiply(num);

  *this = tmp;
}

//-------------------------------------------------------------------
// CMatrix::operator |= (CMatrix) -- concatenates two matrices horizontally
//                                   (concatenates the rows)
void CMatrix::operator |= (const CMatrix &A)
{
  // no exception: resulting matrix is resized
  CtmpMatrix tmp(*this);
  tmp.concat_rows(A);

  delete[] m_data;
  m_data = tmp.steal_data();
  m_nrows = tmp.nrows();
  m_ncols = tmp.ncols();
  m_size = m_nrows * m_ncols;
}

//-------------------------------------------------------------------
// CMatrix::operator &= (CMatrix) -- concatenates two matrices vertically
//                                   (concatenates the columns)
void CMatrix::operator &= (const CMatrix &A)
{
  // no exception: resulting matrix is resized
  CtmpMatrix tmp(*this);
  tmp.concat_columns(A);

  delete[] m_data;
  m_data = tmp.steal_data();
  m_nrows = tmp.nrows();
  m_ncols = tmp.ncols();
  m_size = m_nrows * m_ncols;
}

//-------------------------------------------------------------------
// CMatrix::operator == (CMatrix) -- logical equal-to operator
bool operator == (const CMatrix &A, const CMatrix &B)
{
  if ((A.nrows() != B.nrows()) || (A.ncols() != B.ncols())) {
    return false;
  }

  double *p_a = A.data();
  double *p_b = B.data();

  for (int i = 0; i < A.size(); i++) {
    if (*p_a++ != *p_b++) {
      return false;
    }
  }

  return true;
}

//-------------------------------------------------------------------
// CMatrix::operator = (CMatrix) -- fills the matrix with another CMatrix
// with respect of its size
CMatrix &CMatrix::operator = (const CMatrix &src)
{
  MATRIX_ASSERT((!is_matrix()) || (m_nrows != src.m_nrows || m_ncols != src.m_ncols),
                "CMatrix::operator=: " << "Bad assignment -- "
                << m_nrows << "x" << m_ncols << " matrix = " << src.m_nrows << "x" << src.m_ncols << " matrix");

  memcpy(m_data, src.m_data, m_size * sizeof(*m_data));
  return *this;
}

//-------------------------------------------------------------------
// CMatrix::operator << (CMatrix) -- assigns a matrix to another CMatrix
// with no respect to its size - resulting matrix is resized
CMatrix &CMatrix::operator << (const CMatrix &src)
{
  // no exception: resulting matrix is resized
  CtmpMatrix tmp(src);

  delete[] m_data;
  m_data = tmp.steal_data();
  m_nrows = tmp.nrows();
  m_ncols = tmp.ncols();
  m_size = tmp.size();

  return *this;
}

//-------------------------------------------------------------------
// CMatrix::operator << (double *) -- loads the elements of a matrix from an array.
// This construction does not check that the numbers of elements match correctly.
// If there are too many elements in the array, the extras will be ignored.
// If there are too few, the last few values in the matrix will be filled with garbage.
void CMatrix::operator << (const double *r)
{
  int i = m_size;
  double *s = m_data;
  while (i--) {
    *s++ = *r++;
  }
}

//-------------------------------------------------------------------
// CMatrix::column(int) -- returns a column of a matrix.
CtmpMatrix CMatrix::column(int col) const
{
  MATRIX_ASSERT((!is_matrix() || (0 > col) || (col >= m_ncols)), "CMatrix::column(): " << "Column index <" << col <<
                ">" << " out of bounds in " << m_nrows << "x" << m_ncols << " matrix");

  CtmpMatrix tmp(m_nrows, 1);
  double *p_this;
  double *p_tmp;

  p_this = m_data + col;
  p_tmp = tmp.data();

  for (int row = 0; row < m_nrows; row++) {
    *p_tmp = *p_this;
    p_this += m_ncols;
    p_tmp++;
  }

  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::row(int) -- returns a row of a matrix.
CtmpMatrix CMatrix::row(int row) const
{
  MATRIX_ASSERT((!is_matrix() || (0 > row) || (row >= m_nrows)), "CMatrix::row(): " << "Row index <" << row <<
                ">" << " out of bounds in " << m_nrows << "x" << m_ncols << " matrix");

  CtmpMatrix tmp(1, m_ncols);
  double *p_this;
  double *p_tmp;

  p_this = m_data + row * m_ncols;
  p_tmp = tmp.data();

  for (int col = 0; col < m_ncols; col++) {
    *p_tmp++ = *p_this++;
  }

  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::submatrix(int, int, int , int) -- returns a submatrix from a matrix.
// The arguments fr, lr, fc, lc are the first row, last row, first column,
// last column of the submatrix.
CtmpMatrix CMatrix::submatrix(int fr, int lr, int fc, int lc) const
{
  MATRIX_ASSERT( (!is_matrix() || (0 > fr) || (0 > fc) || (lr >= m_nrows) || (lc >= m_ncols) ||
                  (fr > lr) || (fc > lc)),
                 "CMatrix::submatrix(): " <<
                 "Submatrix (" << fr << ".." << lr << "," << fc << ".." << lc << ")" <<
                 " out of " << m_nrows << "x" << m_ncols << " matrix");

  int nrows = lr-fr+1;
  int ncols = lc-fc+1;
  CtmpMatrix tmp(nrows, ncols);
  double *p_this;
  double *p_tmp;

  p_tmp = tmp.data();
  p_this = m_data + fr*m_ncols + fc;

  for (int row = 0; row < nrows; row++) {
    for (int col = 0; col < ncols; col++) {
      *p_tmp++ = *p_this++;
    }
    p_this += (m_ncols - nrows);
  }

  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::diag -- returns the diagonal matrix of a matrix.
// The diagonal matrix has the same size as the original one and contains only
// values at the main diagonal.
CtmpMatrix CMatrix::diag() const
{
  MATRIX_ASSERT((!is_square()),
                "CMatrix::diag(): " << "Can't create diagonal matrix -- Not square " << m_nrows << "x" << m_ncols << " matrix");

  CtmpMatrix tmp(m_nrows, m_ncols); // create a new matrix and set it to zero
  double *p_this;
  double *p_tmp;

  p_this = m_data;
  p_tmp = tmp.data();

  for (int i = 0; i < m_nrows; i++) {
    *p_tmp = *p_this;
    p_tmp += (m_ncols + 1);
    p_this += (m_ncols + 1);
  }

  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::assign_column(int, CMatrix) -- assignes a column into a matrix at
// given column.
// The assigned column must be inside the matrix.
void CMatrix::assign_column(int col, const CMatrix &C)
{
  MATRIX_ASSERT((!is_matrix()), "CMatrix::assign_column(): " << "Can't assign a column to " <<
                m_nrows << "x" << m_ncols << " matrix");
  MATRIX_ASSERT(((0 > col) || (col >= m_ncols)), "CMatrix::assign_column(): " << "Column index <" << col <<
                ">" << " out of bounds in " << m_nrows << "x" << m_ncols << " matrix");
  MATRIX_ASSERT((!C.is_column()),
                "CMatrix::assign_column(): " << "Assigned " << C.nrows() << "x" << C.ncols() << " matrix is not a column");
  MATRIX_ASSERT((m_nrows != C.nrows()),
                "CMatrix::assign_column(): " << "Bad assignment -- " << m_nrows << "x" << m_ncols << " matrix = " << C.nrows() <<
                "x" << C.ncols() << " matrix -- number of rows must be the same");

  double *p_M;
  double *p_C;

  p_M = m_data + col;
  p_C = C.data();

  for (int row = 0; row < m_nrows; row++) {
    *p_M = *p_C;
    p_M += m_ncols;
    p_C++;
  }
}

//-------------------------------------------------------------------
// CMatrix::assign_row(int, CMatrix) -- assignes a row into a matrix at given row.
// The assigned row must be inside the matrix.
void CMatrix::assign_row(int row, const CMatrix &R)
{
  MATRIX_ASSERT((!is_matrix()), "CMatrix::assign_row(): " << "Can't assign a row to " <<
                m_nrows << "x" << m_ncols << " matrix");
  MATRIX_ASSERT(((0 > row) || (row >= m_nrows)), "CMatrix::assign_row(): " << "Row index <" << row <<
                ">" << " out of bounds in " << m_nrows << "x" << m_ncols << " matrix");
  MATRIX_ASSERT((!R.is_row()),
                "CMatrix::assign_row(): " << "Assigned " << R.nrows() << "x" << R.ncols() << " matrix is not a row");
  MATRIX_ASSERT((m_ncols != R.ncols()),
                "CMatrix::assign_row(): " << "Bad assignment -- " << m_nrows << "x" << m_ncols << " matrix = " << R.nrows() <<
                "x" << R.ncols() << " matrix -- number of columns must be the same");

  double *p_M;
  double *p_R;

  p_M = m_data + row * m_ncols;
  p_R = R.data();

  for (int col = 0; col < m_ncols; col++) {
    *p_M++ = *p_R++;
  }
}

//-------------------------------------------------------------------
// CMatrix::assign_submatrix(int, int, CMatrix)
//   -- Assignes a submatrix into a matrix.
// The arguments row and col are the first row and the first column of the
// submatrix. The submatrix must be inside the matrix.
void CMatrix::assign_submatrix(int row, int col, const CMatrix &M)
{
  MATRIX_ASSERT((!is_matrix()), "CMatrix::assign_submatrix(): " << "Can't create a submatrix to " <<
                m_nrows << "x" << m_ncols << " matrix");
  MATRIX_ASSERT((!M.is_matrix()),
                "CMatrix::assign_submatrix(): " << "Assigned " << M.nrows() << "x" << M.ncols() <<
                " submatrix matrix is empty or is not valid");

  int nrows = M.nrows();
  int ncols = M.ncols();

  MATRIX_ASSERT(((0 > row) || (0 > col) || ((row + nrows) > m_nrows) || ((col + ncols) > m_ncols)),
                "CMatrix::assign_submatrix(): " <<
                "Submatrix (" << row << ".." << (row + nrows - 1) << "," << col << ".." << (col + ncols - 1) << ")" <<
                " out of " << m_nrows << "x" << m_ncols << " matrix");

  double *p_this;
  double *p_M;

  p_M = M.data();
  p_this = m_data + row*m_ncols + col;

  for (int i = 0; i < nrows; i++) {
    for (int j = 0; j < ncols; j++) {
      *p_this++ = *p_M++;
    }
    p_this += (m_ncols - ncols);
  }
}

//-------------------------------------------------------------------
// CMatrix::assign_submatrix(int, int, int, int, CMatrix)
//   -- Assignes a submatrix into a matrix.
// The arguments fr, lr, fc, lc are the first row, last row, first
// column, last column of the submatrix. The sizes of the given submatrix and
// the matrix given bt the arguments fr, lr, fc, lc must correspond and the
// submatrix must be inside the matrix.
void CMatrix::assign_submatrix(int fr, int lr, int fc, int lc, const CMatrix &M)
{
  MATRIX_ASSERT((!is_matrix()), "CMatrix::assign_submatrix(): " << "Can't assign a submatrix to " <<
                m_nrows << "x" << m_ncols << " matrix");
  MATRIX_ASSERT(((0 > fr) || (0 > fc) || (lr >= m_nrows) || (lc >= m_ncols) || (fr > lr) || (fc > lc)),
                "CMatrix::assign_submatrix(): " << "Submatrix (" << fr << ".." << lr << "," << fc << ".." << lc << ")" <<
                " out of " << m_nrows << "x" << m_ncols << " matrix");
  MATRIX_ASSERT((!M.is_matrix()),
                "CMatrix::assign_submatrix(): " << "Assigned " << M.nrows() << "x" << M.ncols() <<
                " submatrix matrix is empty or is not valid");

  int nrows = lr-fr+1;
  int ncols = lc-fc+1;

  MATRIX_ASSERT(((nrows != M.nrows()) || (ncols != M.ncols())),
                "CMatrix::assign_submatrix(): " << "Bad assignment -- " << nrows << "x" << ncols << " matrix = " << M.nrows() <<
                "x" << M.ncols() << " matrix");

  double *p_this;
  double *p_M;

  p_M = M.data();
  p_this = m_data + fr*m_ncols + fc;

  for (int row = 0; row < nrows; row++) {
    for (int col = 0; col < ncols; col++) {
      *p_this++ = *p_M++;
    }
    p_this += (m_ncols - ncols);
  }
}

//-------------------------------------------------------------------
// CMatrix::set(double, ...) -- fills a matrix with an array of values
void CMatrix::set(double firstVal, ...)
{
  va_list ap;

  va_start(ap, firstVal);
  vset(firstVal, ap);
  va_end(ap);
}

//-------------------------------------------------------------------
// CMatrix::vset(double, va_list) -- fills a matrix with an array of values
void CMatrix::vset(double firstVal, va_list ap)
{
  int i;
  double *p = m_data;

  *p++ = firstVal;

  for (i = 1; i < m_size; i++) {
    *p++ = va_arg(ap, double);
  }
}

//-------------------------------------------------------------------
// CMatrix::t() -- transpose of a matrix
CtmpMatrix CMatrix::t(void) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::t(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  CtmpMatrix tmp(m_ncols, m_nrows);
  int row, col;
  double *p_this;
  double *p_tmp;

  p_this = m_data;
  p_tmp = tmp.data();

  for (row = 0; row < m_nrows; row++) {
    for (col = 0; col < m_ncols; col++) {
      *p_tmp = *p_this++;
      p_tmp += m_nrows;
    }

    p_tmp -= m_size - 1;
  }

  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::reduce(int, int ) -- reduces a matrix to a given number of rows and columns.
// It is possible to reduce the matrix up to 0x0 (e.g. for a memory save)
CtmpMatrix CMatrix::reduce(int nrows, int ncols)
{
  MATRIX_ASSERT((0 > nrows || nrows > m_nrows || 0 > ncols || ncols > m_ncols),
                "CMatrix::reduce(): " << "Can't reduce " << m_nrows << "x" << m_ncols <<
                " matrix " << "to " << nrows << "x" << ncols);

  CtmpMatrix tmp(nrows, ncols);
  int row, col;
  double *p_this;
  double *p_tmp;

  p_this = m_data;
  p_tmp = tmp.data();

  for (row = 0; row < nrows; row++) {
    for (col = 0; col < ncols; col++) {
      *p_tmp++ = *p_this++;
    }

    p_this += m_ncols - ncols;
  }

  return tmp;
}

//-------------------------------------------------------------------
//CMatrix::resize(int, int) -- resizes a matrix to a given number of rows and columns.
// It is possible to resize down (reduce) the matrix up to 0x0 (e.g. for a memory save)
void CMatrix::resize(int nrows, int ncols)
{
  MATRIX_ASSERT((0 > nrows || 0 > ncols), "CMatrix::resize(): " << "Can't reduce " << m_nrows <<
                "x" << m_ncols << " matrix " << "to " << nrows << "x" << ncols);

  CtmpMatrix tmp(nrows, ncols); // create a new temporary matrix and set it to zero
  int row, col;
  double *p_this;
  double *p_tmp;

  p_this = m_data;
  p_tmp = tmp.data();

  int Rows = nrows < m_nrows ? nrows : m_nrows;
  int Cols = ncols < m_ncols ? ncols : m_ncols;

  for (row = 0; row < Rows; row++) {
    for (col = 0; col < Cols; col++) {
      *p_tmp++ = *p_this++;
    }

    p_this += m_ncols - Cols;
    p_tmp += ncols - Cols;
  }

  delete[] m_data;
  m_data = tmp.steal_data();
  m_nrows = nrows;
  m_ncols = ncols;
  m_size = nrows * ncols;
}

//-------------------------------------------------------------------
// CMatrix::zero() -- sets zero to all elements of this matrix
void CMatrix::zero(void)
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::zero(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  memset(m_data, 0, m_nrows * m_ncols * sizeof(*m_data));
}

//-------------------------------------------------------------------
// CMatrix::zero(int, int) -- resizes the matrix to nrows x ncols matrix
// and sets zero to all elements of this matrix
void CMatrix::zero(int nrows, int ncols)
{
  MATRIX_ASSERT((0 >= nrows || 0 >= ncols), "CMatrix::zero(): " << "Can't reduce " << m_nrows <<
                "x" << m_ncols << " matrix " << "to " << nrows << "x" << ncols);

  CtmpMatrix tmp(nrows, ncols); // create a new matrix and set it to zero

  delete[] m_data;
  m_data = tmp.steal_data();
  m_nrows = nrows;
  m_ncols = ncols;
  m_size = nrows * ncols;
}

//-------------------------------------------------------------------
// CMatrix::identity() -- sets a matrix to the identity matrix
void CMatrix::identity(void)
{
  MATRIX_ASSERT((!is_square()),
                "CMatrix::identity(): " << "Can't create identity matrix -- Not square "
                << m_nrows << "x" << m_ncols << " matrix");

  double *p_d = m_data;

  memset(p_d, 0, m_size * sizeof(*m_data));

  for (int i = 0; i < m_nrows; i++) {
    *p_d = 1.;  // m_data[i * m_ncols + i]
    p_d += m_ncols + 1;
  }
}

//-------------------------------------------------------------------
// CMatrix::identity(int) -- resizes a matrix to nrows x nrows matrix
// and set it to the identity matrix
void CMatrix::identity(int nrows)
{
  MATRIX_ASSERT((0 >= nrows), "CMatrix::identity(): " << "Can't reduce " << m_nrows <<
                "x" << m_ncols << " matrix " << "to " << nrows << "x" << nrows);

  CtmpMatrix tmp(nrows, nrows); // create a new matrix and set it to zero

  delete[] m_data;
  m_data = tmp.steal_data();
  m_nrows = nrows;
  m_ncols = nrows;
  m_size = nrows * nrows;

  double *p_d = m_data;

  for (int i = 0; i < m_nrows; i++) {
    *p_d = 1.;  // m_data[i * m_ncols + i]
    p_d += m_ncols + 1;
  }
}

//-------------------------------------------------------------------
// CMatrix::swap(CMatrix) -- swaps values of two matrix objects.
void CMatrix::swap(const CMatrix &M)
{
  MATRIX_ASSERT(((m_nrows != M.nrows()) || (m_ncols != M.ncols())),
                "CMatrix::swap(): " << "Can't swap matrices -- "
                << M.nrows() << "x" << M.ncols() << "matrix must be the same as " << m_nrows << "x" << m_ncols << " matrix");

  // it is not possible to only change m_data pointer due to m_data are protected (read only) in the M matrix

  CtmpMatrix tmp(M); // create a new temporary matrix and copy matrix M to it

  size_t nbytes = m_size * sizeof(*m_data);

  memcpy(tmp.m_data, m_data, nbytes);
  memcpy(m_data, M.m_data, nbytes);
  memcpy(M.m_data, tmp.m_data, nbytes);
}

//-------------------------------------------------------------------
// CMatrix::reverse() -- reverses order of the elements of matrix.
CtmpMatrix CMatrix::reverse(void) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::reverse(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  CtmpMatrix tmp(m_nrows, m_ncols);

  double *p_this = m_data + m_size - 1;
  double *p_tmp = tmp.data();

  for (int i = 0; i < m_size; i++) {
    *p_tmp++ = *p_this--;
  }

  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::as_row() -- interprets matrix body as a single row
CtmpMatrix CMatrix::as_row(void) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::as_row(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  CtmpMatrix tmp(1, m_size);

  memcpy(tmp.m_data, m_data, m_size * sizeof(*m_data));

  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::as_column() -- interprets matrix body as a single column
CtmpMatrix CMatrix::as_column(void) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::as_column(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  CtmpMatrix tmp(m_size, 1);

  memcpy(tmp.m_data, m_data, m_size * sizeof(*m_data));

  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::as_diag() -- interprets matrix body as a diagonal matrix
CtmpMatrix CMatrix::as_diag(void) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::as_diag(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  CtmpMatrix tmp(m_size, m_size);

  double *p_this = m_data;
  double *p_tmp = tmp.data();

  for (int i = 0; i < m_size; i++) {
    *p_tmp = *p_this++;
    p_tmp += (m_size + 1);
  }

  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::as_matrix(int, int) -- interprets matrix body as a matrix of given size
CtmpMatrix CMatrix::as_matrix(int nrows, int ncols)
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::as_matrix(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  MATRIX_ASSERT((m_size != nrows * ncols),
                "CMatrix::as_matrix(): " << "Size of " << m_nrows << "x" << m_ncols << " matrix (" << m_size << ") must be the same as requested "
                << nrows << "x" << ncols << " matrix");

  CtmpMatrix tmp(nrows, ncols);

  memcpy(tmp.m_data, m_data, m_size * sizeof(*m_data));

  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::as_scalar() -- interprets matrix body of matrix of size 1x1 as a scalar,
// i.e. converts matrix of size 1x1 to a double
double CMatrix::as_scalar(void)
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::as_scalar(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  MATRIX_ASSERT((m_size != 1),
                "CMatrix::as_scalar(): " << "Can't convert " << m_nrows << "x" << m_ncols << " matrix to scalar");

  return *m_data;
}

//-------------------------------------------------------------------
// CMatrix::i() -- inverts a matrix
// This uses the LU Decomposition method described in
// William H. Press, et al,
// NUMERICAL RECIPES IN C
// pp 40-47
CtmpMatrix CMatrix::i(void) const
{
  MATRIX_ASSERT((!is_square()), "CMatrix::i(): " << "Can't find inverse of " << m_nrows <<
                "x" << m_ncols << " matrix -- " << "must be square");

  CVectorOf<int> OriginalRow;
  OriginalRow.resize(m_nrows);

  CVectorOf<double> colBuf;
  colBuf.resize(m_nrows);

  CtmpMatrix lu(*this);
  CtmpMatrix tmp(m_nrows, m_ncols);
  double *p0, *p1;
  int row, col;
  bool dummy;
  int ZeroPivots;

  ZeroPivots = lu_decompose(lu, &OriginalRow[0], &dummy);
  MATRIX_ASSERT((ZeroPivots), "CMatrix::i(): " << "Can't find inverse of " << m_nrows << "x" <<
                m_ncols << " matrix -- " << "matrix is singular"
                << ":" << endl << ((m_nrows <= 4) ? *this : 0));

  for (col = 0; col < m_ncols; col++) {
    colBuf.clear();
    colBuf[col] = 1;
    lu_solve(lu, &OriginalRow[0], &colBuf[0]);
    p0 = &tmp(0, col);
    p1 = &colBuf[0];
    for (row = 0; row < m_nrows; row++) {
      *p0 = *p1++;
      p0 += m_ncols;
    }
  }

  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::array_multiply(CMatrix) -- array multiplication of two matrices.
// Array multiplication is the element-by-element product of the matrices. (Matlab operator .*)
CtmpMatrix CMatrix::array_multiply(const CMatrix &m) const
{
  CtmpMatrix tmp(*this);

  tmp.array_multiply(m);

  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::abs() -- makes absolute values of the elements of a matrix
CtmpMatrix CMatrix::abs(void) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::abs(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  CtmpMatrix tmp(*this);

  double *p_tmp = tmp.m_data;

  for (int i = 0; i < m_size; i++) {
    *p_tmp = fabs(*p_tmp);
    p_tmp++;
  }

  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::pow(double) -- powers each element of a matrix to the power of a double.
CtmpMatrix CMatrix::pow(double p) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::pow(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  CtmpMatrix tmp(*this);

  double *p_tmp = tmp.m_data;

  for (int i = 0; i < m_size; i++) {
    *p_tmp = ::pow(*p_tmp, p);
    p_tmp++;
  }

  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::pow2() -- powers each element of a matrix to the power of two,
// i.e. squares all the elements
CtmpMatrix CMatrix::pow2(void) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::pow2(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  CtmpMatrix tmp(*this);

  double *p_tmp = tmp.m_data;

  for (int i = 0; i < m_size; i++) {
    *p_tmp = (*p_tmp) * (*p_tmp);
    p_tmp++;
  }

  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::det() -- determinant of a matrix
// This uses the LU Decomposition method described in
// William H. Press, et al,
// NUMERICAL RECIPES IN C
// pp 40-47
double CMatrix::det(void) const
{
  MATRIX_ASSERT((!is_square()), "CMatrix::det(): " << "Can't find determinant of " <<
                m_nrows << "x" << m_ncols << " matrix -- " << "must be square");

  CtmpMatrix lu(*this);
  CVectorOf<int> dummyBuf;
  dummyBuf.resize(m_nrows);

  double d;
  bool NumSwapsWasOdd;
  int ZeroPivots;

  ZeroPivots = lu_decompose(lu, &dummyBuf[0], &NumSwapsWasOdd);
  if (ZeroPivots > 0) {
    // Matrix is singular
    d = 0.0;
  }
  else {
    double *p0 = lu.data();
    d = NumSwapsWasOdd ? -*p0 : *p0;
    for (int i = 1; i < m_nrows; i++) {
      p0 += m_ncols + 1;
      d *= *p0;
    }
  }

  return d;
}

//-------------------------------------------------------------------
// CMatrix::log_det() -- natural logarithm of the determinant of a matrix
// This uses the same LU decomposition method as det()
CLogAndSign CMatrix::log_det(void) const
{
  MATRIX_ASSERT((!is_square()), "CMatrix::log_det(): " << "Can't find determinant of " <<
                m_nrows << "x" << m_ncols << " matrix -- " << "must be square");

  CtmpMatrix lu(*this);
  CVectorOf<int> dummyBuf;
  dummyBuf.resize(m_nrows);

  CLogAndSign ld;
  bool NumSwapsWasOdd;
  int ZeroPivots;


  ZeroPivots = lu_decompose(lu, &dummyBuf[0], &NumSwapsWasOdd);
  if (ZeroPivots > 0) {
    // Matrix is singular
    ld = 0.0;
  }
  else {
    double *p0 = lu.data();
    ld = NumSwapsWasOdd ? -*p0 : *p0;
    for (int i = 1; i < m_nrows; i++) {
      p0 += m_ncols + 1;
      ld *= *p0;
    }
  }

  return ld;
}

//-------------------------------------------------------------------
// CMatrix::sum() -- sum of all elements
double CMatrix::sum(void) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::sum(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  double sum = 0.0;
  double *p_d = m_data;

  for (int i = 0; i < m_size; i++) {
    sum += *p_d++;
  }

  return sum;
}

//-------------------------------------------------------------------
// sum_square() -- sum of squares of all elements
double CMatrix::sum_square(void) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::sum_square(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  double sum = 0.0;
  double *p_d = m_data;

  for (int i = 0; i < m_size; i++) {
    double val = *p_d++;
    sum += val * val;
  }

  return sum;
}

//-------------------------------------------------------------------
// CMatrix::sum_rows() -- sum elements in each row
CtmpMatrix CMatrix::sum_rows(void) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::sum_rows(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  CtmpMatrix tmp(m_nrows,1);

  double *p_this = m_data;
  double *p_tmp = tmp.data();

  for (int row = 0; row < m_nrows; row++) {

    double sum = 0.0;

    for (int col = 0; col < m_ncols; col++) {
      sum += *p_this++;
    }
    *p_tmp++ = sum;
  }

  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::sum_columns() -- sum elements in each column
CtmpMatrix CMatrix::sum_columns(void) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::sum_columns(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  CtmpMatrix tmp(1,m_ncols);

  double *p_tmp = tmp.data();

  for (int col = 0; col < m_ncols; col++) {

    double sum = 0.0;
    double *p_this = m_data + col;

    for (int row = 0; row < m_nrows; row++) {
      sum += *p_this;
      p_this += m_ncols;
    }
    *p_tmp++ = sum;
  }

  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::sum_square_rows() -- sum squares of elements in each row
CtmpMatrix CMatrix::sum_square_rows(void) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::sum_square_rows(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  CtmpMatrix tmp(m_nrows,1);

  double *p_this = m_data;
  double *p_tmp = tmp.data();

  for (int row = 0; row < m_nrows; row++) {

    double sum = 0.0;

    for (int col = 0; col < m_ncols; col++) {
      double val = *p_this++;
      sum += val * val;
    }
    *p_tmp++ = sum;
  }

  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::sum_square_columns() -- sum squares of elements in each column
CtmpMatrix CMatrix::sum_square_columns(void) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::sum_square_columns(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  CtmpMatrix tmp(1,m_ncols);

  double *p_tmp = tmp.data();

  for (int col = 0; col < m_ncols; col++) {

    double sum = 0.0;
    double *p_this = m_data + col;

    for (int row = 0; row < m_nrows; row++) {
      double val = *p_this;
      sum += val * val;
      p_this += m_ncols;
    }
    *p_tmp++ = sum;
  }

  return tmp;
}

//-------------------------------------------------------------------
// CMatrix::min() -- minimum value of elements
double CMatrix::min(void) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::min(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  double *p_d = m_data;
  double minval = *p_d;

  for (int i = 0; i < m_size; i++) {
    double val = *p_d++;
    if (val < minval) {
      minval = val;
    }
  }

  return minval;
}

//-------------------------------------------------------------------
// CMatrix::max() -- maximum value of elements
double CMatrix::max(void) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::max(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  double *p_d = m_data;
  double maxval = *p_d;

  for (int i = 0; i < m_size; i++) {
    double val = *p_d++;
    if (val < maxval) {
      maxval = val;
    }
  }

  return maxval;
}

//-------------------------------------------------------------------
// CMatrix::norm1() -- maximum of sum of absolute values of elements of a column
double CMatrix::norm1(void) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::norm1(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  double maxs = 0.0;

  for (int col = 0; col < m_ncols; col++) {

    double *p_d = m_data + col;
    double s = 0.0;

    for (int row = 0; row < m_nrows; row++) {
      s += fabs(*p_d);  // m_data[row,col]
      p_d += m_ncols;
    }
    if (s > maxs) {
      maxs = s ;
    }
  }

  return maxs;
}

//-------------------------------------------------------------------
// CMatrix::norm_inf() -- for matrix: maximum of sum of absolute values of elements of a row,
//                        for vector: maximum of absolute values of elements
double CMatrix::norm_inf(void) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::norm_inf(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  double *p_d = m_data;
  double maxs= 0.0;

  if (is_vector()) {
    // for vectors: max(abs(v(i)))
    for (int i = 0; i < m_size; i++) {
      double s = fabs(*p_d);
      p_d++;
      if (s > maxs) {
        maxs = s ;
      }
    }
  }
  else {
    // for matrices: largest row sum of A, max(sum(abs(A'))).
    for (int row = 0; row < m_nrows; row++) {

      double s = 0.0;

      for (int col = 0; col < m_ncols; col++) {
        s += fabs(*p_d);  // m_data[row,col]
        p_d++;
      }
      if (s > maxs) {
        maxs = s ;
      }
    }
  }
  return maxs;
}

//-------------------------------------------------------------------
// CMatrix::norm_frob() -- square root of sum of squares
double CMatrix::norm_frob(void) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::norm_frob(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  return sqrt(sum_square());
}

//-------------------------------------------------------------------
// CMatrix::norm() -- L_p norm of vector: sum(abs(A).^p)^(1/p), for any 1 <= p <= inf
double CMatrix::norm(double p) const
{
  MATRIX_ASSERT((!is_matrix()),
                "CMatrix::norm(): " << m_nrows << "x" << m_ncols << " matrix is not valid");

  MATRIX_ASSERT((!is_vector()), "CMatrix::norm(): " << m_nrows << "x" << m_ncols << " matrix is not a vector -- L_p norm is defined only for vectors (use norm_frob)");

  MATRIX_ASSERT((1 > p), "CMatrix::norm(p): " << "p = " << p << " -- p supposed to be 1 <= p <= inf");

  double sum = 0.0;
  double *p_d = m_data;

  for (int i = 0; i < m_size; i++) {
    double val = *p_d++;
    val = fabs(val);
    sum += ::pow(val, p);
  }

  return ::pow(sum, 1/p);
}

//-------------------------------------------------------------------
// CMatrix::trace() -- trace of a matrix, i.e. the sum of the diagonal elements.
double CMatrix::trace(void) const
{
  MATRIX_ASSERT((!is_square()), "CMatrix::trace(): " << "Can't find trace of " << m_nrows <<
                "x" << m_ncols << " matrix -- must be square");

  double *p_d = m_data;
  double sum = 0.0;

  for (int i = 0; i < m_nrows; i++) {
    sum += *p_d;  // m_data[i][i]
    p_d += (m_ncols + 1);
  }

  return sum;
}

//-------------------------------------------------------------------
// CMatrix::solve(CMatrix) -- solves a set of linear equations, Ax=b,
// using LU decomposition method.
// The right hand side vector is the class method argument.
// The matrix A must be square and regular.
CtmpMatrix CMatrix::solve(const CMatrix &b) const
{
  CtmpMatrix x(b);  // copy b to x

  CVectorOf<int> OriginalRow;
  OriginalRow.resize(m_nrows);

  CtmpMatrix lu(*this);
  bool dummy;
  int ZeroPivots;

  MATRIX_ASSERT((!is_square()), "CMatrix::solve(): " << "Can't solve the equation Ax = b -- given A " << m_nrows <<
                "x" << m_ncols << " matrix -- must be square");

  MATRIX_ASSERT(((m_nrows != b.nrows()) || (b.ncols() != 1)), "CMatrix::solve(): " << "Can't solve the equation Ax = b -- given b " << b.nrows() <<
                "x" << b.ncols() << " matrix -- must be a column vector with the same number of rows as matrix A");

  ZeroPivots = lu_decompose(lu, &OriginalRow[0], &dummy);
  MATRIX_ASSERT((ZeroPivots), "CMatrix::solve(): " << "Can't solve the equation Ax = b -- given A " << m_nrows << "x" <<
                m_ncols << " singular matrix -- must have LU decomposition"
                << ":" << endl << ((m_nrows <= 4) ? *this : 0));
  //);

  lu_solve(lu, &OriginalRow[0], x.data());

  return x;
}

//-------------------------------------------------------------------
// CMatrix::print() -- prints out a matrix to the output stream cout
void CMatrix::print(void) const
{
  int row, col;
  double *p = m_data;

  //ios_base::fmtflags ff;    // stream format flags
  //ff = cout.flags();        // remember the output stream format flags
  cout.flags();

  // change the output stream format flags
#ifdef MATRIX_OS_SCIENTIFIC
  cout.precision(MATRIX_OS_PRECISION);
#else // MATRIX_OS_SCIENTIFIC
  // We add one more digit to the MATRIX_OS_PRECISION whould have the same meaning as in the MATRIX_OS_SCIENTIFIC mode
  cout.precision(MATRIX_OS_PRECISION + 1);
#endif // MATRIX_OS_SCIENTIFIC
#ifdef MATRIX_OS_NEW_LINE
  cout << endl;
  ostream_indent(cout, MATRIX_OS_MATRIX_INDENT);
#endif // MATRIX_OS_NEW_LINE
  for (row = 0; row < m_nrows; row++) {

    // Do not indent the first row
    if (row != 0) {
      ostream_indent(cout, MATRIX_OS_MATRIX_INDENT);
    }

    for (col = 0; col < m_ncols; col++) {

      cout.width(MATRIX_OS_PRECISION + 5 + MATRIX_OS_NUM_SEP);

      cout << left;

#ifdef MATRIX_OS_SCIENTIFIC
      cout << scientific;
#endif // MATRIX_OS_SCIENTIFIC
#ifdef MATRIX_OS_SHOW_POINT
      cout << showpoint;
#else // MATRIX_OS_SHOW_POINT
      cout << noshowpoint;
#endif // MATRIX_OS_SHOW_POINT
      cout << (*p) << ' ';

      p++;
    }

    cout << endl;
  }
}

//-------------------------------------------------------------------
// CMatrix::operator << (ostream, CMatrix) -- prints out a matrix to a output
// stream like cout or a file.
ostream &operator << (ostream &os, const CMatrix &A)
{
  int row, col;
  double *p = A.m_data;

  ios_base::fmtflags ff;    // stream format flags
  ff = os.flags();          // remember the output stream format flags

  // change the output stream format flags
#ifdef MATRIX_OS_SCIENTIFIC
  os.precision(MATRIX_OS_PRECISION);
#else // MATRIX_OS_SCIENTIFIC
  // We add one more digit to the MATRIX_OS_PRECISION whould have the same meaning as in the MATRIX_OS_SCIENTIFIC mode
  os.precision(MATRIX_OS_PRECISION + 1);
#endif // MATRIX_OS_SCIENTIFIC
#ifdef MATRIX_OS_NEW_LINE
  os << endl;
  ostream_indent(os, MATRIX_OS_MATRIX_INDENT);
#endif // MATRIX_OS_NEW_LINE
  for (row = 0; row < A.m_nrows; row++) {

    // Do not indent the first row
    if (row != 0) {
      ostream_indent(os, MATRIX_OS_MATRIX_INDENT);
    }

    for (col = 0; col < A.m_ncols; col++) {

      os.width(MATRIX_OS_PRECISION + 5 + MATRIX_OS_NUM_SEP);

      os << left;

#ifdef MATRIX_OS_SCIENTIFIC
      os << scientific;
#endif // MATRIX_OS_SCIENTIFIC
#ifdef MATRIX_OS_SHOW_POINT
      os << showpoint;
#else // MATRIX_OS_SHOW_POINT
      os << noshowpoint;
#endif // MATRIX_OS_SHOW_POINT
      os << (*p) << ' ';

      p++;
    }

    os << endl;
  }

  os.flags(ff);             // restore the output stream format flags
  return os;
}


//-------------------------------------------------------------------
// Type of matrices

//-------------------------------------------------------------------
// CMatrix::is_zero() -- determines if the matrix is a zero matrix
bool CMatrix::is_zero(void) const
{
  if (!is_matrix()) {
    return false;
  }

  double *p_d = m_data;

  for (int i = 0; i < m_size; i++) {
    if (*p_d != 0.0) {
      return false;
    }
    p_d++;
  }
  return true;
}

//-------------------------------------------------------------------
// CMatrix::is_singular() -- determines if the matrix is singular
bool CMatrix::is_singular(void) const
{
  if (is_square()) {
    return (det() == 0.0);
  }
  return true;
}

//-------------------------------------------------------------------
// CMatrix::is_diagonal() -- determines if the matrix is diagonal
bool CMatrix::is_diagonal(void) const
{
  if (!is_square()) {
    return false;
  }

  double *p_d = m_data;

  for (int i = 0; i < m_nrows; i++) {
    for (int j = 0; j < m_ncols; j++) {
      if ((i != j) && (*p_d != 0.0)) {  // *p_d ~ m_data[i][j]
        return false;
      }
      p_d++;
    }
  }
  return true;
}

//-------------------------------------------------------------------
// CMatrix::is_scalar() -- determines if the matrix is scalar
bool CMatrix::is_scalar(void) const
{
  if (!is_diagonal()) {
    return false;
  }

  double *p_d = m_data;
  double v = *p_d;

  for (int i = 1; i < m_nrows; i++) {
    p_d += (m_nrows + 1);
    if (*p_d != v) { // *p_d ~ m_data[i][i]
      return false;
    }
  }
  return true;
}

//-------------------------------------------------------------------
// CMatrix::is_identity() -- determines if the matrix is a identity matrix
bool CMatrix::is_identity(void) const
{
  if (is_scalar() && (*m_data == 1.0)) {
    return true;
  }
  return false;
}

//-------------------------------------------------------------------
// CMatrix::is_symmetric() -- determines if the matrix is symmetric, i.e. satisfies A.t() == A
// e.g. [4  1;
//      [1 -2]
bool CMatrix::is_symmetric(void) const
{
  if (!is_square()) {
    return false;
  }

  double *p_d = m_data;

  for (int i = 0; i < m_nrows; i++) {
    for (int j = 0; j < i; j++) {
      if (*p_d != m_data[j*m_ncols+i]) {  // *p_d ~ m_data[i][j], m_data[j][i]
        return false;
      }
      p_d++;
    }
    p_d += (m_ncols - i);
  }
  return true;
}

//-------------------------------------------------------------------
// CMatrix::is_antisymmetric() -- determines if the matrix is antisymmetric (skew-symmetric),
// i.e. satisfies A.t() == -A
// e.g. [0 -1;
//      [1  0]
// Must have zeros on its diagonal.
bool CMatrix::is_antisymmetric(void) const
{
  if (!is_square()) {
    return false;
  }

  double *p_d = m_data;

  for (int i = 0; i < m_nrows; i++) {
    for (int j = 0; j <= i; j++) {
      if (*p_d != -m_data[j*m_ncols+i]) {  // *p_d ~ m_data[i][j], m_data[j][i]
        return false;
      }
      p_d++;
    }
    p_d += (m_ncols - i - 1);
  }
  return true;
}

//-------------------------------------------------------------------
// CMatrix::is_upper_triangular() -- determines if the matrix is upper triangular
// [a b c .. d;
//  0 e f .. g;
//  . . . .. .;
//  0 0 0    z]
bool CMatrix::is_upper_triangular(void) const
{
  if (!is_square()) {
    return false;
  }

  for (int i = 1; i < m_nrows; i++) {

    double *p_d = m_data + i*m_ncols;

    for (int j = 0; j < i; j++) {
      if (*p_d != 0.0) {  // m_data[i][j]
        return false;
      }
      p_d++;
    }
  }
  return true;
}

//-------------------------------------------------------------------
// CMatrix::is_lower_triangular() -- determines if the matrix is lower triangular
// [a 0 0 .. 0;
//  b c 0 .. 0;
//  . . . .. .;
//  d e f    z]
bool CMatrix::is_lower_triangular(void) const
{
  if (!is_square()) {
    return false;
  }

  for (int i = 0; i < m_nrows; i++) {
    for (int j = i + 1; j < m_ncols; j++) {
      if (m_data[i*m_ncols + j] != 0.0) {  // m_data[i][j]
        return false;
      }
    }
  }

  return true;
}

//-------------------------------------------------------------------
// CMatrix::version() -- returns a string with the version of the matrix library.
const string CMatrix::version(void)
{
  return c_CMatrixVersion + " compiled with " + COMPILER_STRING;
}

// ##### CMatrix class end #####


//-------------------------------------------------------------------
// ##### CtmpMatrix class #####

//-------------------------------------------------------------------
// CtmpMatrix(CMatrix, CMatrix) -- constructs a temporary matrix as
//                                 the product of two matrices
CtmpMatrix::CtmpMatrix(const CMatrix &m0, const CMatrix &m1) :
    CMatrix(m0.nrows(), m1.ncols())
{

  MATRIX_ASSERT((m1.nrows() != m0.ncols()),
                "CtmpMatrix::CtmpMatrix(): " << "Bad multiply -- " <<
                m0.nrows() << "x" << m0.ncols() << " matrix * " << m1.nrows() << "x" << m1.ncols() << " matrix");

  const int num_m0Cols_m1Rows = // both the number of rows in m1
    m0.ncols();         //   and the number of columns in

  //   m0 (they're equal)
  int m0Row;          // row in m0
  int m0Col_m1Row;    // both row in m1 and column in m0
  int m1Col;          // column in m1
  double *p_this_row;  // beginning of row m0Row in

  //   this->m_data
  double *p_this;    // pointer into this->m_data
  double *p_m0;      // pointer into m0->m_data
  double *p_m1;      // pointer into m1->m_data
  memset(m_data, 0, m_size * sizeof(*m_data));

  p_m0 = m0.data();
  p_this_row = m_data;
  for (m0Row = 0; m0Row < m_nrows; m0Row++) {
    p_m1 = m1.data();
    for (m0Col_m1Row = 0; m0Col_m1Row < num_m0Cols_m1Rows; m0Col_m1Row++) {
      p_this = p_this_row;
      for (m1Col = 0; m1Col < m_ncols; m1Col++) {
        *p_this++ += *p_m0 **p_m1++;
      }

      p_m0++;
    }

    p_this_row += m_ncols;
  }
}

//-------------------------------------------------------------------
// CtmpMatrix::add(CMatrix) -- adds a matrix to a CtmpMatrix (destructively)
CtmpMatrix &CtmpMatrix::add(const CMatrix &src)
{
  MATRIX_ASSERT(((m_nrows != src.nrows()) || (m_ncols != src.ncols())),
                "CtmpMatrix::add(): " << "Bad add -- "
                << m_nrows << "x" << m_ncols << " matrix + " << src.nrows() << "x" << src.ncols() << " matrix");

  double *p_this = m_data;
  double const *p_src = src.data();

  for (int i = 0; i < m_size; i++) {
    *p_this++ += *p_src++;
  }

  return *this;
}

//-------------------------------------------------------------------
// CtmpMatrix::add(double) -- adds a scalar value to a CtmpMatrix (destructively)
CtmpMatrix &CtmpMatrix::add(double num)
{
  double *p_this = m_data;

  for (int i = 0; i < m_size; i++) {
    *p_this++ += num;
  }

  return *this;
}

//-------------------------------------------------------------------
// CtmpMatrix::subtract(CMatrix) -- subtracts a matrix from a CtmpMatrix
//                                  (destructively)
CtmpMatrix &CtmpMatrix::subtract(const CMatrix &src)
{
  MATRIX_ASSERT(((m_nrows != src.nrows()) || (m_ncols != src.ncols())),
                "CtmpMatrix::subtract(): " << "Bad subtract -- "
                << m_nrows << "x" << m_ncols << " matrix - " << src.nrows() << "x" << src.ncols() << " matrix");

  double *p_this = m_data;
  double const *p_src = src.data();

  for (int i = 0; i < m_size; i++) {
    *p_this++ -= *p_src++;

  }

  return *this;
}

//-------------------------------------------------------------------
// CtmpMatrix::subtract_from(CMatrix) -- subtracts a CtmpMatrix from a matrix
//                                       (destructively)
CtmpMatrix &CtmpMatrix::subtract_from(const CMatrix &src)
{
  MATRIX_ASSERT(((m_nrows != src.nrows()) || (m_ncols != src.ncols())),
                "CtmpMatrix::subtract_from(): " << "Bad subtract -- "
                << m_nrows << "x" << m_ncols << " matrix - " << src.nrows() << "x" << src.ncols() << " matrix");

  double *p_this = m_data;
  double const *p_src = src.data();

  for (int i = 0; i < m_size; i++) {
    *p_this = *p_src - *p_this;
    p_this++;
    p_src++;
  }

  return *this;
}

//-------------------------------------------------------------------
// CtmpMatrix::multiply(double) -- multiplies a CtmpMatrix by a scalar value
//                                 (destructively)
CtmpMatrix &CtmpMatrix::multiply(double num)
{
  double *p = m_data;

  for (int i = 0; i < m_size; i++) {
    *p++ *= num;
  }

  return *this;
}

//-------------------------------------------------------------------
// CtmpMatrix::negate() -- changes sign of all matrix elements
CtmpMatrix &CtmpMatrix::negate(void)
{
  double *p = m_data;

  for (int i = 0; i < m_size; i++) {
    *p = -*p;
    p++;
  }

  return *this;
}

//-------------------------------------------------------------------
// CtmpMatrix::array_multiply(CMatrix) -- array multiplies a CtmpMatrix by a matrix
//                                        (destructively)
// Array multiplication is the element-by-element product of the matrices. (Matlab operator .*)
CtmpMatrix &CtmpMatrix::array_multiply(const CMatrix &src)
{
  MATRIX_ASSERT((!is_matrix() || (m_nrows != src.nrows()) || (m_ncols != src.ncols())),
                "CtmpMatrix::array_multiply(): " << "Bad array multiply -- "
                << m_nrows << "x" << m_ncols << " matrix element by element multiplied by "
                << src.nrows() << "x" << src.ncols() << " matrix");

  double *p_this = m_data;
  double const *p_src = src.data();

  for (int i = 0; i < m_size; i++) {
    *p_this++ *= *p_src++;
  }

  return *this;
}

//-------------------------------------------------------------------
// CtmpMatrix::steal_data() -- steels data from another CMatrix
double *CtmpMatrix::steal_data(void) const  /* not really const */
{
  double *data;

  MATRIX_ASSERT((m_data == 0),
                "*CtmpMatrix::steal_data(): " << "Trying to steal nonexistant CtmpMatrix data");

  data = m_data;
  ((CtmpMatrix *)this)->m_data = 0;

  return data;
}

//-------------------------------------------------------------------
// CtmpMatrix::operator() (int, int) -- returns a value for CtmpMatrix (row, col)
double &CtmpMatrix::operator () (int row, int col) const
{
  MATRIX_ASSERT(((0 > row || row >= m_nrows) || (0 > col || col >= m_ncols)),
                "&CtmpMatrix::operator(): " <<
                "Matrix index <" <<  row << "," << col << ">" <<
                " out of bounds in " << m_nrows << "x" << m_ncols << " matrix");

  return m_data[row * m_ncols + col];
}

//-------------------------------------------------------------------
// CtmpMatrix::concat_rows() -- concatenates horizontally (concatenates the rows)
//                              (destructively) |
CtmpMatrix &CtmpMatrix::concat_rows(const CMatrix &src)
{
  MATRIX_ASSERT((m_nrows != src.nrows()),
                "CtmpMatrix::concat_rows(): " << "Bad horizontal concatenation -- "
                << m_nrows << "x" << m_ncols << " matrix | "
                << src.nrows() << "x" << src.ncols() << " matrix");

  int ncols = m_ncols + src.ncols();
  CtmpMatrix tmp(m_nrows, ncols);

  double *p_this = m_data;
  double *p_tmp = tmp.data();
  double *p_src = src.data();

  for (int row = 0; row < m_nrows; row++) {
    for (int col = 0; col < ncols; col++) {
      if (col < m_ncols) {
        // We are within this matrix
        *p_tmp++ = *p_this++;
      }
      else {
        // We are within src matrix
        *p_tmp++ = *p_src++;
      }
    }
  }

  delete[] m_data;
  m_data = tmp.steal_data();
  m_ncols = ncols;
  m_size = m_nrows * ncols;

  return *this;
}

//-------------------------------------------------------------------
// CtmpMatrix::concat_columns() -- concatenates vertically (concatenates the rows)
//                                 (destructively) |
CtmpMatrix &CtmpMatrix::concat_columns(const CMatrix &src)
{
  MATRIX_ASSERT((m_ncols != src.ncols()),
                "CtmpMatrix::concat_columns(): " << "Bad vertical concatenation -- " << m_nrows << "x" <<
                m_ncols << " matrix & " << src.nrows() << "x" << src.ncols() << " matrix");

  int nrows = m_nrows + src.nrows();
  CtmpMatrix tmp(nrows, m_ncols);

  double *p_this = m_data;
  double *p_tmp = tmp.data();
  double *p_src = src.data();

  for (int row = 0; row < nrows; row++) {
    if (row < m_nrows) {
      // We are within this matrix
      for (int col = 0; col < m_ncols; col++) {
        *p_tmp++ = *p_this++;
      }
    }
    else {
      // We are within src matrix
      for (int col = 0; col < m_ncols; col++) {
        *p_tmp++ = *p_src++;
      }
    }
  }

  delete[] m_data;
  m_data = tmp.steal_data();
  m_nrows = nrows;
  m_size = nrows * m_ncols;

  return *this;
}

// ##### CtmpMatrix class end #####

//-------------------------------------------------------------------
// ##### Non-member functions #####

//-------------------------------------------------------------------
// array_multiply(CMatrix, CMatrix) -- array multiplication of two matrices.
// Array multiplication is the element-by-element product of the matrices. (Matlab operator .*)
CtmpMatrix array_multiply(const CMatrix &A, const CMatrix &B)
{
  CtmpMatrix tmp(A.array_multiply(B));

  return tmp;
}

//-------------------------------------------------------------------
// solve(CMatrix, CMatrix) -- solves a set of linear equations, Ax=b,
// using LU decomposition method.
// The matrix A and b is the first and the second argument of the function, respectivelly.
// The matrix A must be square and regular.
CtmpMatrix solve(const CMatrix &A, const CMatrix &b)
{
  CtmpMatrix tmp(A.solve(b));

  return tmp;
}

//-------------------------------------------------------------------
// dot_product(CMatrix, CMatrix) -- dot product of A and B interpreted as vectors
// dot protuct of A and B is sum(A.*B)
double dot_product(const CMatrix& A, const CMatrix& B)
{
  MATRIX_ASSERT((!A.is_matrix()), "CtmpMatrix::dot_product(): " << A.nrows() << "x" << A.ncols() << " matrix is not valid");
  MATRIX_ASSERT((!B.is_matrix()), "CtmpMatrix::dot_product(): " << B.nrows() << "x" << B.ncols() << " matrix is not valid");
  MATRIX_ASSERT((A.size() != B.size()),
                "CtmpMatrix::dot_product(): " << "Size of " << A.nrows() << "x" << A.ncols() << " matrix (" << A.size() << ") must be the same as " << B.nrows() << "x" <<
                B.ncols() << " matrix (" << B.size() << ") -- matrices are interpreted as vectors for dot product");

  double sum = 0.0;
  double *p_a = A.data();
  double *p_b = B.data();

  for (int i = 0; i < A.size(); i++) {
    sum += *p_a++ * *p_b++;
  }

  return sum;
}

//-------------------------------------------------------------------
// lu_solve(CMatrix, int *, double *)
//   -- solves the set of simultaneous equations using LU Decomposition
// This uses the LU Decomposition method described in
//   William H. Press, et al,
//   NUMERICAL RECIPES IN C
//   pp 40-47
//
// The matrix returned by lu_decompose, together with the array OriginalRow,
// describe a matrix as an LU Decomposition (see below for a description of how
// to reconstruct the matrix from the decomposition).
//
// This interprets the decomposed matrix as the coefficients in a set of
// simultaneous equations.  It is given the set of right-hand-sides of those
// equations, and solves for the variables.
//
// For example, if we have the set of equations:
//   4x + 2y = 3
//   7x + 8y = 21
// Then, we'd begin by calling lu_decompose() with the matrix
//   4 2
//   7 8
// Then, we'd pass the resulting matrix and OriginalRow array to lu_solve(),
// together with colBuf = { 3, 21 }. After lu_solve(), colBuf contains the values
// of x and y.
static void lu_solve(const CMatrix &lu, int *OriginalRow, double *colBuf)
{
  int firstNonZeroRow = -1;
  double sum = -1;
  int row;
  double *p_row;
  double *p;
  int i;

  // solve the L part of problem by forward substitution
  // this first loop solves up to the first row where the solution is not zero
  for (row = 0; row < lu.nrows(); row++) {
    i = OriginalRow[row];
    sum = colBuf[i];
    colBuf[i] = colBuf[row];

    colBuf[row] = sum;

    if (sum != 0) {
      // A non zero element was encountered, so from now on we will have to do sums in the loop above
      break;
    }
  }

  // this second loop solves the rest
  if (row < lu.nrows()) {
    firstNonZeroRow = row;
    p_row = lu.data() + row * lu.ncols();

    while (++row < lu.nrows()) {
      i = OriginalRow[row];
      sum = colBuf[i];
      colBuf[i] = colBuf[row];

      p_row += lu.ncols();
      for (i = firstNonZeroRow; i < row; i++) {
        sum -= p_row[i] * colBuf[i];
      }

      colBuf[row] = sum;
    }
  }
  else {
    p_row = lu.data() + (row - 1) * lu.ncols();
  }

  // solve the U part of the problem by backward substitution
  p = lu.data() + row * (lu.ncols() + 1);
  while (--row >= 0) {
    p -= lu.ncols() + 1;

    sum = colBuf[row];

    for (i = row + 1; i < lu.nrows(); i++) {
      sum -= p_row[i] * colBuf[i];
    }

    colBuf[row] = sum / *p;

    p_row -= lu.ncols();
  }
}

//-------------------------------------------------------------------
// lu_decompose(CMatrix, int *, bool *) -- compute the LU Decomposition of a matrix.
// It is Crout's method of LU decomposition of square matrix, with implicit partial
// pivoting.
// This uses slightly modified LU Decomposition method described in
//   William H. Press, et al,
//   NUMERICAL RECIPES IN C
//   pp 40-47
//
// To understand what this routine is doing, imagine constructing the following
// matrices after it has run:
//
//   L( r, c ) = M( r, c ) when r > c
//               0, when r < c
//               1, when r == c
//     (This is the lower left triangle of M, excluding M's diagonal. All the
//     diagonal elements are set to 1. All others are set to 0.)
//
//   U( r, c ) = M( r, c ) when r <= c
//               0, when r > c
//     (This is the upper right triangle of M, including M's diagonal.
//     All other elements are set to 0.)
//
//   P( r, c ) = 1, when OriginalRow[ r ] == c
//               0, when OriginalRow[ r ] != c
//     (This is the "pivot" matrix.  It records a shuffling of the rows that was
//     performed to reduce round-off and overflow errors.)
//
// The following property holds:
//
//   P * L * U == the original value of M, before lu_decompose()
//
// This is useful because it's easy to invert triangular matrices
// like L and U.

// Return value is number of zero pivots -> matrix is singular
static int lu_decompose(CMatrix &M, int *OriginalRow, bool *NumSwapsIsOdd)
{
  int row, col;
  double *p_row, *p_col;
  double *p0, *p1, *p2;
  const int nrows = M.nrows();
#define ncols nrows
  double sum;
  CVectorOf<double> scale;
  scale.resize(nrows);

  double biggest;
  int biggestRow = 0;
  double tmpDbl;
  CVectorOf<double> tmpDblArray;
  tmpDblArray.resize(ncols);

  int i;
  int zeros = 0;  // number of zero pivots
  *NumSwapsIsOdd = 0;

  // Find implicit scaling factors
  p0 = M.data();
  for (row = 0; row < nrows; row++) {

    // Loop over rows to get the implicit scaling information
    biggest = fabs(*p0++);
    for (col = 1; col < ncols; col++) {
      if ((tmpDbl = fabs(*p0++)) > biggest) {
        biggest = tmpDbl;
      }
    }

    // if biggest == 0, the matrix is singular, but it will be also detected later
    scale[row] = (biggest == 0.0) ? 0.0 : 1. / biggest; // Save the salling for each row
  }

  p_col = M.data();
  for (col = 0; col < ncols; col++) {

    // This is the loop over columns of Crout's method
    // Run down 'col'th column from top to diag, to form the elements of U
    p_row = M.data();
    p0 = p_col;
    for (row = 0; row < col; row++) {

      // This is equation (2.3.12) except i = j
      sum = *p0;
      p1 = p_row;
      p2 = p_col;
      for (i = 0; i < row; i++) {
        sum -= *p1++ **p2;
        p2 += ncols;
      }

      *p0 = sum;
      p0 += ncols;
      p_row += ncols;
    }

    // Run down 'col'th subdiag to form the residuals after the elimination of the first col-1
    // subdiags.  These residuals divided by the appropriate diagonal term will become the
    // multipliers in the elimination of the 'col'th subdiag. Find index of largest scaled term
    // in imax.
    biggest = 0.0;  // Initialize for the search for largest pivot element
    for (row = col; row < nrows; row++) {

      // This is i = j of equation (2.3.12) and i = j+1...N of equation (2.3.13)
      sum = *p0;
      p1 = p_row;
      p2 = p_col;
      for (i = 0; i < col; i++) {
        sum -= *p1++ **p2;
        p2 += ncols;
      }

      *p0 = sum;
      p0 += ncols;
      p_row += ncols;

      if ((tmpDbl = scale[row] * fabs(sum)) >= biggest) {

        // The figure of merit for the pivot better than the best so far
        biggest = tmpDbl;
        biggestRow = row;
      }
    }

    // Permute current row with biggestRow
    if (col != biggestRow) {

      // Do we need to interchange rows?
      // Yes, do so...
      p0 = &M(biggestRow, 0);
      p1 = &M(col, 0);
      memcpy(&tmpDblArray[0], p0, ncols * sizeof(tmpDblArray[0]));
      memcpy(p0, p1, ncols * sizeof(*M.data()));
      memcpy(p1, &tmpDblArray[0], ncols * sizeof(*M.data()));

      scale[biggestRow] = scale[col]; // interchange the scale factor
      *NumSwapsIsOdd ^= 1;            // change the number of row interchanges flag
    }

    OriginalRow[col] = biggestRow;

    // If diag term is not zero divide subdiag to form multipliers
    p0 = &M(col, col);

    if ( fabs(*p0) <= MATRIX_TINY) {
      // If the pivot element element is zero the matrix is singular (at least to the precision of the algorithm).
      // For some application on singlular matrices, it is desirable to substitute TINY for zero.
      zeros++;
    }
    else if (col != ncols - 1) {

      // Now, finally, divide by the pivot element
      tmpDbl = 1. / *p0;
      p1 = p_col + (col + 1) * ncols;
      for (row = col + 1; row < nrows; row++) {
        *p1 *= tmpDbl;
        p1 += ncols;
      }
    }

    p_col++;
  } // Go back for the next column in the reduction

  // if zeros is not zero, the matrix is singular, however, exceptions will be handled at caller functions.
  return zeros;

#undef ncols
}

// ##### Non-member functions end #####

#include <initializer_list>
#include <cmath>

template<typename T = float>
struct dynmatrix
{
private:
	const uint32_t _height;
	const uint32_t _width;

	T* m;

public:
	T* data() { return m; }
	const T* data() const { return m; }
	uint32_t height() const { return _height; }
	uint32_t width() const { return _width; }
	T& operator()(size_t row, size_t col)
	{
		assert(row < _height && col < _width);
		return m[row * _width + col];
	}
	const T& operator()(size_t row, size_t col) const
	{
		assert(row < _height && col < _width);
		return m[row * _width + col];
	}
	constexpr size_t size() { return _height * _width; }

	dynmatrix(uint32_t height, uint32_t width) : m(new T[height * width]), _height(height), _width(width) { assert(height > 0 && width > 0); }
	dynmatrix(uint32_t height, uint32_t width, std::initializer_list<T> c) : _height(height), _width(width), m(new T[height * width])
	{
		assert(height > 0 && width > 0);
		for (uint32_t i = 0; i < height; ++i)
		{
			for (uint32_t j = 0; j < width; ++j) { operator()(i, j) = *(c.begin() + i * width + j); }
		}
	}
	dynmatrix(dynmatrix&& rhs) : _height(rhs._height), _width(rhs._width), m(rhs.m) { rhs.m = nullptr; }
	dynmatrix& operator=(dynmatrix&& rhs)
	{
		const_cast<uint32_t&>(_height) = rhs._height;
		const_cast<uint32_t&>(_width) = rhs._width;

		m = rhs.m;
		rhs.m = nullptr;
	}
	dynmatrix(const dynmatrix& rhs) = delete;
	dynmatrix& operator=(const dynmatrix& rhs) = delete;
	bool isZero(const T& epsilon)
	{
		for (uint32_t i = 0; i < _height * _width; ++i)
		{
			if (std::abs(m[i]) > std::abs(epsilon)) { return false; }
		}
		return true;
	}

	bool isZero()
	{
		T zero = T(0);
		for (uint32_t i = 0; i < _height * _width; ++i)
		{
			if (m[i] != zero) { return false; }
		}
		return true;
	}

	bool isDiagonal()
	{
		T zero = T(0);
		for (uint32_t i = 0; i < _height; ++i)
		{
			for (uint32_t j = 0; j < _width; ++j)
			{
				if (i != j && m(i, j) != zero) { return false; }
			}
		}
		return true;
	}
	bool isTrueDiagonal()
	{
		T zero = T(0);
		for (uint32_t i = 0; i < _height; ++i)
		{
			for (uint32_t j = 0; j < _width; ++j)
			{
				if ((i != j && m(i, j) != zero) || (i == j && m(i, j) == zero)) { return false; }
			}
		}
		return true;
	}

	~dynmatrix() { delete[] m; }
};

template<typename T>
dynmatrix<bool> equals(const dynmatrix<T>& lhs, const dynmatrix<T>& rhs, T epsilon = T(0))
{
	assert(lhs._height == rhs._height&& lhs._width = rhs._width);
	dynmatrix<bool> retval(lhs.height(), lhs._width);
	for (uint32_t i = 0; i < lhs.height(); ++i)
	{
		for (uint32_t j = 0; j < lhs.width(); ++j) { retval(i, j) = (std::abs(lhs(i, j) - rhs(i, j)) <= epsilon); }
	}
	return retval;
}

template<typename T>
dynmatrix<bool> notEquals(const dynmatrix<T>& lhs, const dynmatrix<T>& rhs, T epsilon = T(0))
{
	assert(lhs.height() == rhs.height() && lhs.width() == rhs.width());
	dynmatrix<bool> retval(lhs.height(), lhs.width());
	for (uint32_t i = 0; i < lhs.height(); ++i)
	{
		for (uint32_t j = 0; j < lhs.width(); ++j)
		{ //
			retval(i, j) = (abs(lhs(i, j) - rhs(i, j)) > epsilon);
		}
	}
	return retval;
}

template<typename T>
dynmatrix<bool> operator==(const dynmatrix<T>& lhs, const dynmatrix<T>& rhs)
{
	assert(lhs._height == rhs._height&& lhs._width = rhs._width);
	dynmatrix<bool> retval(lhs.height(), lhs.width());
	for (uint32_t i = 0; i < lhs.height(); ++i)
	{
		for (uint32_t j = 0; j < lhs.width(); ++j) { retval(i, j) = (lhs(i, j) == rhs(i, j)); }
	}
	return retval;
}

template<typename T>
dynmatrix<bool> operator!=(const dynmatrix<T>& lhs, const dynmatrix<T>& rhs)
{
	assert(lhs._height == rhs._height&& lhs._width = rhs._width);
	dynmatrix<bool> retval(lhs.height(), lhs.width());

	for (uint32_t i = 0; i < lhs.height(); ++i)
	{
		for (uint32_t j = 0; j < lhs.width(); ++j) { retval(i, j) = (lhs(i, j) != rhs(i, j)); }
	}
	return retval;
}

template<typename T = float>
std::string to_string(const dynmatrix<T>& m)
{
	std::stringstream ss;
	for (uint32_t i = 0; i < m.height(); ++i)
	{
		for (uint32_t j = 0; j < m.width(); ++j) { ss << m(i, j) << " "; }
		ss << std::endl;
	}
	return ss.str();
}

// Print the supported command line parameters to standard out
template<typename T>
void printmatrix(dynmatrix<T> m)
{
	std::cout << to_string(m);
}

template<typename T = float>
dynmatrix<T> matmul_transposed_helper(const dynmatrix<T>& m1, const dynmatrix<T>& m2)
{
	assert(m1.width() == m2.width());
	dynmatrix<T> retval(m1.height(), m2.height());

	const uint32_t bigpart = (uint32_t)m1.width() & ~3u;

	for (uint32_t i = 0; i < m1.height(); ++i)
	{
		for (uint32_t j = 0; j < m2.height(); ++j)
		{
			T sum = T();
			uint32_t k = 0;

			for (; k < bigpart; k += 8)
			{
				sum += m1(i, k) * m2(j, k);
				sum += m1(i, k + 1) * m2(j, k + 1);
				sum += m1(i, k + 2) * m2(j, k + 2);
				sum += m1(i, k + 3) * m2(j, k + 3);
				sum += m1(i, k + 4) * m2(j, k + 4);
				sum += m1(i, k + 5) * m2(j, k + 5);
				sum += m1(i, k + 6) * m2(j, k + 6);
				sum += m1(i, k + 7) * m2(j, k + 7);
			}
			// k = leftover
			for (; k < m1.width(); ++k) // j
			{ sum += m1(i, k) * m2(j, k); }

			retval(i, j) = sum;
		}
	}

	return retval;
}

template<typename T = float>
dynmatrix<T> transpose(const dynmatrix<T>& m1)
{
	dynmatrix<T> ret(m1.width(), m1.height());
	for (uint32_t i = 0; i < m1.height(); ++i)
	{
		for (uint32_t j = 0; j < m1.width(); ++j) { ret(j, i) = m1(i, j); }
	}
	return ret;
}

template<typename T = float>
dynmatrix<T> operator*(const dynmatrix<T>& m1, const dynmatrix<T>& m2)
{
	return matmul_transposed_helper(m1, transpose(m2));
}

std::string operator*(const std::string& a, const std::string& b) { return std::string(a + "*" + b); }
template<int N, int M, typename T = float>
struct matrix
{
private:
	T* _m;

public:
	enum
	{
		height = N,
		width = M,
	};
	T& operator()(size_t row, size_t col) { return _m[row * width + col]; }
	const T& operator()(size_t row, size_t col) const { return _m[row * width + col]; }
	constexpr size_t size() { return N * M; }

	matrix() : _m(new T[height * width]) {}
	matrix(std::initializer_list<T> c) : _m(new T[height * width])
	{
		for (uint32_t i = 0; i < N; ++i)
		{
			for (uint32_t j = 0; j < M; ++j) { operator()(i, j) = *(c.begin() + i * width + j); }
		}
	}
	matrix(matrix&& rhs) : _m(rhs._m) { rhs._m = nullptr; }
	matrix& operator=(matrix&& rhs)
	{
		_m = rhs._m;
		rhs._m = nullptr;
	}
	matrix(const matrix& rhs) = delete;
	matrix& operator=(const matrix& rhs) = delete;
	bool isZero(const T& epsilon)
	{
		for (uint32_t i = 0; i < height * width; ++i)
		{
			if (abs(_m[i]) > abs(epsilon)) { return false; }
		}
		return true;
	}

	bool isZero()
	{
		T zero = T(0);
		for (uint32_t i = 0; i < height * width; ++i)
		{
			if (_m[i] != zero) { return false; }
		}
		return true;
	}

	bool isDiagonal()
	{
		T zero = T(0);
		for (uint32_t i = 0; i < height; ++i)
		{
			for (uint32_t j = 0; j < width; ++j)
			{
				if (i != j && _m(i, j) != zero) { return false; }
			}
		}
		return true;
	}
	bool isTrueDiagonal()
	{
		T zero = T(0);
		for (uint32_t i = 0; i < height; ++i)
		{
			for (uint32_t j = 0; j < width; ++j)
			{
				if ((i != j && _m(i, j) != zero) || (i == j && _m(i, j) == zero)) { return false; }
			}
		}
		return true;
	}

	T* data() { return _m; }
	const T* data() const { return _m; }

	~matrix() { delete[] _m; }
};

template<int M, int N, typename T>
matrix<M, N, bool> equals(const matrix<M, N, T>& lhs, const matrix<M, N, T>& rhs, T epsilon = T(0))
{
	matrix<M, N, bool> retval;
	for (uint32_t i = 0; i < M; ++i)
	{
		for (uint32_t j = 0; j < N; ++j) { retval(i, j) = (abs(lhs(i, j) - rhs(i, j)) <= epsilon); }
	}
	return retval;
}

template<int M, int N, typename T>
matrix<M, N, bool> notEquals(const matrix<M, N, T>& lhs, const matrix<M, N, T>& rhs, T epsilon = T(0))
{
	matrix<M, N, bool> retval;
	for (uint32_t i = 0; i < M; ++i)
	{
		for (uint32_t j = 0; j < N; ++j)
		{ //
			retval(i, j) = (abs(lhs(i, j) - rhs(i, j)) > epsilon);
		}
	}
	return retval;
}

template<int M, int N, typename T>
matrix<M, N, bool> operator==(const matrix<M, N, T>& lhs, const matrix<M, N, T>& rhs)
{
	matrix<M, N, bool> retval;
	for (uint32_t i = 0; i < M; ++i)
	{
		for (uint32_t j = 0; j < N; ++j) { retval(i, j) = (lhs(i, j) == rhs(i, j)); }
	}
	return retval;
}

template<int M, int N, typename T>
matrix<M, N, bool> operator!=(const matrix<M, N, T>& lhs, const matrix<M, N, T>& rhs)
{
	matrix<M, N, bool> retval;
	for (uint32_t i = 0; i < M; ++i)
	{
		for (uint32_t j = 0; j < N; ++j) { retval(i, j) = (lhs(i, j) != rhs(i, j)); }
	}
	return retval;
}

template<int M, int N, typename T = float>
std::string to_string(const matrix<M, N, T>& m)
{
	std::stringstream ss;
	for (uint32_t i = 0; i < m.height; ++i)
	{
		for (uint32_t j = 0; j < m.width; ++j) { ss << m(i, j) << " "; }
		ss << std::endl;
	}
	return ss.str();
}

// Print the supported command line parameters to standard out
template<int M, int N, typename T>
void printmatrix(matrix<M, N, T> m)
{
	std::cout << to_string(m);
}

template<int M, int N, int P, typename T = float>
matrix<M, P, T> matmul_naive(const matrix<M, N, T>& m1, const matrix<N, P, T>& m2)
{
	matrix<M, P, T> retval;

	for (uint32_t i = 0; i < M; ++i)
	{
		for (uint32_t j = 0; j < P; ++j)
		{
			T sum = T();
			for (uint32_t k = 0; k < N; ++k) { sum += m1(i, k) * m2(k, j); }
			retval(i, j) = sum;
		}
	}
	return retval;
}

template<int M, int N, int P, typename T = float>
matrix<M, P, T> matmul_transposed(const matrix<M, N, T>& m1, const matrix<P, N, T>& m2)
{
	matrix<M, P, T> retval;

	const uint32_t bigpart = (uint32_t)N & !3u;
	const uint32_t leftover = (uint32_t)N & 3u;

	for (uint32_t i = 0; i < M; ++i)
	{
		for (uint32_t j = 0; j < P; ++j)
		{
			T sum = T();
			int k = 0;

			for (; k < bigpart; k += 8)
			{
				sum += m1(i, k) * m2(j, k);
				sum += m1(i, k + 1) * m2(j, k + 1);
				sum += m1(i, k + 2) * m2(j, k + 2);
				sum += m1(i, k + 3) * m2(j, k + 3);
				sum += m1(i, k + 4) * m2(j, k + 4);
				sum += m1(i, k + 5) * m2(j, k + 5);
				sum += m1(i, k + 6) * m2(j, k + 6);
				sum += m1(i, k + 7) * m2(j, k + 7);
			}
			// k = leftover
			for (; k < N; ++k) // j
			{ sum += m1(i, k) * m2(j, k); }

			retval(i, j) = sum;
		}
	}

	for (uint32_t i = 0; i < M; ++i)
	{
		for (uint32_t j = 0; j < P; ++j)
		{
			T sum = T();
			for (uint32_t k = 0; k < N; ++k) { sum += m1(i, k) * m2(j, k); }
			retval(i, j) = sum;
		}
	}
	return retval;
}
template<int M, int N, int P, typename T = float>
matrix<M, P, T> operator*(const matrix<M, N, T>& m1, const matrix<N, P, T>& m2)
{
	return matmul_transposed(m1, transpose(m2));
}

template<int M, int N, typename T = float>
matrix<N, M, T> transpose(const matrix<M, N, T>& m1)
{
	matrix<N, M, T> ret;
	for (uint32_t i = 0; i < M; ++i)
	{
		for (uint32_t j = 0; j < N; ++j) { ret(j, i) = m1(i, j); }
	}
	return ret;
}

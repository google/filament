/*!
\brief Includes required GLM library components and defines the rest of the information necessary for PowerVR Math
needs.
\file PVRCore/math/MathUtils.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include <PVRCore/types/Types.h>
#include <cmath>
#include <cstdint>
#include "PVRCore/glm.h"

namespace pvr {
namespace math {

/// <summary>Calculate the Greatest Common Divisor of two numbers (the larger number that,
/// if used to divide either value, has a remainder of zero. Order is irrelevant</summary>
/// <typeparam name="T">The type of the values. Must have equality, assignment and modulo
/// defined</typeparam>
/// <param name="lhs">One of the input values</param>
/// <param name="rhs">The other input values</param>
/// <returns>The GCD. If the numbers are "coprime" (have no common divisor exept 1),
/// the GCD is 1.</returns>
template<typename T>
inline T gcd(T lhs, T rhs)
{
	T tmprhs;
	while (true)
	{
		if (rhs == 0) { return lhs; }
		tmprhs = rhs;
		rhs = lhs % rhs;
		lhs = tmprhs;
	}
}

/// <summary>Calculate the Least Common Multiple of two numbers (the smaller integer that
/// is a factor of both numbers). Order is irrelevant. If either of the numbers is 0, will
/// return 0</summary>
/// <typeparam name="T">The type of the values. Must have equality, assignment multiplication
/// and either modulo or a gcd function defined</typeparam>
/// <param name="lhs">One of the input values</param>
/// <param name="rhs">The other input values</param>
/// <returns>The LCM. If the inputs don't have any common factors (except 1), the LCM is
/// equal to lhs * rhs. If either input is 0, returns 0.</returns>
template<typename T>
inline T lcm(T lhs, T rhs)
{
	return (lhs / gcd(lhs, rhs)) * rhs;
}

/// <summary>Calculate the Least Common Multiple of two numbers (the smaller integer that
/// is a multiple of both numbers), but discards 0: If either number is 0, will return the
/// other number</summary>
/// <typeparam name="T">The type of the values. Must have equality, assignment multiplication
/// and either modulo or a gcd function defined</typeparam>
/// <param name="lhs">One of the input values</param>
/// <param name="rhs">The other input values</param>
/// <returns>The LCM. If the numbers don't have any common factors (except 1), the LCM is
/// equal to lhs * rhs. If either input is 0, returns the other</returns>
template<typename T>
inline T lcm_with_max(T lhs, T rhs)
{
	T strict = (lhs / gcd(lhs, rhs)) * rhs;
	if (strict == 0) { strict = std::max(lhs, rhs); }
	return strict;
}

/// <summary>Return the smallest power of two that is greater than or equal to the provided value.</summary>
/// <param name="iVal">An integer value.</param>
/// <returns>The smallest PoT that is greater or equal to iVal</returns>
inline int32_t makePowerOfTwoHigh(int32_t iVal)
{
	int iTmp = 1;
	do
	{
		iTmp <<= 1;
	} while (iTmp < iVal);
	return iTmp;
}

/// <summary>Return the smallest power of two that is less than or equal to the provided value.</summary>
/// <param name="iVal">An integer value.</param>
/// <returns>The smallest PoT that is less or equal to iVal</returns>
inline int32_t makePowerOfTwoLow(int32_t iVal)
{
	int iTmp = 1;
	do
	{
		iTmp <<= 1;
	} while (iTmp < iVal);
	return iTmp;
	iTmp >>= 1;
}

/// <summary>Convert a normalized device coordinate (-1..1) to a number of pixels from the start (left or top)</summary>
/// <param name="ndc">The normalised coordinate along the direction in question (same direction as screenSize)</param>
/// <param name="screenSize">The size of the screen along the direction in question (same as ndc)</param>
/// <returns>Pixel coordinates from normalized device coordinates</returns>
inline int32_t ndcToPixel(float ndc, int32_t screenSize) { return static_cast<int32_t>(ndc * screenSize * .5f + screenSize * .5f); }

/// <summary>Convert a number of pixels (left or top) to a normalized device coordinate (-1..1)</summary>
/// <param name="pixelCoord">The pixel coordinate (number of pixels) along the direction in question (same
/// direction as screenSize)</param>
/// <param name="screenSize">The size of the screen along the direction in question (same as pixelCoord)</param>
/// <returns>Normalized device coordinates (number in the 0..1 range)</returns>
inline float pixelToNdc(int32_t pixelCoord, int32_t screenSize) { return (2.f / screenSize) * (pixelCoord - screenSize * .5f); }

/// <summary>Performs quadratic interpolation between two points, beginning with a faster rate and slowing down.</summary>
/// <param name="start">The starting point.</param>
/// <param name="end">The end point</param>
/// <param name="factor">Current LINEAR interpolation factor, from 0..1</param>0
/// <returns> For <paramRef name="factor"/>=0, returns <paramRef name="start"/>. For <paramRef name="factor"/>=1, returns <paramRef name="end"/>.
/// Closer to 0, the rate of change is faster, closer to 1 slower.</param>
inline float quadraticEaseOut(float start, float end, float factor)
{
	float fTInv = 1.0f - factor;
	return ((start - end) * fTInv * fTInv) + end;
}

/// <summary>Performs quadratic interpolation between two points, beginning with a slow rate and speeding up.</summary>
/// <param name="start">The starting point.</param>
/// <param name="end">The end point</param>
/// <param name="factor">Interpolation factor. At 0, returns start. At 1, returns end. Closer to 0, the rate of change is
/// slower, closer to 1 faster.</param>
/// <returns>The modified value to use, quadratically interpolated between start and end with factor factor.</returns>
inline float quadraticEaseIn(float start, float end, float factor) { return ((end - start) * factor * factor) + start; }

/// <summary>Performs line -to - plane intersection</summary>
/// <typeparam name="genType">A glm:: vector type. Otherwise, a type with the following
/// operations defined: A typename member value_type (type of scalar), +/- (vector add/mul), / (divide
/// by scalar), and a dot() function in either the global or glm:: namespace</typeparam>
/// <param name="origin">The start point of the line</param>
/// <param name="dir">The (positive) direction of the line</param>
/// <param name="planeOrigin">Any point on the plane</param>
/// <param name="planeNormal">The normal of the plane</param>
/// <param name="intersectionDistance">Output parameter: If an intersection happens, this parameter
/// will contain the signed distance from <paramRef name="origin"> towards <paramRef name="dir"> of
/// the intersection point.</param>
/// <param name="epsilon">For any comparison calculations, any value smaller than that will be considered
/// zero (otherwise, if two numbers difference is smaller than this, they are considered equal)</param>
/// <returns>True if the line and plane intersect, otherwise false</returns>
template<typename genType>
bool intersectLinePlane(genType const& origin, genType const& dir, genType const& planeOrigin, genType const& planeNormal, typename genType::value_type& intersectionDistance,
	typename genType::value_type epsilon = std::numeric_limits<typename genType::value_type>::epsilon())
{
	using namespace glm;
	typename genType::value_type d = dot(dir, planeNormal);

	if (glm::abs(d) > epsilon)
	{
		intersectionDistance = dot(planeOrigin - origin, planeNormal) / d;
		return true;
	}
	return false;
}

/// <summary>Get a vector that is perpendicular to another vector</summary>
/// <typeparam name="Vec2">A vector with two components that can be accessed through .x and .y</typeparam>
/// <param name="aVector">A vector</param>
/// <returns>A vector that is perpendicular to <paramRef name="aVector"/></returns>
template<typename Vec2>
Vec2 getPerpendicular(Vec2 const& aVector)
{
	return Vec2(aVector.y, -aVector.x);
}

/// <summary>Calculated a tilted perspective projection matrix</summary>
/// <param name="api">The graphics API for which this matrix will be created. It is used for the
/// Framebuffer coordinate convention.</param>
/// <param name="fovy">The field of vision in the y axis</param>
/// <param name="aspect">The aspect of the viewport</param>
/// <param name="near1">The near clipping plane distance (trailing 1 to avoid win32 keyword)</param>
/// <param name="far1">The far clipping plane distance (trailing 1 to avoid win32 keyword)</param>
/// <param name="rotate">Angle of tilt (rotation around the z axis), in radians</param>
/// <returns>A projection matrix for the specified parameters, tilted by rotate</returns>
inline glm::mat4 perspective(Api api, float fovy, float aspect, float near1, float far1, float rotate = .0f)
{
	glm::mat4 mat = glm::perspective(fovy, aspect, near1, far1);
	if (api == Api::Vulkan)
	{
		mat[1] *= -1.f; // negate the y axis's y component, because vulkan coordinate system is +y down.
		// We would normally negate the entire row, but the rest of the components are zero.
	}
	return (rotate == 0.f ? mat : glm::rotate(rotate, glm::vec3(0.0f, 0.0f, 1.0f)) * mat);
}

/// <summary>Calculated a tilted perspective projection matrix</summary>
/// <param name="fovy">The field of vision in the y axis</param>
/// <param name="width">The width of the viewport</param>
/// <param name="height">The height of the viewport</param>
/// <param name="near1">The near clipping plane distance</param>
/// <param name="far1">The far clipping plane distance</param>
/// <param name="rotate">Angle of tilt (rotation around the z axis), in radians</param>
/// <param name="api">The graphics API for which this matrix will be created. It is used for things such as the
/// Framebuffer coordinate conventions.</param>
/// <returns>A projection matrix for the specified parameters, tilted by rotate</returns>
inline glm::mat4 perspectiveFov(Api api, float fovy, float width, float height, float near1, float far1, float rotate = .0f)
{
	return perspective(api, fovy, width / height, near1, far1, rotate);
}

/// <summary>Calculated an orthographic projection tilted projection matrix</summary>
/// <param name="left">The x coordinate of the left clipping plane</param>
/// <param name="right">The x coordinate of the right clipping plane</param>
/// <param name="bottom">The y coordinate of the bottom clipping plane</param>
/// <param name="top">The y coordinate of the bottom clipping plane</param>
/// <param name="rotate">Angle of tilt (rotation around the z axis), in radians</param>
/// <param name="api">The graphics API for which this matrix will be created. It is used for things such as the
/// Framebuffer coordinate conventions.</param>
/// <returns>An orthographic projection matrix for the specified parameters, tilted by rotate</returns>
inline glm::mat4 ortho(Api api, float left, float right, float bottom, float top, float rotate = 0.0f)
{
	if (api == pvr::Api::Vulkan)
	{
		std::swap(bottom, top); // Vulkan origin y is top
	}
	glm::mat4 proj = glm::ortho<float>(left, right, bottom, top);
	return (rotate == 0.0f ? proj : glm::rotate(rotate, glm::vec3(0.0f, 0.0f, 1.0f)) * proj);
}
} // namespace math
} // namespace pvr

namespace {
inline void addCoefficients(const uint64_t pascalSum, const size_t halfCoefficientsMinusOne, const size_t numCoefficients, const std::vector<uint64_t>& coefficients,
	std::vector<double>& weights, std::vector<double>& offsets)
{
	// Handle cases where the coefficients vector contains coefficients which are to be truncated. i.e. we get rid of the outer set of coefficients
	size_t unneededCoefficients = (coefficients.size() - static_cast<size_t>(numCoefficients)) / 2;
	size_t i = unneededCoefficients;
	for (; i < (size_t)halfCoefficientsMinusOne; i++)
	{
		weights.emplace_back(static_cast<double>(coefficients[i]) / pascalSum);
		double offset = static_cast<double>(i) - static_cast<double>(halfCoefficientsMinusOne);
		offsets.emplace_back(offset);
	}

	weights.emplace_back(static_cast<double>(coefficients[i]) / pascalSum);
	offsets.emplace_back(static_cast<double>(0));

	for (i = static_cast<size_t>(halfCoefficientsMinusOne) + 1; i < static_cast<size_t>(numCoefficients) + unneededCoefficients; i++)
	{
		weights.emplace_back(static_cast<double>(coefficients[i]) / pascalSum);
		offsets.emplace_back(static_cast<double>(i - halfCoefficientsMinusOne));
	}
}
} // namespace

namespace pvr {
namespace math {

/// <summary>Generate the Pascal Triangle row for the given row and store and return its pascal triangle coefficinets along with the sum of the coefficients for the given
/// row.</summary>
/// <param name="row">The row of the Pascal to generate coefficients with the first row being the 0th.</param>
/// <param name="pascalCoefficients">An empty vector capable of storing the pascal coefficients for the given row of the Pascal Triangle.</param>
/// <returns>Returns the sum of the coefficients for the given row of the Pascal Triangle.</returns>
inline uint64_t generatePascalTriangleRow(size_t row, std::vector<uint64_t>& pascalCoefficients)
{
	// Each entry of any given row of the Pascal Triangle is constructed by adding the number above and to the left with the number above and to the right.
	// Entries which fall outside of the Pascal Triangle are treated as having a 0 value. The first row consists of a single entry with a value of 1.
	// The following shows the first 4 rows of the Pascal Triangle
	//        1     row 0 ... sum = 1
	//       1 1    row 1 ... sum = 2
	//      1 2 1   row 2 ... sum = 4
	//     1 3 3 1  row 3 ... sum = 8
	pascalCoefficients.emplace_back(1u);
	uint64_t sum = pascalCoefficients.back();
	for (size_t i = 0; i < row; i++)
	{
		uint64_t val = pascalCoefficients[i] * (row - i) / (i + 1u);
		pascalCoefficients.emplace_back(val);
		sum += pascalCoefficients.back();
	}

	return sum;
}

/// <summary>Adjust a given set of Gaussian weights and offset to be "linearly samplable" meaning we can achieve the same Gaussian Blur using fewer texture samples using Linear
/// Sampling than would be required if not using Linear Sampling when sampling using the offsets</summary>
/// <param name="halfCoefficientsMinusOne">The row of the Pascal to generate coefficients with the first row being the 0th.</param>
/// <param name="weights">A vector containing the Gaussian weights for the given kernel size.</param>
/// <param name="offsets">A vector containing the Gaussian offsets for the given kernel.</param>
inline void adjustOffsetsAndWeightsForLinearSampling(const size_t halfCoefficientsMinusOne, std::vector<double>& weights, std::vector<double>& offsets)
{
	std::vector<double> adjustedWeights;
	std::vector<double> adjustedOffsets;

	// if kernel size minus 1 is divisible by 2 then we have a central sample with offset 0
	if (halfCoefficientsMinusOne % 2u == 0u)
	{
		size_t i = 0u;
		// Ensure not zero
		for (; (halfCoefficientsMinusOne > 0) && (i < halfCoefficientsMinusOne - 1u); i += 2u)
		{
			adjustedWeights.emplace_back(weights[i] + weights[i + 1]);
			double adjustedOffset = ((offsets[i] * weights[i]) + (offsets[i + 1u] * weights[i + 1u])) / adjustedWeights.back();
			adjustedOffsets.emplace_back(adjustedOffset);
		}

		adjustedWeights.emplace_back(weights[halfCoefficientsMinusOne]);
		adjustedOffsets.emplace_back(0.0f);

		for (i = halfCoefficientsMinusOne + 1u; i < static_cast<uint64_t>(offsets.size()); i += 2u)
		{
			adjustedWeights.emplace_back(weights[i] + weights[i + 1u]);
			double adjustedOffset = ((offsets[i] * weights[i]) + (offsets[i + 1u] * weights[i + 1u])) / adjustedWeights.back();
			adjustedOffsets.emplace_back(adjustedOffset);
		}
	}
	else // otherwise we have to duplicate the central sample *but* this means we can handle 3x3 using 2x2 samples
	{
		size_t i = 0;
		for (; i < halfCoefficientsMinusOne; i += 2u)
		{
			double adjustedOffset = 0.0;
			if (i == halfCoefficientsMinusOne - 1u)
			{
				adjustedWeights.emplace_back(weights[i] + weights[i + 1u] * 0.5);
				adjustedOffset = ((offsets[i] * weights[i]) + (offsets[i + 1u] * weights[i + 1u] * 0.5)) / adjustedWeights.back();
			}
			else
			{
				adjustedWeights.emplace_back(weights[i] + weights[i + 1u]);
				adjustedOffset = ((offsets[i] * weights[i]) + (offsets[i + 1u] * weights[i + 1u])) / adjustedWeights.back();
			}
			adjustedOffsets.emplace_back(adjustedOffset);
		}

		for (i = halfCoefficientsMinusOne; i < offsets.size(); i += 2u)
		{
			double adjustedOffset = 0.0;
			if (i == halfCoefficientsMinusOne)
			{
				adjustedWeights.emplace_back(weights[i] * 0.5 + weights[i + 1u]);
				adjustedOffset = ((offsets[i] * weights[i] * 0.5) + (offsets[i + 1u] * weights[i + 1u])) / adjustedWeights.back();
			}
			else
			{
				adjustedWeights.emplace_back(weights[i] + weights[i + 1u]);
				adjustedOffset = ((offsets[i] * weights[i]) + (offsets[i + 1u] * weights[i + 1u])) / adjustedWeights.back();
			}
			adjustedOffsets.emplace_back(adjustedOffset);
		}
	}

	offsets.clear();
	weights.clear();

	for (size_t i = 0; i < adjustedWeights.size(); i++)
	{
		offsets.emplace_back(adjustedOffsets[i]);
		weights.emplace_back(adjustedWeights[i]);
	}
}

/// <summary>Generates a set of Gaussian weights and offsets based on the given configuration values. This function makes use of the Pascal Triangle for calculating the
/// Gaussian distribution. The Gaussian function is a distribution function of the normal distribution who's discrete equivalent is the binomial distribution for
/// which the Pascal Triangle models. The Pascal Triangle provides us with a convenient and efficient mechanism for calculating the Gaussian weights and offsets
/// required. Our method of generating Gaussian weights and offsets was inspired by http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/ .</summary>
/// <param name="kernelSize">The size of the kernel for which to generate Gaussian Weights and Gaussian Offsets. The size of the kernel is used to select the starting row of the
/// Pascal Triangle.</param>
/// <param name="truncateCoefficients">Can be used in combination with "minimumAcceptableCoefficient" to ignore coefficients from the
/// Pascal Triangle which are smaller than the given value and are therefore deemed to be negligible. If the starting row is found to have too few coefficients than are required
/// for the kernel size then the next row of the Pascal Triangle will be scanned for a list of coefficients deemed to be non-negligible and so on.</param>
/// <param name="useLinearSamplerOptimization">Specifies that the Gaussian Weights and Offsets returned will be modified prior to being returned so that they take advantage of
/// Linear Texture Sampling which can provide a simple means of reducing the number of texture samples.</param>
/// <param name="weights">The returned by reference list of Gaussian Weights calculated using the Pascal Triangle.</param>
/// <param name="offsets">The returned by reference list of Gaussian Offsets calculated using the Pascal Triangle.</param>
/// <param name="minimumAcceptableCoefficient">Only used when ignoreNegligibleCoefficients is true and specifies the minimum coefficient value which is deemed to be non-negligible
/// and therefore can be used as one of the returned weights. If a row does not contain enough coefficients deemed to be non-negligible then the next row will be checked for values
/// greater than the specified minimum acceptable coefficient. Take care when specifying larger values for the minimum acceptable coefficient along with larger kernel sizes as you
/// can run out of available precision quite quickly.</param>
/// <returns>Returns the sum of the coefficients for the given row of the Pascal Triangle.</returns>
inline void generateGaussianKernelWeightsAndOffsets(uint32_t kernelSize, bool truncateCoefficients, bool useLinearSamplerOptimization, std::vector<double>& weights,
	std::vector<double>& offsets, float minimumAcceptableCoefficient = 0.0001f)
{
	// Odd kernel sizes are a requirement
	assert((kernelSize % 2u) == 1u);

	// The starting row of the Pascal Triangle being used
	size_t pascalRow = kernelSize - 1u;

	// The number of coefficients minus 1 halved
	// This variable is used to get the index of the coefficient used for an offset of 0
	// (numCoefficients - 1) / 2
	size_t halfCoefficientsMinusOne = pascalRow / 2u;

	// stores the set of pascal coefficients
	std::vector<uint64_t> pascalCoefficients;

	// the number of coefficients skipped due to minimum coefficient checks
	size_t numCoefficientsSkipped = 0u;

	// The simple case
	if (!truncateCoefficients)
	{
		// Generate the Pascal Triangle row and calculate the sum of the row
		uint64_t pascalSum = generatePascalTriangleRow(pascalRow, pascalCoefficients);
		// Store the Pascal Triangle coefficients for the given row
		addCoefficients(pascalSum, halfCoefficientsMinusOne, pascalCoefficients.size(), pascalCoefficients, weights, offsets);
	}
	else
	// Ignoring negligible coefficients - we'll now attempt to find a row which provides enough coefficients for the given kernel size whilst not falling below the
	// given minimal coefficient value.
	{
		size_t currentRow = pascalRow;
		bool foundSuitableRow = false;

		// only accept rows where we have kernelSize coefficients larger than the min coefficient
		while (!foundSuitableRow)
		{
			// clear the offsets and weights
			offsets.clear();
			weights.clear();

			// clear the set of pascal triangle coefficients
			pascalCoefficients.clear();

			// get the pascal coefficients for the current row of the triangle
			uint64_t pascalSum = generatePascalTriangleRow(currentRow, pascalCoefficients);

			halfCoefficientsMinusOne = currentRow / 2u;

			// check how many of the coefficients are negligible in size and therefore should be ignored
			numCoefficientsSkipped = 0u;
			for (size_t i = halfCoefficientsMinusOne; i < pascalCoefficients.size(); i++)
			{
				double currentWeight = static_cast<double>(pascalCoefficients[i]) / pascalSum;
				if (currentWeight < minimumAcceptableCoefficient) { numCoefficientsSkipped++; }
			}

			// if there aren't enough coefficients left to fulfill the requirements for the kernel size then continue to the next row
			if ((halfCoefficientsMinusOne + 1u) - numCoefficientsSkipped < (pascalRow / 2u + 1u))
			{
				currentRow += 2u;
				foundSuitableRow = false;
				continue;
			}
			// if negligible coefficients are to be removed we must also update the overall sums used to give them weighting
			// otherwise repeated blurring will result in darkening of the image
			uint64_t adjustedPascalSum = pascalSum;
			// the extra coefficients which aren't needed which do match the conditions for non-negligibility but would result in us taking extra coefficients
			size_t unrequiredCoefficients = ((pascalCoefficients.size() - kernelSize - (numCoefficientsSkipped * 2u)) / 2u);
			for (size_t i = 0; i < numCoefficientsSkipped + unrequiredCoefficients; i++) { adjustedPascalSum -= 2u * pascalCoefficients[pascalCoefficients.size() - 1u - i]; }

			// add the non-negligible coefficients to the weights and offsets buffers
			size_t numCoefficients = pascalCoefficients.size() - (2u * (numCoefficientsSkipped + unrequiredCoefficients));
			addCoefficients(adjustedPascalSum, halfCoefficientsMinusOne, numCoefficients, pascalCoefficients, weights, offsets);
			halfCoefficientsMinusOne = static_cast<uint64_t>((offsets.size() - 1u) / 2u);
			foundSuitableRow = true;
		}
	}

	// If using the Linear Sampling optimisation then adjust the Gaussian weights and offsets accordingly
	if (useLinearSamplerOptimization) { adjustOffsetsAndWeightsForLinearSampling(halfCoefficientsMinusOne, weights, offsets); }
}

/// <summary>Constructs a scale rotate translation matrix.</summary>
/// <param name="scale">Scaling vector.</param>
/// <param name="rotate">A quaternion handling the rotation.</param>
/// <param name="translation">The translation vector.</param>
/// <returns>The resulting SRT matrix.</param>
inline glm::mat4 constructSRT(const glm::vec3& scale, const glm::quat& rotate, const glm::vec3& translation)
{
	return glm::translate(translation) * glm::toMat4(rotate) * glm::scale(scale);
}
} // namespace math
} // namespace pvr

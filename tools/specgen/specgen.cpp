/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * SPECGEN: Spectral Integration Matrix Generator for Real-Time Dispersion
 *
 * ================================================================================================
 * THEORETICAL BACKGROUND
 * ================================================================================================
 *
 * 1. SPECTRAL RENDERING IN RGB
 * ----------------------------
 * Real-time rendering typically operates in a tristimulus color space (like sRGB). However,
 * physical phenomena like dispersion (refraction splitting light by wavelength) are inherently
 * spectral. To simulate this in an RGB pipeline, we approximate the continuous spectral integration
 * using a finite sum of weighted samples.
 *
 * The fundamental equation for perceived color C is:
 *    C = Integral( L(lambda) * r(lambda) * d_lambda )
 * where:
 *    L(lambda) is the spectral radiance reaching the eye.
 *    r(lambda) is the sensor response function (e.g., CIE 1931 x, y, z matching functions).
 *
 * 2. BASIS TRANSFORMATION STRATEGY
 * --------------------------------
 * We assume the input light is defined in linear sRGB. To process it spectrally:
 *    a. Convert sRGB to CIE XYZ (D65 white point).
 *    b. Assume the spectral distribution of the light is a sum of impulses or narrow bands
 *       weighted by the XYZ components (or simplified directly from sRGB).
 *    c. Apply spectral effects (like wavelength-dependent refraction).
 *    d. Integrate back to XYZ using the CIE Color Matching Functions (CMFs).
 *    e. Convert final XYZ back to sRGB.
 *
 * This tool pre-calculates a set of matrices (Kn) that combine these steps.
 * For a set of N sample wavelengths {lambda_0, ..., lambda_N-1}, the final color is:
 *    C_final = Sum( Kn * C_input ) for n=0..N-1
 *
 * Each matrix Kn represents the contribution of the n-th spectral sample to the final image,
 * accounting for the conversion to/from XYZ and the spectral weight of that sample.
 *
 * Derivation of Kn:
 *    Kn = M_XYZ_to_sRGB * Diag(Wn) * M_sRGB_to_XYZ
 *
 * where Wn is the "spectral weight" vector (x, y, z) for wavelength lambda_n:
 *    Wn = CMF(lambda_n) * weight_n
 *
 * 3. NORMALIZATION (ENERGY CONSERVATION)
 * --------------------------------------
 * To ensure that a white input (1, 1, 1) results in a white output (1, 1, 1) when no dispersion
 * occurs (i.e., all samples land on the same pixel), we must normalize the weights.
 *
 * We require: Sum(Kn) = Identity
 *
 * This implies:
 *    Sum( M_XYZ_to_sRGB * Diag(Wn) * M_sRGB_to_XYZ ) = I
 *    M_XYZ_to_sRGB * Sum( Diag(Wn) ) * M_sRGB_to_XYZ = I
 *    Sum( Diag(Wn) ) = M_sRGB_to_XYZ * I * M_XYZ_to_sRGB
 *    Sum( Diag(Wn) ) = I  (since M * M^-1 = I)
 *
 * Therefore, we normalize Wn such that:
 *    Sum(Wn.x) = 1.0
 *    Sum(Wn.y) = 1.0
 *    Sum(Wn.z) = 1.0
 *
 * Note: We do NOT normalize to the D65 white point (0.95047, 1.0, 1.08883). The M_sRGB_to_XYZ
 * matrix already handles the conversion from linear sRGB (1, 1, 1) to D65 XYZ. If we normalized
 * Wn to D65, we would effectively be applying the white point twice, resulting in a tinted image.
 * By normalizing Sum(Wn) to (1, 1, 1), we ensure that the energy is conserved through the
 * spectral transformation pipeline.
 *
 * In practice, we calculate the raw sum of Wn from the quadrature weights and CMFs, then compute
 * a correction factor:
 *    Correction = 1.0 / Sum(Wn_raw)
 *    Wn_final = Wn_raw * Correction
 *
 * 4. DISPERSION AND IOR OFFSETS
 * -----------------------------
 * The Index of Refraction (IOR) varies with wavelength. We use the Abbe number (Vd) to parameterize
 * this variation relative to a base IOR (nD) at 589.3nm.
 *
 * Cauchy Dispersion Model:
 *    n(lambda) = A + B / lambda^2
 *
 * Using the definition of Abbe number Vd = (nD - 1) / (nF - nC), we can derive:
 *    n(lambda) = nD + ((nD - 1) / Vd) * Offset(lambda)
 *
 * Where Offset(lambda) is pre-calculated by this tool:
 *    Offset(lambda) = (1/lambda^2 - 1/lambda_D^2) / (1/lambda_F^2 - 1/lambda_C^2)
 *
 * This allows the shader to compute the specific IOR for each sample efficiently:
 *    float ior_n = baseIOR + dispersionFactor * offsets[n];
 *
 * ================================================================================================
 */

#include <math/mat3.h>
#include <math/vec3.h>

#include <getopt/getopt.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace filament::math;

// ------------------------------------------------------------------------------------------------
// Constants and Data
// ------------------------------------------------------------------------------------------------

static constexpr double PI = 3.14159265358979323846;

// Standard CIE 1931 2-degree Color Matching Functions (CMFs)
// Range: 380nm - 780nm in 5nm steps.
// These define the standard observer's chromatic response.
struct CMFEntry {
    double lambda;
    double x, y, z;
};

static const std::vector<CMFEntry> CIE_1931_2DEG = {
    { 380, 0.001368, 0.000039, 0.006450 }, { 385, 0.002236, 0.000064, 0.010550 },
    { 390, 0.004243, 0.000120, 0.020050 }, { 395, 0.007650, 0.000217, 0.036210 },
    { 400, 0.014310, 0.000396, 0.067850 }, { 405, 0.023190, 0.000640, 0.110200 },
    { 410, 0.043510, 0.001210, 0.207400 }, { 415, 0.077630, 0.002180, 0.371300 },
    { 420, 0.134380, 0.004000, 0.645600 }, { 425, 0.214770, 0.007300, 1.039050 },
    { 430, 0.283900, 0.011600, 1.385600 }, { 435, 0.328500, 0.016840, 1.622960 },
    { 440, 0.348280, 0.023000, 1.747060 }, { 445, 0.348060, 0.029800, 1.782600 },
    { 450, 0.336200, 0.038000, 1.772110 }, { 455, 0.318700, 0.048000, 1.744100 },
    { 460, 0.290800, 0.060000, 1.669200 }, { 465, 0.251100, 0.073900, 1.528100 },
    { 470, 0.195360, 0.090980, 1.287640 }, { 475, 0.142100, 0.112600, 1.041900 },
    { 480, 0.095640, 0.139020, 0.812950 }, { 485, 0.057950, 0.169300, 0.616200 },
    { 490, 0.032010, 0.208020, 0.465180 }, { 495, 0.014700, 0.258600, 0.353300 },
    { 500, 0.004900, 0.323000, 0.272000 }, { 505, 0.002400, 0.407300, 0.212300 },
    { 510, 0.009300, 0.503000, 0.158200 }, { 515, 0.029100, 0.608200, 0.111700 },
    { 520, 0.063270, 0.710000, 0.078250 }, { 525, 0.109600, 0.793200, 0.057250 },
    { 530, 0.165500, 0.862000, 0.042160 }, { 535, 0.225750, 0.914850, 0.029840 },
    { 540, 0.290400, 0.954000, 0.020300 }, { 545, 0.359700, 0.980300, 0.013400 },
    { 550, 0.433450, 0.994950, 0.008750 }, { 555, 0.512050, 1.000000, 0.005750 },
    { 560, 0.594500, 0.995000, 0.003900 }, { 565, 0.678000, 0.978600, 0.002750 },
    { 570, 0.762100, 0.952000, 0.002100 }, { 575, 0.842500, 0.915400, 0.001800 },
    { 580, 0.916300, 0.870000, 0.001650 }, { 585, 0.978600, 0.816300, 0.001400 },
    { 590, 1.026300, 0.757000, 0.001100 }, { 595, 1.056700, 0.694900, 0.001000 },
    { 600, 1.062200, 0.631000, 0.000800 }, { 605, 1.045600, 0.566800, 0.000600 },
    { 610, 1.002600, 0.503000, 0.000340 }, { 615, 0.938400, 0.441200, 0.000240 },
    { 620, 0.854450, 0.381000, 0.000190 }, { 625, 0.751400, 0.321000, 0.000100 },
    { 630, 0.642400, 0.265000, 0.000050 }, { 635, 0.541900, 0.217000, 0.000030 },
    { 640, 0.447900, 0.175000, 0.000020 }, { 645, 0.360800, 0.138200, 0.000010 },
    { 650, 0.283500, 0.107000, 0.000000 }, { 655, 0.218700, 0.081600, 0.000000 },
    { 660, 0.164900, 0.061000, 0.000000 }, { 665, 0.121200, 0.044580, 0.000000 },
    { 670, 0.087400, 0.032000, 0.000000 }, { 675, 0.063600, 0.023200, 0.000000 },
    { 680, 0.046770, 0.017000, 0.000000 }, { 685, 0.032900, 0.011920, 0.000000 },
    { 690, 0.022700, 0.008210, 0.000000 }, { 695, 0.015840, 0.005723, 0.000000 },
    { 700, 0.011359, 0.004102, 0.000000 }, { 705, 0.008111, 0.002929, 0.000000 },
    { 710, 0.005790, 0.002091, 0.000000 }, { 715, 0.004109, 0.001484, 0.000000 },
    { 720, 0.002899, 0.001047, 0.000000 }, { 725, 0.002049, 0.000740, 0.000000 },
    { 730, 0.001440, 0.000520, 0.000000 }, { 735, 0.001000, 0.000361, 0.000000 },
    { 740, 0.000690, 0.000249, 0.000000 }, { 745, 0.000476, 0.000172, 0.000000 },
    { 750, 0.000332, 0.000120, 0.000000 }, { 755, 0.000235, 0.000085, 0.000000 },
    { 760, 0.000166, 0.000060, 0.000000 }, { 765, 0.000117, 0.000042, 0.000000 },
    { 770, 0.000083, 0.000030, 0.000000 }, { 775, 0.000059, 0.000021, 0.000000 },
    { 780, 0.000042, 0.000015, 0.000000 }
};

// Reference Matrices
// sRGB to XYZ (D65)
// Row-major:
// 0.4124564, 0.3575761, 0.1804375
// 0.2126729, 0.7151522, 0.0721750
// 0.0193339, 0.1191920, 0.9503041
static constexpr mat3 M_sRGB_to_XYZ = mat3(
        0.4124564, 0.2126729, 0.0193339, // Col 0
        0.3575761, 0.7151522, 0.1191920, // Col 1
        0.1804375, 0.0721750, 0.9503041 // Col 2
        );

// XYZ to sRGB (D65)
// Row-major:
//  3.2404542, -1.5371385, -0.4985314
// -0.9692660,  1.8760108,  0.0415560
//  0.0556434, -0.2040259,  1.0572252
static constexpr mat3 M_XYZ_to_sRGB = mat3(
        3.2404542, -0.9692660, 0.0556434, // Col 0
        -1.5371385, 1.8760108, -0.2040259, // Col 1
        -0.4985314, 0.0415560, 1.0572252 // Col 2
        );

// ------------------------------------------------------------------------------------------------
// Helper Classes
// ------------------------------------------------------------------------------------------------

struct Sample {
    double lambda;
    double weight;
};

class CMFProvider {
public:
    virtual ~CMFProvider() = default;
    virtual double3 sample(double lambda) const = 0;
};

// Uses the built-in CIE 1931 table with linear interpolation.
class LUTCMF final : public CMFProvider {
public:
    double3 sample(double const lambda) const override {
        if (lambda < CIE_1931_2DEG.front().lambda || lambda > CIE_1931_2DEG.back().lambda) {
            return double3(0.0);
        }
        // Linear interpolation
        const auto it = std::lower_bound(CIE_1931_2DEG.begin(), CIE_1931_2DEG.end(), lambda,
                [](const CMFEntry& entry, double const val) { return entry.lambda < val; });

        if (it == CIE_1931_2DEG.begin()) return double3(it->x, it->y, it->z);

        const auto& p1 = *(it - 1);
        const auto& p2 = *it;

        const double t = (lambda - p1.lambda) / (p2.lambda - p1.lambda);
        return double3(
                p1.x * (1.0 - t) + p2.x * t,
                p1.y * (1.0 - t) + p2.y * t,
                p1.z * (1.0 - t) + p2.z * t
                );
    }
};

// Uses Wyman's multi-lobe Gaussian approximation for the CIE 1931 CMFs.
// Reference: "Simple Analytic Approximations to the CIE XYZ Color Matching Functions", Wyman et al.
class AnalyticCMF final : public CMFProvider {
    // Wyman's approximation
    static double g(double const lambda, double const mu, double const sigma1, double const sigma2) {
        const double sigma = (lambda < mu) ? sigma1 : sigma2;
        const double t = (lambda - mu) / sigma;
        return std::exp(-0.5 * t * t);
    }

public:
    double3 sample(double const lambda) const override {
        const double x = 1.056 * g(lambda, 599.8, 37.9, 31.0) +
                         0.362 * g(lambda, 442.0, 16.0, 26.7) -
                         0.065 * g(lambda, 501.1, 20.4, 26.2);

        const double y = 0.821 * g(lambda, 568.8, 46.9, 40.5) +
                         0.286 * g(lambda, 530.9, 16.3, 31.1);

        const double z = 1.217 * g(lambda, 437.0, 11.8, 36.0) +
                         0.681 * g(lambda, 459.0, 26.0, 13.8);

        return double3(x, y, z);
    }
};

// ------------------------------------------------------------------------------------------------
// Math Functions
// ------------------------------------------------------------------------------------------------

// Gauss-Legendre Quadrature
// Computes nodes (x) and weights (w) for integration over the interval [-1, 1].
// These are used to optimally sample the spectral range.
static void computeGaussLegendre(int const n, std::vector<double>& x, std::vector<double>& w) {
    x.resize(n);
    w.resize(n);

    constexpr double eps = 1e-14;
    const int m = (n + 1) / 2;

    for (int i = 0; i < m; ++i) {
        double z = std::cos(PI * (i + 0.75) / (n + 0.5));
        double pp = 0.0;
        double p1;

        do {
            p1 = 1.0;
            double p2 = 0.0;
            for (int j = 0; j < n; ++j) {
                double const p3 = p2;
                p2 = p1;
                p1 = ((2.0 * j + 1.0) * z * p2 - j * p3) / (j + 1.0);
            }
            pp = n * (z * p1 - p2) / (z * z - 1.0);
            z = z - p1 / pp;
        } while (std::abs(p1 / pp) > eps);

        x[i] = -z;
        x[n - 1 - i] = z;
        w[i] = 2.0 / ((1.0 - z * z) * pp * pp);
        w[n - 1 - i] = w[i];
    }
}

// ------------------------------------------------------------------------------------------------
// Main Logic
// ------------------------------------------------------------------------------------------------

enum class DispersionModel {
    Cauchy,
    Linear
};

struct Config {
    int n = 4;
    std::string mode = "fraunhofer";
    bool useLut = false;
    bool noCorrect = false;
    bool debug = false;
    std::string format = "text";
    double minLambda = 420.0;
    double maxLambda = 680.0;
    DispersionModel dispersion = DispersionModel::Cauchy;
};

// Computes the IOR offset for a given wavelength.
// This offset is used in the shader to modify the base IOR:
//    ior(lambda) = baseIOR + ((baseIOR - 1) / abbe) * offset(lambda)
static double computeIOROffset(double const lambda, DispersionModel const model) {
    constexpr double lambda_D = 589.3;
    constexpr double lambda_F = 486.1;
    constexpr double lambda_C = 656.3;

    if (model == DispersionModel::Linear) {
        // Linear approximation: n(lambda) = A + B * lambda
        // Offset is normalized such that Offset(D) = 0 and Offset(F) - Offset(C) = 1 (approx)
        // Note: The denominator (F - C) is negative (486.1 - 656.3 = -170.2)
        return (lambda - lambda_D) / (lambda_F - lambda_C);
    }

    // Cauchy dispersion model: n(lambda) = A + B / lambda^2
    // This is physically more accurate for most transparent materials in the visible range.
    //
    // Derivation:
    // 1. n(lambda) = A + B / lambda^2
    // 2. nD = A + B / lambda_D^2  =>  A = nD - B / lambda_D^2
    // 3. n(lambda) = nD + B * (1/lambda^2 - 1/lambda_D^2)
    //
    // Abbe number Vd = (nD - 1) / (nF - nC)
    // nF - nC = B * (1/lambda_F^2 - 1/lambda_C^2)
    // B = (nF - nC) / (1/lambda_F^2 - 1/lambda_C^2)
    // B = ((nD - 1) / Vd) / (1/lambda_F^2 - 1/lambda_C^2)
    //
    // Substitute B back into (3):
    // n(lambda) = nD + ((nD - 1) / Vd) * [ (1/lambda^2 - 1/lambda_D^2) / (1/lambda_F^2 - 1/lambda_C^2) ]
    //
    // The term in brackets is the Offset(lambda).

    const double term = (1.0 / (lambda * lambda)) - (1.0 / (lambda_D * lambda_D));
    constexpr double scale = (1.0 / (lambda_F * lambda_F)) - (1.0 / (lambda_C * lambda_C));

    return term / scale;
}

static void printMatrix(const mat3& m, const std::string& name, const std::string& format,
        double const lambda) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << lambda << "nm";
    const std::string comment = ss.str();

    auto formatNumber = [](double value) {
        std::stringstream s;
        s << std::fixed << std::setprecision(8) << std::showpos << value;
        std::string str = s.str();
        if (str[0] == '+') {
            str[0] = ' ';
        }
        return str;
    };

    if (format == "glsl") {
        std::cout << "const mat3 " << name << " = mat3(\n"
                << "    " << formatNumber(m[0][0]) << ", " << formatNumber(m[0][1]) << ", " << formatNumber(m[0][2]) << ",\n"
                << "    " << formatNumber(m[1][0]) << ", " << formatNumber(m[1][1]) << ", " << formatNumber(m[1][2]) << ",\n"
                << "    " << formatNumber(m[2][0]) << ", " << formatNumber(m[2][1]) << ", " << formatNumber(m[2][2]) << "\n"
                << "); // " << comment << "\n";
    } else if (format == "cpp") {
        std::cout << "const std::array<double, 9> " << name << " = {\n"
                << "    " << formatNumber(m[0][0]) << ", " << formatNumber(m[0][1]) << ", " << formatNumber(m[0][2]) << ",\n"
                << "    " << formatNumber(m[1][0]) << ", " << formatNumber(m[1][1]) << ", " << formatNumber(m[1][2]) << ",\n"
                << "    " << formatNumber(m[2][0]) << ", " << formatNumber(m[2][1]) << ", " << formatNumber(m[2][2]) << "\n"
                << "}; // " << comment << "\n";
    } else {
        std::cout << name << " (" << comment << "):\n"
                << formatNumber(m[0][0]) << " " << formatNumber(m[1][0]) << " " << formatNumber(m[2][0]) << "\n"
                << formatNumber(m[0][1]) << " " << formatNumber(m[1][1]) << " " << formatNumber(m[2][1]) << "\n"
                << formatNumber(m[0][2]) << " " << formatNumber(m[1][2]) << " " << formatNumber(m[2][2]) << "\n";
    }
}

static void printUsage(const char* name) {
    std::cout << "Generates spectral integration matrices (Kn) and IOR offsets for real-time dispersion shaders.\n\n"
            << "Usage: " << name << " [options]\n"
            << "Options:\n"
            << "  --n, -n <int>             Number of samples (default: 4)\n"
            << "  --mode, -m <type>         Sampling distribution: 'fraunhofer', 'gaussian', 'linear' (default: fraunhofer)\n"
            << "  --min, -s <float>         Minimum wavelength in nm (default: 420.0)\n"
            << "  --max, -e <float>         Maximum wavelength in nm (default: 680.0)\n"
            << "  --dispersion, -p <type>   Dispersion model: 'cauchy', 'linear' (default: cauchy)\n"
            << "  --lut, -l                 Use built-in CMF table (380-780nm, 5nm steps)\n"
            << "  --analytic, -a            Use analytic polynomial approximation for CMF (default)\n"
            << "  --no-correct, -c          Disable sum(Kn) = Identity normalization\n"
            << "  --debug, -d               Print debug info\n"
            << "  --format, -f <type>       Output format: 'glsl', 'cpp', 'text' (default: text)\n"
            << "  --help, -h                Print this help message\n"
            << "\nMode Recommendations (for n=4):\n"
            << "  - fraunhofer: (Default) Best for strong visual dispersion ('rainbows'). Uses standard optical\n"
            << "    lines (F, e, D, C) for a guaranteed wide color spread. Ideal for artistic control.\n"
            << "  - gaussian: Best for overall color accuracy. Mathematically optimal for integrating smooth\n"
            << "    spectra, ensuring colors are correct when dispersion is subtle.\n";
}

int main(int argc, char* argv[]) {
    Config config;

    static const option long_options[] = {
        { "n", required_argument, nullptr, 'n' },
        { "mode", required_argument, nullptr, 'm' },
        { "min", required_argument, nullptr, 's' },
        { "max", required_argument, nullptr, 'e' },
        { "dispersion", required_argument, nullptr, 'p' },
        { "lut", no_argument, nullptr, 'l' },
        { "analytic", no_argument, nullptr, 'a' },
        { "no-correct", no_argument, nullptr, 'c' },
        { "debug", no_argument, nullptr, 'd' },
        { "format", required_argument, nullptr, 'f' },
        { "help", no_argument, nullptr, 'h' },
        { nullptr, 0, nullptr, 0 }
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "n:m:s:e:p:lacdf:h", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'n':
                config.n = std::atoi(optarg);
                break;
            case 'm':
                config.mode = optarg;
                break;
            case 's':
                config.minLambda = std::atof(optarg);
                break;
            case 'e':
                config.maxLambda = std::atof(optarg);
                break;
            case 'p':
                if (std::string(optarg) == "linear") config.dispersion = DispersionModel::Linear;
                else config.dispersion = DispersionModel::Cauchy;
                break;
            case 'l':
                config.useLut = true;
                break;
            case 'a':
                config.useLut = false;
                break;
            case 'c':
                config.noCorrect = true;
                break;
            case 'd':
                config.debug = true;
                break;
            case 'f':
                config.format = optarg;
                break;
            case 'h':
                printUsage(argv[0]);
                return 0;
            default:
                return EXIT_FAILURE;
        }
    }

    if (config.minLambda >= config.maxLambda) {
        std::cerr << "Error: min wavelength must be smaller than max wavelength.\n";
        return EXIT_FAILURE;
    }

    if (config.n < 3) {
        std::cerr << "Error: number of samples must be at least 3.\n";
        return EXIT_FAILURE;
    }

    if (config.format != "glsl" && config.format != "cpp" && config.format != "text") {
        std::cerr << "Error: invalid format. Must be 'glsl', 'cpp', or 'text'.\n";
        return EXIT_FAILURE;
    }

    if (config.useLut && config.mode != "fraunhofer") {
        if (config.minLambda < 380.0 || config.maxLambda > 780.0) {
            std::cerr << "Error: wavelength range must be within [380, 780] when using LUT.\n";
            return EXIT_FAILURE;
        }
    }

    std::unique_ptr<CMFProvider> cmf;
    if (config.useLut) {
        cmf = std::make_unique<LUTCMF>();
    } else {
        cmf = std::make_unique<AnalyticCMF>();
    }

    std::vector<Sample> samples;

    if (config.mode == "gaussian") {
        std::vector<double> x, w;
        computeGaussLegendre(config.n, x, w);
        // Map [-1, 1] to [minLambda, maxLambda]
        const double minLambda = config.minLambda;
        const double maxLambda = config.maxLambda;
        for (int i = 0; i < config.n; ++i) {
            const double lambda = 0.5 * (maxLambda - minLambda) * x[i] + 0.5 * (
                                      maxLambda + minLambda);
            const double weight = 0.5 * (maxLambda - minLambda) * w[i];
            samples.push_back({ lambda, weight });
        }
    } else if (config.mode == "linear") {
        const double minLambda = config.minLambda;
        const double maxLambda = config.maxLambda;
        if (config.n == 1) {
            samples.push_back({ (minLambda + maxLambda) * 0.5, maxLambda - minLambda });
        } else {
            const double interval = (maxLambda - minLambda) / (config.n - 1);
            for (int i = 0; i < config.n; ++i) {
                samples.push_back({ minLambda + i * interval, interval });
            }
        }
    } else if (config.mode == "fraunhofer") {
        if (config.n == 3) {
            // F, D, C (sorted low to high wavelength)
            samples = {
                { 486.1, 1.0 }, { 589.3, 1.0 }, { 656.3, 1.0 }
            };
        } else if (config.n == 4) {
            // F, e, D, C (sorted low to high wavelength)
            samples = {
                { 486.1, 1.0 }, { 546.1, 1.0 }, { 589.3, 1.0 }, { 656.3, 1.0 }
            };
        } else {
            std::cerr << "Error: fraunhofer mode only supports n=3 or n=4.\n";
            return EXIT_FAILURE;
        }
    }

    // Compute Wn (XYZ weights)
    std::vector<double3> Wn;
    double3 sumWn(0.0);

    for (auto const [lambda, weight]: samples) {
        const double3 xyz = cmf->sample(lambda);
        const double3 w = xyz * weight;
        Wn.push_back(w);
        sumWn += w;
    }

    // Normalization
    if (!config.noCorrect) {
        // Normalize such that sum(Wn) = (1, 1, 1) to ensure sum(Kn) = Identity
        const double3 correction = double3(1.0 / sumWn.x, 1.0 / sumWn.y, 1.0 / sumWn.z);
        for (auto& w: Wn) {
            w *= correction;
        }
        sumWn = double3(1.0);
    }

    // Compute Kn and Offsets
    std::vector<mat3> Kn;
    std::vector<double> offsets;
    mat3 sumKn(0.0);

    for (size_t i = 0; i < samples.size(); ++i) {
        const double3 w = Wn[i];
        const mat3 diagW = mat3(
                w.x, 0, 0,
                0, w.y, 0,
                0, 0, w.z
                );

        const mat3 K = M_XYZ_to_sRGB * diagW * M_sRGB_to_XYZ;
        Kn.push_back(K);
        sumKn += K;

        offsets.push_back(computeIOROffset(samples[i].lambda, config.dispersion));
    }

    // Output
    if (config.debug) {
        std::cout << "// Debug Info:\n";
        std::cout << "// Sum Wn: " << sumWn.x << ", " << sumWn.y << ", " << sumWn.z << "\n";
        printMatrix(sumKn, "Sum Kn", config.format, 0.0);
    }

    std::cout << std::fixed << std::setprecision(8);

    for (size_t i = 0; i < Kn.size(); ++i) {
        const std::string name = "K" + std::to_string(i);
        printMatrix(Kn[i], name, config.format, samples[i].lambda);
    }

    if (config.format == "glsl") {
        std::cout << "const float offsets[" << samples.size() << "] = float[](";
        for (size_t i = 0; i < offsets.size(); ++i) {
            std::cout << offsets[i] << (i < offsets.size() - 1 ? ", " : "");
        }
        std::cout << ");\n";
    } else if (config.format == "cpp") {
        std::cout << "const std::array<double, " << samples.size() << "> offsets = {";
        for (size_t i = 0; i < offsets.size(); ++i) {
            std::cout << offsets[i] << (i < offsets.size() - 1 ? ", " : "");
        }
        std::cout << "};\n";
    } else {
        std::cout << "Offsets: ";
        for (double const o: offsets) std::cout << o << " ";
        std::cout << "\n";
    }

    if (offsets.size() > 1) {
        double sumDist = 0.0;
        for (size_t i = 0; i < offsets.size() - 1; ++i) {
            sumDist += std::abs(offsets[i + 1] - offsets[i]);
        }
        double const avgDist = sumDist / (double)(offsets.size() - 1);
        if (config.format == "glsl") {
            std::cout << "// Average offset distance: " << avgDist << "\n";
        } else if (config.format == "cpp") {
            std::cout << "// Average offset distance: " << avgDist << "\n";
        } else {
            std::cout << "Average offset distance: " << avgDist << "\n";
        }
    }

    return 0;
}

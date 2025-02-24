//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// matrix_utils_unittests:
//   Unit tests for the matrix utils.
//

#include "matrix_utils.h"

#include <gtest/gtest.h>

using namespace angle;

namespace
{

struct RotateArgs
{
    float angle;
    Vector3 axis;
};

struct TranslateArgs
{
    float x;
    float y;
    float z;
};

struct ScaleArgs
{
    float x;
    float y;
    float z;
};

struct FrustumArgs
{
    float l;
    float r;
    float b;
    float t;
    float n;
    float f;
};

void CheckMat4ExactlyEq(const Mat4 &a, const Mat4 &b)
{
    for (unsigned int i = 0; i < 4; i++)
    {
        for (unsigned int j = 0; i < 4; i++)
        {
            EXPECT_EQ(a.at(i, j), b.at(i, j));
        }
    }
}

// TODO(lfy): Spec out requirements for matrix precision
void CheckMatrixCloseToGolden(float *golden, const Mat4 &m)
{
    const float floatFaultTolarance = 0.000001f;
    const auto &checkElts           = m.elements();
    for (size_t i = 0; i < checkElts.size(); i++)
    {
        EXPECT_NEAR(golden[i], checkElts[i], floatFaultTolarance);
    }
}

void CheckMatrixCloseToGolden(const std::vector<float> &golden, const Mat4 &m)
{
    const float floatFaultTolarance = 0.000001f;
    const auto &checkElts           = m.elements();
    for (size_t i = 0; i < golden.size(); i++)
    {
        EXPECT_NEAR(golden[i], checkElts[i], floatFaultTolarance);
    }
}
}  // namespace

namespace
{

const unsigned int minDimensions = 2;
const unsigned int maxDimensions = 4;

TEST(MatrixUtilsTest, MatrixConstructorTest)
{
    for (unsigned int i = minDimensions; i <= maxDimensions; i++)
    {
        for (unsigned int j = minDimensions; j <= maxDimensions; j++)
        {
            unsigned int numElements = i * j;
            Matrix<float> m(std::vector<float>(numElements, 1.0f), i, j);
            EXPECT_EQ(m.rows(), i);
            EXPECT_EQ(m.columns(), j);
            EXPECT_EQ(m.elements(), std::vector<float>(numElements, 1.0f));
        }
    }

    for (unsigned int i = minDimensions; i <= maxDimensions; i++)
    {
        unsigned int numElements = i * i;
        Matrix<float> m(std::vector<float>(numElements, 1.0f), i);
        EXPECT_EQ(m.size(), i);
        EXPECT_EQ(m.columns(), m.columns());
        EXPECT_EQ(m.elements(), std::vector<float>(numElements, 1.0f));
    }
}

TEST(MatrixUtilsTest, MatrixCompMultTest)
{
    for (unsigned int i = minDimensions; i <= maxDimensions; i++)
    {
        unsigned int numElements = i * i;
        Matrix<float> m1(std::vector<float>(numElements, 2.0f), i);
        Matrix<float> actualResult              = m1.compMult(m1);
        std::vector<float> actualResultElements = actualResult.elements();
        std::vector<float> expectedResultElements(numElements, 4.0f);
        EXPECT_EQ(expectedResultElements, actualResultElements);
    }
}

TEST(MatrixUtilsTest, MatrixOuterProductTest)
{
    for (unsigned int i = minDimensions; i <= maxDimensions; i++)
    {
        for (unsigned int j = minDimensions; j <= maxDimensions; j++)
        {
            unsigned int numElements = i * j;
            Matrix<float> m1(std::vector<float>(numElements, 2.0f), i, 1);
            Matrix<float> m2(std::vector<float>(numElements, 2.0f), 1, j);
            Matrix<float> actualResult = m1.outerProduct(m2);
            EXPECT_EQ(actualResult.rows(), i);
            EXPECT_EQ(actualResult.columns(), j);
            std::vector<float> actualResultElements = actualResult.elements();
            std::vector<float> expectedResultElements(numElements, 4.0f);
            EXPECT_EQ(expectedResultElements, actualResultElements);
        }
    }
}

TEST(MatrixUtilsTest, MatrixTransposeTest)
{
    for (unsigned int i = minDimensions; i <= maxDimensions; i++)
    {
        for (unsigned int j = minDimensions; j <= maxDimensions; j++)
        {
            unsigned int numElements = i * j;
            Matrix<float> m1(std::vector<float>(numElements, 2.0f), i, j);
            Matrix<float> expectedResult =
                Matrix<float>(std::vector<float>(numElements, 2.0f), j, i);
            Matrix<float> actualResult = m1.transpose();
            EXPECT_EQ(expectedResult.elements(), actualResult.elements());
            EXPECT_EQ(actualResult.rows(), expectedResult.rows());
            EXPECT_EQ(actualResult.columns(), expectedResult.columns());
            // transpose(transpose(A)) = A
            Matrix<float> m2 = actualResult.transpose();
            EXPECT_EQ(m1.elements(), m2.elements());
        }
    }
}

TEST(MatrixUtilsTest, MatrixDeterminantTest)
{
    for (unsigned int i = minDimensions; i <= maxDimensions; i++)
    {
        unsigned int numElements = i * i;
        Matrix<float> m(std::vector<float>(numElements, 2.0f), i);
        EXPECT_EQ(m.determinant(), 0.0f);
    }
}

TEST(MatrixUtilsTest, 2x2MatrixInverseTest)
{
    float inputElements[]    = {2.0f, 5.0f, 3.0f, 7.0f};
    unsigned int numElements = 4;
    std::vector<float> input(inputElements, inputElements + numElements);
    Matrix<float> inputMatrix(input, 2);
    float identityElements[] = {1.0f, 0.0f, 0.0f, 1.0f};
    std::vector<float> identityMatrix(identityElements, identityElements + numElements);
    // A * inverse(A) = I, where I is identity matrix.
    Matrix<float> result = inputMatrix * inputMatrix.inverse();
    EXPECT_EQ(identityMatrix, result.elements());
}

TEST(MatrixUtilsTest, 3x3MatrixInverseTest)
{
    float inputElements[]    = {11.0f, 23.0f, 37.0f, 13.0f, 29.0f, 41.0f, 19.0f, 31.0f, 43.0f};
    unsigned int numElements = 9;
    std::vector<float> input(inputElements, inputElements + numElements);
    Matrix<float> inputMatrix(input, 3);
    float identityElements[] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    std::vector<float> identityMatrix(identityElements, identityElements + numElements);
    // A * inverse(A) = I, where I is identity matrix.
    Matrix<float> result              = inputMatrix * inputMatrix.inverse();
    std::vector<float> resultElements = result.elements();
    const float floatFaultTolarance   = 0.000001f;
    for (size_t i = 0; i < numElements; i++)
        EXPECT_NEAR(resultElements[i], identityMatrix[i], floatFaultTolarance);
}

TEST(MatrixUtilsTest, 4x4MatrixInverseTest)
{
    float inputElements[]    = {29.0f, 43.0f, 61.0f, 79.0f, 31.0f, 47.0f, 67.0f, 83.0f,
                                37.0f, 53.0f, 71.0f, 89.0f, 41.0f, 59.0f, 73.0f, 97.0f};
    unsigned int numElements = 16;
    std::vector<float> input(inputElements, inputElements + numElements);
    Matrix<float> inputMatrix(input, 4);
    float identityElements[] = {
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    };
    std::vector<float> identityMatrix(identityElements, identityElements + numElements);
    // A * inverse(A) = I, where I is identity matrix.
    Matrix<float> result              = inputMatrix * inputMatrix.inverse();
    std::vector<float> resultElements = result.elements();
    const float floatFaultTolarance   = 0.00001f;
    for (unsigned int i = 0; i < numElements; i++)
        EXPECT_NEAR(resultElements[i], identityMatrix[i], floatFaultTolarance);
}

// Tests constructors for mat4; using raw float*, std::vector<float>,
// and Matrix<float>.
TEST(MatrixUtilsTest, Mat4Construction)
{
    float elements[] = {
        0.0f, 1.0f, 2.0f,  3.0f,  4.0f,  5.0f,  6.0f,  7.0f,
        8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f,
    };

    std::vector<float> elementsVector(16, 0);
    for (int i = 0; i < 16; i++)
    {
        elementsVector[i] = elements[i];
    }

    Matrix<float> a(elements, 4);
    Mat4 b(elements);
    Mat4 bVec(elementsVector);

    CheckMat4ExactlyEq(a, b);
    CheckMat4ExactlyEq(b, bVec);

    a.setToIdentity();
    b = Mat4();

    CheckMat4ExactlyEq(a, b);

    Mat4 c(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f,
           14.0f, 15.0f);
    Mat4 d(elements);
    Mat4 e(Matrix<float>(elements, 4));

    CheckMat4ExactlyEq(c, d);
    CheckMat4ExactlyEq(e, d);
}

// Tests rotation matrices.
TEST(MatrixUtilsTest, Mat4Rotate)
{
    // Confidence check.
    float elementsExpected[] = {
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    std::vector<float> elementsExpectedVector(16, 0);
    for (int i = 0; i < 16; i++)
    {
        elementsExpectedVector[i] = elementsExpected[i];
    }

    Mat4 r = Mat4::Rotate(0.f, Vector3(0.f, 0.f, 1.f));
    Mat4 golden(elementsExpected);
    CheckMatrixCloseToGolden(elementsExpected, r);
    CheckMatrixCloseToGolden(elementsExpectedVector, r);
    CheckMat4ExactlyEq(r, golden);

    // Randomly-generated inputs, outputs using GLM.
    std::vector<RotateArgs> rotationGoldenInputs;
    std::vector<std::vector<float>> rotationGoldenOutputs;
    rotationGoldenInputs.push_back(
        {-123.2026214599609375f,
         Vector3(1.4951171875000000f, 8.4370708465576172f, 1.8489227294921875f)});
    rotationGoldenInputs.push_back(
        {-185.3049316406250000f,
         Vector3(8.3313980102539062f, 5.8317651748657227f, -5.5201716423034668f)});
    rotationGoldenInputs.push_back(
        {89.1301574707031250f,
         Vector3(-8.5962724685668945f, 2.4547367095947266f, -3.2461600303649902f)});
    rotationGoldenInputs.push_back(
        {64.2547302246093750f,
         Vector3(-1.9640445709228516f, -9.6942234039306641f, 9.1921043395996094f)});
    rotationGoldenInputs.push_back(
        {-298.5585021972656250f,
         Vector3(-5.3600544929504395f, -6.4333534240722656f, -5.3734750747680664f)});
    rotationGoldenInputs.push_back(
        {288.2606201171875000f,
         Vector3(-2.3043875694274902f, -9.8447618484497070f, -0.9124794006347656f)});
    rotationGoldenInputs.push_back(
        {142.3956298828125000f,
         Vector3(-1.0044975280761719f, -2.5834980010986328f, -0.8451175689697266f)});
    rotationGoldenInputs.push_back(
        {-140.0445861816406250f,
         Vector3(-1.3710060119628906f, -2.5042991638183594f, -9.7572088241577148f)});
    rotationGoldenInputs.push_back(
        {-338.5443420410156250f,
         Vector3(6.8056621551513672f, 2.7508878707885742f, -5.8343429565429688f)});
    rotationGoldenInputs.push_back(
        {79.0578613281250000f,
         Vector3(9.0518493652343750f, -5.5615901947021484f, 6.3559799194335938f)});
    rotationGoldenInputs.push_back(
        {4464.6367187500000000f,
         Vector3(-53.9424285888671875f, -10.3614959716796875f, 54.3564453125000000f)});
    rotationGoldenInputs.push_back(
        {-2820.6347656250000000f,
         Vector3(62.1694793701171875f, 82.4977569580078125f, -60.0084800720214844f)});
    rotationGoldenInputs.push_back(
        {3371.0527343750000000f,
         Vector3(-74.5660324096679688f, -31.3026275634765625f, 96.7252349853515625f)});
    rotationGoldenInputs.push_back(
        {5501.7167968750000000f,
         Vector3(15.0308380126953125f, 23.2323913574218750f, 66.8295593261718750f)});
    rotationGoldenInputs.push_back(
        {392.1757812500000000f,
         Vector3(36.5722198486328125f, 69.2820892333984375f, 24.1789474487304688f)});
    rotationGoldenInputs.push_back(
        {-2206.7138671875000000f,
         Vector3(-91.5292282104492188f, 68.2716674804687500f, 42.0627288818359375f)});
    rotationGoldenInputs.push_back(
        {-4648.8623046875000000f,
         Vector3(50.7790374755859375f, 43.3964080810546875f, -36.3877525329589844f)});
    rotationGoldenInputs.push_back(
        {2794.6015625000000000f,
         Vector3(76.2934265136718750f, 63.4901885986328125f, 79.5993041992187500f)});
    rotationGoldenInputs.push_back(
        {2294.6787109375000000f,
         Vector3(-81.3662338256835938f, 77.6944580078125000f, 10.8423461914062500f)});
    rotationGoldenInputs.push_back(
        {2451.0468750000000000f,
         Vector3(-80.6299896240234375f, 51.8244628906250000f, 13.6877517700195312f)});
    rotationGoldenOutputs.push_back({
        -0.5025786161422729f,
        0.0775775760412216f,
        0.8610438108444214f,
        0.0000000000000000f,
        0.4305581450462341f,
        0.8861245512962341f,
        0.1714731603860855f,
        0.0000000000000000f,
        -0.7496895790100098f,
        0.4569081664085388f,
        -0.4787489175796509f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    rotationGoldenOutputs.push_back({
        0.0388860106468201f,
        0.6800884604454041f,
        -0.7320981621742249f,
        0.0000000000000000f,
        0.7683026194572449f,
        -0.4887983798980713f,
        -0.4132642149925232f,
        0.0000000000000000f,
        -0.6389046907424927f,
        -0.5464027523994446f,
        -0.5415209531784058f,
        -0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    rotationGoldenOutputs.push_back({
        0.8196773529052734f,
        -0.5709969997406006f,
        0.0457325875759125f,
        0.0000000000000000f,
        0.1115358471870422f,
        0.0807825028896332f,
        -0.9904716014862061f,
        0.0000000000000000f,
        0.5618618726730347f,
        0.8169679641723633f,
        0.1299021989107132f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    rotationGoldenOutputs.push_back({
        0.4463376104831696f,
        0.6722379326820374f,
        0.5906597971916199f,
        0.0000000000000000f,
        -0.5541059374809265f,
        0.7259115576744080f,
        -0.4074544310569763f,
        0.0000000000000000f,
        -0.7026730179786682f,
        -0.1454258561134338f,
        0.6964926123619080f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    rotationGoldenOutputs.push_back({
        0.6295375227928162f,
        -0.2925493717193604f,
        0.7197898030281067f,
        0.0000000000000000f,
        0.6561784744262695f,
        0.6962769031524658f,
        -0.2909094095230103f,
        0.0000000000000000f,
        -0.4160676598548889f,
        0.6554489731788635f,
        0.6302970647811890f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    rotationGoldenOutputs.push_back({
        0.3487194776535034f,
        0.2365041077136993f,
        -0.9068960547447205f,
        0.0000000000000000f,
        0.0657925531268120f,
        0.9590729475021362f,
        0.2754095196723938f,
        0.0000000000000000f,
        0.9349150061607361f,
        -0.1557076722383499f,
        0.3188872337341309f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    rotationGoldenOutputs.push_back({
        -0.5768983364105225f,
        0.3758956193923950f,
        0.7251832485198975f,
        0.0000000000000000f,
        0.7318079471588135f,
        0.6322251558303833f,
        0.2544573545455933f,
        0.0000000000000000f,
        -0.3628296852111816f,
        0.6774909496307373f,
        -0.6398130059242249f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    rotationGoldenOutputs.push_back({
        -0.7344170808792114f,
        0.6750319004058838f,
        0.0704519301652908f,
        0.0000000000000000f,
        -0.5576634407043457f,
        -0.6593509316444397f,
        0.5042498111724854f,
        0.0000000000000000f,
        0.3868372440338135f,
        0.3310411870479584f,
        0.8606789708137512f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    rotationGoldenOutputs.push_back({
        0.9672066569328308f,
        -0.2128375321626663f,
        -0.1386056393384933f,
        0.0000000000000000f,
        0.2423491925001144f,
        0.9366652965545654f,
        0.2528339624404907f,
        0.0000000000000000f,
        0.0760145336389542f,
        -0.2781336307525635f,
        0.9575299024581909f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    rotationGoldenOutputs.push_back({
        0.6229416728019714f,
        0.2379529774188995f,
        0.7451993227005005f,
        0.0000000000000000f,
        -0.7701886892318726f,
        0.3533243536949158f,
        0.5310096740722656f,
        0.0000000000000000f,
        -0.1369417309761047f,
        -0.9047321081161499f,
        0.4033691287040710f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    rotationGoldenOutputs.push_back({
        0.0691163539886475f,
        0.5770183801651001f,
        -0.8138013482093811f,
        0.0000000000000000f,
        -0.2371776401996613f,
        -0.7828579545021057f,
        -0.5752218365669250f,
        -0.0000000000000000f,
        -0.9690044522285461f,
        0.2327727228403091f,
        0.0827475786209106f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    rotationGoldenOutputs.push_back({
        0.6423799991607666f,
        -0.2559574246406555f,
        -0.7223805189132690f,
        0.0000000000000000f,
        0.6084498763084412f,
        0.7434381246566772f,
        0.2776480317115784f,
        0.0000000000000000f,
        0.4659791886806488f,
        -0.6178879141807556f,
        0.6333070397377014f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    rotationGoldenOutputs.push_back({
        -0.0772442221641541f,
        0.8218145370483398f,
        -0.5644945502281189f,
        0.0000000000000000f,
        -0.3352625370025635f,
        -0.5546256303787231f,
        -0.7615703344345093f,
        -0.0000000000000000f,
        -0.9389528036117554f,
        0.1304270327091217f,
        0.3183652758598328f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    rotationGoldenOutputs.push_back({
        -0.1511210501194000f,
        0.9849811196327209f,
        -0.0835132896900177f,
        0.0000000000000000f,
        -0.8243820667266846f,
        -0.0789582207798958f,
        0.5604994893074036f,
        0.0000000000000000f,
        0.5454874038696289f,
        0.1535501033067703f,
        0.8239331245422363f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    rotationGoldenOutputs.push_back({
        0.8769769668579102f,
        0.2149325907230377f,
        -0.4297852218151093f,
        0.0000000000000000f,
        -0.0991527736186981f,
        0.9560844898223877f,
        0.2758098840713501f,
        0.0000000000000000f,
        0.4701915681362152f,
        -0.1992645263671875f,
        0.8597752451896667f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    rotationGoldenOutputs.push_back({
        0.8634905815124512f,
        -0.3842780590057373f,
        0.3266716897487640f,
        0.0000000000000000f,
        0.1189628839492798f,
        0.7845909595489502f,
        0.6084939241409302f,
        0.0000000000000000f,
        -0.4901345074176788f,
        -0.4865669906139374f,
        0.7232017517089844f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    rotationGoldenOutputs.push_back({
        0.9201348423957825f,
        -0.1924955844879150f,
        -0.3410238325595856f,
        0.0000000000000000f,
        0.3022402822971344f,
        0.9028221964836121f,
        0.3058805167675018f,
        0.0000000000000000f,
        0.2490032464265823f,
        -0.3845224678516388f,
        0.8888981342315674f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    rotationGoldenOutputs.push_back({
        0.4109522104263306f,
        -0.3483857214450836f,
        0.8424642086029053f,
        0.0000000000000000f,
        0.8988372087478638f,
        0.3092637956142426f,
        -0.3105604350566864f,
        0.0000000000000000f,
        -0.1523487865924835f,
        0.8848635554313660f,
        0.4402346611022949f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    rotationGoldenOutputs.push_back({
        0.1795312762260437f,
        -0.7746179103851318f,
        -0.6064119935035706f,
        0.0000000000000000f,
        -0.9110413789749146f,
        0.1016657948493958f,
        -0.3995841145515442f,
        0.0000000000000000f,
        0.3711764216423035f,
        0.6242043375968933f,
        -0.6874569058418274f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    rotationGoldenOutputs.push_back({
        0.8035809993743896f,
        -0.4176068902015686f,
        0.4241013824939728f,
        0.0000000000000000f,
        -0.1537265181541443f,
        0.5427432060241699f,
        0.8257105946540833f,
        0.0000000000000000f,
        -0.5750005841255188f,
        -0.7287209630012512f,
        0.3719408810138702f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    EXPECT_EQ(rotationGoldenInputs.size(), rotationGoldenOutputs.size());

    for (size_t i = 0; i < rotationGoldenInputs.size(); i++)
    {
        const auto &input  = rotationGoldenInputs[i];
        const auto &output = rotationGoldenOutputs[i];
        Mat4 rot           = Mat4::Rotate(input.angle, input.axis);
        CheckMatrixCloseToGolden(output, rot);
    }
}

// Tests mat4 translation.
TEST(MatrixUtilsTest, Mat4Translate)
{
    float elementsExpected[] = {
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    std::vector<float> elementsExpectedVector(16, 0);
    for (int i = 0; i < 16; i++)
    {
        elementsExpectedVector[i] = elementsExpected[i];
    }

    Mat4 r = Mat4::Translate(Vector3(0.f, 0.f, 0.f));
    Mat4 golden(elementsExpected);
    CheckMatrixCloseToGolden(elementsExpected, r);
    CheckMatrixCloseToGolden(elementsExpectedVector, r);
    CheckMat4ExactlyEq(r, golden);

    float elementsExpected1[] = {
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    };

    float elementsExpected2[] = {
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 2.0f, 2.0f, 2.0f, 1.0f,
    };

    Mat4 golden1(elementsExpected1);
    Mat4 golden2(elementsExpected2);

    Mat4 t1   = Mat4::Translate(Vector3(1.f, 1.f, 1.f));
    Mat4 t2   = Mat4::Translate(Vector3(2.f, 2.f, 2.f));
    Mat4 t1t1 = t1.product(t1);

    CheckMat4ExactlyEq(t1, golden1);
    CheckMat4ExactlyEq(t2, golden2);
    CheckMat4ExactlyEq(t1t1, golden2);
}

// Tests scale for mat4.
TEST(MatrixUtilsTest, Mat4Scale)
{

    // Confidence check.
    float elementsExpected[] = {
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    std::vector<float> elementsExpectedVector(16, 0);
    for (int i = 0; i < 16; i++)
    {
        elementsExpectedVector[i] = elementsExpected[i];
    }

    Mat4 r = Mat4::Scale(Vector3(1.f, 1.f, 1.f));
    Mat4 golden(elementsExpected);
    CheckMatrixCloseToGolden(elementsExpected, r);
    CheckMatrixCloseToGolden(elementsExpectedVector, r);
    CheckMat4ExactlyEq(r, golden);

    float elementsExpected2[] = {
        2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    float elementsExpected4[] = {
        4.0f, 0.0f, 0.0f, 0.0f, 0.0f, 4.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    Mat4 golden2(elementsExpected2);
    Mat4 golden4(elementsExpected4);

    Mat4 t2   = Mat4::Scale(Vector3(2.f, 2.f, 2.f));
    Mat4 t2t2 = t2.product(t2);

    CheckMat4ExactlyEq(t2, golden2);
    CheckMat4ExactlyEq(t2t2, golden4);
}

// Tests frustum matrices.
TEST(MatrixUtilsTest, Mat4Frustum)
{
    // Randomly-generated inputs, outputs using GLM.
    std::vector<FrustumArgs> frustumGoldenInputs;
    std::vector<std::vector<float>> frustumGoldenOutputs;

    EXPECT_EQ(frustumGoldenInputs.size(), frustumGoldenOutputs.size());

    frustumGoldenInputs.push_back({6.5640869140625000f, 2.6196002960205078f, 7.6299438476562500f,
                                   -3.5341463088989258f, 1.5971517562866211f, 3.3254327774047852f});
    frustumGoldenInputs.push_back({-9.4570913314819336f, -5.3334407806396484f, 0.8660326004028320f,
                                   -4.5830192565917969f, -6.7980914115905762f,
                                   4.4744148254394531f});
    frustumGoldenInputs.push_back({1.4876422882080078f, 2.8094434738159180f, -1.6803054809570312f,
                                   -0.8823823928833008f, 9.7977848052978516f,
                                   -8.6204166412353516f});
    frustumGoldenInputs.push_back({-3.3456401824951172f, 9.8235015869140625f, 3.5869121551513672f,
                                   5.2356719970703125f, -4.0609388351440430f, 7.7973194122314453f});
    frustumGoldenInputs.push_back({9.5381393432617188f, 7.5244369506835938f, 3.2054557800292969f,
                                   -5.9051651954650879f, -8.1114397048950195f,
                                   -8.9626598358154297f});
    frustumGoldenInputs.push_back({4.5731735229492188f, 1.3278274536132812f, -3.2062773704528809f,
                                   -7.9090023040771484f, -6.6076564788818359f,
                                   5.1205596923828125f});
    frustumGoldenInputs.push_back({1.2519702911376953f, 1.8767604827880859f, 2.7256736755371094f,
                                   -9.6021852493286133f, -3.9267930984497070f,
                                   2.3794260025024414f});
    frustumGoldenInputs.push_back({-8.9887380599975586f, 6.2763042449951172f, 5.8243169784545898f,
                                   9.2890701293945312f, 1.3859443664550781f, -6.4422101974487305f});
    frustumGoldenInputs.push_back({5.7714862823486328f, 1.3688507080078125f, 6.2611656188964844f,
                                   -8.5859031677246094f, -3.2825427055358887f,
                                   -9.7015857696533203f});
    frustumGoldenInputs.push_back({5.4493808746337891f, 7.7383766174316406f, -1.1092796325683594f,
                                   -3.6691951751708984f, -8.1641988754272461f,
                                   4.3122081756591797f});
    frustumGoldenInputs.push_back({-47.3135452270507812f, 1.1683349609375000f, 36.1483764648437500f,
                                   -54.2228546142578125f, 76.4831085205078125f,
                                   51.5773468017578125f});
    frustumGoldenInputs.push_back({60.6750030517578125f, -35.2967681884765625f,
                                   -32.8269577026367188f, 77.3887939453125000f,
                                   73.3114624023437500f, -54.2619438171386719f});
    frustumGoldenInputs.push_back({19.5157546997070312f, 1.3348083496093750f, 34.2350463867187500f,
                                   -11.5907974243164062f, -6.4835968017578125f,
                                   30.2026824951171875f});
    frustumGoldenInputs.push_back({16.5014953613281250f, -59.4170761108398438f,
                                   -22.8201065063476562f, 62.5094299316406250f,
                                   -3.9552688598632812f, -76.2280197143554688f});
    frustumGoldenInputs.push_back({35.6607666015625000f, -49.5569000244140625f,
                                   97.1632690429687500f, 23.1938247680664062f, 18.6621780395507812f,
                                   55.2039489746093750f});
    frustumGoldenInputs.push_back({12.7565383911132812f, -0.8035964965820312f, 94.0040435791015625f,
                                   -73.9960327148437500f, -51.3727264404296875f,
                                   -21.3958053588867188f});
    frustumGoldenInputs.push_back({0.6055984497070312f, -21.7872161865234375f, 22.3246612548828125f,
                                   10.5279464721679688f, -56.8082237243652344f,
                                   24.1726150512695312f});
    frustumGoldenInputs.push_back({69.2176513671875000f, -59.0015220642089844f,
                                   -38.5509605407714844f, 74.0315246582031250f,
                                   47.9032897949218750f, -89.4692459106445312f});
    frustumGoldenInputs.push_back({90.4153137207031250f, 10.0325012207031250f, 16.1712417602539062f,
                                   -9.9705123901367188f, 25.6828689575195312f,
                                   51.8659057617187500f});
    frustumGoldenInputs.push_back({-89.5869369506835938f, -87.6541290283203125f,
                                   -3.0015182495117188f, -46.5026855468750000f,
                                   29.3566131591796875f, -3.5230865478515625f});
    frustumGoldenOutputs.push_back({
        -0.8098147511482239f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.2861230373382568f,
        0.0000000000000000f,
        0.0000000000000000f,
        -2.3282337188720703f,
        -0.3668724894523621f,
        -2.8482546806335449f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -6.1462464332580566f,
        0.0000000000000000f,
    });
    frustumGoldenOutputs.push_back({
        -3.2971229553222656f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        2.4951465129852295f,
        0.0000000000000000f,
        0.0000000000000000f,
        -3.5867569446563721f,
        0.6821345686912537f,
        0.2061366289854050f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        5.3967552185058594f,
        0.0000000000000000f,
    });
    frustumGoldenOutputs.push_back({
        14.8248996734619141f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        24.5582180023193359f,
        0.0000000000000000f,
        0.0000000000000000f,
        3.2509319782257080f,
        -3.2116978168487549f,
        0.0639241635799408f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -9.1714696884155273f,
        0.0000000000000000f,
    });
    frustumGoldenOutputs.push_back({
        -0.6167355179786682f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -4.9260525703430176f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.4918970167636871f,
        5.3510427474975586f,
        -0.3150867819786072f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        5.3404870033264160f,
        0.0000000000000000f,
    });
    frustumGoldenOutputs.push_back({
        8.0562448501586914f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.7806558609008789f,
        0.0000000000000000f,
        0.0000000000000000f,
        -8.4732360839843750f,
        0.2963255345821381f,
        -20.0583839416503906f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        170.8137969970703125f,
        0.0000000000000000f,
    });
    frustumGoldenOutputs.push_back({
        4.0720810890197754f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        2.8101394176483154f,
        0.0000000000000000f,
        0.0000000000000000f,
        -1.8182963132858276f,
        2.3635828495025635f,
        0.1267964988946915f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        5.7698287963867188f,
        0.0000000000000000f,
    });
    frustumGoldenOutputs.push_back({
        -12.5699577331542969f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.6370600461959839f,
        0.0000000000000000f,
        0.0000000000000000f,
        5.0076503753662109f,
        0.5578025579452515f,
        0.2453716099262238f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        2.9632694721221924f,
        0.0000000000000000f,
    });
    frustumGoldenOutputs.push_back({
        0.1815840899944305f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.8000248670578003f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.1776892393827438f,
        4.3620386123657227f,
        -0.6459077596664429f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -2.2811365127563477f,
        0.0000000000000000f,
    });
    frustumGoldenOutputs.push_back({
        1.4911717176437378f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.4421805739402771f,
        0.0000000000000000f,
        0.0000000000000000f,
        -1.6218323707580566f,
        0.1565788835287094f,
        -2.0227515697479248f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        9.9223108291625977f,
        0.0000000000000000f,
    });
    frustumGoldenOutputs.push_back({
        -7.1334328651428223f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        6.3784909248352051f,
        0.0000000000000000f,
        0.0000000000000000f,
        5.7613725662231445f,
        1.8666533231735229f,
        0.3087419867515564f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        5.6435680389404297f,
        0.0000000000000000f,
    });
    frustumGoldenOutputs.push_back({
        3.1551213264465332f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -1.6926428079605103f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.9518032073974609f,
        0.2000025659799576f,
        5.1418004035949707f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        316.7777709960937500f,
        0.0000000000000000f,
    });
    frustumGoldenOutputs.push_back({
        -1.5277713537216187f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.3303264379501343f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.2644343674182892f,
        0.4043145775794983f,
        0.1493220180273056f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -62.3644447326660156f,
        0.0000000000000000f,
    });
    frustumGoldenOutputs.push_back({
        0.7132298350334167f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.2829668223857880f,
        0.0000000000000000f,
        0.0000000000000000f,
        -1.1468359231948853f,
        -0.4941370785236359f,
        -0.6465383172035217f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        10.6754913330078125f,
        0.0000000000000000f,
    });
    frustumGoldenOutputs.push_back({
        0.1041976660490036f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.0927057415246964f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.5652843713760376f,
        0.4651299417018890f,
        -1.1094539165496826f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        8.3434572219848633f,
        0.0000000000000000f,
    });
    frustumGoldenOutputs.push_back({
        -0.4379884898662567f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.5045915246009827f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.1630663424730301f,
        -1.6271190643310547f,
        -2.0214161872863770f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -56.3862075805664062f,
        0.0000000000000000f,
    });
    frustumGoldenOutputs.push_back({
        7.5770230293273926f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.6115797758102417f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.8814766407012939f,
        -0.1190952509641647f,
        2.4274852275848389f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -73.3338088989257812f,
        0.0000000000000000f,
    });
    frustumGoldenOutputs.push_back({
        5.0737905502319336f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        9.6311941146850586f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.9459113478660583f,
        -2.7848947048187256f,
        0.4030041098594666f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        33.9142799377441406f,
        0.0000000000000000f,
    });
    frustumGoldenOutputs.push_back({
        -0.7472094297409058f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.8509900569915771f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.0796770751476288f,
        0.3151517212390900f,
        -0.3025783598423004f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -62.3977890014648438f,
        0.0000000000000000f,
    });
    frustumGoldenOutputs.push_back({
        -0.6390139460563660f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -1.9648925065994263f,
        0.0000000000000000f,
        0.0000000000000000f,
        -1.2496180534362793f,
        -0.2371963709592819f,
        -2.9617946147918701f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -101.7502517700195312f,
        0.0000000000000000f,
    });
    frustumGoldenOutputs.push_back({
        30.3771648406982422f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -1.3496931791305542f,
        0.0000000000000000f,
        0.0000000000000000f,
        -91.7013320922851562f,
        1.1379971504211426f,
        0.7856983542442322f,
        -1.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -6.2911696434020996f,
        0.0000000000000000f,
    });

    for (size_t i = 0; i < frustumGoldenInputs.size(); i++)
    {
        const auto &input  = frustumGoldenInputs[i];
        const auto &output = frustumGoldenOutputs[i];
        Mat4 r             = Mat4::Frustum(input.l, input.r, input.b, input.t, input.n, input.f);
        CheckMatrixCloseToGolden(output, r);
    }
}

// Tests orthographic projection matrices.
TEST(MatrixUtilsTest, Mat4Ortho)
{
    // Randomly-generated inputs, outputs using GLM.
    std::vector<FrustumArgs> orthoGoldenInputs;
    std::vector<std::vector<float>> orthoGoldenOutputs;

    orthoGoldenInputs.push_back({-1.2515230178833008f, 5.6523618698120117f, -0.7427234649658203f,
                                 -2.9657564163208008f, -5.4732933044433594f, -9.6416902542114258f});
    orthoGoldenInputs.push_back({-7.8948307037353516f, -8.4146118164062500f, -4.3893952369689941f,
                                 7.4392738342285156f, -8.1261911392211914f, 3.1107978820800781f});
    orthoGoldenInputs.push_back({3.1667709350585938f, 3.9134168624877930f, -7.1993961334228516f,
                                 -0.2519502639770508f, 5.4625358581542969f, 8.8320560455322266f});
    orthoGoldenInputs.push_back({0.3597183227539062f, 5.7859621047973633f, 4.6786174774169922f,
                                 -6.4736566543579102f, -2.7510557174682617f, 2.9977531433105469f});
    orthoGoldenInputs.push_back({3.2480134963989258f, 9.3551750183105469f, -7.5922241210937500f,
                                 -2.5135083198547363f, -4.5282001495361328f, -5.4564580917358398f});
    orthoGoldenInputs.push_back({-6.6918325424194336f, -9.6249752044677734f, -6.9591665267944336f,
                                 -2.7099137306213379f, -5.5196690559387207f, -9.0791969299316406f});
    orthoGoldenInputs.push_back({5.9386453628540039f, -9.1784019470214844f, -1.4078102111816406f,
                                 -1.0552892684936523f, 3.7563705444335938f, -6.6715431213378906f});
    orthoGoldenInputs.push_back({-8.6238059997558594f, -0.2995386123657227f, 5.6623821258544922f,
                                 7.6483421325683594f, 5.6686410903930664f, -7.1456899642944336f});
    orthoGoldenInputs.push_back({2.3857555389404297f, -2.6020836830139160f, 6.7841358184814453f,
                                 0.9868297576904297f, 5.6463518142700195f, -1.7597074508666992f});
    orthoGoldenInputs.push_back({4.6028537750244141f, 0.1757516860961914f, -6.1434607505798340f,
                                 6.8524093627929688f, 8.4380626678466797f, -1.4824752807617188f});
    orthoGoldenInputs.push_back({40.3382720947265625f, -34.5338973999023438f, -11.1726379394531250f,
                                 21.4206924438476562f, 17.5087890625000000f, 70.2734069824218750f});
    orthoGoldenInputs.push_back({85.2872314453125000f, 22.3899002075195312f, -92.8537139892578125f,
                                 7.6059341430664062f, 32.9500732421875000f, -8.1374511718750000f});
    orthoGoldenInputs.push_back({33.8771057128906250f, -27.6973648071289062f, 90.3841094970703125f,
                                 85.8473358154296875f, 36.0423278808593750f,
                                 -36.5140991210937500f});
    orthoGoldenInputs.push_back({-92.4729461669921875f, 7.1592102050781250f, -75.2177963256835938f,
                                 14.4945983886718750f, 10.6297378540039062f, 53.9828796386718750f});
    orthoGoldenInputs.push_back({90.2037658691406250f, 54.5332946777343750f, -58.9839515686035156f,
                                 56.7301330566406250f, 63.2403869628906250f, 81.1043853759765625f});
    orthoGoldenInputs.push_back({-78.6419067382812500f, 65.5156250000000000f, -78.8265304565429688f,
                                 -37.5036048889160156f, 76.9322204589843750f,
                                 -0.2213287353515625f});
    orthoGoldenInputs.push_back({80.1368560791015625f, 60.2946777343750000f, -27.3523788452148438f,
                                 88.5419616699218750f, -75.3445968627929688f,
                                 83.3852081298828125f});
    orthoGoldenInputs.push_back({55.0712585449218750f, -17.4033813476562500f, -98.6088104248046875f,
                                 81.6600952148437500f, 61.1217803955078125f, 73.5973815917968750f});
    orthoGoldenInputs.push_back({-48.7522583007812500f, 20.8100433349609375f, -45.6365356445312500f,
                                 -13.2819519042968750f, -29.7577133178710938f,
                                 62.1014862060546875f});
    orthoGoldenInputs.push_back({-60.3634567260742188f, 71.4023284912109375f, 59.0719757080078125f,
                                 22.6195831298828125f, -32.6802139282226562f,
                                 -56.3766899108886719f});
    orthoGoldenOutputs.push_back({
        0.2896919548511505f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.8996717929840088f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.4798007607460022f,
        0.0000000000000000f,
        -0.6374438405036926f,
        -1.6682072877883911f,
        -3.6260902881622314f,
        1.0000000000000000f,
    });
    orthoGoldenOutputs.push_back({
        -3.8477735519409180f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.1690807342529297f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.1779836267232895f,
        0.0000000000000000f,
        -31.3775196075439453f,
        -0.2578378617763519f,
        0.4463289380073547f,
        1.0000000000000000f,
    });
    orthoGoldenOutputs.push_back({
        2.6786458492279053f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.2878755927085876f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.5935563445091248f,
        0.0000000000000000f,
        -9.4826574325561523f,
        1.0725302696228027f,
        -4.2423224449157715f,
        1.0000000000000000f,
    });
    orthoGoldenOutputs.push_back({
        0.3685790896415710f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.1793356239795685f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.3478981554508209f,
        0.0000000000000000f,
        -1.1325846910476685f,
        -0.1609572321176529f,
        -0.0429127886891365f,
        1.0000000000000000f,
    });
    orthoGoldenOutputs.push_back({
        0.3274843692779541f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.3938003480434418f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        2.1545734405517578f,
        0.0000000000000000f,
        -2.0636737346649170f,
        1.9898203611373901f,
        -10.7563400268554688f,
        1.0000000000000000f,
    });
    orthoGoldenOutputs.push_back({
        -0.6818625330924988f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.4706709980964661f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.5618722438812256f,
        0.0000000000000000f,
        -5.5629096031188965f,
        2.2754778861999512f,
        -4.1013488769531250f,
        1.0000000000000000f,
    });
    orthoGoldenOutputs.push_back({
        -0.1323009729385376f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        5.6734218597412109f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.1917929202318192f,
        0.0000000000000000f,
        -0.2143114656209946f,
        6.9871010780334473f,
        -0.2795547246932983f,
        1.0000000000000000f,
    });
    orthoGoldenOutputs.push_back({
        0.2402613908052444f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0070695877075195f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.1560752540826797f,
        0.0000000000000000f,
        1.0719676017761230f,
        -6.7024130821228027f,
        -0.1152653917670250f,
        1.0000000000000000f,
    });
    orthoGoldenOutputs.push_back({
        -0.4009752273559570f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.3449878096580505f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.2700491547584534f,
        0.0000000000000000f,
        -0.0433711148798466f,
        1.3404442071914673f,
        0.5247924923896790f,
        1.0000000000000000f,
    });
    orthoGoldenOutputs.push_back({
        -0.4517627954483032f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.1538950353860855f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.2016019672155380f,
        0.0000000000000000f,
        1.0793980360031128f,
        -0.0545518361032009f,
        0.7011300325393677f,
        1.0000000000000000f,
    });
    orthoGoldenOutputs.push_back({
        -0.0267121959477663f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0613622479140759f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.0379041880369186f,
        0.0000000000000000f,
        0.0775237977504730f,
        -0.3144218325614929f,
        -1.6636564731597900f,
        1.0000000000000000f,
    });
    orthoGoldenOutputs.push_back({
        -0.0317978523671627f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0199084915220737f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0486765764653683f,
        0.0000000000000000f,
        1.7119507789611816f,
        0.8485773205757141f,
        0.6038967370986938f,
        1.0000000000000000f,
    });
    orthoGoldenOutputs.push_back({
        -0.0324809923768044f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.4408419132232666f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0275647528469563f,
        0.0000000000000000f,
        0.1003620624542236f,
        38.8451042175292969f,
        -0.0065021286718547f,
        1.0000000000000000f,
    });
    orthoGoldenOutputs.push_back({
        0.0200738403946161f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0222934633493423f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.0461327582597733f,
        0.0000000000000000f,
        0.8562871813774109f,
        0.6768652200698853f,
        -1.4903790950775146f,
        1.0000000000000000f,
    });
    orthoGoldenOutputs.push_back({
        -0.0560687854886055f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0172839816659689f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.1119570210576057f,
        0.0000000000000000f,
        4.0576157569885254f,
        0.0194774791598320f,
        -8.0802049636840820f,
        1.0000000000000000f,
    });
    orthoGoldenOutputs.push_back({
        0.0138737112283707f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0483992844820023f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0259223338216543f,
        0.0000000000000000f,
        0.0910551249980927f,
        2.8151476383209229f,
        0.9942626357078552f,
        1.0000000000000000f,
    });
    orthoGoldenOutputs.push_back({
        -0.1007953882217407f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0172570981085300f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.0126000288873911f,
        0.0000000000000000f,
        7.0774250030517578f,
        -0.5279772877693176f,
        -0.0506559647619724f,
        1.0000000000000000f,
    });
    orthoGoldenOutputs.push_back({
        -0.0275958590209484f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0110945366322994f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.1603129208087921f,
        0.0000000000000000f,
        0.5197387337684631f,
        0.0940190702676773f,
        -10.7986106872558594f,
        1.0000000000000000f,
    });
    orthoGoldenOutputs.push_back({
        0.0287512056529522f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0618150420486927f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.0217724516987801f,
        0.0000000000000000f,
        0.4016861617565155f,
        1.8210244178771973f,
        -0.3521016240119934f,
        1.0000000000000000f,
    });
    orthoGoldenOutputs.push_back({
        0.0151784475892782f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        -0.0548660829663277f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0844007357954979f,
        0.0000000000000000f,
        -0.0837764739990234f,
        2.2410478591918945f,
        -3.7582340240478516f,
        1.0000000000000000f,
    });

    EXPECT_EQ(orthoGoldenInputs.size(), orthoGoldenOutputs.size());

    for (size_t i = 0; i < orthoGoldenInputs.size(); i++)
    {
        const auto &input  = orthoGoldenInputs[i];
        const auto &output = orthoGoldenOutputs[i];
        Mat4 r             = Mat4::Ortho(input.l, input.r, input.b, input.t, input.n, input.f);
        CheckMatrixCloseToGolden(output, r);
    }
}

// Tests for inverse transpose of mat4, which is a frequent operation done to
// the modelview matrix, for things such as transformation of normals.
TEST(MatrixUtilsTest, Mat4InvTr)
{

    std::vector<std::vector<float>> invTrInputs;
    std::vector<std::vector<float>> invTrOutputs;
    invTrInputs.push_back({
        0.3247877955436707f,
        -0.8279561400413513f,
        0.4571666717529297f,
        0.0000000000000000f,
        0.9041201472282410f,
        0.1299063563346863f,
        -0.4070515632629395f,
        0.0000000000000000f,
        0.2776319682598114f,
        0.5455390810966492f,
        0.7907638549804688f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrInputs.push_back({
        -0.8843839168548584f,
        0.3047805428504944f,
        -0.3535164594650269f,
        0.0000000000000000f,
        0.4647803902626038f,
        0.6447106003761292f,
        -0.6068999171257019f,
        0.0000000000000000f,
        0.0429445058107376f,
        -0.7010400891304016f,
        -0.7118276357650757f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrInputs.push_back({
        -0.4487786889076233f,
        0.8632985353469849f,
        0.2308966517448425f,
        0.0000000000000000f,
        -0.5765564441680908f,
        -0.0823006033897400f,
        -0.8129017353057861f,
        -0.0000000000000000f,
        -0.6827739477157593f,
        -0.4979379475116730f,
        0.5346751213073730f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrInputs.push_back({
        0.9865614175796509f,
        0.0901400744915009f,
        -0.1362765878438950f,
        0.0000000000000000f,
        0.0155128352344036f,
        0.7786107659339905f,
        0.6273153424263000f,
        0.0000000000000000f,
        0.1626526862382889f,
        -0.6209991574287415f,
        0.7667490243911743f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrInputs.push_back({
        -0.5566663742065430f,
        0.8302737474441528f,
        -0.0277124047279358f,
        0.0000000000000000f,
        -0.0281167924404144f,
        0.0145094990730286f,
        0.9994993209838867f,
        0.0000000000000000f,
        0.8302601575851440f,
        0.5571669340133667f,
        0.0152677297592163f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrInputs.push_back({
        0.4160673320293427f,
        -0.9093281030654907f,
        0.0032218396663666f,
        0.0000000000000000f,
        -0.1397584974765778f,
        -0.0674473643302917f,
        -0.9878858327865601f,
        -0.0000000000000000f,
        0.8985296487808228f,
        0.4105767309665680f,
        -0.1551489830017090f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrInputs.push_back({
        0.9039891958236694f,
        -0.3967807590961456f,
        -0.1592751741409302f,
        0.0000000000000000f,
        0.4139676988124847f,
        0.9054293632507324f,
        0.0939593166112900f,
        0.0000000000000000f,
        0.1069311797618866f,
        -0.1508729904890060f,
        0.9827527999877930f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrInputs.push_back({
        0.3662135303020477f,
        0.2184857577085495f,
        0.9045174121856689f,
        0.0000000000000000f,
        -0.6651677489280701f,
        0.7412178516387939f,
        0.0902667641639709f,
        0.0000000000000000f,
        -0.6507223844528198f,
        -0.6347126960754395f,
        0.4167736768722534f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrInputs.push_back({
        0.3791426718235016f,
        0.7277372479438782f,
        -0.5715323686599731f,
        0.0000000000000000f,
        -0.3511379361152649f,
        0.6845993995666504f,
        0.6387689113616943f,
        0.0000000000000000f,
        0.8561266660690308f,
        -0.0414978563785553f,
        0.5150970220565796f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrInputs.push_back({
        -0.6766842603683472f,
        -0.0953263640403748f,
        -0.7300762534141541f,
        -0.0000000000000000f,
        -0.5922094583511353f,
        0.6596454977989197f,
        0.4627699255943298f,
        0.0000000000000000f,
        0.4374772906303406f,
        0.7455072402954102f,
        -0.5028247833251953f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrInputs.push_back({
        0.8012667894363403f,
        -0.5429225564002991f,
        0.2514090538024902f,
        0.0000000000000000f,
        0.3384204804897308f,
        0.7577887773513794f,
        0.5578778386116028f,
        0.0000000000000000f,
        -0.4933994412422180f,
        -0.3619270324707031f,
        0.7909271121025085f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrInputs.push_back({
        0.6100972890853882f,
        0.4051006734371185f,
        0.6809366941452026f,
        0.0000000000000000f,
        -0.6470826864242554f,
        0.7507012486457825f,
        0.1331603974103928f,
        0.0000000000000000f,
        -0.4572366476058960f,
        -0.5218631625175476f,
        0.7201343774795532f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrInputs.push_back({
        0.6304160356521606f,
        0.2796489298343658f,
        -0.7241353392601013f,
        0.0000000000000000f,
        0.3735889792442322f,
        0.7084143161773682f,
        0.5988158583641052f,
        0.0000000000000000f,
        0.6804460883140564f,
        -0.6480321288108826f,
        0.3421219885349274f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrInputs.push_back({
        -0.2888220548629761f,
        0.9278694987297058f,
        -0.2358816564083099f,
        0.0000000000000000f,
        0.8899697065353394f,
        0.1693908572196960f,
        -0.4233919680118561f,
        0.0000000000000000f,
        -0.3528962731361389f,
        -0.3322124481201172f,
        -0.8746994137763977f,
        -0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrInputs.push_back({
        0.7878623008728027f,
        0.2866843938827515f,
        0.5450551509857178f,
        0.0000000000000000f,
        -0.5206709504127502f,
        0.7827371954917908f,
        0.3409168124198914f,
        0.0000000000000000f,
        -0.3288993835449219f,
        -0.5523898601531982f,
        0.7659573554992676f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrInputs.push_back({
        -0.3225378990173340f,
        0.9464974999427795f,
        0.0105716586112976f,
        0.0000000000000000f,
        -0.4923008084297180f,
        -0.1582012772560120f,
        -0.8559277057647705f,
        -0.0000000000000000f,
        -0.8084610104560852f,
        -0.2812736034393311f,
        0.5169874429702759f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrInputs.push_back({
        0.3137793838977814f,
        0.3656709790229797f,
        0.8762575387954712f,
        0.0000000000000000f,
        0.8767655491828918f,
        0.2426431477069855f,
        -0.4152186512947083f,
        0.0000000000000000f,
        -0.3644512593746185f,
        0.8985595107078552f,
        -0.2444712668657303f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrInputs.push_back({
        0.8862395882606506f,
        -0.3831786513328552f,
        0.2602950036525726f,
        0.0000000000000000f,
        0.4416945576667786f,
        0.8683440685272217f,
        -0.2255758643150330f,
        0.0000000000000000f,
        -0.1395897567272186f,
        0.3148851394653320f,
        0.9388087987899780f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrInputs.push_back({
        0.4614841341972351f,
        -0.8635553717613220f,
        -0.2032356113195419f,
        0.0000000000000000f,
        -0.7492366433143616f,
        -0.5020523071289062f,
        0.4319583475589752f,
        0.0000000000000000f,
        -0.4750548601150513f,
        -0.0470703244209290f,
        -0.8786963224411011f,
        -0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrInputs.push_back({
        0.7327640056610107f,
        -0.5076418519020081f,
        0.4531629979610443f,
        0.0000000000000000f,
        -0.0702797919511795f,
        0.6059250831604004f,
        0.7924112081527710f,
        0.0000000000000000f,
        -0.6768439412117004f,
        -0.6124986410140991f,
        0.4083231091499329f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        0.3247878849506378f,
        -0.8279562592506409f,
        0.4571668505668640f,
        -0.0000000000000000f,
        0.9041203260421753f,
        0.1299064010381699f,
        -0.4070515930652618f,
        0.0000000000000000f,
        0.2776320576667786f,
        0.5455390810966492f,
        0.7907639741897583f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        0.0000000000000000f,
        -0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        -0.8843840360641479f,
        0.3047805130481720f,
        -0.3535164594650269f,
        -0.0000000000000000f,
        0.4647804200649261f,
        0.6447105407714844f,
        -0.6068999171257019f,
        -0.0000000000000000f,
        0.0429445207118988f,
        -0.7010400891304016f,
        -0.7118277549743652f,
        0.0000000000000000f,
        -0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        -0.4487787485122681f,
        0.8632985949516296f,
        0.2308966219425201f,
        -0.0000000000000000f,
        -0.5765565037727356f,
        -0.0823005959391594f,
        -0.8129017353057861f,
        0.0000000000000000f,
        -0.6827740073204041f,
        -0.4979379475116730f,
        0.5346751213073730f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        0.0000000000000000f,
        -0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        0.9865614175796509f,
        0.0901400819420815f,
        -0.1362766027450562f,
        -0.0000000000000000f,
        0.0155128324404359f,
        0.7786108255386353f,
        0.6273154020309448f,
        -0.0000000000000000f,
        0.1626526862382889f,
        -0.6209992170333862f,
        0.7667490839958191f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        0.0000000000000000f,
        -0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        -0.5566664934158325f,
        0.8302738070487976f,
        -0.0277124084532261f,
        -0.0000000000000000f,
        -0.0281168315559626f,
        0.0145094739273190f,
        0.9994993805885315f,
        -0.0000000000000000f,
        0.8302602171897888f,
        0.5571669340133667f,
        0.0152676850557327f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        0.4160673320293427f,
        -0.9093281030654907f,
        0.0032218731939793f,
        -0.0000000000000000f,
        -0.1397585272789001f,
        -0.0674473419785500f,
        -0.9878858327865601f,
        0.0000000000000000f,
        0.8985296487808228f,
        0.4105767309665680f,
        -0.1551489681005478f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        0.0000000000000000f,
        -0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        0.9039891958236694f,
        -0.3967807590961456f,
        -0.1592751741409302f,
        -0.0000000000000000f,
        0.4139677286148071f,
        0.9054294228553772f,
        0.0939593166112900f,
        0.0000000000000000f,
        0.1069311797618866f,
        -0.1508729755878448f,
        0.9827527999877930f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        0.0000000000000000f,
        -0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        0.3662134706974030f,
        0.2184857726097107f,
        0.9045172333717346f,
        -0.0000000000000000f,
        -0.6651675701141357f,
        0.7412176728248596f,
        0.0902667865157127f,
        0.0000000000000000f,
        -0.6507222652435303f,
        -0.6347125172615051f,
        0.4167735874652863f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        0.0000000000000000f,
        -0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        0.3791427314281464f,
        0.7277373671531677f,
        -0.5715324282646179f,
        -0.0000000000000000f,
        -0.3511379957199097f,
        0.6845995187759399f,
        0.6387690305709839f,
        -0.0000000000000000f,
        0.8561268448829651f,
        -0.0414978675544262f,
        0.5150971412658691f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        0.0000000000000000f,
        -0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        -0.6766842603683472f,
        -0.0953262373805046f,
        -0.7300761342048645f,
        -0.0000000000000000f,
        -0.5922094583511353f,
        0.6596452593803406f,
        0.4627698063850403f,
        0.0000000000000000f,
        0.4374772608280182f,
        0.7455070018768311f,
        -0.5028247833251953f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        0.8012669086456299f,
        -0.5429226160049438f,
        0.2514090836048126f,
        -0.0000000000000000f,
        0.3384204804897308f,
        0.7577888369560242f,
        0.5578778982162476f,
        0.0000000000000000f,
        -0.4933995306491852f,
        -0.3619270920753479f,
        0.7909271717071533f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        0.0000000000000000f,
        -0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        0.6100972294807434f,
        0.4051006138324738f,
        0.6809366941452026f,
        -0.0000000000000000f,
        -0.6470826268196106f,
        0.7507011294364929f,
        0.1331604123115540f,
        0.0000000000000000f,
        -0.4572366178035736f,
        -0.5218631029129028f,
        0.7201343774795532f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        0.0000000000000000f,
        -0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        0.6304160952568054f,
        0.2796489298343658f,
        -0.7241354584693909f,
        -0.0000000000000000f,
        0.3735889494419098f,
        0.7084143161773682f,
        0.5988159179687500f,
        -0.0000000000000000f,
        0.6804461479187012f,
        -0.6480321288108826f,
        0.3421220481395721f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        0.0000000000000000f,
        -0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        -0.2888221442699432f,
        0.9278693199157715f,
        -0.2358815670013428f,
        -0.0000000000000000f,
        0.8899695873260498f,
        0.1693906933069229f,
        -0.4233919084072113f,
        -0.0000000000000000f,
        -0.3528962433338165f,
        -0.3322124481201172f,
        -0.8746994137763977f,
        0.0000000000000000f,
        -0.0000000000000000f,
        0.0000000000000000f,
        0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        0.7878623008728027f,
        0.2866844236850739f,
        0.5450551509857178f,
        -0.0000000000000000f,
        -0.5206709504127502f,
        0.7827371954917908f,
        0.3409168422222137f,
        0.0000000000000000f,
        -0.3288994133472443f,
        -0.5523898601531982f,
        0.7659573554992676f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        0.0000000000000000f,
        -0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        -0.3225379288196564f,
        0.9464974999427795f,
        0.0105716586112976f,
        -0.0000000000000000f,
        -0.4923008382320404f,
        -0.1582012772560120f,
        -0.8559277057647705f,
        0.0000000000000000f,
        -0.8084610104560852f,
        -0.2812735736370087f,
        0.5169873833656311f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        0.0000000000000000f,
        -0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        0.3137793540954590f,
        0.3656708896160126f,
        0.8762574791908264f,
        -0.0000000000000000f,
        0.8767654895782471f,
        0.2426430881023407f,
        -0.4152186512947083f,
        0.0000000000000000f,
        -0.3644512593746185f,
        0.8985593318939209f,
        -0.2444712817668915f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        0.0000000000000000f,
        -0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        0.8862395882606506f,
        -0.3831786811351776f,
        0.2602950334548950f,
        -0.0000000000000000f,
        0.4416945576667786f,
        0.8683440685272217f,
        -0.2255758792161942f,
        0.0000000000000000f,
        -0.1395897865295410f,
        0.3148851692676544f,
        0.9388088583946228f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        0.0000000000000000f,
        -0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        0.4614838957786560f,
        -0.8635552525520325f,
        -0.2032355368137360f,
        -0.0000000000000000f,
        -0.7492365241050720f,
        -0.5020524263381958f,
        0.4319583177566528f,
        0.0000000000000000f,
        -0.4750548005104065f,
        -0.0470703467726707f,
        -0.8786963820457458f,
        0.0000000000000000f,
        -0.0000000000000000f,
        0.0000000000000000f,
        -0.0000000000000000f,
        1.0000000000000000f,
    });
    invTrOutputs.push_back({
        0.7327639460563660f,
        -0.5076418519020081f,
        0.4531629383563995f,
        -0.0000000000000000f,
        -0.0702798143029213f,
        0.6059250235557556f,
        0.7924111485481262f,
        0.0000000000000000f,
        -0.6768438220024109f,
        -0.6124985218048096f,
        0.4083230793476105f,
        -0.0000000000000000f,
        -0.0000000000000000f,
        0.0000000000000000f,
        -0.0000000000000000f,
        1.0000000000000000f,
    });

    EXPECT_EQ(invTrInputs.size(), invTrInputs.size());

    for (size_t i = 0; i < invTrInputs.size(); i++)
    {
        Mat4 a(invTrInputs[i]);
        CheckMatrixCloseToGolden(invTrOutputs[i], a.transpose().inverse());
    }
}

// Tests mat4 matrix/matrix multiplication by multiplying two translation matrices.
TEST(MatrixUtilsTest, Mat4Mult)
{
    Mat4 a, b;
    CheckMatrixCloseToGolden(a.data(), a.product(b));

    Mat4 r = Mat4::Translate(Vector3(1.f, 1.f, 1.f));
    CheckMatrixCloseToGolden(r.data(), a.product(r));

    Mat4 s = Mat4::Translate(Vector3(2.f, 2.f, 2.f));
    Mat4 t = r.product(r);
    CheckMatrixCloseToGolden(s.data(), t);
}

// Tests exact equality.
TEST(MatrixUtilsTest, ExactEquality)
{
    Matrix<float> a(std::vector<float>(16), 4, 4);
    Matrix<float> b(std::vector<float>(16), 4, 4);
    EXPECT_EQ(a, b);
    Matrix<float> c(std::vector<float>(16), 4, 4);
    c(0, 0) += 0.000001f;
    EXPECT_NE(a, c);
}

// Tests near equality.
TEST(MatrixUtilsTest, NearEquality)
{
    Matrix<float> a(std::vector<float>(16), 4, 4);
    Matrix<float> b(std::vector<float>(16), 4, 4);

    float *bData = b.data();
    for (int i = 0; i < 16; i++)
    {
        bData[i] += 0.09f;
    }

    EXPECT_TRUE(a.nearlyEqual(0.1f, b));

    for (int i = 0; i < 16; i++)
    {
        bData[i] -= 2 * 0.09f;
    }

    EXPECT_TRUE(a.nearlyEqual(0.1f, b));
}

}  // namespace

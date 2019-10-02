/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <cstdint>
#include <cstdlib>
#include <complex>
#include <fstream>
#include <iomanip>
#include <iostream>

#include <vector>

#include <getopt/getopt.h>

#include <utils/Path.h>

#include <math/mat3.h>
#include <math/scalar.h>
#include <math/vec3.h>

using namespace filament::math;
using namespace std::complex_literals;

static float g_incidenceAngle = 81.7f;

static void printUsage(const char* name) {
    std::string execName(utils::Path(name).getName());
    std::string usage(
            "SPECULAR_COLOR computes the base color of a conductor from spectral data\n"
                    "Usage:\n"
                    "    SPECULAR_COLOR [options] <spectral data file>\n"
                    "\n"
                    "Options:\n"
                    "   --help, -h\n"
                    "       Print this message\n\n"
                    "   --license\n"
                    "       Print copyright and license information\n\n"
    );

    const std::string from("SPECULAR_COLOR");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), execName);
    }
    printf("%s", usage.c_str());
}

static void license() {
    static const char *license[] = {
        #include "licenses/licenses.inc"
        nullptr
    };

    const char **p = &license[0];
    while (*p)
        std::cout << *p++ << std::endl;
}

static int handleArguments(int argc, char* argv[]) {
    static constexpr const char* OPTSTR = "hla:";
    static const struct option OPTIONS[] = {
            { "help",         no_argument, nullptr, 'h' },
            { "license",      no_argument, nullptr, 'l' },
            { "angle",  required_argument, nullptr, 'a' },
            { nullptr, 0, nullptr, 0 }  // termination of the option list
    };

    int opt;
    int optionIndex = 0;

    while ((opt = getopt_long(argc, argv, OPTSTR, OPTIONS, &optionIndex)) >= 0) {
        std::string arg(optarg ? optarg : "");
        switch (opt) {
            default:
            case 'h':
                printUsage(argv[0]);
                exit(0);
            case 'l':
                license();
                exit(0);
            case 'a':
                g_incidenceAngle = std::stof(arg);
                break;
        }
    }

    return optind;
}

// CIE 1931 2-deg color matching functions (CMFs), from 360nm to 830nm, at 1nm intervals
//
// Data source:
//     http://cvrl.ioo.ucl.ac.uk/cmfs.htm
//     http://cvrl.ioo.ucl.ac.uk/database/text/cmfs/ciexyz31.htm
const size_t CIE_XYZ_START = 360;
const size_t CIE_XYZ_COUNT = 471;
const float3 CIE_XYZ[CIE_XYZ_COUNT] = { // NOLINT
        { 0.0001299f, 0.000003917f, 0.0006061f },
        { 0.000145847f, 0.000004393581f, 0.0006808792f },
        { 0.0001638021f, 0.000004929604f, 0.0007651456f },
        { 0.0001840037f, 0.000005532136f, 0.0008600124f },
        { 0.0002066902f, 0.000006208245f, 0.0009665928f },
        { 0.0002321f, 0.000006965f, 0.001086f },
        { 0.000260728f, 0.000007813219f, 0.001220586f },
        { 0.000293075f, 0.000008767336f, 0.001372729f },
        { 0.000329388f, 0.000009839844f, 0.001543579f },
        { 0.000369914f, 0.00001104323f, 0.001734286f },
        { 0.0004149f, 0.00001239f, 0.001946f },
        { 0.0004641587f, 0.00001388641f, 0.002177777f },
        { 0.000518986f, 0.00001555728f, 0.002435809f },
        { 0.000581854f, 0.00001744296f, 0.002731953f },
        { 0.0006552347f, 0.00001958375f, 0.003078064f },
        { 0.0007416f, 0.00002202f, 0.003486f },
        { 0.0008450296f, 0.00002483965f, 0.003975227f },
        { 0.0009645268f, 0.00002804126f, 0.00454088f },
        { 0.001094949f, 0.00003153104f, 0.00515832f },
        { 0.001231154f, 0.00003521521f, 0.005802907f },
        { 0.001368f, 0.000039f, 0.006450001f },
        { 0.00150205f, 0.0000428264f, 0.007083216f },
        { 0.001642328f, 0.0000469146f, 0.007745488f },
        { 0.001802382f, 0.0000515896f, 0.008501152f },
        { 0.001995757f, 0.0000571764f, 0.009414544f },
        { 0.002236f, 0.000064f, 0.01054999f },
        { 0.002535385f, 0.00007234421f, 0.0119658f },
        { 0.002892603f, 0.00008221224f, 0.01365587f },
        { 0.003300829f, 0.00009350816f, 0.01558805f },
        { 0.003753236f, 0.0001061361f, 0.01773015f },
        { 0.004243f, 0.00012f, 0.02005001f },
        { 0.004762389f, 0.000134984f, 0.02251136f },
        { 0.005330048f, 0.000151492f, 0.02520288f },
        { 0.005978712f, 0.000170208f, 0.02827972f },
        { 0.006741117f, 0.000191816f, 0.03189704f },
        { 0.00765f, 0.000217f, 0.03621f },
        { 0.008751373f, 0.0002469067f, 0.04143771f },
        { 0.01002888f, 0.00028124f, 0.04750372f },
        { 0.0114217f, 0.00031852f, 0.05411988f },
        { 0.01286901f, 0.0003572667f, 0.06099803f },
        { 0.01431f, 0.000396f, 0.06785001f },
        { 0.01570443f, 0.0004337147f, 0.07448632f },
        { 0.01714744f, 0.000473024f, 0.08136156f },
        { 0.01878122f, 0.000517876f, 0.08915364f },
        { 0.02074801f, 0.0005722187f, 0.09854048f },
        { 0.02319f, 0.00064f, 0.1102f },
        { 0.02620736f, 0.00072456f, 0.1246133f },
        { 0.02978248f, 0.0008255f, 0.1417017f },
        { 0.03388092f, 0.00094116f, 0.1613035f },
        { 0.03846824f, 0.00106988f, 0.1832568f },
        { 0.04351f, 0.00121f, 0.2074f },
        { 0.0489956f, 0.001362091f, 0.2336921f },
        { 0.0550226f, 0.001530752f, 0.2626114f },
        { 0.0617188f, 0.001720368f, 0.2947746f },
        { 0.069212f, 0.001935323f, 0.3307985f },
        { 0.07763f, 0.00218f, 0.3713f },
        { 0.08695811f, 0.0024548f, 0.4162091f },
        { 0.09717672f, 0.002764f, 0.4654642f },
        { 0.1084063f, 0.0031178f, 0.5196948f },
        { 0.1207672f, 0.0035264f, 0.5795303f },
        { 0.13438f, 0.004f, 0.6456f },
        { 0.1493582f, 0.00454624f, 0.7184838f },
        { 0.1653957f, 0.00515932f, 0.7967133f },
        { 0.1819831f, 0.00582928f, 0.8778459f },
        { 0.198611f, 0.00654616f, 0.959439f },
        { 0.21477f, 0.0073f, 1.0390501f },
        { 0.2301868f, 0.008086507f, 1.1153673f },
        { 0.2448797f, 0.00890872f, 1.1884971f },
        { 0.2587773f, 0.00976768f, 1.2581233f },
        { 0.2718079f, 0.01066443f, 1.3239296f },
        { 0.2839f, 0.0116f, 1.3856f },
        { 0.2949438f, 0.01257317f, 1.4426352f },
        { 0.3048965f, 0.01358272f, 1.4948035f },
        { 0.3137873f, 0.01462968f, 1.5421903f },
        { 0.3216454f, 0.01571509f, 1.5848807f },
        { 0.3285f, 0.01684f, 1.62296f },
        { 0.3343513f, 0.01800736f, 1.6564048f },
        { 0.3392101f, 0.01921448f, 1.6852959f },
        { 0.3431213f, 0.02045392f, 1.7098745f },
        { 0.3461296f, 0.02171824f, 1.7303821f },
        { 0.34828f, 0.023f, 1.74706f },
        { 0.3495999f, 0.02429461f, 1.7600446f },
        { 0.3501474f, 0.02561024f, 1.7696233f },
        { 0.350013f, 0.02695857f, 1.7762637f },
        { 0.349287f, 0.02835125f, 1.7804334f },
        { 0.34806f, 0.0298f, 1.7826f },
        { 0.3463733f, 0.03131083f, 1.7829682f },
        { 0.3442624f, 0.03288368f, 1.7816998f },
        { 0.3418088f, 0.03452112f, 1.7791982f },
        { 0.3390941f, 0.03622571f, 1.7758671f },
        { 0.3362f, 0.038f, 1.77211f },
        { 0.3331977f, 0.03984667f, 1.7682589f },
        { 0.3300411f, 0.041768f, 1.764039f },
        { 0.3266357f, 0.043766f, 1.7589438f },
        { 0.3228868f, 0.04584267f, 1.7524663f },
        { 0.3187f, 0.048f, 1.7441f },
        { 0.3140251f, 0.05024368f, 1.7335595f },
        { 0.308884f, 0.05257304f, 1.7208581f },
        { 0.3032904f, 0.05498056f, 1.7059369f },
        { 0.2972579f, 0.05745872f, 1.6887372f },
        { 0.2908f, 0.06f, 1.6692f },
        { 0.2839701f, 0.06260197f, 1.6475287f },
        { 0.2767214f, 0.06527752f, 1.6234127f },
        { 0.2689178f, 0.06804208f, 1.5960223f },
        { 0.2604227f, 0.07091109f, 1.564528f },
        { 0.2511f, 0.0739f, 1.5281f },
        { 0.2408475f, 0.077016f, 1.4861114f },
        { 0.2298512f, 0.0802664f, 1.4395215f },
        { 0.2184072f, 0.0836668f, 1.3898799f },
        { 0.2068115f, 0.0872328f, 1.3387362f },
        { 0.19536f, 0.09098f, 1.28764f },
        { 0.1842136f, 0.09491755f, 1.2374223f },
        { 0.1733273f, 0.09904584f, 1.1878243f },
        { 0.1626881f, 0.1033674f, 1.1387611f },
        { 0.1522833f, 0.1078846f, 1.090148f },
        { 0.1421f, 0.1126f, 1.0419f },
        { 0.1321786f, 0.117532f, 0.9941976f },
        { 0.1225696f, 0.1226744f, 0.9473473f },
        { 0.1132752f, 0.1279928f, 0.9014531f },
        { 0.1042979f, 0.1334528f, 0.8566193f },
        { 0.09564f, 0.13902f, 0.8129501f },
        { 0.08729955f, 0.1446764f, 0.7705173f },
        { 0.07930804f, 0.1504693f, 0.7294448f },
        { 0.07171776f, 0.1564619f, 0.6899136f },
        { 0.06458099f, 0.1627177f, 0.6521049f },
        { 0.05795001f, 0.1693f, 0.6162f },
        { 0.05186211f, 0.1762431f, 0.5823286f },
        { 0.04628152f, 0.1835581f, 0.5504162f },
        { 0.04115088f, 0.1912735f, 0.5203376f },
        { 0.03641283f, 0.199418f, 0.4919673f },
        { 0.03201f, 0.20802f, 0.46518f },
        { 0.0279172f, 0.2171199f, 0.4399246f },
        { 0.0241444f, 0.2267345f, 0.4161836f },
        { 0.020687f, 0.2368571f, 0.3938822f },
        { 0.0175404f, 0.2474812f, 0.3729459f },
        { 0.0147f, 0.2586f, 0.3533f },
        { 0.01216179f, 0.2701849f, 0.3348578f },
        { 0.00991996f, 0.2822939f, 0.3175521f },
        { 0.00796724f, 0.2950505f, 0.3013375f },
        { 0.006296346f, 0.308578f, 0.2861686f },
        { 0.0049f, 0.323f, 0.272f },
        { 0.003777173f, 0.3384021f, 0.2588171f },
        { 0.00294532f, 0.3546858f, 0.2464838f },
        { 0.00242488f, 0.3716986f, 0.2347718f },
        { 0.002236293f, 0.3892875f, 0.2234533f },
        { 0.0024f, 0.4073f, 0.2123f },
        { 0.00292552f, 0.4256299f, 0.2011692f },
        { 0.00383656f, 0.4443096f, 0.1901196f },
        { 0.00517484f, 0.4633944f, 0.1792254f },
        { 0.00698208f, 0.4829395f, 0.1685608f },
        { 0.0093f, 0.503f, 0.1582f },
        { 0.01214949f, 0.5235693f, 0.1481383f },
        { 0.01553588f, 0.544512f, 0.1383758f },
        { 0.01947752f, 0.56569f, 0.1289942f },
        { 0.02399277f, 0.5869653f, 0.1200751f },
        { 0.0291f, 0.6082f, 0.1117f },
        { 0.03481485f, 0.6293456f, 0.1039048f },
        { 0.04112016f, 0.6503068f, 0.09666748f },
        { 0.04798504f, 0.6708752f, 0.08998272f },
        { 0.05537861f, 0.6908424f, 0.08384531f },
        { 0.06327f, 0.71f, 0.07824999f },
        { 0.07163501f, 0.7281852f, 0.07320899f },
        { 0.08046224f, 0.7454636f, 0.06867816f },
        { 0.08973996f, 0.7619694f, 0.06456784f },
        { 0.09945645f, 0.7778368f, 0.06078835f },
        { 0.1096f, 0.7932f, 0.05725001f },
        { 0.1201674f, 0.8081104f, 0.05390435f },
        { 0.1311145f, 0.8224962f, 0.05074664f },
        { 0.1423679f, 0.8363068f, 0.04775276f },
        { 0.1538542f, 0.8494916f, 0.04489859f },
        { 0.1655f, 0.862f, 0.04216f },
        { 0.1772571f, 0.8738108f, 0.03950728f },
        { 0.18914f, 0.8849624f, 0.03693564f },
        { 0.2011694f, 0.8954936f, 0.03445836f },
        { 0.2133658f, 0.9054432f, 0.03208872f },
        { 0.2257499f, 0.9148501f, 0.02984f },
        { 0.2383209f, 0.9237348f, 0.02771181f },
        { 0.2510668f, 0.9320924f, 0.02569444f },
        { 0.2639922f, 0.9399226f, 0.02378716f },
        { 0.2771017f, 0.9472252f, 0.02198925f },
        { 0.2904f, 0.954f, 0.0203f },
        { 0.3038912f, 0.9602561f, 0.01871805f },
        { 0.3175726f, 0.9660074f, 0.01724036f },
        { 0.3314384f, 0.9712606f, 0.01586364f },
        { 0.3454828f, 0.9760225f, 0.01458461f },
        { 0.3597f, 0.9803f, 0.0134f },
        { 0.3740839f, 0.9840924f, 0.01230723f },
        { 0.3886396f, 0.9874182f, 0.01130188f },
        { 0.4033784f, 0.9903128f, 0.01037792f },
        { 0.4183115f, 0.9928116f, 0.009529306f },
        { 0.4334499f, 0.9949501f, 0.008749999f },
        { 0.4487953f, 0.9967108f, 0.0080352f },
        { 0.464336f, 0.9980983f, 0.0073816f },
        { 0.480064f, 0.999112f, 0.0067854f },
        { 0.4959713f, 0.9997482f, 0.0062428f },
        { 0.5120501f, 1.0f, 0.005749999f },
        { 0.5282959f, 0.9998567f, 0.0053036f },
        { 0.5446916f, 0.9993046f, 0.0048998f },
        { 0.5612094f, 0.9983255f, 0.0045342f },
        { 0.5778215f, 0.9968987f, 0.0042024f },
        { 0.5945f, 0.995f, 0.0039f },
        { 0.6112209f, 0.9926005f, 0.0036232f },
        { 0.6279758f, 0.9897426f, 0.0033706f },
        { 0.6447602f, 0.9864444f, 0.0031414f },
        { 0.6615697f, 0.9827241f, 0.0029348f },
        { 0.6784f, 0.9786f, 0.002749999f },
        { 0.6952392f, 0.9740837f, 0.0025852f },
        { 0.7120586f, 0.9691712f, 0.0024386f },
        { 0.7288284f, 0.9638568f, 0.0023094f },
        { 0.7455188f, 0.9581349f, 0.0021968f },
        { 0.7621f, 0.952f, 0.0021f },
        { 0.7785432f, 0.9454504f, 0.002017733f },
        { 0.7948256f, 0.9384992f, 0.0019482f },
        { 0.8109264f, 0.9311628f, 0.0018898f },
        { 0.8268248f, 0.9234576f, 0.001840933f },
        { 0.8425f, 0.9154f, 0.0018f },
        { 0.8579325f, 0.9070064f, 0.001766267f },
        { 0.8730816f, 0.8982772f, 0.0017378f },
        { 0.8878944f, 0.8892048f, 0.0017112f },
        { 0.9023181f, 0.8797816f, 0.001683067f },
        { 0.9163f, 0.87f, 0.001650001f },
        { 0.9297995f, 0.8598613f, 0.001610133f },
        { 0.9427984f, 0.849392f, 0.0015644f },
        { 0.9552776f, 0.838622f, 0.0015136f },
        { 0.9672179f, 0.8275813f, 0.001458533f },
        { 0.9786f, 0.8163f, 0.0014f },
        { 0.9893856f, 0.8047947f, 0.001336667f },
        { 0.9995488f, 0.793082f, 0.00127f },
        { 1.0090892f, 0.781192f, 0.001205f },
        { 1.0180064f, 0.7691547f, 0.001146667f },
        { 1.0263f, 0.757f, 0.0011f },
        { 1.0339827f, 0.7447541f, 0.0010688f },
        { 1.040986f, 0.7324224f, 0.0010494f },
        { 1.047188f, 0.7200036f, 0.0010356f },
        { 1.0524667f, 0.7074965f, 0.0010212f },
        { 1.0567f, 0.6949f, 0.001f },
        { 1.0597944f, 0.6822192f, 0.00096864f },
        { 1.0617992f, 0.6694716f, 0.00092992f },
        { 1.0628068f, 0.6566744f, 0.00088688f },
        { 1.0629096f, 0.6438448f, 0.00084256f },
        { 1.0622f, 0.631f, 0.0008f },
        { 1.0607352f, 0.6181555f, 0.00076096f },
        { 1.0584436f, 0.6053144f, 0.00072368f },
        { 1.0552244f, 0.5924756f, 0.00068592f },
        { 1.0509768f, 0.5796379f, 0.00064544f },
        { 1.0456f, 0.5668f, 0.0006f },
        { 1.0390369f, 0.5539611f, 0.0005478667f },
        { 1.0313608f, 0.5411372f, 0.0004916f },
        { 1.0226662f, 0.5283528f, 0.0004354f },
        { 1.0130477f, 0.5156323f, 0.0003834667f },
        { 1.0026f, 0.503f, 0.00034f },
        { 0.9913675f, 0.4904688f, 0.0003072533f },
        { 0.9793314f, 0.4780304f, 0.00028316f },
        { 0.9664916f, 0.4656776f, 0.00026544f },
        { 0.9528479f, 0.4534032f, 0.0002518133f },
        { 0.9384f, 0.4412f, 0.00024f },
        { 0.923194f, 0.42908f, 0.0002295467f },
        { 0.907244f, 0.417036f, 0.00022064f },
        { 0.890502f, 0.405032f, 0.00021196f },
        { 0.87292f, 0.393032f, 0.0002021867f },
        { 0.8544499f, 0.381f, 0.00019f },
        { 0.835084f, 0.3689184f, 0.0001742133f },
        { 0.814946f, 0.3568272f, 0.00015564f },
        { 0.794186f, 0.3447768f, 0.00013596f },
        { 0.772954f, 0.3328176f, 0.0001168533f },
        { 0.7514f, 0.321f, 0.0001f },
        { 0.7295836f, 0.3093381f, 0.00008613333f },
        { 0.7075888f, 0.2978504f, 0.0000746f },
        { 0.6856022f, 0.2865936f, 0.000065f },
        { 0.6638104f, 0.2756245f, 0.00005693333f },
        { 0.6424f, 0.265f, 0.00004999999f },
        { 0.6215149f, 0.2547632f, 0.00004416f },
        { 0.6011138f, 0.2448896f, 0.00003948f },
        { 0.5811052f, 0.2353344f, 0.00003572f },
        { 0.5613977f, 0.2260528f, 0.00003264f },
        { 0.5419f, 0.217f, 0.00003f },
        { 0.5225995f, 0.2081616f, 0.00002765333f },
        { 0.5035464f, 0.1995488f, 0.00002556f },
        { 0.4847436f, 0.1911552f, 0.00002364f },
        { 0.4661939f, 0.1829744f, 0.00002181333f },
        { 0.4479f, 0.175f, 0.00002f },
        { 0.4298613f, 0.1672235f, 0.00001813333f },
        { 0.412098f, 0.1596464f, 0.0000162f },
        { 0.394644f, 0.1522776f, 0.0000142f },
        { 0.3775333f, 0.1451259f, 0.00001213333f },
        { 0.3608f, 0.1382f, 0.00001f },
        { 0.3444563f, 0.1315003f, 0.000007733333f },
        { 0.3285168f, 0.1250248f, 0.0000054f },
        { 0.3130192f, 0.1187792f, 0.0000032f },
        { 0.2980011f, 0.1127691f, 0.000001333333f },
        { 0.2835f, 0.107f, 0.0f },
        { 0.2695448f, 0.1014762f, 0.0f },
        { 0.2561184f, 0.09618864f, 0.0f },
        { 0.2431896f, 0.09112296f, 0.0f },
        { 0.2307272f, 0.08626485f, 0.0f },
        { 0.2187f, 0.0816f, 0.0f },
        { 0.2070971f, 0.07712064f, 0.0f },
        { 0.1959232f, 0.07282552f, 0.0f },
        { 0.1851708f, 0.06871008f, 0.0f },
        { 0.1748323f, 0.06476976f, 0.0f },
        { 0.1649f, 0.061f, 0.0f },
        { 0.1553667f, 0.05739621f, 0.0f },
        { 0.14623f, 0.05395504f, 0.0f },
        { 0.13749f, 0.05067376f, 0.0f },
        { 0.1291467f, 0.04754965f, 0.0f },
        { 0.1212f, 0.04458f, 0.0f },
        { 0.1136397f, 0.04175872f, 0.0f },
        { 0.106465f, 0.03908496f, 0.0f },
        { 0.09969044f, 0.03656384f, 0.0f },
        { 0.09333061f, 0.03420048f, 0.0f },
        { 0.0874f, 0.032f, 0.0f },
        { 0.08190096f, 0.02996261f, 0.0f },
        { 0.07680428f, 0.02807664f, 0.0f },
        { 0.07207712f, 0.02632936f, 0.0f },
        { 0.06768664f, 0.02470805f, 0.0f },
        { 0.0636f, 0.0232f, 0.0f },
        { 0.05980685f, 0.02180077f, 0.0f },
        { 0.05628216f, 0.02050112f, 0.0f },
        { 0.05297104f, 0.01928108f, 0.0f },
        { 0.04981861f, 0.01812069f, 0.0f },
        { 0.04677f, 0.017f, 0.0f },
        { 0.04378405f, 0.01590379f, 0.0f },
        { 0.04087536f, 0.01483718f, 0.0f },
        { 0.03807264f, 0.01381068f, 0.0f },
        { 0.03540461f, 0.01283478f, 0.0f },
        { 0.0329f, 0.01192f, 0.0f },
        { 0.03056419f, 0.01106831f, 0.0f },
        { 0.02838056f, 0.01027339f, 0.0f },
        { 0.02634484f, 0.009533311f, 0.0f },
        { 0.02445275f, 0.008846157f, 0.0f },
        { 0.0227f, 0.00821f, 0.0f },
        { 0.02108429f, 0.007623781f, 0.0f },
        { 0.01959988f, 0.007085424f, 0.0f },
        { 0.01823732f, 0.006591476f, 0.0f },
        { 0.01698717f, 0.006138485f, 0.0f },
        { 0.01584f, 0.005723f, 0.0f },
        { 0.01479064f, 0.005343059f, 0.0f },
        { 0.01383132f, 0.004995796f, 0.0f },
        { 0.01294868f, 0.004676404f, 0.0f },
        { 0.0121292f, 0.004380075f, 0.0f },
        { 0.01135916f, 0.004102f, 0.0f },
        { 0.01062935f, 0.003838453f, 0.0f },
        { 0.009938846f, 0.003589099f, 0.0f },
        { 0.009288422f, 0.003354219f, 0.0f },
        { 0.008678854f, 0.003134093f, 0.0f },
        { 0.008110916f, 0.002929f, 0.0f },
        { 0.007582388f, 0.002738139f, 0.0f },
        { 0.007088746f, 0.002559876f, 0.0f },
        { 0.006627313f, 0.002393244f, 0.0f },
        { 0.006195408f, 0.002237275f, 0.0f },
        { 0.005790346f, 0.002091f, 0.0f },
        { 0.005409826f, 0.001953587f, 0.0f },
        { 0.005052583f, 0.00182458f, 0.0f },
        { 0.004717512f, 0.00170358f, 0.0f },
        { 0.004403507f, 0.001590187f, 0.0f },
        { 0.004109457f, 0.001484f, 0.0f },
        { 0.003833913f, 0.001384496f, 0.0f },
        { 0.003575748f, 0.001291268f, 0.0f },
        { 0.003334342f, 0.001204092f, 0.0f },
        { 0.003109075f, 0.001122744f, 0.0f },
        { 0.002899327f, 0.001047f, 0.0f },
        { 0.002704348f, 0.0009765896f, 0.0f },
        { 0.00252302f, 0.0009111088f, 0.0f },
        { 0.002354168f, 0.0008501332f, 0.0f },
        { 0.002196616f, 0.0007932384f, 0.0f },
        { 0.00204919f, 0.00074f, 0.0f },
        { 0.00191096f, 0.0006900827f, 0.0f },
        { 0.001781438f, 0.00064331f, 0.0f },
        { 0.00166011f, 0.000599496f, 0.0f },
        { 0.001546459f, 0.0005584547f, 0.0f },
        { 0.001439971f, 0.00052f, 0.0f },
        { 0.001340042f, 0.0004839136f, 0.0f },
        { 0.001246275f, 0.0004500528f, 0.0f },
        { 0.001158471f, 0.0004183452f, 0.0f },
        { 0.00107643f, 0.0003887184f, 0.0f },
        { 0.0009999493f, 0.0003611f, 0.0f },
        { 0.0009287358f, 0.0003353835f, 0.0f },
        { 0.0008624332f, 0.0003114404f, 0.0f },
        { 0.0008007503f, 0.0002891656f, 0.0f },
        { 0.000743396f, 0.0002684539f, 0.0f },
        { 0.0006900786f, 0.0002492f, 0.0f },
        { 0.0006405156f, 0.0002313019f, 0.0f },
        { 0.0005945021f, 0.0002146856f, 0.0f },
        { 0.0005518646f, 0.0001992884f, 0.0f },
        { 0.000512429f, 0.0001850475f, 0.0f },
        { 0.0004760213f, 0.0001719f, 0.0f },
        { 0.0004424536f, 0.0001597781f, 0.0f },
        { 0.0004115117f, 0.0001486044f, 0.0f },
        { 0.0003829814f, 0.0001383016f, 0.0f },
        { 0.0003566491f, 0.0001287925f, 0.0f },
        { 0.0003323011f, 0.00012f, 0.0f },
        { 0.0003097586f, 0.0001118595f, 0.0f },
        { 0.0002888871f, 0.0001043224f, 0.0f },
        { 0.0002695394f, 0.0000973356f, 0.0f },
        { 0.0002515682f, 0.00009084587f, 0.0f },
        { 0.0002348261f, 0.0000848f, 0.0f },
        { 0.000219171f, 0.00007914667f, 0.0f },
        { 0.0002045258f, 0.000073858f, 0.0f },
        { 0.0001908405f, 0.000068916f, 0.0f },
        { 0.0001780654f, 0.00006430267f, 0.0f },
        { 0.0001661505f, 0.00006f, 0.0f },
        { 0.0001550236f, 0.00005598187f, 0.0f },
        { 0.0001446219f, 0.0000522256f, 0.0f },
        { 0.0001349098f, 0.0000487184f, 0.0f },
        { 0.000125852f, 0.00004544747f, 0.0f },
        { 0.000117413f, 0.0000424f, 0.0f },
        { 0.0001095515f, 0.00003956104f, 0.0f },
        { 0.0001022245f, 0.00003691512f, 0.0f },
        { 0.00009539445f, 0.00003444868f, 0.0f },
        { 0.0000890239f, 0.00003214816f, 0.0f },
        { 0.00008307527f, 0.00003f, 0.0f },
        { 0.00007751269f, 0.00002799125f, 0.0f },
        { 0.00007231304f, 0.00002611356f, 0.0f },
        { 0.00006745778f, 0.00002436024f, 0.0f },
        { 0.00006292844f, 0.00002272461f, 0.0f },
        { 0.00005870652f, 0.0000212f, 0.0f },
        { 0.00005477028f, 0.00001977855f, 0.0f },
        { 0.00005109918f, 0.00001845285f, 0.0f },
        { 0.00004767654f, 0.00001721687f, 0.0f },
        { 0.00004448567f, 0.00001606459f, 0.0f },
        { 0.00004150994f, 0.00001499f, 0.0f },
        { 0.00003873324f, 0.00001398728f, 0.0f },
        { 0.00003614203f, 0.00001305155f, 0.0f },
        { 0.00003372352f, 0.00001217818f, 0.0f },
        { 0.00003146487f, 0.00001136254f, 0.0f },
        { 0.00002935326f, 0.0000106f, 0.0f },
        { 0.00002737573f, 0.000009885877f, 0.0f },
        { 0.00002552433f, 0.000009217304f, 0.0f },
        { 0.00002379376f, 0.000008592362f, 0.0f },
        { 0.0000221787f, 0.000008009133f, 0.0f },
        { 0.00002067383f, 0.0000074657f, 0.0f },
        { 0.00001927226f, 0.000006959567f, 0.0f },
        { 0.0000179664f, 0.000006487995f, 0.0f },
        { 0.00001674991f, 0.000006048699f, 0.0f },
        { 0.00001561648f, 0.000005639396f, 0.0f },
        { 0.00001455977f, 0.0000052578f, 0.0f },
        { 0.00001357387f, 0.000004901771f, 0.0f },
        { 0.00001265436f, 0.00000456972f, 0.0f },
        { 0.00001179723f, 0.000004260194f, 0.0f },
        { 0.00001099844f, 0.000003971739f, 0.0f },
        { 0.00001025398f, 0.0000037029f, 0.0f },
        { 0.000009559646f, 0.000003452163f, 0.0f },
        { 0.000008912044f, 0.000003218302f, 0.0f },
        { 0.000008308358f, 0.0000030003f, 0.0f },
        { 0.000007745769f, 0.000002797139f, 0.0f },
        { 0.000007221456f, 0.0000026078f, 0.0f },
        { 0.000006732475f, 0.00000243122f, 0.0f },
        { 0.000006276423f, 0.000002266531f, 0.0f },
        { 0.000005851304f, 0.000002113013f, 0.0f },
        { 0.000005455118f, 0.000001969943f, 0.0f },
        { 0.000005085868f, 0.0000018366f, 0.0f },
        { 0.000004741466f, 0.00000171223f, 0.0f },
        { 0.000004420236f, 0.000001596228f, 0.0f },
        { 0.000004120783f, 0.00000148809f, 0.0f },
        { 0.000003841716f, 0.000001387314f, 0.0f },
        { 0.000003581652f, 0.0000012934f, 0.0f },
        { 0.000003339127f, 0.00000120582f, 0.0f },
        { 0.000003112949f, 0.000001124143f, 0.0f },
        { 0.000002902121f, 0.000001048009f, 0.0f },
        { 0.000002705645f, 0.000000977058f, 0.0f },
        { 0.000002522525f, 0.00000091093f, 0.0f },
        { 0.000002351726f, 0.000000849251f, 0.0f },
        { 0.000002192415f, 0.000000791721f, 0.0f },
        { 0.000002043902f, 0.00000073809f, 0.0f },
        { 0.000001905497f, 0.00000068811f, 0.0f },
        { 0.000001776509f, 0.00000064153f, 0.0f },
        { 0.000001656215f, 0.00000059809f, 0.0f },
        { 0.000001544022f, 0.000000557575f, 0.0f },
        { 0.00000143944f, 0.000000519808f, 0.0f },
        { 0.000001341977f, 0.000000484612f, 0.0f },
        { 0.000001251141f, 0.00000045181f, 0.0f }
};

// CIE Standard Illuminant D65 relative spectral power distribution,
// from 300nm to 830, at 5nm intervals
//
// Data source:
//     https://en.wikipedia.org/wiki/Illuminant_D65
//     https://cielab.xyz/pdf/CIE_sel_colorimetric_tables.xls
const size_t CIE_D65_INTERVAL = 5;
const size_t CIE_D65_START = 300;
const size_t CIE_D65_END = 830;
const size_t CIE_D65_COUNT = 107;
const float CIE_D65[CIE_D65_COUNT] = {
        0.0341f,
        1.6643f,
        3.2945f,
        11.7652f,
        20.2360f,
        28.6447f,
        37.0535f,
        38.5011f,
        39.9488f,
        42.4302f,
        44.9117f,
        45.7750f,
        46.6383f,
        49.3637f,
        52.0891f,
        51.0323f,
        49.9755f,
        52.3118f,
        54.6482f,
        68.7015f,
        82.7549f,
        87.1204f,
        91.4860f,
        92.4589f,
        93.4318f,
        90.0570f,
        86.6823f,
        95.7736f,
        104.865f,
        110.936f,
        117.008f,
        117.410f,
        117.812f,
        116.336f,
        114.861f,
        115.392f,
        115.923f,
        112.367f,
        108.811f,
        109.082f,
        109.354f,
        108.578f,
        107.802f,
        106.296f,
        104.790f,
        106.239f,
        107.689f,
        106.047f,
        104.405f,
        104.225f,
        104.046f,
        102.023f,
        100.000f,
        98.1671f,
        96.3342f,
        96.0611f,
        95.7880f,
        92.2368f,
        88.6856f,
        89.3459f,
        90.0062f,
        89.8026f,
        89.5991f,
        88.6489f,
        87.6987f,
        85.4936f,
        83.2886f,
        83.4939f,
        83.6992f,
        81.8630f,
        80.0268f,
        80.1207f,
        80.2146f,
        81.2462f,
        82.2778f,
        80.2810f,
        78.2842f,
        74.0027f,
        69.7213f,
        70.6652f,
        71.6091f,
        72.9790f,
        74.3490f,
        67.9765f,
        61.6040f,
        65.7448f,
        69.8856f,
        72.4863f,
        75.0870f,
        69.3398f,
        63.5927f,
        55.0054f,
        46.4182f,
        56.6118f,
        66.8054f,
        65.0941f,
        63.3828f,
        63.8434f,
        64.3040f,
        61.8779f,
        59.4519f,
        55.7054f,
        51.9590f,
        54.6998f,
        57.4406f,
        58.8765f,
        60.3125f
};

// The wavelength w must be between 300nm and 830nm
static float illuminantD65(float w) {
    auto i0 = size_t((w - CIE_D65_START) / CIE_D65_INTERVAL);
    uint2 indexBounds{i0, std::min(i0 + 1, CIE_D65_END)};

    float2 wavelengthBounds = CIE_D65_START + float2{indexBounds} * CIE_D65_INTERVAL;
    float t = (w - wavelengthBounds.x) / (wavelengthBounds.y - wavelengthBounds.x);
    return lerp(CIE_D65[indexBounds.x], CIE_D65[indexBounds.y], t);
}

struct Sample {
    float w = 0.0f; // wavelength
    std::complex<float> ior; // complex IOR, n + ik
};

// For std::lower_bound
bool operator<(const Sample& lhs, const Sample& rhs) {
    return lhs.w < rhs.w;
}

// The wavelength w must be between 360nm and 830nm
static std::complex<float> findSample(const std::vector<Sample>& samples, float w) {
    auto i1 = std::lower_bound(samples.begin(), samples.end(), Sample{w, 0.0f + 0.0if});
    auto i0 = i1 - 1;

    // Interpolate the complex IORs
    float t = (w - i0->w) / (i1->w - i0->w);
    float n = lerp(i0->ior.real(), i1->ior.real(), t);
    float k = lerp(i0->ior.imag(), i1->ior.imag(), t);
    return {n, k};
}

// Fresnel equation for complex IORs at normal incidence
static float fresnel(const std::complex<float>& sample) {
    return (((sample - (1.0f + 0if)) * (std::conj(sample) - (1.0f + 0if))) /
            ((sample + (1.0f + 0if)) * (std::conj(sample) + (1.0f + 0if)))).real();
}

// Fresnel equation for complex IORs at a specified angle
static float fresnel(const std::complex<float>& sample, float cosTheta) {
    float cosTheta2 = cosTheta * cosTheta;
    float sinTheta2 = 1.0f - cosTheta2;
    float eta2 = sample.real() * sample.real();
    float etak2 = sample.imag() * sample.imag();

    float t0 = eta2 - etak2 - sinTheta2;
    float a2plusb2 = sqrt(t0 * t0 + 4 * eta2 * etak2);
    float t1 = a2plusb2 + cosTheta2;
    float a = std::sqrt(0.5f * (a2plusb2 + t0));
    float t2 = 2 * a * cosTheta;
    float Rs = (t1 - t2) / (t1 + t2);

    float t3 = cosTheta2 * a2plusb2 + sinTheta2 * sinTheta2;
    float t4 = t2 * sinTheta2;
    float Rp = Rs * (t3 - t4) / (t3 + t4);

    return 0.5f * (Rp + Rs);
}

static float3 linear_to_sRGB(float3 linear) noexcept {
    float3 sRGB{linear};
#pragma nounroll
    for (size_t i = 0; i < sRGB.size(); i++) {
        sRGB[i] = (sRGB[i] <= 0.0031308f) ?
                sRGB[i] * 12.92f : (powf(sRGB[i], 1.0f / 2.4f) * 1.055f) - 0.055f;
    }
    return sRGB;
}

static float3 XYZ_to_sRGB(const float3& v) {
    const mat3f XYZ_sRGB{
             3.2404542f, -0.9692660f,  0.0556434f,
            -1.5371385f,  1.8760108f, -0.2040259f,
            -0.4985314f,  0.0415560f,  1.0572252f
    };
    return XYZ_sRGB * v;
}

struct Reflectance {
    float3 f0{0.0f};
    float3 f82{0.0f};
};

static Reflectance computeColor(const std::vector<Sample>& samples) {
    Reflectance xyz;

    float y = 0.0f;

    // We default to 81.7° but this can be specified by the user, so we use the
    // notation theta82/f81 in the code
    float cosTheta82 = std::cos(g_incidenceAngle * F_PI / 180.0f);

    // We need to evaluate the Fresnel equation at each spectral sample of
    // complex IOR over the visible spectrum. For each spectral sample, we
    // obtain a spectral reflectance sample. To find the RGB color at normal
    // incidence (f0, or baseColor in our material system), we must multiply
    // each sample by the CIE XYZ CMFs (color matching functions) and the
    // spectral power distribution of the desired illuminant. We choose the
    // standard illuminant D65 because we want to compute a color in the
    // sRGB color space.
    // We then sum (integrate) and normalize all the samples to obtain f0
    // in the XYZ color space. From there, a simple color space conversion
    // yields a linear sRGB color.
    for (size_t i = 0; i < CIE_XYZ_COUNT; i++) {
        // Current wavelength
        float w = float(CIE_XYZ_START + i);

        // Find most appropriate CIE XYZ sample for the wavelength
        auto sample = findSample(samples, w);

        // Compute Fresnel reflectance at normal incidence
        float f0 = fresnel(sample);

        // Compute Fresnel reflectance at the peak angle for the Lazanyi error term
        float f82 = fresnel(sample, cosTheta82);

        // We need to multiply by the spectral power distribution of the illuminant
        float d65 = illuminantD65(w);

        xyz.f0  += f0  * CIE_XYZ[i] * d65;
        xyz.f82 += f82 * CIE_XYZ[i] * d65;

        y += CIE_XYZ[i].y * d65;
    }

    xyz.f0  /= y;
    xyz.f82 /= y;

    xyz.f0  = XYZ_to_sRGB(xyz.f0);
    xyz.f82 = XYZ_to_sRGB(xyz.f82);

    // Rescale values that are outside of the sRGB gamut (gold for instance)
    // We should provide an option to compute f0 in wide gamut color spaces
    if (any(greaterThan(xyz.f0,  float3{1.0f}))) xyz.f0  *= 1.0f / max(xyz.f0);
    if (any(greaterThan(xyz.f82, float3{1.0f}))) xyz.f82 *= 1.0f / max(xyz.f82);

    return xyz;
}

static bool parseSpectralData(std::ifstream& in, std::vector<Sample>& samples) {
    std::string value;

    in >> value;
    if (value != "wl") return false;

    in >> value;
    if (value != "n") return false;

    while (in.good()) {
        in >> value;
        if (value == "wl") {
            in >> value;
            if (value != "k") return false;
            break;
        }

        Sample sample = { 0 };
        sample.w = std::strtof(value.c_str(), nullptr) * 1000.0f; // um to nm

        float n;
        in >> n;
        sample.ior.real(n);

        samples.push_back(sample);
    }

    size_t i = 0;
    while (in.good() && i < samples.size()) {
        Sample& sample = samples[i++];

        float w;
        in >> w;
        if (w * 1000.0f != sample.w) return false;

        float k;
        in >> k;
        sample.ior.imag(k);
    }

    return true;
}

void printColor(const float3& linear) {
    float3 sRGB = linear_to_sRGB(saturate(linear));

    std::cout << std::setfill(' ') << std::setw(12) << "linear: ";
    std::cout << std::fixed << std::setprecision(3) << linear.r << ", ";
    std::cout << std::fixed << std::setprecision(3) << linear.g << ", ";
    std::cout << std::fixed << std::setprecision(3) << linear.b;
    std::cout << std::endl;

    std::cout << std::setfill(' ') << std::setw(12) << "sRGB: ";
    std::cout << std::fixed << std::setprecision(3) << sRGB.r << ", ";
    std::cout << std::fixed << std::setprecision(3) << sRGB.g << ", ";
    std::cout << std::fixed << std::setprecision(3) << sRGB.b;
    std::cout << std::endl;

    std::cout << std::setfill(' ') << std::setw(12) << "sRGB: ";
    std::cout << std::setprecision(3) << int(sRGB.r * 255.99f) << ", ";
    std::cout << std::setprecision(3) << int(sRGB.g * 255.99f) << ", ";
    std::cout << std::setprecision(3) << int(sRGB.b * 255.99f);
    std::cout << std::endl;

    std::cout << std::setfill(' ') << std::setw(12) << "hex: ";
    std::cout << "#";
    std::cout << std::hex << std::setfill('0') << std::setw(2) << int(sRGB.r * 255.99f);
    std::cout << std::hex << std::setfill('0') << std::setw(2) << int(sRGB.g * 255.99f);
    std::cout << std::hex << std::setfill('0') << std::setw(2) << int(sRGB.b * 255.99f);
    std::cout << std::endl;

    // Reset decimal numbers
    std::cout << std::dec;
}

int main(int argc, char* argv[]) {
    int optionIndex = handleArguments(argc, argv);

    int numArgs = argc - optionIndex;
    if (numArgs < 1) {
        printUsage(argv[0]);
        return 1;
    }

    utils::Path src(argv[optionIndex]);
    if (!src.exists()) {
        std::cerr << "The spectral data " << src << " does not exist." << std::endl;
        return 1;
    }

    std::ifstream in(src.c_str(), std::ifstream::in);
    if (in.is_open()) {
        std::vector<Sample> samples;
        if (!parseSpectralData(in, samples)) {
            std::cerr << "Invalid spectral data file " << src << std::endl;
            return 1;
        }

        Reflectance reflectance = computeColor(samples);

        std::cout << "Material: " << src.getNameWithoutExtension() << std::endl;
        std::cout << std::endl;

        std::cout << "Reflectance at 0.0° (f0):" << std::endl;
        printColor(reflectance.f0);

        std::cout << std::endl;

        std::cout << "Reflectance at " <<
                std::fixed << std::setprecision(1) << g_incidenceAngle << "°:" << std::endl;
        printColor(reflectance.f82);
    } else {
        std::cerr << "Could not open the source material " << src << std::endl;
        return 1;
    }

    return 0;
}

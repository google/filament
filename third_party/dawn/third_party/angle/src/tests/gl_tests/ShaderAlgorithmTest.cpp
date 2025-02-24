#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

const int GRID_SIZE = 3;

class ShaderAlgorithmTest : public ANGLETest<>
{
  protected:
    ShaderAlgorithmTest()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    ~ShaderAlgorithmTest() {}
};

// Simplied version of dEQP test dEQP?GLES2.functional.shaders.algorithm.rgb_to_hsl_vertex
TEST_P(ShaderAlgorithmTest, rgb_to_hsl_vertex_shader)
{
    const char kVS[] =
        "attribute highp vec3 a_position;\n"
        "attribute highp vec3 a_unitCoords;\n"
        "varying mediump vec3 v_color;\n"

        "void main()\n"
        "{\n"
        "    gl_Position =vec4(a_position.x, a_position.y, a_position.z, 1.0);\n"
        "    mediump vec3 coords = a_unitCoords;\n"
        "    mediump vec3 res = vec3(0.0);\n"
        "    mediump float r = coords.x, g = coords.y, b = coords.z;\n"
        "    mediump float minVal = min(min(r, g), b);\n"
        "    mediump float maxVal = max(max(r, g), b);\n"
        "    mediump float H = 0.0; \n"
        "    mediump float S = 0.0; \n"
        "    if (r == maxVal)\n"
        "       H = 1.0;\n"
        "    else\n"
        "       S = 1.0;\n"
        "    res = vec3(H, S, 0);\n"
        "    v_color = res;\n"
        "}\n";

    const char kFS[] =
        "varying mediump vec3 v_color;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(v_color, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    // compute a_position vertex data
    std::vector<Vector3> positions{Vector3(-1.0f, -1.0f, 0.0f), Vector3(-1.0f, 1.0f, 0.0f),
                                   Vector3(1.0f, 1.0f, 0.0f), Vector3(1.0f, -1.0f, 0.0f)};

    // Pass the vertex data to VBO
    GLBuffer posBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions[0]) * positions.size(), positions.data(),
                 GL_STATIC_DRAW);

    // Link position data to "a_position" vertex attrib
    GLint posLocation = glGetAttribLocation(program, "a_position");
    ASSERT_NE(-1, posLocation);
    glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posLocation);

    // Pass the index data to EBO
    std::vector<GLuint> indices{0, 1, 2, 0, 2, 3};
    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(),
                 GL_STATIC_DRAW);

    // Initialize the "a_unitCoords" vertex attributes data
    std::vector<Vector3> unitcoords;
    unitcoords.resize(4);
    GLBuffer unitCoordBuffer;
    GLint unitCoordLocation = glGetAttribLocation(program, "a_unitCoords");
    ASSERT_NE(-1, unitCoordLocation);

    const float epsilon = 1.0e-7;
    int gridSize        = GRID_SIZE;

    for (int y = 0; y < gridSize + 1; y++)
    {
        for (int x = 0; x < gridSize + 1; x++)
        {
            float sx = (float)x / (float)gridSize;
            float sy = (float)y / (float)gridSize;

            // Pass the a_unitCoords data to VBO
            for (int vtx = 0; vtx < 4; vtx++)
            {
                unitcoords[vtx] = Vector3(sx, sy, 0.33f * sx + 0.5f * sy);
            }
            glBindBuffer(GL_ARRAY_BUFFER, unitCoordBuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(unitcoords[0]) * unitcoords.size(),
                         unitcoords.data(), GL_STATIC_DRAW);

            // Link the unitcoords data to "a_unitCoords" vertex attrib
            glVertexAttribPointer(unitCoordLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(unitCoordLocation);

            // Draw and verify
            glUseProgram(program);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

            ASSERT_GL_NO_ERROR();

            float maxVal = std::max({sx, sy, 0.33f * sx + 0.5f * sy});
            if (abs(maxVal - sx) <= epsilon)
            {
                EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
            }
            else
            {
                EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
            }
        }
    }
}

ANGLE_INSTANTIATE_TEST_ES2_AND(
    ShaderAlgorithmTest,
    ES2_VULKAN().enable(Feature::AvoidOpSelectWithMismatchingRelaxedPrecision));

}  // namespace

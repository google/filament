
namespace lsprotocol_tests;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;


public class LSPTests
{
    public static IEnumerable<object[]> JsonTestData()
    {
        string folderPath;
        // Read test data path from environment variable
        if (!string.IsNullOrEmpty(Environment.GetEnvironmentVariable("LSP_TEST_DATA_PATH")))
        {
            folderPath = Environment.GetEnvironmentVariable("LSP_TEST_DATA_PATH");
        }
        else
        {
            throw new Exception("LSP_TEST_DATA_PATH environment variable not set");
        }

        string[] jsonFiles = Directory.GetFiles(folderPath, "*.json");
        foreach (string filePath in jsonFiles)
        {
            yield return new object[] { filePath };
        }
    }

    [Theory]
    [MemberData(nameof(JsonTestData))]
    public void ValidateLSPTypes(string filePath)
    {
        string original = File.ReadAllText(filePath);

        // Get the class name from the file name
        // format: <class-name>-<valid>-<test-id>.json
        // classname => Class name of the type to deserialize to
        // valid => true if the file is valid, false if it is invalid
        // test-id => unique id for the test
        string fileName = Path.GetFileNameWithoutExtension(filePath);
        string[] nameParts = fileName.Split('-');
        string className = nameParts[0];
        bool valid = nameParts[1] == "True";

        Type type = Type.GetType($"Microsoft.LanguageServer.Protocol.{className}, lsprotocol") ?? throw new Exception($"Type {className} not found");
        RunTest(valid, original, type);
    }

    private static void RunTest(bool valid, string data, Type type)
    {
        if (valid)
        {
            try
            {
                var settings = new JsonSerializerSettings
                {
                    MissingMemberHandling = MissingMemberHandling.Error
                };
                object? deserializedObject = JsonConvert.DeserializeObject(data, type, settings);
                string newJson = JsonConvert.SerializeObject(deserializedObject, settings);

                JToken token1 = JToken.Parse(data);
                JToken token2 = JToken.Parse(newJson);
                RemoveNullProperties(token1);
                RemoveNullProperties(token2);
                Assert.True(JToken.DeepEquals(token1, token2), $"JSON before and after serialization don't match:\r\nBEFORE:{data}\r\nAFTER:{newJson}");
            }
            catch (Exception e)
            {
                // Explicitly fail the test
                Assert.True(false, $"Should not have thrown an exception for [{type.Name}]: {data} \r\n{e}");
            }
        }
        else
        {
            try
            {
                JsonConvert.DeserializeObject(data, type);
                // Explicitly fail the test
                Assert.True(false, $"Should have thrown an exception for [{type.Name}]: {data}");
            }
            catch
            {
                // Worked as expected.
            }
        }
    }

    private static void RemoveNullProperties(JToken token)
    {
        if (token.Type == JTokenType.Object)
        {
            var obj = (JObject)token;

            var propertiesToRemove = obj.Properties()
                .Where(p => p.Value.Type == JTokenType.Null)
                .ToList();

            foreach (var property in propertiesToRemove)
            {
                property.Remove();
            }

            foreach (var property in obj.Properties())
            {
                RemoveNullProperties(property.Value);
            }
        }
        else if (token.Type == JTokenType.Array)
        {
            var array = (JArray)token;

            for (int i = array.Count - 1; i >= 0; i--)
            {
                RemoveNullProperties(array[i]);
                if (array[i].Type == JTokenType.Null)
                {
                    array.RemoveAt(i);
                }
            }
        }
    }
}
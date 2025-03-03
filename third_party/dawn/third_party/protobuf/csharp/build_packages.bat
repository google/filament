@rem Builds Google.Protobuf NuGet packages

dotnet restore src/Google.Protobuf.sln
dotnet pack -c Release src/Google.Protobuf.sln -p:ContinuousIntegrationBuild=true || goto :error

goto :EOF

:error
echo Failed!
exit /b %errorlevel%

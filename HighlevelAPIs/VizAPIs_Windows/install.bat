cd ..
mkdir install
cd install
mkdir include
mkdir lib
mkdir bin
cd include
mkdir components
cd ../../VizAPIs_Windows

copy ..\API_SOURCE\VizEngineAPIs.h ..\install\\include\
copy ..\API_SOURCE\VizComponentAPIs.h ..\install\include\
copy ..\API_SOURCE\VzComponents.h ..\install\include\
copy ..\API_SOURCE\components\*.h ..\install\include\components\

copy ..\bin\Debug\VizAPIs_Windowsd.lib ..\install\lib\
copy ..\bin\Debug\VizAPIs_Windowsd.dll ..\install\bin\

copy ..\bin\Release\VizAPIs_Windows.lib ..\install\lib\
copy ..\bin\Release\VizAPIs_Windows.dll ..\install\bin\

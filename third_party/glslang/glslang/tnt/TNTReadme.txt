In order to work around an issue with Clang 3.8, a line of MachineIndependent/TShader.cpp had to be
changed:

line  992 had to me changed from:

SourceLineSynchronizer lineSync(std::bind(&TInputScanner::getLastValidSourceIndex, &input), &outputStream);

to:

SourceLineSynchronizer lineSync([&input](){return input.getLastValidSourceIndex();}, &outputStream);
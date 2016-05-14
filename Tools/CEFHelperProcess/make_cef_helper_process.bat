::clean build directory
rmdir /s /q _build
mkdir _build
cd _build

::generate project and building
cmake -G"Visual Studio 12" ../ -DUNITY_BUILD=true -DDISABLE_SOUNDS=true
cmake --build . --config Release

::leave directory and copy artifacts to Tools/Bin/cef
cd ..
copy /Y _build\Release\CEFHelperProcess.exe ..\Bin\cef
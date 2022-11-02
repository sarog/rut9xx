README
------
This document provides breif instructions on how to build the C SDK using CMake.
CMake is a build tool that will generate make or project files for many different platforms and IDEs

Note that the existing static make files provided in previous releases are still present and can be used to build this release as well.
See the documentation for more information on these files.

Below is a simple procedure for building the C SDK as a shared and static library, its sample programs and tests.

1. Download and install CMake from https://cmake.org/. Cmake comes as a command line and a GUI application. This document will use the command line version only.
2. Unzip the C SDK and change to this directory from your command prompt
3. Create a directory to build in, for this example call it "bin".
       >mkdir bin
       >cd bin
4. Run the CMake command listed below. This assumes CMake is already on you PATH.
       cmake ..
5. CMake has now produced a set of project files which should be compatible with your devlopment environment. For unix this will will a set of "make files". For Windows this will be a visual studio solution.
   To building the project will be specific to your platform but two examples are listed below:

   Unix
   ----
   Run make using the command below
       >make

   Windows
   -------
   Assuming your command line environment has your visual studio tools on the command line use the command below:

       >msbuild tw-c-sdk.sln /t:build

   Be aware that CMake does its best to determine what version of Visual Studio you have but you may wish to specify which version to use if you have more than one installed on your computer.
   Below is an example of forcing CMake to use a specific version of Visual Studio:

       >cmake -G "Visual Studio 12 2013" ..

   You also have the alternative of opening the tw-c-sdk.sln from within Visual Studio and building in this IDE.

6. Once your build completes you will find the build products in the following directories:

    Unix
    ----

    ./bin/src/libtwCSdk_static.a  (Static Library)
    ./bin/src/libtwCSdk.so        (Shared Library)
    ./bin/examples/SteamSensor/SteamSensor (Sample Application)

    Windows
    -------
    .\bin\src\<Debug/Release>\twCSdk_static.lib  (Static Library)
    .\bin\src\<Debug/Release>\twCSdk.dll         (Shared Library)
    .\bin\examples\<Debug/Release>\SteamSensor\SteamSensor.exe  (Sample Application)


Additional Notes
----------------

Choosing your IDE

CMake is capable of producing build files for many IDEs as well as command line build files which may be useful for integrating the C SDK into your own projects.
Generating build/project files for your particular IDE requires adding the -G option. Below are some sample CMake commands for build for specific IDEs.
To see a complete list use the "cmake -G" command with no other parameters.

Debug vs Release Builds

By default, CMake will generate a build for the creation of a release binary. If you want to generate a debug build, use the command below:

    >cmake -DBUILD_DEBUG=ON ..

Consult the documentation for information on other build options.
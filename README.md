# Building

In the source directory, create the build directory with
`mkdir build && cd build`
Then configure the build files with
`cmake ..`
You can now build the application with
`cmake --build . --config release`

# Install

Make sure that you have built the program before installing
To install, from the build directory run
`cmake --install . --prefix ../install`

# Running

To run the cache simulator, first install the project then run the command
`./install/bin/Main -c confs/* -s traces/*`
By default, this will generate all of the output files into the output folder.
For help with the options you can run
`./install/bin/Main --help`

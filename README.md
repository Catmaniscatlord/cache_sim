# Requirements

To build and run the cache simulator you will need the following bits of software

- `gcc 11.4.0`
- `cmake 3.22`
- `gnuplot` - this is required only to generate the graphs

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

To run the cache simulator, you must first install the project
After you have installed the project, in the source directory, run
`./install/bin/Main -c confs/* -s traces/*`
By default, this will generate all of the output files into the output folder.
For help with the options you can run
`./install/bin/Main --help`

# Testing

The test target is built by default when building the simulator. To run the tests, cd into the build directory and run
`ctest -VV --output-on-failure`

[![Build Status](https://travis-ci.org/open-rsx/rsb-spread-cpp.svg?branch=master)](https://travis-ci.org/open-rsx/rsb-spread-cpp) [![Coverage Report](https://codecov.io/gh/open-rsx/rsb-spread-cpp/branch/master/graph/badge.svg)](https://codecov.io/gh/open-rsx/rsb-spread-cpp)

# Introduction

This repository contains the C++ implementation of the [Spread][spread]-based transport for the [Robotics Service Bus][rsb] middleware.

**The full documentation for RSB can be found [here][rsb-doc].**

**This product uses software developed by Spread Concepts LLC for use in the Spread toolkit. For more information about Spread see <http://www.spread.org>.**

# Dependencies

* [CMake][cmake] build system
* [Boost C++ libraries][boost]
* [The RSC C++ library][rsc]
* [The C++ implementation of RSB][rsb-cpp]
* The C client library of the [Spread group communication framework][spread]

# Building and Installing

This project uses the default CMake workflow (assuming the RSC and RSB libraries have already been installed into `<prefix>`):

```sh
cd <path to code>/build/
cmake -DCMAKE_BUILD_TYPE=debug -DCMAKE_INSTALL_PREFIX=<prefix> ..
make
make test       # opt. run unit tests
make install    # install
```

To generate a coverage report chose `coverage` as CMake build type and do

```sh
make
make test
make coverage
```

# Contributing

If you want to contribute to this project, please

* Submit your intended changes as coherent pull requests
* Rebase onto the master branch and squash any fixups and corrections
* Make sure the unit tests pass (See [Building and Installing](#building-and-installing))

# Acknowledgments

The development of this software has been supported as follows:

* This research was funded by the EC 7th Framework Programme (FP7/2007-2013), in the TA2 (grant agreement ICT-2007-214 793) and HUMAVIPS (grant aggrement ICT-2009-247525) projects.
* The development of this software was supported by CoR-Lab, Research Institute for Cognition and Robotics Bielefeld University.
* This work was supported by the Cluster of Excellence Cognitive Interaction Technology ‘CITEC’ (EXC 277) at Bielefeld University, which is funded by the German Research Foundation (DFG).

[boost]: https://www.boost.org/

[cmake]: https://cmake.org/

[spread]: https://www.spread.org

[rsc]: https://github.com/open-rsx/rsc
[rsb]: https://github.com/open-rsx
[rsb-cpp]: https://github.com/open-rsx/rsb-cpp
[rsb-doc]: https://open-rsx.github.io/rsb-manual/

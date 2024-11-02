# NS-FLS: A Network Simulation Platform for Flying Light Specks

## Introduction

NS-FLS is a Network Simulator tailored for FLS systems and built on the widely used ns-3 platform. NS-FLS allows users to input movement patterns and timed user traffic, simulating FLS communication and get the simulation results. User can use this tool to analyze FLSes' communication condition in different network environments.

## Getting Started

### Dependencies

NS-FLS is build on ns3 plarform(release 3.442), therefore we need to install it before we can use NS-FLS.
There are two main options:

1. Download a release tarball. This will unpack to a directory such as ns-allinone-3.42 containing ns-3 and some other programs. Below is a command-line download using wget, but a browser download will also work:

```
$ wget https://www.nsnam.org/releases/ns-allinone-3.42.tar.bz2
$ tar xfj ns-allinone-3.42.tar.bz2
$ cd ns-allinone-3.42/ns-3.42
```

2. Clone ns-3 from the Git repository. The ns-3-allinone can be cloned, as well as ns-3-dev by itself. Below, we illustrate the latter:
   '''
   $ git clone https://gitlab.com/nsnam/ns-3-dev.git
   $ cd ns-3-dev
   '''

After that, we need to install a optional ns-3 module called NetAnim to show the FLS's moving pattern, we can refer this [page](https://www.nsnam.org/wiki/NetAnim_3.107) for instructions

### Installing

First we will create a NS-FLS directory under ns-3-dev home directory. Go to ns-3-dev home directory and run
'''
$ mkdir NS-FLS
$ cd NS-FLS
$ git clone https://github.com/GeNeLa/NS-FLS.git
'''

Then, we build our NS-FLS platform in ns-3-dev home directory
'''
$ ./ns3 configure --enable-examples --enable-tests
$ ./ns3 build
'''

### Executing program

You should put your trace file in traces directory, for node X, the name should be packet_trace_node_X for traffic trace file, and trace_node_X for position trace file.

In home directory, use the following command to run NS-FLS simulation

```
./ns3 run fls-simulation
```

You will see network statistics after simulation done.

## Join the Community

If you have any technical issue, please submit Issues. For any other question, please contact ychen1329@ucr.edu.

## Version History

- 0.1
  - Initial Release

## License

This project is licensed under the [GPL-3.0] License - see the LICENSE.md file for details

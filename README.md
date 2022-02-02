# kata

Kata [(kata.wtf)](https://kata.wtf) is a software platform for developing applications and libraries that run on desktop, mobile, web, and HPC environments from a single source.

Planned environments:

  * Native Linux builds
  * Native MacOS builds
  * Native Windows builds
  * WASM Web builds
  * Tiny/micro Arduino builds

## Install

Check out the install guides at [kata.wtf/install](https://kata.wtf/install)

Once the project is working, these commands should work:


### apt-based (Debian/Ubuntu/...)

```shell
$ sudo apt install kata
```


### brew-based (MacOS homebrew)

```shell
$ brew install kata
```

## Build

Check out the build guides at [kata.wtf/build](https://kata.wtf/build)

Or, find your build guide here:

  * UNIX/Linux-like OSes: [kata.wtf/build/unix](https://kata.wtf/build/unix)
    * Tested on Ubuntu 22.04
  * Windows OSes: [kata.wtf/build/windows](https://kata.wtf/build/windows)

### Build UNIX

To build on UNIX-like platforms, you can use the following:

```shell
$ make
```


## Develop

First, clone the repository:

```shell
$ git clone https://github.com/kataproj/kata.git
$ cd kata
```

Then, run the following for your platform:

### Setup apt-based (Debian/Ubuntu/...)

```shell
$ sudo apt install gcc 
```

### Run Tests

To run a series of tests, run:

```shell
$ make test
```

### Run Performance Analysis

To run a series of performance tests/analysis, run:

```shell
$ make perf
```

TODO: actually make this work

## Software Used

  * [libbf](https://bellard.org/libbf/): used for arbitrary precision arithmetic (`kint` and `kreal`)

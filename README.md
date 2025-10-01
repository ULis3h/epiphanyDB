<p align="center">
  <a href="https://epiphanydb.io"><img src="docs/assets/header.svg" alt="EpiphanyDB Header" width="100"/></a>
</p>

<h1 align="center"><a href="https://epiphanydb.io" style="text-decoration:none; color:inherit;">EpiphanyDB</a></h1>

<p align="center">
  <a href="#"><img src="https://img.shields.io/badge/Embedded-DB-blue" alt="Static Badge"/></a>
  <a href="https://github.com/ULis3h/epiphanyDB/actions/workflows/codeql.yml"><img src="https://github.com/ULis3h/epiphanyDB/actions/workflows/codeql.yml/badge.svg?event=status" alt="CodeQL Advanced"/></a>
  <a href="#"><img src="https://img.shields.io/github/license/mashape/apistatus" alt="GitHub License"/></a>
</p>


`EpiphanyDB` is a general database management system that supports running in both operating system and non-operating system environments. It provides a unified storage interface and can adapt to a variety of mainstream database systems.

## Features

### Core Features

### Storage Features

## Supported Platforms
- Windows
- Unix-like
- MacOS


## Functionality

## Module Design
Except for modules implemented with templates, the external interfaces of all modules are provided in the `include` directory and implemented in their respective module directories. This means that `include` only contains declarations. Therefore, to update a module, one only needs to implement the corresponding declarations in `include` without modifying other modules. The module organization is shown in the figure below:
![img](docs/assets/module_arch.png)


* [X] [Common Components (common)](docs/design/common/README.md)
* [ ] [Buffer Pool Module](docs/design/pager/README.md)
* [ ] [core](docs/design/core/README.md)
* [ ] [Index Module (index)](docs/design/index/README.md)
* [ ] [mco_n](docs/design/mco_n/README.md)
* [ ] [os](docs/design/os/README.md)
* [ ] [txn](docs/design/txn/README.md)

## Programming API

* [X] [epiphanydb capi](docs/user/user_capi.md)
* [ ] [epiphanydb cppapi](docs/user/user_cppapi.md)


## Build

Clone the source code:
```
$ git clone http://192.168.3.248:9091/root/enpiphany.git
```

### Unix-like
```bash
$ mkdir build && cd build
```
```bash
$ cmake ..
```


### Windows - MinGW
```bash
$ mkdir build && cd build
```

```bash
$ cmake -G "MinGW Makefiles" ..
```

```bash
$ make
```

## Testing

### Basic Tests

## TODO

- os  
  1. `TLFS` algorithm.  
- common  
  1. Memory pool implementation [Done] 
  2. LRU cache eviction policy [Done] [To be tested]  
  3. LRU-K cache eviction policy

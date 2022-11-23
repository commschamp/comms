# Overview
This project contains the **COMMS Library**, which is a core component of the
[CommsChampion Ecosystem](https://commschamp.github.io).
It comes to help in developing binary communication protocols, with main focus on
**embedded systems** with limited resources (including 
bare-metal ones) and choosing **C++(11)** programming language to do so. 

**COMMS** is the **C++(11)** headers only, platform independent library, 
which makes the implementation of a communication
protocol to be an easy and relatively quick process. It provides all the necessary
types and classes to make the definition of the custom messages, as well as
wrapping transport data fields, to be simple declarative statements of type and
class definitions. These statements will specify **WHAT** needs to be implemented. 
The **COMMS** library internals handle the **HOW** part.

The internals of the **COMMS** library is mostly template classes which use 
multiple meta-programming techniques. As the result, only the functionality,
required by the protocol being developed, gets compiled in, providing the best code size and
speed performance possible. The down side is that compilation process may
take a significant amount of time and consume a lot of memory.

The **COMMS** library allows having **single implementation** of the binary 
protocol messages, which can be re-compiled and used for any possible application:
bare-metal with constrained resources, Linux based embedded systems, even 
independent GUI analysis tools.

The **COMMS** library was specifically developed to be used in **embedded** systems
including **bare-metal** ones. It doesn't use exceptions and/or RTTI. It also
minimises usage of dynamic memory allocation and provides an ability to exclude
it altogether if required, which may be needed when developing **bare-metal**
embedded systems. 

Core ideas and architecture of the **COMMS** library is described in
[Guide to Implementing Communication Protocols in C++](https://commschamp.github.io/comms_protocols_cpp/) free e-book.
However, at this stage the library internals are much more advanced and sophisticated, than
examples in the e-book, please don't treat the latter as a guide to library's internals.

Over the years the 
**COMMS Library** grew with features and accumulated
multiple nuances to be remembered when defining a new protocol. In order to
simplify protocol definition work, a separate toolset, called 
[commsdsl](https://github.com/commschamp/commsdsl) (hosted as separate repository), 
has been developed. It allows much easier and simpler definition of the protocol, 
using schema files written in XML based domain specific language, called 
[CommsDSL](https://github.com/commschamp/CommsDSL-Specification). The toolset
will generate a C++11 code that defines the protocol using appropriate
**COMMS Library** classes and functions. Many binary protocols 
may have nuances that are difficult to express in the existing schema language. 
In order to still allow usage of the schema files for the protocol definition, the
toolset allows injection of extra custom code to modify or extend the generated
one. The generated code itself is also highly compile time customisable. It
allows selection of custom data structures for data storage as well as polymorphic
interfaces relevant to the application being developed.

As the result, manual implementation of binary communication protocols 
**from scratch** using 
**COMMS Library** is not recommended and should be avoided. Please use
**commsdsl2comms** code generator from 
[commsdsl](https://github.com/commschamp/commsdsl) project, which also
lists multiple available protocols (with usage examples) that can be used
as reference.

The [cc_tutorial](https://github.com/commschamp/cc_tutorial/) repository 
contains a full tutorial how to use the **COMMS Library** in conjunction with 
[commsdsl2comms](https://github.com/commschamp/commsdsl) code generator.

# Library Documentation
The COMMS library is [doxygen documented](https://commschamp.github.io/comms_doc/).
It contains detailed tutorial and examples how to use the provided 
classes and functions. The same documentation can
downloaded as **doc_comms_vX.zip** archive from the
[release artefacts](https://github.com/commschamp/comms/releases).

The documentation contains two major parts (pages):

- [How to Use Defined Custom Protocol](https://commschamp.github.io/comms_doc/page_use_prot.html) - 
    explains how to use the defined protocol, useful in understanding the protocol
    definintion code generated by the 
    [commsdsl2comms](https://github.com/commschamp/commsdsl).
- [How to Define New Custom Protocol](https://commschamp.github.io/comms_doc/page_define_prot.html) -
    explains how to define new protocol, useful in understanding how to provide custom
    functionality when the default one generated by the 
    [commsdsl2comms](https://github.com/commschamp/commsdsl) is 
    insufficient or incorrect.

# How to Build and Use
Detailed instructions on how to build and install can be
found in [doc/BUILD.md](doc/BUILD.md) file.

Instructions on how to use the **COMMS Library** in another CMake
project can be found in [doc/CMake.md](doc/CMake.md) file.

# Supported Compilers
The **COMMS Library** requires proper C++11 (or later) support which
is provided by the following compilers:
- **GCC**: >=4.8
- **Clang**: >=3.9
- **MSVC**: >= 2015

# Branching Model
This repository will follow the 
[Successful Git Branching Model](http://nvie.com/posts/a-successful-git-branching-model/).

The **master** branch will always point to the latest release, the
development is performed on **develop** branch. As the result it is safe
to just clone the sources of this repository and use it without
any extra manipulations of looking for the latest stable version among the tags and
checking it out.

# Contact Information
For bug reports, feature requests, or any other question you may open an issue
here in **github** or e-mail me directly to: **arobenko@gmail.com**. I usually
respond within 24 hours.
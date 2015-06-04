# Asymptus Complexity Analyzer

Asymptus is a tool for automatic inference of function complexity. It uses a hibrid approach, 
mixing static analysis and dynamic profiling, in order to provide more accurate results.


# Installing

Asymptus requires LLVM 3.4 to performe static analysis. To install LLVM 3.4 you can follow
these tutorials: [downloading a specific version of LLVM](http://llvm.org/docs/GettingStarted.html#checkout-llvm-from-subversion)
and [installing LLVM](http://llvm.org/docs/GettingStarted.html#getting-started-quickly-a-summary).
Note that its necessary to also install clang, the LLVM front-end.

Once you have a working installation of LLVM 3.4, you can download the Asymptus binaries or
source code and install it.

## Usage

Asymptus executes over either a C/C++ file or LLVM bytecodes. The simplest way to run our tool
is to execute `./asymptus input_file.(c|cpp|bc)`. It will identify how many command line arguments
the input program reads and generate numeric values for them. You may also use the following
options:
```
 --args type ....         Allows the user to specify the types of command line arguments.
                          It is necessary to specify as many argument as the program reads
                          in order to have a correct execution. The types may be one of the
                          following: int, long, float, double, num, char or string.

 --mix ARG ..             Allows the user to mix random generated arguments with concrete 
                          data. When using this option, the random arguments types have to 
                          come between {}. For instance, --mix myInput {int}.
                          
 --man                    This option will ask the user for only concrete inputs. Asymptus will
                          ask for the inputs of each desired execution. An empty line means all
                          data has been provided.
                              
 -v                       Verbose mode. Prints the polynomial for each loop inside a function.
 
 -h or --help             Prints this usage message.
```

## Download

- [Linux binaries](linx here)
- [Mac OS X binaries](link here)

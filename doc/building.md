# Building the project

## Clone

To clone the current project in your working setup simply follows the list of commands below:

```shell
git clone http://mpegx.int-evry.fr/software/haptics/rm0.git
```

***<u>Note :</u>*** The previous command clone the project through *HTTPS* protocol, but the *SSH* one is still possible. To do it, you simply need to replace use this other command:

```shell
git clone git@mpegx.int-evry.fr/software/haptics/rm0.git
``` 

## Compilation

### Windows

We recommand to use Visual Studio 19. CMake is supported by this tool, the clone folder can be directly opened in VS. Please refer to the [official documentation](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170).

An alternative is to generate a Visual Studio project with the [cmake-gui](https://cmake.org/runningcmake/).

Don't forget to manually activate [clang-tidy](https://devblogs.microsoft.com/cppblog/code-analysis-with-clang-tidy-in-visual-studio/) (see [coding_style.md](coding_style.md))

Then select *Build/Build all* to start the compilation.

### Linux

```shell
mkdir build & cd build
ccmake ..
make
```

## Testing

Run [unit tests](testing.md) to make sure that everything is ok. Otherwise contact the software coordinator.
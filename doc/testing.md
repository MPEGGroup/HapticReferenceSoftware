# Testing

## Unit tests

Unit tests are performed with [catch2](https://github.com/catchorg/Catch2). The framework is automatically downloaded by cmake.

For each *.cpp* file, a file *.test.cpp* should be created. **Each contributor is reponsible for the unit tests associated to the implemented contribution.**

Once the project is compiled, unit tests can be started with
```shell
make test
```
for Linux and directly from the menu *Test/Run CTests for RM0* in MS Visual Studio.

Pushed code that does not provide enough unit tests, and does not pass these tests will not be merged.
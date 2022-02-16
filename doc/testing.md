# Testing

## Unit tests

Unit tests are performed with [catch2](https://github.com/catchorg/Catch2). The framework is automatically downloaded by cmake.

For each *.cpp* file, a file *.test.cpp* should be created. **Each contributor is reponsible for the unit tests associated to the implemented contribution. Pushed code that does not provide enough unit tests, and does not pass these tests will not be merged.**

Once the project is compiled, unit tests can be started with
```shell
make test
```
for Linux and directly from the menu *Test/Run CTests for RM0* in MS Visual Studio.

Tests can also be run independently, executables are built during the compilation. They provide more detailed information.

These tests will be automatically performed when a merge is requested. The reference machine is a Linux AWS instance handled by InterDigital. A merge will not be accepted until all tests are passing. Please contact the software coordinator if you encounter any issue.

## PSNRs

PSNRs are automatically computed at the test stage of the CI pipeline. Data are fetch from the mpeg content website, verified, and processed by python scripts. The file to be tested are listed in [test/psnr_2k.json](../test/psnr_2k.json).

If you want to compute the PSNRs locally, you need to first download the data from [mpeg content](https://mpegfs.int-evry.fr/mpegcontent) : MPEG-I/Part40-HapticSupport (right click on the left menu, Download). Password is provided by the AHG group chairman.

Then data integrity must be checked (md5 hash are compared to a reference *list_files.md5*). Run this command from the root of the repo :
```shell
python ./scripts/tools/check_data.py --data_dir ~/data --md5 ./test/list_files.md5
```

Finally PSNRs are computed with :
```shell
pytest ./scripts/test/integration_tests.py --autopad 
      --install_dir ~/install/ 
      --data_dir ~/data/Test/
      --junitxml=reports/report.xml -o junit_family="xunit1" 
      -n auto --csv reports/report.csv 
      --psnr_ref ./test/psnr_2k.json
      --csv-columns host,system,python_version,function,status,success,duration,properties_as_columns
```
The [pytest](https://docs.pytest.org/) framework is used to run all the tests. The plugins *xdist* and *csv* are required. The [dockerfile](../scripts/docker/integration_tests/) may be useful to setup a test environment. It is assumed that RM0 is compiled and installed (in *~/install/* for instance here).

The script produces a *report.xml* file directly ingested by the gitlab CI system, and a *report.csv* file that can be used to compute statistics. The argument *psnr_ref* may be removed to compute the PSNRs of all wav files in the data folder.

If a single file have to be processed, the following script may be called :
```shell
python --autopad ./scripts/test/psnr.py <original_file.wav> <decompressed_file.wav>
```
The autopad option is optional and allows comparaison of two files with a different samples number.
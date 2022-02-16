# Instructions

This docker image should be built on the instance hosting the gitlab runner. It will be used during the testing stage of the CI pipeline.

The file *mpeg_ftp.json* contains the information allowing the connection to the MPEG FTP. The password entry should be set before building the docker image.

```shell
docker build -t gcc-cmake-python:RM0 ./scripts/docker/integration_tests/
```
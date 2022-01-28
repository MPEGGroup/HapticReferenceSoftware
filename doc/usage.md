# Usage

Assuming that the executables has been [installed](building.md), Encoder and Synthesizer are located in the *bin* folder. They can be used as follows:

## Encoding

```shell
usages: ./Encoder [-h] [{-v, -q}] -f <FILE> [-o <OUTPUT_FILE>]

This piece of software converts binary encoded RM0 files submitted to the MPEG CfP call for Haptic standardization into their human-readable format

positional arguments:
        -f, --file <FILE>               file to convert

optional arguments:
        -h, --help                      show this help message and exit
        -v, --verbose                   be more verbose
        -q, --quiet                     be more quiet
        -o, --output<OUTPUT_FILE>       output file
```

Example
```shell
 ./Encoder -f IDCC-vib-Paper-8kHz-16-pad.wav -o IDCC-vib-Paper-8kHz-16-pad.gmpg
```

### Synthesizing

```shell
usages: ./Synthesizer [-h] [{-v, -q}] -f <FILE> [-o <OUTPUT_FILE>]

This piece of software converts binary encoded RM0 files submitted to the MPEG CfP call for Haptic standardization into their human-readable format

positional arguments:
        -f, --file <FILE>               file to convert

optional arguments:
        -h, --help                      show this help message and exit
        -v, --verbose                   be more verbose
        -q, --quiet                     be more quiet
        -o, --output<OUTPUT_FILE>       output file
```

Example
```shell
 ./Synthesizer -f IDCC-vib-Paper-8kHz-16-pad.gmpg -o IDCC-vib-Paper-8kHz-16-pad.wav
```
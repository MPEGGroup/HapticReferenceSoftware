# Usage

Assuming that the executables has been [installed](building.md), Encoder, Decoder and Synthesizer are located in the *bin* folder. They can be used as follows:

## Encoding

```shell
usages: Encoder [-h] -f <FILE> -o <OUTPUT_FILE> [-b] [-kin]

This piece of software encodes an input file into a MPEG Haptics file

positional arguments:
        -f, --file <FILE>              file to convert
        -o, --output <OUTPUT_FILE>     output file

optional arguments:
        -h, --help                                            show this help message and exit
        -b, --binary                                          the file will be encoded into its binary format. If not provided the encoder will output a file in a human-readable format
        -r, --refactor                                        the file will be refactored. Every effect used multiple times will be placed in the library and replaced by a referennce
        -l, --linearize                                       the file will be linearized. Every referenced effect from the library will be copied into the main timeline.
        -kb, --bitrate                                        target bitrate of the encoded file
        -bu,                                                  wavelet bitbudget, if custom setting needed
        -bl,                                                  wavelet block length, if custom setting needed
        -cf,                                                  cutoff frequency used to split pcm signals in high and low frequencies. Default value is 72 Hz. If the value is set to zero, the signal will not be split.
        --disable-wavelet,                                    the encoder will encode the data using a single vectorial band for low frequencies. This argument will only affect PCM input content.
        --disable-vectorial,                                  the encoder will encode the data using a single wavelet band for high frequencies. This argument will only affect PCM input content.

```

Examples:
```shell
 ./Encoder -f IDCC-vib-Paper-8kHz-16-pad.wav -o IDCC-vib-Paper-8kHz-16-pad.hjif
```

```shell
 ./Encoder -f IDCC-vib-Paper-8kHz-16-pad.wav -o IDCC-vib-Paper-8kHz-16-pad.hmpg --binary -kb 16
```

### Decoder
usages: Decoder [-h] -f <FILE> -o <OUTPUT_FILE>

This piece of software converts MPEG Haptics binary encoded HMPG files into their human-readable format

positional arguments:
        -f,--file <FILE>                                      file to convert
        -o,--output <OUTPUT_FILE>\                            output file

optional arguments:
        -h,--help                                             show this help message and exit

Example:
```shell
 ./Decoder -f IDCC-vib-Paper-8kHz-16-pad.hmpg -o IDCC-vib-Paper-8kHz-16-pad.hjif
```

### Synthesizing

```shell
usages: Synthesizer [-h] -f <FILE> -o <OUTPUT_FILE> [-fs <FREQUENCY_SAMPLING>] [--pad <PADDING>] [--generate_ohm]

This piece of software ingest an MPEG Haptics binary encoded RM1 files (into its human-readable format) and evaluate it to output a PCM file corresponding to the synthezised input

positional arguments:
         -f,--file <FILE>                                     file to ingest
         -o,--output <OUTPUT_FILE>                            output file

optional arguments:
         -h,--help                                            show this help message and exit
         -fs,--sampling_frequency <FREQUENCY_SAMPLING>        the frequency sampling used to synthezised the output (default value is DEFAULT_FS Hz)
         --pad <PADDING>                                      add a padding on the resulting file. The padding provided should be in milliseconds
         --generate_ohm                                       generate an output ohm files corresponding to the file metadata

```

Example
```shell
 ./Synthesizer -f IDCC-vib-Paper-8kHz-16-pad.hjif -o IDCC-vib-Paper-8kHz-16-pad.wav
```

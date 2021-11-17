# MPEG Haptics Reference Model

![3 contributors](https://img.shields.io/badge/contributors-3-brightgreen.svg?style=flat-square)
![C++ | Json language](https://img.shields.io/badge/type-C++%20|%20glTF-blue.svg?style=flat-square)
![Version](https://img.shields.io/badge/version-RM0-blueviolet.svg?style=flat-square)


> This project answer the Call for Proposals (CfP) on a Coded Representation of Haptics - Phase 1 (*refs. m56255_v6*).
>
> A reference model is proposed in collaboration between **InterDigital Corporation**, **Technical University of Munich** and **Interhaptics** in this repository. This project contains 3 reference software corresponding to an encoder, a decoder and a synthesizer.

---

## Table of Contents

[[_TOC_]]

---

This git is decomposed into 3 majors folders:

+ **RM0_Encoder:** *this C++ project is able to ingest a reference file format (AHAP, IVS and WAV) and encode it in the RM0 format (human-readable or binary-compressed)*
+ **RM0_Decoder:** *this C++ project is able to ingest an RM0 file binary compressed and transcode it into the human-readable format*
+ **RM0_Synthesizer** *this project (coding language to determine) is able to ingest an RM0 file in human-readable format and generate a wav file corresponding to the appropriate haptic feedback*

## Build instructions

This piece of software requires Windows 10 with Visual Studio 2019, or Linux with gcc-10/clang-11. Git and cmake are needed to clone and generate the project. Instructions are provided in [doc/building.md](doc/building.md).

## Contributing

### Workflow

If you are willing to contribute to the project, please follow this workflow:

1. Create an issue with the appropriate label, documentation, contribution number, etc. Issues can be created from the left panel menu: [Issues/List](http://mpegx.int-evry.fr/software/haptics/rm0/-/issues), New issue button.
2. Fork a branch from *develop* as indicated in the *Git* section
3. Commit you work in this branch according to the commit convention
4. Write and run [unit tests](doc/testing.md) before any push
5. Push your branch to the repo. A branch can be pushed at any time, no need for the task to be completed
6. Once the task is complete and all the code committed, request a merge. Go the left panel menu: [Repository/Branches](http://mpegx.int-evry.fr/software/haptics/rm0/-/branches), Merge request button associated to your branch
7. A software coordinator will review the code and merge the branch if all the rules are respected, and all the tests are ok. The branch will be then deleted.
8. Close the issue.

### Coding style

The coding style and rules can be found in [doc/coding_style.md](doc/coding_style.md). The use of the these rules is mandatory for all software implementation work. Changes that do not comply with these rules will not be accepted by the software coordinators.

### Git

Branching architecture and commmit convention are described in section [doc/git.md](doc/git.md).

### Testing

Unit tests are described in section [doc/testing.md](doc/testing.md).

## File structure

### Human-readable format

![RM0-human-readable-format](diagrams/Rendered/RM0HumanReadableFormat.png "RM0 human-readable format")

### Binary bitstream format

![RM0-bitstream-format](diagrams/Rendered/RM0BitsreamFormat.png "bitstream structure")

---

## Software architecture

![RM0-General-softwares](diagrams/Rendered/RM0GeneralSoftwares.png "General file software structure")

### RM0 Encoder structure

![RM0-Encoder](diagrams/Rendered/RM0EncoderStructure.png "RM0 Encoder structure")

### RM0 Decoder structure

![RM0-Decoder](diagrams/Rendered/RM0DecoderStructure.png "RM0 Decoder structure")

---






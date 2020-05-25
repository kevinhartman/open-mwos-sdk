[![CI](https://github.com/kevinhartman/os9-toolchain/workflows/CI/badge.svg)](https://github.com/kevinhartman/os9-toolchain/actions?query=workflow%3ACI)

This project aims to be an open source toolchain for developing applications for the Microware OS-9 portable operating system. It's implemented using publicly available documentation.

## Status
This project is in early stages, and probably won't be of much use at the moment.

### Tools
| tool    | description                                             | status                                                                                                                        | next feature                                      |
|---------|---------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------|
| `ident` | Show info about an OS-9 module.                         | Working.<br><br> Supports MIPS BE and i386. Others may work, but may need endian flipping without proper support.             |                                                   |
| `amips` | Assemble OS-9 flavor MIPS assembly to ROF object files. | Partially working.<br><br> Can produce valid code / ROFs, but is missing support for many directives and pseudo instructions. | Implement ROF writer support for data references. |

Note: Conformance with original tooling is not necessarily a goal (at least right now).

## Road map
The current goal is to implement minimal toolchain support for a single CPU target (MIPS BE I), building a collection of reusable libraries that enable additional targets to be added without *much* effort by the community.

The `Assembler` and `ROF` libraries are currently in active development.

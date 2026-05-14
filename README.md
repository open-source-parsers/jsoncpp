# JsonCpp

[![Conan Center](https://img.shields.io/conan/v/jsoncpp)](https://conan.io/center/recipes/jsoncpp)
[![badge](https://img.shields.io/badge/license-MIT-blue)](https://github.com/open-source-parsers/jsoncpp/blob/master/LICENSE)
[![badge](https://img.shields.io/badge/document-doxygen-brightgreen)](http://open-source-parsers.github.io/jsoncpp-docs/doxygen/index.html)
[![Coverage Status](https://coveralls.io/repos/github/open-source-parsers/jsoncpp/badge.svg?branch=master)](https://coveralls.io/github/open-source-parsers/jsoncpp?branch=master)

[JSON][json-org] is a lightweight data-interchange format. It can represent
numbers, strings, ordered sequences of values, and collections of name/value
pairs.

[json-org]: http://json.org/

JsonCpp is a C++ library that allows manipulating JSON values, including
serialization and deserialization to and from strings. It can also preserve
existing comment in deserialization/serialization steps, making it a convenient
format to store user input files.

## Project Status

JsonCpp is a mature project in maintenance mode. Our priority is providing a stable,
reliable JSON library for the long tail of C++ development.

### Current Focus

* **Security:** Addressing vulnerabilities and fuzzing results.
* **Compatibility:** Ensuring the library builds without warnings on the latest versions of GCC,
Clang, and MSVC.
* **Reliability:** Fixing regressions and critical logical bugs.

### Out of Scope

* **Performance:** We are not competing with SIMD-accelerated or reflection-based parsers.
* **Features:** We are generally not accepting requests for new data formats or major API changes.

JsonCpp remains a primary choice for developers who require comment preservation and support for
legacy toolchains where modern C++ standards are unavailable. The library is intended to be a
reliable dependency that does not require frequent updates or major migration efforts.

## A note on backward-compatibility

* **`1.y.z` (master):** Actively maintained. Requires C++11.

* **`0.y.z`:** Legacy support for pre-C++11 compilers. Maintenance is limited to critical security fixes.

* **`00.11.z`:** Discontinued.

Major versions maintain binary compatibility. Critical security fixes are accepted for both the `master` and `0.y.z` branches.

## Integration

> [!NOTE]
> Package manager ports (vcpkg, Conan, etc.) are community-maintained. Please report outdated versions or missing generators to their respective repositories.

### Meson

```sh
meson wrap install jsoncpp
```

### Amalgamated source

For projects requiring a single-header approach, JsonCpp provides a script to generate an amalgamated source and header file.

You can generate the amalgamated files by running the following Python script from the top-level directory:

```sh
python3 amalgamate.py
```

This will generate a `dist` directory containing `jsoncpp.cpp`, `json/json.h`, and `json/json-forwards.h`. You can then drop these files directly into your project's source tree and compile `jsoncpp.cpp` alongside your other source files.

## Documentation

Documentation is generated via [Doxygen](http://open-source-parsers.github.io/jsoncpp-docs/doxygen/index.html). 
Additional information is available on the [Project Wiki](https://github.com/open-source-parsers/jsoncpp/wiki).

## License

JsonCpp is licensed under the MIT license, or public domain where recognized.
See [LICENSE](./LICENSE) for details.

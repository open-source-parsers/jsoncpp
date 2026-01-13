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

## Documentation

[JsonCpp documentation][JsonCpp-documentation] is generated using [Doxygen][].

[JsonCpp-documentation]: http://open-source-parsers.github.io/jsoncpp-docs/doxygen/index.html
[Doxygen]: http://www.doxygen.org

## A note on backward-compatibility

* `1.y.z` is built with C++11.
* `0.y.z` can be used with older compilers.
* `00.11.z` can be used both in old and new compilers.
* Major versions maintain binary-compatibility.

### Special note

The branch `00.11.z`is a new branch, its major version number `00` is to show
that it is different from `0.y.z` and `1.y.z`, the main purpose of this branch
is to make a balance between the other two branches. Thus, users can use some
new features in this new branch that introduced in 1.y.z, but can hardly applied
into 0.y.z.

## Using JsonCpp in your project

### The vcpkg dependency manager

You can download and install JsonCpp using the [vcpkg](https://github.com/Microsoft/vcpkg/)
dependency manager, which has installation instruction dependent on your
build system. For example, if you are in a CMake project, the
[CMake install tutorial](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell)
suggests the follow installation method.

First, clone and set up `vcpkg`.

```sh
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
```

Then, create a [vcpkg.json manifest](https://learn.microsoft.com/en-us/vcpkg/reference/vcpkg-json),
enabling manifest mode and adding JsonCpp to the manifest's dependencies list.

```sh
    vcpkg new --application
    vcpkg add port jsoncpp
```

> [!NOTE]: you can use vcpkg in either classic mode or manifest mode (recommended).

#### Classic mode

If your project does not have a `vcpkg.json`,
your project is in [Classic mode](https://learn.microsoft.com/en-us/vcpkg/concepts/classic-mode)
you can install JsonCpp by directly invoking the `install` command:

```sh
    vcpkg install jsoncpp
```

### Manifest mode

If your project *does* have a vcpkg.json manifest, your project is in [Manifest mode](https://learn.microsoft.com/en-us/vcpkg/concepts/manifest-mode)
and you need to add JsonCpp to your package manifest dependencies, then invoke
install with no arguments.

```sh
    vcpkg add port jsoncpp
    vcpkg install
```

Example manifest:

```sh
{
    "name": "best-app-ever",
    "dependencies": [ "jsoncpp" ],
}
```

> [!NOTE] The JsonCpp port in vcpkg is kept up to date by Microsoft team members and community contributors.
> If the version is out of date, please [create an issue or pull request](https://github.com/Microsoft/vcpkg)
> on the vcpkg repository.

### Conan package manager

You can download and install JsonCpp using the [Conan](https://conan.io/)
package manager:

```sh
    conan install -r conancenter --requires="jsoncpp/[*]" --build=missing
```

The JsonCpp package in Conan Center is kept up to date by [ConanCenterIndex](https://github.com/conan-io/conan-center-index)
contributors. If the version is out of date, please create an issue or pull request on the
Conan Center Index repository.

### Amalgamated source

See the [Wiki entry on Amalgamated Source](https://github.com/open-source-parsers/jsoncpp/wiki/Amalgamated-(Possibly-outdated)).

### The Meson Build System

If you are using the [Meson Build System](http://mesonbuild.com), then you can
get a wrap file by downloading it from [Meson WrapDB](https://mesonbuild.com/Wrapdb-projects.html),
or simply use `meson wrap install jsoncpp`.

### Other ways

If you have trouble, see the
[Wiki](https://github.com/open-source-parsers/jsoncpp/wiki), or post a question
as an Issue.

## License

See the [LICENSE](./LICENSE) file for details. In summary, JsonCpp is licensed
under the MIT license, or public domain if desired and recognized in your
jurisdiction.

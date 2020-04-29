# JsonCpp

[JSON][json-org] is a lightweight data-interchange format. It can represent
numbers, strings, ordered sequences of values, and collections of name/value
pairs.

[json-org]: http://json.org/

JsonCpp is a C++ library that allows manipulating JSON values, including
serialization and deserialization to and from strings. It can also preserve
existing comment in unserialization/serialization steps, making it a convenient
format to store user input files.

## Release notes
The 00.11.z branch is a new version, its major version number `00` is to shows that it is
different from branch 0.y.z and 1.y.z. The main purpose of this branch is to give users a
third choice, that is, users can only have a copy of the code, but can build in different environments,
so it can be used with old or newer compilers.
The benefit is that users can used some new features in this new branch that introduced in 1.y.z,
but can hardly applied into 0.y.z.

## Documentation

[JsonCpp documentation][JsonCpp-documentation] is generated using [Doxygen][].

[JsonCpp-documentation]: http://open-source-parsers.github.io/jsoncpp-docs/doxygen/index.html
[Doxygen]: http://www.doxygen.org


## A note on backward-compatibility

* `1.y.z` is built with C++11.
* `0.y.z` can be used with older compilers.
* `00.11.z` can be used with older compilers , with new features from `1.y.z`
* Major versions maintain binary-compatibility.


## Using JsonCpp in your project

### The vcpkg dependency manager
You can download and install JsonCpp using the [vcpkg](https://github.com/Microsoft/vcpkg/) dependency manager:

    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
    ./vcpkg integrate install
    vcpkg install jsoncpp

The JsonCpp port in vcpkg is kept up to date by Microsoft team members and community contributors. If the version is out of date, please [create an issue or pull request](https://github.com/Microsoft/vcpkg) on the vcpkg repository.

### Amalgamated source
https://github.com/open-source-parsers/jsoncpp/wiki/Amalgamated-(Possibly-outdated)

### The Meson Build System
If you are using the [Meson Build System](http://mesonbuild.com), then you can get a wrap file by downloading it from [Meson WrapDB](https://wrapdb.mesonbuild.com/jsoncpp), or simply use `meson wrap install jsoncpp`.

### Other ways
If you have trouble, see the [Wiki](https://github.com/open-source-parsers/jsoncpp/wiki), or post a question as an Issue.

## License

See the `LICENSE` file for details. In summary, JsonCpp is licensed under the
MIT license, or public domain if desired and recognized in your jurisdiction.

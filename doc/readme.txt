The API documentation is generated with doxygen (https://www.doxygen.org)
and published to https://open-source-parsers.github.io/jsoncpp/ by the
"docs" GitHub Actions workflow on every push to master.

To build it locally, from the top of the source tree:

    python3 doxybuild.py --open            # HTML only
    python3 doxybuild.py --with-dot --open # also class/call graphs (needs graphviz)

Output goes to dist/doxygen/. Any doxygen warning fails the build.

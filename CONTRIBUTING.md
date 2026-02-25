# Contributing to V2V Database

First off, thanks for taking the time to contribute!

The following is a set of guidelines for contributing to V2V Database on GitHub. These are mostly guidelines, not rules. Use your best judgment, and feel free to propose changes to this document in a pull request.

## Code of Conduct

This project and everyone participating in it is governed by the [Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code.

## How Can I Contribute?

### Reporting Bugs

This section guides you through submitting a bug report for V2V Database. Following these guidelines helps maintainers and the community understand your report, reproduce the behavior, and find related reports.

-   **Use a clear and descriptive title** for the issue to identify the problem.
-   **Describe the exact steps to reproduce the problem** in as many details as possible.
-   **Describe the behavior you observed after following the steps** and point out what exactly is the problem with that behavior.
-   **Explain which behavior you expected to see instead and why.**

### Suggesting Enhancements

This section guides you through submitting an enhancement suggestion for V2V Database, including completely new features and minor improvements to existing functionality.

-   **Use a clear and descriptive title** for the issue to identify the suggestion.
-   **Provide a step-by-step description of the suggested enhancement** in as many details as possible.
-   **Explain why this enhancement would be useful** to most users.

### Pull Requests

The process described here has several goals:

-   Maintain strict quality standards.
-   Fix problems that are important to users.
-   Engage the community in working toward the best possible code.

1.  Fork the repo and create your branch from `main`.
2.  If you've added code that should be tested, add tests.
3.  If you've changed APIs, update the documentation.
4.  Ensure the test suite passes.
5.  Make sure your code follows the existing coding standards (Google C++ Style Guide is generally preferred).
6.  Issue that pull request!

## Styleguides

### C++ Styleguide

-   We generally follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).
-   Use `clang-format` to format your code before submitting.

### CMake Styleguide

-   Use modern CMake practices (target-based approach).
-   Keep `CMakeLists.txt` clean and readable.

## Building from Source (For Contributors)

If you are modifying the codebase, you can build the project using the provided scripts:

-   **Windows**: `build-all.bat`
-   **Linux/macOS**: `./build-all.sh`

These scripts will compile the project and place the artifacts in the `releases/` directory.

## License

By contributing, you agree that your contributions will be licensed under its Apache License 2.0.

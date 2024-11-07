# clang-tool
clang-tool is a simple and powerful project template for clang-based tools using libtooling[1]. It helps getting started to write standalone tools useful for refactoring, static code analysis, auto-completion etc.

## Installation
Install the necessary LLVM and Clang headers and libraries for your system and run:

```bash
sudo apt-get install llvm clang
```

```bash
git clone https://github.com/firolino/clang-tool
cd clang-tool
mkdir build
cd build
cmake ..
make
```

## Usage
Before running the script, ensure it has the necessary permissions:

```bash
chmod +x catchbyreferencetransformer.sh
```

The current usage of the tool is as simple as running the provided script:

```bash
./catchbyreferencetransformer.sh
```

The `catchbyreferencetransformer.sh` script ensures that exceptions in your C++ code are caught by reference rather than by value.
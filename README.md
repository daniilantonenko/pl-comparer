# Package list comprare
Shared library for comparing binary packages of 2 arch. 
Works only with json sorted in descending order!

Output JSON structure:
- **missing** (all packages that are in the 1st but not in the 2nd)
- **extra** (all packages that are in the 2nd but are not in the 1st)
- **updated** (all packages whose version-release is greater in the 1st than in the 2nd)

## Examples
Some examples to give you an idea how to use the class. Beside the examples below, you may want to browse the standalone example files. 

Has a dependency on [nlohmann::json](https://github.com/nlohmann/json).

### Add to project
```cpp
#include <pl-comparer/pl-comparer.h>
using namespace PackageListComparer;
using json = nlohmann::json;
```

### Getting binary packages
Specify architecture for comparison (ALT Linux p10). For example: X86_64, ppc64le, i586, armh, aarch64, noarch.
```cpp
std::string branch = "p10";
std::string first = "aarch64";
std::string second = "x86_64";
json source = loadJson(branch, first);
json target = loadJson(branch, second);
```

### Run comparison
Make comparison and get result:
```cpp
nlohmann::json resultJson = comparing(first, second);
```

### CLI utility
Then the command line is given as:
```bash
PLcomparer -b p10 -s aarch64 -t x86_64 -p
```

- `b [branch]` Branch selection (last branch by default)
- `s [arch]` Arch source name.
- `t [arch]` Arch target name.
- `p` Print Json comparison result.
- `d` Print beautiful Json dump comparison result.
- `f` Saving beautiful Json dump comparison result.
- `h` Print this help message and exit.


### Installing the library from source files
Automatic compilation of CMake with subsequent installation into the system:
```bash
./install.sh
```

## Plan
- [X] Basic comparison functionality
- [X] Get lists of binary packages
- [X] CLI utility
- [X] Installation by FHS standard
- [X] Selecting a branch
- [ ] Getting arch by API
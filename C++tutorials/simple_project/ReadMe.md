
## Directory Structure
simple_project/
├── CMakeLists.txt          # Root config
├── lib/
│   ├── CMakeLists.txt      # Library config
│   ├── hello.cc
│   └── hello.hh
└── app/
    ├── CMakeLists.txt      # App config
    └── main.cc


## Build It
```bash
mkdir build
cd build
cmake ..
make
./app/myapp

```
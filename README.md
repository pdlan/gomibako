Gomibako is a blog software in modern C++. The name is from another blog software [dustbin](https://github.com/pdlan/dustbin),
as gomibako(ゴミ箱) means dustbin in Japanese.

## Features:

 -  No database.
    - All data and configuration are storaged in memory or text files.
    - It's very easy to create / edit / delete articles / pages mannualy.
 - In pure modern C++.
 - Very lightweight.
 - Simple and flexible theme system.
   - See include/theme_api.h for further information.

## How to Build

### Requirements

- Modern C++ compiler with good C++11 support
  - g++ 4.8 or higher version
  - clang 3.3 or higher version
  - Visual Studio 2013 or higher version
- boost library (boost_date_time, boost_system)
- CMake 3.1.0 or higher version
- Following libraries (as git submodules)
  - [Crow](https://github.com/ipkn/crow)
  - [yaml-cpp](https://github.com/jbeder/yaml-cpp)

### Building

#### Linux, BSD, macOS and so on

For example, install gomibako to /wherever/you/want/

```
git clone https://github.com/pdlan/gomibako.git --recursive
cd gomibako
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/wherever/you/want/ ..
make
make install
```

#### Windows
Not tested.

### Installating Gomibako

Run

```
./gomibako install
```

to install interactively.

When asked which theme to use, you need to input the file name of the theme.
The default theme is clean designed by [whtsky](https://github.com/whtsky)

In Linux, it should be libtheme_clean.so

In macOS, it should be libtheme_clean.dylib

In Windows it should be theme_clean.dll

Then enjoy your blog. Visit /admin/ to administrate your website.

## Attributions

Gomibako uses the following libraries.

    PicoSHA2
    
    https://github.com/okdshin/PicoSHA2
    
    Copyright © 2017 okdshin
    
    base64
    
    https://github.com/tkislan/base64
    
    Copyright (C) 2013 Tomas Kislan
    Copyright (C) 2013 Adam Rudd
    
    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the   "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish,   distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to  the following conditions:
    
    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
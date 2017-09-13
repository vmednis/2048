# 2048
A terminal based clone of the popular game 2048.

![Screenshot](/github/screenshot.png)

Avoids use of any libraries, should compile and run on any system that is somewhat POSIX compliant.

# Compiling
```bash
g++ -std=c++11 -o 2048 2048.cpp
```

# Tested
Currently tested only on a few operating systems:
* Linux
* Minix (with `-DASCIIONLY`)
* Haiku (with `-DASCIIONLY` and `-DIJKL_FALLBACK`)

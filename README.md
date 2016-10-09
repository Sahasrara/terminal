# terminal
Chromium Embedded Framework Terminal

1) Create "build" directory

2) cd build/

3) cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..

4) make -j4 terminal

5) cd ../build/terminal/Release/

6) ln -s ../../../terminal/web_files/ web_files

7) ./terminal

if [ -d gccBuild ]; then
   rm gccBuild -rf
fi
mkdir gccBuild && cd gccBuild
cmake ..
cmake --build .

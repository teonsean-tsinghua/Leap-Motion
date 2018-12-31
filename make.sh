cmake .
make
install_name_tool -change @loader_path/libLeap.dylib ./lib/libLeap.dylib LeapMotion
./LeapMotion

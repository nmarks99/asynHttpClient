# asynHttpClient

## Dependencies

### cpr

Compile cpr as both a shared library and static library. To do this, run `cmake`
with -DCMAKE_BUILD_SHARED_LIBS=ON/OFF and -DCMAKE_POSITION_INDEPENDENT_CODE=ON,
then run `make install` to install to the specified install prefix.

To compile as a static library:
```
cmake .. -DCPR_USE_SYSTEM_CURL=ON -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=/home/beams/NMARKS/.local -DCMAKE_BUILD_TYPE=Release -DCMAKE_POSITION_INDEPENDENT_CODE=ON
```

To compile as a shared library:
```
cmake .. -DCPR_USE_SYSTEM_CURL=ON -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=/home/beams/NMARKS/.local -DCMAKE_BUILD_TYPE=Release
```

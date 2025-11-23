# Amalgamated libdeflate source code

To make things easier for distribution and compilation libdeflate is included as an amalgamated source. It's not the whole libdeflate library, but only portions needed for PNG enoding. If an updated version needs to be generated, here are the steps to take on Windows:

1. Download and extract Amalgamate tool (windows binary is provided as part of the release) https://github.com/rindeal/Amalgamate/releases
2. Download/clone libdeflate release source code https://github.com/ebiggers/libdeflate/releases/ into ./support
3. Run the following Amalgamate commands inside ./support directory
    - `amalgamate -i "./lib" libdeflate.h ../libdeflate_amalgamated.h`
    - `amalgamate -i "./lib" libdeflate.c ../libdeflate_amalgamated.c`
4. That it! There should now be `libdeflate_amalgamated.c` and `libdeflate_amalgamated.h` ready for compilation 


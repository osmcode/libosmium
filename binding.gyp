{
  "includes": [ "common.gypi" ],
  "variables": {
      "module_name":"osmium",
      "module_path":"./lib/"
  },
  "targets": [
    {
      "target_name": "<(module_name)",
      "sources": [
        "src/node_osmium.cpp"
      ],
      "include_dirs": [
          "../libosmium/include/",
          "./src/"
      ],
      "defines": [
        "_LARGEFILE_SOURCE",
        "_FILE_OFFSET_BITS=64",
        "OSMIUM_WITH_DEBUG"
      ],
      "xcode_settings": {
        "GCC_ENABLE_CPP_RTTI": "YES",
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "OTHER_CPLUSPLUSFLAGS":["-stdlib=libc++"],
        "OTHER_LDFLAGS":["-stdlib=libc++"],
        "CLANG_CXX_LANGUAGE_STANDARD":"c++11",
        "MACOSX_DEPLOYMENT_TARGET":"10.7"
      },
      "cflags_cc!": ["-fno-rtti", "-fno-exceptions"],
      "cflags_cc" : ["-std=c++11"],
      "libraries": [
          "-losmpbf",
          "-lprotobuf-lite",
          "-lexpat",
          "-lz",
          "-lbz2"
      ]
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }
  ]
}

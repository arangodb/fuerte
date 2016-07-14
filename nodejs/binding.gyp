{
    "targets":
    [
        {
            "target_name": "AsyncDbVer",

            "sources":
            [
                "GetVersion.cpp",
                "Buffer.cpp",
            ],

            "include_dirs":
            [
                "<!(node -e \"require('nan')\")",
                "./usr/local/include",
            ],

            "link_settings":
            {
                "libraries":
                [
                    "-L../usr/local/lib",
                    "-lfuerte",
		    "-lcurl",
                ]
            },

            "conditions":
            [
                [
                    "OS==\"linux\"",
                    {
                        "cflags":
                        [
                            "-std=c++11",
                        ],

                        "cflags!":
                        [
                            "-fno-exceptions",
                        ],

                        "cflags_cc!":
                        [
                            "-fno-exceptions",
                            "-fno-rtti",
                        ]
                    }
                ],

                [
                    "OS==\"mac\"",
                    {
                        "defines":
                        [
                            "HAVE_CONFIG_H",
                        ],

                        "xcode_settings":
                        {
                            "GCC_ENABLE_CPP_RTTI": "YES",
                            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
                            "OTHER_CPLUSPLUSFLAGS": [
                                "-std=c++11",
                                "-stdlib=libc++"
                            ],
                            "OTHER_LDFLAGS": [
                                "-stdlib=libc++"
                            ],
                        }
                    }
                ]
            ]
        }
    ]
}

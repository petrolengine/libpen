BaseType = {
    "base_type": [
        "int", "const char *", "float", "double", "uint8_t", "int8_t",
        "uint16_t", "int16_t", "uint32_t", "int32_t", "uint64_t", "int64_t",
        "unsigned int", "short int", "unsigned short int", "long int",
        "unsigned long int", "long long int", "unsigned long long int",
        "char *", "bool", "size_t"
    ],
    "enum": {
        "sock_type": [
            "SOCK_TYPE_TCP",
            "SOCK_TYPE_UDP",
            "SOCK_TYPE_HTTP",
            "SOCK_TYPE_HTTPS",
        ],
    },
    # TODO nested array and nested object is not supported yet.
    "type": {
        "server_type": {
            "timeout": {
                "type": "int",
                "default": "-1",
            },
            "maxclients": {
                "type": "size_t",
                "default": "0",
            },
            "socktype": {
                "type": "sock_type",
                "default": "SOCK_TYPE_TCP",
            },
            "rcvbuffer": {
                "type": "size_t",
                "default": "0",
            },
            "sndbuffer": {
                "type": "size_t",
                "default": "0",
            },
        },
        "tcp_listener": {
            "name": {
                "type": "const char *",
                "default": "NULL",
                "todel": True,
            },
            "host": {
                "type": "const char *",
                "default": "NULL",
                "todel": True,
            },
            "port": {
                "type": "unsigned short int",
            },
            "backlog": {
                "type": "int",
                "default": "5",
            },
            "servertype": {
                "type": "uint8_t",
                "default": "0",
            },
        },
        "unix_listener": {
            "name": {
                "type": "const char *",
                "default": "NULL",
                "todel": True,
            },
            "host": {
                "type": "const char *",
                "todel": True,
            },
            "servertype": {
                "type": "uint8_t",
                "default": "0",
            },
            "backlog": {
                "type": "int",
                "default": "5",
            },
        },
        "tcp_connector": {
            "name": {
                "type": "const char *",
                "default": "NULL",
                "todel": True,
            },
            "ip": {
                "type": "const char *",
                "default": "NULL",
                "todel": True,
            },
            "port": {
                "type": "unsigned short int",
            },
            "servertype": {
                "type": "uint8_t",
                "default": "0",
            }
        },
        "unix_connector": {
            "name": {
                "type": "const char *",
                "default": "NULL",
                "todel": True,
            },
            "host": {
                "type": "const char *",
                "todel": True,
            },
            "timeout": {
                "type": "int",
                "default": "-1",
            },
            "servertype": {
                "type": "uint8_t",
                "default": "0",
            }
        },
    },
    "data": {
        "timer_size": {
            "type": "size_t",
            "default": "32",
        },
        "log_level": {
            "type": "unsigned int",
            "default": "1",
        },
        "log_dir": {
            "type": "const char *",
            "default": "NULL",
            "todel": True,
        },
        "log_console": {
            "type": "bool",
            "default": "true",
        },
        "tcp_listener": {
            "type": "tcp_listener",
        },
        "unix_listener": {
            "type": "unix_listener",
        },
        "tcp_connector": {
            "type": "tcp_connector",
        },
        "unix_connector": {
            "type": "unix_connector",
        },
    },
    "array": {
        "server_types": {
            "type": "server_type",
        },
        "tcp_listeners": {
            "type": "tcp_listener",
        },
        "unix_listeners": {
            "type": "unix_listener",
        },
        "tcp_connectors": {
            "type": "tcp_connector",
        },
        "unix_connectors": {
            "type": "unix_connector",
        },
        "dummy": {
            "type": "int",
            "default": "0",
        },
    },
}

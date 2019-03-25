#!/usr/bin/env python3

import sys
import os

from base_type import BaseType


cur_dir = os.path.dirname(os.path.abspath(__file__))


def get_file_data(filename):
    with open(os.path.join(cur_dir, filename), 'r') as f:
        return f.read()
    return None


def getc_do_set_value_code(k, obj, ts):
    ret = ' ' * ts
    if 'todel' in obj and obj['todel']:
        ret += '%s = strdup(value);\n' % k
    elif obj['type'] in BaseType['enum']:
        ret += '%s =\n' % k
        ret += '%sPEN_OPTION_ENUM_FUNC(%s)(value);\n' % (
                ' ' * (ts + 8), obj['type'])
    elif obj['type'] == 'bool':
        ret += '%s = (strcmp(value, "true") == 0);\n' % k
    elif obj['type'] == 'float' or obj['type'] == 'double':
        ret += '%s = atof(value);\n' % k
    elif obj['type'] in BaseType['base_type']:
        ret += '%s = (%s)atol(value);\n' % (k, obj['type'])
    else:
        assert(False)
    return ret


def getc_set_value_code(k, obj):
    ret =  '    if (strcmp("%s", key) == 0) {\n' % k
    ret += getc_do_set_value_code('PEN_OPTION_NAME(%s)' % k, obj, 8)
    ret += '    }'
    return ret


def getc_set_object_value_code(t, obj):
    ret  = '    if (strcmp("%s", name) == 0) {\n' % t
    type_obj = BaseType['type'][obj['type']]
    for k in type_obj:
        ret += '        if (strcmp("%s", key) == 0) {\n' % k
        ret += getc_do_set_value_code(
                'PEN_OPTION_NAME(%s).%s' % (t, k), type_obj[k], 12)
        ret += '        }\n'
    ret += '    }\n'
    return ret


def getc_init_object_value_code(t, obj):
    ret  = '    if (strcmp("%s", name) == 0) {\n' % t
    ret += '%sPEN_OPTION_INIT_OBJECT_FUNC(%s)(\n' % (' ' * 8, t)
    ret += '%s&PEN_OPTION_NAME(%s));\n' % (' ' * 16, t)
    ret += '    }\n'
    return ret


def getc_set_array_value_code(t, obj):
    ret  = '    if (strcmp("%s", name) == 0) {\n' % t
    ret += getc_do_set_value_code('PEN_OPTION_NAME(%s)[idx]' % t, obj, 8)
    ret += '    }\n'
    return ret


def getc_set_array_object_value_code(t, obj):
    ret  = '    if (strcmp("%s", name) == 0) {\n' % t
    type_obj = BaseType['type'][obj['type']]
    for k in type_obj:
        ret += '        if (strcmp("%s", key) == 0) {\n' % k
        ret += getc_do_set_value_code(
                'PEN_OPTION_NAME(%s)[idx].%s' % (t, k), type_obj[k], 12)
        ret += '        }\n'
    ret += '    }\n'
    return ret


def getc_init_array_value_code(t, obj):
    ret  = '    if (strcmp("%s", name) == 0) {\n' % t
    ret += '        assert(PEN_OPTION_NAME(%s) == NULL);\n' % t
    ret += '        PEN_OPTION_NAME(%s) = (%s*)calloc(\n' % (t, obj['type'])
    ret += '                                PEN_OPTION_NAME(%s_size),\n' % t
    ret += '                                sizeof(%s));\n' % obj['type']
    ret += '        assert(PEN_OPTION_NAME(%s) != NULL);\n' % t
    ret += '%sfor (size_t i = 0; i < PEN_OPTION_NAME(%s_size); i++) {\n' % (
            ' ' * 8, t)
    ret += '%sPEN_OPTION_NAME(%s)[i] = %s;\n' % (' ' * 12, t, obj['default'])
    ret += '%s}\n' % (' ' * 8)
    ret += '    }\n'
    return ret


def getc_init_array_object_value_code(t, obj):
    ret  = '    if (strcmp("%s", name) == 0) {\n' % t
    ret += '        assert(PEN_OPTION_NAME(%s) == NULL);\n' % t
    ret += '        PEN_OPTION_NAME(%s) = ' % t
    ret += '(PEN_OPTION_STRUCT_NAME(%s)*)calloc(\n' % obj['type']
    ret += '                            PEN_OPTION_NAME(%s_size),\n' % t
    ret += '                            sizeof(*PEN_OPTION_NAME(%s)));\n' % t
    ret += '        assert(PEN_OPTION_NAME(%s) != NULL);\n' % t
    ret += '%sfor (size_t i = 0; i < PEN_OPTION_NAME(%s_size); i++) {\n' % (
            ' ' * 8, t)
    ret += '%sPEN_OPTION_INIT_OBJECT_FUNC(%s)(\n' % (' ' * 12, obj['type'])
    ret += '%s&PEN_OPTION_NAME(%s)[i]);\n' % (' ' * 20, t)
    ret += '%s}\n' % (' ' * 8)
    ret += '    }\n'
    return ret


def getc_set_array_size_code(t):
    ret =  '    if (strcmp("%s_size", key) == 0) {\n' % t
    ret += '        PEN_OPTION_NAME(%s_size) = atol(value);\n' % t
    ret += '    }'
    return ret


def getc_free_object_code(t, obj, todel):
    ret = ''
    type_obj = BaseType['type'][obj['type']]
    for k in type_obj:
        if "todel" in type_obj[k] and type_obj[k]['todel']:
            ret += '    FREE(PEN_OPTION_NAME(%s).%s);\n' % (t, k)
    if len(ret) > 0:
        todel.append(ret)


def getc_free_array_object_code(t, obj, todel):
    ret = ''
    if getc_object_is_need_free(obj['type']):
        ret += '    for (size_t i = 0; '
        ret += 'i < PEN_OPTION_NAME(%s_size); i++) {\n' % t
        type_obj = BaseType['type'][obj['type']]
        for k in type_obj:
            if "todel" in type_obj[k] and type_obj[k]['todel']:
                ret += '        FREE(PEN_OPTION_NAME(%s)[i].%s);\n' % (t, k)
        ret += '    }\n'

    ret += '    FREE(PEN_OPTION_NAME(%s));' % t
    todel.append(ret)


def getc_object_is_need_free(t):
    type_obj = BaseType['type'][t]
    for k in type_obj:
        if "todel" in type_obj[k] and type_obj[k]['todel']:
            return True
    return False


def getc_option_data():
    ret = []
    todel = []
    codes_data = [ [], [], [] ]
    codes_array = [ [], [], [] ]
    # object
    data = BaseType['data']
    for k in data:
        if data[k]['type'] in BaseType['base_type']:
            tt, td = data[k]['type'], data[k]['default']
            ret.append("%s PEN_OPTION_NAME(%s) = %s;" % (tt, k, td))
            if "todel" in data[k] and data[k]['todel']:
                todel.append("    FREE(PEN_OPTION_NAME(%s));" % k)
            codes_data[0].append(getc_set_value_code(k, data[k]))
        elif data[k]['type'] in BaseType['enum']:
            tt, td = data[k]['type'], data[k]['default']
            ret.append("PEN_OPTION_ENUM_NAME(%s) PEN_OPTION_NAME(%s) = %s;"
                    % (tt, k, td))
            codes_data[0].append(getc_set_value_code(k, data[k]))
        elif data[k]['type'] in BaseType['type']:
            # TODO add delete object data
            ret.append("PEN_OPTION_STRUCT_NAME(%s) PEN_OPTION_NAME(%s);"
                    % (data[k]['type'], k))
            codes_data[1].append(getc_set_object_value_code(k, data[k]))
            codes_data[2].append(getc_init_object_value_code(k, data[k]))
            getc_free_object_code(k, data[k], todel)
        else:
            assert(False)
    # array
    data = BaseType['array']
    for k in data:
        ret.append('size_t PEN_OPTION_NAME(%s_size) = 0;' % k)
        codes_data[0].append(getc_set_array_size_code(k))
        if data[k]['type'] in BaseType['base_type']:
            ret.append("%s *PEN_OPTION_NAME(%s) = NULL;" % (data[k]['type'], k))
            codes_array[0].append(getc_set_array_value_code(k, data[k]))
            codes_array[2].append(getc_init_array_value_code(k, data[k]))
            if "todel" in data[k] and data[k]['todel']:
                temp  = '    for (int i = 0; i < '
                temp += 'PEN_OPTION_NAME(%s_size); i++) {\n' % k
                temp += '        FREE(PEN_OPTION_NAME(%s)[i]);\n' % k
                temp += '    }\n'
                todel.append(temp)
            todel.append("    FREE(PEN_OPTION_NAME(%s));" % k)
        elif data[k]['type'] in BaseType['enum']:
            ret.append("PEN_OPTION_ENUM_NAME(%s) *PEN_OPTION_NAME(%s) = NULL;"
                    % (data[k]['type'], k))
            codes_array[0].append(getc_set_array_value_code(k, data[k]))
            codes_array[2].append(getc_init_array_value_code(k, data[k]))
            todel.append("    FREE(PEN_OPTION_NAME(%s));" % k)
        elif data[k]['type'] in BaseType['type']:
            # TODO add delete array object data
            ret.append("PEN_OPTION_STRUCT_NAME(%s) *PEN_OPTION_NAME(%s) = NULL;"
                    % (data[k]['type'], k))
            codes_array[1].append(getc_set_array_object_value_code(k, data[k]))
            codes_array[2].append(getc_init_array_object_value_code(k, data[k]))
            getc_free_array_object_code(k, data[k], todel)
        else:
            assert(False)

    return ('\n'.join(sorted(ret)), '\n'.join(sorted(todel)),
            codes_data, codes_array)


def getc_one_enum_code(t, obj):
    ret  = "static inline PEN_OPTION_ENUM_NAME(%s)\n" % t
    ret += "PEN_OPTION_ENUM_FUNC(%s)(const char *name)\n{\n" % t
    for k in obj:
        ret += '    if (strcmp("%s", name) == 0) return %s;\n' % (k, k)
    ret += "    assert(false);\n    return 0;\n}\n"
    return ret


def getc_name_to_enum_code():
    ret = []
    for k in BaseType['enum']:
        ret.append(getc_one_enum_code(k, BaseType['enum'][k]))
    return '\n'.join(sorted(ret))


def getc_one_init_struct_code(t, obj):
    ret  = 'static inline void\nPEN_OPTION_INIT_OBJECT_FUNC(%s)(\n' % t
    ret += '        PEN_OPTION_STRUCT_NAME(%s) *obj)\n{\n' % t
    ret += '   bzero(obj, sizeof(*obj));\n'
    for k in obj:
        if 'default' in obj[k]:
            ret += '    obj->%s = %s;\n' % (k, obj[k]["default"])

    ret += '}\n'
    return ret


def getc_init_object_func_code():
    ret = []
    for k in BaseType['type']:
        ret.append(getc_one_init_struct_code(k, BaseType['type'][k]))
    return '\n'.join(sorted(ret))


def generate_c_file():
    template_file = get_file_data("option.c.in")
    data, todel, codes_data, codes_array = getc_option_data()
    return template_file % {
        "OPTION_DATA": data,
        "OPTION_DATA_TO_FREE": todel,
        "OPTION_SET_VALUE": '\n'.join(sorted(codes_data[0])),
        "OPTION_SET_OBJ_VALUE": '\n'.join(sorted(codes_data[1])),
        "OPTION_INIT_OBJECT": '\n'.join(sorted(codes_data[2])),
        "OPTION_SET_ARRAY_VALUE": '\n'.join(sorted(codes_array[0])),
        "OPTION_SET_ARRAY_OBJ_VALUE": '\n'.join(sorted(codes_array[1])),
        "OPTION_INIT_ARRAY": '\n'.join(sorted(codes_array[2])),
        "OPTION_NAME_TO_ENUM": getc_name_to_enum_code(),
        "OPTION_INIT_OBJECT_FUNC": getc_init_object_func_code(),
    }


def get_real_type_name(t):
    if t in BaseType['base_type']:
        return t
    if t in BaseType['enum']:
        return "PEN_OPTION_ENUM_NAME(%s)" % t
    if t in BaseType['type']:
        return "PEN_OPTION_STRUCT_NAME(%s)" % t
    print(t)
    assert(False)


# Get enum type
def get_one_option_enum_type(t, obj):
    ret = "typedef enum PEN_OPTION_ENUM_NAME(%s) {\n" % t
    for k in BaseType['enum'][t]:
        ret += "    %s,\n" % k
    ret += "} PEN_OPTION_ENUM_NAME(%s);\n" % t
    return ret


def geth_option_enum_type():
    ret = []
    for k in BaseType['enum']:
        ret.append(get_one_option_enum_type(k, BaseType['enum'][k]))
    return '\n'.join(sorted(ret))


# Get struct type
def get_one_option_struct_type(t, obj):
    ret = "typedef struct PEN_OPTION_STRUCT_NAME(%s) {\n" % t
    for k in BaseType['type'][t]:
        ret += "    %s %s;\n" % (get_real_type_name(obj[k]['type']), k)
    ret += "} PEN_OPTION_STRUCT_NAME(%s);\n" % t
    return ret


def geth_option_struct_type():
    ret = []
    for k in BaseType['type']:
        ret.append(get_one_option_struct_type(k, BaseType['type'][k]))
    return '\n'.join(sorted(ret))


def geth_option_data():
    ret = []
    for k in BaseType['data']:
        ret.append("extern %s PEN_OPTION_NAME(%s);"
                % (get_real_type_name(BaseType['data'][k]['type']), k))
    for k in BaseType['array']:
        ret.append('extern size_t PEN_OPTION_NAME(%s_size);' % k)
        ret.append("extern %s *PEN_OPTION_NAME(%s);"
                % (get_real_type_name(BaseType['array'][k]['type']), k))
    return '\n'.join(sorted(ret))


def generate_h_file():
    template_file = get_file_data("option.h.in")
    return template_file % {
        "OPTION_ENUM_TYPE": geth_option_enum_type(),
        "OPTION_STRUCT_TYPE": geth_option_struct_type(),
        "OPTION_DATA": geth_option_data()
    }


def get_example_value(obj):
    ret = ''

    if 'default' in obj:
        ret += obj['default'] + ';'
    elif obj["type"] in BaseType["type"]:
        ret += '{\n'
        type_obj = BaseType['type'][obj['type']]
        for k in type_obj:
            ret += '#    %s = %s\n' % (k, get_example_value(type_obj[k]))
        ret += '# }'
    else:
        ret += ';'

    return ret


def get_example_array_value(obj):
    ret = ''

    if obj['type'] in BaseType['base_type']:
        if 'default' in obj:
            ret += '#    %s\n' % obj['default']
    elif obj['type'] in BaseType['type']:
        ret += '#    {\n'
        type_obj = BaseType['type'][obj['type']]
        for k in type_obj:
            ret += '#        %s = %s\n' % (k, get_example_value(type_obj[k]))
        ret += '#    },\n'
    elif obj['type'] in BaseType['enum']:
        for k in BaseType['enum'][obj['type']]:
            ret += '#    %s\n' % k
    else:
        assert(False)

    return ret


def generate_conf_file():
    ret = []
    for k in BaseType['data']:
        ret.append("# %s = %s\n" % (k, get_example_value(BaseType['data'][k])))
    for k in BaseType['array']:
        ret.append("# %s_size = 0;\n# %s = [\n%s# ]\n" % (
            k, k, get_example_array_value(BaseType['array'][k])))
    return '\n'.join(sorted(ret))


def save_result(filename, data):
    with open(filename, "w") as f:
        f.write(data)


def generate_output_file(gtype, outfile):
    for k in gtype:
        output = None
        if k == 'c':
            output = generate_c_file()
        elif k == 'h':
            output = generate_h_file()
        elif k == 'e':
            output = generate_conf_file()
        else:
            assert(False)
        if output is not None:
            save_result(outfile, output)


def print_usage():
    print('usage: ./auto_gen_option_c.py [c|h|e] [output]')
    sys.exit(-1)


def check_base_type():
    for k in BaseType['type']:
        if k in BaseType['base_type']:
            return False
    for k in BaseType['data']:
        t = BaseType['data'][k]['type']
        if t not in BaseType['type'] and t not in BaseType['base_type']:
            return False
    return True


def main(argv):
    if (len(argv) != 2):
        print_usage()

    assert(check_base_type())
    assert(os.access(os.path.dirname(argv[1]), os.W_OK))
    assert(not os.access(argv[1], os.F_OK) or os.access(argv[1], os.W_OK))
    generate_output_file(argv[0], argv[1])


if __name__ == '__main__':
    main(sys.argv[1:])

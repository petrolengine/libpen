
class MyBaseType():
    def __init__(self, k):
        self.key_ = k
        self.toDel_ = k in ["const char *", "char *"]
        self.isObj_ = False

    @property
    def set_value_code(self):
        if self.toDel_:
            return 'strdup(value);'
        if self.key_ == 'bool':
            return '(strcmp(value, "true") == 0);'
        if self.key_ in ['float', 'double']:
            return 'atof(value);'
        return '(%s)atol(value);' % self.key_

    def set_name(self, name):
        return 'PEN_OPTION_NAME(%s)' % name

    @property
    def name(self):
        return self.key_


class MyEnum():
    def __init__(self, k, data):
        self.key_ = k
        self.data_ = data
        self.isObj_ = False
        self.toDel_ = False

    @property
    def codef(self):
        ret = ''
        for k in self.data_:
            ret += '#   %s\n' % k
        return ret

    @property
    def set_value_code(self):
        return '\n%sPEN_OPTION_ENUM_FUNC(%s)(value);' % (
            ' ' * 12, self.key_
        )

    @property
    def set_func(self):
        ret = "static inline PEN_OPTION_ENUM_NAME(%s)\n" % self.key_
        ret += "PEN_OPTION_ENUM_FUNC(%s)(const char *name)\n{\n" % self.key_
        for k in self.data_:
            ret += '    if (strcmp("%s", name) == 0) return %s;\n' % (k, k)
        ret += "    assert(false);\n    return 0;\n}\n"
        return ret

    def set_name(self, name):
        return 'PEN_OPTION_NAME(%s)' % name

    @property
    def name(self):
        return 'PEN_OPTION_ENUM_NAME(%s)' % self.key_

    @property
    def head_def(self):
        ret = 'typedef enum PEN_OPTION_ENUM_NAME(%s) {\n' % self.key_
        for k in self.data_:
            ret += "    %s,\n" % k
        ret += "} PEN_OPTION_ENUM_NAME(%s);\n" % self.key_
        return ret


class MyObject():
    # TODO not support obj in obj yet
    def __get_item_type(self, t, base_type, enum_type):
        return base_type[t] if t in base_type else enum_type[t]

    def __init__(self, k, data, base_type, enum_type):
        self.isObj_ = True
        self.key_ = k
        self.items_ = {
            k: {
                "bt_": self.__get_item_type(v['type'], base_type, enum_type),
                "defVal_": v["default"] if "default" in v else 0
            } for k, v in data.items()
        }

    @property
    def codef(self):
        ret = '{\n'
        for k, v in self.items_.items():
            ret += '#   %s = %s\n' % (k, v['defVal_'])
        ret += '# }\n'
        return ret

    def set_value_code(self, name):
        ret = ''
        tbs = ' ' * 8
        for k, v in self.items_.items():
            ret += '%sif (strcmp("%s", name) == 0) {\n' % (tbs, k)
            ret += '%s%s.%s = %s\n' % (
                ' ' * 12, name, k, v['bt_'].set_value_code
            )
            ret += '%s}\n' % tbs
        return ret

    def get_free_code(self, name, ext, tb):
        ret = ''
        for k, v in self.items_.items():
            if v['bt_'].toDel_:
                ret += '%sFREE(PEN_OPTION_NAME(%s)%s.%s);\n' % (tb, name, ext, k)
        return ret

    @property
    def init_func(self):
        ret = 'static inline void\n'
        ret += 'PEN_OPTION_INIT_OBJECT_FUNC(%s)(\n' % self.key_
        ret += '        PEN_OPTION_STRUCT_NAME(%s) *obj)\n{\n' % self.key_
        ret += '    bzero(obj, sizeof(*obj));\n'
        for k, v in self.items_.items():
            ret += '    obj->%s = %s;\n' % (k, v['defVal_'])
        ret += '}\n'
        return ret

    @property
    def name(self):
        return 'PEN_OPTION_STRUCT_NAME(%s)' % self.key_

    @property
    def head_def(self):
        ret = 'typedef struct PEN_OPTION_STRUCT_NAME(%s) {\n' % self.key_
        for k, v in self.items_.items():
            ret += '    %s %s;\n' % (v['bt_'].name, k)
        ret += '} PEN_OPTION_STRUCT_NAME(%s);\n' % self.key_
        return ret

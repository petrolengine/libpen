from abc import ABC, abstractmethod


class GenType(ABC):
    def __init__(self, k, data, tp):
        self.type_ = tp
        self.defVal_ = data['default'] if 'default' in data else 0
        self.key_ = k

    @property
    @abstractmethod
    def codec(self):
        pass

    @property
    def codeh(self):
        ret = ''
        if self.isArray_:
            ret += 'extern size_t %s;\n' % self.array_size
            ret += 'extern %s *%s;' % (self.type_.name, self.name)
        else:
            ret += 'extern %s %s;' % (self.type_.name, self.name)
        return ret

    @property
    @abstractmethod
    def codef(self):
        pass

    @property
    def name(self):
        return 'PEN_OPTION_NAME(%s)' % self.key_

    @property
    def array_size(self):
        return 'PEN_OPTION_NAME(%s_size)' % self.key_

    def set_value_code(self, eq):
        ret = '%sif (strcmp("%s", key) == 0) {\n' % (' ' * 4, self.key_)
        if self.type_.isObj_:
            ret += self.type_.set_value_code(self.name + eq)
        else:
            ret += '        %s%s%s\n' % (
                self.type_.set_name(self.key_), eq, self.type_.set_value_code
            )
        ret += '    }'
        return ret

    @property
    def set_array_size_code(self):
        ret = '    if (strcmp("%s_size", key) == 0) {\n' % self.key_
        ret += '        %s = (size_t)atol(value);\n' % self.array_size
        ret += '    }'
        return ret

    @property
    def init_value_code(self):
        ret = '    if (strcmp("%s", name) == 0) {\n' % self.key_
        ret += '        assert(%s == NULL);\n' % self.name
        ret += '        %s = (%s*)calloc(\n' % (self.name, self.type_.name)
        ret += '%s%s,\n' % (' ' * 12, self.array_size)
        ret += '%ssizeof(%s));\n' % (' ' * 12, self.type_.name)
        ret += '        assert(%s != NULL);\n' % self.name
        ret += '%sfor (size_t i = 0; i < %s; i++) {\n' % (
            ' ' * 8, self.array_size
        )
        if self.type_.isObj_:
            ret += '%sPEN_OPTION_INIT_OBJECT_FUNC(%s)(\n' % (
                ' ' * 12, self.type_.key_
            )
            ret += '%s&%s[i]);\n' % (' ' * 16, self.name)
        else:
            ret += '%s%s[i] = %s;\n' % (' ' * 12, self.name, self.defVal_)
        ret += '        }\n'
        ret += '    }\n'
        return ret


class GenBaseType(GenType):
    def __init__(self, k, data, tp):
        super().__init__(k, data, tp)
        self.isArray_ = False

    @property
    def codec(self):
        data = "%s %s = %s;" % (self.type_.key_, self.name, self.defVal_)
        todel = "    FREE(%s);" % (self.name) if self.type_.toDel_ else ""
        setval = self.set_value_code(' = ')

        return {
            "OPTION_DATA": data,
            "OPTION_DATA_TO_FREE": todel,
            "OPTION_SET_VALUE": setval,
        }

    @property
    def codef(self):
        return '# %s = %s\n' % (self.key_, self.defVal_)


class GenEnumType(GenType):
    def __init__(self, k, data, tp):
        super().__init__(k, data, tp)
        self.isArray_ = False

    @property
    def codec(self):
        data = "%s %s = %s;" % (self.type_.name, self.name, self.defVal_)
        setval = self.set_value_code(' =\n            ')

        return {
            "OPTION_DATA": data,
            "OPTION_SET_VALUE": setval,
        }

    @property
    def codef(self):
        return '# %s = %s\n' % (self.key_, self.defVal_)


class GenObjectType(GenType):
    def __init__(self, k, data, tp):
        super().__init__(k, data, tp)
        self.isArray_ = False

    @property
    def codec(self):
        data = '%s %s;' % (self.type_.name, self.name)
        todel = self.type_.get_free_code(self.key_, '', '    ')
        setObjVal = self.set_value_code('')
        initObjVal = '    if (strcmp("%s", name) == 0) {\n' % self.key_
        initObjVal += '%sPEN_OPTION_INIT_OBJECT_FUNC(%s)(\n' % (
            ' ' * 8, self.key_
        )
        initObjVal += '%s&%s);\n' % (' ' * 12, self.name)
        initObjVal += '    }\n'

        return {
            "OPTION_DATA": data,
            "OPTION_DATA_TO_FREE": todel,
            "OPTION_SET_OBJ_VALUE": setObjVal,
            "OPTION_INIT_OBJECT": initObjVal,
        }

    @property
    def codef(self):
        return '# %s = %s' % (self.key_, self.type_.codef)


class GenArrayType(GenType):
    def __init__(self, k, data, tp):
        super().__init__(k, data, tp)
        self.isArray_ = True

    @property
    def __delete_code(self):
        ret = '%sfor (int i = 0; i < %s; i++) {\n' % (' ' * 4, self.array_size)
        ret += '%sFREE(%s[i]);\n' % (' ' * 8, self.name)
        ret += '    }\n'
        return ret

    @property
    def codec(self):
        data = 'size_t %s = 0;\n' % self.array_size
        data += '%s *%s = NULL;' % (self.type_.key_, self.name)
        todel = self.__delete_code if self.type_.toDel_ else ''
        todel += '    FREE(%s);\n' % self.name
        setArrVal = self.set_value_code('[idx] = ')
        initArr = self.init_value_code

        return {
            "OPTION_DATA": data,
            "OPTION_DATA_TO_FREE": todel,
            "OPTION_SET_VALUE": self.set_array_size_code,
            "OPTION_SET_ARRAY_VALUE": setArrVal,
            "OPTION_INIT_ARRAY": initArr,
        }

    @property
    def codef(self):
        return '# %s_size = 0;\n# %s = [ %s ]\n' % (
            self.key_, self.key_, self.defVal_
        )


class GenArrayEnumType(GenType):
    def __init__(self, k, data, tp):
        super().__init__(k, data, tp)
        self.isArray_ = True

    @property
    def codec(self):
        data = 'size_t %s = 0;\n' % self.array_size
        data += '%s *%s = NULL;' % (self.type_.name, self.name)
        todel = '    FREE(%s);\n' % self.name
        setArrVal = self.set_value_code('[idx] =\n            ')
        initArr = self.init_value_code

        return {
            "OPTION_DATA": data,
            "OPTION_SET_VALUE": self.set_array_size_code,
            "OPTION_DATA_TO_FREE": todel,
            "OPTION_SET_ARRAY_VALUE": setArrVal,
            "OPTION_INIT_ARRAY": initArr
        }

    @property
    def codef(self):
        return '# %s_size = 0;\n# %s = [\n%s# ]\n' % (
            self.key_, self.key_, self.type_.codef
        )


class GenArrayObjectType(GenType):
    def __init__(self, k, data, tp):
        super().__init__(k, data, tp)
        self.isArray_ = True

    @property
    def free_code(self):
        base = self.type_.get_free_code(self.key_, '[i]', ' ' * 8)
        if len(base) == 0:
            return ''
        ret = '%sfor (size_t i = 0; i < %s; i++) {\n' % (
            ' ' * 4, self.array_size
        )
        ret += base
        ret += '    }\n'
        ret += '    FREE(%s);' % self.name
        return ret

    @property
    def codec(self):
        data = 'size_t %s = 0;\n' % self.array_size
        data += '%s *%s = NULL;' % (self.type_.name, self.name)
        todel = self.free_code
        setArrVal = self.set_value_code('[idx]')
        initArr = self.init_value_code

        return {
            "OPTION_DATA": data,
            "OPTION_DATA_TO_FREE": todel,
            "OPTION_SET_VALUE": self.set_array_size_code,
            "OPTION_SET_ARRAY_OBJ_VALUE": setArrVal,
            "OPTION_INIT_ARRAY": initArr,
        }

    @property
    def codef(self):
        return '# %s_size = 0;\n# %s = [\n# %s# ]\n' % (
            self.key_, self.key_, self.type_.codef
        )


def test():
    arr = []
    arr.append(GenBaseType(arr))
    arr.append(GenEnumType(arr))
    arr.append(GenObjectType(arr))
    arr.append(GenArrayType(arr))
    arr.append(GenArrayObjectType(arr))

    for a in arr:
        a.codec


if __name__ == '__main__':
    test()

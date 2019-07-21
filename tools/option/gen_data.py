import os
from base_type import BaseType
from gen_type import GenBaseType, GenEnumType, GenObjectType
from gen_type import GenArrayType, GenArrayEnumType, GenArrayObjectType
from my_type import MyBaseType, MyEnum, MyObject


cur_dir = os.path.dirname(os.path.abspath(__file__))


def get_file_data(filename):
    with open(os.path.join(cur_dir, filename), 'r') as f:
        return f.read()
    return None


def save_result(filename, data):
    with open(filename, "w") as f:
        f.write(data)


class GenData:

    def __init__(self):

        clss = [
            [GenBaseType, GenEnumType, GenObjectType],
            [GenArrayType, GenArrayEnumType, GenArrayObjectType]
        ]
        arr = [BaseType["data"], BaseType["array"]]

        self.mp_ = []
        self.baseType_ = {k: MyBaseType(k) for k in BaseType['base_type']}
        self.enumType_ = {k: MyEnum(k, v) for k, v in BaseType['enum'].items()}
        self.objType_ = {
            k: MyObject(
                k, v, self.baseType_, self.enumType_
            ) for k, v in BaseType['type'].items()
        }

        for tp in range(len(arr)):
            data = arr[tp]
            for k in data:
                t = data[k]['type']
                if t in self.baseType_:
                    self.mp_.append(clss[tp][0](k, data[k], self.baseType_[t]))
                elif t in self.enumType_:
                    self.mp_.append(clss[tp][1](k, data[k], self.enumType_[t]))
                elif t in self.objType_:
                    self.mp_.append(clss[tp][2](k, data[k], self.objType_[t]))
                else:
                    assert(False)

    @property
    def codec(self):
        ret = {
            "OPTION_DATA": [],
            "OPTION_DATA_TO_FREE": [],
            "OPTION_SET_VALUE": [],
            "OPTION_SET_OBJ_VALUE": [],
            "OPTION_INIT_OBJECT": [],
            "OPTION_SET_ARRAY_VALUE": [],
            "OPTION_SET_ARRAY_OBJ_VALUE": [],
            "OPTION_INIT_ARRAY": [],
            "OPTION_NAME_TO_ENUM": [
                v.set_func for _, v in self.enumType_.items()
            ],
            "OPTION_INIT_OBJECT_FUNC": [
                v.init_func for _, v in self.objType_.items()
            ]
        }

        for m in self.mp_:
            data = m.codec
            for k in data:
                if len(data[k]) > 0:
                    ret[k].append(data[k])

        for k in ret:
            ret[k] = '\n'.join(sorted(ret[k]))
        return ret

    @property
    def codeh(self):
        ret = {
            "OPTION_ENUM_TYPE": [
                v.head_def for _, v in self.enumType_.items()
            ],
            "OPTION_STRUCT_TYPE": [
                v.head_def for _, v in self.objType_.items()
            ],
            "OPTION_DATA": [
                v.codeh for v in self.mp_
            ]
        }
        for k in ret:
            ret[k] = '\n'.join(sorted(ret[k]))
        return ret

    @property
    def codef(self):
        return '\n'.join(sorted([v.codef for v in self.mp_]))


def test():
    template_file = get_file_data("option.c.in")
    save_result("option.c", template_file % GenData().codec)

    template_file = get_file_data("option.h.in")
    save_result("option.h", template_file % GenData().codeh)

    save_result("my.conf", GenData().codef)


if __name__ == '__main__':
    test()

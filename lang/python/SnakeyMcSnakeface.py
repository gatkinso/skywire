import struct 

sizedict = {
        "char": 1,
        "unsigned char" : 1,
        "uint8_t" : 1,
        "int8_t" : 1,
        "uint16_t" : 2,
        "int16_t" : 2,
        "uint32_t" : 4,
        "int32_t" : 4,
        "uint64_t" : 8,
        "int64_t" : 8,
        "float" : 4,
        "double" : 8
}

typedict = {
        "char": "b",
        "unsigned char" : "B",
        "uint8_t" : "b",
        "int8_t" : "B",
        "uint16_t" : "H",
        "int16_t" : "h",
        "uint32_t" : "I",
        "int32_t" : "i",
        "uint64_t" : "Q",
        "int64_t" : "q",
        "float" : "f",
        "double" : "d"
}

endiandict = {
    0 : ">",
    1 : "<",
    2 : "@"
}

class SnakeyMcSnakeface:

    def __init__(self):
        self.myfloat_100 = 0.0
        self.myuint32_200 = 0
        self.myint64_201 = 0

    def size(self):
        retval = sizedict["float"] + 
        sizedict["uint32_t"] + 
        sizedict["int64_t"]

        return retval

    def equals(self, rhs):
        retval = (self.myfloat_100 == rhs.myfloat_100) and
        (self.myuint32_200 == rhs.myuint32_200) and 
        (self.myint64_201 == rhs.myint64_201)

    def serialize(self, buffer):
        struct.pack_into('>f<I<q', buffer, 0,
                         self.myfloat_100,
                         self.myuint32_200,
                         self.myint64_201
        )
        return self.size()

    def deserialize(self, buffer):
        struct.unpack_from('>f<I<q', buffer, 0,
                         self.myfloat_100,
                         self.myuint32_200,
                         self.myint64_201
        )


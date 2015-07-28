# -*- python -*-


#  Copyright (c)  Harri Rautila, 2015
#  All rights reserved.
# 
#  Redistribution and use in source and binary forms, with or without 
#  modification, are permitted provided that the following conditions are met:
# 
#    #  Redistributions of source code must retain the above copyright notice, 
#       this list of conditions and the following disclaimer.
#    #  Redistributions in binary form must reproduce the above copyright notice,
#       this list of conditions and the following disclaimer in the documentation
#       and/or other materials provided with the distribution.
#    #  Neither the name of the Authors nor the names of its contributors
#       may be used to endorse or promote products derived from this software
#       without specific prior written permission.
# 
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
#  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
#  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
#  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
#  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
#  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
#  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
#  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


from cpython.mem cimport PyMem_Malloc, PyMem_Realloc, PyMem_Free
from array import array

## https://gist.github.com/GaelVaroquaux/1249305
##from libc cimport stdint

ctypedef char   int8_t
ctypedef short  int16_t
ctypedef int    int32_t
ctypedef long   int64_t
ctypedef unsigned char   uint8_t
ctypedef unsigned short  uint16_t
ctypedef unsigned int    uint32_t
ctypedef unsigned long   uint64_t

ctypedef long long int *int64_ptr
ctypedef double *double_ptr


cdef enum glme_types:
     GLME_ANY = 0, GLME_BOOLEAN = 1, GLME_UINT = 2, GLME_INT = 3, GLME_FLOAT = 4,
     GLME_VECTOR = 5, GLME_STRING = 6, GLME_COMPLEX = 7, GLME_ARRAY = 10, GLME_MAP = 11,
     GLME_BASE_MAX = 15


cdef extern from "gobber.h":
    int gob_encode_uint64(char *buf, size_t len, uint64_t ival)
    int gob_encode_int64(char *buf, size_t len, int64_t ival)
    int gob_encode_double(char *buf, size_t len, double dval)
    int gob_encode_bytes(char *buf, size_t len, void *s, size_t vlen)

cdef union decval:
    int64_t ival
    double fval

cdef extern:
    int gobdec_int64(decval *dc, char *buf, size_t len)
    int gobdec_uint64(decval *dc, char *buf, size_t len)
    int gobdec_double(decval *dc, char *buf, size_t len)

cdef class GBuffer:
    """GLME stream encoder and decoder.
    """
    cdef readonly unsigned long size
    cdef readonly unsigned long count
    cdef readonly unsigned long current
    cdef int owner
    cdef char *buf
     
    def __cinit__(self, size_t num=0):
        self.owner = 0
        if num > 0:
            self.buf = <char *>PyMem_Malloc(num)
            if not self.buf:
                raise MemoryError()
            self.owner = 1
        self.size = num
        self.count = 0
        self.current = 0

    def __dealloc__(self):
        if self.owner == 1:
            PyMem_Free(self.buf)
        self.size = 0

    def resize(self, size_t newsize):
        if self.owner != 1:
            raise  MemoryError()
        mem = <char *>PyMem_Realloc(self.buf, newsize)
        if not mem:
            raise MemoryError()
        self.buf = mem
        self.size = newsize

    def data(self):
        cdef bytes py_buf
        py_buf = self.buf[:self.count]
        return py_buf

    def reset(self):
        """Reset read and write pointers"""
        self.count = 0
        self.current = 0

    def seek(self, size_t index):
        """Set read pointer"""
        if index > self.count:
            raise IndexError()
        self.current = index

    cdef int __encode_uint_val(self, uint64_t ival):
        cdef int n
        n = gob_encode_uint64(&self.buf[self.count], self.size-self.count, ival)
        if n < 0:
            self.resize(self.size+1024)
            n = gob_encode_uint64(&self.buf[self.count], self.size-self.count, ival)
        if n > 0:
            self.count = self.count + n
        return n

    cdef int __encode_int_val(self, int64_t ival):
        cdef int n
        n = gob_encode_int64(&self.buf[self.count], self.size-self.count, ival)
        if n < 0:
            self.resize(self.size+1024)
            n = gob_encode_int64(&self.buf[self.count], self.size-self.count, ival)
        if n > 0:
            self.count = self.count + n
        return n
            
    cdef int __encode_float_val(self, double fval):
        cdef int n
        n = gob_encode_double(&self.buf[self.count], self.size-self.count, fval)
        if n < 0:
            self.resize(self.size+1024)
            n = gob_encode_double(&self.buf[self.count], self.size-self.count, fval)
        if n > 0:
            self.count = self.count + n
        return n

    def encodeUIntValue(self, uint64_t ival):
        """Encode uint value"""
        return self.__encode_uint_val(ival)

    def encodeIntValue(self, int64_t ival):
        """Encode int value"""
        return self.__encode_int_val(ival)

    def encodeInt(self, ival):
        """Encode int element"""
        __at = self.count
        n = self.__encode_int_val(GLME_INT)
        if n > 0:
            n = self.__encode_int_val(ival)
        if n > 0:
            return self.count - __at
        self.count = __at
        return n

    def encodeFloatValue(self, fval):
        """Encode floating point value"""
        self.__encode_float_val(fval)
        
    def encodeFloat(self, fval):
        """Encode floating point element"""
        __at = self.count
        n = self.__encode_int_val(GLME_FLOAT)
        if n > 0:
            n = self.__encode_float_val(fval)
        if n > 0:
            return self.count - __at
        self.count = __at
        return n
        
    def encodeBoolValue(self, bval):
        __at = self.count
        ival = 1
        if bval == False:
            ival = 0
        self.encodeUIntValue(ival)
        return self.count - __at

    def encodeBool(self, bval):
        __at = self.count
        n = self.encodeIntValue(GLME_BOOLEAN)
        if n > 0:
            n = self.encodeBoolValue(bval)
        if n > 0:
            return self.count - __at
        self.count = __at

    cdef int __encode_bytes(self, char[:] bdata):
        n = gob_encode_bytes(&self.buf[self.count], self.size-self.count, <void *>&bdata[0], len(bdata))
        if n > 0:
            self.count = self.count + n
        return n
        
    cdef int __encode_array_uint8(self, unsigned char[:] arr):
        __at = self.count
        self.__encode_int_val(GLME_UINT)
        self.__encode_uint_val(len(arr))
        for u in arr:
            self.__encode_uint_val(u)
        return self.count - __at

    cdef int __encode_array_int8(self, char[:] arr):
        __at = self.count
        self.__encode_int_val(GLME_INT)
        self.__encode_uint_val(len(arr))
        for u in arr:
            self.__encode_int_val(u)
        return self.count - __at

    cdef int __encode_array_uint16(self, uint16_t[:] arr):
        __at = self.count
        self.__encode_int_val(GLME_UINT)
        self.__encode_uint_val(len(arr))
        for u in arr:
            self.__encode_uint_val(u)
        return self.count - __at

    cdef int __encode_array_int16(self, int16_t[:] arr):
        __at = self.count
        self.__encode_int_val(GLME_INT)
        self.__encode_uint_val(len(arr))
        for u in arr:
            self.__encode_int_val(u)
        return self.count - __at

    cdef int __encode_array_uint32(self, uint32_t[:] arr):
        __at = self.count
        self.__encode_int_val(GLME_UINT)
        self.__encode_uint_val(len(arr))
        for u in arr:
            self.__encode_uint_val(u)
        return self.count - __at

    cdef int __encode_array_int32(self, int32_t[:] arr):
        __at = self.count
        self.__encode_int_val(GLME_INT)
        self.__encode_uint_val(len(arr))
        for u in arr:
            self.__encode_int_val(u)
        return self.count - __at

    cdef int __encode_array_uint64(self, uint64_t[:] arr):
        __at = self.count
        self.__encode_int_val(GLME_UINT)
        self.__encode_uint_val(len(arr))
        for u in arr:
            self.__encode_uint_val(u)
        return self.count - __at

    cdef int __encode_array_int64(self, int64_t[:] arr):
        __at = self.count
        self.__encode_int_val(GLME_INT)
        self.__encode_uint_val(len(arr))
        for u in arr:
            self.__encode_int_val(u)
        return self.count - __at
                
    cdef int __encode_array_float(self, float[:] arr):
        __at = self.count
        self.encodeIntValue(GLME_FLOAT)
        self.encodeUIntValue(len(arr))
        for u in arr:
            self.encodeFloatValue(u)
        return self.count - __at

    cdef int __encode_array_double(self, double[:] arr):
        __at = self.count
        self.encodeIntValue(GLME_FLOAT)
        self.encodeUIntValue(len(arr))
        for u in arr:
            self.encodeFloatValue(u)
        return self.count - __at

    def encodeByteData(self, bstr, encoding='utf-8'):
        """Encode byte, bytearray or string value"""
        b_arr = bstr
        if type(bstr) == str:
            b_arr = bytearray(bstr, encoding=encoding)
        elif type(bstr) == bytes:
            b_arr = bytearray(bstr)
        return self.__encode_bytes(b_arr)
            
    def encodeBytes(self, bstr):
        """Encode byte/bytearray vector element"""
        __at = self.count
        if type(bstr) == str:
            raise TypeError()
        n = self.encodeIntValue(GLME_VECTOR)
        if n > 0:
            n = self.encodeByteData(bstr)
        if n > 0:
            return self.count - __at
        self.count = __at
        return n

    def encodeString(self, bstr, encoding='utf-8'):
        """Encode string element"""
        __at = self.count
        n = self.encodeIntValue(GLME_STRING)
        if n > 0:
            n = self.encodeByteData(bstr, encoding=encoding)
        if n > 0:
            return self.count - __at
        self.count = __at
        return n
        
    def encodeArrayData(self, arr):
        """Encode array.array value"""
        if type(arr) != array:
           raise TypeError("not an array!")
        __at = self.count
        signed = arr.typecode.islower()
        tc = arr.typecode.lower()
        if tc == 'b':
            # uint8_t and int8_t
            if signed:
                self.__encode_array_int8(arr)
            else:
                self.__encode_array_uint8(arr)

        elif tc == 'h':
            # uin16_t and int16_t
            if signed:
                self.__encode_array_int16(arr)
            else:
                self.__encode_array_uint16(arr)

        elif tc == 'i':
            # uin32_t and int32_t
            if signed:
                self.__encode_array_int32(arr)
            else:
                self.__encode_array_uint32(arr)

        elif tc == 'q' or tc == 'l':
            # uin64_t and int64_t
            if signed:
                self.__encode_array_int64(arr)
            else:
                self.__encode_array_uint64(arr)

        elif tc == 'f':
            # float32
            self.__encode_array_float(arr)
        elif tc == 'd':
            # float64
            self.__encode_array_double(arr)
        return self.count - __at
            
    def encodeDictData(self, dct):
        """Encode dictionary value"""
        __at = self.count
        keytype = type(list(dct.keys())[0])
        if keytype == str:
            self.encodeIntValue(GLME_STRING)
        elif keytype == int:
            self.encodeIntValue(GLME_INT)
        else:
            raise TypeError("Key type " + str(keytype) + " not supported")

        self.encodeIntValue(GLME_ANY)
        self.encodeUIntValue(len(dct))
        if keytype == str:
            for k, v in dct.items():
                self.encodeByteData(k)
                self.encodeElement(v)
        elif keytype == int:
            for k, v in dct.items():
                self.encodeIntValue(k)
                self.encodeElement(v)
        return self.count - __at

    def encodeDict(self, dct):
        """Encode dictionary element"""
        __at = self.count
        if type(dct) != dict:
            raise TypeError()
        self.encodeIntValue(GLME_MAP)
        self.encodeDictData(dct)
        return self.count - __at

    def encodeElement(self, e):
        """Encode element"""
        __at = self.count
        if type(e) == int:
            self.encodeInt(e)
        elif type(e) == float:
            self.encodeFloat(e)
        elif type(e) == str:
            self.encodeString(e)
        elif type(e) == bytes or type(e) == bytearray:
            self.encodeBytes(e)
        elif type(e) == array:
            self.encodeArray(e)
        elif type(e) == dict:
            self.encodeDict(e)
        elif type(e) == list:
            self.encodeArray(e)
        elif type(e) == bool:
            self.encodeBool(e)
        else:
            raise TypeError("encoding of " + str(type(e)) + " not supported")
        return self.count - __at

    def encodeListData(self, lst):
        """Encode list value"""
        __at = self.count
        self.encodeIntValue(GLME_ANY)
        self.encodeUIntValue(len(lst))
        for e in lst:
            self.encodeElement(e)
        return self.count - __at

    def encodeArray(self, arr):
        """Encode array or list element"""
        if type(arr) != array and type(arr) != list:
            raise TypeError()
        __at = self.count
        self.encodeIntValue(GLME_ARRAY)
        if type(arr) == array:
            self.encodeArrayData(arr)
        else:
            self.encodeListData(arr)
        return self.count - __at
            

    def encode(self, *args):
        """Encode elements"""
        __at = self.count
        for e in args:
            self.encodeElement(e)
        return self.count - __at

    cdef int64_t __decode_int(self):
        cdef decval dc
        n = gobdec_int64(&dc, &self.buf[self.current], self.count-self.current)
        if n < 0:
            raise IndexError()
        self.current = self.current + n
        return dc.ival

    cdef int64_t __decode_uint(self):
        cdef decval dc
        n = gobdec_uint64(&dc, &self.buf[self.current], self.count-self.current)
        if n < 0:
            raise IndexError()
        self.current = self.current + n
        return dc.ival

    cdef double __decode_float(self):
        cdef decval dc
        n = gobdec_double(&dc, &self.buf[self.current], self.count-self.current)
        if n < 0:
            raise IndexError()
        self.current = self.current + n
        return dc.fval

    def decodeIntValue(self):
        """Decode int value"""
        return self.__decode_int()

    def decodeUIntValue(self):
        """Decode unsigned int value"""
        return self.__decode_uint()

    def decodeInt(self):
        """Decode int element"""
        cdef int64_t ival
        __at = self.current
        ival = self.__decode_int()
        if ival != GLME_INT and ival != GLME_UINT:
            self.current = __at
            raise TypeError()
        if ival == GLME_UINT:
            return self.__decode_uint()
        return self.__decode_int()

    def decodeBoolValue(self):
        """Decode boolean value"""
        ival =  self.__decode_uint()
        if ival == 0:
            return False
        return True
        
    def decodeBool(self):
        """Decode boolean element"""
        __at = self.current
        ival = self.__decode_int()
        if ival != GLME_BOOLEAN:
            self.current = __at
            raise TypeError()
        return self.decodeBoolValue()

    def decodeFloatValue(self):
        """Decode floating point value"""
        return self.__decode_float()

    def decodeFloat(self):
        """Decode floating point element"""
        cdef int64_t ival
        __at = self.current
        ival = self.__decode_int()
        if ival != GLME_FLOAT:
            self.current = __at
            raise TypeError()
        return self.__decode_float()
            
    def decodeByteData(self):
        """Decode byte vector value"""
        cdef int64_t ilen
        __at = self.current
        ilen = self.__decode_uint()
        b_arr = bytearray(self.buf[self.current:self.current+ilen])
        self.current = self.current + ilen
        return b_arr

    def decodeBytes(self):
        """Decode byte vector element"""
        cdef int64_t ival
        __at = self.current
        ival = self.__decode_int()
        if ival != GLME_VECTOR:
            self.current = __at
            raise TypeError()
        return self.decodeByteData()

    def decodeString(self, encoding='utf-8'):
        """Decode string element"""
        cdef int64_t ival
        __at = self.current
        ival = self.__decode_int()
        if ival != GLME_STRING:
            self.current = __at
            raise TypeError()
        b_arr = self.decodeByteData()
        return str(b_arr, encoding=encoding)
        
    cdef int __decode_array_int8(self, size_t count, int8_t[:] b_arr):
        for i in range(count):
            b_arr[i] = <int8_t>self.__decode_int()
        return 0

    cdef int __decode_array_uint8(self, size_t count, uint8_t[:] b_arr):
        for i in range(count):
            b_arr[i] = <uint8_t>self.__decode_uint()
        return 0

    cdef int __decode_array_int16(self, size_t count, int16_t[:] b_arr):
        for i in range(count):
            b_arr[i] = <int16_t>self.__decode_int()
        return 0

    cdef int __decode_array_uint16(self, size_t count, uint16_t[:] b_arr):
        for i in range(count):
            b_arr[i] = <uint16_t>self.__decode_uint()
        return 0

    cdef int __decode_array_int32(self, size_t count, int32_t[:] b_arr):
        for i in range(count):
            b_arr[i] = <int32_t>self.__decode_int()
        return 0

    cdef int __decode_array_uint32(self, size_t count, uint32_t[:] b_arr):
        for i in range(count):
            b_arr[i] = <uint32_t>self.__decode_uint()
        return 0

    cdef int __decode_array_int64(self, size_t count, int64_t[:] b_arr):
        for i in range(count):
            b_arr[i] = self.__decode_int()
        return 0

    cdef int __decode_array_uint64(self, size_t count, uint64_t[:] b_arr):
        for i in range(count):
            b_arr[i] = self.__decode_uint()
        return 0

    cdef int __decode_array_float(self, size_t count, float[:] b_arr):
        for i in range(count):
            b_arr[i] = <float>self.__decode_float()
        return 0

    cdef int __decode_array_double(self, size_t count, double[:] b_arr):
        for i in range(count):
            b_arr[i] = self.__decode_float()
        return 0

    def decodeTypedValue(self, etyp):
        """Decode value of given type id"""
        if etyp == GLME_INT:
            return self.decodeIntValue()
        elif etyp == GLME_UINT:
            return self.decodeUIntValue()
        elif etyp == GLME_BOOLEAN:
            return self.decodeBoolValue()
        elif etyp == GLME_FLOAT:
            return self.decodeFloatValue()
        elif etyp == GLME_VECTOR:
            return self.decodeByteData()
        elif etyp == GLME_STRING:
            return self.decodeByteData()
        elif etyp == GLME_ARRAY:
            return self.decodeArrayData()
        elif etyp == GLME_MAP:
            return self.decodeDictData({})
        else:
            raise TypeError("Unexpected type code {0} decoded at {1}.".format(etyp, self.current))

    def decodeElement(self, typecode=''):
        """Decode element"""
        etyp = self.decodeIntValue()
        if etyp == GLME_INT:
            return self.decodeIntValue()
        elif etyp == GLME_UINT:
            return self.decodeUIntValue()
        elif etyp == GLME_BOOLEAN:
            return self.decodeBoolValue()
        elif etyp == GLME_FLOAT:
            return self.decodeFloatValue()
        elif etyp == GLME_VECTOR:
            return self.decodeByteData()
        elif etyp == GLME_STRING:
            return str(self.decodeByteData(), encoding='utf-8')
        elif etyp == GLME_ARRAY:
            return self.decodeArrayData(typecode=typecode)
        elif etyp == GLME_MAP:
            return self.decodeDictData({})
        else:
            raise TypeError("Unexpected type code {0} decoded at {1}".format(etyp, self.current))

    def __decode_list(self, count, lst):
        for i in range(count):
            lst.append(self.decodeElement())
        return lst
        
    def __decode_string_list(self, count, lst):
        for i in range(count):
            lst.append(self.decodeByteData())
        return lst

    def decodeArrayData(self, typecode=''):
        """Decode array or list data"""
        __at = self.current
        etyp = self.decodeIntValue()
        count = self.decodeUIntValue()
        if etyp == GLME_INT:
            if typecode == '':
                typecode = 'q'
            elif typecode not in ['b', 'h', 'i', 'l', 'q']:
                self.current = __at
                raise TypeError()
            b_arr = array(typecode, [0]*count)
            if typecode == 'q' or typecode == 'l':
                self.__decode_array_int64(count, b_arr)
            elif typecode == 'i':
                self.__decode_array_int32(count, b_arr)
            elif typecode == 'h':
                self.__decode_array_int16(count, b_arr)
            else:
                self.__decode_array_int8(count, b_arr)
        elif etyp == GLME_UINT:
            if typecode == '':
                typecode = 'Q'
            elif typecode not in ['B', 'H', 'I', 'L', 'Q']:
                self.current = __at
                raise TypeError()
            b_arr = array(typecode, [0]*count)
            if typecode == 'q' or typecode == 'l':
                self.__decode_array_uint64(count, b_arr)
            elif typecode == 'i':
                self.__decode_array_uint32(count, b_arr)
            elif typecode == 'h':
                self.__decode_array_uint16(count, b_arr)
            else:
                self.__decode_array_uint8(count, b_arr)
        elif etyp == GLME_FLOAT:
            if typecode == '':
                typecode = 'd'
            elif typecode not in ['f', 'd']:
                self.current = __at
                raise TypeError()
            b_arr = array(typecode, [0.0]*count)
            if typecode == 'f':
                self.__decode_array_float(count, b_arr)
            else:
                self.__decode_array_double(count, b_arr)

        elif etyp == GLME_STRING:
            self.__decode_string_list(count, [])

        elif etyp == GLME_ANY:
            return self.__decode_list(count, [])

        return b_arr

    def decodeArray(self, typecode=''):
        """Decode array or list element"""
        __at = self.current
        typ = self.decodeIntValue()
        if typ != GLME_ARRAY:
            self.current = __at
            raise TypeError()
        return self.decodeArrayData(typecode=typecode)

    def decodeDictData(self, dct):
        """Decode dictionary data"""
        ktyp = self.decodeIntValue()
        if ktyp != GLME_STRING and ktyp != GLME_INT:
            raise TypeError("Key type " + str(ktyp) + " not supported for dictionary")
        etyp = self.decodeIntValue()
        count = self.decodeUIntValue()
        if etyp == GLME_ANY:
            for i in range(count):
                if ktyp == GLME_INT:
                    kval = self.decodeIntValue()
                else:
                    kval = str(self.decodeByteData(), encoding='utf-8')
                elem = self.decodeElement()
                dct[kval] = elem
        else:
            for i in range(count):
                if ktyp == GLME_INT:
                    kval = self.decodeIntValue()
                else:
                    kval = str(self.decodeByteData(), encoding='utf-8')
                elem = self.decodeTypedValue(etyp)
                dct[kval] = elem
        return dct

    def decodeDict(self):
        """Decode dictionary element"""
        etyp = self.decodeIntValue()
        if etyp != GLME_MAP:
            raise TypeError("Current element not a MAP type")
        return self.decodeDictData({})

    def decode(self, typecode=''):
        """Decode next element."""
        if self.current >= self.count:
            return None
        return self.decodeElement(typecode=typecode)

# Local Variables:
# indent-tabs-mode: nil
# End:

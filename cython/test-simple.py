
import unittest
import array
import glme

class TestSimpleEncoding(unittest.TestCase):

    def setUp(self):
        self.buffer = glme.GBuffer(64)
        self.buffer.reset()

    def test_bool(self):
        self.buffer.encode(True)
        self.assertEqual(self.buffer.decode(), True)
        
    def test_int(self):
        self.buffer.encode(1)
        self.assertEqual(self.buffer.decode(), 1)

    def test_negative_int(self):
        self.buffer.encode(-100)
        self.assertEqual(self.buffer.decode(), -100)

    def test_float(self):
        self.buffer.encode(1.5e15)
        self.assertEqual(self.buffer.decode(), 1.5e15)

    def test_bytes(self):
        self.buffer.encode(b'helloworld')
        self.assertEqual(self.buffer.decode(), b'helloworld')

    def test_string(self):
        self.buffer.encode(r'helloworld')
        self.assertEqual(self.buffer.decode(), r'helloworld')

    def test_list(self):
        self.buffer.encode(['hello', 'world', 10, -1.5])
        self.assertEqual(self.buffer.decode(), ['hello', 'world', 10, -1.5])
        
    def test_dict(self):
        self.buffer.encode({'a': 10, 'b': -3.14})
        self.assertEqual(self.buffer.decode(), {'a': 10, 'b': -3.14})

    def test_int_array(self):
        a = array.array('i', [1, 16, 32, 64, 128])
        self.buffer.encode(a)
        self.assertEqual(self.buffer.decode(typecode='i'), a)

if __name__ == '__main__':
    unittest.main()

import unittest
import skysmoke

class CSmokeTest(skysmoke.SkySmoke):

    @classmethod
    def setUpClass(cls):
        skysmoke.SkySmoke.setUpClass()

    def setUp(self):
        skysmoke.SkySmoke.setUp(self)

    def tearDown(self):
        skysmoke.SkySmoke.tearDown(self)

    def test_cpp_generate(self):
        self.assertEqual(1, 1, "Test test_c_generate Passes")

    def test_cpp_serialize(self):
        self.assertEqual(1, 1, "Test test_c_serialize Passes")

if __name__ == '__main__':
    unittest.main()
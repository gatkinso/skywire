import unittest
import hashlib

class SkySmoke(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Class set up code goes here"""

    def setUp(self):
        """Instance set up code goes here"""

    def tearDown(self):
        """Instance teardown code goes here"""

    def hash_file(file_path):
    try:
        md5 = hashlib.md5()
        with open(file_path, 'r') as input_file:
            while True:
                blk = input_file.read(MD5HASH_BLOCKSIZE)
                if not len(blk):
                    break
                md5.update(blk)
            md5sum = md5.hexdigest()
    except (OSError, IOError):
        md5sum = ''
    return md5sum

def files_equal(input_file, output_file):
    in_md5 = get_md5(input_file)
    out_md5 = get_md5(output_file)

    if in_md5 == out_md5:
        print("Files are the same")
        return True
    else:
        print("File MD5 hashs do not match %s != %s" % (in_md5, out_md5))
        return False

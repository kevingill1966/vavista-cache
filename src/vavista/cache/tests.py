import unittest

from vavista.cache import INOUT, mget, mset

class TestCache(unittest.TestCase):

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def testTopLevel(self):
        mset("^DD", "1")
        value = mget("^DD")
        self.assertEquals(value, "1")

    def testLevel1(self):
        mset("^DD(0)", "2")
        value = mget("^DD(0)")
        self.assertEquals(value, "2")

    def testLevel2(self):
        mset("^DD(0,0)", "3")
        value = mget("^DD(0,0)")
        self.assertEquals(value, "3")


test_cases = (TestCache,)

def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    for test_class in test_cases:
        tests = loader.loadTestsFromTestCase(test_class)
        suite.addTests(tests)
    return suite



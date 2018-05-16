import unittest
import vmtranslator

class TestVmTranslator(unittest.TestCase):
    def testParseLine(self):
        ex1 = vmtranslator.parseLine("push constant 0")
        self.assertEqual(ex1, ["push", "constant", "0"])

        ex2 = vmtranslator.parseLine("push local 1   //  asdf asdf adsf ")
        self.assertEqual(ex2, ["push", "local", "1"])

        ex3 = vmtranslator.parseLine("// just a comment")
        self.assertEqual(ex3, [])

if __name__ == "__main__":
    unittest.main()


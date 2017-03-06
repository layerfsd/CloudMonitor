import unittest
import random

class TestSequenceFunctions(unittest.TestCase):

   def setUp(self):
       self.seq = list(range(10))

   def test_shuffle(self):
       # make sure the shuffled sequence does not lose any elements
       random.shuffle(self.seq)
       self.seq.sort()
       self.assertEqual(self.seq, list(range(10)))

       # should raise an exception for an immutable sequence
       self.assertRaises(TypeError, random.shuffle, (1,2,3))

   def test_choice(self):
       element = random.choice(self.seq)
       self.assertTrue(element in self.seq)


   def test_error(self):
          element = random.choice(self.seq)
          self.assertFalse(element not in self.seq)


if __name__ == '__main__':
   unittest.main()
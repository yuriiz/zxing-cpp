from unittest import TestCase

from PIL import Image

from zxing import decode, error


class MainTest(TestCase):

    def test_decode(self):
        img = Image.open('tests/sample.png')
        self.assertEqual(
            decode(img.width, img.height, 4, img.convert('RGBA').tostring()),
            'It works!')
        self.assertRaises(error, decode, 1, 1, 4, ' ' * 4)

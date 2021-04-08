#!/usr/env/python3

import numpy as np
import matplotlib.pyplot as plt
from PIL import Image

# YUV420P
y420 = 'out/yuv_420p.y'
u420 = 'out/yuv_420p.u'
v420 = 'out/yuv_420p.v'
gray420 = 'out/yuv_420p.gray'
# YUV444P
y444 = './out/yuv_444p.y'
u444 = './out/yuv_444p.u'
v444 = './out/yuv_444p.v'
# RGB888
r888 = './out/rgb_888.r'
g888 = './out/rgb_888.g'
b888 = './out/rgb_888.b'


def imshow(uri, w=256, h=256):
    mat = np.fromfile(uri, dtype='uint8')
    mat.resize(w, h)
    img = Image.fromarray(mat)
    img.show()


# imshow(y420)
# imshow(u420, 128, 128)
# imshow(v420, 128, 128)

# imshow(y444)
# imshow(u444)
# imshow(v444)

# imshow(r888, 500, 500)
# imshow(g888, 500, 500)
# imshow(b888, 500, 500)

imshow(gray420)

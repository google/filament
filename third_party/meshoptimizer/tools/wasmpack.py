#!/usr/bin/python3

import sys

# regenerate with wasmpack.py generate
table = [32, 0, 65, 2, 1, 106, 34, 33, 3, 128, 11, 4, 13, 64, 6, 253, 10, 7, 15, 116, 127, 5, 8, 12, 40, 16, 19, 54, 20, 9, 27, 255, 113, 17, 42, 67, 24, 23, 146, 148, 18, 14, 22, 45, 70, 69, 56, 114, 101, 21, 25, 63, 75, 136, 108, 28, 118, 29, 73, 115]

palette = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:;";

def encode(buffer):
	result = ''

	for ch in buffer.read():
		if ch in table:
			index = table.index(ch)
			result += palette[index]
		else:
			result += palette[60 + ch // 64]
			result += palette[ch % 64]

	return result

def stats(buffer):
	hist = [0] * 256
	for ch in buffer.read():
		hist[ch] += 1

	result = [i for i in range(256)]
	result.sort(key=lambda i: hist[i], reverse=True)

	return result

if sys.argv[-1] == 'generate':
	print(stats(sys.stdin.buffer)[:60])
else:
	print(encode(sys.stdin.buffer))

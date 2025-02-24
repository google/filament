import numpy
import numpy as np
import imageio
import sys
import argparse

iteration = 0
max_num_pixels = 100000

# Provide a list of indices to attemt to blur
def initial_pixels(imgin, blur_threshold):

	width = imgin.shape[1]
	height = imgin.shape[0]
	channels = imgin.shape[2]

	print "Image: ", width, "x", height," - ", channels, " channels -- Initial number of pixels over cutoff: ",

	indices = numpy.argwhere(imgin.max(axis=2)>blur_threshold)

	print len(indices)

	if max_num_pixels and len(indices) > max_num_pixels:
		print "This image contains a very large number of pixels over the threshold. It may take a large amount of time to process, and will be very strongly blurred. Consider raising the threshold or the maximum number of initial affected pixels. Press ctrl-c to cancel."
	return indices

def blur_pixel_list(imgin, original_pixel_list, blur_threshold, blur_factor):
	global iteration
	iteration += 1
	width = imgin.shape[1]
	height = imgin.shape[0]
	channels = imgin.shape[2]

	print " * Iteration:", iteration, "\tInput MAX: ", numpy.max(imgin),

	imgout = numpy.copy(imgin)
	pixel_list = set()

	# GENERATE A MASK

	if (blur_factor < 0 or blur_factor >1):
		raise "This is wrong"

	surrounding_factor = (1. - blur_factor) / 8.

	for coord in original_pixel_list:
		row = coord[0]
		col = coord[1]
		rowneg = 1 if row == 0 else row-1
		rowpos = height-2 if row == height-1 else row+1
		colneg = width - 1 if col == 0 else col-1
		colpos = 0 if col == width-1 else col+1
		coords = [(rowneg,colneg),(rowneg,col),(rowneg,colpos),(row,colneg),(row,colpos),(rowpos,colneg),(rowpos,col),(rowpos,colpos)]

		imgout[row, col] = imgout[row, col] - (imgin[row,col] * (1. - blur_factor))

		surrounding_delta = imgin[row ,col] * surrounding_factor


		for offset_coord in coords:
			imgout[offset_coord] = imgout[offset_coord] + surrounding_delta
			if imgout[offset_coord][0] > blur_threshold or imgout[offset_coord][1] > blur_threshold or imgout[offset_coord][2] > blur_threshold:
				pixel_list.add(offset_coord)

	print  "\tOUTPUT Max: ", numpy.max(imgout), "  \tPixels over cutoff: ", len(pixel_list), numpy.sum(imgout)

	return imgout, pixel_list

def blur_with_threshold(imgin, blur_threshold):
	global iteration
	iteration = 0
	prev_pixel_list = initial_pixels(imgin, blur_threshold)
	imgout, pixel_list= blur_pixel_list(imgin, prev_pixel_list, blur_threshold, .25)
	while len(pixel_list):
		imgin = imgout
		prev_pixel_list = pixel_list
		imgout, pixel_list = blur_pixel_list(imgin, prev_pixel_list, blur_threshold, .25)
	return imgout

if __name__ == "__main__":
	imageio.plugins.freeimage.download()
	parser = argparse.ArgumentParser()
	description='Scale, clip, or blur an image to a predetermined value. Default is scaling to a max value of 60,000'
	parser.add_argument("input_file", help="Input image.")
	parser.add_argument("-m", "--mode", help="Operation mode. Choices: scale, clip, blur. 'scale' scales the image so that its maximum value equals threshold. 'clip' clips maximum values so that they are at most equal to the threshold. 'blur' diffuses pixels into neighbouring pixels until no pixels over the maximum exist (Not guaranteed to finish in finite time). Default: 'scale'", choices=['scale','blur', 'clip'], default='scale')
	parser.add_argument("-t", "--threshold", default=60000, type=float, help="The max value of the output image. Can be fractional. Default: 60,000")
	args = parser.parse_args()

	imgin = imageio.imread(args.input_file)

	output_filename = args.input_file

	if args.mode == "blur":
		imgout = blur_with_threshold(imgin, args.threshold)
		output_filename += ".blurred_to_"+str(int(args.threshold)) + ".exr"
		imageio.imwrite(output_filename, imgout)
	elif args.mode == "scale":
		imginmax = imgin.max();
		if imginmax > args.threshold:
			imgout = imgin * (args.threshold / imginmax)
		else:
			imgout = imgin
			imginmax = args.threshold

		output_filename += ".scaled_by_"+str(args.threshold / imginmax) + ".exr"
		imageio.imwrite(output_filename, imgout)
	elif args.mode == "clip":
		imgout = imgin.copy()
		imgout.fill(args.threshold)
		imgout = numpy.minimum(imgin, imgout)

		output_filename += ".scaled_by_"+str(args.threshold / imginmax) + ".exr"
		imageio.imwrite(output_filename, imgout)

	print("Saved processed image as: " + output_filename)
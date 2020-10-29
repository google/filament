/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <AVFoundation/AVFoundation.h>

#define USE_ADAPTOR 0

void encode(CVPixelBufferRef pixelBuffer) {
    NSError* error = nil;
    AVAssetWriter* videoWriter = [[AVAssetWriter alloc] initWithURL:[NSURL fileURLWithPath:@"output.mp4"]
                                                           fileType:AVFileTypeAppleM4V
                                                              error:&error];
    assert(!error);

    NSDictionary* videoSettings = [NSDictionary dictionaryWithObjectsAndKeys:
            AVVideoCodecTypeH264, AVVideoCodecKey,
            [NSNumber numberWithInt:512], AVVideoWidthKey,
            [NSNumber numberWithInt:512], AVVideoHeightKey,
            nil];

    AVAssetWriterInput* writerInput = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeVideo
                                                                         outputSettings:videoSettings];
    writerInput.expectsMediaDataInRealTime = YES;

    [videoWriter addInput:writerInput];

    NSDictionary *source_attributes = [NSDictionary dictionaryWithObjectsAndKeys:
         [NSNumber numberWithInt:kCVPixelFormatType_32ARGB], kCVPixelBufferPixelFormatTypeKey,
         [NSNumber numberWithInt:512], kCVPixelBufferWidthKey,
         [NSNumber numberWithInt:512], kCVPixelBufferHeightKey,
          nil];

    [videoWriter startWriting];
    [videoWriter startSessionAtSourceTime:kCMTimeZero];

    // Add a single frame.
    assert(writerInput.readyForMoreMediaData);

#if USE_ADAPTOR
    AVAssetWriterInputPixelBufferAdaptor* adaptor  = [AVAssetWriterInputPixelBufferAdaptor
        assetWriterInputPixelBufferAdaptorWithAssetWriterInput:writerInput
                                   sourcePixelBufferAttributes:source_attributes];

    double seconds = 0.0;
    BOOL result = [adaptor appendPixelBuffer:pixelBuffer
                        withPresentationTime:CMTimeMakeWithSeconds(seconds, 30)];
    assert(result);
#else
    CMSampleBufferRef sampleBuffer;
    CMVideoFormatDescriptionRef formatDesc;
    CMVideoFormatDescriptionCreateForImageBuffer(kCFAllocatorDefault, pixelBuffer, &formatDesc);

    double seconds = 0.0;
    CMTime pts = CMTimeMake(seconds, 30);
    CMTime duration = CMTimeMake(1, 30);
    CMSampleTimingInfo timing = { duration, pts, kCMTimeInvalid };

    OSStatus result = CMSampleBufferCreateReadyWithImageBuffer(kCFAllocatorDefault,
            (CVImageBufferRef) pixelBuffer, formatDesc, &timing, &sampleBuffer);
    assert(result == 0);
    [writerInput appendSampleBuffer:sampleBuffer];
#endif

    // Finish
    [writerInput markAsFinished];
    [videoWriter finishWritingWithCompletionHandler:^{
        NSLog(@"Finished writing");
    }];
}

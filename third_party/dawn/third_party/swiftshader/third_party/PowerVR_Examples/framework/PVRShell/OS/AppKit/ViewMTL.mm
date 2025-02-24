/*!
\brief Implementation of the OS specific part of the PVRShell class for the AppKit macOS implementation
\file PVRShell/OS/AppKit/ShellOS.mm
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "ViewMTL.h"
#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>

@implementation ViewMTL

- (ViewMTL*) initWithFrame:(NSRect) frame
{
    m_device = MTLCreateSystemDefaultDevice();
    [m_device retain];
    [super initWithFrame:frame];
    self.wantsLayer = YES;
    return self;
}

- (void)dealloc
{
    [m_device release];
    [super dealloc];
}

- (BOOL) wantsUpdateLayer { return YES; }

+ (Class) layerClass { return [CAMetalLayer class]; }

- (CALayer*) makeBackingLayer
{
    CAMetalLayer* layer = [self.class.layerClass layer];
    CGSize viewScale = [self convertSizeToBacking: CGSizeMake(1, 1)];
    layer.contentsScale = MIN(viewScale.width, viewScale.height);
    layer.device = m_device;
    layer.backgroundColor = CGColorCreateGenericRGB(0.0, 0.0, 0.0, 1.0);
    return layer;
}

@end

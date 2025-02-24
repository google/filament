/*!
 \brief Contains the declaration of the MVKView class.
 \file PVRShell/OS/AppKit/ViewMTL.h
 \author PowerVR by Imagination, Developer Technology Team
 \copyright Copyright (c) Imagination Technologies Limited.
 */
#pragma once
#include <AppKit/NSView.h>

@interface ViewMTL : NSView
{
    id<MTLDevice>  m_device; // Metal device
}

-(ViewMTL*) initWithFrame:(NSRect) frame;

@end

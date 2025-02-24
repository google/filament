/*
 * DemoViewController.h
 *
 * Copyright (c) 2014-2018 The Brenwill Workshop Ltd. (http://www.brenwill.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import <AppKit/AppKit.h>

#pragma mark -
#pragma mark DemoViewController

/** The main view controller for the demo storyboard. */
@interface DemoViewController : NSViewController

- (void)quit;

@end

#pragma mark -
#pragma mark DemoView

/** The Metal-compatibile view for the demo Storyboard. */
@interface DemoView : NSView
@end

#include "ProgressBackend.h"

#include "imgui.h"
#include "imgui_impl_metal.h"
#include "imgui_impl_osx.h"

#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/QuartzCore.h>

@interface SCProgressWindowDelegate : NSObject <NSWindowDelegate>
@property (nonatomic, assign) void* progress;
@property (nonatomic, assign) void* closeFlag;
@end

@implementation SCProgressWindowDelegate
- (BOOL)windowShouldClose:(NSWindow*)sender {
    (void) sender;
    if (_closeFlag)
        *(bool*) _closeFlag = true;
    if (_progress)
        ((sc::Adobe::ExportProgress*) _progress)->RequestCancel();
    return NO;
}
@end

namespace sc::Adobe {
    namespace {
        class ProgressBackendMac final : public IProgressBackend {
        public:
            bool Create(const char* title, ExportProgress* progress) override {
                @autoreleasepool {
                    [NSApplication sharedApplication];

                    NSRect rect = NSMakeRect(0.0, 0.0, PROGRESS_WINDOW_WIDTH, PROGRESS_WINDOW_HEIGHT);
                    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable;

                    _device = MTLCreateSystemDefaultDevice();
                    if (!_device)
                        return false;

                    _window = [[NSWindow alloc] initWithContentRect:rect
                                                          styleMask:style
                                                            backing:NSBackingStoreBuffered
                                                              defer:NO];
                    [_window setTitle:[NSString stringWithUTF8String:title ? title : "Publishing"]];
                    [_window center];
                    [_window setLevel:NSFloatingWindowLevel];

                    _view = [[MTKView alloc] initWithFrame:rect device:_device];
                    _view.clearColor = MTLClearColorMake(0.16, 0.16, 0.16, 1.0);
                    _view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
                    _view.framebufferOnly = NO;
                    [_window setContentView:_view];

                    _commandQueue = [_device newCommandQueue];

                    _delegate = [[SCProgressWindowDelegate alloc] init];
                    _delegate.progress = progress;
                    _delegate.closeFlag = &_closeRequested;
                    [_window setDelegate:_delegate];

                    ImGui_ImplMetal_Init(_device);
                    ImGui_ImplOSX_Init(_view);
                    _initialized = true;

                    [_window makeKeyAndOrderFront:nil];
                    return true;
                }
            }

            void BeginFrame() override {
                @autoreleasepool {
                    NSEvent* event = nil;
                    while ((event = [NSApp nextEventMatchingMask:NSEventMaskUntilDate
                                                        inMode:NSDefaultRunLoopMode
                                                       dequeue:YES]) != nil) {
                        [NSApp sendEvent:event];
                    }

                    ImGui_ImplMetal_NewFrame([_view currentRenderPassDescriptor]);
                    ImGui_ImplOSX_NewFrame(_view);
                    ImGui::NewFrame();
                }
            }

            void EndFrame() override {
                @autoreleasepool {
                    ImGui::Render();

                    MTLRenderPassDescriptor* pass = [_view currentRenderPassDescriptor];
                    if (!pass)
                        return;

                    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
                    id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:pass];
                    ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, encoder);
                    [encoder endEncoding];

                    [commandBuffer presentDrawable:[_view currentDrawable]];
                    [commandBuffer commit];
                }
            }

            bool IsOpen() const override { return _window != nil && !_closeRequested; }

            void Destroy() override {
                @autoreleasepool {
                    if (_initialized) {
                        ImGui_ImplOSX_Shutdown();
                        ImGui_ImplMetal_Shutdown();
                        _initialized = false;
                    }

                    [_window setDelegate:nil];
                    [_window release];
                    [_view release];
                    [_delegate release];
                    [_commandQueue release];
                    [_device release];

                    _window = nil;
                    _view = nil;
                    _delegate = nil;
                    _commandQueue = nil;
                    _device = nil;
                }
            }

        private:
            NSWindow* _window = nil;
            MTKView* _view = nil;
            SCProgressWindowDelegate* _delegate = nil;
            id<MTLDevice> _device = nil;
            id<MTLCommandQueue> _commandQueue = nil;
            bool _closeRequested = false;
            bool _initialized = false;
        };
    }

    std::unique_ptr<IProgressBackend> CreateProgressBackend() {
        return std::make_unique<ProgressBackendMac>();
    }
}

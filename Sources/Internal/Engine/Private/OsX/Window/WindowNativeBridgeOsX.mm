#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/Window/WindowNativeBridgeOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <AppKit/NSWindow.h>
#import <AppKit/NSScreen.h>

#include "Engine/Window.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/OsX/Window/WindowBackendOsX.h"
#include "Engine/Private/OsX/Window/RenderViewOsX.h"
#include "Engine/Private/OsX/Window/WindowDelegateOsX.h"

#include "Platform/SystemTimer.h"
#include "Logger/Logger.h"

namespace DAVA
{
namespace Private
{
WindowNativeBridge::WindowNativeBridge(WindowBackend* windowBackend)
    : windowBackend(windowBackend)
    , window(windowBackend->window)
    , mainDispatcher(windowBackend->mainDispatcher)
{
}

WindowNativeBridge::~WindowNativeBridge() = default;

bool WindowNativeBridge::CreateWindow(float32 x, float32 y, float32 width, float32 height)
{
    // clang-format off
    NSUInteger style = NSTitledWindowMask |
                       NSMiniaturizableWindowMask |
                       NSClosableWindowMask |
                       NSResizableWindowMask;
    // clang-format on

    NSRect viewRect = NSMakeRect(x, y, width, height);
    windowDelegate = [[WindowDelegate alloc] initWithBridge:this];
    renderView = [[RenderView alloc] initWithFrame:viewRect andBridge:this];

    nswindow = [[NSWindow alloc] initWithContentRect:viewRect
                                           styleMask:style
                                             backing:NSBackingStoreBuffered
                                               defer:NO];
    [nswindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [nswindow setContentView:renderView];
    [nswindow setDelegate:windowDelegate];

    {
        float32 dpi = GetDpi();
        CGSize surfaceSize = [renderView convertSizeToBacking:viewRect.size];
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowCreatedEvent(window, viewRect.size.width, viewRect.size.height, surfaceSize.width, surfaceSize.height, dpi, eFullscreen::Off));
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, true));
    }

    [nswindow makeKeyAndOrderFront:nil];
    return true;
}

void WindowNativeBridge::ResizeWindow(float32 width, float32 height)
{
    [nswindow setContentSize:NSMakeSize(width, height)];
}

void WindowNativeBridge::CloseWindow()
{
    [nswindow close];
}

void WindowNativeBridge::SetTitle(const char8* title)
{
    NSString* nsTitle = [NSString stringWithUTF8String:title];
    [nswindow setTitle:nsTitle];
    [nsTitle release];
}

void WindowNativeBridge::SetFullscreen(Fullscreen newMode)
{
    bool isFullscreenRequested = newMode == eFullscreen::On;

    if (isFullscreen != isFullscreenRequested)
    {
        [nswindow toggleFullScreen:nil];
    }
}

void WindowNativeBridge::TriggerPlatformEvents()
{
    dispatch_async(dispatch_get_main_queue(), [this]() {
        windowBackend->ProcessPlatformEvents();
    });
}

void WindowNativeBridge::ApplicationDidHideUnhide(bool hidden)
{
    isAppHidden = hidden;
}

void WindowNativeBridge::WindowDidMiniaturize()
{
    isMiniaturized = true;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, false));
}

void WindowNativeBridge::WindowDidDeminiaturize()
{
    isMiniaturized = false;
}

void WindowNativeBridge::WindowDidBecomeKey()
{
    if (isMiniaturized || isAppHidden)
    {
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, true));
    }
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowFocusChangedEvent(window, true));
}

void WindowNativeBridge::WindowDidResignKey()
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowFocusChangedEvent(window, false));
    if (isAppHidden)
    {
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, false));
    }
}

void WindowNativeBridge::WindowDidResize()
{
    CGSize size = [renderView frame].size;
    CGSize surfSize = [renderView convertSizeToBacking:size];
    Fullscreen fullscreen = isFullscreen ? eFullscreen::On : eFullscreen::Off;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSizeChangedEvent(window, size.width, size.height, surfSize.width, surfSize.height, fullscreen));
}

void WindowNativeBridge::WindowDidChangeScreen()
{
    CGSize size = [renderView frame].size;
    CGSize surfSize = [renderView convertSizeToBacking:size];
    float32 dpi = GetDpi();
    Fullscreen fullscreen = isFullscreen ? eFullscreen::On : eFullscreen::Off;

    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSizeChangedEvent(window, size.width, size.height, surfSize.width, surfSize.height, fullscreen));
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowDpiChangedEvent(window, dpi));
}

bool WindowNativeBridge::WindowShouldClose()
{
    if (!windowBackend->closeRequestByApp)
    {
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateUserCloseRequestEvent(window));
        return false;
    }
    return true;
}

void WindowNativeBridge::WindowWillClose()
{
    windowBackend->WindowWillClose();
    mainDispatcher->SendEvent(MainDispatcherEvent::CreateWindowDestroyedEvent(window));

    [nswindow setContentView:nil];
    [nswindow setDelegate:nil];

    [renderView release];
    [windowDelegate release];
}

void WindowNativeBridge::WindowWillEnterFullScreen()
{
    isFullscreen = true;
}

void WindowNativeBridge::WindowWillExitFullScreen()
{
    isFullscreen = false;
}

void WindowNativeBridge::MouseClick(NSEvent* theEvent)
{
    eMouseButtons button = GetMouseButton(theEvent);
    if (button != eMouseButtons::NONE)
    {
        MainDispatcherEvent::eType type = MainDispatcherEvent::DUMMY;
        switch ([theEvent type])
        {
        case NSLeftMouseDown:
        case NSRightMouseDown:
        case NSOtherMouseDown:
            type = MainDispatcherEvent::MOUSE_BUTTON_DOWN;
            break;
        case NSLeftMouseUp:
        case NSRightMouseUp:
        case NSOtherMouseUp:
            type = MainDispatcherEvent::MOUSE_BUTTON_UP;
            break;
        default:
            return;
        }

        NSSize sz = [renderView frame].size;
        NSPoint pt = [theEvent locationInWindow];

        float32 x = pt.x;
        float32 y = sz.height - pt.y;
        eModifierKeys modifierKeys = GetModifierKeys(theEvent);
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseClickEvent(window, type, button, x, y, 1, modifierKeys, false));
    }
}

void WindowNativeBridge::MouseMove(NSEvent* theEvent)
{
    NSSize sz = [renderView frame].size;
    NSPoint pt = theEvent.locationInWindow;

    float32 x = pt.x;
    float32 y = sz.height - pt.y;

    eModifierKeys modifierKeys = GetModifierKeys(theEvent);
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseMoveEvent(window, x, y, modifierKeys, false));
}

void WindowNativeBridge::MouseEntered(NSEvent* theEvent)
{
}

void WindowNativeBridge::MouseExited(NSEvent* theEvent)
{
}

void WindowNativeBridge::MouseWheel(NSEvent* theEvent)
{
    // detect the wheel event device
    // http://stackoverflow.com/questions/13807616/mac-cocoa-how-to-differentiate-if-a-nsscrollwheel-event-is-from-a-mouse-or-trac
    if (NSEventPhaseNone != [theEvent momentumPhase] || NSEventPhaseNone != [theEvent phase])
    {
        // TODO: add support for mouse/touch in DispatcherEvent
        //event.device = DAVA::UIEvent::Device::TOUCH_PAD;
    }
    else
    {
        //event.device = DAVA::UIEvent::Device::MOUSE;
    }

    const float32 scrollK = 10.0f;

    NSSize sz = [renderView frame].size;
    NSPoint pt = theEvent.locationInWindow;

    float32 x = pt.x;
    float32 y = sz.height - pt.y;
    float32 deltaX = [theEvent scrollingDeltaX];
    float32 deltaY = [theEvent scrollingDeltaY];
    if ([theEvent hasPreciseScrollingDeltas] == YES)
    {
        // Touchpad or other precise device send integer values (-3, -1, 0, 1, 40, etc)
        deltaX /= scrollK;
        deltaY /= scrollK;
    }
    else
    {
        // Mouse sends float values from 0.1 for one wheel tick
        deltaX *= scrollK;
        deltaY *= scrollK;
    }
    eModifierKeys modifierKeys = GetModifierKeys(theEvent);
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseWheelEvent(window, x, y, deltaX, deltaY, modifierKeys, false));
}

void WindowNativeBridge::KeyEvent(NSEvent* theEvent)
{
    uint32 key = [theEvent keyCode];
    bool isRepeated = [theEvent isARepeat];
    bool isPressed = [theEvent type] == NSKeyDown;

    eModifierKeys modifierKeys = GetModifierKeys(theEvent);
    MainDispatcherEvent::eType type = isPressed ? MainDispatcherEvent::KEY_DOWN : MainDispatcherEvent::KEY_UP;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowKeyPressEvent(window, type, key, modifierKeys, isRepeated));

    // macOS translates some Ctrl key combinations into ASCII control characters.
    // It seems to me that control character are not wanted by game to handle in character message.
    if ([theEvent type] == NSKeyDown && (modifierKeys & eModifierKeys::CONTROL) == eModifierKeys::NONE)
    {
        NSString* chars = [theEvent characters];
        NSUInteger n = [chars length];
        if (n > 0)
        {
            MainDispatcherEvent e = MainDispatcherEvent::CreateWindowKeyPressEvent(window, MainDispatcherEvent::KEY_CHAR, 0, modifierKeys, false);
            for (NSUInteger i = 0; i < n; ++i)
            {
                uint32 key = [chars characterAtIndex:i];
                e.keyEvent.key = key;
                mainDispatcher->PostEvent(e);
            }
        }
    }
}

void WindowNativeBridge::FlagsChanged(NSEvent* theEvent)
{
    // Here we detect modifier key flags presses (Shift, Alt, Ctrl, Cmd, Capslock).
    // But Capslock is toggle key so we cannot determine it is pressed or unpressed
    // only is toggled and untoggled.

    static constexpr uint32 interestingFlags[] = {
        NX_DEVICELCTLKEYMASK,
        NX_DEVICERCTLKEYMASK,
        NX_DEVICELSHIFTKEYMASK,
        NX_DEVICERSHIFTKEYMASK,
        NX_DEVICELCMDKEYMASK,
        NX_DEVICERCMDKEYMASK,
        NX_DEVICELALTKEYMASK,
        NX_DEVICERALTKEYMASK,
        NX_ALPHASHIFTMASK, // Capslock
    };

    uint32 newModifierFlags = [theEvent modifierFlags];
    uint32 changedModifierFlags = newModifierFlags ^ lastModifierFlags;

    uint32 key = [theEvent keyCode];
    eModifierKeys modifierKeys = GetModifierKeys(theEvent);
    MainDispatcherEvent e = MainDispatcherEvent::CreateWindowKeyPressEvent(window, MainDispatcherEvent::KEY_DOWN, key, modifierKeys, false);
    for (uint32 flag : interestingFlags)
    {
        if (flag & changedModifierFlags)
        {
            bool isPressed = (flag & newModifierFlags) == flag;
            e.type = isPressed ? MainDispatcherEvent::KEY_DOWN : MainDispatcherEvent::KEY_UP;
            mainDispatcher->PostEvent(e);
        }
    }
    lastModifierFlags = newModifierFlags;
}

void WindowNativeBridge::MagnifyWithEvent(NSEvent* theEvent)
{
    eModifierKeys modifierKeys = GetModifierKeys(theEvent);
    float32 magnification = [theEvent magnification];
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMagnificationGestureEvent(window, magnification, modifierKeys));
}

void WindowNativeBridge::RotateWithEvent(NSEvent* theEvent)
{
    eModifierKeys modifierKeys = GetModifierKeys(theEvent);
    float32 rotation = [theEvent rotation];
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowRotationGestureEvent(window, rotation, modifierKeys));
}

void WindowNativeBridge::SwipeWithEvent(NSEvent* theEvent)
{
    eModifierKeys modifierKeys = GetModifierKeys(theEvent);
    float32 deltaX = [theEvent deltaX];
    float32 deltaY = [theEvent deltaY];
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSwipeGestureEvent(window, deltaX, deltaY, modifierKeys));
}

eModifierKeys WindowNativeBridge::GetModifierKeys(NSEvent* theEvent)
{
    // TODO: NSControlKeyMask, NSAlternateKeyMask, etc are deprecated in xcode 8 and replaced with NSEventModifierFlagControl, ...

    eModifierKeys result = eModifierKeys::NONE;
    NSEventModifierFlags flags = [theEvent modifierFlags];
    if (flags & NSShiftKeyMask)
    {
        result |= eModifierKeys::SHIFT;
    }
    if (flags & NSControlKeyMask)
    {
        result |= eModifierKeys::CONTROL;
    }
    if (flags & NSAlternateKeyMask)
    {
        result |= eModifierKeys::ALT;
    }
    if (flags & NSCommandKeyMask)
    {
        result |= eModifierKeys::COMMAND;
    }
    return result;
}

float32 WindowNativeBridge::GetDpi()
{
    NSScreen* screen = [NSScreen mainScreen];
    NSDictionary* description = [screen deviceDescription];
    NSSize displayPixelSize = [[description objectForKey:NSDeviceSize] sizeValue];
    CGSize displayPhysicalSize = CGDisplayScreenSize([[description objectForKey:@"NSScreenNumber"] unsignedIntValue]);

    // there being 25.4 mm in an inch
    return (displayPixelSize.width / displayPhysicalSize.width) * 25.4f;
}

eMouseButtons WindowNativeBridge::GetMouseButton(NSEvent* theEvent)
{
    eMouseButtons result = static_cast<eMouseButtons>([theEvent buttonNumber] + 1);
    if (eMouseButtons::FIRST <= result && result <= eMouseButtons::LAST)
    {
        return result;
    }
    return eMouseButtons::NONE;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__

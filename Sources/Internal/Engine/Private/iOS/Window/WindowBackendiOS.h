#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Functional/Function.h"

#include "Engine/Private/EnginePrivateFwd.h"
#include "Engine/Private/Dispatcher/UIDispatcher.h"

namespace DAVA
{
namespace Private
{
class WindowBackend final
{
public:
    WindowBackend(EngineBackend* e, Window* w);
    ~WindowBackend();

    void* GetHandle() const;
    MainDispatcher* GetDispatcher() const;
    Window* GetWindow() const;
    WindowNativeService* GetNativeService() const;

    bool Create(float32 width, float32 height);
    void Resize(float32 width, float32 height);
    void Close();

    void RunAsyncOnUIThread(const Function<void()>& task);

    void TriggerPlatformEvents();
    void ProcessPlatformEvents();

private:
    void EventHandler(const UIDispatcherEvent& e);

private:
    EngineBackend* engineBackend = nullptr;
    MainDispatcher* dispatcher = nullptr;
    Window* window = nullptr;

    UIDispatcher platformDispatcher;

    WindowNativeBridge* bridge = nullptr;
    std::unique_ptr<WindowNativeService> nativeService;

    size_t sigidAppBecomeOrResignActive = 0;
    size_t sigidAppDidEnterForegroundOrBackground = 0;

    // Friends
    friend class PlatformCore;
    friend struct WindowNativeBridge;
};

inline MainDispatcher* WindowBackend::GetDispatcher() const
{
    return dispatcher;
}

inline Window* WindowBackend::GetWindow() const
{
    return window;
}

inline WindowNativeService* WindowBackend::GetNativeService() const
{
    return nativeService.get();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__

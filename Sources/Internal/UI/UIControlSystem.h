/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_UI_CONTROL_SYSTEM_H__
#define __DAVAENGINE_UI_CONTROL_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/Singleton.h"
#include "UI/UIControl.h"
#include "UI/UIEvent.h"
#include "UI/UIScreenTransition.h"
#include "UI/UILoadingTransition.h"
#include "UI/UIPopup.h"
#include "Base/FastName.h"

#include "UI/Systems/UISystem.h"

#define FRAME_SKIP	5

/**
	\defgroup controlsystem	UI System
*/
namespace DAVA
{

class UIScreen;

class ScreenSwitchListener
{
public:
	virtual void OnScreenWillSwitch(UIScreen* newScreen) {}
    virtual void OnScreenDidSwitch(UIScreen* newScreen) {}
};

	/**
	 \brief	UIControlSystem it's a core of the all controls work.
		ControlSystem managed all update, draw, appearance and disappearance of the controls.
		ControlSystem works with th UIScreenManager to process screen setting and switching.
		Also ControlSystem processed all user input events to the controls.
	 */

extern const FastName FRAME_QUERY_UI_DRAW;

class UIControlSystem : public Singleton<UIControlSystem>
{
	friend void Core::CreateSingletons();
	
	int frameSkip;
	int transitionType;

protected:
	~UIControlSystem();
	/**
	 \brief Don't call this constructor!
	 */
	UIControlSystem();
			
public:
    /* 
       Player + 6 ally bots. All visible on the screen
    Old measures:
    UIControlSystem::inputs: 309
    UIControlSystem::updates: 310
    UIControlSystem::draws: 310

    New measures:
    UIControlSystem::inputs: 42
    */
    int32 updateCounter;
    int32 drawCounter;

	/**
	 \brief Sets the requested screen as current.
		Screen will be seted only on the next frame.
		Previous seted screen will be removed.
	 \param[in] Screen you want to set as current
	 \param[in] Transition you want to use for the screen setting.
	 */
	void SetScreen(UIScreen *newMainControl, UIScreenTransition * transition = 0);

	/**
	 \brief Sets the requested screen as current.
	 \returns currently seted screen
	 */
	UIScreen *GetScreen();

	/**
	 \brief Adds new popup to the popup container.
	 \param[in] Popup control to add.
	 */
	void AddPopup(UIPopup *newPopup);

	/**
	 \brief Removes popup from the popup container.
	 \param[in] Popup control to remove.
	 */
	void RemovePopup(UIPopup *newPopup);

	/**
	 \brief Removes all popups from the popup container.
	 */
	void RemoveAllPopups();
	
	/**
	 \brief Returns popups container.
		User can manage this container manually (change popup sequence, removes or adds popups)
	 \returns popup container
	 */
	UIControl *GetPopupContainer();
	
	/**
	 \brief Sets the current screen to 0 LOL.
	 */
	void Reset();

	/**
	 \brief Callse very frame by the system for update.
	 */
	void Update();

	/**
	 \brief Calls every frame by the system for draw.
		Draws all controls hierarchy to the screen.
	 */
	void Draw();
	
//	void SetTransitionType(int newTransitionType);
	
	/**
	 \brief Returns base geometric data seted in the system.
		Base GeometricData is usually has parameters looks a like:
		baseGeometricData.position = Vector2(0, 0);
		baseGeometricData.size = Vector2(0, 0);
		baseGeometricData.pivotPoint = Vector2(0, 0);
		baseGeometricData.scale = Vector2(1.0f, 1.0f);
		baseGeometricData.angle = 0;
		But system can change this parameters for the 
		specific device capabilities.
	 
	 \returns GeometricData used for the base draw
	 */
	const UIGeometricData &GetBaseGeometricData() const;
	
	/**
	 \brief Used internally by Replay class
	 */
	void ReplayEvents();

	/**
	 \brief Called by the core when screen size is changed
	 */
    void ScreenSizeChanged();


	/**
	 \brief Called by the control to set himself as the hovered control
	 */
    void SetHoveredControl(UIControl *newHovered);

	/**
	 \brief Returns control hovered by the mouse for now
	 */
    UIControl *GetHoveredControl(UIControl *newHovered);

    /**
	 \brief Called by the control to set himself as the focused control
	 */
    void SetFocusedControl(UIControl *newFocused, bool forceSet);
    
	/**
	 \brief Returns currently focused control
	 */
    UIControl *GetFocusedControl();
	
	void AddScreenSwitchListener(ScreenSwitchListener * listener);
	void RemoveScreenSwitchListener(ScreenSwitchListener * listener);

	/**
	 \brief Disallow screen switch.
	 Locking screen switch or incrementing lock counter.
	 \returns current screen switch lock counter
	 */
	int32 LockSwitch();

	/**
	 \brief Allow screen switch.
	 Decrementing lock counter if counter is zero unlocking screen switch.
	 \returns current screen switch lock counter
	 */
	int32 UnlockSwitch();

    void UI3DViewAdded();
    void UI3DViewRemoved();

    inline UISystem* GetSystem(const UISystem::eSystemType system) const;
    template<class T>
    inline T* GetSystem() const;

private:
	/**
	 \brief Instantly replace one screen to another.
		Call this only on your own risk if you are really know what you need. 
		May cause to abnormal behavior!
		Internally used by UITransition.
	 \param[in] Screen you want to set as current.
	 */
	void ReplaceScreen(UIScreen *newMainControl);

	void ProcessScreenLogic();

    void NotifyListenersWillSwitch( UIScreen* screen );
    void NotifyListenersDidSwitch( UIScreen* screen );

	Vector<ScreenSwitchListener*> screenSwitchListeners;

	UIScreen * currentScreen;
	UIScreen * nextScreen;
	UIScreen * prevScreen;

	int32 screenLockCount;

	bool removeCurrentScreen;
	
	UIControl *hovered;
    
    UIControl *focusedControl;

	UIControl * popupContainer;
	Set<UIPopup*> popupsToRemove;
	
	UIScreenTransition * nextScreenTransition;
	
	UIGeometricData baseGeometricData;

    int32 ui3DViewCount;
	
	friend class UIScreenTransition;
	friend class UIScreenManager;

    Vector<UISystem*> systems;

    friend class UIRenderSystem;
    friend class UIInputSystem;
};

inline UISystem* UIControlSystem::GetSystem(const UISystem::eSystemType system) const
{
    return systems[(uint32)system];
}

template<class T>
inline T* UIControlSystem::GetSystem() const
{
    return reinterpret_cast<T*>(GetSystem((const UISystem::eSystemType)T::TYPE));
}

};

#endif

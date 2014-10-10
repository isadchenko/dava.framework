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


#ifndef __DAVAENGINE_SCENE3D_SOUNDUPDATESYSTEM_H__
#define	__DAVAENGINE_SCENE3D_SOUNDUPDATESYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{

class SoundEvent;
class SoundComponent;
class SoundUpdateSystem : public SceneSystem
{
    struct AutoTriggerSound
    {
        AutoTriggerSound(Entity * owner, SoundEvent * sound);

        Entity * owner;
        SoundEvent * soundEvent;
        float32 maxSqDistance;
    };

public:
    SoundUpdateSystem(Scene * scene);
    virtual ~SoundUpdateSystem();

    virtual void ImmediateEvent(Entity * entity, uint32 event);
    virtual void Process(float32 timeElapsed);
    virtual void AddEntity(Entity * entity);
    virtual void RemoveEntity(Entity * entity);

protected:
    void AddAutoTriggerSound(Entity * soundOwner, SoundEvent * sound);
    void RemoveAutoTriggerSound(Entity * soundOwner, SoundEvent * sound = 0);

    Vector<AutoTriggerSound> autoTriggerSounds;

    friend class SoundComponent;
};
    
} // ns

#endif	/* __DAVAENGINE_SCENE3D_SOUNDUPDATESYSTEM_H__ */


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


#include "ControlNode.h"

#include "UI/UIControl.h"
#include "Base/ObjectFactory.h"

#include "PackageNode.h"
#include "../PackageSerializer.h"
#include "../ControlProperties/RootProperty.h"

#include "ControlPrototype.h"

using namespace DAVA;

ControlNode::ControlNode(UIControl *control)
    : ControlsContainerNode(nullptr)
    , control(SafeRetain(control))
    , rootProperty(nullptr)
    , prototype(nullptr)
    , creationType(CREATED_FROM_CLASS)
{
    rootProperty = new RootProperty(this, nullptr, AbstractProperty::CT_COPY);
}

ControlNode::ControlNode(ControlNode *node)
    : ControlsContainerNode(nullptr)
    , control(nullptr)
    , rootProperty(nullptr)
    , prototype(SafeRetain(node->prototype))
    , creationType(node->creationType)
{
    control = ObjectFactory::Instance()->New<UIControl>(node->control->GetClassName());
    
    rootProperty = new RootProperty(this, node->rootProperty, RootProperty::CT_COPY);
    
    for (ControlNode *sourceChild : nodes)
    {
        RefPtr<ControlNode> childNode(sourceChild->Clone());
        Add(childNode.Get());
    }
}

ControlNode::ControlNode(ControlPrototype *_prototype, eCreationType _creationType)
    : ControlsContainerNode(nullptr)
    , control(nullptr)
    , rootProperty(nullptr)
    , prototype(SafeRetain(_prototype))
    , creationType(_creationType)
{
    control = ObjectFactory::Instance()->New<UIControl>(prototype->GetControlNode()->GetControl()->GetClassName());

    rootProperty = new RootProperty(this, prototype->GetControlNode()->GetRootProperty(), RootProperty::CT_INHERIT);
    
    prototype->GetControlNode()->AddControlToInstances(this);

    for (ControlNode *sourceChild : prototype->GetControlNode()->nodes)
    {
        ScopedPtr<ControlPrototype> childPrototype(new ControlPrototype(sourceChild, prototype->GetPackageRef()));
        ScopedPtr<ControlNode> childNode(new ControlNode(childPrototype, CREATED_FROM_PROTOTYPE_CHILD));
        Add(childNode);
    }
}

ControlNode::~ControlNode()
{
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
        (*it)->Release();
    nodes.clear();
    
    SafeRelease(control);
    SafeRelease(rootProperty);

    if (prototype)
        prototype->GetControlNode()->RemoveControlFromInstances(this);
    SafeRelease(prototype);
    
    DVASSERT(instances.empty());
}

ControlNode *ControlNode::CreateFromControl(DAVA::UIControl *control)
{
    return new ControlNode(control);
}

ControlNode *ControlNode::CreateFromPrototype(ControlNode *sourceNode, PackageRef *nodePackage)
{
    ScopedPtr<ControlPrototype> prototype(new ControlPrototype(sourceNode, nodePackage));
    return new ControlNode(prototype, CREATED_FROM_PROTOTYPE);
}

ControlNode *ControlNode::CreateFromPrototypeChild(ControlNode *sourceNode, PackageRef *nodePackage)
{
    ScopedPtr<ControlPrototype> prototype(new ControlPrototype(sourceNode, nodePackage));
    return new ControlNode(prototype, CREATED_FROM_PROTOTYPE_CHILD);
}

ControlNode *ControlNode::Clone()
{
    return new ControlNode(this);
}

void ControlNode::Add(ControlNode *node)
{
    DVASSERT(node->GetParent() == nullptr);
    node->SetParent(this);
    nodes.push_back(SafeRetain(node));
    control->AddControl(node->GetControl());
    node->GetControl()->UpdateLayout();
}

void ControlNode::InsertAtIndex(int index, ControlNode *node)
{
    if (index >= nodes.size())
    {
        Add(node);
    }
    else
    {
        DVASSERT(node->GetParent() == nullptr);
        node->SetParent(this);
        
        UIControl *belowThis = nodes[index]->GetControl();
        
        nodes.insert(nodes.begin() + index, SafeRetain(node));
        control->InsertChildBelow(node->GetControl(), belowThis);
        node->GetControl()->UpdateLayout();
    }
}

void ControlNode::Remove(ControlNode *node)
{
    auto it = find(nodes.begin(), nodes.end(), node);
    if (it != nodes.end())
    {
        DVASSERT(node->GetParent() == this);
        node->SetParent(nullptr);

        node->GetControl()->RemoveFromParent();
        nodes.erase(it);
        SafeRelease(node);
    }
    else
    {
        DVASSERT(false);
    }
}

int ControlNode::GetCount() const
{
    return (int) nodes.size();
}

ControlNode *ControlNode::Get(int index) const
{
    return nodes[index];
}

ControlNode *ControlNode::FindByName(const DAVA::String &name) const
{
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
    {
        if ((*it)->GetName() == name)
            return *it;
    }
    return nullptr;
}

String ControlNode::GetName() const
{
    return control->GetName();
}

UIControl *ControlNode::GetControl() const
{
    return control;
}

ControlPrototype *ControlNode::GetPrototype() const
{
    return prototype;
}

const Vector<ControlNode*> &ControlNode::GetInstances() const
{
    return instances;
}

int ControlNode::GetFlags() const
{
    int flag = 0;
    switch (creationType) {
        case CREATED_FROM_CLASS:
            flag |= FLAG_CONTROL_CREATED_FROM_CLASS;
            break;
        case CREATED_FROM_PROTOTYPE:
            flag |= FLAG_CONTROL_CREATED_FROM_PROTOTYPE;
            break;
        case CREATED_FROM_PROTOTYPE_CHILD:
            flag |= FLAG_CONTROL_CREATED_FROM_PROTOTYPE_CHILD;
            break;
            
        default:
            DVASSERT(false);
            break;
    }
    return IsReadOnly() ? (FLAG_READ_ONLY | flag) : flag ;
}

bool ControlNode::IsEditingSupported() const
{
    return !IsReadOnly();
}

bool ControlNode::IsInsertingSupported() const
{
    return !IsReadOnly();
}

bool ControlNode::CanInsertControl(ControlNode *node, DAVA::int32 pos) const
{
    if (IsReadOnly())
        return false;
    
    if (pos < nodes.size() && nodes[pos]->GetCreationType() == CREATED_FROM_PROTOTYPE_CHILD)
        return false;
    
    if (node && node->IsInstancedFrom(this))
        return false;
    
    return true;
}

bool ControlNode::CanRemove() const
{
    return !IsReadOnly() && creationType != CREATED_FROM_PROTOTYPE_CHILD;
}

bool ControlNode::CanCopy() const
{
    return creationType != CREATED_FROM_PROTOTYPE_CHILD;
}

void ControlNode::RefreshProperties()
{
    rootProperty->Refresh();
    for (ControlNode *node : nodes)
        node->RefreshProperties();
}

void ControlNode::MarkAsRemoved()
{
    if (prototype)
        prototype->GetControlNode()->RemoveControlFromInstances(this);
}

void ControlNode::MarkAsAlive()
{
    if (prototype)
        prototype->GetControlNode()->AddControlToInstances(this);
}

void ControlNode::Serialize(PackageSerializer *serializer) const
{
    serializer->BeginMap();
    
    rootProperty->Serialize(serializer);
    
    if (!nodes.empty())
    {
        bool shouldProcessChildren = true;
        Vector<ControlNode*> prototypeChildrenWithChanges;

        if (creationType == CREATED_FROM_PROTOTYPE)
        {
            CollectPrototypeChildrenWithChanges(prototypeChildrenWithChanges);
            shouldProcessChildren = !prototypeChildrenWithChanges.empty() || HasNonPrototypeChildren();
        }
        
        if (shouldProcessChildren)
        {
            serializer->BeginArray("children");

            for (const auto &child : prototypeChildrenWithChanges)
                child->Serialize(serializer);

            for (const auto &child : nodes)
            {
                if (child->GetCreationType() != CREATED_FROM_PROTOTYPE_CHILD)
                    child->Serialize(serializer);
            }
            
            serializer->EndArray();
        }
    }
    
    serializer->EndMap();
}

String ControlNode::GetPathToPrototypeChild(bool withRootPrototypeName) const
{
    if (creationType == CREATED_FROM_PROTOTYPE_CHILD)
    {
        String path = GetName();
        PackageBaseNode *p = GetParent();
        while (p != nullptr && p->GetControl() != nullptr && static_cast<ControlNode*>(p)->GetCreationType() != CREATED_FROM_PROTOTYPE)
        {
            path = p->GetName() + "/" + path;
            p = p->GetParent();
        }
        
        if (withRootPrototypeName && p != nullptr && p->GetControl() != nullptr && static_cast<ControlNode*>(p)->GetCreationType() == CREATED_FROM_PROTOTYPE)
        {
            ControlNode *c = static_cast<ControlNode*>(p);
            if (c->GetPrototype())
                path = c->GetPrototype()->GetName(true) + "/" + path;
        }
        
        return path;
    }
    return "";
}

void ControlNode::CollectPrototypeChildrenWithChanges(Vector<ControlNode*> &out) const
{
    for (auto child : nodes)
    {
        if (child->GetCreationType() == CREATED_FROM_PROTOTYPE_CHILD)
        {
            if (child->HasNonPrototypeChildren() || child->rootProperty->HasChanges())
                out.push_back(child);
            
            child->CollectPrototypeChildrenWithChanges(out);
        }
    }
}

bool ControlNode::HasNonPrototypeChildren() const
{
    for (const auto &child : nodes)
    {
        if (child->GetCreationType() != CREATED_FROM_PROTOTYPE_CHILD)
            return true;
    }
    return false;
}

bool ControlNode::IsInstancedFrom(const ControlNode *prototypeControl) const
{
    const ControlNode *test = this;
    
    while (test)
    {
        ControlPrototype *prototype = test->GetPrototype();
        if (prototype != nullptr)
        {
            if (prototype->GetControlNode() == prototypeControl)
                return true;
            test = prototype->GetControlNode();
        }
        else
        {
            test = nullptr;
        }
    }
    
    for (const ControlNode *child : nodes)
    {
        if (child->IsInstancedFrom(prototypeControl))
            return true;
    }
    
    return false;
}

void ControlNode::AddControlToInstances(ControlNode *control)
{
    auto it = std::find(instances.begin(), instances.end(), control);
    if (it == instances.end())
        instances.push_back(control);
}

void ControlNode::RemoveControlFromInstances(ControlNode *control)
{
    auto it = std::find(instances.begin(), instances.end(), control);
    if (it != instances.end())
        instances.erase(it);
}

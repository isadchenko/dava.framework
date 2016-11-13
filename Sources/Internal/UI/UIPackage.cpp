#include "UIPackage.h"

#include "UI/UIControl.h"
#include "UI/UIControlPackageContext.h"

namespace DAVA
{
UIPackage::UIPackage()
    :
    controlPackageContext(new UIControlPackageContext())
{
}

UIPackage::~UIPackage()
{
    for (UIControl* control : controls)
        control->Release();
    controls.clear();

    for (UIControl* prototype : prototypes)
        prototype->Release();
    prototypes.clear();

    SafeRelease(controlPackageContext);
}

const Vector<UIControl*>& UIPackage::GetPrototypes() const
{
    return prototypes;
}

UIControl* UIPackage::GetPrototype(const String& name) const
{
    return GetPrototype(FastName(name));
}

UIControl* UIPackage::GetPrototype(const FastName& name) const
{
    for (UIControl* prototype : prototypes)
    {
        if (prototype->GetName() == name)
            return prototype;
    }

    return nullptr;
}

void UIPackage::AddPrototype(UIControl* control)
{
    control->SetPackageContext(controlPackageContext);
    prototypes.push_back(SafeRetain(control));
}

void UIPackage::RemovePrototype(UIControl* control)
{
    Vector<UIControl*>::iterator iter = std::find(prototypes.begin(), prototypes.end(), control);
    if (iter != prototypes.end())
    {
        SafeRelease(*iter);
        prototypes.erase(iter);
    }
}

const Vector<UIControl*>& UIPackage::GetControls() const
{
    return controls;
}

int32 UIPackage::GetControlsCount() const
{
    return static_cast<int32>(controls.size());
}

UIControl* UIPackage::GetControl(const String& name) const
{
    return GetControl(FastName(name));
}

UIControl* UIPackage::GetControl(const FastName& name) const
{
    for (UIControl* control : controls)
    {
        if (control->GetName() == name)
            return control;
    }

    return nullptr;
}

UIControl* UIPackage::GetControl(int32 index) const
{
    DVASSERT(0 <= index && index < static_cast<int32>(controls.size()));
    return controls[index];
}

void UIPackage::AddControl(UIControl* control)
{
    control->SetPackageContext(controlPackageContext);
    controls.push_back(SafeRetain(control));
}

void UIPackage::RemoveControl(UIControl* control)
{
    Vector<UIControl*>::iterator iter = std::find(controls.begin(), controls.end(), control);
    if (iter != controls.end())
    {
        SafeRelease(*iter);
        controls.erase(iter);
    }
}

UIControlPackageContext* UIPackage::GetControlPackageContext()
{
    return controlPackageContext;
}

RefPtr<UIPackage> UIPackage::Clone() const
{
    RefPtr<UIPackage> package(new UIPackage());

    package->controls.resize(controls.size());

    std::transform(controls.begin(), controls.end(), package->controls.begin(),
                   [](UIControl* control) -> UIControl*
                   {
                       return control->Clone();
                   });
    return package;
}

}

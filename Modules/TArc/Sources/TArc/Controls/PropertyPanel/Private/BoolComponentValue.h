#pragma once

#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"

namespace DAVA
{
namespace TArc
{
class BoolComponentValue : public BaseComponentValue
{
public:
    QQmlComponent* GetComponent(QQmlEngine* engine) const override;

private:
    int GetValue() const;
    void SetValue(int v);

    bool IsReadOnly() const;

private:
    DAVA_VIRTUAL_REFLECTION(BoolComponentValue, BaseComponentValue);
};
}
}
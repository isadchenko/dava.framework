#ifndef __UI_EDITOR_SUB_VALUE_PROPERTY__
#define __UI_EDITOR_SUB_VALUE_PROPERTY__

#include "AbstractProperty.h"

class ValueProperty;

class SubValueProperty;

class SubValueProperty : public AbstractProperty
{
public:
    SubValueProperty(int index, const DAVA::String &propName);

protected:
    virtual ~SubValueProperty();

public:
    virtual int GetCount() const override;
    virtual AbstractProperty *GetProperty(int index) const override;
    virtual void Serialize(PackageSerializer *serializer) const override {};
    virtual const DAVA::String &GetName() const;
    virtual ePropertyType GetType() const;
    virtual DAVA::VariantType GetValue() const;
    virtual void SetValue(const DAVA::VariantType &newValue);
    virtual DAVA::VariantType GetDefaultValue() const;
    virtual void SetDefaultValue(const DAVA::VariantType &newValue) override;
    virtual void ResetValue() override;
    virtual bool IsReplaced() const;

private:
    ValueProperty *GetValueProperty() const;
    int index;
    DAVA::String name;
};

#endif // __UI_EDITOR_SUB_VALUE_PROPERTY__

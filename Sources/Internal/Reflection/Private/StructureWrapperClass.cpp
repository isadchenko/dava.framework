#include "Reflection/Private/StructureWrapperClass.h"

namespace DAVA
{
StructureWrapperClass::StructureWrapperClass(const RttiType* rttiType)
{
    FillCache(rttiType);
}

void StructureWrapperClass::FillCache(const RttiType* rttiType)
{
    FillCacheEntries(rttiType);

    const RttiInheritance* inheritance = rttiType->GetInheritance();
    if (NULL != inheritance)
    {
        const Vector<RttiInheritance::Info>& baseTypesInfo = inheritance->GetBaseTypes();
        for (auto& baseInfo : baseTypesInfo)
        {
            FillCache(baseInfo.type);
        }
    }
}

void StructureWrapperClass::FillCacheEntries(const RttiType* rttiType)
{
    const ReflectedType* reflectedType = ReflectedTypeDB::GetByRttiType(rttiType);
    const ReflectedStructure* structure = reflectedType->GetStrucutre();

    for (auto& f : structure->fields)
    {
        const ReflectedStructure::Field* field = f.get();
        fieldsCache.push_back(field);
        fieldsNameIndexes[field->name] = fieldsCache.size() - 1;
    }

    for (auto& m : structure->methods)
    {
        const ReflectedStructure::Method* method = m.get();
        methodsCache.push_back(method);
        methodsNameIndexes[method->name] = methodsCache.size() - 1;
    }
}

bool StructureWrapperClass::HasFields(const ReflectedObject& object, const ValueWrapper* vw) const
{
    return !fieldsCache.empty();
}

Reflection StructureWrapperClass::GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
{
    if (!fieldsCache.empty())
    {
        String name = key.Cast<String>(String());

        if (name.empty())
        {
            name = key.Cast<const char*>("");
        }

        if (!name.empty())
        {
            auto it = fieldsNameIndexes.find(name);
            if (it != fieldsNameIndexes.end())
            {
                const ReflectedStructure::Field* field = fieldsCache[it->second];
                return Reflection(vw->GetValueObject(object), field->reflectedType, field->meta.get(), field->valueWrapper.get());
            }
        }
    }

    return Reflection();
}

Vector<Reflection::Field> StructureWrapperClass::GetFields(const ReflectedObject& object, const ValueWrapper* vw) const
{
    Vector<Reflection::Field> ret;

    ret.reserve(fieldsCache.size());
    for (auto field : fieldsCache)
    {
        ret.push_back({ DAVA::Any(field->name), Reflection(vw->GetValueObject(object), field->reflectedType, field->meta.get(), field->valueWrapper.get()) });
    }

    return ret;
}

bool StructureWrapperClass::HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const
{
    return !methodsCache.empty();
}

AnyFn StructureWrapperClass::GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
{
    String name = key.Cast<String>(String());
    if (!name.empty())
    {
        void* methodThis = vw->GetValueObject(object).GetVoidPtr();

        auto it = methodsNameIndexes.find(name);
        if (it != methodsNameIndexes.end())
        {
            return methodsCache[it->second]->methodWrapper->anyFn.BindThis(methodThis);
        }
    }
    return AnyFn();
}

Vector<Reflection::Method> StructureWrapperClass::GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const
{
    Vector<Reflection::Method> ret;

    void* methodThis = vw->GetValueObject(object).GetVoidPtr();

    ret.reserve(methodsCache.size());
    for (auto m : methodsCache)
    {
        ret.push_back({ m->name, m->methodWrapper->anyFn.BindThis(methodThis) });
    }

    return ret;
}

} //namespace DAVA

/*
#include "Base/Platform.h"

#include "Reflection/Private/StructureWrapperClass.h"
#include "Reflection/ReflectedType.h"
#include "Reflection/ReflectedObject.h"

namespace DAVA
{
StructureWrapperClass::StructureWrapperClass(const RttiType* type)
    : thisType(type)
{
}

bool StructureWrapperClass::HasFields(const ReflectedObject& object, const ValueWrapper* vw) const
{
    bool ret = !fields.empty();

    if (!ret)
    {
        InitBaseClasses();

        for (ClassBase& base : bases)
        {
            const StructureWrapper* baseSW = base.refType->structureWrapper.get();
            ret |= baseSW->HasFields(base.GetBaseObject(object), vw);

            if (ret)
            {
                break;
            }
        }
    }

    return ret;
}

Reflection StructureWrapperClass::GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
{
    Reflection ret;

    String name;

    if (key.CanGet<String>())
    {
        name = key.Get<String>();
    }
    else if (key.CanGet<const char*>())
    {
        name = key.Get<const char*>();
    }

    if (!name.empty())
    {
        auto end = fields.end();
        auto it = fields.begin();

        for (; it != end; ++it)
        {
            if (it->first == name)
            {
                ret = Reflection(vw->GetValueObject(object), it->second.type, it->second.meta.get(), it->second.vw.get());
                break;
            }
        }

        if (it == end)
        {
            InitBaseClasses();

            for (ClassBase& base : bases)
            {
                if (nullptr != base.refType)
                {
                    const StructureWrapper* baseSW = base.refType->structureWrapper.get();

                    ret = baseSW->GetField(base.GetBaseObject(object), vw, key);
                    if (ret.IsValid())
                    {
                        break;
                    }
                }
            }
        }
    }

    return ret;
}

Vector<Reflection::Field> StructureWrapperClass::GetFields(const ReflectedObject& object, const ValueWrapper* vw) const
{
    Vector<Reflection::Field> ret;

    InitBaseClasses();

    for (auto& baseIt : bases)
    {
        const ReflectedType* baseRefType = baseIt.refType;
        if (nullptr != baseRefType)
        {
            const StructureWrapper* baseSW = baseRefType->structureWrapper.get();
            auto baseFields = baseSW->GetFields(baseIt.GetBaseObject(object), vw);

            std::move(baseFields.begin(), baseFields.end(), std::inserter(ret, ret.end()));
        }
    }

    for (auto& it : fields)
    {
        Reflection::Field rf;

        rf.key = it.first;
        rf.ref = Reflection(vw->GetValueObject(object), it.second.type, it.second.meta.get(), it.second.vw.get());

        ret.emplace_back(std::move(rf));
    }

    return ret;
}

bool StructureWrapperClass::HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const
{
    bool ret = !methods.empty();

    if (!ret)
    {
        InitBaseClasses();

        for (ClassBase& base : bases)
        {
            const StructureWrapper* baseSW = base.refType->structureWrapper.get();
            ret |= baseSW->HasMethods(base.GetBaseObject(object), vw);

            if (ret)
            {
                break;
            }
        }
    }

    return ret;
}

AnyFn StructureWrapperClass::GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
{
    AnyFn ret;

    if (key.CanCast<String>())
    {
        const String& name = key.Cast<String>();

        auto end = methods.end();
        auto it = methods.begin();

        for (; it != end; ++it)
        {
            if (it->first == name)
            {
                ret = it->second.BindThis(object.GetVoidPtr());
                break;
            }
        }

        if (it == end)
        {
            InitBaseClasses();

            for (ClassBase& base : bases)
            {
                if (nullptr != base.refType)
                {
                    const StructureWrapper* baseSW = base.refType->structureWrapper.get();

                    ret = baseSW->GetMethod(base.GetBaseObject(object), vw, key);
                    if (ret.IsValid())
                    {
                        break;
                    }
                }
            }
        }
    }

    return ret;
}

Vector<Reflection::Method> StructureWrapperClass::GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const
{
    Vector<Reflection::Method> ret;

    InitBaseClasses();

    for (auto& baseIt : bases)
    {
        const ReflectedType* baseRefType = baseIt.refType;
        if (nullptr != baseRefType)
        {
            const StructureWrapper* baseSW = baseRefType->structureWrapper.get();
            auto baseMethods = baseSW->GetMethods(baseIt.GetBaseObject(object), vw);

            std::move(baseMethods.begin(), baseMethods.end(), std::inserter(ret, ret.end()));
        }
    }

    for (auto& it : methods)
    {
        ret.emplace_back(Reflection::Method{ it.first, it.second.BindThis(object.GetVoidPtr()) });
    }

    return ret;
}

void StructureWrapperClass::InitBaseClasses() const
{
    if (!basesInitialized)
    {
        basesInitialized = true;

        const RttiInheritance* ti = thisType->GetInheritance();
        if (nullptr != ti)
        {
            auto bases_ = ti->GetBaseTypes();

            bases.reserve(bases_.size());
            for (auto& bc : bases_)
            {
                ClassBase classBase;
                classBase.type = bc.first;
                classBase.castToBaseOP = bc.second;
                classBase.refType = ReflectedType::GetByRttiType(bc.first);

                bases.emplace_back(std::move(classBase));
            }
        }
    }
}

ReflectedObject StructureWrapperClass::ClassBase::GetBaseObject(const ReflectedObject& object) const
{
    void* derVoidPtr = object.GetVoidPtr();
    void* baseVoidPtr = (*castToBaseOP)(derVoidPtr);

    return ReflectedObject(baseVoidPtr, type->Pointer());
}

} // namespace DAVA
*/
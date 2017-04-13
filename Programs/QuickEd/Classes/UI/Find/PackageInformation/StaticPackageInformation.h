#pragma once

#include <Base/BaseTypes.h>
#include "UI/Find/PackageInformation/PackageInformation.h"
#include "UI/Find/PackageInformation/StaticControlInformation.h"

class StaticPackageInformation
: public PackageInformation
{
public:
    StaticPackageInformation(const DAVA::String& path);

    DAVA::String GetPath() const override;

    void VisitImportedPackages(const DAVA::Function<void(const PackageInformation*)>& visitor) const override;
    void VisitControls(const DAVA::Function<void(const ControlInformation*)>& visitor) const override;
    void VisitPrototypes(const DAVA::Function<void(const ControlInformation*)>& visitor) const override;

    void AddImportedPackage(const std::shared_ptr<StaticPackageInformation>& package);
    void AddControl(const std::shared_ptr<StaticControlInformation>& control);
    void AddPrototype(const std::shared_ptr<StaticControlInformation>& prototype);

    const DAVA::Vector<std::shared_ptr<StaticPackageInformation>>& GetImportedPackages() const;
    const DAVA::Vector<std::shared_ptr<StaticControlInformation>>& GetPrototypes() const;
    const DAVA::Vector<std::shared_ptr<StaticControlInformation>>& GetControls() const;
    std::shared_ptr<StaticControlInformation> FindPrototypeByName(const DAVA::FastName& name) const;

private:
    DAVA::String path;
    DAVA::Vector<std::shared_ptr<StaticPackageInformation>> importedPackages;
    DAVA::Vector<std::shared_ptr<StaticControlInformation>> controls;
    DAVA::Vector<std::shared_ptr<StaticControlInformation>> prototypes;
};

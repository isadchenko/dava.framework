#include "QECommands/ChangeStylePropertyCommand.h"
#include "QECommands/QECommandIDs.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/ControlProperties/AbstractProperty.h"

ChangeStylePropertyCommand::ChangeStylePropertyCommand(PackageNode* package, StyleSheetNode* node_, AbstractProperty* property_, const DAVA::VariantType& newValue_)
    : QEPackageCommand(package, CHANGE_STYLE_PROPERTY_COMMAND, DAVA::String("change ") + property_->GetName().c_str())
    , node(SafeRetain(node_))
    , property(SafeRetain(property_))
    , newValue(newValue_)
{
    oldValue = property->GetValue();
}

ChangeStylePropertyCommand::~ChangeStylePropertyCommand()
{
    SafeRelease(node);
    SafeRelease(property);
}

void ChangeStylePropertyCommand::Redo()
{
    package->SetStyleProperty(node, property, newValue);
}

void ChangeStylePropertyCommand::Undo()
{
    package->SetStyleProperty(node, property, oldValue);
}

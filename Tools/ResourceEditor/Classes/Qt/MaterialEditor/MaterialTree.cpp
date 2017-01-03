#include "MaterialTree.h"
#include "MaterialFilterModel.h"
#include "Main/mainwindow.h"
#include "Scene/SceneSignals.h"
#include "MaterialEditor/MaterialAssignSystem.h"
#include "QtTools/WidgetHelpers/SharedIcon.h"

#include "Commands2/RemoveComponentCommand.h"
#include "Commands2/Base/RECommandBatch.h"
#include "Commands2/Base/RECommandNotificationObject.h"
#include "Entity/Component.h"

#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QHeaderView>

MaterialTree::MaterialTree(QWidget* parent /* = 0 */)
    : QTreeView(parent)
{
    treeModel = new MaterialFilteringModel(new MaterialModel(this));
    setModel(treeModel);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setIconSize(QSize(24, 24));

    QObject::connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenu(const QPoint&)));

    QObject::connect(SceneSignals::Instance(), &SceneSignals::CommandExecuted, this, &MaterialTree::OnCommandExecuted);
    QObject::connect(SceneSignals::Instance(), &SceneSignals::StructureChanged, this, &MaterialTree::OnStructureChanged);
    QObject::connect(SceneSignals::Instance(), &SceneSignals::SelectionChanged, this, &MaterialTree::OnSelectionChanged);

    header()->setSortIndicator(0, Qt::AscendingOrder);
    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    header()->setSectionResizeMode(1, QHeaderView::Fixed);
    header()->setSectionResizeMode(2, QHeaderView::Fixed);
    header()->resizeSection(1, 35);
    header()->resizeSection(2, 35);
}

MaterialTree::~MaterialTree()
{
}

void MaterialTree::SetScene(SceneEditor2* sceneEditor)
{
    setSortingEnabled(false);
    treeModel->SetScene(sceneEditor);

    if (nullptr != sceneEditor)
    {
        OnSelectionChanged(sceneEditor, &sceneEditor->selectionSystem->GetSelection(), nullptr);
    }
    else
    {
        treeModel->SetSelection(nullptr);
    }

    sortByColumn(0);
    setSortingEnabled(true);
}

DAVA::NMaterial* MaterialTree::GetMaterial(const QModelIndex& index) const
{
    return treeModel->GetMaterial(index);
}

void MaterialTree::SelectMaterial(DAVA::NMaterial* material)
{
    selectionModel()->clear();

    QModelIndex index = treeModel->GetIndex(material);
    if (index.isValid())
    {
        selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        scrollTo(index, QAbstractItemView::PositionAtCenter);
    }
}

void MaterialTree::SelectEntities(const QList<DAVA::NMaterial*>& materials)
{
    SceneEditor2* curScene = treeModel->GetScene();

    if (nullptr != curScene && materials.size() > 0)
    {
        std::function<void(DAVA::NMaterial*)> fn = [&fn, &curScene](DAVA::NMaterial* material) {
            DAVA::Entity* entity = curScene->materialSystem->GetEntity(material);
            if (nullptr != entity)
            {
                curScene->selectionSystem->AddObjectToSelection(curScene->selectionSystem->GetSelectableEntity(entity));
            }
            const DAVA::Vector<DAVA::NMaterial*>& children = material->GetChildren();
            for (auto child : children)
            {
                fn(child);
            }
        };

        curScene->selectionSystem->Clear();
        for (int i = 0; i < materials.size(); i++)
        {
            DAVA::NMaterial* material = materials.at(i);
            fn(material);
        }

        LookAtSelection(curScene);
    }
}

void MaterialTree::Update()
{
    treeModel->Sync();
    treeModel->invalidate();
    emit Updated();
}

int MaterialTree::getFilterType() const
{
    return treeModel->getFilterType();
}

void MaterialTree::setFilterType(int filterType)
{
    treeModel->setFilterType(filterType);
}

void MaterialTree::ShowContextMenu(const QPoint& pos)
{
    QMenu contextMenu(this);

    contextMenu.addAction(SharedIcon(":/QtIcons/zoom.png"), "Select entities", this, SLOT(OnSelectEntities()));

    emit ContextMenuPrepare(&contextMenu);
    contextMenu.exec(mapToGlobal(pos));
}

void MaterialTree::dragEnterEvent(QDragEnterEvent* event)
{
    QTreeView::dragEnterEvent(event);
    dragTryAccepted(event);
}

void MaterialTree::dragMoveEvent(QDragMoveEvent* event)
{
    QTreeView::dragMoveEvent(event);
    dragTryAccepted(event);
}

void MaterialTree::dropEvent(QDropEvent* event)
{
    QTreeView::dropEvent(event);

    event->setDropAction(Qt::IgnoreAction);
    event->accept();
}

void MaterialTree::dragTryAccepted(QDragMoveEvent* event)
{
    int row, col;
    QModelIndex parent;

    GetDropParams(event->pos(), parent, row, col);
    if (treeModel->dropCanBeAccepted(event->mimeData(), event->dropAction(), row, col, parent))
    {
        event->setDropAction(Qt::MoveAction);
        event->accept();
        treeModel->invalidate();
    }
    else
    {
        event->setDropAction(Qt::IgnoreAction);
        event->accept();
    }
}

void MaterialTree::GetDropParams(const QPoint& pos, QModelIndex& index, int& row, int& col)
{
    row = -1;
    col = -1;
    index = indexAt(pos);

    switch (dropIndicatorPosition())
    {
    case QAbstractItemView::OnItem:
    case QAbstractItemView::AboveItem:
        row = index.row();
        col = index.column();
        index = index.parent();
        break;
    case QAbstractItemView::BelowItem:
        row = index.row() + 1;
        col = index.column();
        index = index.parent();
        break;
    case QAbstractItemView::OnViewport:
        index = QModelIndex();
        break;
    }
}

void MaterialTree::OnCommandExecuted(SceneEditor2* scene, const RECommandNotificationObject& commandNotification)
{
    if (treeModel->GetScene() == scene)
    {
        if (commandNotification.MatchCommandIDs({ CMDID_INSP_MEMBER_MODIFY, CMDID_REFLECTED_FIELD_MODIFY }))
        {
            treeModel->invalidate();
        }
        else if (commandNotification.MatchCommandIDs({ CMDID_DELETE_RENDER_BATCH, CMDID_CLONE_LAST_BATCH, CMDID_CONVERT_TO_SHADOW, CMDID_MATERIAL_SWITCH_PARENT,
                                                       CMDID_MATERIAL_REMOVE_CONFIG, CMDID_MATERIAL_CREATE_CONFIG, CMDID_LOD_DELETE, CMDID_LOD_CREATE_PLANE, CMDID_LOD_COPY_LAST_LOD }))
        {
            Update();
        }
        else
        {
            auto processRemoveCommand = [this](const RECommand* command, bool redo)
            {
                if (command->MatchCommandID(CMDID_COMPONENT_REMOVE))
                {
                    const RemoveComponentCommand* removeCommand = static_cast<const RemoveComponentCommand*>(command);
                    DVASSERT(removeCommand->GetComponent() != nullptr);
                    if (removeCommand->GetComponent()->GetType() == DAVA::Component::RENDER_COMPONENT)
                    {
                        Update();
                    }
                }
            };

            commandNotification.ExecuteForAllCommands(processRemoveCommand);
        }
    }
}

void MaterialTree::OnStructureChanged(SceneEditor2* scene, DAVA::Entity* parent)
{
    treeModel->Sync();
}

void MaterialTree::OnSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected)
{
    if (treeModel->GetScene() == scene)
    {
        treeModel->SetSelection(selected);
        treeModel->invalidate();
    }
}

void MaterialTree::OnSelectEntities()
{
    const QModelIndexList selection = selectionModel()->selectedRows();
    QList<DAVA::NMaterial*> materials;

    materials.reserve(selection.size());
    for (int i = 0; i < selection.size(); i++)
    {
        DAVA::NMaterial* material = treeModel->GetMaterial(selection.at(i));
        if (material != nullptr)
            materials << material;
    }

    SelectEntities(materials);
}

#ifndef __ResourceEditorQt__LayerForceWidget__
#define __ResourceEditorQt__LayerForceWidget__

#include <DAVAEngine.h>

#include <QWidget>
#include "BaseParticleEditorContentWidget.h"

class TimeLineWidget;
class QVBoxLayout;

class LayerForceWidget : public BaseParticleEditorContentWidget
{
    Q_OBJECT

public:
    explicit LayerForceWidget(QWidget* parent = 0);
    ~LayerForceWidget();

    void Init(SceneEditor2* scene, DAVA::ParticleLayer* layer, DAVA::uint32 forceIndex, bool updateMinimized);
    DAVA::ParticleLayer* GetLayer() const
    {
        return layer;
    };
    DAVA::int32 GetForceIndex() const
    {
        return forceIndex;
    };

    void Update();

    virtual void StoreVisualState(DAVA::KeyedArchive* visualStateProps);
    virtual void RestoreVisualState(DAVA::KeyedArchive* visualStateProps);

signals:
    void ValueChanged();

protected slots:
    void OnValueChanged();

protected:
    void InitWidget(QWidget* widget);

private:
    QVBoxLayout* mainBox;
    DAVA::ParticleLayer* layer;
    DAVA::int32 forceIndex;

    TimeLineWidget* forceTimeLine;
    TimeLineWidget* forceOverLifeTimeLine;

    bool blockSignals;
};

#endif /* defined(__ResourceEditorQt__LayerForceWidget__) */

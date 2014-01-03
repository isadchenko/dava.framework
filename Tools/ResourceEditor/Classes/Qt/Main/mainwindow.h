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



#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include "ModificationWidget.h"
#include "Tools/QtWaitDialog/QtWaitDialog.h"

#include "DAVAEngine.h"

#include "Scene/SceneEditor2.h"
#include "Tools/QtPosSaver/QtPosSaver.h"

// TODO: remove old screen -->
#include "Classes/SceneEditor/MaterialEditor.h"
// <---

class AddSwitchEntityDialog;
class Request;
class QtLabelWithActions;
class LandscapeDialog;
class HangingObjectsHeight;
class QtMainWindow : public QMainWindow, public DAVA::Singleton<QtMainWindow>
{
	Q_OBJECT

protected:
    
    static const int GLOBAL_INVALIDATE_TIMER_DELTA = 1000;

public:
	explicit QtMainWindow(QWidget *parent = 0);
	~QtMainWindow();

	Ui::MainWindow* GetUI();
	SceneTabWidget* GetSceneWidget();
	SceneEditor2* GetCurrentScene();

    bool OpenScene(const QString & path);
	bool SaveScene(SceneEditor2 *scene);
	bool SaveSceneAs(SceneEditor2 *scene);

	void SetGPUFormat(DAVA::eGPUFamily gpu);
	DAVA::eGPUFamily GetGPUFormat();

	void WaitStart(const QString &title, const QString &message, int min = 0, int max = 100);
	void WaitSetMessage(const QString &messsage);
	void WaitSetValue(int value);
	void WaitStop();

	void BeastWaitSetMessage(const QString &messsage);
	bool BeastWaitCanceled();

	void EnableGlobalTimeout(bool enable);

signals:
    void GlobalInvalidateTimeout();
    
// qt actions slots
public slots:
	void OnProjectOpen();
	void OnProjectClose();
	void OnSceneNew();
	void OnSceneOpen();
	void OnSceneSave();
	void OnSceneSaveAs();
	void OnSceneSaveToFolder();
	void OnRecentTriggered(QAction *recentAction);
	void ExportMenuTriggered(QAction *exportAsAction);

    void OnImportSpeedTreeXML();

	void OnUndo();
	void OnRedo();

	void OnEditorGizmoToggle(bool show);
	void OnAllowOnSceneSelectionToggle(bool allow);

	void OnReloadTextures();
	void OnReloadTexturesTriggered(QAction *reloadAction);
	void OnReloadSprites();

	void OnSelectMode();
	void OnMoveMode();
	void OnRotateMode();
	void OnScaleMode();
	void OnPivotCenterMode();
	void OnPivotCommonMode();
	void OnManualModifMode();
	void OnPlaceOnLandscape();
	void OnSnapToLandscape();
	void OnResetTransform();

	void OnMaterialEditor();
	void OnTextureBrowser();
	void OnSceneLightMode();

	void OnCubemapEditor();
		
	void OnLandscapeDialog();
	void OnLightDialog();
	void OnCameraDialog();
	void OnEmptyEntity();

	void OnUserNodeDialog();
	void OnSwitchEntityDialog();
	void OnParticleEffectDialog();
	void OnUniteEntitiesWithLODs();
	void OnAddEntityMenuAboutToShow();
	void OnAddEntityFromSceneTree();

	void OnSetSkyboxNode();
	
	void OnShowSettings();
	void OnOpenHelp();

	void OnSetShadowColor();
	void OnShadowBlendModeWillShow();
	void OnShadowBlendModeAlpha();
	void OnShadowBlendModeMultiply();

	void OnSaveHeightmapToPNG();
	void OnSaveTiledTexture();

	void OnConvertModifiedTextures();
    
	void OnCloseTabRequest(int tabIndex, Request *closeRequest);

	void OnBeast();
	void OnBeastAndSave();

	void OnConvertToShadow();

	void OnCameraSpeed0();
	void OnCameraSpeed1();
	void OnCameraSpeed2();
	void OnCameraSpeed3();
	void OnCameraLookFromTop();

	void OnLandscapeEditorToggled(SceneEditor2* scene);
	void OnCustomColorsEditor();
	void OnHeightmapEditor();
	void OnRulerTool();
	void OnTilemaskEditor();
	void OnVisibilityTool();
	void OnNotPassableTerrain();
	
	void OnAddActionComponent();
	void OnRemoveActionComponent();

	void OnObjectsTypeMenuWillShow();
	void OnObjectsTypeChanged(QAction *action);
    void OnObjectsTypeChanged(int type);

	void OnHangingObjects();
	void OnHangingObjectsHeight(double value);

	bool PrepareCustomColorsEditorDisabling(bool& saveNeeded);
	
protected:
	virtual bool eventFilter(QObject *object, QEvent *event);
	void closeEvent(QCloseEvent * e);

	void SetupMainMenu();
	void SetupToolBars();
	void SetupStatusBar();
	void SetupDocks();
	void SetupActions();
	void SetupTitle();
	void SetupShortCuts();

	void InitRecent();
	void AddRecent(const QString &path);
    
    void CreateMaterialEditorIfNeed();
    
    void StartGlobalInvalidateTimer();

	void RunBeast();


	bool IsAnySceneChanged();

	void SetLandscapeSettingsEnabled(bool);

	void DiableUIForFutureUsing();

protected slots:
	void ProjectOpened(const QString &path);
	void ProjectClosed();

	void SceneCommandExecuted(SceneEditor2 *scene, const Command2* command, bool redo);
	void SceneActivated(SceneEditor2 *scene);
	void SceneDeactivated(SceneEditor2 *scene);

    void OnGlobalInvalidateTimeout();

	void EditorLightEnabled(bool enabled);

	void OnSnapToLandscapeChanged(SceneEditor2* scene, bool isSpanToLandscape);

	void UnmodalDialogFinished(int);
private:
	Ui::MainWindow *ui;
	QtWaitDialog *waitDialog;
	QtWaitDialog *beastWaitDialog;

	QtPosSaver posSaver;
	bool globalInvalidate;

	QList<QAction *> recentScenes;
	ModificationWidget *modificationWidget;

	// TODO: remove this old screen -->
	MaterialEditor *materialEditor;
	// <--

	QtLabelWithActions *objectTypesLabel;
    QComboBox *objectTypesWidget;

	AddSwitchEntityDialog*	addSwitchEntityDialog;
	LandscapeDialog*		landscapeDialog;
	HangingObjectsHeight*	hangingObjectsWidget;

	void EnableSceneActions(bool enable);
	void EnableProjectActions(bool enable);
	void UpdateConflictingActionsState(bool enable);

	void LoadViewState(SceneEditor2 *scene);
	void LoadUndoRedoState(SceneEditor2 *scene);
	void LoadModificationState(SceneEditor2 *scene);
	void LoadEditorLightState(SceneEditor2 *scene);
	void LoadShadowBlendModeState(SceneEditor2* scene);
	void LoadGPUFormat();
	void LoadLandscapeEditorState(SceneEditor2* scene);
	void LoadObjectTypes(SceneEditor2 *scene);
	void LoadHangingObjects(SceneEditor2 *scene);

	bool SaveTilemask(bool forAllTabs = true);

	// Landscape editor specific
	// TODO: remove later -->
	bool IsTilemaskModificationCommand(const Command2* cmd);
	bool LoadAppropriateTextureFormat();
	bool IsSavingAllowed();
	// <--
};


#endif // MAINWINDOW_H

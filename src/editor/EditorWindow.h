#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFile>
#include <QLabel>
#include <QMainWindow>
#include <QScrollArea>
#include <QSplitter>
#include <QtMultimedia/QAudioOutput>

#include "EditState.h"
#include "EditorScrollArea.h"
#include "KeyboardEditor.h"
#include "PxtoneIODevice.h"
#include "SideMenu.h"
#include "UnitListModel.h"
#include "pxtone/pxtnService.h"
#include "server/BroadcastServer.h"
#include "server/Client.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class EditorWindow;
}
QT_END_NAMESPACE

class EditorWindow : public QMainWindow {
  Q_OBJECT

 public:
  EditorWindow(QWidget* parent = nullptr);
  ~EditorWindow();

 public slots:
  void resetAudio();
 private slots:
  void connectToHost();

 private:
  void Host(bool load_file);
  void keyPressEvent(QKeyEvent* event);
  QAudioOutput* m_audio;
  KeyboardEditor* m_keyboard_editor;
  pxtnService m_pxtn;
  EditorScrollArea* m_scroll_area;
  PxtoneIODevice m_pxtn_device;
  QSplitter* m_splitter;
  SideMenu* m_side_menu;
  BroadcastServer* m_server;
  Client* m_client;
  QString m_filename;
  QLabel* m_server_status;
  QLabel* m_client_status;
  UnitListModel* m_units;

  std::unique_ptr<NotePreview> m_note_preview;

  Ui::EditorWindow* ui;
  void togglePlayState();
  void resetAndSuspendAudio();
  bool saveToFile(QString filename);
  void save();
  void saveAs();
  bool loadDescriptor(pxtnDescriptor& desc);
  void refreshSideMenuWoices();
  void refreshSideMenuTempoBeat();
};
#endif  // MAINWINDOW_H

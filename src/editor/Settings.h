#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QString>

#include "pxtone/pxtnEvelist.h"

const extern QString WOICE_DIR_KEY;
const extern QString BUFFER_LENGTH_KEY;
const extern double DEFAULT_BUFFER_LENGTH;
const extern QString VOLUME_KEY;

const extern QString PTCOP_FILE_KEY;
const extern QString PTREC_SAVE_FILE_KEY;
const extern QString DISPLAY_NAME_KEY;

const extern QString HOST_SERVER_PORT_KEY;
const extern int DEFAULT_PORT;

const extern QString SAVE_RECORDING_ENABLED_KEY;
const extern QString HOSTING_ENABLED_KEY;

const extern QString CONNECT_SERVER_KEY;

namespace Settings {
namespace TextSize {
int get();
void increase();
void decrease();
}  // namespace TextSize

namespace Version {
const QString &string();
}

namespace CustomStyle {
bool get();
void set(bool);
}  // namespace CustomStyle

namespace ChordPreview {
bool get();
void set(bool);
}  // namespace ChordPreview

namespace SpacebarStop {
bool get();
void set(bool);
}  // namespace SpacebarStop

namespace VelocityDrag {
bool get();
void set(bool);
}  // namespace VelocityDrag

namespace SwapScrollOrientation {
bool get();
void set(bool);
}  // namespace SwapScrollOrientation

namespace SwapZoomOrientation {
bool get();
void set(bool);
}  // namespace SwapZoomOrientation

namespace AutoAddUnit {
bool get();
void set(bool);
}  // namespace AutoAddUnit

namespace AutoAdvance {
bool get();
void set(bool);
}  // namespace AutoAdvance

namespace ChangeDialogDirectory {
bool get();
void set(bool);
}  // namespace ChangeDialogDirectory

namespace PolyphonicMidiNotePreview {
bool get();
void set(bool);
}  // namespace PolyphonicMidiNotePreview

namespace UnitPreviewClick {
bool get();
void set(bool);
}  // namespace UnitPreviewClick

namespace RenderFileDestination {
QString get();
void set(QString);
}  // namespace RenderFileDestination

namespace SideMenuWidth {
QList<int> get();
void set(const QList<int> &);
}  // namespace SideMenuWidth

namespace BottomBarHeight {
QList<int> get();
void set(const QList<int> &);
}  // namespace BottomBarHeight

namespace CopyKinds {
QList<int> get();
void set(const QList<int> &);
}  // namespace CopyKinds

}  // namespace Settings

#endif  // SETTINGS_H

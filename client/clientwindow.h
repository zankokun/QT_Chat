#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QAudioInput>
#include <QAudioOutput>
#include <QBuffer>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class ClientWindow;
}
QT_END_NAMESPACE

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

private slots:
    void connectToServer();
    void disconnectFromServer();
    void sendTextMessage();
    void startRecording();
    void stopRecording();
    void startCall();
    void endCall();
    void onSocketConnected();
    void onSocketDisconnected();
    void onTextMessageReceived(const QString &sender, const QString &message);
    void onVoiceMessageReceived(const QString &sender, const QByteArray &audioData);
    void onCallRequestReceived(const QString &caller);
    void onCallResponseReceived(bool accepted, const QString &callee);
    void onCallEnded(const QString &peer);
    void onVoiceDataReceived(const QByteArray &data);
    void handleAudioInputStateChanged(QAudio::State state);
    void handleAudioOutputStateChanged(QAudio::State state);

private:
    Ui::ClientWindow *ui;
    QTcpSocket *socket;
    QString userName;
    bool isRecording;
    bool inCall;
    QString callPeer;
    QAudioInput *audioInput;
    QAudioOutput *audioOutput;
    QBuffer inputBuffer;
    QBuffer outputBuffer;
    QAudioFormat format;

    void setupAudio();
    void playAudio(const QByteArray &audioData);
    void appendMessage(const QString &sender, const QString &message, bool isMyMessage);
};

#endif // CLIENTWINDOW_H
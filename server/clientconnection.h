#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include <QTcpSocket>
#include <QAudioFormat>
#include <QAudioInput>
#include <QAudioOutput>

class ClientConnection : public QTcpSocket
{
    Q_OBJECT

public:
    explicit ClientConnection(QObject *parent = nullptr);
    ~ClientConnection();

    QString getName() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    void sendTextMessage(const QString &sender, const QString &message);
    void sendVoiceMessage(const QString &sender, const QByteArray &audioData);
    void sendCallRequest(const QString &caller);
    void sendCallResponse(bool accepted, const QString &callee);
    void sendCallEnded(const QString &peer);
    void sendVoiceData(const QByteArray &data);

signals:
    void textMessageReceived(const QString &sender, const QString &message);
    void voiceMessageReceived(const QString &sender, const QByteArray &audioData);
    void callRequestReceived(const QString &caller);
    void callResponseReceived(bool accepted, const QString &callee);
    void callEnded(const QString &peer);
    void voiceDataReceived(const QByteArray &data);

private slots:
    void onReadyRead();

private:
    QString m_name;
    quint32 m_blockSize;
};

#endif // CLIENTCONNECTION_H
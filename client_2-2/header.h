#ifndef HEADER_H
#define HEADER_H

#include <iostream>
#include <WinSock2.h>
#include <windows.h>
#include <sstream>
#include <WS2tcpip.h>
#include <string>
#include <thread>
#include <vector>
#include <QPushButton>
#include <QApplication>
#include <QDialog>
#include <QHBoxLayout>
#include <QDebug>
#include <QLabel>
#include <stdio.h>
#include <math.h>
#include <QLineEdit>
#include <QInputDialog>
#include <QCoreApplication>

#include <QMediaDevices>
#include <QAudioDevice>
#include <QMediaFormat>
#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QMediaRecorder>

#include <QFile>
#include <QDebug>
#include <QTimer>
#include <QByteArray>
#include <QString>
#include <QTextStream>


class body
{
private:
    bool working = true;

public:
    void shutdown(){working = false;}
    bool isWorking() const { return working;}
    void coding(std::string& str);

    void decoding(std::string& str);

    void work();

    void name();

    void audio();

    void doing();

};

class windows : public QObject
{
    Q_OBJECT
private:

    QWidget* window;
    QPushButton* Sending;
    QLineEdit* Texting;
    QPushButton* startButton;
    QPushButton* stopButton;

    std::vector<QLabel*> ylabels;
    std::vector<QLabel*> olabels;

    QLabel* your;
    QLabel* op;
    body &ob;
    QLabel* Warnings;

    int WindowSizeX = 400;
    int WindowSizeY = 800;

    int ButSendingSizeX = 50;
    int ButSendingSizeY = ButSendingSizeX;

    int LbTextingSizeX = WindowSizeX - 90 - (3*ButSendingSizeX);
    int LbTextingSizeY = ButSendingSizeY;


    QAudioInput *audioInput = nullptr;
    QMediaCaptureSession *captureSession = nullptr;
    QMediaRecorder *recorder = nullptr;

    QLabel *statusLabel = nullptr;

    QString Base = "recordingmessage";
    QString Time = "";
    QString Format = ".wav";

    QString Result;


public:
    windows(body& ob):QObject{}, ob{ob} {}
    void setupSettings();

    void creating();

    void setgeometry();

    void showing();

    void connecting();

    void messages();

    static QString converttoascii(const QString &filePath);


    std::string fileToString(const QString &filePath);
    static bool convertFromAscii(const QString &txtFilePath, const QString &wavFilePath);

    bool saveStringToFileQt(const QString &filePath, const QString &content);
private slots:
    void sendingslot();

    void startRecording();

    void stopRecording();
};

#endif

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "streamthread.h"
#include "global.h"

// allows better looking ui elements, used throughout main
QString greenTextBox = "background-color: green; border-style: outset; "
                       "border-width: 2px; border-radius: 10px; border-color: beige; "
                       "min-width: 10em; padding: 6px;";
QString redTextBox = "background-color: red; border-style: outset; "
                     "border-width: 2px; border-radius: 10px; border-color: beige; "
                     "min-width: 10em; padding: 6px;";
QString greyTextBox = "background-color: grey; border-style: outset; "
                      "border-width: 2px; border-radius: 10px; border-color: beige; "
                      "min-width: 10em; padding: 6px;";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // setup the header text
    ui->header->setText("    Mac Side Connection:");
    ui->header->setStyleSheet("font: bold 16px;");

    // setup the instruction text
    ui->instructions->setText("All steps must be performed in order:\n\n"
                              "1. Check the connection, if no connection must fix\n    before continuing\n"
                              "2. Output the stimulation points chosen in\n    Brainsight\n"
                              "3. Begin streaming camera information from\n    Brainsight");

    // setup the points, streaming and connection labels and buttons
    ui->pointsPushButton->setEnabled(false);
    ui->streamingPushButton->setEnabled(false);
    ui->streamingLabel->setStyleSheet(greyTextBox);
    ui->pointsLabel->setStyleSheet(greyTextBox);
    ui->checkConnectionLabel->setStyleSheet(greyTextBox);
    ui->checkConnectionLabel->setText("Not Checked...");
    ui->pointsLabel->setText("Not Submitted...");
    ui->streamingLabel->setText("Not Streaming...");

}

MainWindow::~MainWindow()
{
    delete ui;
}

// control the streaming of the python script
void MainWindow::on_streamingPushButton_clicked()
{
    // depending on streaming conditions want to have different functionality
    if (stopThread) {
        // initialize the stop streaming functionality
        ui->streamingLabel->setText("Streaming!");
        ui->streamingLabel->setStyleSheet(greenTextBox);
        ui->streamingPushButton->setText("End Streaming");

        // start the streaming thread
        stopThread = false;
        StreamThread *streamThread = new StreamThread;
        streamThread->start();

    } else {
        // initialize the start streaming functionality
        ui->streamingLabel->setText("Not Streaming...");
        ui->streamingLabel->setStyleSheet(redTextBox);
        ui->streamingPushButton->setText("Begin Streaming");

        // change this so thread can be executed again
        stopThread = true;
    }
}

// controls the point submission to the windows computer
void MainWindow::on_pointsPushButton_clicked()
{
    // assert that the points have been successfully submitted
    // assert that the connection is valid
    QString inputFilename = "/Users/as_763/Desktop/Exported Session Data.txt";
    QString outputFilename = "/Users/as_763/Desktop/MOCKSHARE/points.txt";
    QFile f_in (inputFilename);
    QFile f_out (outputFilename);
    f_in.open(QIODevice::ReadOnly | QIODevice::Text);
    f_out.open(QIODevice::WriteOnly | QIODevice::Text);

    // if the file is not open then obviously no connection
    if(!f_in.isOpen()){
        qDebug() << "No Points!!";
        ui->pointsLabel->setText("No Points!");
        f_in.close();
        f_out.close();
        return;
    }

    // if cannot open writing file then an internal issue has occurred
    if(!f_out.isOpen()){
        qDebug() << "Writing Issue!";
        ui->pointsLabel->setText("Internal Issue!");
        f_in.close();
        f_out.close();
        return;
    }

    // get the relavent data
    QTextStream inStream(&f_in);
    QTextStream outStream(&f_out);

    // read through the first 8 lines of useless info
    for (int i = 0; i < 8; i++) {
        inStream.readLine();
    }

    // get all the useful information, break when finished
    while (true) {
        QString checkLine = inStream.readLine();

        // if a line begins with # then all useful information obtained
        if (checkLine.startsWith("#")) {
            break;
        }

        // output the data
        outStream << checkLine << "\n";
    }

    // if at this point then the points have been successfully submitted
    ui->pointsLabel->setText("Points Submitted!");
    ui->pointsLabel->setStyleSheet(greenTextBox);
    ui->pointsPushButton->setEnabled(false);

    // if points submitted then must setup streaming button for last step
    ui->streamingLabel->setStyleSheet(redTextBox);
    ui->streamingPushButton->setEnabled(true);

    // close the file
    f_in.close();
    f_out.close();
}

// checks the connection to the share between mac and windows machines
void MainWindow::on_checkConnectionPushButton_clicked()
{
    // assert that the connection is valid
    QString inputFilename = "/Users/as_763/Desktop/MOCKSHARE/connection.txt";
    QFile file (inputFilename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    // if the file is not open then obviously no connection
    if(!file.isOpen()){
        qDebug() << "No connection!";
        ui->checkConnectionLabel->setText("Connection Bad!");
        ui->checkConnectionLabel->setStyleSheet(redTextBox);
        file.close();
        return;
    }

    // of connection good then signal the user
    ui->checkConnectionLabel->setText("Connection Good!");
    ui->checkConnectionLabel->setStyleSheet(greenTextBox);
    ui->checkConnectionPushButton->setEnabled(false); // TODO: may need to be able to check connection many times so possibly adjust

    // if connection good then must set the points button for next step
    ui->pointsLabel->setStyleSheet(redTextBox);
    ui->pointsPushButton->setEnabled(true);

    file.close();
}

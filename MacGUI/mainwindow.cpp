#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "streamthread.h"
#include "global.h"

// allows better looking ui elements, used throughout main
QString greenTextBox = "background-color: green; border-style: outset; "
                       "border-width: 2px; border-radius: 10px; border-color: beige; "
                       "min-width: 10em; padding: 6px; qproperty-alignment: AlignCenter;";
QString redTextBox = "background-color: red; border-style: outset; "
                     "border-width: 2px; border-radius: 10px; border-color: beige; "
                     "min-width: 10em; padding: 6px; qproperty-alignment: AlignCenter;";
QString greyTextBox = "background-color: grey; border-style: outset; "
                      "border-width: 2px; border-radius: 10px; border-color: beige; "
                      "min-width: 10em; padding: 6px; qproperty-alignment: AlignCenter;";
QString greenTextBoxSmaller = "background-color: green; border-style: outset; "
                              "border-width: 2px; border-radius: 10px; border-color: beige; "
                              "min-width: 8em; padding: 6px; qproperty-alignment: AlignCenter;";
QString redTextBoxSmaller = "background-color: red; border-style: outset; "
                            "border-width: 2px; border-radius: 10px; border-color: beige; "
                            "min-width: 8em; padding: 6px; qproperty-alignment: AlignCenter;";
QString greyTextBoxSmaller = "background-color: grey; border-style: outset; "
                             "border-width: 2px; border-radius: 10px; border-color: beige; "
                             "min-width: 8em; padding: 6px; qproperty-alignment: AlignCenter;";

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
                              "3. Begin streaming camera information from\n    Brainsight\n\n"
                              "** If any issues click Reset, also make sure all\n"
                              "    boxes are green before continuing to Windows\n"
                              "    setup");

    // setup the points, streaming and connection labels and buttons
    ui->pointsPushButton->setEnabled(false);
    ui->streamingPushButton->setEnabled(false);
    ui->streamingLabel->setStyleSheet(greyTextBox);
    ui->pointsLabel->setStyleSheet(greyTextBox);
    ui->checkConnectionLabel->setStyleSheet(greyTextBoxSmaller);
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

        // check if file is accessible
        // also navigate to the correct location of the Tool Dump Location
        QString inputFilename = QFileDialog::getExistingDirectory(this, tr("Navigate to the Tool Dump Locations TextFile from Brainsight..."),
                "Desktop", QFileDialog::ShowDirsOnly) + "/Tool Locations Dump.txt";
        QFile f_in(inputFilename);
        f_in.open(QIODevice::ReadOnly | QIODevice::Text);

        if(!f_in.isOpen()) {
            ui->streamingLabel->setText("File Not Found!");
            f_in.close();
            return;
        }

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
    // assert that the connection is valid by navigating to the directory that contains Exported Session Data
    QString inputFilename = QFileDialog::getExistingDirectory(this, tr("Navigate to the Exported Session Data TextFile from Brainsight..."),
            "Desktop", QFileDialog::ShowDirsOnly) + "/Exported Session Data.txt";
    QString outputFilename = shareOutput + "/points.txt";
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

        // process the data to follow a specific standard, and the output this data
        QStringList removeTabs = checkLine.split("\t");
        outStream << removeTabs[0] << " ";
        outStream << removeTabs[4] << " " << removeTabs[7] << " " << removeTabs[10] << " " << removeTabs[1] << " ";
        outStream << removeTabs[5] << " " << removeTabs[8] << " " << removeTabs[11] << " " << removeTabs[2] << " ";
        outStream << removeTabs[6] << " " << removeTabs[9] << " " << removeTabs[12] << " " << removeTabs[3] << " ";
        outStream << "0 0 0 1\n";
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
    // assert that the connection is valid by navigating to share
    QString inputFilename = QFileDialog::getExistingDirectory(this, tr("Find Correct Share..."), "Desktop", QFileDialog::ShowDirsOnly);

    // open the share and check for the correct connection file present
    QFile file (inputFilename + "/connection.txt");
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    // if the file is not open then obviously no connection
    if(!file.isOpen()){
        qDebug() << "No connection!";
        ui->checkConnectionLabel->setText("Connection Bad!");
        ui->checkConnectionLabel->setStyleSheet(redTextBoxSmaller);
        file.close();
        return;
    }

    // submit the share location as a global variable for all streaming
    shareOutput = inputFilename;

    // of connection good then signal the user
    ui->checkConnectionLabel->setText("Connection Good!");
    ui->checkConnectionLabel->setStyleSheet(greenTextBoxSmaller);
    ui->checkConnectionPushButton->setEnabled(false); // TODO: may need to be able to check connection many times so possibly adjust

    // if connection good then must set the points button for next step
    ui->pointsLabel->setStyleSheet(redTextBox);
    ui->pointsPushButton->setEnabled(true);

    file.close();
}

void MainWindow::on_resetButton_clicked()
{
    // if the streaming thread is running then can be stopped
    stopThread = true;

    // reset the connection checker
    shareOutput = "";

    // reset all labels, buttons and text to the initial state
    ui->pointsPushButton->setEnabled(false);
    ui->streamingPushButton->setEnabled(false);
    ui->checkConnectionPushButton->setEnabled(true);
    ui->streamingLabel->setStyleSheet(greyTextBox);
    ui->pointsLabel->setStyleSheet(greyTextBox);
    ui->checkConnectionLabel->setStyleSheet(greyTextBoxSmaller);
    ui->streamingPushButton->setText("Begin Streaming");
    ui->checkConnectionLabel->setText("Not Checked...");
    ui->pointsLabel->setText("Not Submitted...");
    ui->streamingLabel->setText("Not Streaming...");
}

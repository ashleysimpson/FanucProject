#include "mainwindow.h"
#include "ui_mainwindow.h"

// global variables
bool isStreaming;
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

    // a global check for whether streaming is happening
    isStreaming = false;

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
    if (!isStreaming) {
        ui->streamingLabel->setText("Streaming!");
        ui->streamingLabel->setStyleSheet(greenTextBox);
        ui->streamingPushButton->setText("End Streaming");
        isStreaming = true;
    } else {
        ui->streamingLabel->setText("Not Streaming...");
        ui->streamingLabel->setStyleSheet(redTextBox);
        ui->streamingPushButton->setText("Begin Streaming");
        isStreaming = false;
    }
}

// controls the point submission to the windows computer
void MainWindow::on_pointsPushButton_clicked()
{
    ui->pointsLabel->setText("Points Submitted!");
    ui->pointsLabel->setStyleSheet(greenTextBox);
    ui->pointsPushButton->setEnabled(false);

    // if points submitted then must setup streaming button for last step
    ui->streamingLabel->setStyleSheet(redTextBox);
    ui->streamingPushButton->setEnabled(true);
}

// checks the connection to the share between mac and windows machines
void MainWindow::on_checkConnectionPushButton_clicked()
{
    ui->checkConnectionLabel->setText("Connection Good!");
    ui->checkConnectionLabel->setStyleSheet(greenTextBox);
    ui->checkConnectionPushButton->setEnabled(false); // TODO: may need to be able to check connection many times so possibly adjust

    // if connection good then must set the points button for next step
    ui->pointsLabel->setStyleSheet(redTextBox);
    ui->pointsPushButton->setEnabled(true);
}

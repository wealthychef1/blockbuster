/*
  #
  # $RCSfile: sidecar.h,v $
  # $Name:  $
  # 
  # ASCI Visualization Project 
  #
  # Lawrence Livermore National Laboratory
  # Information Management and Graphics Group
  # P.O. Box 808, Mail Stop L-561
  # Livermore, CA 94551-0808
  #
  # For information about this project see:
  # 	http://www.llnl.gov/sccd/lc/img/ 
  #
  # 	or contact: asciviz@llnl.gov
  #
  # For copyright and disclaimer information see:
  #	$(ASCIVIS_ROOT)/copyright_notice_1.txt
  #
  # $Id: sidecar.h,v 1.50 2009/05/07 21:52:44 wealthychef Exp $
  #
  #   Abstract: header which contains the objects related to the sidecar Qt GUI
  #
  #   Author: Rich Cook
  #
  # This work performed under the auspices of the U.S. Department of Energy by Lawrence Livermore National Laboratory under Contract DE-AC52-07NA27344.
  # This document was prepared as an account of work sponsored by an agency of the United States government. Neither the United States government nor Lawrence Livermore National Security, LLC, nor any of their employees makes any warranty, expressed or implied, or assumes any legal liability or responsibility for the accuracy, completeness, or usefulness of any information, apparatus, product, or process disclosed, or represents that its use would not infringe privately owned rights. Reference herein to any specific commercial product, process, or service by trade name, trademark, manufacturer, or otherwise does not necessarily constitute or imply its endorsement, recommendation, or favoring by the United States government or Lawrence Livermore National Security, LLC. The views and opinions of authors expressed herein do not necessarily state or reflect those of the United States government or Lawrence Livermore National Security, LLC, and shall not be used for advertising or product endorsement purposes.
  #
*/
// the ui_sidecar.h file is generated by Qt during the make process, from the sidecar.ui file generated by Qt Designer
#ifndef CLIENT_H
#define CLIENT_H 1
#include "ui_sidecar.h"
#include "ui_blockbusterLaunchDialog.h"
#include "RemoteControl.h"
#include "MovieCues.h"
#include <QMainWindow>
#include <QDialog>
#include <QTcpSocket>
#include <QApplication>
#include <QProcess>
#include <QHostInfo>
#include <QTimer>
#ifndef Q_MOC_RUN
// See: https://bugreports.qt-project.org/browse/QTBUG-22829
#include <boost/algorithm/string.hpp>
#include "Prefs.h"
#include "errmsg.h"
#endif

extern Preferences gPrefs; 

typedef enum {
  BB_ERROR=1,  // something went wrong but don't terminate yet
  BB_DISCONNECTED, 
  BB_STARTING, // before QProcess emits started() signal
  BB_WAIT_CONNECTION, // Waiting for mCommandSocket to get established
  // BB_WAIT_BACKCHANNEL, // Waiting for mBackChannel to get established
  BB_CONNECTED, // both commands and backchannel are operative
  BB_DISCONNECTING, 
  BB_FINISHED // signal to terminate thread
} connectionState; 




//=================================================================
class SideCar: public QMainWindow, public Ui::SideCarWindow {
  Q_OBJECT
    public: 
  SideCar(QApplication *app, Preferences *prefs, QWidget *parent=NULL);
  ~SideCar() {
    mExiting = true;  // prevent segfaults from certain callback loops at exit
    if (mBlockbusterProcess) {
      mBlockbusterProcess -> kill(); 
    }
  } 

  void endBlockbusterProcess(void) {
    if (mBlockbusterProcess) {
      dbprintf(1, "Ending blockbuster process\n"); 
      mBlockbusterProcess->kill(); 
      mBlockbusterProcess->deleteLater(); 
      mBlockbusterProcess = NULL; 
    }
    return;
  }
  //prefs:
  void doStressTests(void) {mDoStressTests = true; }
  void ReadCueFile(std::string filename); 
  QString getDefaultBlockbusterPath(void) {
    return (mPrefs->GetValue("sidecarDir") + "/blockbuster").c_str(); 
  }


  QProcess *createNewBlockbusterProcess(void);
  void askLaunchBlockbuster(const MovieCue *cue, QString moviename = "", bool executeWithoutAsking=false); 
  int listenForBlockbuster() {
    mBlockbusterServer.listen(QHostAddress::Any); 
    return mBlockbusterServer.serverPort(); 
  }
  //virtual QSize sizeHint () const;
  public slots:
  void setState(connectionState); 

  private slots:
  void movieSnapshotEndFrame(); 
  void movieSnapshotAltEndFrame(); 
  void movieSnapshotStartFrame(); 

  // connections: 
  void blockbusterFinished(int, QProcess::ExitStatus) { 
    mBlockbusterProcess->deleteLater(); 
    mBlockbusterProcess = NULL; 
    disconnectFromBlockbuster(); 
  }
  void setBlockbusterSocket(QTcpSocket *newSocket);
  void setBlockbusterPort(QString port);
  void connectToBlockbuster(); 
  void connectedToBlockbuster(); 
  void incomingBlockbusterConnection();  
  void checkBlockbusterData(); 
  void socketError(QAbstractSocket::SocketError socketError);
  void disconnectFromBlockbuster();
  void disconnected();
  //void ProcessStarted();
  void blockbusterProcessError(QProcess::ProcessError error);
  void blockbusterProcessExit(int exitcode=-1, QProcess::ExitStatus status=QProcess::CrashExit);
  void blockbusterReadStdErr();
  void blockbusterReadStdOut();
 
  void on_captureKeystrokesCheckBox_clicked();
  void on_launchBlockbusterButton_clicked();
  void on_connectButton_clicked();
  void on_HostField_returnPressed();
  void on_PortField_returnPressed();
  // commands:
  void on_actionOpen_Cue_File_triggered();
  void on_actionSave_Cue_File_triggered();
  void on_actionSave_Cue_File_As_triggered();
  // void on_actionQuit_triggered(); // not needed, connected to quit in sidecar constructor
  void on_showCuesButton_clicked(); 
  void on_actionToggle_Controls_triggered();
  void on_actionPlay_triggered(); 
  void on_actionStepAhead_triggered(); 
  void on_actionStepBackward_triggered(); 
  void on_actionStop_triggered(); 
  void on_actionGo_To_Beginning_triggered(); 
  void on_actionGo_To_End_triggered(); 
  void on_actionGo_To_Frame_triggered(); 

  // remote controls for blockbuster
  void quitButton_clicked();
  void openButton_clicked();   
  void stereoCheckBox_stateChanged(int);  
  void centerPushButton_clicked();   
  void fullSizeButton_clicked();   
  void zoomToFitCheckBox_clicked();   
  void fullScreenCheckBox_clicked();   
  void startButton_clicked();   
  void backStepButton_clicked();   
  void reverseButton_clicked();   
  void stopButton_clicked();   
  void playButton_clicked();   
  void stepButton_clicked();   
  void endButton_clicked();   
  
  void frameSlider_valueChanged(int);   
  void frameField_returnPressed();   
  void saveImageButton_clicked() ;
  void zoomSpinBox_valueChanged(double);   
  void lodSpinBox_valueChanged(int);   
  void fpsSpinBox_valueChanged(double);   
  void foreverCheckBox_stateChanged(int) ;   
  void noScreensaverCheckBox_stateChanged(int) ;   
  void repeatCheckBox_stateChanged(int);  
  void pingpongCheckBox_stateChanged(int);  
 
  void sendString(); 
  void SendEvent(MovieEvent event); 
  
  // from CueManager:
  void executeCue(MovieCue* iCue); 
  void takeSnapshot(); 
  void cueSelectionChanged(MovieCue *); 

  // from the keystroke filter in mApp:
  void InterestingKey(QKeyEvent *key); 

  // app is about to quit...
  void aboutToQuit() {
    closeEvent(NULL); 
  }
  // quit the application if this window is closed
  void closeEvent(QCloseEvent *event) {
    // give the cues window a say in the matter:
    if (mCueManager->okToQuit()) {
      mCueManager->blockExecution();  
      QCoreApplication::quit (); 
    } else {
      if (event) {
        event->ignore(); 
      } else {        
        this->show(); 
      }
    }
    return; 
  }
  
 public:
  Preferences *mPrefs; 

 private: 
  void enableMovieActions(bool enabled=true); 
  void enableRemoteControl(bool enabled); 
  //  void GetNewBackChannel(void); 
  void checkCaptureKeystrokes(void); 

  QApplication *mApp; 
  class BlockbusterLaunchDialog *mLaunchDialog;
  QProcess *mBlockbusterProcess; 
  KeyPressIntercept mKeyPressIntercept; 
  bool mDoStressTests; 
  connectionState mState; 
  QString mHost, mPortString, mMovieName;
  QString mBlockbusterCWD; 
  /* NEW */
  QTcpServer mBlockbusterServer; 
  QTcpSocket *mBlockbusterSocket; 
  /* end NEW */
  uint32_t mNextCommandID;
  MovieCueManager *mCueManager; 
  RemoteControl *mRemoteControl; 
  bool mExiting, mCueExecuting; 
}; 

//=======================================================
struct HostProfile {
  
  // =============================================================
  HostProfile() {
    init(); 
    return; 
  }
 
 
  // =============================================================
  // make an exact duplicate of "in"
  HostProfile(const HostProfile *in) {
    if (in) {
      *this = *in; 
    }
    return;     
  }

  // =============================================================
  // make a copy of "in" that is not readOnly and has name "name"
  HostProfile(QString name, const HostProfile *in){
    init(); 
    *this = *in; 
    mName = name; 
    mProfileFile = mUserHostProfileFile ;
    return;     
  }

  
  // =============================================================
  QString displayName(void) const {
    if (mReadOnly) return mName+" (readonly)"; 
    return mName; 
  }


  // =============================================================

  HostProfile& operator =(const HostProfile& other);

  bool operator <(const HostProfile& other) const;
  bool operator >(const HostProfile& other) const;

  void init(void);

  HostProfile& operator << (std::string profileString);

  operator QString() const;
  operator std::string() const;

  bool operator !() const { return mValid; }

  static QString mUserHostProfileFile; 

  public: 
  QString mName;
  QString mHostName, mPort, mVerbosity, mRsh, mSidecarHost,  
    mDisplay, mBlockbusterPath, mProfileFile; 
  bool mSetDisplay,  mReadOnly, mAutoSidecarHost, 
    mPlay, mFullScreen, mShowControls, mUseDMX, mMpiFrameSync, 
    mNoSmallWindows, mAutoBlockbusterPath;
  bool mValid; 
}; 


//=======================================================
class BlockbusterLaunchDialog: public QDialog, 
  public Ui::blockbusterLaunchDialog {
  Q_OBJECT
    public:
  BlockbusterLaunchDialog(SideCar *sidecar, QString host, QString port, QString file, connectionState state, QString rshCmd, long bbVerbose);
  ~BlockbusterLaunchDialog() {
    saveHistory(fileNameComboBox, "fileNameComboBox.history"); 
  }
  void setState(connectionState newstate) {
    mState = newstate; 
    emit stateChanged(newstate); 
  }
  QString getPort(void) { return hostPortField->text(); }
  QString getHost(void) { return hostNameField->text(); }
 signals:   
  void stateChanged(connectionState newstate); 
  void listenForBlockbuster(); 

  public slots:
  void blockbusterConnected() {      
    hide(); 
  }
  
  public slots:
  void comboBoxItemChanged(int num) {
    if (num == -1 && fileNameComboBox->count() == 0) {
      fileNameComboBox->addItem("/Type/movie/path/here"); 
    } else {
      if (fileNameComboBox->count() && fileNameComboBox->itemText(0) == "/Type/movie/path/here") {
        fileNameComboBox->removeItem(0); 
      }
    }
    return; 
  }
  
  void on_browseButton_clicked();
  void on_connectButton_clicked();
  void on_cancelButton_clicked() {
    dbprintf(5, "User canceled dialog\n"); 
    //KillProcess(); 
    this->hide(); 
  }
  bool ProfileNameExists(QString name);
  void createNewProfile(const HostProfile *inProfile);
  void on_deleteProfilePushButton_clicked(); 
  void on_saveProfilePushButton_clicked(); 
  void on_newProfilePushButton_clicked(); 
  void on_duplicateProfilePushButton_clicked(); 
  void setupGuiAndCurrentProfile(int index);
  void on_hostProfilesComboBox_currentIndexChanged(int index); 
  void on_deleteMoviePushButton_clicked(); 
  void on_launchButton_clicked();
  void on_useDMXCheckBox_clicked();
  void on_autoBlockbusterPathCheckBox_clicked();
  void on_autoSidecarHostCheckBox_clicked();
  void on_setDisplayCheckBox_clicked();
  void on_hostNameField_editingFinished(); 

  // editing checks for dirty profiles etc. 
  bool hostProfileModified(void); 
  // overload: 
  void hostProfileModified(const QString & ) {
    hostProfileModified(); 
  }

 public:
  void trySetProfile (QString name);  
  void saveHistory(QComboBox *box, QString filename);
  void saveAndRefreshHostProfiles(HostProfile *inProfile);
  void removeHostProfiles(void); 
  void sortAndSaveHostProfiles(void); 
  void readAndSortHostProfiles(void) ;
  int readHostProfileFile(QString filename, bool isGlobal);
  void initMovieComboBox(QString initialItem);   

  SideCar *mSidecar; 
  connectionState mState;
  QProcess *mProcess; 
  int32_t mBlockbusterPort; 
  
  vector<HostProfile *> mHostProfiles; 
  HostProfile *mCurrentProfile; 
};

#endif


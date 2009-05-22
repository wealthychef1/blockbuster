/* Copyright (c) 2003 Tungsten Graphics, Inc.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files ("the
 * Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:  The above copyright notice, the Tungsten
 * Graphics splash screen, and this permission notice shall be included
 * in all copies or substantial portions of the Software.  THE SOFTWARE
 * IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT
 * SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <QWidget>
#include <QTcpSocket>
#include <QStringList>
#include "errmsg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "slave.h"
#include "canvas.h"
#include "errmsg.h"
#include "splash.h"
#include "frames.h"
#include "sm.h"
#include "cache.h"
#include <time.h>
#include "util.h"
#include <fstream>
#include "xwindow.h"

#ifdef USE_MPI
#include "mpi.h"

MPI_Comm workers;

#endif

using namespace std; 

QTcpSocket gMasterSocket; 

/* RDC: 
   Here the slave attempts to load some frames from a list of files.  
   return 0 on failure, 1 on success */
static int
LoadFrames(Canvas *canvas, const char *files)
{
  DEBUGMSG("LoadFrames"); 
    FrameList *frames;
    int numFiles = 0;
    const char *start, *lastChar;

    QStringList fileList; 
    lastChar = files + strlen(files);

    start = files;
    while (1) {
        char *end = strchr(start, ' ');
        if (!end)
            end = (char *) lastChar;
        if (end && end > start) {
            /* found a filename between <start> and <end> */
            char save = *end;
            *end = 0;
            /* save this filename */
            fileList.append(start); 
            numFiles++;
            /* restore character at <end> */
            *end = save;
            if (end == lastChar)
                break;
            start = end + 1;
        }
        else {
            break;
        }
    }

    /* debug */
    if (0) {
        int i;
        for (i = 0; i < numFiles; i++) {
          printf("File %d: %s\n", i, fileList[i].toStdString().c_str());
        }
    }

    if (numFiles == 1 &&
        fileList[0] == splashScreenFrameList.getFrame(0)->filename) {
      frames = &splashScreenFrameList;
    }
    else {
      frames = new FrameList; 
      frames->LoadFrames(fileList);
    }
    canvas->SetFrameList(canvas, frames);
	if (!canvas->frameList) {
	  return 0;
	}
	return 1;
}

/*! 
  Provide a standard message format:
*/ 
void SendMessage(QString msg) {
  static string filename = string("/g/g0/rcook/bb_debug-")+GetLocalHostname() + ".txt"; 
  //static ofstream *dbfile = new ofstream(filename.c_str()); 
  QDataStream stream(&gMasterSocket); 
  DEBUGMSG((QString("Slave Sending message: ")+msg));
  //*dbfile << "Sending message: " << msg.toStdString() << endl; 
  stream << msg;
  gMasterSocket.waitForBytesWritten(-1);
  DEBUGMSG("Message sent");
}

/*! 
  Provide a standard error message format:
*/ 
void SendError(QString msg) {
  ERROR(msg); 
  SendMessage(QString("ERROR: ")+msg);  
  return; 
}

/*
  An interface for main() to call to enable the slave to start 
  the slave socket and get a display.  It's necessary to do this early
  because the master sends the DISPLAY and we want it to be set before
  GTK snarfs it up. 
*/
int SlaveInitialize(ProgramOptions *options) {
  SuppressMessageDialogs(); 
  DEBUGMSG("SlaveInitialize() called"); 
  cerr << "useMPI is " << options->useMPI << endl; 
  gMasterSocket.connectToHost(options->masterHost, options->masterPort); 
  if (!gMasterSocket.waitForConnected(5000)) {
    QString err = gMasterSocket.errorString(); 
    cerr << err.toStdString() << endl; 
    ERROR("Connection to master FAILED."); 
    return 0; 
  } else {
    INFO("Connection to master succeeded."); 
    SendMessage(QString("Slave awake")); 
  }
    
  DEBUGMSG("Slave ready for messages, gMasterSocket in state %d", gMasterSocket.state());
  // the next thing that happens is the master sends the display info
  QDataStream masterStream(&gMasterSocket);
  QString message, token; 
  while (1) {
    gCoreApp->processEvents(); 
    if (gMasterSocket.bytesAvailable()) {
      masterStream >> message; 
      QStringList messageList = message.split(" "); 
      token = messageList[0]; 
      DEBUGMSG((QString("Slave got message: \"") + message + "\"")); 
      if (token != "DISPLAY") {
        QString msg("Expected DISPLAY command but got something else!"); 
        ERROR(msg); 
        SendError(msg); 
        return 0; 
      } 
      options->displayName = messageList[1]; 
      DEBUGMSG(QString("set display to %1").arg(messageList[1])); 
      break; 
    } else {
      DEBUGMSG("Waiting for DISPLAY message from server"); 
      usleep (500000); 
    }
  }
  DEBUGMSG("END SlaveInitialize"); 
  return 1; 
}

//=========================================================
// ERRORGRABBER is a debugging tool for slave socket errors... 
ErrorGrabber::~ErrorGrabber(){ return; }
ErrorGrabber::ErrorGrabber(QObject *){ return; }
void ErrorGrabber::SocketStateChanged(QAbstractSocket::SocketState state) {
  DEBUGMSG("Socket state changed to %d", state); 
  return; 
}

void ErrorGrabber::SocketError(QAbstractSocket::SocketError ) {
  DEBUGMSG(QString("Socket error: %1").arg(gMasterSocket.errorString())); 
  return; 
}
//=========================================================
/* Try direct socket manipulation for speed */
bool GetNextMessage(QString &msg) {
  
}

//=========================================================
/*
 * When the movie player is operating in DMX slave mode, we'll take
 * our commands from the master instance of the player, rather than
 * the keyboard/mouse.
 */
int SlaveLoop(ProgramOptions *options)
{
  DEBUGMSG("SlaveLoop (thread %p), gMasterSocket state %d", QThread::currentThread(), gMasterSocket.state()); 
  ErrorGrabber errorGrabber(NULL); 
  QObject::connect(&gMasterSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), 
                   &errorGrabber, SLOT(SocketStateChanged(QAbstractSocket::SocketState ))); 
  QObject::connect(&gMasterSocket, SIGNAL(error(QAbstractSocket::SocketError)), 
                   &errorGrabber, SLOT(SocketError(QAbstractSocket::SocketError ))); 
  int argc = 0;
  double fps = 0, recentEndTime, recentStartTime, elapsedTime;
  long recentFrameCount = 0; 
  int32_t lastImageRendered = -1; 
  gCoreApp = new QApplication(argc, NULL); 
  Canvas *canvas = NULL;
  UserInterface *userInterface = options->userInterface;
  bool idle = false; 
  Rectangle currentRegion; 
  qint32 destX = 0,  destY = 0,  lod = 0;
  float zoom;
  /* contact the master movie player */
  try {
    bool speedTest=false; 
    int32_t  playFrame = 0, playFirstFrame = 0, playLastFrame = 0; 
    int32_t playStep = 0;  // how much to advance the next frame by
	QString message, token;
	QDataStream masterStream(&gMasterSocket);
    time_t lastheartbeat=time(NULL), now; 
    while (1) {
      now = time(NULL); 
      if (now - lastheartbeat > 300) {
        ERROR("It's been more than 5 minutes since the server checked in -- exiting."); 
        exit(1); 
      }
      DEBUGMSG("About to process events. gMasterSocket state is %d", gMasterSocket.state()); 
      /*!
        if (GetNextMessage(message) ) {
        if (message == "disconnected") {
        ERROR("Error:  lost connection to the master server (state is %d).  Exiting.\n", gMasterSocket.state());
        break; 
        }
      */ 
      if (gMasterSocket.state() != QAbstractSocket::ConnectedState) {
        ERROR("Error:  lost connection to the master server (state is %d).  Exiting.\n", gMasterSocket.state());
        break; 
      }
      if (gMasterSocket.bytesAvailable()) {      
        masterStream >> message;         
        
		QStringList messageList = message.split(" "); 
        token = messageList[0]; 
		QString dbgmsg = QString("Slave got message: \"") + message + "\""; 
        DEBUGMSG(dbgmsg); 
		if (token == "Heartbeat") {
		  //SendMessage("pong");
          lastheartbeat = time(NULL); 
        } // end "Heartbeat" 
        else if (token == "SlaveIdle")  {
          if (messageList.size() != 2) {
			SendError("Bad SlaveIdle message: "+message); 
			continue; 
		  }
          idle = (messageList[1] == "true"); 
          // if messageList[1] is "true" then we're idle.  If it's "false",
          // then we're a worker.
#ifdef USE_MPI
          MPI_Comm_split(MPI_COMM_WORLD, idle, 0, &workers);
#endif
        } // end "SlaveIdle"
        else if (token == "SpeedTest") {
          DEBUGMSG("slave got speedTest"); 
          speedTest = true;
          recentStartTime = GetCurrentTime();
          recentFrameCount = 0;
        }
        else if (token == "Exit") {
          ERROR("Slave received Exit signal from server.");
          exit(0);           
        } // end "Exit"
        else if (!idle) {
          if (token == "Render")  {
            if (messageList.size() != 10) {
              SendError("Bad Render message: "+message); 
              continue; 
            }
            //Rectangle src;
            qint32 imageNum = messageList[1].toLong(); 
            currentRegion.x = messageList[2].toLong(); 
            currentRegion.y = messageList[3].toLong(); 
            currentRegion.width = messageList[4].toLong(); 
            currentRegion.height = messageList[5].toLong(); 
            destX = messageList[6].toLong();
            destY = messageList[7].toLong();
            lod = messageList[9].toLong();
            zoom = messageList[8].toFloat();
            
            if (canvas->frameList) {
              canvas->Render(canvas, imageNum, &currentRegion, 
                             destX, destY, zoom, lod);
              lastImageRendered = imageNum; 
            }
            else {
              SendError("No frames to render\n");
              lastImageRendered = -1; 
            }
          }// end "Render"
          else if (token == "DrawString") {
            if (messageList.size() != 3) {
              SendError("Bad DrawString message: "+message); 
              continue; 
            }
            bool rowOK=false, colOK=false; 
            qint32 row = messageList[1].toLong(&rowOK), 
              col = messageList[2].toLong(&colOK);
            if (!rowOK or !colOK) {
              SendError(QString("Bad number in DrawString command")
                        + message); 
              continue; 
            }
            if (! gMasterSocket.bytesAvailable()) {
              INFO("Waiting for incoming data for DrawString"); 
              while (!gMasterSocket.bytesAvailable()) {
                usleep (1000); 
              }
            }
            masterStream >> message; 
            DEBUGMSG((QString("string to draw is: ")+message)); 
            canvas->DrawString(canvas, row, col, message.toAscii());          
          }// "DrawString"
          else if (token == "SwapBuffers") {
#ifdef USE_MPI
            DEBUGMSG("frame %d, canvas %d: MPI_Barrier", lastImageRendered, canvas); 
            /*!
              TOXIC:  disable MPI for right now during testing
            */ 
            //if (0 && useMPI) MPI_Barrier(workers); 
            if (options->useMPI) MPI_Barrier(workers); 
#endif
            if (canvas && canvas->frameList) { 
              // only swap if there is a valid frame  
              DEBUGMSG("frame %d: canvas->SwapBuffers", lastImageRendered); 
              canvas->SwapBuffers(canvas);
            }
#ifdef USE_MPI
            DEBUGMSG("frame %d: Finished MPI_Barrier", lastImageRendered); 
#endif
            /* send ack */
            SendMessage(QString("SwapBuffers complete %1 %2").arg(messageList[1]).arg(messageList[2])); 
          } // end "SwapBuffers"
          else if (token == "Preload") {
            if (messageList.size() != 7) {
              SendError("Bad Preload message: "+message); 
              continue; 
            }
            if (canvas->frameList) {			   
              //Rectangle region;
              qint32 frame;
              bool ok = false; 
              frame=messageList[1].toLong(&ok);
              if (ok) currentRegion.x=messageList[2].toLong(&ok) ;
              if (ok) currentRegion.y=messageList[3].toLong(&ok);
              if (ok) currentRegion.width=messageList[4].toLong(&ok);
              if (ok) currentRegion.height=messageList[5].toLong(&ok);
              if (ok) lod=messageList[6].toLong(&ok); 
              if (!ok) {
                SendError("Bad Preload argument in message: "+message); 
                continue; 
              }
              canvas->Preload(canvas, frame, &currentRegion, lod);
            }
            /* else {
               SendError("Slave has no frames to preload.");
               }
            */ 
          }// end "Preload"
          else if (token == "CreateCanvas") {
            if (messageList.size() != 8) {
              SendError("Bad CreateCanvas message: "+message); 
              continue; 
            }
            QString displayName = messageList[1];
            qint32 parentWin,  w, h, d, cacheFrames, numThreads;
            bool ok = false; 
            MovieEvent junkEvent; 
            parentWin = messageList[2].toLong(&ok); 
            if (ok) w = messageList[3].toLong(&ok); 
            if (ok) h = messageList[4].toLong(&ok);
            if (ok) d = messageList[5].toLong(&ok);
            if (ok) cacheFrames = messageList[6].toLong(&ok); 
            if (ok) numThreads = messageList[7].toLong(&ok);; 
            if (!ok) {
              SendError("Bad CreateCanvas argument in message: "+message); 
              continue; 
            }
            /* Create our canvas.  This canvas will not have any reader
             * threads associated with it, nor any frames in its
             * imageCache.
             */
            ProgramOptions *createOptions= new ProgramOptions;
            createOptions->frameCacheSize = cacheFrames;
            createOptions->readerThreads = numThreads;
            createOptions->fontName = DEFAULT_X_FONT;
            createOptions->geometry.width = w;
            createOptions->geometry.height = h;
            createOptions->displayName = displayName;
            createOptions->suggestedTitle = "Blockbuster Slave";
            canvas = new Canvas(userInterface, options->rendererIndex, 
                                createOptions, parentWin);
            GetXEvent(canvas, 0, &junkEvent); 
            //canvas->GetEvent(canvas, 0, &junkEvent); 
            delete createOptions; 
            
            if (canvas == NULL) {
              ERROR("Could not create a canvas");
              SendError( "Error Could not create a canvas");
              return -1;
            }
         }// end "CreateCanvas"
          else if (token == "DestroyCanvas") {
            // this never did do anything that I remember.  
          } // end "DestroyCanvas"
          else if (token == "MoveResizeCanvas") {
            if (messageList.size() != 5) {
              SendError("Bad MoveResizeCanvas message: "+message); 
              continue; 
            }
            qint32 w, h, x, y;
            bool ok = false; 
            w = messageList[1].toLong(&ok);; 
            if (ok) h = messageList[2].toLong(&ok);; 
            if (ok) x = messageList[3].toLong(&ok);; 
            if (ok) y = messageList[4].toLong(&ok);; 
            if (!ok) {
              SendError("Bad MoveResize argument in message: "+message); 
              continue; 
            }
            
            if (!canvas || !canvas->Resize) {
              SendError("ResizeCanvas requested, but canvas is not ready"); 
            } else {
              canvas->Resize(canvas, w, h, 0);
              canvas->Move(canvas, x, y, 0); // the zero means "force this" 
            }
          }// end "MoveResizeCanvas"
          else if (token == "SetFrameList") {
            /* This is the "file load" part of the code, rather misleading */
            /*we need to destroy image cache/reader threads etc before smBase destructor */
            DestroyImageCache(canvas);
            if (canvas->frameList) {
              canvas->frameList->DeleteFrames(); 
              delete canvas->frameList; 
            }
           message.remove(0, 13); //strip "SetFrameList " from front
            DEBUGMSG((QString("File list is: ")+message)); 
            if (!LoadFrames(canvas, message.toAscii())) {			
              SendError("No frames could be loaded."); 
            }
            playFirstFrame = 0; 
            playLastFrame = canvas->frameList->numStereoFrames()-1; 
          }// end "SetFrameList"
          else if (token == "PlayForward") {
            playStep = 1; 
          }
          else {
            QString msg = QString("Bad message: ")+ message;
            SendError(msg);
            break;
          } // end (bad message)
        } // end if (!idle)
      }  /* end if bytesAvailable */
      else {
        /* no bytes available, be nice to the OS and sleep 2 milliseconds
           unless you are going 100 fps, this should not hurt frame rate...
        */
        gCoreApp->processEvents(); 
        usleep(2*1000); 
      }

      /*!
        NEW CODE:  Behave like DisplayLoop, in that you do not need explicit master control of when to swap the next frame.  Hopefully, this eliminates delays inherent in that model.  
      */ 
      if (playStep) {
        if (canvas && canvas->frameList) {
          canvas->Render(canvas, playFrame, &currentRegion, 
                         destX, destY, zoom, lod);
        }
        lastImageRendered = playFrame; 
        playFrame ++; 
        if (playFrame > playLastFrame) {
          playStep = 0; 
        }
#ifdef USE_MPI
        if (options->useMPI) { 
          DEBUGMSG("frame %d, %d:  MPI_Barrier",lastImageRendered, canvas); 
          MPI_Barrier(workers); 
        }
#endif
        if (canvas && canvas->frameList) { 
          // only swap if there is a valid frame  
          DEBUGMSG( "frame %d: canvas->SwapBuffers\n", lastImageRendered); 
          canvas->SwapBuffers(canvas);
        }
#ifdef USE_MPI
        DEBUGMSG( "frame %d: Finished MPI_Barrier\n", lastImageRendered); 
#endif
      }
      
      /*!
        The below is purely debugging code.  It sets the slaves off and running, rendering as fast as they can and printing frame rates.  This is obviously good for getting baseline for performance, so don't delete this unless you are just sick of it and don't give a crap any longer. It ignores heartbeats and everything, so the master will assume the slaves have died and error out at some point. 
      */ 
      if (speedTest) {
        while (1) {
          
          /* render the next frame and advance the counter */
          if (canvas && canvas->frameList) {
            canvas->Render(canvas, playFrame, &currentRegion, 
                           destX, destY, zoom, lod);
          }
          lastImageRendered = playFrame; 
          playFrame ++; 
          if (playFrame > playLastFrame) {
            playFrame = 0; 
          }
#ifdef USE_MPI
          if (options->useMPI) { 
            DEBUGMSG("speedTest: frame %d, %d:  MPI_Barrier",lastImageRendered, canvas); 
            MPI_Barrier(workers); 
          }
#endif
          if (canvas && canvas->frameList) { 
            // only swap if there is a valid frame  
            DEBUGMSG( "speedTest: frame %d: canvas->SwapBuffers\n", lastImageRendered); 
            canvas->SwapBuffers(canvas);
          }
#ifdef USE_MPI
          DEBUGMSG( "speedTest: frame %d: Finished MPI_Barrier\n", lastImageRendered); 
#endif
          /* send ack */
          // SendMessage(QString("SwapBuffers complete %1 %2").arg(messageList[1]).arg(messageList[2])); 
          /*! 
            Compute frames per second 
          */ 
          recentFrameCount++;
          recentEndTime = GetCurrentTime();
          elapsedTime = recentEndTime - recentStartTime;
          if (elapsedTime >= 1.0) {
            fps = (double) recentFrameCount / elapsedTime;
            DEBUGMSG( "speedTest: Frame Rate: %g\n", fps); 
            /* reset timing info so we compute FPS over last 2 seconds */
            recentStartTime = GetCurrentTime();
            recentFrameCount = 0;
          }
        }
      } 
      /*! 
        End of debugging code for performance baseline
      */
    }/* end while loop */ 
  } catch (...) {
    SendError(QString("Exception in MessageLoop"));
  }
  
  SendMessage(QString("Slave exiting"));
  /* Done with the canvas */
  if (canvas) {
    delete canvas;
  }
  
  return 0;
}

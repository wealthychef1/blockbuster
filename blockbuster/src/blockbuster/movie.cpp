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
#include "qmetatype.h"
#include "QInputDialog"
#include "QDir"
#include "blockbuster_qt.h"
#include "timer.h"
#include "splash.h"
#include "SidecarServer.h"
#include "util.h"
#include "remotecommands.h"
#include "errmsg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/times.h>
#include <sys/types.h>
#include "movie.h"
#include "errmsg.h"
#include "frames.h"
#include "../libpng/pngsimple.h"
#include <libgen.h>
#include "Renderer.h"


// ====================================================================
/*
 * Given a zoom factor, compute appropriate level of detail.
 */
int LODFromZoom(float zoom)
{
  /* XXX this may need tuning */
  int lod = 0;
  while (zoom <= 0.5) {
    zoom *= 2.0;
    lod++;
  }
  return lod;
}

// ====================================================================
static float ComputeZoomToFill(Renderer *renderer, int width, int height)
{
  float xZoom, yZoom, zoom;

  xZoom = (float) renderer->mWidth / width;
  yZoom = (float) renderer->mHeight / height;
  zoom = xZoom < yZoom ? xZoom : yZoom;

  /* The return value is the "integer zoom", a value which
   * matches well how the user interface is handling zooming.
   * It will be 0 for no zooming, >0 if the canvas is bigger
   * than the image and so the image has to be expanded, or
   * <0 if the canvas is smaller than the image and the
   * image has to be shrunk.
   */
  /*RDC -- what is with all these 500's ?  */ 
  // return (int) ((zoom - 1.0) * 500.0);
  return zoom; 
}


// ====================================================================
void ClampStartEndFrames(FrameListPtr allFrames, 
						 int32_t &startFrame, 
						 int32_t &endFrame, 
						 int32_t &frameNumber, 
						 bool warn = false) {
  DEBUGMSG(QString("BEGIN ClampStartEndFrames(%1, %2, %3, %4...").arg(allFrames->getFrame(0)->mFilename.c_str()).arg(startFrame).arg(endFrame).arg(frameNumber)); 
  if (endFrame <= 0) {
    endFrame = allFrames->numStereoFrames()-1;
  }
  if (startFrame < 0) {
    startFrame = 0;
  }
  if (startFrame > static_cast<int32_t>(allFrames->numStereoFrames()) -1){
    if (warn) WARNING("startFrame value of %d is greater than number of frames in movie (%d), clamping to last frame", startFrame, allFrames->numStereoFrames()-1); 
    startFrame = allFrames->numStereoFrames() -1;
  }
  if (endFrame > static_cast<int32_t>(allFrames->numStereoFrames()) -1){
    if (warn) WARNING("endFrame value of %d is greater than number of frames in movie (%d), clamping to last frame", endFrame, allFrames->numStereoFrames()-1); 
    endFrame = allFrames->numStereoFrames() -1;
  }
  if (endFrame < startFrame) {
    // always warn about this: 
    WARNING("endFrame value of %d is less than endFrame value (%d), reversing then", endFrame, startFrame);
    int32_t tmp = endFrame; 
    endFrame = startFrame; 
    startFrame = tmp; 
  }
  if (frameNumber > endFrame) frameNumber = endFrame; 
  if (frameNumber < startFrame) frameNumber = startFrame; 
  DEBUGMSG(QString("END ClampStartEndFrames(%1, %2, %3, %4...").arg(allFrames->getFrame(0)->mFilename.c_str()).arg(startFrame).arg(endFrame).arg(frameNumber)); 
  return;
}

// ====================================================================
/*
 * Main UI / image display loop
 * XXX this should get moved to a new file.
 */
int DisplayLoop(ProgramOptions *options, vector<MovieEvent> script)
{
  int32_t previousFrame = -1, cueEndFrame = 0;
  uint totalFrameCount = 0, recentFrameCount = 0;
  FrameInfoPtr frameInfo;
  Renderer * renderer = NULL;
  int loopCount = options->loopCount; 
  int drawInterface = options->drawInterface;
  int skippedDelayCount = 0, usedDelayCount = 0;
  int done =0;
  /* Timing information */
  struct tms startTime, endTime;
  clock_t startClicks, endClicks;
  long Hertz;
  double recentStartTime = GetCurrentTime(), 
    recentEndTime, elapsedTime, userTime, systemTime;
  double fps = 0.0;
  double nextSwapTime = 0.0, previousSwapTime = 0.0;
  float targetFPS = 0.0;
  FrameListPtr allFrames; 
  double noscreensaverStartTime = GetCurrentTime();


  bool pingpong = false; // play forward, then backward, then forward, etc
  bool cuePlaying = false;  // when true, status will not change. 
  /* UI / events */
  int xOffset = 0, yOffset = 0, oldXOffset = 0, oldYOffset = 0; // image position relative to center of canvas
  int32_t panning = 0, panStartX = 0, panStartY = 0, panDeltaX = 0, panDeltaY = 0;
  int zooming = 0, zoomStartY = 0, zoomDelta = 0;  // integer zoom for mouse
  /*int izoom = 0*/  // for mouse zoom
  float currentZoom = 1.0, newZoom = 1.0, startZoom, oldZoom; // actual zoom factor
  int lod = 0, maxLOD, baseLOD = 0, lodBias = options->LOD;
  long sleepAmt=1; // for incremental backoff during idle
  /* Region Of Interest of the image */
  RectanglePtr roi;
  int destX, destY; // position on canvas to render the image

  
  int32_t playDirection = 0, 
    startFrame= options->startFrame, 
    frameNumber = options->currentFrame, 
    endFrame = options->endFrame,
    preloadFrames = options->preloadFrames; 

  /* We'll need this for timing */
  Hertz = sysconf(_SC_CLK_TCK);

  
  /* Start timing */
  startClicks = times(&startTime);
  recentStartTime = GetCurrentTime();
  
  if ( options->play != 0 )
    playDirection = options->play;
  
  
  if ( options->zoom != 1.0 )
    newZoom =options->zoom;
  
 
  deque<MovieEvent>  events; 

					
  time_t lastheartbeat = time(NULL); 
  MovieSnapshot oldSnapshot; 

  while(! done) {
    // TIMER_PRINT("loop start"); 
    MovieEvent newEvent; 
    int oldPlay = playDirection;

    /* Get an event from the renderer or network and process it.
     */   
 
    gCoreApp->processEvents(); 
    if (gMainWindow->GetEvent(newEvent)) {  
      events.push_back(newEvent); 
    }
    if (renderer) renderer->DMXCheckNetwork();
    if (GetNetworkEvent(&newEvent)) { /* Qt events from e.g. Sidecar */
      events.push_back(newEvent); 
    } 
    newEvent.mEventType = "MOVIE_NONE"; 
    if (renderer) {
      /* every minute or so, tell any DMX slaves we are alive */ 
      if (time(NULL) - lastheartbeat > 60) {
        renderer->DMXSendHeartbeat(); 
        lastheartbeat = time(NULL); 
      }
      renderer->GetXEvent(0, &newEvent); 
      if (playDirection && newEvent.mEventType == "MOVIE_GOTO_FRAME" && 
          newEvent.mNumber == frameNumber+playDirection) {
        newEvent.mEventType = "MOVIE_NONE"; 
      }   
    }
    events.push_back(newEvent); 


    // we now have at least one event, even if it's a 'MOVIE_NONE'
    while (events.size()) {
      bool swapBuffers = false; 
      MovieEvent event = events[0];
      events.pop_front(); 

      bool sendSnapshot = false; 
             
      if (event.mEventType == "MOVIE_NONE") {
        if (script.size()) {
          event = script[0]; 
          script.erase(script.begin()); 
        }
      }
      if (event.mEventType != "MOVIE_NONE") {        
        DEBUGMSG("GOT EVENT ------- %s\n", string(event).c_str()); 
        if (options->mTraceEvents) {
          if (!options->mTraceEventsFile.is_open() ) {
            if (options->mTraceEventsFilename == "") {
              options->mTraceEventsFilename = "events.log"; 
            }
            options->mTraceEventsFile.open(options->mTraceEventsFilename.toStdString().c_str()); 
          }
          if (!options->mTraceEventsFile.is_open() ) {
            dbprintf(0, "WARNMNG: Could not open tracefule %s for writing.\n", options->mTraceEventsFilename.toStdString().c_str());
            options->mTraceEvents = false; 
          } else {
            options->mTraceEventsFile << string(event) << endl; 
            options->mTraceEventsFile.flush(); 
          }        
        }
        if (event.mEventType == "MOVIE_TRACE_EVENTS") {
          if (options->mTraceEventsFile.is_open() ) {
            options->mTraceEventsFile.close(); 
          }
        
          options->mTraceEvents = event.mNumber;
          if (options->mTraceEvents) {
            if (event.mString != "") {
              options->mTraceEventsFilename = event.mString.c_str();           
            }
            options->mTraceEventsFile.open(options->mTraceEventsFilename.toStdString().c_str()); 
          }
        }
        else if (event.mEventType == "MOVIE_OPEN_FILE" ||
            event.mEventType == "MOVIE_OPEN_FILE_NOCHANGE") {         
          DEBUGMSG("Got Open_File command"); 
          QStringList filenames; 
          filenames.append(event.mString.c_str());
          
          allFrames.reset(new FrameList); // this should delete our old FrameList
          if (!allFrames->LoadFrames(filenames)) {	    
            ERROR("Could not open movie file %s", event.mString.c_str()); 
            gSidecarServer->SendEvent(MovieEvent("MOVIE_STOP_ERROR", "No frames found in movie - nothing to display"));
            continue; 
          }
          if (!options->stereoSwitchDisable) { 
            dbprintf(5, "auto-switch stereo based on detected movie type...\n"); 
            if (options->rendererName != "dmx") { 
              dbprintf(5, "No DMX: switch frontend renderer as needed.\n"); 
              if ((allFrames->mStereo && options->rendererName != "gl_stereo")) {
                options->rendererName = "gl_stereo";
              }
              if (!allFrames->mStereo && options->rendererName == "gl_stereo") {
                options->rendererName = "gl";
              }            
            } else { 
              dbprintf(5, "DMX case: switch backend renderer as needed.\n"); 
              if ((allFrames->mStereo && options->backendRendererName != "gl_stereo")) {
                options->backendRendererName = "gl_stereo";
              }
              if (!allFrames->mStereo && options->backendRendererName == "gl_stereo") {
                options->backendRendererName = "gl";
              }
            }
            if (options->rendererName == "") {
              options->rendererName = "gl";
            }
          } 
          dbprintf(5, "Renderer name: %s\n", options->rendererName.toStdString().c_str()); 
          int width, height, depth; 
          allFrames->GetInfo(width, height, depth, maxLOD, targetFPS);
          if (options->frameRate != 0.0) {
            targetFPS = options->frameRate; 
            options->frameRate = 0; 
          }
          /*  if (options->geometry.width != DONT_CARE) {
              width = options->geometry.width; 
              height = options->geometry.height; 
              }
          */ 
          startFrame = 0; 
          endFrame = allFrames->numStereoFrames()-1; 
          frameNumber = event.mNumber;  
          options->currentFrame = frameNumber;         
          
          /* Reset our various parameters regarding positions */
          xOffset = yOffset = 0; 
          panStartX = panStartY = 0; 
          panDeltaX = panDeltaY = 0;
          zoomStartY = 0; zoomDelta = 0;
          playDirection = 0;
          panning = 0; 
          
          if ( options->play != 0 ) {
            playDirection = options->play;
            options->play = 0;
          }

          options->geometry.width = width; 
          options->geometry.height = height; 
        
          if (options->decorations) {
            options->decorations = !options->fullScreen; 
          }
          if (renderer) {
            if (allFrames->mStereo != (renderer->mName == "gl_stereo")) {
              DEBUGMSG("toggle stereo automatically\n");           
              delete renderer; 
              renderer = NULL; 
            }
            else if (renderer && options->fullScreen != renderer->mFullScreen) {
              DEBUGMSG("toggle to or from fullscreen mode\n");           
              delete renderer; 
              renderer = NULL; 
            }
          }
          if (!renderer) {
            renderer = Renderer::CreateRenderer(options, 0, gMainWindow);
            if (!renderer) {
              ERROR("Could not create a renderer");
              return 1;
            }
          } 
          if (options->zoomToFill || event.mEventType != "MOVIE_OPEN_FILE_NOCHANGE") {
            /* Compute a Shrink to Fit zoom */
            newZoom = ComputeZoomToFill(renderer, width, height);
            if (newZoom > 1.0) {
              newZoom = 1.0; /* don't need expanding initially */
            }
          }
          if (event.mEventType != "MOVIE_OPEN_FILE_NOCHANGE") {
            if (options->LOD) {
              lodBias = options->LOD;
            } else {
              lodBias = 0;
            }
            //events.push_front(MovieEvent("MOVIE_MOVE_RESIZE", 0,0,0,0)); 
          } 
          renderer->DestroyImageCache();        
          renderer->SetFrameList(allFrames);
          renderer->ReportFrameListChange(allFrames);
          renderer->ReportRateChange(targetFPS); 
          renderer->ReportFrameChange(frameNumber);
          renderer->ReportDetailRangeChange(-maxLOD, maxLOD);
          renderer->ReportZoomChange(newZoom);
          swapBuffers = true; 
          previousSwapTime = 0.0; 
          frameInfo =  renderer->GetFrameInfoPtr(0);
          preloadFrames = MIN2(options->preloadFrames, static_cast<int32_t>(allFrames->numStereoFrames()));
        }  // END event.mEventType == "MOVIE_OPEN_FILE") || event.mEventType == "MOVIE_OPEN_FILE_NOCHANGE" 
    
        else if (event.mEventType == "MOVIE_SET_STEREO") {
        
          QString rendererName = "gl", saved; 
          if (event.mNumber) {// TURN ON STEREO
            dbprintf(1, "TURN ON STEREO\n"); 
            rendererName = "gl_stereo";
          } else {
            dbprintf(1, "TURN OFF STEREO\n"); 
          }          
          
          if (options->rendererName == "dmx") {
            saved = options->backendRendererName; 
            options->backendRendererName = rendererName;
          } else {
            saved = options->rendererName;
            options->rendererName = rendererName; 
          } 
          
          if (saved != rendererName) {
            dbprintf(1, "Changed Renderer type\n"); 
            delete renderer; 
            renderer = Renderer::CreateRenderer(options, 0, gMainWindow);
            if (!renderer) {
              ERROR("Could not create a renderer");
              return 1;
            }
            
             renderer->SetFrameList(allFrames);
            renderer->ReportFrameListChange(allFrames);
            renderer->ReportRateChange(targetFPS); 
          }        
        }
        else if (event.mEventType == "MOVIE_DISABLE_DIALOGS") {   
          SuppressMessageDialogs(true); 
          continue;
        }
        else if (event.mEventType == "MOVIE_SIDECAR_STATUS" ||
                 event.mEventType == "MOVIE_SNAPSHOT" ||   
                 event.mEventType == "MOVIE_SNAPSHOT_STARTFRAME" ||   
                 event.mEventType == "MOVIE_SNAPSHOT_ENDFRAME" ||   
                 event.mEventType == "MOVIE_SNAPSHOT_ALT_ENDFRAME") {   
          sendSnapshot = true; 
        }
        else if (event.mEventType == "MOVIE_SIDECAR_BACKCHANNEL") {       
          gSidecarServer->connectToSidecar(event.mString.c_str()); 
        }
        else if (event.mEventType == "MOVIE_CUE_PLAY_ON_LOAD") { 
          playDirection = event.mHeight; 
        }
        else if (event.mEventType == "MOVIE_CUE_BEGIN") { 
          cueEndFrame = event.mNumber;  // for now, a cue will be defined to be "executing" until the end frame is reached, then the cue is complete
          if (renderer) renderer->reportMovieCueStart(); 
        }
        else if (event.mEventType == "MOVIE_CUE_END") { 
          /* This event is just the end of the cue data stream, and is
             not related to when the cue is done executing.  
             In fact, the cue is not really playing until now.  
          */ 
          cuePlaying = true;
        }
        /* if (event.mEventType == "MOVIE_CUE_MOVIE_NAME" || 
           event.mEventType == "MOVIE_CUE_PLAY_BACKWARD") { 
           break; */
        else if (event.mEventType == "MOVIE_PWD") {
          QString pwd =  ParentDir(QString(frameInfo->mFilename.c_str()));
          gSidecarServer->SendEvent(MovieEvent("MOVIE_PWD", pwd)); 
        }
        else if (event.mEventType == "MOVIE_START_END_FRAMES") {
          /*  Store this new event as the user's new global preference */
          options->startFrame = event.mWidth;
          options->endFrame = event.mHeight; 
          // apply the start and end frames to the current movie
          // clamp frame values, generate a warning if they are funky
          ClampStartEndFrames(allFrames, options->startFrame, options->endFrame, frameNumber, true); 
          startFrame = options->startFrame;
          endFrame = options->endFrame;
          if (renderer) renderer->ReportFrameChange(frameNumber); 
          DEBUGMSG("START_END_FRAMES: start %d end %d current %d\n", startFrame, endFrame, frameNumber); 
        }
        else if (event.mEventType == "MOVIE_LOG_TO_FILE") {
          DEBUGMSG("MOVIE_LOG_TO_FILE event: %s\n", event.mString.c_str()); 
          enableLogging(true, event.mString.c_str());   
        }
        /* if (event.mEventType == "MOVIE_MESSAGE") {
           DEBUGMSG("MOVIE_MESSAGE event: %s\n", event.mString.c_str()); 
           }
        */ 
        else if (event.mEventType == "MOVIE_QUIT") {
          done = 1;
        }
        else if (event.mEventType == "MOVIE_IMAGE_MOVE") {
          xOffset = -event.mX; 
          yOffset = event.mY; 
        }
        else if (event.mEventType == "MOVIE_FULLSCREEN") {
          if (event.mNumber == 2) {
            options->fullScreen = !options->fullScreen; 
          } else {
            options->fullScreen = event.mNumber; 
          }
          if(options->fullScreen) { 
            if (renderer && options->zoomToFill) {
              newZoom = ComputeZoomToFill(renderer, renderer->mWidth,
                                         renderer->mHeight);
              renderer->Move(0,0, 0);            
            } 
          } else {
            options->decorations = true; 
          }
          if (renderer) {
            renderer->SetFullScreen(options->fullScreen); 
          }
        }
        else if (event.mEventType == "MOVIE_MOVE" ||
                 event.mEventType == "MOVIE_RESIZE" ||
                 event.mEventType == "MOVIE_MOVE_RESIZE") {
          // The code below moves, resizes or does both as appropriate.  Doing it this way was easier than making two new functions, so sue me.  
          // =====================================
          // MOVE ===========================
          if (event.mEventType == "MOVIE_MOVE" || event.mEventType == "MOVIE_MOVE_RESIZE") {
            //DEBUGMSG("MOVE"); 
            if (event.mX == -1) 
              event.mX = renderer->mXPos;
            if (event.mY == -1) 
              event.mY = renderer->mYPos;
            renderer->Move(event.mX, event.mY, event.mNumber);            
          }
          // END MOVE ===========================
        
          // ====================================
        
          // RESIZE ===========================
          if (event.mEventType == "MOVIE_RESIZE" || event.mEventType == "MOVIE_MOVE_RESIZE") {
            //DEBUGMSG("RESIZE"); 
            if (event.mWidth == 0) {
              /* if there are frames, use the frame width for the window width 
                 else use the current width */ 
              if(frameInfo) {
                event.mWidth = frameInfo->mWidth;
              } else {
                if (renderer) {
                  event.mWidth = renderer->mWidth;
                } 
              }
            }         
            if (event.mHeight == 0)  {
              /* if there are frames, use the frame height for the window height 
                 else use the current height */ 
              if(frameInfo) {
                event.mHeight = frameInfo->mHeight;
              } else {
                if (renderer) {
                  event.mHeight = renderer->mHeight;
                }
              }
            }
            if (event.mWidth > renderer->mScreenWidth) {
              event.mWidth = renderer->mScreenWidth; 
            }
            if (event.mHeight > renderer->mScreenHeight) {
              event.mHeight = renderer->mScreenHeight; 
            }
            if (event.mWidth && event.mHeight) {
              renderer->Resize(event.mWidth,
                               event.mHeight, event.mNumber);
              /* Set these in case there is no Resize function */
              renderer->mWidth = event.mWidth;
              renderer->mHeight = event.mHeight;
  
              if (options->zoomToFill/* && !event.mNumber*/) {
                goto MOVIE_ZOOM_TO_FIT;
              }
            }
          }
          // END RESIZE ===========================
          swapBuffers = true; 
        } // END "MOVIE_MOVE", "MOVIE_RESIZE" OR "MOVIE_MOVE_RESIZE"
        
        /*  if (event.mEventType == "MOVIE_EXPOSE") {
            break;

            if (event.mEventType == "MOVIE_NONE") {
            break; 
        */ 
        else if (event.mEventType == "MOVIE_SET_PINGPONG") {
          pingpong = event.mNumber;
          if (renderer) renderer->ReportPingPongBehaviorChange(event.mNumber); 
        }
        else if (event.mEventType == "MOVIE_SET_LOOP") {
          loopCount = event.mNumber;    
          if (loopCount == 1) loopCount = 2; // this loops once. 
          else if (loopCount == 0) loopCount = 1; // this plays once, no loop
          options->loopCount = loopCount; 
          if (loopCount) {
            pingpong = false;
          }
          if (renderer) renderer->ReportLoopBehaviorChange(loopCount); 
        }
        else if (event.mEventType == "MOVIE_PLAY_FORWARD") { 
          playDirection = 1; loopCount = options->loopCount; 
        }
        else if (event.mEventType == "MOVIE_PLAY_BACKWARD") { 
          playDirection = -1; loopCount = options->loopCount; 
        }
        else if (event.mEventType == "MOVIE_STOP") { 
          DEBUGMSG("Got stop event"); 
          playDirection = 0; 
        }
        else if (event.mEventType == "MOVIE_STOP_ERROR") { 
          DEBUGMSG("Got stop error"); 
          playDirection = 0; 
          gSidecarServer->SendEvent(MovieEvent("MOVIE_STOP_ERROR", event.mString.c_str()));
        }
        else  if (event.mEventType == "MOVIE_PAUSE") { 
          playDirection = !playDirection;
        }
        else if (event.mEventType == "MOVIE_STEP_FORWARD") { 
          frameNumber++;
        }
        else if (event.mEventType == "MOVIE_STEP_BACKWARD") { 
          frameNumber--;
        }
        else if (event.mEventType == "MOVIE_SKIP_FORWARD") { 
          frameNumber += 20; 
        }
        else if (event.mEventType == "MOVIE_SKIP_BACKWARD") { 
          frameNumber -= 20; 
        }
        else if (event.mEventType == "MOVIE_SECTION_FORWARD") { 
          frameNumber += MAX2(allFrames->numStereoFrames() / 4, 1);
        }
        else if (event.mEventType == "MOVIE_SECTION_BACKWARD") {
          frameNumber -= MAX2(allFrames->numStereoFrames() / 4, 1);
        }
        
        else if (event.mEventType == "MOVIE_GOTO_START") {
          frameNumber = startFrame;
        }
        
        else if (event.mEventType == "MOVIE_GOTO_END") {
          frameNumber = endFrame;
        }
        else if (event.mEventType == "MOVIE_GOTO_FRAME") {
          frameNumber = event.mNumber;
          if (frameNumber < 0) {
            /* special meaning to SideCar: go to the last frame */
            frameNumber = endFrame;
          } 
          if (frameNumber < startFrame) {
            frameNumber = startFrame; 
          }
        }
        
        else if (event.mEventType == "MOVIE_ZOOM_TO_FIT") { 
        MOVIE_ZOOM_TO_FIT: 
          if(frameInfo && renderer) {
            newZoom = ComputeZoomToFill(renderer, frameInfo->mWidth,
                                       frameInfo->mHeight);
            DEBUGMSG("Zoom to Fit: %f", newZoom);
          } else if (allFrames) {
            // Caution:  RDC: Zooming was not working right upon movie startup, so here I'm reusing some old code that was commented out -- maybe assumption that allFrames->frames[0] is not NULL is not valid?  
            //bb_assert (allFrames->frames[0] != NULL); 
            newZoom = ComputeZoomToFill(renderer,
                                       allFrames->getFrame(0)->mWidth,
                                       allFrames->getFrame(0)->mHeight);
          }
          xOffset = yOffset = 0;
          options->zoomToFill = true; 
        }
        else if (event.mEventType == "MOVIE_ZOOM_ONE") {
          options->zoomToFill = false; 
          newZoom = 1.0;
          zooming = 0;       
        }
        else if (event.mEventType == "MOVIE_ZOOM_SET") {
          newZoom = event.mRate;
          if (newZoom <= 0.0) {
            options->zoomToFill = true; 
            newZoom = ComputeZoomToFill(renderer,
                                       allFrames->getFrame(0)->mWidth,
                                       allFrames->getFrame(0)->mHeight);  
          }
        }
        else if (event.mEventType == "MOVIE_ZOOM_UP") {
          options->zoomToFill = false; 
          newZoom = 1.2*currentZoom;
          zooming = 0;
        }
        else if (event.mEventType == "MOVIE_ZOOM_DOWN") {
         options->zoomToFill = false; 
          newZoom = 0.8*currentZoom;
          if (newZoom < 0.05) newZoom = 0.05; 
          zooming = 0;
        }
        
        else if (event.mEventType == "MOVIE_KEYBOARD_HELP") {
          PrintKeyboardControls();
        } 
        else if (event.mEventType == "MOVIE_CENTER") {
          xOffset = yOffset = 0;
          panning = 0;
        }
        else if (event.mEventType == "MOVIE_TOGGLE_CURSOR") {
          renderer->ToggleCursor(); 
        } 
        else if (event.mEventType == "MOVIE_NOSCREENSAVER") {
          options->noscreensaver = event.mNumber;
          noscreensaverStartTime = GetCurrentTime();
        } 
        else if (event.mEventType == "MOVIE_MOUSE_PRESS_1") {
          panning = 1;
          panStartX = event.mX;
          panStartY = event.mY;
          panDeltaX = 0;
          panDeltaY = 0;
        }
        else if (event.mEventType == "MOVIE_MOUSE_RELEASE_1") {
          if (panning){ // avoid spurious signals
            panDeltaX = static_cast<int>((panStartX - event.mX) / currentZoom);
            panDeltaY = static_cast<int>((panStartY - event.mY) / currentZoom);
            xOffset += panDeltaX;
            yOffset += panDeltaY;
            panDeltaX = 0;
            panDeltaY = 0;
            panning = 0;
          }
        }
        else if (event.mEventType == "MOVIE_MOUSE_PRESS_2") {
          zoomStartY = event.mY;
          startZoom = currentZoom; 
          zooming = 1;
        }
        else if (event.mEventType == "MOVIE_MOUSE_RELEASE_2") {
          //newZoom += zoomDelta;
          newZoom = startZoom*(1+(float)zoomDelta/(renderer->mHeight));
          zoomDelta = 0;
          zooming = 0;
        }
        else if (event.mEventType == "MOVIE_MOUSE_MOVE") {
          if (panning) {
            panDeltaX = static_cast<int>((panStartX - event.mX) / currentZoom);
            panDeltaY = static_cast<int>((panStartY - event.mY) / currentZoom);
          }
          else if (zooming) {          
            zoomDelta = zoomStartY - event.mY;
            newZoom = startZoom*(1+(float)zoomDelta/(renderer->mHeight));
          }
        }
        else if (event.mEventType == "MOVIE_INCREASE_RATE") {
          targetFPS += 1.0;
          if (renderer) renderer->ReportRateChange(targetFPS);
        
        }
        else if (event.mEventType == "MOVIE_DECREASE_RATE") {
          targetFPS -= 1.0;
          if (targetFPS < 0.0) {
            targetFPS = 0.5;
          }
          if (renderer) renderer->ReportRateChange(targetFPS);
          if (targetFPS < 0.2) {
            targetFPS = 0.2; 
            if (renderer) renderer->ReportRateChange(targetFPS);
          }
        }
        else if (event.mEventType == "MOVIE_SET_RATE") {
          /* User changed the frame rate slider */
          targetFPS = MAX2(event.mRate, 0.2);
          if (renderer) renderer->ReportRateChange(targetFPS);
        }
        else if (event.mEventType == "MOVIE_INCREASE_LOD") {
          lodBias++;
          if (lodBias > maxLOD) lodBias = maxLOD; 
          if (renderer) renderer->ReportDetailChange(lodBias);
        }
        else if (event.mEventType == "MOVIE_DECREASE_LOD") {
          lodBias--;
          if (lodBias < 0) lodBias = 0; 
          if (renderer) renderer->ReportDetailChange(lodBias);
        }
        else if (event.mEventType == "MOVIE_SET_LOD") {
          lodBias = event.mNumber;
          if (renderer) renderer->ReportDetailChange(lodBias);
        }
        /* case "MOVIE_MOUSE_PRESS_"3:
           break;*/ 
        else if (event.mEventType == "MOVIE_SHOW_INTERFACE") {
          drawInterface = true; 
          renderer->ShowInterface(true);
        }
        else if (event.mEventType == "MOVIE_HIDE_INTERFACE") {
          drawInterface = false; 
          if (renderer) renderer->ShowInterface(false);        
        }
        else if (event.mEventType == "MOVIE_MOUSE_RELEASE_3" || 
            event.mEventType == "MOVIE_TOGGLE_INTERFACE") {
          if (!gMainWindow->isVisible()) {
            gMainWindow->show(); 
            gMainWindow->raise(); 
            gCoreApp->processEvents(); 
            gMainWindow->activateWindow(); 
          } else {
            gMainWindow->hide(); 
            gMainWindow->setVisible(false); 
          }
        }
       
        else if (event.mEventType == "MOVIE_SAVE_IMAGE") {
          renderer->WriteImageToFile(frameNumber);
        }
        else if (event.mEventType == "MOVIE_SAVE_FRAME") {
          if (frameInfo ) {
            QString filename = event.mString.c_str(); 
            bool ok = true; 
            if (filename == "") {          
              filename =  QInputDialog::
                getText(NULL, "Filename",
                        "Please give the image a name.",
                        QLineEdit::Normal,
                        ParentDir(QString(frameInfo->mFilename.c_str())),
                        &ok);
            }
           
            if (ok && !filename.isEmpty()) {
              if (! filename.endsWith(".png")) {
                filename += ".png"; 
              }
              Rectangle region; 
              region.x = region.y = 0; 
              region.width = frameInfo->mWidth; 
              region.height = frameInfo->mHeight; 
              ImagePtr image = renderer->GetImage(frameNumber,&region,0); 
              int size[3] = {region.width, region.height, 3}; 
              int result = 
                write_png_file(filename.toAscii().data(), 
                               (unsigned char*)image->Data(), 
                               size);
              if (result == -1) {
                ERROR("Could not write png file %s.", filename.toAscii().data()); 
              } else {
                WARNING("Successfully wrote png file %s.", filename.toAscii().data()); 
              }
            }
          }
        
        } // END event.mEventType == "MOVIE_SAVE_FRAME"
        else {
          cerr << "Warning: unknown event: " << event.mEventType << endl; 
        }
      }

      //===============================================================
      /* END SWITCH on type of event */
      //===================================================================

      if (renderer && options->noscreensaver) {
        // Generate mouse click to defeat the screen saver
        double currentTime = GetCurrentTime();
        //dbprintf(3, "noscreensaverStartTime = %f, currentTime = %f\n", 
        //        noscreensaverStartTime, currentTime); 
        if (currentTime - noscreensaverStartTime > 90) {
          dbprintf(1, "Generating false mouse click to defeat screen saver.\n"); 
          renderer->fakeMouseClick(); 
          noscreensaverStartTime = currentTime;
        }
      }
      
      //TIMER_PRINT("end switch, frame %d", frameNumber); 
      if (frameNumber != previousFrame) {
        DEBUGMSG("frameNumber changed  to %d after switch", frameNumber); 
      }
      /*! check if we have reached the end of a cue */
      if (cuePlaying && 
          (!playDirection  || 
           (!loopCount && playDirection > 0 && cueEndFrame != -1 && frameNumber > cueEndFrame) || 
           (!loopCount && playDirection < 0 && cueEndFrame != -1 && frameNumber < cueEndFrame)) ) {
        dbprintf(2, QString("Ending cue with playDirection=%1, cueEnd=%2, frameNumber=%3\n").arg(playDirection).arg(cueEndFrame).arg(frameNumber)); 
        cuePlaying = false; 
        if (renderer) renderer->reportMovieCueComplete();
        gSidecarServer->SendEvent(MovieEvent("MOVIE_CUE_COMPLETE")); 
      }
      
      if (event.mEventType == "MOVIE_NONE" && !playDirection) {
        // We have no useful work that we are doing, sleep a bit.
        usleep(sleepAmt); 
        if (sleepAmt < 100*1000)
          sleepAmt *= 2; // sleep more next time
        // dbprintf(0, "sleeping %ld\n", sleepAmt);       
        //dbprintf(5, " start back up at outer loop\n"); 
        continue;
      }
      else if (event.mEventType == "MOVIE_QUIT") {
        dbprintf(5, "break from the outer loop too\n"); 
        break;
      }
      else {
        // We are doing something useful -- don't sleep; allow framerate to climb. 
        // dbprintf(0, "Not sleeping because swapBuffers=%d, eventType=%d, frameNumber=%d, previousFrame=%d\n", swapBuffers, event.mEventType, frameNumber, previousFrame);
        sleepAmt = 1; 
      }
  

      if (!oldPlay && playDirection) {
        /* We're starting playback now so reset counters and timers */
        recentStartTime = GetCurrentTime();
        recentFrameCount = 0;
        if (targetFPS > 0.0)
          nextSwapTime = GetCurrentTime() + 1.0 / targetFPS;
        else
          nextSwapTime = 0.0;
      }
      
      //=====================================================================
      // The frame number manipulations above may have advanced
      // or recessed the frame number beyond bounds.  We do the right thing here.
      if (frameNumber > endFrame) {    
        if (playDirection > 0) { // we're playing forward and reached the end of movie
          if (pingpong) {
            playDirection = -playDirection; 
            frameNumber = endFrame; 
          } 
          else {
            if (loopCount>0) {
              loopCount --; 
              if (renderer) renderer->ReportLoopBehaviorChange(loopCount);               
            }
            if (loopCount) {
              frameNumber = startFrame; 
            } else {
              frameNumber = endFrame; 
              playDirection = 0; 
            } 
          }// if not pingpong
        } else { // we are stopped, or are playing backward, either way, it's not the end of a loop, so just fix the issue, which is probably that the user asked to skip beyond the end of the movie
          frameNumber = endFrame;
        }
      }    
      if (frameNumber < startFrame) {
        if (playDirection < 0) { // we're playing backward and reached the end of movie
          if (pingpong) {
            playDirection = -playDirection; 
            frameNumber = startFrame; 
          } 
          else {
            if (loopCount > 0) {
              loopCount --; 
              if (renderer) renderer->ReportLoopBehaviorChange(loopCount); 
            }
            if (loopCount) {
              frameNumber = endFrame; 
            } else { // time to stop, just stick at the end and don't render
              frameNumber = startFrame; 
              playDirection = 0; 
            } 
          }// end ! pingpong
        } else { // we are stopped, or are playing forward, either way, it's not the end of a loop, so just fix the issue, which is probably that the user asked to skip beyond the start of the movie
          frameNumber = startFrame;
        }
      }
      //=====================================================================
      
      if (allFrames && renderer) {
        /* frameInfo = allFrames->frames[frameNumber];  */
        frameInfo =  renderer->GetFrameInfoPtr(frameNumber); /* wrapper for stereo support */
        
        if (currentZoom != newZoom) {
          currentZoom = newZoom;
          if (renderer) renderer->ReportZoomChange(currentZoom);
        }      
      
        string filename("none"); 
        
        if (static_cast<int32_t>(allFrames->numStereoFrames()) > frameNumber) {
          filename = allFrames->getFrame(frameNumber)->mFilename; 
        }
        int loopmsg = 0, imageHeight=0, imageWidth = 0;
        if (loopCount < 0) loopmsg = -1;
        if (loopCount > 1) loopmsg = 1;
        if (loopCount == 1 || loopCount == 0) loopmsg = 0; 
        if (allFrames->numStereoFrames()) {
          imageHeight = allFrames->getFrame(0)->mHeight; 
          imageWidth = allFrames->getFrame(0)->mWidth;
        }
        
        { 
          MovieSnapshot newSnapshot(event.mEventType, filename, fps, targetFPS, currentZoom, lodBias, renderer->mName == "gl_stereo", playDirection, startFrame, endFrame, allFrames->numStereoFrames(), frameNumber, loopmsg, pingpong, options->fullScreen, options->zoomToFill, options->noscreensaver, renderer->mHeight, renderer->mWidth, renderer->mXPos, renderer->mYPos, imageHeight, imageWidth, -xOffset, yOffset); 

          if (sendSnapshot || newSnapshot != oldSnapshot) {
            
            if (renderer) {
              renderer->reportWindowMoved(renderer->mXPos, renderer->mYPos); 
              renderer->reportWindowResize(renderer->mWidth, renderer->mHeight); 
              renderer->reportMovieMoved(xOffset, yOffset); 
              renderer->reportMovieFrameSize(imageWidth, imageHeight); 
              renderer->reportMovieDisplayedSize
                (static_cast<int>(newZoom*imageWidth), 
                 static_cast<int>(newZoom*imageHeight)); 
            }
            dbprintf(5, str(boost::format("Sending snapshot %1%\n") 
                            % string(newSnapshot))); 
            gSidecarServer->SendEvent
              (MovieEvent("MOVIE_SIDECAR_STATUS", string(newSnapshot).c_str())); 
            oldSnapshot = newSnapshot; 
          }
        }
        /*
         * Compute ROI: region of the image that's visible in the window 
         */
        {
          const int imgWidth = frameInfo->mWidth;
          const int imgHeight = frameInfo->mHeight;
          const int winWidth = renderer->mWidth;
          const int winHeight = renderer->mHeight;
          int x, y;
          int imgLeft, imgRight, imgBottom, imgTop;
          
          /* (x,y) = image coordinate at center of window (renderer) */
          x = (imgWidth / 2) + (xOffset + panDeltaX);
          y = (imgHeight / 2) + (yOffset + panDeltaY);
          
          /* Compute image coordinates which correspond to the left,
           * right, top and bottom edges of the window.
           */
          imgLeft = static_cast<int>(x - (winWidth / 2) / currentZoom);
          imgRight = static_cast<int>(x + (winWidth / 2) / currentZoom);
          imgTop = static_cast<int>(y - (winHeight / 2) / currentZoom);
          imgBottom = static_cast<int>(y + (winHeight / 2) / currentZoom);
          
          /* Compute region of the image that's visible in the window and
           * its position destX,destY relative to upper-left corner of window.
           */
          /* X axis */
          qint32 region_x,region_y,region_width,region_height; 
          region_width = imgRight - imgLeft;
          region_x = imgLeft;
          if (region_x < 0) { // left edge of the image is off screen
            /* clip left */
            region_x = 0;
            region_width += region_x; // this is odd. 
          }
          if (region_x + region_width > imgWidth) {
            /* clip right */
            region_width = imgWidth - region_x;
          }
          if (region_width < 0) {
            /* check for null image */
            region_x = region_width = 0;
          }
          
          // placement of image on renderer:  destX
          destX = static_cast<int>(-imgLeft * currentZoom);
          if (destX < 0) {
            /* left clip */
            destX = 0;
          }
          if (destX + region_width * currentZoom > winWidth) {
            /* right clip */
            region_width = static_cast<int>((winWidth - destX) / currentZoom);
            if (region_width < 0)
              region_width = 0;
          }
          
          /* Y axis */
          region_height = imgBottom - imgTop;
          region_y = imgTop;
          if (region_y < 0) {
            /* clip top */
            region_y = 0;
            region_height += region_y; // huh?  it's 0! 
          }
          if (region_y + region_height > imgHeight) {
            /* clip bottom */
            region_height = imgHeight - region_y;
          }
          if (region_height < 0) {
            /* check for null image */
            region_y = region_height = 0;
          }
          // placement of image on renderer:  destY
          destY = static_cast<int>(-imgTop * currentZoom);
          if (destY < 0) {
            /* top clip */
            destY = 0;
          }
          if (destY + region_height * currentZoom > winHeight) {
            /* bottom clip */
            region_height = static_cast<int>((winHeight - destY) / currentZoom);
            if (region_height < 0)
              region_height = 0;
          }
          
          if (!roi || region_y != roi->y || region_x != roi->x || 
              region_height != roi->height || region_width != roi->width) {
            roi.reset(new Rectangle(region_x,region_y,region_width,region_height)); 
          }
          /*
            printf("roi: %d, %d  %d x %d  dest: %d, %d\n",
            roi.x, roi.y, roi.width, roi.height, destX, destY);
          */
        }
        
        if (lodBias < 0) {
          lodBias = 0; 
        }        
        if ((uint32_t) lodBias > frameInfo->mLOD) {
          lodBias = maxLOD; 
        }        
        if (renderer) renderer->ReportDetailChange(lodBias);
        
        if (options->noAutoRes) {
          baseLOD = 0; 
        } else {
          baseLOD = LODFromZoom(currentZoom);
        }
        lod = baseLOD > lodBias? baseLOD: lodBias;
        if ((uint32_t)lod > frameInfo->mLOD) {
          lod = frameInfo->mLOD;
        }
        /* Call the renderer to render the desired area of the frame.
         * Most rendereres will refer to their own image caches to load
         * the image and render it.  Some will just send the
         * request "downstream".
         *
         * If we're paused or stopped, render the frame at maximum
         * level of detail, regardless of what was requested during
         * playback.  If we're playing forward or backward, then use
         * the given level of detail.
         */
        
        TIMER_PRINT("before render"); 
        renderer->Render(frameNumber, previousFrame, 
                         preloadFrames, playDirection, 
                         startFrame, endFrame, roi, destX, destY, 
                         currentZoom, lod);
        
        TIMER_PRINT("after render"); 
        
        /* Print info in upper-left corner */
        /*!
          RDC Note:  this is only done in x11 interface mode, which is going away.  But it might be useful to have a "draw letters on screen" option.  Note that it used OpenGL to do the actual lettering. 
        */ 
        /*     if (drawInterface && renderer->DrawString != NULL) {
               char str[100];
               int row = 0;
               sprintf(str, "Frame %d of %d", frameNumber + 1, allFrames->numStereoFrames());
               renderer->DrawString(row++, 0, str);
               sprintf(str, "Frame Size: %d by %d pixels",
               frameInfo->mWidth, frameInfo->mHeight);
               renderer->DrawString(row++, 0, str);
               sprintf(str, "Position: %d, %d",
               -(xOffset + panDeltaX), yOffset + panDeltaY);
               renderer->DrawString(row++, 0, str);
               sprintf(str, "Zoom: %5.2f  LOD: %d (%d + %d)", currentZoom, lod, baseLOD, lodBias);
             
               renderer->DrawString(row++, 0, str);
               sprintf(str, "FPS: %5.1f (target %.1f)", fps, targetFPS);
               renderer->DrawString(row++, 0, str);
               }
        */
 
        if (playDirection) {
          /* See if we need to introduce a pause to prevent exceeding
           * the target frame rate.
           */
          double delay = nextSwapTime - GetCurrentTime();
          if (delay - previousSwapTime > 0.0) {
            usedDelayCount++;
            //delay *= 0.95;  /* an empirical constant */
            usleep((unsigned long) ((delay - previousSwapTime) * 1000.0 * 1000.0));
          }
          else {
            skippedDelayCount++;
          }
          /* Compute next targetted swap time */
          nextSwapTime = GetCurrentTime() + 1.0 / targetFPS;
        }
        TIMER_PRINT("Swap buffers\n"); 
        renderer->mPreloadFrames = preloadFrames; 
        renderer->mPlayDirection = playDirection;
        renderer->mStartFrame = startFrame; 
        renderer->mEndFrame = endFrame; 
        double beforeSwap = GetCurrentTime(); 
        renderer->SwapBuffers();
        previousSwapTime = GetCurrentTime() - beforeSwap; 

        if (playDirection && options->speedTest) {
          cerr << "requesting speedTest of slaves" << endl; 
          renderer->DMXSpeedTest(); 
          playDirection = 0; 
        }
    
        if (frameNumber != previousFrame) {
          if (renderer) renderer->ReportFrameChange(frameNumber);
        }
        previousFrame = frameNumber; 
        oldZoom = currentZoom; 
        oldXOffset = xOffset;
        oldYOffset = yOffset;

        TIMER_PRINT("after swap"); 
        /* Advance to the next frame */
        if (playDirection) {
          /* Compute next frame number (+/- 1) */
          frameNumber = frameNumber + playDirection; // let this wrap around, do not fix
          /* Update timing info */
          totalFrameCount++;
          recentFrameCount++;
        
          endClicks = times(&endTime);
        
          recentEndTime = GetCurrentTime();
          elapsedTime = recentEndTime - recentStartTime;
          if (elapsedTime >= 1.0/(options->fpsSampleFrequency)) {
            fps = (double) recentFrameCount / elapsedTime;
            SuppressMessageDialogs(true); 
            WARNING("Frame Rate on frame %d: %g fps", frameNumber, fps);
            SuppressMessageDialogs(false); 
            /* reset timing info so we compute FPS over last 2 seconds */
            recentStartTime = GetCurrentTime();
            recentFrameCount = 0;
          }
        }
        else {
          recentFrameCount = 0;
          fps = 0.0;
        }
        if (renderer) renderer->reportActualFPS(fps); 
        if ( (options->playExit > 0 && frameNumber >= options->playExit) ||
             (options->playExit == -1 && frameNumber > endFrame-1)) { 
          events.push_back(MovieEvent("MOVIE_QUIT")); 
        }
      }
    }
  }
  /* Finish our timing information */
  endClicks = times(&endTime);
  elapsedTime = (endClicks - startClicks) / (double) Hertz;
  userTime = (endTime.tms_utime - startTime.tms_utime) / (double) Hertz;
  systemTime = (endTime.tms_stime - startTime.tms_stime) / (double) Hertz;
#if DEBUG
  INFO(
       "%.2f frames/sec [%d frames, %.2f elapsed, %d%% user, %d%% system, %d%% blocked]",
       totalFrameCount / elapsedTime,
       totalFrameCount,
       elapsedTime,
       (int) (userTime * 100.0/elapsedTime + 0.5),
       (int) (systemTime * 100.0/elapsedTime + 0.5),
       (int) ((elapsedTime - userTime - systemTime)*100.0/elapsedTime + 0.5)
       );
  INFO("Skipped delay count: %d; used delay count: %d",
       skippedDelayCount, usedDelayCount);
#endif
  
  /* Done with the frames */
  return 0; 
} // end DisplayLoop


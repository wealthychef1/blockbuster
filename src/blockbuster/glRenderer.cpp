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

#include "canvas.h"
#include "glRenderer.h"
#include "xwindow.h"
#include "util.h"
#include "cache.h"
#include "frames.h"
#include "errmsg.h"
#include "errmsg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include "timer.h"


glRenderer::glRenderer(ProgramOptions *opt, Canvas *canvas, Window parentWindow, QString name):
  Renderer(opt, canvas, parentWindow, name) {

  // from glFinishInitialization: 
  Bool rv;
  Font id = fontInfo->fid;
  unsigned int first = fontInfo->min_char_or_byte2;
  unsigned int last = fontInfo->max_char_or_byte2;
  //  glRenderer *renderer = dynamic_cast<glRenderer*>(canvas->mRenderer); 
  
  /* All GL rendering in X11 requires a glX context. */
  context = glXCreateContext(display, visInfo,
                                       NULL, GL_TRUE);
  if (!context) {
    ERROR("couldn't create GLX context");
    return ;
  }
  
  rv = glXMakeCurrent(display, window, context);
  if (rv == False) {
    ERROR("couldn't make graphics context current");
    glXDestroyContext(display, context);
    return ;
  }
  
  DEBUGMSG("GL_RENDERER = %s", (char *) glGetString(GL_RENDERER));
  
  /* OpenGL display list font bitmaps */
  fontBase = glGenLists((GLuint) last + 1);
  if (!fontBase) {
    ERROR("Unable to allocate display lists for fonts");
    glXDestroyContext(display, context);
    return ;
  }
  
  glXUseXFont(id, first, last - first + 1, fontBase + first);
  glListBase(fontBase);
  
  /* Specify our required format.  For OpenGL, always assume we're
   * getting 24-bit RGB pixels.
   */
  canvas->requiredImageFormat.scanlineByteMultiple = 1;
  canvas->requiredImageFormat.rowOrder = ROW_ORDER_DONT_CARE;
  canvas->requiredImageFormat.byteOrder = MSB_FIRST;
  canvas->requiredImageFormat.bytesPerPixel = 3;
  
  return; 
}

//=============================================================
glRenderer::~glRenderer() {
  glXDestroyContext(display, context);
  return; 
}


//=============================================================
XVisualInfo *glRenderer::ChooseVisual(void)
{
  DEBUGMSG("glChooseVisual (no stereo)"); 
  static int attributes[] = {
    GLX_USE_GL, GLX_RGBA, GLX_DOUBLEBUFFER,
    GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1,
    None
  };
  XVisualInfo *visualInfo;
  
  visualInfo = glXChooseVisual(display, screenNumber, attributes);
  if (visualInfo == NULL) {
    ERROR("cannot find a GLX visual on %s to create new OpenGL window",
          DisplayString(display));
    return NULL;
  }
  return visualInfo;
}

//=============================================================
void glRenderer::DrawString(int row, int column, const char *str)
{
  const int x = (column + 1) * mCanvas->mRenderer->fontHeight;
  const int y = (row + 1) * mCanvas->mRenderer->fontHeight;
  glPushAttrib(GL_CURRENT_BIT);
  glBitmap(0, 0, 0, 0, x, mCanvas->height - y - 1, NULL);
  glCallLists(strlen(str), GL_UNSIGNED_BYTE, (GLubyte *) str);
  glPopAttrib();
  return; 
}
//=============================================================
void glRenderer::Render(int frameNumber, 
                        const Rectangle *imageRegion,
                        int destX, int destY, float zoom, int lod){
  int lodScale;
  int localFrameNumber;
  Rectangle region = *imageRegion;
  Image *image;
  int saveSkip;
  DEBUGMSG("gl_Render begin, frame %d, %d x %d  at %d, %d  zoom=%f  lod=%d", 
           frameNumber,
           imageRegion->width, imageRegion->height,
           imageRegion->x, imageRegion->y, zoom, lod);

  /*
   * Compute possibly reduced-resolution image region to display.
   */
  if (mCanvas->frameList->stereo) {
        localFrameNumber = frameNumber *2; /* we'll display left frame only */
  }
  else {
        localFrameNumber = frameNumber;
  }

 
  if (lod > mCanvas->frameList->getFrame(localFrameNumber)->maxLOD) {
    ERROR("Error in glRenderer::Render:  lod is greater than max\n"); 
    abort(); 
  }

  lodScale = 1 << lod;

 
  {
    int rr;

    rr = ROUND_TO_MULTIPLE(imageRegion->x,lodScale);
    if(rr > imageRegion->x) {
      region.x = rr - lodScale;
    }
    region.x /= lodScale;
   
    rr = ROUND_TO_MULTIPLE(imageRegion->y,lodScale);
    if(rr > imageRegion->y) {
      region.y = rr - lodScale;
    }
    region.y /= lodScale;

    rr = ROUND_TO_MULTIPLE(imageRegion->width,lodScale);
    if(rr > imageRegion->width) {
      region.width = rr - lodScale;
    }
    region.width /= lodScale;

    rr = ROUND_TO_MULTIPLE(imageRegion->height,lodScale);
    if(rr > imageRegion->height) {
      region.height = rr - lodScale;
    }

    region.height /= lodScale;
  }
  
  zoom *= (float) lodScale;


  TIMER_PRINT("Pull the image from our cache "); 
  image = mCanvas->mRenderer->GetImage(localFrameNumber, &region, lod);
  TIMER_PRINT("Got image"); 
  if (image == NULL) {
    /* error has already been reported */
    return;
  }

  saveSkip =  image->height - (region.y + region.height);
 

  bb_assert(region.x >= 0);
  bb_assert(region.y >= 0);
  /*bb_assert(region.x + region.width <= image->width); */
  /*bb_assert(region.y + region.height <= image->height);*/

  glViewport(0, 0, mCanvas->width, mCanvas->height);


  /* only clear the window if we have to */
  if (destX > 0 || destY > 0 ||
      region.width * zoom < mCanvas->width ||
      region.height * zoom < mCanvas->height) {
    glClearColor(0.0, 0.0, 0.0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  DEBUGMSG(QString("Frame %1 row order is %2\n").arg(frameNumber).arg(image->imageFormat.rowOrder)); 
  
  if (image->imageFormat.rowOrder == BOTTOM_TO_TOP) {
    /*
     * Do adjustments to flip Y axis.
     * Yes, this is tricky to understand.
     */
    destY = mCanvas->height - static_cast<int32_t>((image->height * zoom) + destY) + static_cast<int32_t>(region.y * zoom);
    
    if (destY < 0) {
      region.y = static_cast<int32_t>(-destY / zoom);
      destY = 0;
    }
    else {
      region.y = 0;
    }
    glPixelZoom(zoom, zoom);
  }
  else {    
    destY = mCanvas->height - destY - 1;
    glPixelZoom(zoom, -zoom);
  }
  TIMER_PRINT("before draw"); 
  /*fprintf(stderr,"Region %d %d %d %d : LodScale %d : Zoom %f\n",region.x,region.y,region.width,region.height,lodScale,zoom);*/
  
  //glRasterPos2i(destX, destY); 
  // use glBitMap to set raster position
  glBitmap(0, 0, 0, 0, destX, destY, NULL);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, image->width);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, saveSkip);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, region.x);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 
                mCanvas->requiredImageFormat.scanlineByteMultiple);

  DEBUGMSG("Buffer for frame %d is %dw x %dh, region is %dw x %dh, destX = %d, destY = %d\n", frameNumber, image->width, image->height, region.width, region.height, destX, destY); 

  if (region.width > image->width || region.height > image->height ||
      region.width < 0 || region.height < 0 ||
      region.width*region.height > image->width*image->height) {
    DEBUGMSG("Abort before glDrawPixels due to programming error.  Sanity check failed.\n"); 
    abort(); 
  } else {    
    glDrawPixels(region.width, region.height,
                 GL_RGB, GL_UNSIGNED_BYTE,
                 image->imageData);
    DEBUGMSG("Done with glDrawPixels\n"); 
  }

  // move the raster position back to 0,0
  glBitmap(0, 0, 0, 0, -destX, -destY, NULL);
 //  glRasterPos2i(0,0); 
  
  /* This is bad, we are managing the cache in the render thread.  Sigh.  Anyhow, have to release the image, or the cache will fill up */  
  //mCanvas->imageCache->ReleaseImage(image);
  TIMER_PRINT("glRenderer::Render end"); 
}

//***********************************************************************
void glRenderer::SwapBuffers(void) {
  glXSwapBuffers(display, window);
  return; 
}


//***********************************************************************
void glStereoRenderer::Render(int frameNumber,
                              const Rectangle *imageRegion,
                              int destX, int destY, float zoom, int lod)
{
  int lodScale;
  int localFrameNumber;
  Rectangle region = *imageRegion;
  Image *image;
  int saveSkip;
  int saveDestX;
  int saveDestY;
  
  DEBUGMSG("glStereoRenderer::Render(frame %d) region @ (%d, %d) size %d x %d render at %d, %d, with zoom=%f  lod=%d, stereo = %d", frameNumber, 
           imageRegion->x, imageRegion->y,
           imageRegion->width, imageRegion->height,
           destX, destY, zoom, lod, 
           (int)(mCanvas->frameList->stereo));
  
  /*
   * Compute possibly reduced-resolution image region to display.
   */
 
  if (mCanvas->frameList->stereo) {
        localFrameNumber = frameNumber *2; 
        /* start with left frame*/
        glDrawBuffer(GL_BACK_LEFT);
  }
  else {
        localFrameNumber = frameNumber;
  }

  
  bb_assert(lod <= mCanvas->frameList->getFrame(localFrameNumber)->maxLOD);
  lodScale = 1 << lod;

  {
    int rr;

    rr = ROUND_TO_MULTIPLE(imageRegion->x,lodScale);
    if(rr > imageRegion->x) {
      region.x = rr - lodScale;
    }
    region.x /= lodScale;
   
    rr = ROUND_TO_MULTIPLE(imageRegion->y,lodScale);
    if(rr > imageRegion->y) {
      region.y = rr - lodScale;
    }
    region.y /= lodScale;

    rr = ROUND_TO_MULTIPLE(imageRegion->width,lodScale);
    if(rr > imageRegion->width) {
      region.width = rr - lodScale;
    }
    region.width /= lodScale;

    rr = ROUND_TO_MULTIPLE(imageRegion->height,lodScale);
    if(rr > imageRegion->height) {
      region.height = rr - lodScale;
    }
    region.height /= lodScale;
  }
  
  zoom *= (float) lodScale;

  /* Pull the image from our cache */
  image = mCanvas->mRenderer->GetImage(localFrameNumber, &region, lod);

  if (image == NULL) {
    /* error has already been reported */
    return;
  }

  saveSkip =  image->height - (region.y + region.height);
  saveDestX = destX;
  saveDestY = destY;
 

  bb_assert(region.x >= 0);
  bb_assert(region.y >= 0);
  /*bb_assert(region.x + region.width <= image->width);*/
  /*bb_assert(region.y + region.height <= image->height);*/

  glViewport(0, 0, mCanvas->width, mCanvas->height);


  /* only clear the window if we have to */
  if (destX > 0 || destY > 0 ||
      region.width * zoom < mCanvas->width ||
      region.height * zoom < mCanvas->height) {
    glClearColor(0.0, 0.0, 0.0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  if (image->imageFormat.rowOrder == BOTTOM_TO_TOP) {
    /*
     * Do adjustments to flip Y axis.
     * Yes, this is tricky to understand.
     * And we're not going to help you. 
     */
    destY = mCanvas->height - static_cast<int32_t>((image->height * zoom + destY)) + static_cast<int32_t>(region.y * zoom);
    if (destY < 0) {
      region.y = static_cast<int32_t>(-destY / zoom);
      destY = 0;
    }
    else {
      region.y = 0;
    }
    /* RasterPos is (0,0).  Offset it by (destX, destY) */
    
    DEBUGMSG("BOTTOM_TO_TOP: glDrawPixels(%d, %d, GL_RGB, GL_UNSIGNED_BYTE, data)\n",  region.width, region.height); 
    glPixelZoom(zoom, zoom);
  }
  else {
    DEBUGMSG("TOP_TO_BOTTOM: glDrawPixels(%d, %d, GL_RGB, GL_UNSIGNED_BYTE, data)\n",   region.width, region.height); 
    destY = mCanvas->height - destY - 1;
    /* RasterPos is (0,0).  Offset it by (destX, destY) */
    glPixelZoom(zoom, -zoom);
  }
  glBitmap(0, 0, 0, 0, destX, destY, NULL);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, image->width);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, saveSkip);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, region.x);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 
                mCanvas->requiredImageFormat.scanlineByteMultiple);
  glDrawPixels(region.width, region.height,
               GL_RGB, GL_UNSIGNED_BYTE,
               image->imageData);
  
  /* Offset raster pos by (-destX, -destY) to put it back to (0,0) */
  glBitmap(0, 0, 0, 0, -destX, -destY, NULL);
  
  /* Have to release the image, or the cache will fill up */
  //mCanvas->mRenderer->ReleaseImage(image);
  
  if(mCanvas->frameList->stereo) {
    glDrawBuffer(GL_BACK_RIGHT);
    localFrameNumber++;
    
    /* Pull the image from our cache */
    image = mCanvas->mRenderer->GetImage(localFrameNumber, &region, lod);
    if (image == NULL) {
      /* error has already been reported */
      return;
    }
    
    glViewport(0, 0, mCanvas->width, mCanvas->height);
    
    /* only clear the window if we have to */
    if (saveDestX > 0 || saveDestY > 0 ||
        region.width * zoom < mCanvas->width ||
        region.height * zoom < mCanvas->height) {
      glClearColor(0.0, 0.0, 0.0, 0);
      glClear(GL_COLOR_BUFFER_BIT);
    }
    
    
    if (image->imageFormat.rowOrder == BOTTOM_TO_TOP) {
      
      glPixelZoom(zoom, zoom);
      
    }
    else {
      
      DEBUGMSG("Image order is %d\n", image->imageFormat.rowOrder); 
      /* RasterPos is (0,0).  Offset it by (destX, destY) */
      glPixelZoom(zoom, -zoom);
    }
    
    glBitmap(0, 0, 0, 0, destX, destY, NULL);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, image->width);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, saveSkip);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, region.x);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 
                  mCanvas->requiredImageFormat.scanlineByteMultiple);
    glDrawPixels(region.width, region.height,
                 GL_RGB, GL_UNSIGNED_BYTE,
                 image->imageData);
    
    /* Offset raster pos by (-destX, -destY) to put it back to (0,0) */
    glBitmap(0, 0, 0, 0, -destX, -destY, NULL);
    
    /* Have to release the image, or the cache will fill up */
    //mCanvas->mRenderer->ReleaseImage(image);
    
  }
  
  
  return; 
  
}
//===========================================================
// glStereoRenderer
// ==========================================================
XVisualInfo *glStereoRenderer::ChooseVisual(void)
{
  DEBUGMSG("glStereoChooseVisual"); 
  static int attributes[] = {
    GLX_USE_GL, GLX_RGBA, GLX_DOUBLEBUFFER, GLX_STEREO,
    GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1,
    None
  };
  XVisualInfo *visualInfo;
  
  visualInfo = glXChooseVisual(display, screenNumber, attributes);
  if (visualInfo == NULL) {
    ERROR("cannot find a GLX stereo visual on %s to create new OpenGL window",
          DisplayString(display));
    return NULL;
  }
  
  return visualInfo;
}


//===========================================================
// glTextureRenderer
// ==========================================================

glTextureRenderer::glTextureRenderer(ProgramOptions *opt, Canvas *canvas, Window parentWindow):
  glRenderer(opt, canvas, parentWindow, "gltexture") {

  printf("using texture rendering\n"); 
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureWidth);
  maxTextureHeight = maxTextureWidth;
  
  if (maxTextureWidth >= 4096 ||
      maxTextureHeight >= 4096) {
	/* XXX NVIDIA's GeForce reports 4Kx4K but that size doesn't
	 * actually work!
	 * Furthermore, smaller textures seem to be faster when tiling.
	 */
	maxTextureWidth = 2048;
	maxTextureHeight = 2048;
  }
  INFO("Max Texture Size: %d x %d",
       maxTextureWidth,
       maxTextureHeight);
  
  texIntFormat = GL_RGB;
  texFormat = GL_RGB;
    
  return; 
}

//====================================================================
/* This is used to upscale texture sizes to the nearest power of 2, 
 * which is necessary in OpenGL.
 */
int32_t glTextureRenderer::MinPowerOf2(int x)
{
  int32_t rv = 1;
  while (rv > 0 && rv < x) {
	rv <<= 1;
  }
  return rv;
}



//====================================================================
/*
 * Set new projecton matrix and viewport parameters for the given
 * window size.
 */
void glTextureRenderer::UpdateProjectionAndViewport(int newWidth, int newHeight)
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  /* Note: we're flipping the Y axis here */
  glOrtho(0.0, (GLdouble) newWidth,
          (GLdouble) newHeight, 0.0,
          -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glViewport(0, 0, newWidth, newHeight);
}


TextureObject *glTextureRenderer::GetTextureObject(int frameNumber)
{
  static GLuint clock = 1;
  TextureObject *texObj = mCanvas->frameList->getFrame(frameNumber)->mTextureObject;
  
  if (!texObj) {
	/* find a free texture object */
	GLuint oldestAge = ~0;
	int oldestPos = -1;
	int i;
	/* find LRU texture object */
	for (i = 0; i < MAX_TEXTURES; i++) {
      if (mTextures[i].age < oldestAge) {
		oldestAge = mTextures[i].age;
		oldestPos = i;
      }
	}
    
	bb_assert(oldestPos >= 0);
	bb_assert(oldestPos < MAX_TEXTURES);
	bb_assert(oldestAge != ~(uint32_t)0);
    
	texObj = mTextures + oldestPos;
    
	/* unlink FrameInfo pointer */
	if (texObj->frameInfo)
      texObj->frameInfo->mTextureObject = NULL;
    
	/* update/init texObj fields */
	texObj->age = clock++;	/* XXX handle clock wrap-around! */
	texObj->frameInfo = mCanvas->frameList->getFrame(frameNumber);
	for (i = 0; i < MAX_IMAGE_LEVELS; i++)
      texObj->valid[i].width = texObj->valid[i].height = -1;
	texObj->anyLoaded = GL_FALSE;
	mCanvas->frameList->getFrame(frameNumber)->mTextureObject = texObj;
  }
  
  return texObj;
}


void glTextureRenderer::Render(int frameNumber, const Rectangle *imageRegion,
                               int destX, int destY, float zoom, int lod) {
  TextureObject *texObj;
  GLfloat s0, t0, s1, t1;
  GLfloat x0, y0, x1, y1;
  int32_t lodScale;
  int localFrameNumber;
  Rectangle region;
  Image *image;
  
#if 0
  printf("gltexture::Render %d, %d  %d x %d  at %d, %d  zoom=%f  lod=%d\n",
         imageRegion->x, imageRegion->y,
         imageRegion->width, imageRegion->height,
         destX, destY, zoom, lod);
#endif
  
  if (glXMakeCurrent(display, window, context) == False) {
    WARNING("couldn't make graphics context current before rendering");
  }
  
  UpdateProjectionAndViewport(mCanvas->width, mCanvas->height);
  glEnable(GL_TEXTURE_2D);
  
  if (mCanvas->frameList->stereo) {
    localFrameNumber = frameNumber *2; /* we'll display left frame only */
  }
  else {
    localFrameNumber = frameNumber;
  }
  
  /*
   * Compute possibly reduced-resolution image region to display.
   */
  bb_assert(lod <= mCanvas->frameList->getFrame(localFrameNumber)->maxLOD);
  lodScale = 1 << lod;
  region.x = imageRegion->x / lodScale;
  region.y = imageRegion->y / lodScale;
  region.width = imageRegion->width / lodScale;
  region.height = imageRegion->height / lodScale;
  zoom *= (float) lodScale;
  
  /* Pull the image from our cache */
  image = GetImage( localFrameNumber, &region, lod);
  if (image == NULL) {
    /* error has already been reported */
    return;
  }
  
  /* get texture object */
  texObj = GetTextureObject(localFrameNumber);
  bb_assert(texObj);
  bb_assert(texObj->frameInfo == mCanvas->frameList->getFrame(localFrameNumber));
  
  /* Setup/bind the texture object */
  if (texObj->texture) {
    glBindTexture(GL_TEXTURE_2D, texObj->texture);
  }
  else {
    /* create initial texture image */
    glGenTextures(1, &texObj->texture);
    glBindTexture(GL_TEXTURE_2D, texObj->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  
  /* Initial texture size & contents */
  if (!texObj->width) {
    int w, h, level;
    /* compute best texture size */
    texObj->width = MIN2(MinPowerOf2(image->width), maxTextureWidth);
    texObj->height = MIN2(MinPowerOf2(image->height), maxTextureHeight);
    /* make initial image (undefined contents) */
    w = texObj->width;
    h = texObj->height;
    level = 0;
    while (w > 1 || h > 1) {
      glTexImage2D(GL_TEXTURE_2D, level, texIntFormat, 
                   w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
      if (w > 1)
        w /= 2;
      if (h > 1)
        h /= 2;
      level++;
    }
  }
  
  glClearColor(0.25, 0.25, 0.25, 0);  /* XXX temporary */
  
  /*
   * Three possible rendering paths...
   */
  if (image->width <=texObj->width / lodScale
      && image->height <= texObj->height / lodScale) {
    /* The movie image can completely fit into a single texture.
     * We'll load subtextures which exactly match the image region
     * that we need to draw.
     */
    const int origRegY = region.y; /* save this */
    
    /* make sure the sub image region is valid */
    if (!texObj->anyLoaded ||
        !RectContainsRect(&texObj->valid[lod], &region)) {
      /* load the texture data now */
      
      glPixelStorei(GL_UNPACK_ROW_LENGTH, image->width);
      glPixelStorei(GL_UNPACK_SKIP_PIXELS, region.x);
      glPixelStorei(GL_UNPACK_ALIGNMENT,
                    image->imageFormat.scanlineByteMultiple);
      
      if (image->imageFormat.rowOrder == TOP_TO_BOTTOM) {
        glPixelStorei(GL_UNPACK_SKIP_ROWS, region.y);
      }
      else {
        /* invert Y coord of region to load */
        int skipRows = 0;
        bb_assert(image->imageFormat.rowOrder == BOTTOM_TO_TOP);
        /* this is a bit tricky */
        if (region.y + static_cast<int32_t>(region.height) < static_cast<int32_t>(image->height)) {
          skipRows = image->height - (region.y + region.height);
        }
        glPixelStorei(GL_UNPACK_SKIP_ROWS, skipRows);
        region.y = image->height - region.y - region.height;
      }
      
      glTexSubImage2D(GL_TEXTURE_2D, lod,
                      region.x, region.y,
                      region.width, region.height,
                      texFormat, GL_UNSIGNED_BYTE,
                      image->imageData);
      
      if (texObj->anyLoaded)
        texObj->valid[lod] = RectUnionRect(&texObj->valid[lod], imageRegion);
      else
        texObj->valid[lod] = *imageRegion;
      texObj->anyLoaded = GL_TRUE;
    }
    
	/* Choose active mipmap level */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, lod);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, lod);
    
    /* compute texcoords and vertex coords */
    s0 = (float) region.x / (texObj->width / lodScale);
    s1 = (float) (region.x + region.width) / (texObj->width / lodScale);
    if (image->imageFormat.rowOrder == TOP_TO_BOTTOM) {
      t0 = (float) region.y / (texObj->height / lodScale);
      t1 = (float) (region.y + region.height) / (texObj->height / lodScale);
    }
    else {
      int top = image->height - origRegY;
      int bot = top - region.height;
      t0 = (float) top / (texObj->height / lodScale);
      t1 = (float) bot / (texObj->height / lodScale);
    }
    x0 = destX;
    y0 = destY;
    x1 = destX + region.width * zoom;
    y1 = destY + region.height * zoom;
    
    /* XXX don't clear if the polygon fills the window */
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_QUADS);
    glTexCoord2f(s0, t0);
    glVertex2f(x0, y0);
    glTexCoord2f(s0, t1);
    glVertex2f(x0, y1);
    glTexCoord2f(s1, t1);
    glVertex2f(x1, y1);
    glTexCoord2f(s1, t0);
    glVertex2f(x1, y0);
    glEnd();
  }
  else if (region.width <= static_cast<int32_t>(texObj->width) &&
           region.height <= static_cast<int32_t>(texObj->height)) {
    /* The region of interest to draw fits entirely into one texture,
     * but the full movie frame is too large to fit into one texture.
     * Load a subtexture at (0,0) for the region of interest.
     */
    glPixelStorei(GL_UNPACK_ROW_LENGTH, image->width);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, region.x);
    if (image->imageFormat.rowOrder == BOTTOM_TO_TOP) {
      int skip;
      skip = image->height - (region.y + region.height);
      glPixelStorei(GL_UNPACK_SKIP_ROWS, skip);
    }
    else {
      glPixelStorei(GL_UNPACK_SKIP_ROWS, region.y);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT,
                  image->imageFormat.scanlineByteMultiple);
    
    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    0, 0, /* pos */
                    region.width, region.height,
                    texFormat, GL_UNSIGNED_BYTE,
                    image->imageData);
    /* invalidate valid region, for sake of first path, above */
    texObj->valid[lod].x = 0;
    texObj->valid[lod].y = 0;
    texObj->valid[lod].width = 0;
    texObj->valid[lod].height = 0;
    
    s0 = 0.0;
    t0 = 0.0;
    s1 = (float) region.width / texObj->width;
    t1 = (float) region.height / texObj->height;
    if (image->imageFormat.rowOrder == BOTTOM_TO_TOP) {
      /* invert texcoords */
      GLfloat temp = t0;
      t0 = t1;
      t1 = temp;
    }
    
    x0 = destX;
    y0 = destY;
    x1 = destX + region.width * zoom;
    y1 = destY + region.height * zoom;
    
    /* XXX don't clear if the polygon fills the window */
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_QUADS);
    glTexCoord2f(s0, t0);
    glVertex2f(x0, y0);
    glTexCoord2f(s0, t1);
    glVertex2f(x0, y1);
    glTexCoord2f(s1, t1);
    glVertex2f(x1, y1);
    glTexCoord2f(s1, t0);
    glVertex2f(x1, y0);
    glEnd();
  }
  else {
    /* We're drawing an image that's larger than the max texture size.
     * I.e. a _really_ big movie image.
     * Draw it in pieces, as a tiling of quadrilaterals.
     */
    const int tileWidth = maxTextureWidth;
    const int tileHeight = maxTextureHeight;
    int row, col, width, height;
    
    /* invalidate valid region, for sake of first path, above */
    texObj->valid[lod].x = 0;
    texObj->valid[lod].y = 0;
    texObj->valid[lod].width = 0;
    texObj->valid[lod].height = 0;
    
    glPixelStorei(GL_UNPACK_ROW_LENGTH, image->width);
    glPixelStorei(GL_UNPACK_ALIGNMENT,
                  image->imageFormat.scanlineByteMultiple);
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    /* march over the sub image region, drawing a tile at a time */
    for (row = 0; row < region.height; row += tileHeight) {
      for (col = 0; col < region.width; col += tileWidth) {
        
        /* compute tile / texture size */
        if (col + tileWidth > region.width) {
          width = region.width - col;
        }
        else {
          width = tileWidth;
        }
        if (row + tileHeight > region.height) {
          height = region.height - row;
          bb_assert(height > 0);
        }
        else {
          height = tileHeight;
        }
        
        /* compute SKIP_PIXELS and load texture data */
        if (image->imageFormat.rowOrder == BOTTOM_TO_TOP) {
          int bottom = region.height - row - height;
          if (region.y + region.height 
              < static_cast<int32_t>(image->height)) {
            int d = image->height
              - (region.y + region.height);
            bottom += d;
          }
          glPixelStorei(GL_UNPACK_SKIP_ROWS, bottom);
        }
        else {
          glPixelStorei(GL_UNPACK_SKIP_ROWS, region.y + row);
        }
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, region.x + col);
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        0, 0, width, height,
                        texFormat, GL_UNSIGNED_BYTE,
                        image->imageData);
        
        /* tex coords */
        s0 = 0;
        t0 = 0;
        s1 = (float) width / maxTextureWidth;
        t1 = (float) height / maxTextureHeight;
        if (image->imageFormat.rowOrder == BOTTOM_TO_TOP) {
          /* invert texcoords */
          GLfloat temp = t0;
          t0 = t1;
          t1 = temp;
        }
        
        /* vertex coords */
        x0 = destX + col * zoom;
        y0 = destY + row * zoom;
        x1 = x0 + width * zoom;
        y1 = y0 + height * zoom;
        
        /* draw quad */
        glBegin(GL_QUADS);
        glTexCoord2f(s0, t0);
        glVertex2f(x0, y0);
        glTexCoord2f(s0, t1);
        glVertex2f(x0, y1);
        glTexCoord2f(s1, t1);
        glVertex2f(x1, y1);
        glTexCoord2f(s1, t0);
        glVertex2f(x1, y0);
        glEnd();
        
      } /* for col */
    } /* for row */
    
    texObj->valid[lod].x = 0;
    texObj->valid[lod].y = 0;
    texObj->valid[lod].width = 0;
    texObj->valid[lod].height = 0;
  }
  
  /* debug */
  {
    int err = glGetError();
    if (err) {
      ERROR("OpenGL Error 0x%x\n", err);
    }
  }
  
  /* Have to release the image, or the cache will fill up */
  ReleaseImage( image);
  
  glDisable(GL_TEXTURE_2D);
}


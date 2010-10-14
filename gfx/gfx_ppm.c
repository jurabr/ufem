/*
   File name: gfx_ppm.c
   Date:      2004/01/05 23:14
   Author:    

   Copyright (C) 2004 

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
  
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
  
   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA.
*/

#include "fem_gfx.h"

#ifdef _USE_GFX_

#ifdef _USE_LIB_TIFF_
#include <tiffio.h>
#endif

/** Saves plotting as a PPM image
 * @param filename name of file
 * @param with width of window and image
 * @param height height of window and image
 * @return status
 * */
int gfxSavePpm(char *filename, int width, int height)
{
  int            rv    = AF_OK ;
  unsigned char *image = NULL ;
  FILE          *fw    = NULL ;
  int            i, j ;

   /* image buffer */
   if ((image = malloc(3*width*height*sizeof(char))) == NULL) 
   {
      fprintf(msgout,"[E] %s!\n",_("No memory for image"));
      return(AF_ERR_MEM);
   }

   if ((fw = fopen(filename,"w")) == NULL) 
   {
      fprintf(msgout,"[e} %s!\n",_("Unable to open image file"));
      return(AF_ERR_IO);
   }

   glPixelStorei(GL_PACK_ALIGNMENT,1);

   /* getting of image from buffer */
   glReadBuffer(GL_BACK_LEFT);
   glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,image);

   /* ppm header */
   fprintf(fw,"P6\n%d %d\n255\n",width,height);

   /* and image itself */
   for (j=height-1;j>=0;j--) 
   {
      for (i=0;i<width;i++) 
      {
         fputc(image[3*j*width+3*i+0],fw);
         fputc(image[3*j*width+3*i+1],fw);
         fputc(image[3*j*width+3*i+2],fw);
      }
   }

   free(image);
   image = NULL ;

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);

   if ((rv = fclose(fw)) != 0 )
   {
      fprintf(msgout,"[e} %s!\n",_("Unable to close image file"));
   }

   return(rv);
}

/** Saves plotting as a PPM image
 * @param filename name of file
 * @param with width of window and image
 * @param height height of window and image
 * @return status
 * */
int gfxSaveTiff(char *filename, int width, int height)
{
#ifdef _USE_LIB_TIFF_
  int      rv    = AF_OK ;
  int      compression = 1 ;
  TIFF    *file  = NULL ;
  GLubyte *p     = NULL ;
  GLubyte *image = NULL ;
  int      i ;

  if ((file = TIFFOpen(filename, "w")) == NULL)
  {
    fprintf(msgout,"[E] %s!\n", _("Cannot open TIFF image"));
    return(AF_ERR_MEM);
  }

  if ((image = (GLubyte *) malloc(3 * width * height * sizeof(GLubyte))) == NULL)
  {
    TIFFClose(file) ;
    fprintf(msgout,"[E] %s!\n", _("Cannot allocate memory for the TIFF image"));
    return(AF_ERR_MEM);
  }

  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  glReadBuffer(GL_BACK_LEFT);
  glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,image);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  TIFFSetField(file, TIFFTAG_IMAGEWIDTH, (uint32) width);
  TIFFSetField(file, TIFFTAG_IMAGELENGTH, (uint32) height);
  TIFFSetField(file, TIFFTAG_BITSPERSAMPLE, 8);
  TIFFSetField(file, TIFFTAG_COMPRESSION, compression);
  TIFFSetField(file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
  TIFFSetField(file, TIFFTAG_SAMPLESPERPIXEL, 3);
  TIFFSetField(file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(file, TIFFTAG_ROWSPERSTRIP, 1);
  TIFFSetField(file, TIFFTAG_IMAGEDESCRIPTION, "FEM UI Plot");

  p = image;

  for (i = height - 1; i >= 0; i--) 
  {
    if (TIFFWriteScanline(file, p, i, 0) < 0) 
    {
      free(image);
      TIFFClose(file);

      fprintf(msgout, "[E] %s!\n", _("Cannot write TIFF image"));
      return(AF_ERR_IO) ;
    }

    p += (3 * width * sizeof(GLubyte)) ;
  }

  free(image);
  TIFFClose(file);
  return(rv);
#else
  fprintf(msgout, "[E] %s!\n", _("TIFF export is not available on your platform"));
  return(AF_ERR_VAL);
#endif
}

#endif /* _USE_GFX_ */
/* end of gfx_ppm.c */

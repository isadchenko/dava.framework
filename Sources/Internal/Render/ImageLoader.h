/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_IMAGELOADER_H__
#define __DAVAENGINE_IMAGELOADER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "FileSystem/FilePath.h"

namespace DAVA 
{

class File;
class Image;
class ImageLoader
{
public:

	static Vector<Image *> CreateFromFileByExtension(const FilePath & pathname);

	static Vector<Image *> CreateFromFileByContent(const FilePath & pathname);
	static Vector<Image *> CreateFromFileByContent(File *file);

    static void Save(Image *image, const FilePath & pathname);
    
protected:

    static Vector<Image *> CreateFromPNGFile(const FilePath & pathname);
	static Vector<Image *> CreateFromPVRFile(const FilePath & pathname);
	static Vector<Image *> CreateFromDDSFile(const FilePath & pathname);
	static Vector<Image *> CreateFromPNG(File *file);
	static Vector<Image *> CreateFromPVR(File *file);
	static Vector<Image *> CreateFromDDS(File *file);
    
    static bool IsPVRFile(File *file);
    static bool IsPNGFile(File *file);
	static bool IsDDSFile(File *file);
};
	
};

#endif // __DAVAENGINE_IMAGELOADER_H__

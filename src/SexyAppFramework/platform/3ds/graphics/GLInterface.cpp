/*
 * Portions of this file are based on the PopCap Games Framework
 * Copyright (C) 2005-2009 PopCap Games, Inc.
 * 
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * This file is part of PvZ-Portable.
 *
 * PvZ-Portable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PvZ-Portable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PvZ-Portable. If not, see <https://www.gnu.org/licenses/>.
 */

#include <3ds.h>

#include "graphics/GLInterface.h"
#include "graphics/GLImage.h"
#include "SexyAppBase.h"
#include <mutex>
#include "graphics/Graphics.h"
#include "graphics/MemoryImage.h"

#include "colored_shbin.h"
#include "textured_shbin.h"

#define MAX_VERTICES 2048
#define GetColorFromTriVertex(theVertex, theColor) (theVertex.color?theVertex.color:theColor)

#define DISPLAY_TRANSFER_FLAGS \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
	GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
	GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

using namespace Sexy;

static int gMinTextureWidth;
static int gMinTextureHeight;
static int gMaxTextureWidth;
static int gMaxTextureHeight;
static int gSupportedPixelFormats;
static bool gTextureSizeMustBePow2;
static const int MAX_TEXTURE_SIZE = 1024;
static bool gLinearFilter = false;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static C3D_Mtx projection;

struct Shader {
	DVLB_s* dvlb;
	shaderProgram_s program;
	int uf_projection;
};
static Shader* currShader;
static Shader shaders[2];
static C3D_RenderTarget* bottomTarget;

static GLVertex* gVertices;
static int gStartVertex;
static int gNumVertices;
static int gVertexMode;

static void GfxBegin(int vertexMode)
{
	if (gVertexMode != -1) return;
	gVertexMode = vertexMode;
}

static void GfxEnd()
{
	if (gVertexMode == -1) return;

	C3D_DrawArrays((GPU_Primitive_t)gVertexMode, gStartVertex, gNumVertices);
	gStartVertex += gNumVertices;

	gVertexMode = -1;
	gNumVertices = 0;
}

static void GfxAddVertices(const GLVertex *arr, int arrCount)
{
	if (gVertexMode == -1) return;

	if (gStartVertex + gNumVertices + arrCount >= MAX_VERTICES)
	{
		int oldMode = gVertexMode;
		GfxEnd();
		GfxBegin(oldMode);
	}
	
	for (int i=gStartVertex + gNumVertices; i<gStartVertex + gNumVertices + arrCount; i++)
	{
		gVertices[i] = arr[i];
	}
	gNumVertices += arrCount;
}

static void GfxAddVertices(VertexList &arr)
{
	if (gVertexMode == -1) return;

	if (gStartVertex + gNumVertices + arr.size() >= MAX_VERTICES)
	{
		int oldMode = gVertexMode;
		GfxEnd();
		GfxBegin(oldMode);
	}

	for (int i=gStartVertex + gNumVertices; i<gStartVertex + gNumVertices + arr.size(); i++)
	{
		gVertices[i] = arr[i];
	}
	gNumVertices += arr.size();
}

static void GfxAddVertices(const TriVertex arr[][3], int arrCount, unsigned int theColor, float tx, float ty, float aMaxTotalU, float aMaxTotalV)
{
	if (gVertexMode == -1) return;

	if (gStartVertex + gNumVertices + arrCount*3 >= MAX_VERTICES)
	{
		int oldMode = gVertexMode;
		GfxEnd();
		GfxBegin(oldMode);
	}

	for (int aTriangleNum=0; aTriangleNum < arrCount; aTriangleNum++)
	{
		TriVertex* aTriVerts = (TriVertex*) arr[aTriangleNum];

		for (int i = 0; i < 3; i++)
		{
			gVertices[gStartVertex+gNumVertices+i].sx = aTriVerts[i].x + tx;
			gVertices[gStartVertex+gNumVertices+i].sy = aTriVerts[i].y + ty;
			gVertices[gStartVertex+gNumVertices+i].color = GetColorFromTriVertex(aTriVerts[i], theColor);
			gVertices[gStartVertex+gNumVertices+i].tu = aTriVerts[i].u * aMaxTotalU;
			gVertices[gStartVertex+gNumVertices+i].tv = aTriVerts[i].v * aMaxTotalV;
		}

		gNumVertices += 3;
	}
}

static void SetTexture(C3D_Tex *theTex, bool force=false)
{
	Shader* oldShader = currShader;

	if (!theTex)
		currShader = &shaders[0];
	else
	{
		currShader = &shaders[1];
		C3D_TexBind(0, theTex);
	}

	if (force || oldShader != currShader)
	{
		C3D_BindProgram(&currShader->program);

		// Configure attributes for use with the vertex shader
		C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
		AttrInfo_Init(attrInfo);
		AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
		AttrInfo_AddLoader(attrInfo, 1, GPU_UNSIGNED_BYTE, 4); // v1=color
		if (theTex) AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 2); // v2=texcoord0

		// Configure buffers
		C3D_BufInfo* bufInfo = C3D_GetBufInfo();
		BufInfo_Init(bufInfo);
		if (!theTex)
			BufInfo_Add(bufInfo, gVertices, sizeof(GLVertex), 2, 0x10);
		else
			BufInfo_Add(bufInfo, gVertices, sizeof(GLVertex), 3, 0x210);

		C3D_TexEnv* env = C3D_GetTexEnv(0);
		C3D_TexEnvInit(env);
		if (!theTex)
		{
			// Configure the first fragment shading substage to just pass through the vertex color
			// See https://www.opengl.org/sdk/docs/man2/xhtml/glTexEnv.xml for more insight
			C3D_TexEnvSrc(env, C3D_Both, GPU_PRIMARY_COLOR, (GPU_TEVSRC)0, (GPU_TEVSRC)0);
			C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
		}
		else
		{
			// Configure the first fragment shading substage to blend the texture color with
			// the vertex color (calculated by the vertex shader using a lighting algorithm)
			// See https://www.opengl.org/sdk/docs/man2/xhtml/glTexEnv.xml for more insight
			C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, (GPU_TEVSRC)0);
			C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static inline u32 CalcZOrder(u32 a)
{
	// Simplified "Interleave bits by Binary Magic Numbers" from
	// http://graphics.stanford.edu/~seander/bithacks.html#InterleaveBMN
	a = (a | (a << 2)) & 0x33;
	a = (a | (a << 1)) & 0x55;
	return a;
	// equivalent to return (a & 1) | ((a & 2) << 1) | (a & 4) << 2;
	//  but compiles to less instructions
}

// Pixels are arranged in a recursive Z-order curve / Morton offset
// They are arranged into 8x8 tiles, where each 8x8 tile is composed of
//  four 4x4 subtiles, which are in turn composed of four 2x2 subtiles
static void ToMortonTexture32(C3D_Tex* tex, u32* dst, u32* src, int originX, int originY, int width, int height)
{
	u32 pixel;
	unsigned int mortonX, mortonY;
	unsigned int dstX, dstY, tileX, tileY;

	for (int y = 0; y < height; y++)
	{
		dstY    = tex->height - 1 - (y + originY);
		tileY   = dstY & ~0x07;
		mortonY = CalcZOrder(dstY & 0x07) << 1;

		for (int x = 0; x < width; x++)
		{
			dstX    = x + originX;
			tileX   = dstX & ~0x07;
			mortonX = CalcZOrder(dstX & 0x07);
			pixel   = src[x + (y * width)];

			u8 r = pixel & 0xff;
			u8 g = (pixel >> 8) & 0xff;
			u8 b = (pixel >> 16) & 0xff;
			u8 a = (pixel >> 24) & 0xff;
			pixel = (a<<0) | (r<<8) | (g<<16) | (b<<24);

			dst[(mortonX | mortonY) + (tileX * 8) + (tileY * tex->width)] = pixel;
		}
	}
}

static void ToMortonTexture16(C3D_Tex* tex, u16* dst, u16* src, int originX, int originY, int width, int height)
{
	u16 pixel;
	unsigned int mortonX, mortonY;
	unsigned int dstX, dstY, tileX, tileY;

	for (int y = 0; y < height; y++)
	{
		dstY    = tex->height - 1 - (y + originY);
		tileY   = dstY & ~0x07;
		mortonY = CalcZOrder(dstY & 0x07) << 1;

		for (int x = 0; x < width; x++)
		{
			dstX    = x + originX;
			tileX   = dstX & ~0x07;
			mortonX = CalcZOrder(dstX & 0x07);
			pixel   = src[x + (y * width)];

			u8 r = pixel & 0xff;
			u8 g = (pixel >> 4) & 0xff;
			u8 b = (pixel >> 8) & 0xff;
			u8 a = (pixel >> 12) & 0xff;
			pixel = (a<<0) | (r<<4) | (g<<8) | (b<<12);

			dst[(mortonX | mortonY) + (tileX * 8) + (tileY * tex->width)] = pixel;
		}
	}
}

static void ToMortonTexture565(C3D_Tex* tex, u16* dst, u16* src, int originX, int originY, int width, int height)
{
	u16 pixel;
	unsigned int mortonX, mortonY;
	unsigned int dstX, dstY, tileX, tileY;

	for (int y = 0; y < height; y++)
	{
		dstY    = tex->height - 1 - (y + originY);
		tileY   = dstY & ~0x07;
		mortonY = CalcZOrder(dstY & 0x07) << 1;

		for (int x = 0; x < width; x++)
		{
			dstX    = x + originX;
			tileX   = dstX & ~0x07;
			mortonX = CalcZOrder(dstX & 0x07);
			pixel   = src[x + (y * width)];

			dst[(mortonX | mortonY) + (tileX * 8) + (tileY * tex->width)] = pixel;
		}
	}
}

static void CopyImageToTexture8888(C3D_Tex *theTex, MemoryImage *theImage, int offx, int offy, int theWidth, int theHeight, int theDestPitch, int theDestHeight, bool rightPad, bool bottomPad, bool create)
{
	uint32_t *aDest = new uint32_t[theDestPitch * theDestHeight];

	if (theImage->mColorTable == nullptr)
	{
		uint32_t *srcRow = (uint32_t*)theImage->GetBits() + offy * theImage->GetWidth() + offx;
		uint32_t *dstRow = aDest;

		for(int y=0; y<theHeight; y++)
		{
			uint32_t *src = srcRow;
			uint32_t *dst = dstRow;
			for(int x=0; x<theWidth; x++)
				*dst++ = *src++;

			if (rightPad)
				*dst = *(dst-1);

			srcRow += theImage->GetWidth();
			dstRow += theDestPitch;
		}
	}
	else // palette
	{
		uint8_t *srcRow = (uint8_t*)theImage->mColorIndices + offy * theImage->GetWidth() + offx;
		uint32_t *dstRow = aDest;
		uint32_t *palette = (uint32_t*)theImage->mColorTable;

		for(int y=0; y<theHeight; y++)
		{
			uint8_t *src = srcRow;
			uint32_t *dst = dstRow;
			for(int x=0; x<theWidth; x++)
				*dst++ = palette[*src++];

			if (rightPad)
				*dst = *(dst-1);

			srcRow += theImage->GetWidth();
			dstRow += theDestPitch;
		}
	}

	if (bottomPad)
	{
		uint32_t *dstrow = aDest + (theDestPitch*theHeight);
		memcpy(dstrow, dstrow-(theDestPitch*4), (theDestPitch*4));
	}

	if (create)
		ToMortonTexture32(theTex, (u32*)theTex->data, (u32*)aDest, 0, 0, theDestPitch, theDestHeight);
	else
		ToMortonTexture32(theTex, (u32*)theTex->data, (u32*)aDest, offx, offy, theDestPitch, theDestHeight);

	delete[] aDest;
}

static void CopyImageToTexture4444(C3D_Tex *theTex, MemoryImage *theImage, int offx, int offy, int theWidth, int theHeight, int theDestPitch, int theDestHeight, bool rightPad, bool bottomPad, bool create)
{
	uint16_t *aDest = new uint16_t[theDestPitch * theDestHeight];

	if (theImage->mColorTable == nullptr)
	{
		uint32_t *srcRow = (uint32_t*)theImage->GetBits() + offy * theImage->GetWidth() + offx;
		uint16_t *dstRow = aDest;

		for(int y=0; y<theHeight; y++)
		{
			uint32_t *src = srcRow;
			uint16_t *dst = dstRow;
			for(int x=0; x<theWidth; x++)
			{
				uint32_t aPixel = *src++;
				*dst++ = ((aPixel>>16)&0xF000) | ((aPixel>>12)&0x0F00) | ((aPixel>>8)&0x00F0) | ((aPixel>>4)&0x000F);
			}

			if (rightPad)
				*dst = *(dst-1);

			srcRow += theImage->GetWidth();
			dstRow += theDestPitch;
		}
	}
	else // palette
	{
		uint8_t *srcRow = (uint8_t*)theImage->mColorIndices + offy * theImage->GetWidth() + offx;
		uint16_t *dstRow = aDest;
		uint32_t *palette = (uint32_t*)theImage->mColorTable;

		for(int y=0; y<theHeight; y++)
		{
			uint8_t *src = srcRow;
			uint16_t *dst = dstRow;
			for(int x=0; x<theWidth; x++)
			{
				uint32_t aPixel = palette[*src++];
				*dst++ = ((aPixel>>16)&0xF000) | ((aPixel>>12)&0x0F00) | ((aPixel>>8)&0x00F0) | ((aPixel>>4)&0x000F);
			}

			if (rightPad)
				*dst = *(dst-1);

			srcRow += theImage->GetWidth();
			dstRow += theDestPitch;
		}
	}

	if (bottomPad)
	{
		uint16_t *dstrow = aDest + (theDestPitch*theHeight);
		memcpy(dstrow, dstrow-(theDestPitch*2), (theDestPitch*2));
	}

	if (create)
		ToMortonTexture16(theTex, (u16*)theTex->data, (u16*)aDest, 0, 0, theDestPitch, theDestHeight);
	else
		ToMortonTexture16(theTex, (u16*)theTex->data, (u16*)aDest, offx, offy, theDestPitch, theDestHeight);

	delete[] aDest;
}

static void CopyImageToTexture565(C3D_Tex *theTex, MemoryImage *theImage, int offx, int offy, int theWidth, int theHeight, int theDestPitch, int theDestHeight, bool rightPad, bool bottomPad, bool create)
{
	uint16_t *aDest = new uint16_t[theDestPitch * theDestHeight];

	if (theImage->mColorTable == nullptr)
	{
		uint32_t *srcRow = (uint32_t*)theImage->GetBits() + offy * theImage->GetWidth() + offx;
		uint16_t *dstRow = aDest;

		for(int y=0; y<theHeight; y++)
		{
			uint32_t *src = srcRow;
			uint16_t *dst = dstRow;
			for(int x=0; x<theWidth; x++)
			{
				uint32_t aPixel = *src++;
				*dst++ = ((aPixel>>8)&0xF800) | ((aPixel>>5)&0x07E0) | ((aPixel>>3)&0x001F);
			}

			if (rightPad)
				*dst = *(dst-1);

			srcRow += theImage->GetWidth();
			dstRow += theDestPitch;
		}
	}
	else
	{
		uint8_t *srcRow = (uint8_t*)theImage->mColorIndices + offy * theImage->GetWidth() + offx;
		uint16_t *dstRow = aDest;
		uint32_t *palette = (uint32_t*)theImage->mColorTable;

		for(int y=0; y<theHeight; y++)
		{
			uint8_t *src = srcRow;
			uint16_t *dst = dstRow;
			for(int x=0; x<theWidth; x++)
			{
				uint32_t aPixel = palette[*src++];
				*dst++ = ((aPixel>>8)&0xF800) | ((aPixel>>5)&0x07E0) | ((aPixel>>3)&0x001F);
			}

			if (rightPad)
				*dst = *(dst-1);

			srcRow += theImage->GetWidth();
			dstRow += theDestPitch;
		}
	}

	if (bottomPad)
	{
		uint16_t *dstrow = aDest + (theDestPitch*theHeight);
		memcpy(dstrow, dstrow-(theDestPitch*2), (theDestPitch*2));
	}

	if (create)
		ToMortonTexture565(theTex, (u16*)theTex->data, (u16*)aDest, 0, 0, theDestPitch, theDestHeight);
	else
		ToMortonTexture565(theTex, (u16*)theTex->data, (u16*)aDest, offx, offy, theDestPitch, theDestHeight);

	delete[] aDest;
}

static void CopyImageToTexturePalette8(C3D_Tex *theTex, MemoryImage *theImage, int offx, int offy, int theWidth, int theHeight, int theDestPitch, int theDestHeight, bool rightPad, bool bottomPad, bool create)
{
	Sexy::PrintF("PALETTE %d %d - %d %d - %d %d", offx, offy, theWidth, theHeight, theDestPitch, theDestHeight);

	uint32_t *aDest = new uint32_t[theDestPitch * theDestHeight];

	uint8_t *srcRow = (uint8_t*)theImage->mColorIndices + offy * theImage->GetWidth() + offx;
	uint32_t *dstRow = aDest;
	uint16_t *palette = (uint16_t*)theImage->mColorTable;

	for(int y=0; y<theHeight; y++)
	{
		uint8_t *src = srcRow;
		uint32_t *dst = dstRow;
		for(int x=0; x<theWidth; x++)
		{
			uint32_t aPixel = palette[*src++];
			*dst++ = (aPixel&0xFF00FF00) | ((aPixel>>16)&0xFF) | ((aPixel<<16)&0xFF0000);
		}

		if (rightPad) 
			*dst = *(dst-1);

		srcRow += theImage->GetWidth();
		dstRow += theDestPitch;
	}

	if (bottomPad)
	{
		uint32_t *dstrow = aDest + (theDestPitch*theHeight);
		memcpy(dstrow, dstrow-(theDestPitch*4), (theDestPitch*4));
	}

	if (create)
		ToMortonTexture32(theTex, (u32*)theTex->data, (u32*)aDest, 0, 0, theDestPitch, theDestHeight);
	else
		ToMortonTexture32(theTex, (u32*)theTex->data, (u32*)aDest, offx, offy, theDestPitch, theDestHeight);

	delete[] aDest;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void CopyImageToTexture(C3D_Tex *theTex, MemoryImage *theImage, int offx, int offy, int texWidth, int texHeight, PixelFormat theFormat, bool create)
{
	GPU_TEXTURE_FILTER_PARAM filter = (gLinearFilter) ? GPU_LINEAR : GPU_NEAREST;
	C3D_TexSetFilter(theTex, filter, filter);
	auto wrap = (theImage->mRenderFlags & RenderImageFlag_Repeat) ? GPU_REPEAT : GPU_CLAMP_TO_EDGE;
	C3D_TexSetWrap(theTex, wrap, wrap);

	int aWidth = std::min(texWidth,(theImage->GetWidth()-offx));
	int aHeight = std::min(texHeight,(theImage->GetHeight()-offy));

	bool rightPad = aWidth<texWidth;
	bool bottomPad = aHeight<texHeight;

	if(aWidth>0 && aHeight>0)
	{
		switch (theFormat)
		{
			case PixelFormat_A8R8G8B8:	CopyImageToTexture8888(theTex, theImage, offx, offy, aWidth, aHeight, texWidth, texHeight, rightPad, bottomPad, create); break;
			case PixelFormat_A4R4G4B4:	CopyImageToTexture4444(theTex, theImage, offx, offy, aWidth, aHeight, texWidth, texHeight, rightPad, bottomPad, create); break;
			case PixelFormat_R5G6B5:	CopyImageToTexture565(theTex, theImage, offx, offy, aWidth, aHeight, texWidth, texHeight, rightPad, bottomPad, create); break;
			case PixelFormat_Palette8:	CopyImageToTexturePalette8(theTex, theImage, offx, offy, aWidth, aHeight, texWidth, texHeight, rightPad, bottomPad, create); break;
			case PixelFormat_Unknown: break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static int GetClosestPowerOf2Above(int theNum)
{
	int aPower2 = 1;
	while (aPower2 < theNum)
		aPower2<<=1;
	return aPower2;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static bool IsPowerOf2(int theNum)
{
	int aNumBits = 0;
	while (theNum>0)
	{
		aNumBits += theNum&1;
		theNum >>= 1;
	}

	return aNumBits==1;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void GetBestTextureDimensions(int &theWidth, int &theHeight, bool isEdge, bool usePow2, uint32_t theImageFlags)
{
//	theImageFlags = RenderImageFlag_MinimizeNumSubdivisions;
	if (theImageFlags & RenderImageFlag_Use64By64Subdivisions)
	{
		theWidth = theHeight = 64;
		return;
	}

	static int aGoodTextureSize[MAX_TEXTURE_SIZE];
	static bool haveInited = false;
	if (!haveInited)
	{
		haveInited = true;
		int i;
		int aPow2 = 1;
		for (i=0; i<MAX_TEXTURE_SIZE; i++)
		{
			if (i > aPow2)
				aPow2 <<= 1;

			int aGoodValue = aPow2;
			if ((aGoodValue - i ) > 64)
			{
				aGoodValue >>= 1;
				while (true)
				{
					int aLeftOver = i % aGoodValue;
					if (aLeftOver<64 || IsPowerOf2(aLeftOver))
						break;

					aGoodValue >>= 1;
				}
			}
			aGoodTextureSize[i] = aGoodValue;
		}
	}

	int aWidth = theWidth;
	int aHeight = theHeight;

	if (usePow2)
	{
		if (isEdge || (theImageFlags & RenderImageFlag_MinimizeNumSubdivisions))
		{
			aWidth = aWidth >= gMaxTextureWidth ? gMaxTextureWidth : GetClosestPowerOf2Above(aWidth);
			aHeight = aHeight >= gMaxTextureHeight ? gMaxTextureHeight : GetClosestPowerOf2Above(aHeight);
		}
		else
		{
			aWidth = aWidth >= gMaxTextureWidth ? gMaxTextureWidth : aGoodTextureSize[aWidth];
			aHeight = aHeight >= gMaxTextureHeight ? gMaxTextureHeight : aGoodTextureSize[aHeight];
		}
	}

	if (aWidth < gMinTextureWidth)
		aWidth = gMinTextureWidth;

	if (aHeight < gMinTextureHeight)
		aHeight = gMinTextureHeight;

	theWidth = aWidth;
	theHeight = aHeight;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
TextureData::TextureData()
{
	mWidth = 0;
	mHeight = 0;
	mTexVecWidth = 0;
	mTexVecHeight = 0;
	mBitsChangedCount = 0;
	mTexMemSize = 0;
	mTexPieceWidth = 64;
	mTexPieceHeight = 64;

	//mPalette = nullptr;
	mPixelFormat = PixelFormat_Unknown;
	mImageFlags = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
TextureData::~TextureData()
{
	ReleaseTextures();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TextureData::ReleaseTextures()
{
	for(int i=0; i<(int)mTextures.size(); i++)
	{
		C3D_Tex* aSurface = &mTextures[i].mTexture;
		C3D_TexDelete(aSurface);
	}

	mTextures.clear();

	mTexMemSize = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TextureData::CreateTextureDimensions(MemoryImage *theImage)
{
	int aWidth = theImage->GetWidth();
	int aHeight = theImage->GetHeight();
	unsigned int i;

	// Calculate inner piece sizes
	mTexPieceWidth = aWidth;
	mTexPieceHeight = aHeight;
	bool usePow2 = true; //gTextureSizeMustBePow2 || mPixelFormat==PixelFormat_Palette8;
	GetBestTextureDimensions(mTexPieceWidth, mTexPieceHeight, false, usePow2, mImageFlags);

	// Calculate right boundary piece sizes
	int aRightWidth = aWidth % mTexPieceWidth;
	int aRightHeight = mTexPieceHeight;
	if (aRightWidth > 0)
		GetBestTextureDimensions(aRightWidth, aRightHeight, true, usePow2, mImageFlags);
	else
		aRightWidth = mTexPieceWidth;

	// Calculate bottom boundary piece sizes
	int aBottomWidth = mTexPieceWidth;
	int aBottomHeight = aHeight % mTexPieceHeight;
	if (aBottomHeight > 0)
		GetBestTextureDimensions(aBottomWidth, aBottomHeight, true, usePow2, mImageFlags);
	else
		aBottomHeight = mTexPieceHeight;

	// Calculate corner piece size
	int aCornerWidth = aRightWidth;
	int aCornerHeight = aBottomHeight;
	GetBestTextureDimensions(aCornerWidth, aCornerHeight, true, usePow2, mImageFlags);

	// Allocate texture array
	mTexVecWidth = (aWidth + mTexPieceWidth - 1) / mTexPieceWidth;
	mTexVecHeight = (aHeight + mTexPieceHeight - 1) / mTexPieceHeight;
	mTextures.resize(mTexVecWidth * mTexVecHeight);

	// Assign inner pieces
	for (i = 0; i < mTextures.size(); i++)
	{
		TextureDataPiece& aPiece = mTextures[i];
		aPiece.mWidth = mTexPieceWidth;
		aPiece.mHeight = mTexPieceHeight;
	}

	// Assign right pieces
	for (i = mTexVecWidth - 1; i < mTextures.size(); i += mTexVecWidth)
	{
		TextureDataPiece& aPiece = mTextures[i];
		aPiece.mWidth = aRightWidth;
		aPiece.mHeight = aRightHeight;
	}

	// Assign bottom pieces
	for (i = mTexVecWidth * (mTexVecHeight - 1); i < mTextures.size(); i++)
	{
		TextureDataPiece& aPiece = mTextures[i];
		aPiece.mWidth = aBottomWidth;
		aPiece.mHeight = aBottomHeight;
	}

	// Assign corner piece
	mTextures.back().mWidth = aCornerWidth;
	mTextures.back().mHeight = aCornerHeight;
	/**/

	mMaxTotalU = aWidth / (float)mTexPieceWidth;
	mMaxTotalV = aHeight / (float)mTexPieceHeight;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TextureData::CreateTextures(MemoryImage *theImage)
{
	theImage->DeleteSWBuffers(); // don't need these buffers for 3d drawing

	// Choose appropriate pixel format
	PixelFormat aFormat = PixelFormat_A8R8G8B8;
	//theImage->mRenderFlags = RenderImageFlag_UseA4R4G4B4;

	theImage->CommitBits();
	if (!theImage->mHasAlpha && !theImage->mHasTrans && (gSupportedPixelFormats & PixelFormat_R5G6B5))
	{
		if (!(theImage->mRenderFlags & RenderImageFlag_UseA8R8G8B8))
			aFormat = PixelFormat_R5G6B5;
	}

	if (theImage->mColorIndices != nullptr && (gSupportedPixelFormats & PixelFormat_Palette8))
	{
	}

	if ((theImage->mRenderFlags & RenderImageFlag_UseA4R4G4B4) && aFormat==PixelFormat_A8R8G8B8 && (gSupportedPixelFormats & PixelFormat_A4R4G4B4))
		aFormat = PixelFormat_A4R4G4B4;

	if (aFormat==PixelFormat_A8R8G8B8 && !(gSupportedPixelFormats & PixelFormat_A8R8G8B8))
		aFormat = PixelFormat_A4R4G4B4;

	// Release texture if image size has changed
	bool createTextures = false;
	if (mWidth!=theImage->mWidth || mHeight!=theImage->mHeight || aFormat!=mPixelFormat || (theImage->mRenderFlags & RenderImageFlag_TextureMask)!=mImageFlags)
	{
		ReleaseTextures();

		mPixelFormat = aFormat;
		mImageFlags = theImage->mRenderFlags & RenderImageFlag_TextureMask;
		CreateTextureDimensions(theImage);
		createTextures = true;
	}

	int i,x,y;

	int aHeight = theImage->GetHeight();
	int aWidth = theImage->GetWidth();

	int aFormatSize = 4;
	GPU_TEXCOLOR aFormatType = GPU_RGBA8;
	if (aFormat==PixelFormat_R5G6B5)
	{
		aFormatSize = 2;
		aFormatType = GPU_RGB565;
	}
	else if (aFormat==PixelFormat_A4R4G4B4)
	{
		aFormatSize = 2;
		aFormatType = GPU_RGBA4;
	}

	i=0;
	for(y=0; y<aHeight; y+=mTexPieceHeight)
	{
		for(x=0; x<aWidth; x+=mTexPieceWidth, i++)
		{
			TextureDataPiece &aPiece = mTextures[i];
			if (createTextures)
			{
				C3D_TexInit(&aPiece.mTexture, aPiece.mWidth, aPiece.mHeight, aFormatType);
				mTexMemSize += aPiece.mWidth*aPiece.mHeight*aFormatSize;
			}

			CopyImageToTexture(&aPiece.mTexture, theImage,x,y,aPiece.mWidth,aPiece.mHeight,aFormat, createTextures);
		}
	}

	mWidth = theImage->mWidth;
	mHeight = theImage->mHeight;
	mBitsChangedCount = theImage->mBitsChangedCount;
	mPixelFormat = aFormat;
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TextureData::CheckCreateTextures(MemoryImage *theImage)
{
	if(mPixelFormat==PixelFormat_Unknown || theImage->mWidth != mWidth || theImage->mHeight != mHeight || theImage->mBitsChangedCount != mBitsChangedCount || (theImage->mRenderFlags & RenderImageFlag_TextureMask) != mImageFlags)
		CreateTextures(theImage);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
C3D_Tex* TextureData::GetTexture(int x, int y, int &width, int &height, float &u1, float &v1, float &u2, float &v2)
{
	int tx = x/mTexPieceWidth;
	int ty = y/mTexPieceHeight;

	TextureDataPiece &aPiece = mTextures[ty*mTexVecWidth + tx];

	int left = x%mTexPieceWidth;
	int top = y%mTexPieceHeight;
	int right = left+width;
	int bottom = top+height;

	if(right > aPiece.mWidth)
		right = aPiece.mWidth;

	if(bottom > aPiece.mHeight)
		bottom = aPiece.mHeight;

	width = right-left;
	height = bottom-top;

	u1 = (float)left/aPiece.mWidth;
	v1 = (float)top/aPiece.mHeight;
	u2 = (float)right/aPiece.mWidth;
	v2 = (float)bottom/aPiece.mHeight;

	return &aPiece.mTexture;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
C3D_Tex* TextureData::GetTextureF(float x, float y, float &width, float &height, float &u1, float &v1, float &u2, float &v2)
{
	int tx = x/mTexPieceWidth;
	int ty = y/mTexPieceHeight;

	TextureDataPiece &aPiece = mTextures[ty*mTexVecWidth + tx];

	float left = x - tx*mTexPieceWidth;
	float top = y - ty*mTexPieceHeight;
	float right = left+width;
	float bottom = top+height;

	if(right > aPiece.mWidth)
		right = aPiece.mWidth;

	if(bottom > aPiece.mHeight)
		bottom = aPiece.mHeight;

	width = right-left;
	height = bottom-top;

	u1 = (float)left/aPiece.mWidth;
	v1 = (float)top/aPiece.mHeight;
	u2 = (float)right/aPiece.mWidth;
	v2 = (float)bottom/aPiece.mHeight;

	return &aPiece.mTexture;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void SetLinearFilter(bool linear)
{
	if (gLinearFilter != linear)
	{
		gLinearFilter = linear;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TextureData::Blt(float theX, float theY, const Rect& theSrcRect, const Color& theColor)
{
	int srcLeft = theSrcRect.mX;
	int srcTop = theSrcRect.mY;
	int srcRight = srcLeft + theSrcRect.mWidth;
	int srcBottom = srcTop + theSrcRect.mHeight;
	int srcX, srcY;
	float dstX, dstY;
	int aWidth,aHeight;
	float u1,v1,u2,v2;

	srcY = srcTop;
	dstY = theY;

	uint32_t aColor = (theColor.mRed << 0) | (theColor.mGreen << 8) | (theColor.mBlue << 16) | (theColor.mAlpha << 24);

	if ((srcLeft >= srcRight) || (srcTop >= srcBottom))
		return;

	while(srcY < srcBottom)
	{
		srcX = srcLeft;
		dstX = theX;
		while(srcX < srcRight)
		{
			aWidth = srcRight-srcX;
			aHeight = srcBottom-srcY;
			C3D_Tex* aTexture = GetTexture(srcX, srcY, aWidth, aHeight, u1, v1, u2, v2);

			float x = dstX - 0.5f;
			float y = dstY - 0.5f;

			GLVertex aVertex[4] = {
				{ {x},        {y},         {0},{aColor},{u1},{v1} },
				{ {x},        {y+aHeight}, {0},{aColor},{u1},{v2} },
				{ {x+aWidth}, {y},         {0},{aColor},{u2},{v1} },
				{ {x+aWidth}, {y+aHeight}, {0},{aColor},{u2},{v2} }
			};

			SetTexture(aTexture);

			GfxBegin(GPU_TRIANGLE_STRIP);
			GfxAddVertices(aVertex, 4);
			GfxEnd();

			srcX += aWidth;
			dstX += aWidth;
		}

		srcY += aHeight;
		dstY += aHeight;
	}
}

static inline float GetCoord(const GLVertex& theVertex, int theCoord)
{
	switch (theCoord)
	{
	case 0: return theVertex.sx;
	case 1: return theVertex.sy;
	case 2: return theVertex.sz;
	case 3: return theVertex.tu;
	case 4: return theVertex.tv;
	default: return 0;
	}
}

static inline GLVertex Interpolate(const GLVertex &v1, const GLVertex &v2, float t)
{
	GLVertex aVertex = v1;
	aVertex.sx = v1.sx + t*(v2.sx-v1.sx);
	aVertex.sy = v1.sy + t*(v2.sy-v1.sy);
	aVertex.tu = v1.tu + t*(v2.tu-v1.tu);
	aVertex.tv = v1.tv + t*(v2.tv-v1.tv);
	if (v1.color!=v2.color)
	{
		int r = ((v1.color >> 0) & 0xff) + t*( ((v2.color >> 0) & 0xff) - ((v1.color >> 0) & 0xff) );
		int g = ((v1.color >> 8) & 0xff) + t*( ((v2.color >> 8) & 0xff) - ((v1.color >> 8) & 0xff) );
		int b = ((v1.color >> 16) & 0xff) + t*( ((v2.color >> 16) & 0xff) - ((v1.color >> 16) & 0xff) );
		int a = ((v1.color >> 24) & 0xff) + t*( ((v2.color >> 24) & 0xff) - ((v1.color >> 24) & 0xff) );
		aVertex.color = (r << 0) | (g << 8) | (b << 16) | (a << 24);
	}

	return aVertex;
}

template<class Pred>
struct PointClipper
{
	Pred mPred;

	void ClipPoint(int n, float clipVal, const GLVertex& v1, const GLVertex& v2, VertexList& out)
	{
		if (!mPred(GetCoord(v1, n), clipVal))
		{
			if (!mPred(GetCoord(v2, n), clipVal)) // both inside
				out.push_back(v2);
			else // inside -> outside
			{
				float t = (clipVal - GetCoord(v1, n)) / (GetCoord(v2, n) - GetCoord(v1, n));
				out.push_back(Interpolate(v1, v2, t));
			}
		}
		else
		{
			if (!mPred(GetCoord(v2, n), clipVal)) // outside -> inside
			{
				float t = (clipVal - GetCoord(v1, n)) / (GetCoord(v2, n) - GetCoord(v1, n));
				out.push_back(Interpolate(v1, v2, t));
				out.push_back(v2);
			}
			//			else // outside -> outside
		}
	}

	void ClipPoints(int n, float clipVal, VertexList& in, VertexList& out)
	{
		if (in.size() < 2)
			return;

		ClipPoint(n, clipVal, in[in.size() - 1], in[0], out);
		for (VertexList::size_type i = 0; i < in.size() - 1; i++)
			ClipPoint(n, clipVal, in[i], in[i + 1], out);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void DrawPolyClipped(const Rect *theClipRect, const VertexList &theList)
{
	VertexList l1, l2;
	l1 = theList;

	int left = theClipRect->mX;
	int right = left + theClipRect->mWidth;
	int top = theClipRect->mY;
	int bottom = top + theClipRect->mHeight;

	VertexList *in = &l1, *out = &l2;
	PointClipper<std::less<float> > aLessClipper;
	PointClipper<std::greater_equal<float> > aGreaterClipper;

	aLessClipper.ClipPoints(0,left,*in,*out); std::swap(in,out); out->clear();
	aLessClipper.ClipPoints(1,top,*in,*out); std::swap(in,out); out->clear();
	aGreaterClipper.ClipPoints(0,right,*in,*out); std::swap(in,out); out->clear();
	aGreaterClipper.ClipPoints(1,bottom,*in,*out);

	VertexList &aList = *out;

	if (aList.size() >= 3)
	{
		GfxBegin(GPU_TRIANGLE_FAN);
		GfxAddVertices(aList);
		GfxEnd();
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void DoPolyTextureClip(VertexList &theList)
{
	VertexList l2;

	float left = 0;
	float right = 1;
	float top = 0;
	float bottom = 1;

	VertexList *in = &theList, *out = &l2;
	PointClipper<std::less<float> > aLessClipper;
	PointClipper<std::greater_equal<float> > aGreaterClipper;

	aLessClipper.ClipPoints(3,left,*in,*out); std::swap(in,out); out->clear();
	aLessClipper.ClipPoints(4,top,*in,*out); std::swap(in,out); out->clear();
	aGreaterClipper.ClipPoints(3,right,*in,*out); std::swap(in,out); out->clear();
	aGreaterClipper.ClipPoints(4,bottom,*in,*out);
}


void TextureData::BltTransformed(const SexyMatrix3 &theTrans, const Rect& theSrcRect, const Color& theColor, const Rect *theClipRect, float theX, float theY, bool center)
{
	int srcLeft = theSrcRect.mX;
	int srcTop = theSrcRect.mY;
	int srcRight = srcLeft + theSrcRect.mWidth;
	int srcBottom = srcTop + theSrcRect.mHeight;
	int srcX, srcY;
	float dstX, dstY;
	int aWidth;
	int aHeight;
	float u1,v1,u2,v2;
	float startx = 0, starty = 0;
	float pixelcorrect = 0.5f;

	if (center)
	{
		startx = -theSrcRect.mWidth/2.0f;
		starty = -theSrcRect.mHeight/2.0f;
		pixelcorrect = 0.0f;
	}			

	srcY = srcTop;
	dstY = starty;

	uint32_t aColor = (theColor.mRed << 0) | (theColor.mGreen << 8) | (theColor.mBlue << 16) | (theColor.mAlpha << 24);

	if ((srcLeft >= srcRight) || (srcTop >= srcBottom))
		return;

	while(srcY < srcBottom)
	{
		srcX = srcLeft;
		dstX = startx;
		while(srcX < srcRight)
		{
			aWidth = srcRight-srcX;
			aHeight = srcBottom-srcY;
			C3D_Tex* aTexture = GetTexture(srcX, srcY, aWidth, aHeight, u1, v1, u2, v2);

			float x = dstX; // - 0.5f;
			float y = dstY; // - 0.5f;

			SexyVector2 p[4] = { SexyVector2(x, y), SexyVector2(x,y+aHeight), SexyVector2(x+aWidth, y) , SexyVector2(x+aWidth, y+aHeight) };
			SexyVector2 tp[4];

			int i;
			for (i=0; i<4; i++)
			{
				tp[i] = theTrans*p[i];
				tp[i].x -= pixelcorrect - theX;
				tp[i].y -= pixelcorrect - theY;
			}

			bool clipped = false;
			if (theClipRect != nullptr)
			{
				int left = theClipRect->mX;
				int right = left + theClipRect->mWidth;
				int top = theClipRect->mY;
				int bottom = top + theClipRect->mHeight;
				for (i=0; i<4; i++)
				{
					if (tp[i].x<left || tp[i].x>=right || tp[i].y<top || tp[i].y>=bottom)
					{
						clipped = true;
						break;
					}
				}
			}

			GLVertex aVertex[4] = {
				{ {tp[0].x},{tp[0].y},{0},{aColor},{u1},{v1} },
				{ {tp[1].x},{tp[1].y},{0},{aColor},{u1},{v2} },
				{ {tp[2].x},{tp[2].y},{0},{aColor},{u2},{v1} },
				{ {tp[3].x},{tp[3].y},{0},{aColor},{u2},{v2} }
			};

			SetTexture(aTexture);

			if (!clipped)
			{
				GfxBegin(GPU_TRIANGLE_STRIP);
				GfxAddVertices(aVertex, 4);
				GfxEnd();
			}
			else
			{
				VertexList aList;
				aList.push_back(aVertex[0]);
				aList.push_back(aVertex[1]);
				aList.push_back(aVertex[3]);
				aList.push_back(aVertex[2]);

				DrawPolyClipped(theClipRect, aList);
			}

			srcX += aWidth;
			dstX += aWidth;
		}

		srcY += aHeight;
		dstY += aHeight;
	}
}

void TextureData::BltTriangles(const TriVertex theVertices[][3], int theNumTriangles, unsigned int theColor, float tx, float ty)
{
	if ((mMaxTotalU <= 1.0) && (mMaxTotalV <= 1.0))
	{
		SetTexture(&mTextures[0].mTexture);

		GfxBegin(GPU_TRIANGLES);
		GfxAddVertices(theVertices, theNumTriangles, theColor, tx, ty, mMaxTotalU, mMaxTotalV);
		GfxEnd();
	}
	else
	{
		for (int aTriangleNum = 0; aTriangleNum < theNumTriangles; aTriangleNum++)
		{
			TriVertex* aTriVerts = (TriVertex*) theVertices[aTriangleNum];

			GLVertex aVertex[3] = {
				{ {aTriVerts[0].x + tx},{aTriVerts[0].y + ty},	{0},{GetColorFromTriVertex(aTriVerts[0],theColor)},	{aTriVerts[0].u*mMaxTotalU},{aTriVerts[0].v*mMaxTotalV} },
				{ {aTriVerts[1].x + tx},{aTriVerts[1].y + ty},	{0},{GetColorFromTriVertex(aTriVerts[1],theColor)},	{aTriVerts[1].u*mMaxTotalU},{aTriVerts[1].v*mMaxTotalV} },
				{ {aTriVerts[2].x + tx},{aTriVerts[2].y + ty},	{0},{GetColorFromTriVertex(aTriVerts[2],theColor)},	{aTriVerts[2].u*mMaxTotalU},{aTriVerts[2].v*mMaxTotalV} }
			};

			float aMinU = mMaxTotalU, aMinV = mMaxTotalV;
			float aMaxU = 0, aMaxV = 0;

			int i,j,k;
			for (i=0; i<3; i++)
			{
				if(aVertex[i].tu < aMinU)
					aMinU = aVertex[i].tu;

				if(aVertex[i].tv < aMinV)
					aMinV = aVertex[i].tv;

				if(aVertex[i].tu > aMaxU)
					aMaxU = aVertex[i].tu;

				if(aVertex[i].tv > aMaxV)
					aMaxV = aVertex[i].tv;
			}

			VertexList aMasterList;
			aMasterList.push_back(aVertex[0]);
			aMasterList.push_back(aVertex[1]);
			aMasterList.push_back(aVertex[2]);


			VertexList aList;

			int aLeft = floorf(aMinU);
			int aTop = floorf(aMinV);
			int aRight = ceilf(aMaxU);
			int aBottom = ceilf(aMaxV);
			if (aLeft < 0)
				aLeft = 0;
			if (aTop < 0)
				aTop = 0;
			if (aRight > mTexVecWidth)
				aRight = mTexVecWidth;
			if (aBottom > mTexVecHeight)
				aBottom = mTexVecHeight;

			TextureDataPiece &aStandardPiece = mTextures[0];
			for (i=aTop; i<aBottom; i++)
			{
				for (j=aLeft; j<aRight; j++)
				{
					TextureDataPiece &aPiece = mTextures[i*mTexVecWidth + j];


					VertexList aList = aMasterList;
					for(k=0; k<3; k++)
					{
						aList[k].tu -= j;
						aList[k].tv -= i;
						if (i==mTexVecHeight-1)
							aList[k].tv *= (float)aStandardPiece.mHeight / aPiece.mHeight;
						if (j==mTexVecWidth-1)
							aList[k].tu *= (float)aStandardPiece.mWidth / aPiece.mWidth;
					}

					DoPolyTextureClip(aList);
					if (aList.size() >= 3)
					{
						SetTexture(&aPiece.mTexture);
						GfxBegin(GPU_TRIANGLE_FAN);
						GfxAddVertices(aList);
						GfxEnd();
					}
				}
			}
		}
	}
}


GLInterface::GLInterface(SexyAppBase* theApp)
{
	mApp = theApp;
	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
	mDisplayWidth = mWidth;
	mDisplayHeight = mHeight;

	mPresentationRect = Rect( 0, 0, mWidth, mHeight );

	mRefreshRate = 60;
	mMillisecondsPerFrame = 1000/mRefreshRate;

	mScreenImage = 0;

	mNextCursorX = 0;
	mNextCursorY = 0;
	mCursorX = 0;
	mCursorY = 0;

	gVertexMode = -1;
	gNumVertices = 0;
	gStartVertex = 0;
	gVertices = (GLVertex*)linearAlloc(sizeof(GLVertex) * MAX_VERTICES);
	memset(gVertices, 0, sizeof(GLVertex) * MAX_VERTICES);
}

GLInterface::~GLInterface()
{
	Flush();

	ImageSet::iterator anItr;
	for(anItr = mImageSet.begin(); anItr != mImageSet.end(); ++anItr)
	{
		MemoryImage *anImage = *anItr;
		delete (TextureData*)anImage->mRenderData;
		anImage->mRenderData = nullptr;
	}

	delete[] gVertices;
}

void GLInterface::SetDrawMode(int theDrawMode)
{
	if (theDrawMode == Graphics::DRAWMODE_NORMAL)
	{
		C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
	}
	else // Additive
	{
		C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE, GPU_SRC_ALPHA, GPU_ONE);
	}
}

void GLInterface::AddGLImage(GLImage* theGLImage)
{
	std::scoped_lock anAutoCrit(mCritSect);

	mGLImageSet.insert(theGLImage);
}

void GLInterface::RemoveGLImage(GLImage* theGLImage)
{
	std::scoped_lock anAutoCrit(mCritSect);

	GLImageSet::iterator anItr = mGLImageSet.find(theGLImage);
	if (anItr != mGLImageSet.end())
		mGLImageSet.erase(anItr);
}

void GLInterface::Remove3DData(MemoryImage* theImage)
{
	if (theImage->mRenderData != nullptr)
	{
		delete (TextureData*)theImage->mRenderData;
		theImage->mRenderData = nullptr;

		std::scoped_lock aCrit(mCritSect); // Make images thread safe
		mImageSet.erase(theImage);
	}
}

GLImage* GLInterface::GetScreenImage()
{
	return mScreenImage;
}

void GLInterface::UpdateViewport()
{
	// Restrict to 4:3
	// https://bumbershootsoft.wordpress.com/2018/11/29/forcing-an-aspect-ratio-in-3d-with-opengl/

	int viewport_x = 0;
	int viewport_y = 0;
	int width = 320;
	int height = 240;
	int viewport_width = width;
	int viewport_height = height;
	if (width * 3 > height * 4)
	{
		viewport_width = height * 4 / 3;
		viewport_x = (width - viewport_width) / 2;
	}
	else if (width * 3 < height * 4)
	{
		viewport_height = width * 3 / 4;
		viewport_y = (height - viewport_height) / 2;
	}
	C3D_SetViewport(viewport_x, viewport_y, viewport_width, viewport_height);
	mPresentationRect = Rect( viewport_x, viewport_y, viewport_width, viewport_height );

	//glClear(GL_COLOR_BUFFER_BIT);
}

int GLInterface::Init(bool IsWindowed)
{
	static bool inited = false;
	if (!inited)
	{
		inited = true;

		// Initialize the render target
		bottomTarget = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
		C3D_RenderTargetSetOutput(bottomTarget, GFX_BOTTOM, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

		u32 color = ((u8)(0*255) << 24) | ((u8)(0*255) << 16) | ((u8)(0*255) << 8) | 0xFF;
		C3D_RenderTargetClear(bottomTarget, C3D_CLEAR_ALL, color, 0);

		for (int i=0; i<2; i++)
		{
			u32* bin = (!i) ? (u32*)colored_shbin : (u32*)textured_shbin;
			u32 binsize = (!i) ? colored_shbin_size : textured_shbin_size;

			// Load the vertex shader, create a shader program and bind it
			shaders[i].dvlb = DVLB_ParseFile(bin, binsize);
			shaderProgramInit(&shaders[i].program);
			shaderProgramSetVsh(&shaders[i].program, &shaders[i].dvlb->DVLE[0]);
			C3D_BindProgram(&shaders[i].program);

			// Get the location of the uniform
			shaders[i].uf_projection = shaderInstanceGetUniformLocation(shaders[i].program.vertexShader, "projection");
		}
	}

	Mtx_OrthoTilt(&projection, 0, mWidth-1, mHeight-1, 0, -10.0f, 10.0f, true);
	for (int i=0; i<2; i++)
		C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, shaders[i].uf_projection, &projection);

	// Use untextured shader by default
	SetTexture(0, true);

	gTextureSizeMustBePow2 = false;
	gMinTextureWidth = 8;
	gMinTextureHeight = 8;
	gMaxTextureWidth = 512;
	gMaxTextureHeight = 512;
	gSupportedPixelFormats = PixelFormat_A8R8G8B8 | PixelFormat_A4R4G4B4 | PixelFormat_R5G6B5 | PixelFormat_Palette8;
	gLinearFilter = false;

	C3D_AlphaTest(true, GPU_GREATER, 0);
	C3D_BlendingColor(0);
	C3D_DepthTest(false, GPU_ALWAYS, GPU_WRITE_ALL);
	C3D_CullFace(GPU_CULL_NONE);

	mRGBBits = 32;

	mRedBits = 8;
	mGreenBits = 8;
	mBlueBits = 8;
	
	mRedShift = 0;
	mGreenShift = 8;
	mBlueShift = 16;

	mRedMask = (0xFFU << mRedShift);
	mGreenMask = (0xFFU << mGreenShift);
	mBlueMask = (0xFFU << mBlueShift);

	SetVideoOnlyDraw(false);

	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C3D_FrameDrawOn(bottomTarget);

	return 1;
}

bool GLInterface::Redraw(Rect* theClipRect)
{
	Flush();
	return true;
}

void GLInterface::SetVideoOnlyDraw(bool videoOnly)
{
	if (mScreenImage) delete mScreenImage;
	mScreenImage = new GLImage(this);
	//mScreenImage->SetSurface(useSecondary ? mSecondarySurface : mDrawSurface);		
	//mScreenImage->mNoLock = mVideoOnlyDraw;
	//mScreenImage->mVideoMemory = mVideoOnlyDraw;
	mScreenImage->mWidth = mWidth;
	mScreenImage->mHeight = mHeight;
	mScreenImage->SetImageMode(false, false);
}

void GLInterface::SetCursorPos(int theCursorX, int theCursorY)
{
	mNextCursorX = theCursorX;
	mNextCursorY = theCursorY;
}

bool GLInterface::PreDraw()
{
	gLinearFilter = false;
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	return true;
}

void GLInterface::Flush()
{
	C3D_FrameEnd(0);
	gNumVertices = 0;
	gStartVertex = 0;

	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C3D_FrameDrawOn(bottomTarget);
}

bool GLInterface::CreateImageTexture(MemoryImage *theImage)
{
	bool wantPurge = false;

	if(theImage->mRenderData==nullptr)
	{
		theImage->mRenderData = new TextureData();
		
		// The actual purging was deferred
		wantPurge = theImage->mPurgeBits;

		std::scoped_lock aCrit(mCritSect); // Make images thread safe
		mImageSet.insert(theImage);
	}

	TextureData *aData = (TextureData*)theImage->mRenderData;
	aData->CheckCreateTextures(theImage);

	if (wantPurge)
		theImage->PurgeBits();

	return aData->mPixelFormat != PixelFormat_Unknown;
}

bool GLInterface::RecoverBits(MemoryImage* theImage)
{
	if (theImage->mRenderData == nullptr)
		return false;

	TextureData* aData = (TextureData*) theImage->mRenderData;
	if (aData->mBitsChangedCount != theImage->mBitsChangedCount) // bits have changed since texture was created
		return false;

	Sexy::PrintF("recover");
	for (int aPieceRow = 0; aPieceRow < aData->mTexVecHeight; aPieceRow++)
	{
		for (int aPieceCol = 0; aPieceCol < aData->mTexVecWidth; aPieceCol++)
		{
			TextureDataPiece* aPiece = &aData->mTextures[aPieceRow*aData->mTexVecWidth + aPieceCol];

			int offx = aPieceCol*aData->mTexPieceWidth;
			int offy = aPieceRow*aData->mTexPieceHeight;
			int aWidth = std::min(theImage->mWidth-offx, aPiece->mWidth);
			int aHeight = std::min(theImage->mHeight-offy, aPiece->mHeight);

			//glEnable(GL_TEXTURE_2D);
			//glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_2D, aPiece->mTexture);

			//glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, theImage->GetBits());
		}
	}

	return true;
}

void GLInterface::PushTransform(const SexyMatrix3 &theTransform, bool concatenate)
{
	if (mTransformStack.empty() || !concatenate)
		mTransformStack.push_back(theTransform);
	else
	{
		SexyMatrix3 &aTrans = mTransformStack.back();
		mTransformStack.push_back(theTransform*aTrans);
	}
}

void GLInterface::PopTransform()
{
	if (!mTransformStack.empty())
		mTransformStack.pop_back();
}

void GLInterface::Blt(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter)
{
	if (!mTransformStack.empty())
	{
		BltClipF(theImage,theX,theY,theSrcRect,nullptr,theColor,theDrawMode);
		return;
	}

	if (!PreDraw())
		return;

	MemoryImage* aSrcMemoryImage = (MemoryImage*) theImage;

	if (!CreateImageTexture(aSrcMemoryImage))
		return;

	SetDrawMode(theDrawMode);

	TextureData *aData = (TextureData*)aSrcMemoryImage->mRenderData;

	SetLinearFilter(linearFilter);
	aData->Blt(theX,theY,theSrcRect,theColor);
}

void GLInterface::BltClipF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect *theClipRect, const Color& theColor, int theDrawMode)
{
	SexyTransform2D aTransform;
	aTransform.Translate(theX, theY);

	BltTransformed(theImage,theClipRect,theColor,theDrawMode,theSrcRect,aTransform,true);
}

void GLInterface::BltMirror(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter)
{
	SexyTransform2D aTransform;		

	aTransform.Translate(-theSrcRect.mWidth,0);
	aTransform.Scale(-1, 1);
	aTransform.Translate(theX, theY);

	BltTransformed(theImage,nullptr,theColor,theDrawMode,theSrcRect,aTransform,linearFilter);
}

void GLInterface::StretchBlt(Image* theImage,  const Rect& theDestRect, const Rect& theSrcRect, const Rect* theClipRect, const Color &theColor, int theDrawMode, bool fastStretch, bool mirror)
{
	float xScale = (float)theDestRect.mWidth / theSrcRect.mWidth;
	float yScale = (float)theDestRect.mHeight / theSrcRect.mHeight;

	SexyTransform2D aTransform;
	if (mirror)
	{
		aTransform.Translate(-theSrcRect.mWidth,0);
		aTransform.Scale(-xScale, yScale);
	}
	else
		aTransform.Scale(xScale, yScale);

	aTransform.Translate(theDestRect.mX, theDestRect.mY);
	BltTransformed(theImage,theClipRect,theColor,theDrawMode,theSrcRect,aTransform,!fastStretch);
}

void GLInterface::BltRotated(Image* theImage, float theX, float theY, const Rect* theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY, const Rect& theSrcRect)
{
	SexyTransform2D aTransform;

	aTransform.Translate(-theRotCenterX, -theRotCenterY);
	aTransform.RotateRad(theRot);
	aTransform.Translate(theX+theRotCenterX,theY+theRotCenterY);

	BltTransformed(theImage,theClipRect,theColor,theDrawMode,theSrcRect,aTransform,true);
}

void GLInterface::BltTransformed(Image* theImage, const Rect* theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, const SexyMatrix3 &theTransform, bool linearFilter, float theX, float theY, bool center)
{
	if (!PreDraw())
		return;

	MemoryImage* aSrcMemoryImage = (MemoryImage*) theImage;

	if (!CreateImageTexture(aSrcMemoryImage))
		return;

	SetDrawMode(theDrawMode);

	TextureData *aData = (TextureData*)aSrcMemoryImage->mRenderData;

	if (!mTransformStack.empty())
	{
		SetLinearFilter(true); // force linear filtering in the case of a global transform
		if (theX!=0 || theY!=0)
		{
			SexyTransform2D aTransform;
			if (center)
				aTransform.Translate(-theSrcRect.mWidth/2.0f,-theSrcRect.mHeight/2.0f);

			aTransform = theTransform * aTransform;
			aTransform.Translate(theX,theY);
			aTransform = mTransformStack.back() * aTransform;

			aData->BltTransformed(aTransform, theSrcRect, theColor, theClipRect);
		}
		else
		{
			SexyTransform2D aTransform = mTransformStack.back()*theTransform;
			aData->BltTransformed(aTransform, theSrcRect, theColor, theClipRect, theX, theY, center);
		}
	}
	else
	{
		SetLinearFilter(linearFilter);
		aData->BltTransformed(theTransform, theSrcRect, theColor, theClipRect, theX, theY, center);
	}
}

void GLInterface::DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode)
{
	if (!PreDraw())
		return;

	SetDrawMode(theDrawMode);

	float x1, y1, x2, y2;

	if (!mTransformStack.empty())
	{
		SexyVector2 p1(theStartX,theStartY);
		SexyVector2 p2(theEndX,theEndY);
		p1 = mTransformStack.back()*p1;
		p2 = mTransformStack.back()*p2;

		x1 = p1.x;
		y1 = p1.y;
		x2 = p2.x;
		y2 = p2.y;
	}
	else
	{
		x1 = theStartX;
		y1 = theStartY;
		x2 = theEndX;
		y2 = theEndY;
	}

	SetTexture(0);

	GLVertex aVertex[3] = {
		{ {x1},{y1},{0},{theColor.ToInt()},{0},{0} },
		{ {x2},{y2},{0},{theColor.ToInt()},{0},{0} },
		{ {x2},{y2},{0},{theColor.ToInt()},{0},{0} },
	};

	GfxBegin(GPU_TRIANGLES);
	GfxAddVertices(aVertex, 3);
	GfxEnd();
}

void GLInterface::FillRect(const Rect& theRect, const Color& theColor, int theDrawMode)
{
	if (!PreDraw())
		return;

	SetDrawMode(theDrawMode);

	float x = theRect.mX - 0.5f;
	float y = theRect.mY - 0.5f;
	float aWidth = theRect.mWidth;
	float aHeight = theRect.mHeight;

	GLVertex aVertex[4] = {
		{ {x},        {y},         {0},{theColor.ToInt()},{0},{0} },
		{ {x},        {y+aHeight}, {0},{theColor.ToInt()},{0},{0} },
		{ {x+aWidth}, {y},         {0},{theColor.ToInt()},{0},{0} },
		{ {x+aWidth}, {y+aHeight}, {0},{theColor.ToInt()},{0},{0} }
	};

	if (!mTransformStack.empty())
	{
		SexyVector2 p[4] = { SexyVector2(x, y), SexyVector2(x,y+aHeight), SexyVector2(x+aWidth, y) , SexyVector2(x+aWidth, y+aHeight) };

		int i;
		for (i=0; i<4; i++)
		{
			p[i] = mTransformStack.back()*p[i];
			p[i].x -= 0.5f;
			p[i].y -= 0.5f;
			aVertex[i].sx = p[i].x;
			aVertex[i].sy = p[i].y;
		}
	}

	SetTexture(0);

	GfxBegin(GPU_TRIANGLE_STRIP);
	GfxAddVertices(aVertex, 4);
	GfxEnd();
}

void GLInterface::DrawTriangle(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor, int theDrawMode)
{
	if (!PreDraw())
		return;

	SetDrawMode(theDrawMode);

	unsigned int aColor = (theColor.mRed << 0) | (theColor.mGreen << 8) | (theColor.mBlue << 16) | (theColor.mAlpha << 24);
	unsigned int col1 = GetColorFromTriVertex(p1, aColor);
	unsigned int col2 = GetColorFromTriVertex(p1, aColor);
	unsigned int col3 = GetColorFromTriVertex(p1, aColor);

	SetTexture(0);

	GLVertex aVertex[3] = {
		{ {p1.x}, {p1.y}, {0}, {col1}, {0},{0} },
		{ {p2.x}, {p2.y}, {0}, {col2}, {0},{0} },
		{ {p3.x}, {p3.y}, {0}, {col3}, {0},{0} },
	};

	GfxBegin(GPU_TRIANGLE_STRIP);
	GfxAddVertices(aVertex, 3);
	GfxEnd();
}

void GLInterface::DrawTriangleTex(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor, int theDrawMode, Image *theTexture, bool blend)
{
	TriVertex aVertices[1][3] = {{p1, p2, p3}};
	DrawTrianglesTex(aVertices,1,theColor,theDrawMode,theTexture,blend);
}

void GLInterface::DrawTrianglesTex(const TriVertex theVertices[][3], int theNumTriangles, const Color &theColor, int theDrawMode, Image *theTexture, float tx, float ty, bool blend)
{
	if (!PreDraw()) return;

	MemoryImage* aSrcMemoryImage = (MemoryImage*)theTexture;

	if (!CreateImageTexture(aSrcMemoryImage))
		return;

	SetDrawMode(theDrawMode);

	TextureData *aData = (TextureData*)aSrcMemoryImage->mRenderData;

	SetLinearFilter(blend);

	unsigned int aColor = (theColor.mRed << 0) | (theColor.mGreen << 8) | (theColor.mBlue << 16) | (theColor.mAlpha << 24);
	aData->BltTriangles(theVertices, theNumTriangles, aColor, tx, ty);
}

void GLInterface::DrawTrianglesTexStrip(const TriVertex theVertices[], int theNumTriangles, const Color &theColor, int theDrawMode, Image *theTexture, float tx, float ty, bool blend)
{
	TriVertex aList[100][3];
	int aTriNum = 0;
	while (aTriNum < theNumTriangles)
	{
		int aMaxTriangles = std::min(100,theNumTriangles - aTriNum);
		for (int i=0; i<aMaxTriangles; i++)
		{
			aList[i][0] = theVertices[aTriNum];
			aList[i][1] = theVertices[aTriNum+1];
			aList[i][2] = theVertices[aTriNum+2];
			aTriNum++;
		}
		DrawTrianglesTex(aList,aMaxTriangles,theColor,theDrawMode,theTexture, tx, ty, blend);
	}
}

void GLInterface::FillPoly(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty)
{
	if (theNumVertices<3)
		return;

	if (!PreDraw())
		return;

	SetDrawMode(theDrawMode);
	unsigned int aColor = (theColor.mRed << 0) | (theColor.mGreen << 8) | (theColor.mBlue << 16) | (theColor.mAlpha << 24);

	SetTexture(0);

	VertexList aList;
	for (int i=0; i<theNumVertices; i++)
	{
		GLVertex vert = { {theVertices[i].mX + (float)tx}, {theVertices[i].mY + (float)ty}, {0}, {aColor}, {0}, {0} };
		if (!mTransformStack.empty())
		{
			SexyVector2 v(vert.sx,vert.sy);
			v = mTransformStack.back()*v;
			vert.sx = v.x;
			vert.sy = v.y;
		}

		aList.push_back(vert);
	}

	if (theClipRect != nullptr)
		DrawPolyClipped(theClipRect,aList);
	else
	{
		GfxBegin(GPU_TRIANGLE_FAN);
		GfxAddVertices(aList);
		GfxEnd();
	}
}

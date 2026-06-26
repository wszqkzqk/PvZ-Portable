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

#define GLAD_GLES2_IMPLEMENTATION
#include "graphics/GLPlatform.h"

#include <SDL.h>

#include "graphics/GLInterface.h"
#include "graphics/GLImage.h"
#include "graphics/Graphics.h"
#include "graphics/MemoryImage.h"
#include "SexyAppBase.h"
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <vector>

#define MAX_VERTICES 16384

#ifndef GL_FRAMEBUFFER_SRGB
#define GL_FRAMEBUFFER_SRGB 0x8DB9 // Not in GLES 2.0 headers, but needed to disable sRGB on Windows.
#endif

using namespace Sexy;

bool gDesktopGLFallback = false;

static inline uint32_t ArgbToRgba(uint32_t argb) noexcept
{
	uint32_t abgr = (argb & 0xFF00FF00u)
					| ((argb >> 16) & 0x000000FFu)
					| ((argb << 16) & 0x00FF0000u);
	return ToLE32(abgr);
}

static inline uint32_t RgbaToArgb(uint32_t rgba) noexcept
{
	uint32_t abgr = FromLE32(rgba);
	return (abgr & 0xFF00FF00u)
		| ((abgr >> 16) & 0x000000FFu)
		| ((abgr << 16) & 0x00FF0000u);
}

static inline uint32_t VertexColor(uint32_t triVertexColor, uint32_t fallback) noexcept
{
	return triVertexColor ? ArgbToRgba(triVertexColor) : fallback;
}

#define GetColorFromTriVertex(v, c) VertexColor((v).color, (c))

static int gMinTextureWidth;
static int gMinTextureHeight;
static int gMaxTextureWidth;
static int gMaxTextureHeight;
static int gSupportedPixelFormats;
static bool gTextureSizeMustBePow2;
static int MAX_TEXTURE_SIZE;
static bool gLinearFilter = false;

static std::vector<GLVertex> gVertices;
static int gNumVertices;
static GLenum gVertexMode;
static GLuint gProgram;
static GLuint gVbo;
static GLint gUfViewProjMtx, gUfTexture, gUfUseTexture, gUfUvBounds, gUfClampUvEnabled;

static void GfxBegin(GLenum vertexMode)
{
	if (gVertexMode != (GLenum)-1) return;
	gVertexMode = vertexMode;
}

static void GfxEnd()
{
	if (gVertexMode == (GLenum)-1) return;

	if (gNumVertices > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, gVbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLVertex) * gNumVertices, gVertices.data(), GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT,         GL_FALSE, sizeof(GLVertex), (const void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE,  GL_TRUE,  sizeof(GLVertex), (const void*)(sizeof(float)*3));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT,         GL_FALSE, sizeof(GLVertex), (const void*)(sizeof(float)*3 + sizeof(uint32_t)));
		glEnableVertexAttribArray(2);

		glDrawArrays(gVertexMode, 0, gNumVertices);
	}

	gVertexMode = (GLenum)-1;
	gNumVertices = 0;
	gVertices.clear();
}

static void GfxFlushIfOverBudget()
{
	if (gVertexMode == (GLenum)-1 || gNumVertices < MAX_VERTICES) return;
	GLenum oldMode = gVertexMode;
	GfxEnd();
	GfxBegin(oldMode);
}

static void GfxAddVertices(const GLVertex *arr, int arrCount)
{
	if (gVertexMode == (GLenum)-1) return;
	if (arrCount <= 0) return;

	const int oldCount = gNumVertices;
	gNumVertices += arrCount;
	gVertices.resize(gNumVertices);
	memcpy(gVertices.data() + oldCount, arr, sizeof(GLVertex) * arrCount);

	GfxFlushIfOverBudget();
}

static void GfxAddVertices(VertexList &arr)
{
	GfxAddVertices(arr.mVerts, (int)arr.size());
}

static void GfxAddVertices(const TriVertex arr[][3], int arrCount, unsigned int theColor,
                           float tx, float ty, float aMaxTotalU, float aMaxTotalV)
{
	if (gVertexMode == (GLenum)-1) return;
	if (arrCount <= 0) return;

	const int oldCount = gNumVertices;
	gNumVertices += arrCount * 3;
	gVertices.resize(gNumVertices);

	GLVertex* dst = gVertices.data() + oldCount;
	for (int tri = 0; tri < arrCount; tri++)
	{
		const TriVertex* v = arr[tri];
		for (int i = 0; i < 3; i++)
		{
			dst[i].sx    = v[i].x + tx;
			dst[i].sy    = v[i].y + ty;
			dst[i].color = GetColorFromTriVertex(v[i], theColor);
			dst[i].tu    = v[i].u * aMaxTotalU;
			dst[i].tv    = v[i].v * aMaxTotalV;
		}
		dst += 3;
	}

	GfxFlushIfOverBudget();
}

// Unified GLSL body; VERT_IN / V2F / FRAG_OUT / TEX2D macros from GLPlatform.h.
static constexpr const char *SHADER_CODE = R"DELIMITER(
V2F vec4 v_color;
V2F vec2 v_uv;

#ifdef VERTEX
	uniform mat4 u_viewProj;
	VERT_IN vec3 a_position;
	VERT_IN vec4 a_color;
	VERT_IN vec2 a_uv;
	void main() {
		v_color = a_color;
		v_uv = a_uv;
		gl_Position = u_viewProj * vec4(a_position, 1.0);
	}
#endif
#ifdef FRAGMENT
	uniform sampler2D u_texture;
	uniform int u_useTexture;
	uniform vec4 u_uvBounds;
	uniform int u_clampUvEnabled;
	void main() {
		if (u_useTexture != 0) {
			vec2 uv = (u_clampUvEnabled != 0) ? clamp(v_uv, u_uvBounds.xy, u_uvBounds.zw) : v_uv;
			FRAG_OUT = TEX2D(u_texture, uv) * v_color;
		}
		else
			FRAG_OUT = v_color;
	}
#endif
)DELIMITER";

static GLuint shaderCompile(const char *src, uint32_t srcLen, GLenum type)
{
	// GLSL ES 1.00 for native ES contexts; GLSL 1.20 for desktop GL fallback.
	const char *versionLine = gDesktopGLFallback
		? "#version 120\n"
		: "#version 100\nprecision mediump float;\n";
	const char *macros = (type == GL_VERTEX_SHADER)
		? GLSL_VERT_MACROS "#define VERTEX\n"
		: GLSL_FRAG_MACROS "#define FRAGMENT\n";

	const GLchar *strings[3]  = { versionLine, macros, src };
	GLint         lengths[3]  = { (GLint)strlen(versionLine), (GLint)strlen(macros), (GLint)srcLen };

	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 3, strings, lengths);
	glCompileShader(shader);

	GLint ok;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (!ok)
	{
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		char *log = (char*)malloc(logLen);
		glGetShaderInfoLog(shader, logLen, &logLen, log);
		Sexy::PrintF("Shader error: %s\n%s%s%s\n", log, strings[0], strings[1], strings[2]);
		fflush(stdout);
		free(log);
		glDeleteShader(shader);
		if (gSexyAppBase != nullptr)
			gSexyAppBase->Shutdown();
		else
			exit(1);
		return 0;
	}
	return shader;
}

static GLuint shaderLoad(const char *src)
{
	GLuint vert = shaderCompile(src, strlen(src), GL_VERTEX_SHADER);
	GLuint frag = shaderCompile(src, strlen(src), GL_FRAGMENT_SHADER);
	if (vert == 0 || frag == 0)
	{
		if (vert != 0)
			glDeleteShader(vert);
		if (frag != 0)
			glDeleteShader(frag);
		return 0;
	}

	GLuint prog = glCreateProgram();
	glAttachShader(prog, vert);
	glAttachShader(prog, frag);

	const char *attribs[] = { "a_position", "a_color", "a_uv" };
	for (int i = 0; i < 3; i++)
		glBindAttribLocation(prog, i, attribs[i]);

	glLinkProgram(prog);
	glDeleteShader(vert);
	glDeleteShader(frag);
	return prog;
}

// Orthographic projection
static void MakeOrthoMatrix(float l, float r, float b, float t, float n, float f, float* m)
{
	m[0]  = 2.0f / (r - l);  m[1]  = 0;                m[2]  = 0;                 m[3]  = 0;
	m[4]  = 0;                m[5]  = 2.0f / (t - b);   m[6]  = 0;                 m[7]  = 0;
	m[8]  = 0;                m[9]  = 0;                 m[10] = -2.0f / (f - n);   m[11] = 0;
	m[12] = -(r+l)/(r-l);    m[13] = -(t+b)/(t-b);     m[14] = -(f+n)/(f-n);      m[15] = 1;
}

static void CopyImageToTexture8888(MemoryImage *img, int offx, int offy,
	int w, int h, int pitch, int dstH, bool padR, bool padB, bool create)
{
	std::vector<uint32_t> aDst(pitch * dstH);
	uint32_t *dst = aDst.data();

	if (img->mColorTable == nullptr)
	{
		uint32_t *srcRow = (uint32_t*)img->GetBits() + offy * img->GetWidth() + offx;
		uint32_t *dstRow = dst;
		for (int y = 0; y < h; y++)
		{
			uint32_t *s = srcRow, *d = dstRow;
			for (int x = 0; x < w; x++)
				*d++ = ArgbToRgba(*s++);
			if (padR) *d = *(d - 1);
			srcRow += img->GetWidth();
			dstRow += pitch;
		}
	}
	else
	{
		uint8_t  *srcRow = (uint8_t*)img->mColorIndices + offy * img->GetWidth() + offx;
		uint32_t *dstRow = dst;
		uint32_t *pal = (uint32_t*)img->mColorTable;
		for (int y = 0; y < h; y++)
		{
			uint8_t *s = srcRow; uint32_t *d = dstRow;
			for (int x = 0; x < w; x++)
				*d++ = ArgbToRgba(pal[*s++]);
			if (padR) *d = *(d - 1);
			srcRow += img->GetWidth();
			dstRow += pitch;
		}
	}

	if (padB)
	{
		uint32_t *lastRow = dst + pitch * (h - 1);
		for (int y = h; y < dstH; y++)
			memcpy(dst + pitch * y, lastRow, pitch * sizeof(uint32_t));
	}

	if (create)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pitch, dstH, 0, GL_RGBA, GL_UNSIGNED_BYTE, dst);
	else
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pitch, dstH, GL_RGBA, GL_UNSIGNED_BYTE, dst);
}

static void CopyImageToTexture4444(MemoryImage *img, int offx, int offy,
	int w, int h, int pitch, int dstH, bool padR, bool padB, bool create)
{
	std::vector<uint16_t> aDst(pitch * dstH);
	uint16_t *dst = aDst.data();

	auto argbTo4444 = [](uint32_t p) -> uint16_t {
		return static_cast<uint16_t>(
			((p >> 8)  & 0xF000) | ((p >> 4) & 0x0F00) |
			 (p        & 0x00F0) | ((p >> 28) & 0x000F));
	};

	if (img->mColorTable == nullptr)
	{
		uint32_t *srcRow = (uint32_t*)img->GetBits() + offy * img->GetWidth() + offx;
		uint16_t *dstRow = dst;
		for (int y = 0; y < h; y++)
		{
			uint32_t *s = srcRow; uint16_t *d = dstRow;
			for (int x = 0; x < w; x++)
				*d++ = argbTo4444(*s++);
			if (padR) *d = *(d - 1);
			srcRow += img->GetWidth();
			dstRow += pitch;
		}
	}
	else
	{
		uint8_t  *srcRow = (uint8_t*)img->mColorIndices + offy * img->GetWidth() + offx;
		uint16_t *dstRow = dst;
		uint32_t *pal = (uint32_t*)img->mColorTable;
		for (int y = 0; y < h; y++)
		{
			uint8_t *s = srcRow; uint16_t *d = dstRow;
			for (int x = 0; x < w; x++)
				*d++ = argbTo4444(pal[*s++]);
			if (padR) *d = *(d - 1);
			srcRow += img->GetWidth();
			dstRow += pitch;
		}
	}

	if (padB)
	{
		uint16_t *lastRow = dst + pitch * (h - 1);
		for (int y = h; y < dstH; y++)
			memcpy(dst + pitch * y, lastRow, pitch * sizeof(uint16_t));
	}

	if (create)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pitch, dstH, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, dst);
	else
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pitch, dstH, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, dst);
}

static void CopyImageToTexture565(MemoryImage *img, int offx, int offy,
	int w, int h, int pitch, int dstH, bool padR, bool padB, bool create)
{
	std::vector<uint16_t> aDst(pitch * dstH);
	uint16_t *dst = aDst.data();

	auto argbTo565 = [](uint32_t p) -> uint16_t {
		return static_cast<uint16_t>(
			((p >> 8) & 0xF800) | ((p >> 5) & 0x07E0) | ((p >> 3) & 0x001F));
	};

	if (img->mColorTable == nullptr)
	{
		uint32_t *srcRow = (uint32_t*)img->GetBits() + offy * img->GetWidth() + offx;
		uint16_t *dstRow = dst;
		for (int y = 0; y < h; y++)
		{
			uint32_t *s = srcRow; uint16_t *d = dstRow;
			for (int x = 0; x < w; x++)
				*d++ = argbTo565(*s++);
			if (padR) *d = *(d - 1);
			srcRow += img->GetWidth();
			dstRow += pitch;
		}
	}
	else
	{
		uint8_t  *srcRow = (uint8_t*)img->mColorIndices + offy * img->GetWidth() + offx;
		uint16_t *dstRow = dst;
		uint32_t *pal = (uint32_t*)img->mColorTable;
		for (int y = 0; y < h; y++)
		{
			uint8_t *s = srcRow; uint16_t *d = dstRow;
			for (int x = 0; x < w; x++)
				*d++ = argbTo565(pal[*s++]);
			if (padR) *d = *(d - 1);
			srcRow += img->GetWidth();
			dstRow += pitch;
		}
	}

	if (padB)
	{
		uint16_t *lastRow = dst + pitch * (h - 1);
		for (int y = h; y < dstH; y++)
			memcpy(dst + pitch * y, lastRow, pitch * sizeof(uint16_t));
	}

	if (create)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pitch, dstH, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, dst);
	else
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pitch, dstH, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, dst);
}

static void CopyImageToTexturePalette8(MemoryImage *img, int offx, int offy,
	int w, int h, int pitch, int dstH, bool padR, bool padB, bool create)
{
	std::vector<uint32_t> aDst(pitch * dstH);
	uint32_t *dst = aDst.data();
	uint8_t  *srcRow = (uint8_t*)img->mColorIndices + offy * img->GetWidth() + offx;
	uint32_t *dstRow = dst;
	uint32_t *pal = (uint32_t*)img->mColorTable;

	for (int y = 0; y < h; y++)
	{
		uint8_t *s = srcRow; uint32_t *d = dstRow;
		for (int x = 0; x < w; x++)
			*d++ = ArgbToRgba(pal[*s++]);
		if (padR) *d = *(d - 1);
		srcRow += img->GetWidth();
		dstRow += pitch;
	}

	if (padB)
	{
		uint32_t *lastRow = dst + pitch * (h - 1);
		for (int y = h; y < dstH; y++)
			memcpy(dst + pitch * y, lastRow, pitch * sizeof(uint32_t));
	}

	if (create)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pitch, dstH, 0, GL_RGBA, GL_UNSIGNED_BYTE, dst);
	else
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pitch, dstH, GL_RGBA, GL_UNSIGNED_BYTE, dst);
}

static void CopyImageToTexture(MemoryImage *img, int offx, int offy,
	int texW, int texH, PixelFormat fmt, bool create)
{
	GLint wrap = (img->mRenderFlags & RenderImageFlag_Repeat) ? GL_REPEAT : GL_CLAMP_TO_EDGE;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

	int w = std::min(texW, img->GetWidth()  - offx);
	int h = std::min(texH, img->GetHeight() - offy);
	bool padR = w < texW, padB = h < texH;

	if (w > 0 && h > 0)
	{
		switch (fmt)
		{
		case PixelFormat_A8R8G8B8: CopyImageToTexture8888    (img, offx, offy, w, h, texW, texH, padR, padB, create); break;
		case PixelFormat_A4R4G4B4: CopyImageToTexture4444    (img, offx, offy, w, h, texW, texH, padR, padB, create); break;
		case PixelFormat_R5G6B5:   CopyImageToTexture565     (img, offx, offy, w, h, texW, texH, padR, padB, create); break;
		case PixelFormat_Palette8: CopyImageToTexturePalette8(img, offx, offy, w, h, texW, texH, padR, padB, create); break;
		case PixelFormat_Unknown:  break;
		}
	}
}

static int GetClosestPowerOf2Above(int n)
{
	int p = 1;
	while (p < n) p <<= 1;
	return p;
}

static bool IsPowerOf2(int n)
{
	return n > 0 && (n & (n - 1)) == 0;
}

static void GetBestTextureDimensions(int &w, int &h, bool isEdge, bool usePow2, uint32_t flags)
{
	if (flags & RenderImageFlag_Use64By64Subdivisions) { w = h = 64; return; }

	static int* sGoodSizes = nullptr;
	if (!sGoodSizes)
	{
		sGoodSizes = new int[MAX_TEXTURE_SIZE];
		int pow2 = 1;
		for (int i = 0; i < MAX_TEXTURE_SIZE; i++)
		{
			if (i > pow2) pow2 <<= 1;
			int good = pow2;
			if (good - i > 64)
			{
				good >>= 1;
				while (true)
				{
					int leftover = i % good;
					if (leftover < 64 || IsPowerOf2(leftover)) break;
					good >>= 1;
				}
			}
			sGoodSizes[i] = good;
		}
	}

	int aw = w, ah = h;
	if (usePow2)
	{
		if (isEdge || (flags & RenderImageFlag_MinimizeNumSubdivisions))
		{
			aw = aw >= gMaxTextureWidth  ? gMaxTextureWidth  : GetClosestPowerOf2Above(aw);
			ah = ah >= gMaxTextureHeight ? gMaxTextureHeight : GetClosestPowerOf2Above(ah);
		}
		else
		{
			aw = aw >= gMaxTextureWidth  ? gMaxTextureWidth  : sGoodSizes[aw];
			ah = ah >= gMaxTextureHeight ? gMaxTextureHeight : sGoodSizes[ah];
		}
	}
	aw = std::max(aw, gMinTextureWidth);
	ah = std::max(ah, gMinTextureHeight);
	w = aw; h = ah;
}

TextureData::TextureData()
	: mWidth(0), mHeight(0), mTexVecWidth(0), mTexVecHeight(0),
	  mBitsChangedCount(0), mTexMemSize(0), mTexPieceWidth(64), mTexPieceHeight(64),
	  mPixelFormat(PixelFormat_Unknown), mImageFlags(0)
{
}

TextureData::~TextureData()
{
	ReleaseTextures();
}

void TextureData::ReleaseTextures()
{
	for (auto &piece : mTextures)
		glDeleteTextures(1, &piece.mTexture);
	mTextures.clear();
	mTexMemSize = 0;
}

void TextureData::CreateTextureDimensions(MemoryImage *theImage)
{
	int aWidth  = theImage->GetWidth();
	int aHeight = theImage->GetHeight();

	mTexPieceWidth  = aWidth;
	mTexPieceHeight = aHeight;
	GetBestTextureDimensions(mTexPieceWidth, mTexPieceHeight, false, true, mImageFlags);

	int aRightWidth  = aWidth % mTexPieceWidth;
	int aRightHeight = mTexPieceHeight;
	if (aRightWidth > 0)
		GetBestTextureDimensions(aRightWidth, aRightHeight, true, true, mImageFlags);
	else
		aRightWidth = mTexPieceWidth;

	int aBottomWidth  = mTexPieceWidth;
	int aBottomHeight = aHeight % mTexPieceHeight;
	if (aBottomHeight > 0)
		GetBestTextureDimensions(aBottomWidth, aBottomHeight, true, true, mImageFlags);
	else
		aBottomHeight = mTexPieceHeight;

	int aCornerWidth  = aRightWidth;
	int aCornerHeight = aBottomHeight;
	GetBestTextureDimensions(aCornerWidth, aCornerHeight, true, true, mImageFlags);

	mTexVecWidth  = (aWidth  + mTexPieceWidth  - 1) / mTexPieceWidth;
	mTexVecHeight = (aHeight + mTexPieceHeight - 1) / mTexPieceHeight;
	mTextures.resize(mTexVecWidth * mTexVecHeight);

	for (auto &p : mTextures)        { p.mTexture = 0; p.mWidth = mTexPieceWidth; p.mHeight = mTexPieceHeight; }
	for (unsigned i = mTexVecWidth - 1; i < mTextures.size(); i += mTexVecWidth)
	                                  { mTextures[i].mWidth = aRightWidth;  mTextures[i].mHeight = aRightHeight; }
	for (unsigned i = mTexVecWidth * (mTexVecHeight - 1); i < mTextures.size(); i++)
	                                  { mTextures[i].mWidth = aBottomWidth; mTextures[i].mHeight = aBottomHeight; }
	mTextures.back().mWidth  = aCornerWidth;
	mTextures.back().mHeight = aCornerHeight;

	mMaxTotalU = aWidth  / (float)mTexPieceWidth;
	mMaxTotalV = aHeight / (float)mTexPieceHeight;
}

void TextureData::CreateTextures(MemoryImage *theImage)
{
	theImage->DeleteSWBuffers();

	PixelFormat aFormat = PixelFormat_A8R8G8B8;
	theImage->CommitBits();

	if (!theImage->mHasAlpha && !theImage->mHasTrans
	    && (gSupportedPixelFormats & PixelFormat_R5G6B5)
	    && !(theImage->mRenderFlags & RenderImageFlag_UseA8R8G8B8))
		aFormat = PixelFormat_R5G6B5;

	if ((theImage->mRenderFlags & RenderImageFlag_UseA4R4G4B4)
	    && aFormat == PixelFormat_A8R8G8B8
	    && (gSupportedPixelFormats & PixelFormat_A4R4G4B4))
		aFormat = PixelFormat_A4R4G4B4;

	if (aFormat == PixelFormat_A8R8G8B8 && !(gSupportedPixelFormats & PixelFormat_A8R8G8B8))
		aFormat = PixelFormat_A4R4G4B4;

	bool createTextures = false;
	if (mWidth != theImage->mWidth || mHeight != theImage->mHeight
	    || aFormat != mPixelFormat
	    || (theImage->mRenderFlags & RenderImageFlag_TextureMask) != mImageFlags)
	{
		ReleaseTextures();
		mPixelFormat = aFormat;
		mImageFlags  = theImage->mRenderFlags & RenderImageFlag_TextureMask;
		CreateTextureDimensions(theImage);
		createTextures = true;
	}

	int aHeight = theImage->GetHeight();
	int aWidth  = theImage->GetWidth();
	int fmtSize = (aFormat == PixelFormat_R5G6B5 || aFormat == PixelFormat_A4R4G4B4) ? 2 : 4;

	int idx = 0;
	for (int y = 0; y < aHeight; y += mTexPieceHeight)
	{
		for (int x = 0; x < aWidth; x += mTexPieceWidth, idx++)
		{
			TextureDataPiece &piece = mTextures[idx];
			if (createTextures)
			{
				glGenTextures(1, &piece.mTexture);
				mTexMemSize += piece.mWidth * piece.mHeight * fmtSize;
			}
			glBindTexture(GL_TEXTURE_2D, piece.mTexture);
			CopyImageToTexture(theImage, x, y, piece.mWidth, piece.mHeight, aFormat, createTextures);
		}
	}

	mWidth  = theImage->mWidth;
	mHeight = theImage->mHeight;
	mBitsChangedCount = theImage->mBitsChangedCount;
	mPixelFormat = aFormat;
}

void TextureData::CheckCreateTextures(MemoryImage *theImage)
{
	if (mPixelFormat == PixelFormat_Unknown
	    || theImage->mWidth != mWidth || theImage->mHeight != mHeight
	    || theImage->mBitsChangedCount != mBitsChangedCount
	    || (theImage->mRenderFlags & RenderImageFlag_TextureMask) != mImageFlags)
		CreateTextures(theImage);
}

GLuint& TextureData::GetTexture(int x, int y, int &width, int &height,
                                float &u1, float &v1, float &u2, float &v2,
                                float *uvBounds)
{
	int tx = x / mTexPieceWidth, ty = y / mTexPieceHeight;
	TextureDataPiece &p = mTextures[ty * mTexVecWidth + tx];

	int left = x % mTexPieceWidth, top = y % mTexPieceHeight;
	int right  = std::min(left + width,  p.mWidth);
	int bottom = std::min(top  + height, p.mHeight);
	width  = right - left;
	height = bottom - top;

	u1 = (float)left  / p.mWidth;  v1 = (float)top    / p.mHeight;
	u2 = (float)right / p.mWidth;  v2 = (float)bottom / p.mHeight;

	// Half-texel inset for shader UV clamping; midpoint fallback guarantees min <= max.
	float halfU = 0.5f / p.mWidth, halfV = 0.5f / p.mHeight;
	float midU = (u1 + u2) * 0.5f, midV = (v1 + v2) * 0.5f;
	uvBounds[0] = std::min(u1 + halfU, midU);  uvBounds[1] = std::min(v1 + halfV, midV);
	uvBounds[2] = std::max(u2 - halfU, midU);  uvBounds[3] = std::max(v2 - halfV, midV);
	return p.mTexture;
}

GLuint& TextureData::GetTextureF(float x, float y, float &width, float &height,
                                 float &u1, float &v1, float &u2, float &v2,
                                 float *uvBounds)
{
	int tx = (int)(x / mTexPieceWidth), ty = (int)(y / mTexPieceHeight);
	TextureDataPiece &p = mTextures[ty * mTexVecWidth + tx];

	float left   = x - tx * mTexPieceWidth;
	float top    = y - ty * mTexPieceHeight;
	float right  = std::min(left + width,  (float)p.mWidth);
	float bottom = std::min(top  + height, (float)p.mHeight);
	width  = right - left;
	height = bottom - top;

	u1 = left   / p.mWidth;  v1 = top    / p.mHeight;
	u2 = right  / p.mWidth;  v2 = bottom / p.mHeight;

	float halfU = 0.5f / p.mWidth, halfV = 0.5f / p.mHeight;
	float midU = (u1 + u2) * 0.5f, midV = (v1 + v2) * 0.5f;
	uvBounds[0] = std::min(u1 + halfU, midU);  uvBounds[1] = std::min(v1 + halfV, midV);
	uvBounds[2] = std::max(u2 - halfU, midU);  uvBounds[3] = std::max(v2 - halfV, midV);
	return p.mTexture;
}

static void SetLinearFilter(bool linear)
{
	gLinearFilter = linear;
}

static constexpr float kDefaultUvBounds[4] = { 0.f, 0.f, 1.f, 1.f };

static void GfxBindTexture(GLuint tex, const float *uvBounds = kDefaultUvBounds, bool clampUv = true)
{
	glBindTexture(GL_TEXTURE_2D, tex);
	int f = gLinearFilter ? GL_LINEAR : GL_NEAREST;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, f);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, f);
	glUniform1i(gUfClampUvEnabled, clampUv ? 1 : 0);
	glUniform4fv(gUfUvBounds, 1, uvBounds);
}

void TextureData::Blt(float theX, float theY, const Rect& theSrcRect, const Color& theColor)
{
	int srcLeft   = theSrcRect.mX;
	int srcTop    = theSrcRect.mY;
	int srcRight  = srcLeft + theSrcRect.mWidth;
	int srcBottom = srcTop  + theSrcRect.mHeight;
	if (srcLeft >= srcRight || srcTop >= srcBottom) return;

	uint32_t aColor = theColor.ToGLColor();
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(gUfUseTexture, 1);

	int srcX, srcY;
	float dstX, dstY;
	int w, h;
	float u1, v1, u2, v2;
	float uvb[4];

	srcY = srcTop; dstY = theY;
	while (srcY < srcBottom)
	{
		srcX = srcLeft; dstX = theX;
		while (srcX < srcRight)
		{
			w = srcRight - srcX; h = srcBottom - srcY;
			GLuint &tex = GetTexture(srcX, srcY, w, h, u1, v1, u2, v2, uvb);
			float x = dstX, y = dstY;

			GLVertex v[4] = {
				{ x,     y,     0, aColor, u1, v1 },
				{ x,     y + h, 0, aColor, u1, v2 },
				{ x + w, y,     0, aColor, u2, v1 },
				{ x + w, y + h, 0, aColor, u2, v2 },
			};
			GfxBindTexture(tex, uvb);
			GfxBegin(GL_TRIANGLE_STRIP);
			GfxAddVertices(v, 4);
			GfxEnd();

			srcX += w; dstX += w;
		}
		srcY += h; dstY += h;
	}
}

static inline float GetCoord(const GLVertex& v, int n)
{
	switch (n)
	{
	case 0: return v.sx; case 1: return v.sy; case 2: return v.sz;
	case 3: return v.tu; case 4: return v.tv; default: return 0;
	}
}

static inline GLVertex Interpolate(const GLVertex &a, const GLVertex &b, float t)
{
	GLVertex r = a;
	r.sx = a.sx + t * (b.sx - a.sx);
	r.sy = a.sy + t * (b.sy - a.sy);
	r.tu = a.tu + t * (b.tu - a.tu);
	r.tv = a.tv + t * (b.tv - a.tv);
	if (a.color != b.color)
	{
		auto lerp = [&](int shift) {
			return (int)(((a.color >> shift) & 0xff) + t * (((b.color >> shift) & 0xff) - ((a.color >> shift) & 0xff)));
		};
		r.color = (lerp(0)) | (lerp(8) << 8) | (lerp(16) << 16) | (lerp(24) << 24);
	}
	return r;
}

template<class Pred>
struct PointClipper
{
	Pred mPred;

	void ClipPoint(int n, float clipVal, const GLVertex& v1, const GLVertex& v2, VertexList& out)
	{
		bool c1 = mPred(GetCoord(v1, n), clipVal);
		bool c2 = mPred(GetCoord(v2, n), clipVal);
		if (!c1)
		{
			if (!c2)
				out.push_back(v2);
			else
				out.push_back(Interpolate(v1, v2, (clipVal - GetCoord(v1, n)) / (GetCoord(v2, n) - GetCoord(v1, n))));
		}
		else if (!c2)
		{
			out.push_back(Interpolate(v1, v2, (clipVal - GetCoord(v1, n)) / (GetCoord(v2, n) - GetCoord(v1, n))));
			out.push_back(v2);
		}
	}

	void ClipPoints(int n, float clipVal, VertexList& in, VertexList& out)
	{
		if (in.size() < 2) return;
		ClipPoint(n, clipVal, in[in.size() - 1], in[0], out);
		for (int i = 0; i < in.size() - 1; i++)
			ClipPoint(n, clipVal, in[i], in[i + 1], out);
	}
};

static void DrawPolyClipped(const Rect *clip, const VertexList &list)
{
	VertexList l1, l2;
	l1 = list;

	int left = clip->mX, top = clip->mY;
	int right = left + clip->mWidth, bottom = top + clip->mHeight;

	VertexList *in = &l1, *out = &l2;
	PointClipper<std::less<float>>          lc;
	PointClipper<std::greater_equal<float>> gc;

	lc.ClipPoints(0, left,   *in, *out); std::swap(in, out); out->clear();
	lc.ClipPoints(1, top,    *in, *out); std::swap(in, out); out->clear();
	gc.ClipPoints(0, right,  *in, *out); std::swap(in, out); out->clear();
	gc.ClipPoints(1, bottom, *in, *out);

	if (out->size() >= 3)
	{
		GfxBegin(GL_TRIANGLE_FAN);
		GfxAddVertices(*out);
		GfxEnd();
	}
}

static void DoPolyTextureClip(VertexList &list)
{
	VertexList l2;
	VertexList *in = &list, *out = &l2;
	PointClipper<std::less<float>>          lc;
	PointClipper<std::greater_equal<float>> gc;

	lc.ClipPoints(3, 0, *in, *out); std::swap(in, out); out->clear();
	lc.ClipPoints(4, 0, *in, *out); std::swap(in, out); out->clear();
	gc.ClipPoints(3, 1, *in, *out); std::swap(in, out); out->clear();
	gc.ClipPoints(4, 1, *in, *out);
}

void TextureData::BltTransformed(const SexyMatrix3 &theTrans, const Rect& theSrcRect,
	const Color& theColor, const Rect *theClipRect, float theX, float theY, bool center)
{
	int srcLeft   = theSrcRect.mX;
	int srcTop    = theSrcRect.mY;
	int srcRight  = srcLeft + theSrcRect.mWidth;
	int srcBottom = srcTop  + theSrcRect.mHeight;
	if (srcLeft >= srcRight || srcTop >= srcBottom) return;

	float startx = 0, starty = 0;
	if (center)
	{
		startx = -theSrcRect.mWidth  / 2.0f;
		starty = -theSrcRect.mHeight / 2.0f;
	}

	uint32_t aColor = theColor.ToGLColor();
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(gUfUseTexture, 1);

	int srcX, srcY;
	float dstX, dstY;
	int w, h;
	float u1, v1, u2, v2;
	float uvb[4];

	srcY = srcTop; dstY = starty;
	while (srcY < srcBottom)
	{
		srcX = srcLeft; dstX = startx;
		while (srcX < srcRight)
		{
			w = srcRight - srcX; h = srcBottom - srcY;
			GLuint &tex = GetTexture(srcX, srcY, w, h, u1, v1, u2, v2, uvb);

			float x = dstX, y = dstY;
			SexyVector2 p[4] = { {x, y}, {x, y+h}, {x+w, y}, {x+w, y+h} };
			SexyVector2 tp[4];
			for (int i = 0; i < 4; i++)
			{
				tp[i] = theTrans * p[i];
				tp[i].x += theX;
				tp[i].y += theY;
			}

			bool clipped = false;
			if (theClipRect)
			{
				int cl = theClipRect->mX, ct = theClipRect->mY;
				int cr = cl + theClipRect->mWidth, cb = ct + theClipRect->mHeight;
				for (int i = 0; i < 4; i++)
					if (tp[i].x < cl || tp[i].x >= cr || tp[i].y < ct || tp[i].y >= cb)
						{ clipped = true; break; }
			}

			GLVertex vtx[4] = {
				{ tp[0].x, tp[0].y, 0, aColor, u1, v1 },
				{ tp[1].x, tp[1].y, 0, aColor, u1, v2 },
				{ tp[2].x, tp[2].y, 0, aColor, u2, v1 },
				{ tp[3].x, tp[3].y, 0, aColor, u2, v2 },
			};
			GfxBindTexture(tex, uvb);

			if (!clipped)
			{
				GfxBegin(GL_TRIANGLE_STRIP);
				GfxAddVertices(vtx, 4);
				GfxEnd();
			}
			else
			{
				VertexList vl;
				vl.push_back(vtx[0]); vl.push_back(vtx[1]);
				vl.push_back(vtx[3]); vl.push_back(vtx[2]);
				DrawPolyClipped(theClipRect, vl);
			}

			srcX += w; dstX += w;
		}
		srcY += h; dstY += h;
	}
}

void TextureData::BltTriangles(const TriVertex theVertices[][3], int theNumTriangles,
                               unsigned int theColor, float tx, float ty, bool clampUv)
{
	if (mMaxTotalU <= 1.0 && mMaxTotalV <= 1.0)
	{
		// Single-texture fast path
		TextureDataPiece& piece = mTextures[0];
		float halfU = 0.5f / piece.mWidth;
		float halfV = 0.5f / piece.mHeight;
		float midU = mMaxTotalU * 0.5f;
		float midV = mMaxTotalV * 0.5f;
		float uvb[4] = {
			std::min(halfU, midU),
			std::min(halfV, midV),
			std::max(mMaxTotalU - halfU, midU),
			std::max(mMaxTotalV - halfV, midV)
		};
		glActiveTexture(GL_TEXTURE0);
		GfxBindTexture(piece.mTexture, uvb, clampUv);
		glUniform1i(gUfUseTexture, 1);

		GfxBegin(GL_TRIANGLES);
		GfxAddVertices(theVertices, theNumTriangles, theColor, tx, ty, mMaxTotalU, mMaxTotalV);
		GfxEnd();
		return;
	}

	// Multi-texture path
	for (int tri = 0; tri < theNumTriangles; tri++)
	{
		TriVertex* tv = (TriVertex*)theVertices[tri];
		GLVertex vtx[3] = {
			{ tv[0].x+tx, tv[0].y+ty, 0, GetColorFromTriVertex(tv[0], theColor), tv[0].u*mMaxTotalU, tv[0].v*mMaxTotalV },
			{ tv[1].x+tx, tv[1].y+ty, 0, GetColorFromTriVertex(tv[1], theColor), tv[1].u*mMaxTotalU, tv[1].v*mMaxTotalV },
			{ tv[2].x+tx, tv[2].y+ty, 0, GetColorFromTriVertex(tv[2], theColor), tv[2].u*mMaxTotalU, tv[2].v*mMaxTotalV },
		};

		float minU = mMaxTotalU, minV = mMaxTotalV, maxU = 0, maxV = 0;
		for (int i = 0; i < 3; i++)
		{
			minU = std::min(minU, vtx[i].tu); maxU = std::max(maxU, vtx[i].tu);
			minV = std::min(minV, vtx[i].tv); maxV = std::max(maxV, vtx[i].tv);
		}

		VertexList master;
		master.push_back(vtx[0]); master.push_back(vtx[1]); master.push_back(vtx[2]);

		int aLeft   = std::max(0, (int)floorf(minU));
		int aTop    = std::max(0, (int)floorf(minV));
		int aRight  = std::min(mTexVecWidth,  (int)ceilf(maxU));
		int aBottom = std::min(mTexVecHeight, (int)ceilf(maxV));

		TextureDataPiece &std0 = mTextures[0];
		for (int i = aTop; i < aBottom; i++)
		{
			for (int j = aLeft; j < aRight; j++)
			{
				TextureDataPiece &piece = mTextures[i * mTexVecWidth + j];
				float halfU = 0.5f / piece.mWidth;
				float halfV = 0.5f / piece.mHeight;
				float uvb[4] = {
					std::min(halfU, 0.5f),
					std::min(halfV, 0.5f),
					std::max(1.0f - halfU, 0.5f),
					std::max(1.0f - halfV, 0.5f)
				};
				VertexList vl = master;
				for (int k = 0; k < 3; k++)
				{
					vl[k].tu -= j;
					vl[k].tv -= i;
					if (i == mTexVecHeight - 1)
						vl[k].tv *= (float)std0.mHeight / piece.mHeight;
					if (j == mTexVecWidth - 1)
						vl[k].tu *= (float)std0.mWidth / piece.mWidth;
				}
				DoPolyTextureClip(vl);
				if (vl.size() >= 3)
				{
					GfxBindTexture(piece.mTexture, uvb, clampUv);
					GfxBegin(GL_TRIANGLE_FAN);
					GfxAddVertices(vl);
					GfxEnd();
				}
			}
		}
	}
}

GLInterface::GLInterface(SexyAppBase* theApp)
{
	mApp = theApp;
	mWidth  = mApp->mWidth;
	mHeight = mApp->mHeight;
	mDisplayWidth  = mWidth;
	mDisplayHeight = mHeight;
	mPresentationRect = Rect(0, 0, mWidth, mHeight);
	mRefreshRate = 60;
	mMillisecondsPerFrame = 1000 / mRefreshRate;
	mScreenImage = nullptr;
	mNextCursorX = mNextCursorY = 0;
	mCursorX = mCursorY = 0;

	gVertexMode  = (GLenum)-1;
	gNumVertices = 0;
	gVertices.clear();
	gVertices.reserve(MAX_VERTICES);
}

GLInterface::~GLInterface()
{
	Flush();
	for (auto *img : mImageSet)
	{
		delete (TextureData*)img->mRenderData;
		img->mRenderData = nullptr;
	}
}

void GLInterface::SetDrawMode(int theDrawMode)
{
	if (theDrawMode == Graphics::DRAWMODE_NORMAL)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void GLInterface::AddGLImage(GLImage* theGLImage)
{
	std::scoped_lock lk(mCritSect);
	mGLImageSet.insert(theGLImage);
}

void GLInterface::RemoveGLImage(GLImage* theGLImage)
{
	std::scoped_lock lk(mCritSect);
	mGLImageSet.erase(theGLImage);
}

void GLInterface::Remove3DData(MemoryImage* theImage)
{
	if (theImage->mRenderData)
	{
		delete (TextureData*)theImage->mRenderData;
		theImage->mRenderData = nullptr;
		std::scoped_lock lk(mCritSect);
		mImageSet.erase(theImage);
	}
}

GLImage* GLInterface::GetScreenImage() { return mScreenImage; }

void GLInterface::UpdateViewport()
{
	int vx = 0, vy = 0, vw, vh;

#ifdef __SWITCH__
	int width = 1280, height = 720;
#else
	int width, height;
	SDL_GL_GetDrawableSize((SDL_Window*)mApp->mWindow, &width, &height);
#endif

	vw = width; vh = height;

	// Letterbox to 4:3
	if (width * 3 > height * 4)
	{
		vw = height * 4 / 3;
		vx = (width - vw) / 2;
	}
	else if (width * 3 < height * 4)
	{
		vh = width * 3 / 4;
		vy = (height - vh) / 2;
	}

	glViewport(vx, vy, vw, vh);
	mPresentationRect = Rect(vx, vy, vw, vh);
}

int GLInterface::Init(bool IsWindowed)
{
	static bool inited = false;
	if (!inited)
	{
		inited = true;
		PlatformGLInit();

		gProgram = shaderLoad(SHADER_CODE);
		if (gProgram == 0)
			return 0;
		gUfViewProjMtx = glGetUniformLocation(gProgram, "u_viewProj");
		gUfTexture     = glGetUniformLocation(gProgram, "u_texture");
		gUfUseTexture  = glGetUniformLocation(gProgram, "u_useTexture");
		gUfUvBounds    = glGetUniformLocation(gProgram, "u_uvBounds");
		gUfClampUvEnabled = glGetUniformLocation(gProgram, "u_clampUvEnabled");

		glGenBuffers(1, &gVbo);
		glBindBuffer(GL_ARRAY_BUFFER, gVbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLVertex) * MAX_VERTICES, nullptr, GL_DYNAMIC_DRAW);
	}

	int aMaxSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &aMaxSize);
	MAX_TEXTURE_SIZE = aMaxSize;

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	gTextureSizeMustBePow2 = false;
	gMinTextureWidth  = 8;
	gMinTextureHeight = 8;
	gMaxTextureWidth  = aMaxSize;
	gMaxTextureHeight = aMaxSize;
	gSupportedPixelFormats = PixelFormat_A8R8G8B8 | PixelFormat_A4R4G4B4 | PixelFormat_R5G6B5 | PixelFormat_Palette8;
	gLinearFilter = false;

	glUseProgram(gProgram);
	float ortho[16];
	MakeOrthoMatrix(0, (float)mWidth, (float)mHeight, 0, -10, 10, ortho);
	glUniformMatrix4fv(gUfViewProjMtx, 1, GL_FALSE, ortho);
	glUniform1i(gUfTexture, 0);
	glUniform1i(gUfClampUvEnabled, 1);

	glEnable(GL_BLEND);
	glDisable(GL_DITHER);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_FRAMEBUFFER_SRGB); // Prevent double gamma correction (already sRGB passthrough)
	glGetError(); // clear GL_INVALID_ENUM on pure GLES implementations

	mRGBBits   = 32;
	mRedBits   = 8; mGreenBits = 8; mBlueBits  = 8;
	mRedShift  = 0; mGreenShift = 8; mBlueShift = 16;
	mRedMask   = 0xFFu << mRedShift;
	mGreenMask = 0xFFu << mGreenShift;
	mBlueMask  = 0xFFu << mBlueShift;

	SetVideoOnlyDraw(false);
	return 1;
}

bool GLInterface::Redraw(Rect*)
{
	Flush();
	return true;
}

void GLInterface::SetVideoOnlyDraw(bool)
{
	delete mScreenImage;
	mScreenImage = new GLImage(this);
	mScreenImage->mWidth  = mWidth;
	mScreenImage->mHeight = mHeight;
	mScreenImage->SetImageMode(false, false);
}

void GLInterface::SetCursorPos(int x, int y)
{
	mNextCursorX = x;
	mNextCursorY = y;
}

bool GLInterface::PreDraw()
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	return true;
}

void GLInterface::Flush()
{
	gNumVertices = 0;
#ifdef __SWITCH__
	eglSwapBuffers(mApp->mWindow, mApp->mSurface);
#else
	SDL_GL_SwapWindow((SDL_Window*)mApp->mWindow);
#endif
#ifndef __EMSCRIPTEN__
	// Clear back buffer after swap (content undefined)
	glClear(GL_COLOR_BUFFER_BIT);
#endif // Emscripten: browser composites after rAF, no clear needed
}

bool GLInterface::CreateImageTexture(MemoryImage *theImage)
{
	bool wantPurge = false;
	if (theImage->mRenderData == nullptr)
	{
		theImage->mRenderData = new TextureData();
		wantPurge = theImage->mPurgeBits;
		std::scoped_lock lk(mCritSect);
		mImageSet.insert(theImage);
	}

	TextureData *data = (TextureData*)theImage->mRenderData;
	data->CheckCreateTextures(theImage);

	if (wantPurge)
		theImage->PurgeBits();
	return data->mPixelFormat != PixelFormat_Unknown;
}

bool GLInterface::RecoverBits(MemoryImage* theImage)
{
	if (!theImage->mRenderData) return false;

	TextureData* data = (TextureData*)theImage->mRenderData;
	if (data->mBitsChangedCount != theImage->mBitsChangedCount) return false;

	for (int row = 0; row < data->mTexVecHeight; row++)
	{
		for (int col = 0; col < data->mTexVecWidth; col++)
		{
			TextureDataPiece &piece = data->mTextures[row * data->mTexVecWidth + col];
			int offx = col * data->mTexPieceWidth;
			int offy = row * data->mTexPieceHeight;
			int w = std::min(theImage->mWidth  - offx, piece.mWidth);
			int h = std::min(theImage->mHeight - offy, piece.mHeight);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, piece.mTexture);

			// FBO readback (ES 2.0 has no glGetTexImage)
			GLuint fbo;
			glGenFramebuffers(1, &fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, piece.mTexture, 0);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glDeleteFramebuffers(1, &fbo);
				return false;
			}

			std::vector<uint32_t> buf(w * h);
			glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buf.data());

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDeleteFramebuffers(1, &fbo);

			uint32_t* dst = theImage->GetBits() + offy * theImage->GetWidth() + offx;
			const uint32_t* src = buf.data();
			for (int r = 0; r < h; r++)
			{
				for (int c = 0; c < w; c++)
					dst[c] = RgbaToArgb(src[c]);
				src += w;
				dst += theImage->GetWidth();
			}
		}
	}
	return true;
}

void GLInterface::PushTransform(const SexyMatrix3 &theTransform, bool concatenate)
{
	if (mTransformStack.empty() || !concatenate)
		mTransformStack.push_back(theTransform);
	else
		mTransformStack.push_back(theTransform * mTransformStack.back());
}

void GLInterface::PopTransform()
{
	if (!mTransformStack.empty())
		mTransformStack.pop_back();
}

void GLInterface::Blt(Image* theImage, float theX, float theY,
	const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter)
{
	if (!mTransformStack.empty())
	{
		BltClipF(theImage, theX, theY, theSrcRect, nullptr, theColor, theDrawMode, linearFilter);
		return;
	}
	if (!PreDraw()) return;

	MemoryImage* mem = (MemoryImage*)theImage;
	if (!CreateImageTexture(mem)) return;

	SetDrawMode(theDrawMode);
	SetLinearFilter(linearFilter);
	((TextureData*)mem->mRenderData)->Blt(theX, theY, theSrcRect, theColor);
}

void GLInterface::BltClipF(Image* theImage, float theX, float theY,
	const Rect& theSrcRect, const Rect *theClipRect, const Color& theColor, int theDrawMode, bool linearFilter)
{
	SexyTransform2D t;
	t.Translate(theX, theY);
	BltTransformed(theImage, theClipRect, theColor, theDrawMode, theSrcRect, t, linearFilter);
}

void GLInterface::BltMirror(Image* theImage, float theX, float theY,
	const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter)
{
	SexyTransform2D t;
	t.Translate(-theSrcRect.mWidth, 0);
	t.Scale(-1, 1);
	t.Translate(theX, theY);
	BltTransformed(theImage, nullptr, theColor, theDrawMode, theSrcRect, t, linearFilter);
}

void GLInterface::StretchBlt(Image* theImage, const Rect& theDestRect,
	const Rect& theSrcRect, const Rect* theClipRect, const Color &theColor,
	int theDrawMode, bool fastStretch, bool mirror)
{
	float xs = (float)theDestRect.mWidth  / theSrcRect.mWidth;
	float ys = (float)theDestRect.mHeight / theSrcRect.mHeight;

	SexyTransform2D t;
	if (mirror)
	{
		t.Translate(-theSrcRect.mWidth, 0);
		t.Scale(-xs, ys);
	}
	else
		t.Scale(xs, ys);

	t.Translate(theDestRect.mX, theDestRect.mY);
	BltTransformed(theImage, theClipRect, theColor, theDrawMode, theSrcRect, t, !fastStretch);
}

void GLInterface::BltRotated(Image* theImage, float theX, float theY,
	const Rect* theClipRect, const Color& theColor, int theDrawMode,
	double theRot, float theRotCenterX, float theRotCenterY, const Rect& theSrcRect)
{
	SexyTransform2D t;
	t.Translate(-theRotCenterX, -theRotCenterY);
	t.RotateRad(theRot);
	t.Translate(theX + theRotCenterX, theY + theRotCenterY);
	BltTransformed(theImage, theClipRect, theColor, theDrawMode, theSrcRect, t, true);
}

void GLInterface::BltTransformed(Image* theImage, const Rect* theClipRect,
	const Color& theColor, int theDrawMode, const Rect &theSrcRect,
	const SexyMatrix3 &theTransform, bool linearFilter, float theX, float theY, bool center)
{
	if (!PreDraw()) return;

	MemoryImage* mem = (MemoryImage*)theImage;
	if (!CreateImageTexture(mem)) return;

	SetDrawMode(theDrawMode);
	SetLinearFilter(linearFilter);
	TextureData *data = (TextureData*)mem->mRenderData;

	if (!mTransformStack.empty())
	{
		if (theX != 0 || theY != 0)
		{
			SexyTransform2D t;
			if (center) t.Translate(-theSrcRect.mWidth / 2.0f, -theSrcRect.mHeight / 2.0f);
			t = theTransform * t;
			t.Translate(theX, theY);
			t = mTransformStack.back() * t;
			data->BltTransformed(t, theSrcRect, theColor, theClipRect);
		}
		else
		{
			SexyTransform2D t = mTransformStack.back() * theTransform;
			data->BltTransformed(t, theSrcRect, theColor, theClipRect, theX, theY, center);
		}
	}
	else
	{
		data->BltTransformed(theTransform, theSrcRect, theColor, theClipRect, theX, theY, center);
	}
}

void GLInterface::DrawLine(double x1, double y1, double x2, double y2,
	const Color& theColor, int theDrawMode)
{
	if (!PreDraw()) return;
	SetDrawMode(theDrawMode);

	float fx1 = x1, fy1 = y1, fx2 = x2, fy2 = y2;
	if (!mTransformStack.empty())
	{
		SexyVector2 p1(x1, y1), p2(x2, y2);
		p1 = mTransformStack.back() * p1;
		p2 = mTransformStack.back() * p2;
		fx1 = p1.x; fy1 = p1.y; fx2 = p2.x; fy2 = p2.y;
	}

	glUniform1i(gUfUseTexture, 0);
	uint32_t c = theColor.ToGLColor();
	GLVertex v[3] = {
		{ fx1, fy1, 0, c, 0, 0 },
		{ fx2, fy2, 0, c, 0, 0 },
		{ fx2 + 0.5f, fy2 + 0.5f, 0, c, 0, 0 },
	};
	GfxBegin(GL_LINE_STRIP);
	GfxAddVertices(v, 3);
	GfxEnd();
}

void GLInterface::FillRect(const Rect& theRect, const Color& theColor, int theDrawMode)
{
	if (!PreDraw()) return;
	SetDrawMode(theDrawMode);

	float x = theRect.mX, y = theRect.mY;
	float w = theRect.mWidth,     h = theRect.mHeight;
	uint32_t c = theColor.ToGLColor();

	GLVertex v[4] = {
		{ x,     y,     0, c, 0, 0 },
		{ x,     y + h, 0, c, 0, 0 },
		{ x + w, y,     0, c, 0, 0 },
		{ x + w, y + h, 0, c, 0, 0 },
	};

	if (!mTransformStack.empty())
	{
		SexyVector2 p[4] = { {x, y}, {x, y+h}, {x+w, y}, {x+w, y+h} };
		for (int i = 0; i < 4; i++)
		{
			p[i] = mTransformStack.back() * p[i];
			v[i].sx = p[i].x;
			v[i].sy = p[i].y;
		}
	}

	glUniform1i(gUfUseTexture, 0);
	GfxBegin(GL_TRIANGLE_STRIP);
	GfxAddVertices(v, 4);
	GfxEnd();
}

void GLInterface::DrawTriangle(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3,
	const Color &theColor, int theDrawMode)
{
	if (!PreDraw()) return;
	SetDrawMode(theDrawMode);

	uint32_t c = theColor.ToGLColor();
	glUniform1i(gUfUseTexture, 0);

	GLVertex v[3] = {
		{ p1.x, p1.y, 0, GetColorFromTriVertex(p1, c), 0, 0 },
		{ p2.x, p2.y, 0, GetColorFromTriVertex(p2, c), 0, 0 },
		{ p3.x, p3.y, 0, GetColorFromTriVertex(p3, c), 0, 0 },
	};
	GfxBegin(GL_TRIANGLE_STRIP);
	GfxAddVertices(v, 3);
	GfxEnd();
}

void GLInterface::DrawTriangleTex(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3,
	const Color &theColor, int theDrawMode, Image *theTexture, bool blend)
{
	TriVertex arr[1][3] = { {p1, p2, p3} };
	DrawTrianglesTex(arr, 1, theColor, theDrawMode, theTexture, blend);
}

void GLInterface::DrawTrianglesTex(const TriVertex theVertices[][3], int theNumTriangles,
	const Color &theColor, int theDrawMode, Image *theTexture, float tx, float ty, bool blend)
{
	if (!PreDraw()) return;

	MemoryImage* mem = (MemoryImage*)theTexture;
	if (!CreateImageTexture(mem)) return;

	SetDrawMode(theDrawMode);
	SetLinearFilter(blend);

	uint32_t c = theColor.ToGLColor();
	bool clampUv = (mem->mRenderFlags & RenderImageFlag_Repeat) == 0;
	((TextureData*)mem->mRenderData)->BltTriangles(theVertices, theNumTriangles, c, tx, ty, clampUv);
}

void GLInterface::DrawTrianglesTexStrip(const TriVertex theVertices[], int theNumTriangles,
	const Color &theColor, int theDrawMode, Image *theTexture, float tx, float ty, bool blend)
{
	TriVertex batch[100][3];
	int done = 0;
	while (done < theNumTriangles)
	{
		int n = std::min(100, theNumTriangles - done);
		for (int i = 0; i < n; i++)
		{
			batch[i][0] = theVertices[done];
			batch[i][1] = theVertices[done + 1];
			batch[i][2] = theVertices[done + 2];
			done++;
		}
		DrawTrianglesTex(batch, n, theColor, theDrawMode, theTexture, tx, ty, blend);
	}
}

void GLInterface::FillPoly(const Point theVertices[], int theNumVertices,
	const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty)
{
	if (theNumVertices < 3) return;
	if (!PreDraw()) return;
	SetDrawMode(theDrawMode);

	uint32_t c = theColor.ToGLColor();
	glUniform1i(gUfUseTexture, 0);

	VertexList vl;
	for (int i = 0; i < theNumVertices; i++)
	{
		GLVertex v = { theVertices[i].mX + (float)tx, theVertices[i].mY + (float)ty, 0, c, 0, 0 };
		if (!mTransformStack.empty())
		{
			SexyVector2 p(v.sx, v.sy);
			p = mTransformStack.back() * p;
			v.sx = p.x; v.sy = p.y;
		}
		vl.push_back(v);
	}

	if (theClipRect)
		DrawPolyClipped(theClipRect, vl);
	else
	{
		GfxBegin(GL_TRIANGLE_FAN);
		GfxAddVertices(vl);
		GfxEnd();
	}
}

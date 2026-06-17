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

#define XMD_H

#include "Common.h"
#include "ImageLib.h"
#include "png.h"
#include <math.h>
#include <algorithm>
#include <array>
#include <cctype>
#include <string_view>
#include <mutex>
#include <unordered_set>
#include "paklib/PakInterface.h"

extern "C"
{
#include "jpeglib.h"
#include "jerror.h"
}

using namespace ImageLib;
using namespace std::string_view_literals;

Image::Image()
{
	mWidth = 0;
	mHeight = 0;
	mBits = nullptr;
}

Image::~Image()
{
	delete[] mBits;
}

int	Image::GetWidth()
{
	return mWidth;
}

int	Image::GetHeight()
{
	return mHeight;
}

uint32_t* Image::GetBits()
{
	return mBits;
}

//////////////////////////////////////////////////////////////////////////
// PNG Pak Support

static void png_pak_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_size_t check;

	/* fread() returns 0 on error, so it is OK to store this in a png_size_t
	* instead of an int, which is what fread() actually returns.
	*/
	check = (png_size_t)p_fread(data, (png_size_t)1, length,
		(PFILE*)png_get_io_ptr(png_ptr));

	if (check != length)
	{
		png_error(png_ptr, "Read Error");
	}
}

Image* GetPNGImage(const std::string& theFileName)
{
	png_structp png_ptr;
	png_infop info_ptr;
	//unsigned int sig_read = 0;
	png_uint_32 width, height;
	//int bit_depth, color_type, interlace_type;
	PFILE *fp;

	if ((fp = p_fopen(theFileName.c_str(), "rb")) == nullptr)
		return nullptr;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
	  nullptr, nullptr, nullptr);
	png_set_read_fn(png_ptr, (png_voidp)fp, png_pak_read_data);

	if (png_ptr == nullptr)
	{
		p_fclose(fp);
		return nullptr;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == nullptr)
	{
		p_fclose(fp);
		png_destroy_read_struct(&png_ptr, (png_infopp)nullptr, (png_infopp)nullptr);
		return nullptr;
	}

   /* Set error handling if you are using the setjmp/longjmp method (this is
    * the normal method of doing things with libpng).  REQUIRED unless you
    * set up your own error handlers in the png_create_read_struct() earlier.
    */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)nullptr);
		p_fclose(fp);
		/* If we get here, we had a problem reading the file */
		return nullptr;
	}

	//png_init_io(png_ptr, fp);

	//png_ptr->io_ptr = (png_voidp)fp;

	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, nullptr, nullptr,
       nullptr, nullptr, nullptr);

	png_set_expand(png_ptr);
	if constexpr (std::endian::native == std::endian::big)
	{
		png_set_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
	}
	else
	{
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
		png_set_bgr(png_ptr);
	}
	png_set_palette_to_rgb(png_ptr);
	png_set_gray_to_rgb(png_ptr);

//	int aNumBytes = png_get_rowbytes(png_ptr, info_ptr) * height / 4;
	png_bytep* row_pointers = new png_bytep[height];
	uint32_t* aBits = new uint32_t[width*height];
	for (uint i = 0; i < height; i++)
	{
		row_pointers[i] = (png_bytep)(aBits + i*width);
	}
	png_read_image(png_ptr, row_pointers);

	/* read rest of file, and get additional chunks in info_ptr - REQUIRED */
	png_read_end(png_ptr, info_ptr);

	/* clean up after the read, and free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)nullptr);

	/* close the file */
	p_fclose(fp);
	delete[] row_pointers;

	Image* anImage = new Image();
	anImage->mWidth = width;
	anImage->mHeight = height;
	anImage->mBits = aBits;

	return anImage;
}

Image* GetTGAImage(const std::string& theFileName)
{
	PFILE* aTGAFile = p_fopen(theFileName.c_str(), "rb");
	if (aTGAFile == nullptr)
		return nullptr;

	uint8_t aHeaderIDLen;
	p_fread(&aHeaderIDLen, sizeof(uint8_t), 1, aTGAFile);

	uint8_t aColorMapType;
	p_fread(&aColorMapType, sizeof(uint8_t), 1, aTGAFile);
	
	uint8_t anImageType;
	p_fread(&anImageType, sizeof(uint8_t), 1, aTGAFile);

	uint16_t aFirstEntryIdx;
	p_fread(&aFirstEntryIdx, sizeof(uint16_t), 1, aTGAFile);
	aFirstEntryIdx = Sexy::FromLE16(aFirstEntryIdx);

	uint16_t aColorMapLen;
	p_fread(&aColorMapLen, sizeof(uint16_t), 1, aTGAFile);
	aColorMapLen = Sexy::FromLE16(aColorMapLen);

	uint8_t aColorMapEntrySize;
	p_fread(&aColorMapEntrySize, sizeof(uint8_t), 1, aTGAFile);	

	uint16_t anXOrigin;
	p_fread(&anXOrigin, sizeof(uint16_t), 1, aTGAFile);
	anXOrigin = Sexy::FromLE16(anXOrigin);

	uint16_t aYOrigin;
	p_fread(&aYOrigin, sizeof(uint16_t), 1, aTGAFile);
	aYOrigin = Sexy::FromLE16(aYOrigin);

	uint16_t anImageWidth;
	p_fread(&anImageWidth, sizeof(uint16_t), 1, aTGAFile);	
	anImageWidth = Sexy::FromLE16(anImageWidth);

	uint16_t anImageHeight;
	p_fread(&anImageHeight, sizeof(uint16_t), 1, aTGAFile);	
	anImageHeight = Sexy::FromLE16(anImageHeight);

	uint8_t aBitCount = 32;
	p_fread(&aBitCount, sizeof(uint8_t), 1, aTGAFile);	

	uint8_t anImageDescriptor = 8 | (1<<5);
	p_fread(&anImageDescriptor, sizeof(uint8_t), 1, aTGAFile);

	if ((aBitCount != 32) ||
		(anImageDescriptor != (8 | (1<<5))))
	{
		p_fclose(aTGAFile);
		return nullptr;
	}

	Image* anImage = new Image();

	anImage->mWidth = anImageWidth;
	anImage->mHeight = anImageHeight;
	anImage->mBits = new uint32_t[anImageWidth*anImageHeight];

	p_fread(anImage->mBits, 4, anImage->mWidth*anImage->mHeight, aTGAFile);

	// TGA stores BGRA in LE; on BE need to byteswap each pixel
	if constexpr (std::endian::native == std::endian::big)
	{
		uint32_t* ptr = anImage->mBits;
		for (int i = 0; i < anImageWidth * anImageHeight; i++, ptr++)
			*ptr = Sexy::ByteSwap32(*ptr);
	}

	p_fclose(aTGAFile);

	return anImage;
}

int ReadBlobBlock(PFILE* fp, char* data)
{
	unsigned char aCount = 0;
	p_fread(&aCount, sizeof(char), 1, fp);
	p_fread(data, sizeof(char), aCount, fp);
	return aCount;
}

Image* GetGIFImage(const std::string& theFileName)
{
	#define BitSet(byte,bit)  (((byte) & (bit)) == (bit))
	#define LSBFirstOrder(x,y)  (((y) << 8) | (x))

	int
		opacity,
		status;

	int i;

	unsigned char *p;

	unsigned char
		background,			// 背景色在全局颜色列表中的索引（背景色：图像中没有被指定颜色的像素会被背景色填充）
		c,
		flag,				// 图像标志的压缩字节
		*global_colormap,	// 全局颜色列表
		header[1664],
		magick[12];

	unsigned int
		delay,
		dispose,
		global_colors,		// 全局颜色列表大小
		image_count,
		iterations;

	/*
	Open image file.
	*/

	PFILE *fp;

	if ((fp = p_fopen(theFileName.c_str(), "rb")) == nullptr)
		return nullptr;
	/*
	Determine if this is a GIF file.
	*/
	status = p_fread(magick, sizeof(char), 6, fp);  // 读取文件头（包含文件签名与版本号，共 6 字节）
	(void)status; // unused

	// 文件头的 ASCII 值为“GIF87a”或”GIF89a”，其中前三位为 GIF 签名，后三位为不同年份的版本号
	if (((strncmp((char*)magick, "GIF87", 5) != 0) && (strncmp((char*)magick, "GIF89", 5) != 0)))
		return nullptr;

	global_colors = 0;
	global_colormap = (unsigned char*)nullptr;

	uint16_t pw;  // 图像宽度
	uint16_t ph;  // 图像高度

	// 读取逻辑屏幕描述符，共 7 字节
	p_fread(&pw, sizeof(short), 1, fp);  // 读取图像渲染区域的宽度
	p_fread(&ph, sizeof(short), 1, fp);  // 读取图像渲染区域的高度
	pw = Sexy::FromLE16(pw);
	ph = Sexy::FromLE16(ph);
	p_fread(&flag, sizeof(char), 1, fp);  // 读取图像标志
	p_fread(&background, sizeof(char), 1, fp);  // 读取背景色在全局颜色列表中的索引，若无全局颜色列表则此字节无效
	p_fread(&c, sizeof(char), 1, fp);  // 读取像素宽高比

	if (BitSet(flag, 0x80))  // 如果存在全局颜色列表
	{
		/*
		opacity global colormap.
		*/
		global_colors = 1 << ((flag & 0x07) + 1);  // 压缩字节的最低 3 位表示全局颜色列表的大小，设其二进制数值为 N，则列表大小 = 2 ^ (N + 1)
		global_colormap = new unsigned char[3 * global_colors];  // 每个颜色占 3 个字节，按 RGB 排列
		if (global_colormap == (unsigned char*)nullptr)
			return nullptr;

		p_fread(global_colormap, sizeof(char), 3 * global_colors, fp);  // 读取全局颜色列表
	}

	delay = 0;
	dispose = 0;
	iterations = 1;
	opacity = (-1);
	image_count = 0;

	for (; ; )
	{
		if (p_fread(&c, sizeof(char), 1, fp) == 0)
			break;  // 如果读取错误或读取到文件尾则退出，返回空指针

		if (c == ';')  // 当读取到 gif 结束块标记符（End Of File）
			break;  /* terminator */
		if (c == '!')  // 当读取到 gif 拓展块标记符
		{
			/*
			GIF Extension block.
			*/
			p_fread(&c, sizeof(char), 1, fp);  // 读取拓展块的功能编码号

			switch (c)
			{
			case 0xf9:
			{
				/*
				Read Graphics Control extension.
				*/
				while (ReadBlobBlock(fp, (char*)header) > 0);

				dispose = header[0] >> 2;
				delay = (header[2] << 8) | header[1];
				(void)delay; // Unused
				if ((header[0] & 0x01) == 1)
					opacity = header[3];
				break;
			}
			case 0xfe:
			{
				/*
				Read/Discard Comment extension.
				*/
				while (ReadBlobBlock(fp, (char*)header) > 0);
				break;
			}
			case 0xff:
			{
				int
					loop;

				/*
				Read Netscape Loop extension.
				*/
				loop = false;
				if (ReadBlobBlock(fp, (char*)header) > 0)
					loop = !strncmp((char*)header, "NETSCAPE2.0", 11);
				while (ReadBlobBlock(fp, (char*)header) > 0)
					if (loop)
						iterations = (header[2] << 8) | header[1];
				break;
			}
			default:
			{
				while (ReadBlobBlock(fp, (char*)header) > 0);
				break;
			}
			}
		}

		if (c != ',')  // 如果读取的不为图像描述符
			continue;

		if (image_count != 0)
		{
			/*
			Allocate next image structure.
			*/

			/*AllocateNextImage(image_info,image);
			if (image->next == (Image *) nullptr)
			{
			DestroyImages(image);
			return((Image *) nullptr);
			}
			image=image->next;
			MagickMonitor(LoadImagesText,TellBlob(image),image->filesize);*/
		}
		image_count++;

		uint16_t pagex;
		uint16_t pagey;
		uint16_t width;
		uint16_t height;
		int colors;
		bool interlaced;

		p_fread(&pagex, sizeof(short), 1, fp);  // 读取帧的横坐标（Left）
		p_fread(&pagey, sizeof(short), 1, fp);  // 读取帧的纵坐标（Top）
		p_fread(&width, sizeof(short), 1, fp);  // 读取帧的横向宽度（Width）
		p_fread(&height, sizeof(short), 1, fp);  // 取得帧的纵向高度（Height）
		pagex = Sexy::FromLE16(pagex);
		pagey = Sexy::FromLE16(pagey);
		width = Sexy::FromLE16(width);
		height = Sexy::FromLE16(height);
		p_fread(&flag, sizeof(char), 1, fp);  // 读取帧标志的压缩字节

		colors = !BitSet(flag, 0x80) ? global_colors : 1 << ((flag & 0x07) + 1);  // 判断使用全局颜色列表或使用局部颜色列表，并取得列表大小
		uint32_t* colortable = new uint32_t[colors];  // 申请颜色列表

		interlaced = BitSet(flag, 0x40);  // 当前帧图像数据存储方式，为 1 表示交织顺序存储，0 表示顺序存储

		delay = 0;
		dispose = 0;
		(void)dispose; // unused
		iterations = 1;
		(void)iterations; //unused
		/*if (image_info->ping)
		{
		f (opacity >= 0)
		/image->matte=true;

		CloseBlob(image);
		return(image);
		}*/
		if ((width == 0) || (height == 0))
			return nullptr;
		/*
		Inititialize colormap.
		*/
		/*if (!AllocateImageColormap(image,image->colors))
		ThrowReaderException(ResourceLimitWarning,"Memory allocation failed",
		image);*/
		if (!BitSet(flag, 0x80))  // 如果使用全局颜色列表
		{
			/*
			Use global colormap.
			*/
			p = global_colormap;
			for (i = 0; i < (int)colors; i++)
			{
				int r = *p++;
				int g = *p++;
				int b = *p++;

				colortable[i] = 0xFF000000 | (r << 16) | (g << 8) | (b);
			}

			//image->background_color=
			//image->colormap[Min(background,image->colors-1)];
		}
		else
		{
			unsigned char
				* colormap;

			/*
			Read local colormap.
			*/
			colormap = new unsigned char[3 * colors];

			int pos = p_ftell(fp);
			(void)pos; // unused

			p_fread(colormap, sizeof(char), 3 * colors, fp);

			p = colormap;
			for (i = 0; i < (int)colors; i++)
			{
				int r = *p++;
				int g = *p++;
				int b = *p++;

				colortable[i] = 0xFF000000 | (r << 16) | (g << 8) | (b);
			}
			delete[] colormap;
		}

		/*if (opacity >= (int) colors)
		{
		for (i=colors; i < (opacity+1); i++)
		{
		image->colormap[i].red=0;
		image->colormap[i].green=0;
		image->colormap[i].blue=0;
		}
		image->colors=opacity+1;
		}*/
		/*
		Decode image.
		*/
		//status=DecodeImage(image,opacity,exception);

		//if (global_colormap != (unsigned char *) nullptr)
		// LiberateMemory((void **) &global_colormap);
		if (global_colormap != nullptr)
		{
			delete[] global_colormap;
			global_colormap = nullptr;
		}

		//while (image->previous != (Image *) nullptr)
		//    image=image->previous;

#define MaxStackSize  4096
#define NullCode  (-1)

		int
			available,
			bits,
			code,
			clear,
			code_mask,
			code_size,
			count,
			end_of_information,
			in_code,
			offset,
			old_code,
			pass,
			y;

		int
			x;

		unsigned int
			datum;

		short
			* prefix;

		unsigned char
			data_size,
			first,
			* packet,
			* pixel_stack,
			* suffix,
			* top_stack;

		/*
		Allocate decoder tables.
		*/

		packet = new unsigned char[256];
		prefix = new short[MaxStackSize];
		suffix = new unsigned char[MaxStackSize];
		pixel_stack = new unsigned char[MaxStackSize + 1];

		/*
		Initialize GIF data stream decoder.
		*/
		p_fread(&data_size, sizeof(char), 1, fp);
		if (data_size < 2 || data_size > 11)
		{
			delete[] pixel_stack;
			delete[] suffix;
			delete[] prefix;
			delete[] packet;
			delete[] colortable;
			p_fclose(fp);
			return nullptr;
		}
		clear = 1 << data_size;
		end_of_information = clear + 1;
		available = clear + 2;
		old_code = NullCode;
		code_size = data_size + 1;
		code_mask = (1 << code_size) - 1;
		for (code = 0; code < clear; code++)
		{
			prefix[code] = 0;
			suffix[code] = (unsigned char)code;
		}
		/*
		Decode GIF pixel stream.
		*/
		datum = 0;
		bits = 0;
		c = 0;
		count = 0;
		first = 0;
		offset = 0;
		pass = 0;
		top_stack = pixel_stack;

		uint32_t* aBits = new uint32_t[width * height];

		unsigned char* c = nullptr;

		for (y = 0; y < (int)height; y++)
		{
			//q=SetImagePixels(image,0,offset,width,1);
			//if (q == (PixelPacket *) nullptr)
			//break;
			//indexes=GetIndexes(image);

			uint32_t* q = aBits + offset * width;



			for (x = 0; x < (int)width; )
			{
				if (top_stack == pixel_stack)
				{
					if (bits < code_size)
					{
						/*
						Load bytes until there is enough bits for a code.
						*/
						if (count == 0)
						{
							/*
							Read a new data block.
							*/
							int pos = p_ftell(fp);
							(void)pos; // unused

							count = ReadBlobBlock(fp, (char*)packet);
							if (count <= 0)
								break;
							c = packet;
						}
						datum += (*c) << bits;
						bits += 8;
						c++;
						count--;
						continue;
					}
					/*
					Get the next code.
					*/
					code = datum & code_mask;
					datum >>= code_size;
					bits -= code_size;
					/*
					Interpret the code
					*/
					if ((code > available) || (code == end_of_information))
						break;
					if (code == clear)
					{
						/*
						Reset decoder.
						*/
						code_size = data_size + 1;
						code_mask = (1 << code_size) - 1;
						available = clear + 2;
						old_code = NullCode;
						continue;
					}
					if (old_code == NullCode)
					{
						*top_stack++ = suffix[code];
						old_code = code;
						first = (unsigned char)code;
						continue;
					}
					in_code = code;
					if (code >= available)
					{
						*top_stack++ = first;
						code = old_code;
					}
					while (code >= clear)
					{
						*top_stack++ = suffix[code];
						code = prefix[code];
					}
					first = suffix[code];
					/*
					Add a new string to the string table,
					*/
					if (available >= MaxStackSize)
						break;
					*top_stack++ = first;
					prefix[available] = old_code;
					suffix[available] = first;
					available++;
					if (((available & code_mask) == 0) && (available < MaxStackSize))
					{
						code_size++;
						code_mask += available;
					}
					old_code = in_code;
				}
				/*
				Pop a pixel off the pixel stack.
				*/
				top_stack--;

				int index = (*top_stack);

				*q = colortable[index];

				if (index == opacity)
					*q = 0;

				x++;
				q++;
			}

			if (!interlaced)
				offset++;
			else
			{
				switch (pass)
				{
				case 0:
				default:
				{
					offset += 8;
					if (offset >= height)
					{
						pass++;
						offset = 4;
					}
					break;
				}
				case 1:
				{
					offset += 8;
					if (offset >= height)
					{
						pass++;
						offset = 2;
					}
					break;
				}
				case 2:
				{
					offset += 4;
					if (offset >= height)
					{
						pass++;
						offset = 1;
					}
					break;
				}
				case 3:
				{
					offset += 2;
					break;
				}
				}
			}

			if (x < width)
				break;

			/*if (image->previous == (Image *) nullptr)
			if (QuantumTick(y,image->rows))
			MagickMonitor(LoadImageText,y,image->rows);*/
		}
		delete[] pixel_stack;
		delete[] suffix;
		delete[] prefix;
		delete[] packet;

		delete[] colortable;

		//if (y < image->rows)
		//failed = true;

		Image* anImage = new Image();

		anImage->mWidth = width;
		anImage->mHeight = height;
		anImage->mBits = aBits;

		//TODO: Change for animation crap
		p_fclose(fp);
		return anImage;
	}

	p_fclose(fp);

	return nullptr;
}

typedef struct my_error_mgr * my_error_ptr;

struct my_error_mgr
{
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);

}

bool ImageLib::WriteJPEGImage(const std::string& theFileName, Image* theImage)
{
	FILE *fp;

	if ((fp = fopen(theFileName.c_str(), "wb")) == nullptr)
		return false;

	struct jpeg_compress_struct cinfo{};
	struct my_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	if (setjmp(jerr.setjmp_buffer))
	{
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */
		jpeg_destroy_compress(&cinfo);
		fclose(fp);
		return false;
	}

	jpeg_create_compress(&cinfo);

	cinfo.image_width = theImage->mWidth;
	cinfo.image_height = theImage->mHeight;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	cinfo.optimize_coding = TRUE;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 80, TRUE);

	jpeg_stdio_dest(&cinfo, fp);

	jpeg_start_compress(&cinfo, TRUE);

	int row_stride = theImage->GetWidth() * 3;

	unsigned char* aTempBuffer = new unsigned char[row_stride];

	uint32_t* aSrcPtr = theImage->mBits;

	for (int aRow = 0; aRow < theImage->mHeight; aRow++)
	{
		unsigned char* aDest = aTempBuffer;

		for (int aCol = 0; aCol < theImage->mWidth; aCol++)
		{
			uint32_t src = *(aSrcPtr++);

			*aDest++ = (src >> 16) & 0xFF;
			*aDest++ = (src >>  8) & 0xFF;
			*aDest++ = (src      ) & 0xFF;
		}

		jpeg_write_scanlines(&cinfo, &aTempBuffer, 1);
	}

	delete [] aTempBuffer;

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	fclose(fp);

	return true;
}

bool ImageLib::WritePNGImage(const std::string& theFileName, Image* theImage)
{
	png_structp png_ptr;
	png_infop info_ptr;

	FILE *fp;

	if ((fp = fopen(theFileName.c_str(), "wb")) == nullptr)
		return false;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
	  nullptr, nullptr, nullptr);

	if (png_ptr == nullptr)
	{
		fclose(fp);
		return false;
	}

	// Allocate/initialize the memory for image information.  REQUIRED.
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == nullptr)
	{
		fclose(fp);
		png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
		return false;
	}

   // Set error handling if you are using the setjmp/longjmp method (this is
   // the normal method of doing things with libpng).  REQUIRED unless you
   // set up your own error handlers in the png_create_write_struct() earlier.

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		// Free all of the memory associated with the png_ptr and info_ptr
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		// If we get here, we had a problem writeing the file
		return false;
	}

	png_init_io(png_ptr, fp);

	png_color_8 sig_bit;
	sig_bit.red = 8;
	sig_bit.green = 8;
	sig_bit.blue = 8;
	/* if the image has an alpha channel then */
	sig_bit.alpha = 8;
	png_set_sBIT(png_ptr, info_ptr, &sig_bit);
	png_set_bgr(png_ptr);

	png_set_IHDR(png_ptr, info_ptr, theImage->mWidth, theImage->mHeight, 8, PNG_COLOR_TYPE_RGB_ALPHA,
       PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	// Add filler (or alpha) byte (before/after each RGB triplet)
	//png_set_expand(png_ptr);
	//png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	//png_set_gray_1_2_4_to_8(png_ptr);
	//png_set_palette_to_rgb(png_ptr);
	//png_set_gray_to_rgb(png_ptr);

	png_write_info(png_ptr, info_ptr);

	for (int i = 0; i < theImage->mHeight; i++)
	{
		png_bytep aRowPtr = (png_bytep) (theImage->mBits + i*theImage->mWidth);
		png_write_rows(png_ptr, &aRowPtr, 1);
	}

	// write rest of file, and get additional chunks in info_ptr - REQUIRED
	png_write_end(png_ptr, info_ptr);

	// clean up after the write, and free any memory allocated - REQUIRED
	png_destroy_write_struct(&png_ptr, &info_ptr);

	// close the file
	fclose(fp);

	return true;
}

bool ImageLib::WriteTGAImage(const std::string& theFileName, Image* theImage)
{
	FILE* aTGAFile = fopen(theFileName.c_str(), "wb");
	if (aTGAFile == nullptr)
		return false;

	uint8_t aHeaderIDLen = 0;
	fwrite(&aHeaderIDLen, sizeof(uint8_t), 1, aTGAFile);

	uint8_t aColorMapType = 0;
	fwrite(&aColorMapType, sizeof(uint8_t), 1, aTGAFile);
	
	uint8_t anImageType = 2;
	fwrite(&anImageType, sizeof(uint8_t), 1, aTGAFile);

	uint16_t aFirstEntryIdx = 0;
	aFirstEntryIdx = Sexy::ToLE16(aFirstEntryIdx);
	fwrite(&aFirstEntryIdx, sizeof(uint16_t), 1, aTGAFile);

	uint16_t aColorMapLen = 0;
	aColorMapLen = Sexy::ToLE16(aColorMapLen);
	fwrite(&aColorMapLen, sizeof(uint16_t), 1, aTGAFile);

	uint8_t aColorMapEntrySize = 0;
	fwrite(&aColorMapEntrySize, sizeof(uint8_t), 1, aTGAFile);	

	uint16_t anXOrigin = 0;
	anXOrigin = Sexy::ToLE16(anXOrigin);
	fwrite(&anXOrigin, sizeof(uint16_t), 1, aTGAFile);

	uint16_t aYOrigin = 0;
	aYOrigin = Sexy::ToLE16(aYOrigin);
	fwrite(&aYOrigin, sizeof(uint16_t), 1, aTGAFile);

	uint16_t anImageWidth = theImage->mWidth;
	anImageWidth = Sexy::ToLE16(anImageWidth);
	fwrite(&anImageWidth, sizeof(uint16_t), 1, aTGAFile);	

	uint16_t anImageHeight = theImage->mHeight;
	anImageHeight = Sexy::ToLE16(anImageHeight);
	fwrite(&anImageHeight, sizeof(uint16_t), 1, aTGAFile);	

	uint8_t aBitCount = 32;
	fwrite(&aBitCount, sizeof(uint8_t), 1, aTGAFile);	

	uint8_t anImageDescriptor = 8 | (1<<5);
	fwrite(&anImageDescriptor, sizeof(uint8_t), 1, aTGAFile);

	if constexpr (std::endian::native == std::endian::little)
	{
		fwrite(theImage->mBits, 4, theImage->mWidth * theImage->mHeight, aTGAFile);
	}
	else
	{
		std::vector<uint32_t> aPixelsLE(theImage->mBits, theImage->mBits + theImage->mWidth * theImage->mHeight);
		for (uint32_t& pixel : aPixelsLE)
			pixel = Sexy::ToLE32(pixel);
		fwrite(aPixelsLE.data(), 4, aPixelsLE.size(), aTGAFile);
	}

	fclose(aTGAFile);

	return true;
}

////////////////////////////////////////////////////////////////////////// 
// JPEG Pak Reader

typedef struct {
	struct jpeg_source_mgr pub;	/* public fields */

	PFILE * infile;		/* source stream */
	JOCTET * buffer;		/* start of buffer */
	boolean start_of_file;	/* have we gotten any data yet? */
} pak_source_mgr;

typedef pak_source_mgr * pak_src_ptr;

#define INPUT_BUF_SIZE 4096

METHODDEF(void) init_source (j_decompress_ptr cinfo)
{
	pak_src_ptr src = (pak_src_ptr) cinfo->src;
	src->start_of_file = TRUE;
}

METHODDEF(boolean) fill_input_buffer (j_decompress_ptr cinfo)
{
	pak_src_ptr src = (pak_src_ptr) cinfo->src;
	size_t nbytes;

	nbytes = p_fread(src->buffer, 1, INPUT_BUF_SIZE, src->infile);
	//((size_t) fread((void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))

	if (nbytes <= 0) {
		if (src->start_of_file)	/* Treat empty input file as fatal error */
			ERREXIT(cinfo, JERR_INPUT_EMPTY);
		WARNMS(cinfo, JWRN_JPEG_EOF);
		/* Insert a fake EOI marker */
		src->buffer[0] = (JOCTET) 0xFF;
		src->buffer[1] = (JOCTET) JPEG_EOI;
		nbytes = 2;
	}

	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = nbytes;
	src->start_of_file = FALSE;

	return TRUE;
}

METHODDEF(void) skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
	pak_src_ptr src = (pak_src_ptr) cinfo->src;

	if (num_bytes > 0) {
		while (num_bytes > (long) src->pub.bytes_in_buffer) {
			num_bytes -= (long) src->pub.bytes_in_buffer;
			(void) fill_input_buffer(cinfo);
		}
		src->pub.next_input_byte += (size_t) num_bytes;
		src->pub.bytes_in_buffer -= (size_t) num_bytes;
	}
}

METHODDEF(void) term_source (j_decompress_ptr /* cinfo */)
{
	/* no work necessary here */
}

void jpeg_pak_src (j_decompress_ptr cinfo, PFILE* infile)
{
	pak_src_ptr src;

	/* The source object and input buffer are made permanent so that a series
	* of JPEG images can be read from the same file by calling jpeg_stdio_src
	* only before the first one.  (If we discarded the buffer at the end of
	* one image, we'd likely lose the start of the next one.)
	* This makes it unsafe to use this manager and a different source
	* manager serially with the same JPEG object.  Caveat programmer.
	*/
	if (cinfo->src == nullptr) {	/* first time for this JPEG object? */
		cinfo->src = (struct jpeg_source_mgr *)
			(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
			sizeof(pak_source_mgr));
		src = (pak_src_ptr) cinfo->src;
		src->buffer = (JOCTET *)
			(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
			INPUT_BUF_SIZE * sizeof(JOCTET));
	}

	src = (pak_src_ptr) cinfo->src;
	src->pub.init_source = init_source;
	src->pub.fill_input_buffer = fill_input_buffer;
	src->pub.skip_input_data = skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
	src->pub.term_source = term_source;
	src->infile = infile;
	src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
	src->pub.next_input_byte = nullptr; /* until buffer loaded */
}


Image* GetJPEGImage(const std::string& theFileName)
{
	PFILE *fp;

	if ((fp = p_fopen(theFileName.c_str(), "rb")) == nullptr)
		return nullptr;

	struct jpeg_decompress_struct cinfo{};
	struct my_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	if (setjmp(jerr.setjmp_buffer))
	{
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */
		jpeg_destroy_decompress(&cinfo);
		p_fclose(fp);
		return 0;
	}

	jpeg_create_decompress(&cinfo);
	jpeg_pak_src(&cinfo, fp);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);
	int row_stride = cinfo.output_width * cinfo.output_components;

	unsigned char** buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	uint32_t* aBits = new uint32_t[cinfo.output_width*cinfo.output_height];
	uint32_t* q = aBits;

	if (cinfo.output_components==1)
	{
		while (cinfo.output_scanline < cinfo.output_height)
		{
			jpeg_read_scanlines(&cinfo, buffer, 1);

			unsigned char* p = *buffer;
			for (unsigned int i = 0; i < cinfo.output_width; i++)
			{
				int r = *p++;
				*q++ = 0xFF000000 | (r << 16) | (r << 8) | (r);
			}
		}
	}
	else
	{
		while (cinfo.output_scanline < cinfo.output_height)
		{
			jpeg_read_scanlines(&cinfo, buffer, 1);

			unsigned char* p = *buffer;
			for (unsigned int i = 0; i < cinfo.output_width; i++)
			{
				int r = *p++;
				int g = *p++;
				int b = *p++;

				*q++ = 0xFF000000 | (r << 16) | (g << 8) | (b);
			}
		}
	}

	Image* anImage = new Image();
	anImage->mWidth = cinfo.output_width;
	anImage->mHeight = cinfo.output_height;
	anImage->mBits = aBits;

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	p_fclose(fp);

	return anImage;
}

int ImageLib::gAlphaComposeColor = 0xFFFFFF;
bool ImageLib::gAutoLoadAlpha = true;
bool ImageLib::gIgnoreJPEG2000Alpha = true;

static unsigned char Sample(int w, int h, const unsigned char *pData, int u, int v, int Offset, int ScaleW, int ScaleH, int Bpp)
{
	int Value = 0;
	for(int x = 0; x < ScaleW; x++)
		for(int y = 0; y < ScaleH; y++)
			Value += pData[((v+y)*w+(u+x))*Bpp+Offset];
	return Value/(ScaleW*ScaleH);
}

static unsigned char *Rescale(int Width, int Height, int NewWidth, int NewHeight, const unsigned char *pData)
{
	unsigned char *pTmpData;
	int ScaleW = Width/NewWidth;
	int ScaleH = Height/NewHeight;

	int Bpp = 4;

	pTmpData = new unsigned char[NewWidth*NewHeight*Bpp];

	int c = 0;
	for(int y = 0; y < NewHeight; y++)
		for(int x = 0; x < NewWidth; x++, c++)
		{
			pTmpData[c*Bpp] = Sample(Width, Height, pData, x*ScaleW, y*ScaleH, 0, ScaleW, ScaleH, Bpp);
			pTmpData[c*Bpp+1] = Sample(Width, Height, pData, x*ScaleW, y*ScaleH, 1, ScaleW, ScaleH, Bpp);
			pTmpData[c*Bpp+2] = Sample(Width, Height, pData, x*ScaleW, y*ScaleH, 2, ScaleW, ScaleH, Bpp);
			if(Bpp == 4)
				pTmpData[c*Bpp+3] = Sample(Width, Height, pData, x*ScaleW, y*ScaleH, 3, ScaleW, ScaleH, Bpp);
		}

	return pTmpData;
}

using ImageLoader = Image* (*)(const std::string&);
using ImageExtEntry = std::pair<std::string_view, ImageLoader>;
static constexpr std::array<ImageExtEntry, 4> kImageExts = {
	ImageExtEntry{ ".png"sv, GetPNGImage },
	ImageExtEntry{ ".jpg"sv, GetJPEGImage },
	ImageExtEntry{ ".gif"sv, GetGIFImage },
	ImageExtEntry{ ".tga"sv, GetTGAImage },
};

static bool EqualsIgnoreCase(std::string_view theLeft, std::string_view theRight)
{
	if (theLeft.size() != theRight.size())
		return false;

	for (size_t i = 0; i < theLeft.size(); i++)
	{
		const unsigned char aLeftChar = static_cast<unsigned char>(theLeft[i]);
		const unsigned char aRightChar = static_cast<unsigned char>(theRight[i]);
		if (std::tolower(aLeftChar) != std::tolower(aRightChar))
			return false;
	}

	return true;
}

static std::mutex gMissingDirMutex;
static std::unordered_set<std::string> gMissingDirCache;

static bool CheckSinglePath(std::string_view thePath)
{
	if (thePath.empty())
		return false;

	if (gPakInterface)
	{
		if (gPakInterface->mPakRecordMap.contains(PakInterface::NormalizePakPath(thePath)))
			return true;
	}

	const std::string aPathString(thePath);
	if (!Sexy::IsPathRooted(aPathString))
	{
		const auto& aResourceBase = Sexy::GetResourceFolder();
		if (!aResourceBase.empty())
		{
			const std::string aDir = Sexy::GetFileDir(aPathString);
			if (!aDir.empty())
			{
				std::scoped_lock lock(gMissingDirMutex);
				if (gMissingDirCache.contains(aDir))
					return false;
			}

			if (Sexy::FileExists(Sexy::GetResourcePath(aPathString)))
				return true;

			if (!aDir.empty())
			{
				if (!Sexy::FileExists(Sexy::GetResourcePath(aDir)))
				{
					std::scoped_lock lock(gMissingDirMutex);
					gMissingDirCache.insert(aDir);
				}
			}

			return false;
		}
	}

	return Sexy::FileExists(aPathString);
}

static bool FastFileExists(std::string_view thePath)
{
	if (thePath.empty())
		return false;

	const auto aFilePath = Sexy::PathFromU8(thePath);
	if (aFilePath.has_extension())
		return CheckSinglePath(thePath);

	std::string aCandidate(thePath);
	const auto aBaseLen = aCandidate.size();
	for (const auto& [aExt, _] : kImageExts)
	{
		aCandidate.resize(aBaseLen);
		aCandidate.append(aExt);
		if (CheckSinglePath(aCandidate))
			return true;
	}

	return false;
}

static Image* TryLoadByExt(const std::string& theBaseName, std::string_view theExt)
{
	for (const auto& [aKnownExt, aLoader] : kImageExts)
	{
		if (theExt.empty() || EqualsIgnoreCase(theExt, aKnownExt))
		{
			if (Image* aImage = aLoader(theBaseName + std::string(aKnownExt)))
				return aImage;
		}
	}
	return nullptr;
}

static void ComposeAlpha(Image* theImage, Image* theAlphaImage)
{
	if (theImage->mWidth != theAlphaImage->mWidth ||
		theImage->mHeight != theAlphaImage->mHeight)
		return;

	uint32_t* aDstBits = theImage->mBits;
	const uint32_t* aSrcBits = theAlphaImage->mBits;
	const int aSize = theImage->mWidth * theImage->mHeight;

	for (int i = 0; i < aSize; i++)
		aDstBits[i] = (aDstBits[i] & 0x00FFFFFF) | ((aSrcBits[i] & 0xFF) << 24);
}

static void ApplyAlphaAsImage(Image* theImage, uint32_t theBaseColor)
{
	uint32_t* aBits = theImage->mBits;
	const int aSize = theImage->mWidth * theImage->mHeight;

	for (int i = 0; i < aSize; i++)
		aBits[i] = theBaseColor | ((aBits[i] & 0xFF) << 24);
}

Image* ImageLib::GetImage(const std::string& theFilename, bool lookForAlphaImage)
{
	if (!gAutoLoadAlpha)
		lookForAlphaImage = false;

	const auto aLastSlashPos = theFilename.rfind('/');
	const auto aLastDotPos = theFilename.rfind('.');

	std::string_view anExt;
	std::string aFilename;

	if (aLastDotPos != std::string::npos &&
		(aLastSlashPos == std::string::npos || aLastDotPos > aLastSlashPos))
	{
		anExt = std::string_view(theFilename).substr(aLastDotPos);
		aFilename = theFilename.substr(0, aLastDotPos);
	}
	else
		aFilename = theFilename;

	// Load image, trying each supported format
	Image* anImage = TryLoadByExt(aFilename, anExt);

	// Downscale only when configured to do so
#if IMG_DOWNSCALE != 1
	if (anImage)
	{
		const int aNewWidth = anImage->mWidth / IMG_DOWNSCALE;
		const int aNewHeight = anImage->mHeight / IMG_DOWNSCALE;
		if (aNewWidth > 0 && aNewHeight > 0)
		{
			auto* aNewData = Rescale(anImage->mWidth, anImage->mHeight, aNewWidth, aNewHeight, (unsigned char*)anImage->mBits);
			delete[] anImage->mBits;
			anImage->mBits = (uint32_t*)aNewData;
			anImage->mWidth = aNewWidth;
			anImage->mHeight = aNewHeight;
		}
	}
#endif

	// Probe alpha images with fast existence check
	Image* anAlphaImage = nullptr;
	if (lookForAlphaImage)
	{
		const auto slashEnd = (aLastSlashPos != std::string::npos) ? aLastSlashPos + 1 : 0;
		const std::string alphaPath1 = theFilename.substr(0, slashEnd) + "_" +
			theFilename.substr(slashEnd);

		if (FastFileExists(alphaPath1))
			anAlphaImage = GetImage(alphaPath1, false);

		if (!anAlphaImage)
		{
			const std::string alphaPath2 = theFilename + "_";
			if (FastFileExists(alphaPath2))
				anAlphaImage = GetImage(alphaPath2, false);
		}
	}

	// Compose alpha channel with image
	if (anAlphaImage)
	{
		if (anImage)
		{
			ComposeAlpha(anImage, anAlphaImage);
			delete anAlphaImage;
		}
		else
		{
			anImage = anAlphaImage;
			ApplyAlphaAsImage(anImage, static_cast<uint32_t>(gAlphaComposeColor));
		}
	}

	return anImage;
}

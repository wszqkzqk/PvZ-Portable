{
	uint32_t* aDestPixelsRow = ((uint32_t*) GetBits()) + (theY * mWidth) + theX;
	SRC_TYPE* aSrcPixelsRow = aSrcBits + (theSrcRect.mY * theImage->mWidth) + theSrcRect.mX;

	if (theColor == Color::White)
	{
		if (aSrcMemoryImage->mHasAlpha)
		{
			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				uint32_t* aDestPixels = aDestPixelsRow;
				SRC_TYPE* aSrcPtr = aSrcPixelsRow;

				for (int x = 0; x < theSrcRect.mWidth; x++)
				{
					uint32_t src = NEXT_SRC_COLOR;
					uint32_t dest = *aDestPixels;

					int a = (src&0xFF000000)>>24;
					int r = aMaxTable[((dest & 0xFF0000) + (((src & 0xFF0000)*a)>>8)) >> 16];
					int g = aMaxTable[((dest & 0x00FF00) + (((src & 0x00FF00)*a)>>8)) >> 8 ];
					int b = aMaxTable[((dest & 0x0000FF) + (((src & 0x0000FF)*a)>>8))      ];

					*(aDestPixels++) = (dest & 0xFF000000) | (r << 16) | (g << 8) | (b);
				}

				aDestPixelsRow += mWidth;
				aSrcPixelsRow += theImage->mWidth;
			}
		}
		else
		{
			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				uint32_t* aDestPixels = aDestPixelsRow;
				SRC_TYPE* aSrcPtr = aSrcPixelsRow;

				for (int x = 0; x < theSrcRect.mWidth; x++)
				{
					uint32_t src = NEXT_SRC_COLOR;
					uint32_t dest = *aDestPixels;

					int r = aMaxTable[((dest & 0xFF0000) + (src & 0xFF0000)) >> 16];
					int g = aMaxTable[((dest & 0x00FF00) + (src & 0x00FF00)) >> 8 ];
					int b = aMaxTable[((dest & 0x0000FF) + (src & 0x0000FF))      ];

					*(aDestPixels++) = (dest & 0xFF000000) | (r << 16) | (g << 8) | (b);
				}

				aDestPixelsRow += mWidth;
				aSrcPixelsRow += theImage->mWidth;
			}
		}
	}
	else
	{
		int ca = theColor.mAlpha;
		int cr = (theColor.mRed * ca) / 255;
		int cg = (theColor.mGreen * ca) / 255;
		int cb = (theColor.mBlue * ca) / 255;

		if (aSrcMemoryImage->mHasAlpha)
		{
			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				uint32_t* aDestPixels = aDestPixelsRow;
				SRC_TYPE* aSrcPtr = aSrcPixelsRow;

				for (int x = 0; x < theSrcRect.mWidth; x++)
				{
					uint32_t src = NEXT_SRC_COLOR;
					uint32_t dest = *aDestPixels;

					int a = (src&0xFF000000)>>24;
					int r = aMaxTable[((dest & 0xFF0000) + (((((src & 0xFF0000) * cr) >> 8)*a)>>8)) >> 16];
					int g = aMaxTable[((dest & 0x00FF00) + (((((src & 0x00FF00) * cg) >> 8)*a)>>8)) >>  8];
					int b = aMaxTable[((dest & 0x0000FF) + (((((src & 0x0000FF) * cb) >> 8)*a)>>8))      ];

					*(aDestPixels++) = (dest & 0xFF000000) | (r << 16) | (g << 8) | (b);
				}

				aDestPixelsRow += mWidth;
				aSrcPixelsRow += theImage->mWidth;
			}
		}
		else
		{
			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				uint32_t* aDestPixels = aDestPixelsRow;
				SRC_TYPE* aSrcPtr = aSrcPixelsRow;

				for (int x = 0; x < theSrcRect.mWidth; x++)
				{
					uint32_t src = NEXT_SRC_COLOR;
					uint32_t dest = *aDestPixels;

					int r = aMaxTable[((dest & 0xFF0000) + (((src & 0xFF0000) * cr) >> 8)) >> 16];
					int g = aMaxTable[((dest & 0x00FF00) + (((src & 0x00FF00) * cg) >> 8)) >>  8];
					int b = aMaxTable[((dest & 0x0000FF) + (((src & 0x0000FF) * cb) >> 8))      ];

					*(aDestPixels++) = (dest & 0xFF000000) | (r << 16) | (g << 8) | (b);
				}

				aDestPixelsRow += mWidth;
				aSrcPixelsRow += theImage->mWidth;
			}
		}
	}
}

{
	if (!LockSurface())
		return;

	int aFPSrcY = theSrcRect.mY * 0x10000;

	int anAddX = (theSrcRect.mWidth * 0x10000) / theDestRect.mWidth;
	int anAddY = (theSrcRect.mHeight * 0x10000) / theDestRect.mHeight;

	if (theColor == Color::White)
	{
		if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 16)
		{
			ushort* aDestPixelsRow = ((ushort*) mLockedSurfaceDesc.lpSurface) + (theDestRect.mY * mLockedSurfaceDesc.lPitch/2) + theDestRect.mX;

			uint32_t aRMask = mLockedSurfaceDesc.ddpfPixelFormat.dwRBitMask;
			uint32_t aGMask = mLockedSurfaceDesc.ddpfPixelFormat.dwGBitMask;
			uint32_t aBMask = mLockedSurfaceDesc.ddpfPixelFormat.dwBBitMask;

			// unused
			// uint32_t aRRoundAdd = aRMask >> 1;
			// uint32_t aGRoundAdd = aGMask >> 1;
			// uint32_t aBRoundAdd = aBMask >> 1;

			for (int y = 0; y < theDestRect.mHeight; y++)
			{
				int aFPSrcX = theSrcRect.mX * 0x10000;

				ushort* aDestPixels = aDestPixelsRow;
				SRC_TYPE* aSrcPixelsRow = aSrcBits + (aSrcMemoryImage->mWidth * (aFPSrcY >> 16));

				for (int x = 0; x < theDestRect.mWidth; x++, aFPSrcX += anAddX)
				{
					SRC_TYPE* aSrcPixels = aSrcPixelsRow + (aFPSrcX >> 16);

					uint32_t src = READ_COLOR(aSrcPixels);
					uint32_t dest = *aDestPixels;

					int a = src >> 24;

					if (a != 0)
					{
						int oma = 256 - a;

						*(aDestPixels _PLUSPLUS) =
							src +
							(((((dest & aRMask) * oma)) >> 8) & aRMask) +
							(((((dest & aGMask) * oma)) >> 8) & aGMask) +
							(((((dest & aBMask) * oma)) >> 8) & aBMask);
					}
					else
						aDestPixels _PLUSPLUS;
				}

				aDestPixelsRow += mLockedSurfaceDesc.lPitch/2;
				aFPSrcY += anAddY;
			}
		}
		else if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 32)
		{
			uint32_t* aDestPixelsRow = ((uint32_t*) mLockedSurfaceDesc.lpSurface) + (theDestRect.mY * mLockedSurfaceDesc.lPitch/4) + theDestRect.mX;

			uint32_t aRMask = mLockedSurfaceDesc.ddpfPixelFormat.dwRBitMask;
			uint32_t aGMask = mLockedSurfaceDesc.ddpfPixelFormat.dwGBitMask;
			uint32_t aBMask = mLockedSurfaceDesc.ddpfPixelFormat.dwBBitMask;

			// unused
			// uint32_t aRRoundAdd = aRMask >> 1;
			// uint32_t aGRoundAdd = aGMask >> 1;
			// uint32_t aBRoundAdd = aBMask >> 1;

			for (int y = 0; y < theDestRect.mHeight; y++)
			{
				int aFPSrcX = theSrcRect.mX * 0x10000;

				uint32_t* aDestPixels = aDestPixelsRow;
				SRC_TYPE* aSrcPixelsRow = aSrcBits + (aSrcMemoryImage->mWidth * (aFPSrcY >> 16));

				for (int x = 0; x < theDestRect.mWidth; x++, aFPSrcX += anAddX)
				{
					SRC_TYPE* aSrcPixels = aSrcPixelsRow + (aFPSrcX >> 16);

					uint32_t src = READ_COLOR(aSrcPixels);
					uint32_t dest = *aDestPixels;

					int a = src >> 24;

					if (a != 0)
					{
						int oma = 256 - a;

						*(aDestPixels _PLUSPLUS) =
							src +
							(((((dest & aRMask) * oma)) >> 8) & aRMask) +
							(((((dest & aGMask) * oma)) >> 8) & aGMask) +
							(((((dest & aBMask) * oma)) >> 8) & aBMask);
					}
					else
						aDestPixels _PLUSPLUS;
				}

				aDestPixelsRow += mLockedSurfaceDesc.lPitch/4;
				aFPSrcY += anAddY;
			}
		}
	}
	else
	{
		int ca = theColor.mAlpha;
		int cr = (theColor.mRed * ca) / 255;
		int cg = (theColor.mGreen * ca) / 255;
		int cb = (theColor.mBlue * ca) / 255;

		if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 16)
		{
			ushort* aDestPixelsRow = ((ushort*) mLockedSurfaceDesc.lpSurface) + (theDestRect.mY * mLockedSurfaceDesc.lPitch/2) + theDestRect.mX;

			uint32_t aRMask = mLockedSurfaceDesc.ddpfPixelFormat.dwRBitMask;
			uint32_t aGMask = mLockedSurfaceDesc.ddpfPixelFormat.dwGBitMask;
			uint32_t aBMask = mLockedSurfaceDesc.ddpfPixelFormat.dwBBitMask;

			// unused
			// uint32_t aRRoundAdd = aRMask >> 1;
			// uint32_t aGRoundAdd = aGMask >> 1;
			// uint32_t aBRoundAdd = aBMask >> 1;

			for (int y = 0; y < theDestRect.mHeight; y++)
			{
				int aFPSrcX = theSrcRect.mX * 0x10000;

				ushort* aDestPixels = aDestPixelsRow;
				SRC_TYPE* aSrcPixelsRow = aSrcBits + (aSrcMemoryImage->mWidth * (aFPSrcY >> 16));

				for (int x = 0; x < theDestRect.mWidth; x++, aFPSrcX += anAddX)
				{
					SRC_TYPE* aSrcPixels = aSrcPixelsRow + (aFPSrcX >> 16);

					uint32_t src = READ_COLOR(aSrcPixels);
					uint32_t dest = *aDestPixels;

					int a = ((src >> 24) * ca) / 255;

					if (a != 0)
					{
						int oma = 256 - a;

						*(aDestPixels _PLUSPLUS) =
							(((((dest & aRMask) * oma) + ((src & aRMask) * cr)) >> 8) & aRMask) +
							(((((dest & aGMask) * oma) + ((src & aGMask) * cg)) >> 8) & aGMask) +
							(((((dest & aBMask) * oma) + ((src & aBMask) * cb)) >> 8) & aBMask);
					}
					else
						aDestPixels _PLUSPLUS;
				}

				aDestPixelsRow += mLockedSurfaceDesc.lPitch/2;
				aFPSrcY += anAddY;
			}
		}
		else if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 32)
		{
			uint32_t* aDestPixelsRow = ((uint32_t*) mLockedSurfaceDesc.lpSurface) + (theDestRect.mY * mLockedSurfaceDesc.lPitch/4) + theDestRect.mX;

			uint32_t aRMask = mLockedSurfaceDesc.ddpfPixelFormat.dwRBitMask;
			uint32_t aGMask = mLockedSurfaceDesc.ddpfPixelFormat.dwGBitMask;
			uint32_t aBMask = mLockedSurfaceDesc.ddpfPixelFormat.dwBBitMask;

			// unused
			// uint32_t aRRoundAdd = aRMask >> 1;
			// uint32_t aGRoundAdd = aGMask >> 1;
			// uint32_t aBRoundAdd = aBMask >> 1;

			for (int y = 0; y < theDestRect.mHeight; y++)
			{
				int aFPSrcX = theSrcRect.mX * 0x10000;

				uint32_t* aDestPixels = aDestPixelsRow;
				SRC_TYPE* aSrcPixelsRow = aSrcBits + (aSrcMemoryImage->mWidth * (aFPSrcY >> 16));

				for (int x = 0; x < theDestRect.mWidth; x++, aFPSrcX += anAddX)
				{
					SRC_TYPE* aSrcPixels = aSrcPixelsRow + (aFPSrcX >> 16);

					uint32_t src = READ_COLOR(aSrcPixels);
					uint32_t dest = *aDestPixels;

					int a = ((src >> 24) * ca) / 255;
					int oma = 256 - a;

					if (a != 0)
					{
						// redundant line
						// int oma = 256 - a;

						*(aDestPixels _PLUSPLUS) =
							(((((dest & aRMask) * oma) + ((src & aRMask) * cr)) >> 8) & aRMask) +
							(((((dest & aGMask) * oma) + ((src & aGMask) * cg)) >> 8) & aGMask) +
							(((((dest & aBMask) * oma) + ((src & aBMask) * cb)) >> 8) & aBMask);
					}
					else
						aDestPixels _PLUSPLUS;
				}

				aDestPixelsRow += mLockedSurfaceDesc.lPitch/4;
				aFPSrcY += anAddY;
			}
		}
	}

	UnlockSurface();
}

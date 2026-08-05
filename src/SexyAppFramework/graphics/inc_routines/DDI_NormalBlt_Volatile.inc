{
	if (mNoLock)
		return;

	// Special path that avoids generating cached native alpha data

	LPDIRECTDRAWSURFACE aSurface = GetSurface();
	(void)aSurface; // unused

	if (!LockSurface())
		return;

	if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 16)
	{
		ushort* aDestPixelsRow = ((ushort*) mLockedSurfaceDesc.lpSurface) + (theY * mLockedSurfaceDesc.lPitch/2) + theX;
		SRC_TYPE* aSrcPixelsRow = aSrcBits +
			(theSrcRect.mY * theImage->mWidth) + theSrcRect.mX;

		uint32_t aRMask = mLockedSurfaceDesc.ddpfPixelFormat.dwRBitMask;
		uint32_t aGMask = mLockedSurfaceDesc.ddpfPixelFormat.dwGBitMask;
		uint32_t aBMask = mLockedSurfaceDesc.ddpfPixelFormat.dwBBitMask;

		uint32_t aRRoundAdd = aRMask >> 1;
		uint32_t aGRoundAdd = aGMask >> 1;
		uint32_t aBRoundAdd = aBMask >> 1;

#ifdef OPTIMIZE_SOFTWARE_DRAWING
		bool performNormalBlit = true;
		if (gOptimizeSoftwareDrawing) // we know the ordering of the bit masks.
		{
			if (aGMask == 0x7E0)
			{
				performNormalBlit = false;
				for (int y = 0; y < theSrcRect.mHeight; y++)
				{
					ushort* aDestPixels = aDestPixelsRow;
					SRC_TYPE* aSrcPixels = aSrcPixelsRow;

					for (int x = 0; x < theSrcRect.mWidth; x++)
					{
						uint32_t src = NEXT_SRC_COLOR;
						uint32_t dest = *aDestPixels;

						int a = src >> 27;
						//int oma = 256 - a;

						src = ((src & 0xFC00) << 11) + ((src >> 8) & 0xF800) + ((src >> 3) & 0x1F);
						dest = (dest | (dest << 16)) & 0x7E0F81F;
						dest += ((src - dest) * a) >> 5;
						dest &= 0x7E0F81F;
						*(aDestPixels++) = dest | (dest >> 16);
							//(((((dest & aBMask) * oma) + (((src & 0x000000FF) * a * aBMask) >> 8) + aBRoundAdd) >> 8) & aBMask) +
							//(((((dest & aGMask) * oma) + ((((src & 0x0000FF00) >> 8) * a * aGMask) >> 8) + aGRoundAdd) >> 8) & aGMask) +
							//(((((dest & aRMask) * oma) + ((((src & 0x00FF0000) >> 16) * a * aRMask) >> 8) + aRRoundAdd) >> 8) & aRMask);
					}

					aDestPixelsRow += mLockedSurfaceDesc.lPitch/2;
					aSrcPixelsRow += theImage->mWidth;
				}
			}
			else if (aGMask == 0x3E0)
			{
				performNormalBlit = false;
				for (int y = 0; y < theSrcRect.mHeight; y++)
				{
					ushort* aDestPixels = aDestPixelsRow;
					SRC_TYPE* aSrcPixels = aSrcPixelsRow;

					for (int x = 0; x < theSrcRect.mWidth; x++)
					{
						uint32_t src = NEXT_SRC_COLOR;
						uint32_t dest = *aDestPixels;

						int a = src >> 27;
						//int oma = 256 - a;

						src = ((src & 0xF800) << 10) + ((src >> 9) & 0x7C00) + ((src >> 3) & 0x1F);
						dest = (dest | (dest << 16)) & 0x3E07C1F;
						dest += ((src - dest) * a) >> 5;		// 1*d - a*d = (1-a)*d = oma*d :)
						dest &= 0x3E07C1F;
						*(aDestPixels++) = dest | (dest >> 16);
							//(((((dest & aBMask) * oma) + (((src & 0x000000FF) * a * aBMask) >> 8) + aBRoundAdd) >> 8) & aBMask) +
							//(((((dest & aGMask) * oma) + ((((src & 0x0000FF00) >> 8) * a * aGMask) >> 8) + aGRoundAdd) >> 8) & aGMask) +
							//(((((dest & aRMask) * oma) + ((((src & 0x00FF0000) >> 16) * a * aRMask) >> 8) + aRRoundAdd) >> 8) & aRMask);
					}


					aDestPixelsRow += mLockedSurfaceDesc.lPitch/2;
					aSrcPixelsRow += theImage->mWidth;
				}
			}
		}
		if (performNormalBlit)
#endif
		{
			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				ushort* aDestPixels = aDestPixelsRow;
				SRC_TYPE* aSrcPixels = aSrcPixelsRow;

				for (int x = 0; x < theSrcRect.mWidth; x++)
				{
					uint32_t src = NEXT_SRC_COLOR;
					uint32_t dest = *aDestPixels;

					int a = src >> 24;
					int oma = 256 - a;

					*(aDestPixels++) =
						(((((dest & aBMask) * oma) + (((src & 0x000000FF) * a * aBMask) >> 8) + aBRoundAdd) >> 8) & aBMask) +
						(((((dest & aGMask) * oma) + ((((src & 0x0000FF00) >> 8) * a * aGMask) >> 8) + aGRoundAdd) >> 8) & aGMask) +
						(((((dest & aRMask) * oma) + ((((src & 0x00FF0000) >> 16) * a * aRMask) >> 8) + aRRoundAdd) >> 8) & aRMask);
				}

				aDestPixelsRow += mLockedSurfaceDesc.lPitch/2;
				aSrcPixelsRow += theImage->mWidth;
			}
		}
	}
	else if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 32)
	{
		uint32_t* aDestPixelsRow = ((uint32_t*) mLockedSurfaceDesc.lpSurface) + (theY * mLockedSurfaceDesc.lPitch/4) + theX;
		SRC_TYPE* aSrcPixelsRow = aSrcBits +
			(theSrcRect.mY * theImage->mWidth) + theSrcRect.mX;

		uint32_t aRMask = mLockedSurfaceDesc.ddpfPixelFormat.dwRBitMask;
		uint32_t aGMask = mLockedSurfaceDesc.ddpfPixelFormat.dwGBitMask;
		uint32_t aBMask = mLockedSurfaceDesc.ddpfPixelFormat.dwBBitMask;

		uint32_t aRRoundAdd = aRMask >> 1;
		uint32_t aGRoundAdd = aGMask >> 1;
		uint32_t aBRoundAdd = aBMask >> 1;

#ifdef OPTIMIZE_SOFTWARE_DRAWING
		if (gOptimizeSoftwareDrawing)
		{
			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				uint32_t* aDestPixels = aDestPixelsRow;
				SRC_TYPE* aSrcPixels = aSrcPixelsRow;

				for (int x = 0; x < theSrcRect.mWidth; x++)
				{
					uint32_t src = NEXT_SRC_COLOR;
					uint32_t dest = *aDestPixels;

					int a = src >> 24;
					//int oma = 256 - a;
					uint32_t srctmp = src & 0xFF00FF;
					uint32_t desttmp = dest & 0xFF00FF;
					desttmp = (desttmp + (((srctmp - desttmp) * a) >> 8)) & 0xFF00FF;
					src &= 0x00FF00;
					dest &= 0x00FF00;
					dest = (dest + (((src - dest) * a) >> 8)) & 0x00FF00;

					*(aDestPixels++) = dest | desttmp;
						//(((((dest & aBMask) * oma) + ((((src & 0x000000FF) * a) >> 8) * aBMask) + aBRoundAdd) >> 8) & aBMask) +
						//(((((dest & aGMask) * oma) + (((((src & 0x0000FF00) >> 8) * a) >> 8) * aGMask) + aGRoundAdd) >> 8) & aGMask) +
						//(((((dest & aRMask) * oma) + (((((src & 0x00FF0000) >> 16) * a) >> 8) * aRMask) + aRRoundAdd) >> 8) & aRMask);
				}

				aDestPixelsRow += mLockedSurfaceDesc.lPitch/4;
				aSrcPixelsRow += theImage->mWidth;
			}
		}
		else
#endif
		{
			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				uint32_t* aDestPixels = aDestPixelsRow;
				SRC_TYPE* aSrcPixels = aSrcPixelsRow;

				for (int x = 0; x < theSrcRect.mWidth; x++)
				{
					uint32_t src = NEXT_SRC_COLOR;
					uint32_t dest = *aDestPixels;

					int a = src >> 24;
					int oma = 256 - a;

					*(aDestPixels++) =
						(((((dest & aBMask) * oma) + ((((src & 0x000000FF) * a) >> 8) * aBMask) + aBRoundAdd) >> 8) & aBMask) +
						(((((dest & aGMask) * oma) + (((((src & 0x0000FF00) >> 8) * a) >> 8) * aGMask) + aGRoundAdd) >> 8) & aGMask) +
						(((((dest & aRMask) * oma) + (((((src & 0x00FF0000) >> 16) * a) >> 8) * aRMask) + aRRoundAdd) >> 8) & aRMask);
				}

				aDestPixelsRow += mLockedSurfaceDesc.lPitch/4;
				aSrcPixelsRow += theImage->mWidth;
			}
		}
	}

	UnlockSurface();
}

{
	double aCos = cos(theRot);
	double aSin = sin(theRot);

	int aCosLong = (int) (aCos * 0x10000);
	int aSinLong = (int) (aSin * 0x10000);

/* unused
#ifdef OPTIMIZE_SOFTWARE_DRAWING
	int aRotCenterXLong = (int)(theRotCenterX * 0x10000);
	int aRotCenterYLong = (int)(theRotCenterY * 0x10000);
#endif
*/

	if (!LockSurface())
		return;

	int* aMaxRedTable = mDDInterface->mRedAddTable;
	int* aMaxGreenTable = mDDInterface->mGreenAddTable;
	int* aMaxBlueTable = mDDInterface->mBlueAddTable;

	int aRedShift = mDDInterface->mRedShift;
	int aGreenShift = mDDInterface->mGreenShift;
	int aBlueShift = mDDInterface->mBlueShift;

	int aRedRightShift = 32 - mDDInterface->mRedBits;
	int aGreenRightShift = 24 - mDDInterface->mGreenBits;
	int aBlueRightShift = 16 - mDDInterface->mBlueBits;

	uint32_t aRMask = mLockedSurfaceDesc.ddpfPixelFormat.dwRBitMask;
	uint32_t aGMask = mLockedSurfaceDesc.ddpfPixelFormat.dwGBitMask;
	uint32_t aBMask = mLockedSurfaceDesc.ddpfPixelFormat.dwBBitMask;

	// unused
	// uint32_t aRRoundAdd = aRMask >> 1;
	// uint32_t aGRoundAdd = aGMask >> 1;
	// uint32_t aBRoundAdd = aBMask >> 1;

	if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 16)
	{
		ushort* aDestPixelsRow = ((ushort*) mLockedSurfaceDesc.lpSurface) + ((int)aDestRect.mY * mLockedSurfaceDesc.lPitch/2) + (int)aDestRect.mX;
		int aDestPixelsPitch = mLockedSurfaceDesc.lPitch/2;

		//TODO: if (theColor == Color::White)
		{
			int ca = theColor.mAlpha;
			int cr = (theColor.mRed * ca) / 255;
			int cg = (theColor.mGreen * ca) / 255;
			int cb = (theColor.mBlue * ca) / 255;

			#define DEST_PIXEL_TYPE ushort
			#define WRITE_PIXEL\
			{\
				ushort dest = *aDestPixels;\
				\
				int r = aMaxRedTable[((dest & aRMask)>>aRedShift) + \
					((((((src1 & 0xFF0000) * a1) + \
						((src2 & 0xFF0000) * a2) + \
						((src3 & 0xFF0000) * a3) + \
						((src4 & 0xFF0000) * a4)) >> aRedRightShift) * cr) >> 8)];\
				int g = aMaxGreenTable[((dest & aGMask)>>aGreenShift) + \
					(((((src1 & 0x00FF00) * a1) + \
						((src2 & 0x00FF00) * a2) + \
						((src3 & 0x00FF00) * a3) + \
						((src4 & 0x00FF00) * a4)) * cg) >> (aGreenRightShift + 8))];\
				int b = aMaxBlueTable[((dest & aBMask)>>aBlueShift) + \
					(((((src1 & 0x0000FF) * a1) + \
						((src2 & 0x0000FF) * a2) + \
						((src3 & 0x0000FF) * a3) + \
						((src4 & 0x0000FF) * a4)) * cb) >> (aBlueRightShift + 8))];\
				\
				*(aDestPixels++) = \
					(r << aRedShift) | \
					(g << aGreenShift) | \
					(b << aBlueShift); \
			}
			#include "BltRotatedHelper.inc"

			#undef WRITE_PIXEL
			#undef DEST_PIXEL_TYPE
		}
		/*else
		{

		}*/
	}
	else if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 32)
	{
		uint32_t* aDestPixelsRow = ((uint32_t*) mLockedSurfaceDesc.lpSurface) + ((int)aDestRect.mY * mLockedSurfaceDesc.lPitch/4) + (int)aDestRect.mX;
		int aDestPixelsPitch = mLockedSurfaceDesc.lPitch/4;

		//if (theColor == Color::White)
		{
			int ca = theColor.mAlpha;
			int cr = (theColor.mRed * ca) / 255;
			int cg = (theColor.mGreen * ca) / 255;
			int cb = (theColor.mBlue * ca) / 255;


			#define DEST_PIXEL_TYPE uint32_t
			#define WRITE_PIXEL\
			{\
				uint32_t dest = *aDestPixels;\
				\
				int r = aMaxRedTable[((dest & aRMask)>>aRedShift) + \
					((((((src1 & 0xFF0000) * a1) + \
						((src2 & 0xFF0000) * a2) + \
						((src3 & 0xFF0000) * a3) + \
						((src4 & 0xFF0000) * a4)) >> aRedRightShift) * cr) >> 8)];\
				int g = aMaxGreenTable[((dest & aGMask)>>aGreenShift) + \
					(((((src1 & 0x00FF00) * a1) + \
						((src2 & 0x00FF00) * a2) + \
						((src3 & 0x00FF00) * a3) + \
						((src4 & 0x00FF00) * a4)) * cg) >> (aGreenRightShift + 8))];\
				int b = aMaxBlueTable[((dest & aBMask)>>aBlueShift) + \
					(((((src1 & 0x0000FF) * a1) + \
						((src2 & 0x0000FF) * a2) + \
						((src3 & 0x0000FF) * a3) + \
						((src4 & 0x0000FF) * a4)) * cb) >> (aBlueRightShift + 8))];\
				\
				*(aDestPixels++) = \
					(r << aRedShift) | \
					(g << aGreenShift) | \
					(b << aBlueShift);\
			}

			#include "BltRotatedHelper.inc"

			#undef WRITE_PIXEL
			#undef DEST_PIXEL_TYPE


		}
		/*else
		{

		}*/
	}

	UnlockSurface();
}

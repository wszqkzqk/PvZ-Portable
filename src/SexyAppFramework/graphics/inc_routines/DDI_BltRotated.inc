{
	double aCos = cos(theRot);
	double aSin = sin(theRot);

	int aCosLong = (int) (aCos * 0x10000);
	int aSinLong = (int) (aSin * 0x10000);

	if (!LockSurface())
		return;

	int aRedShift = mDDInterface->mRedShift;
	int aGreenShift = mDDInterface->mGreenShift;
	int aBlueShift = mDDInterface->mBlueShift;

	int aRedRightShift = 32 - mDDInterface->mRedBits;
	int aGreenRightShift = 24 - mDDInterface->mGreenBits;
	int aBlueRightShift = 16 - mDDInterface->mBlueBits;

	int aNumBits = mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount;

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

		if (theColor == Color::White)
		{
			#define DEST_PIXEL_TYPE ushort
			#define WRITE_PIXEL\
			{\
				ushort dest = *aDestPixels;\
				int oma = 256 - (a1 + a2 + a3 + a4);\
				*(aDestPixels++) = \
					(((((dest & aBMask) * oma) >> 8) & aBMask) + \
						(((((src1 & 0x0000FF) * a1)  + \
							((src2 & 0x0000FF) * a2)  + \
							((src3 & 0x0000FF) * a3)  + \
							((src4 & 0x0000FF) * a4)) >> aBlueRightShift) << aBlueShift)) | \
					(((((dest & aGMask) * oma) >> 8) & aGMask) + \
						(((((src1 & 0x00FF00) * a1)  + \
							((src2 & 0x00FF00) * a2)  + \
							((src3 & 0x00FF00) * a3)  + \
							((src4 & 0x00FF00) * a4)) >> aGreenRightShift) << aGreenShift)) | \
					(((((dest & aRMask) * oma) >> 8) & aRMask) + \
						(((((src1 & 0xFF0000) * a1)  + \
							((src2 & 0xFF0000) * a2)  + \
							((src3 & 0xFF0000) * a3)  + \
							((src4 & 0xFF0000) * a4)) >> aRedRightShift) << aRedShift)); \
			}
			#define WRITE_PIXEL_OPTIMIZED_565\
			{\
				uint32_t dest = (*aDestPixels | (*aDestPixels << 16)) & 0x7E0F81F;\
				int oma = (256 - (a1 + a2 + a3 + a4)) >> 3;\
				uint32_t src_b = ((src1 & 0xFF00FF) * a1 + (src2 & 0xFF00FF) * a2 + (src3 & 0xFF00FF) * a3 + (src4 & 0xFF00FF) * a4) & 0xFF00FF00;\
				src_b = ((((src1 & 0xFF00) * a1 + (src2 & 0xFF00) * a2 + (src3 & 0xFF00) * a3 + (src4 & 0xFF00) * a4) & 0xFF0000) | src_b) >> 8;\
				uint32_t s = ((src_b & 0xfc00) << 11) + (src_b >> 8 & 0xf800) + (src_b >> 3 & 0x1f);\
				dest = ((dest * oma >> 5) & 0x7E0F81F) + s;\
				*(aDestPixels++) = dest | (dest >> 16);\
			}
			#define WRITE_PIXEL_OPTIMIZED_555\
			{\
				uint32_t dest = (*aDestPixels | (*aDestPixels << 16)) & 0x3E07C1F;\
				int oma = (256 - (a1 + a2 + a3 + a4)) >> 3;\
				uint32_t src_b = ((src1 & 0xFF00FF) * a1 + (src2 & 0xFF00FF) * a2 + (src3 & 0xFF00FF) * a3 + (src4 & 0xFF00FF) * a4) & 0xFF00FF00;\
				src_b = ((((src1 & 0xFF00) * a1 + (src2 & 0xFF00) * a2 + (src3 & 0xFF00) * a3 + (src4 & 0xFF00) * a4) & 0xFF0000) | src_b) >> 8;\
				uint32_t s = ((src_b & 0xf800) << 10) + (src_b >> 9 & 0x7c00) + (src_b >> 3 & 0x1f);\
				dest = ((dest * oma >> 5) & 0x3E07C1F) + s;\
				*(aDestPixels++) = dest | (dest >> 16);\
			}
			#define WRITE_PIXEL_OPTIMIZED

			#include "BltRotatedHelper.inc"

			#undef WRITE_PIXEL
			#undef DEST_PIXEL_TYPE
			#undef WRITE_PIXEL_OPTIMIZED
			#undef WRITE_PIXEL_OPTIMIZED_555
			#undef WRITE_PIXEL_OPTIMIZED_565
		}
		else
		{
			// Colorized version

			int ca = theColor.mAlpha;
			int cr = theColor.mRed + 1;
			int cg = theColor.mGreen + 1;
			int cb = theColor.mBlue + 1;

			#define DEST_PIXEL_TYPE ushort
			#define WRITE_PIXEL\
			{\
				ushort dest = *aDestPixels;\
				a1 = (a1*ca)>>8; a2 = (a2*ca)>>8; a3 = (a3*ca)>>8; a4 = (a4*ca)>>8; \
				int oma = 256 - (a1 + a2 + a3 + a4); \
				*(aDestPixels++) = \
					( ((((dest & aBMask) * oma) >> 8) & aBMask) + \
						(((( (((src1 & 0x0000FF) * a1)  + \
								((src2 & 0x0000FF) * a2)  + \
								((src3 & 0x0000FF) * a3)  + \
								((src4 & 0x0000FF) * a4)) >> 8) * cb) >> aBlueRightShift) << aBlueShift) ) | \
					( ((((dest & aGMask) * oma) >> 8) & aGMask) + \
						(((( (((src1 & 0x00FF00) * a1)  + \
								((src2 & 0x00FF00) * a2)  + \
								((src3 & 0x00FF00) * a3)  + \
								((src4 & 0x00FF00) * a4)) >> 8) * cg) >> aGreenRightShift) << aGreenShift) ) | \
					( ((((dest & aRMask) * oma) >> 8) & aRMask) + \
						(((( (((src1 & 0xFF0000) * a1)  + \
								((src2 & 0xFF0000) * a2)  + \
								((src3 & 0xFF0000) * a3)  + \
								((src4 & 0xFF0000) * a4)) >> 8) * cr) >> aRedRightShift) << aRedShift) ); \
			}

			#include "BltRotatedHelper.inc"

			#undef WRITE_PIXEL
			#undef DEST_PIXEL_TYPE
		}
	}
	else if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 32)
	{
		uint32_t* aDestPixelsRow = ((uint32_t*) mLockedSurfaceDesc.lpSurface) + ((int)aDestRect.mY * mLockedSurfaceDesc.lPitch/4) + (int)aDestRect.mX;
		int aDestPixelsPitch = mLockedSurfaceDesc.lPitch/4;

		if (theColor == Color::White)
		{
			#define DEST_PIXEL_TYPE uint32_t
			#define WRITE_PIXEL\
			{\
				uint32_t dest = *aDestPixels;\
				int oma = 256 - (a1 + a2 + a3 + a4);\
				*(aDestPixels++) = \
					(((((dest & aBMask) * oma) >> 8) & aBMask) + \
						(((((src1 & 0x0000FF) * a1)  + \
							((src2 & 0x0000FF) * a2)  + \
							((src3 & 0x0000FF) * a3)  + \
							((src4 & 0x0000FF) * a4)) >> aBlueRightShift) << aBlueShift)) | \
					(((((dest & aGMask) * oma) >> 8) & aGMask) + \
						(((((src1 & 0x00FF00) * a1)  + \
							((src2 & 0x00FF00) * a2)  + \
							((src3 & 0x00FF00) * a3)  + \
							((src4 & 0x00FF00) * a4)) >> aGreenRightShift) << aGreenShift)) | \
					(((((dest & aRMask) * oma) >> 8) & aRMask) + \
						(((((src1 & 0xFF0000) * a1)  + \
							((src2 & 0xFF0000) * a2)  + \
							((src3 & 0xFF0000) * a3)  + \
							((src4 & 0xFF0000) * a4)) >> aRedRightShift) << aRedShift));\
			}
			#define WRITE_PIXEL_OPTIMIZED\
			{\
				uint32_t dest = *aDestPixels;\
				int oma = 256 - (a1 + a2 + a3 + a4);\
				uint32_t src_b = ((src1 & 0xFF00FF) * a1 + (src2 & 0xFF00FF) * a2 + (src3 & 0xFF00FF) * a3 + (src4 & 0xFF00FF) * a4) & 0xFF00FF00;\
				uint32_t src_b2 = ((src1 & 0xFF00) * a1 + (src2 & 0xFF00) * a2 + (src3 & 0xFF00) * a3 + (src4 & 0xFF00) * a4) & 0xFF0000;\
				*(aDestPixels++) =\
						((((dest & 0xFF00FF) * oma + src_b) >> 8) & 0xFF00FF) |\
						((((dest & 0x00FF00) * oma + src_b2) >> 8) & 0x00FF00);\
			}



			#include "BltRotatedHelper.inc"

			#undef WRITE_PIXEL
			#undef WRITE_PIXEL_OPTIMIZED
			#undef DEST_PIXEL_TYPE
		}
		else
		{
			// Colorized version

			int ca = theColor.mAlpha;
			int cr = theColor.mRed + 1;
			int cg = theColor.mGreen + 1;
			int cb = theColor.mBlue + 1;

			#define DEST_PIXEL_TYPE uint32_t
			#define WRITE_PIXEL\
			{\
				uint32_t dest = *aDestPixels;\
				a1 = (a1*ca)>>8; a2 = (a2*ca)>>8; a3 = (a3*ca)>>8; a4 = (a4*ca)>>8; \
				int oma = 256 - (a1 + a2 + a3 + a4);\
				*(aDestPixels++) = \
					( ((((dest & aBMask) * oma) >> 8) & aBMask) + \
						(((( (((src1 & 0x0000FF) * a1)  + \
								((src2 & 0x0000FF) * a2)  + \
								((src3 & 0x0000FF) * a3)  + \
								((src4 & 0x0000FF) * a4)) >> 8) * cb) >> aBlueRightShift) << aBlueShift) ) | \
					( ((((dest & aGMask) * oma) >> 8) & aGMask) + \
						(((( (((src1 & 0x00FF00) * a1)  + \
								((src2 & 0x00FF00) * a2)  + \
								((src3 & 0x00FF00) * a3)  + \
								((src4 & 0x00FF00) * a4)) >> 8) * cg) >> aGreenRightShift) << aGreenShift) ) | \
					( ((((dest & aRMask) * oma) >> 8) & aRMask) + \
						(((( (((src1 & 0xFF0000) * a1)  + \
								((src2 & 0xFF0000) * a2)  + \
								((src3 & 0xFF0000) * a3)  + \
								((src4 & 0xFF0000) * a4)) >> 8) * cr) >> aRedRightShift) << aRedShift) );\
			}

			#include "BltRotatedHelper.inc"

			#undef WRITE_PIXEL
			#undef DEST_PIXEL_TYPE

		}
	}

	UnlockSurface();
}

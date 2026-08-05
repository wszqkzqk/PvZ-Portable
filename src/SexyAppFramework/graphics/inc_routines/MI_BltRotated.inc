{
	double aCos = cos(theRot);
	double aSin = sin(theRot);

	int aCosLong = (int) (aCos * 0x10000);
	int aSinLong = (int) (aSin * 0x10000);


	uint32_t* aDestPixelsRow = GetBits() + ((int)aDestRect.mY * mWidth) + (int)aDestRect.mX;
	int aDestPixelsPitch = mWidth;

	if (theColor == Color::White)
	{
		#define DEST_PIXEL_TYPE uint32_t
		#define WRITE_PIXEL\
		{\
			int a = a1+a2+a3+a4;			\
			if(a==0) /* transparent */ \
				aDestPixels++; \
			else \
			{ \
				uint32_t r = (((((src1&0xFF0000)*a1)) + (((src2&0xFF0000)*a2)) + (((src3&0xFF0000)*a3)) + (((src4&0xFF0000)*a4)))&0xFF000000); \
				uint32_t g = (((((src1&0x00FF00)*a1)) + (((src2&0x00FF00)*a2)) + (((src3&0x00FF00)*a3)) + (((src4&0x00FF00)*a4)))&0x00FF0000); \
				uint32_t b = (((((src1&0x0000FF)*a1)) + (((src2&0x0000FF)*a2)) + (((src3&0x0000FF)*a3)) + (((src4&0x0000FF)*a4)))&0x0000FF00); \
				if(a > 250) /* opaque */ \
					*aDestPixels++ = 0xFF000000 | (r>>8) | (g>>8) | (b>>8);\
				else /* blend */ \
				{\
					uint32_t destPixel = *aDestPixels; \
					uint32_t da = destPixel >> 24;\
					\
					uint32_t dr = (((destPixel&0xFF0000)*da)>>8) & 0xFF0000;\
					uint32_t dg = (((destPixel&0x00FF00)*da)>>8) & 0x00FF00;\
					uint32_t db = (((destPixel&0x0000FF)*da)>>8) & 0x0000FF;\
					\
					int finalAlpha = 256 - (((256 - a)*(256 - da))>>8);\
					r = ((r + (256-a)*dr)/finalAlpha) & 0xFF0000;\
					g = ((g + (256-a)*dg)/finalAlpha) & 0x00FF00;\
					b = ((b + (256-a)*db)/finalAlpha) & 0x0000FF;\
					\
					*aDestPixels++ = ((finalAlpha-1)<<24) | r | g | b;					\
				}\
			}	\
		}

		#include "BltRotatedHelper.inc"

		#undef WRITE_PIXEL
		#undef DEST_PIXEL_TYPE


	}
	else
	{
		int ca = theColor.mAlpha;
		int cr = theColor.mRed + 1;
		int cg = theColor.mGreen + 1;
		int cb = theColor.mBlue + 1;

		#define DEST_PIXEL_TYPE uint32_t
		#define WRITE_PIXEL\
		{\
			a1 = (a1*ca)>>8; a2 = (a2*ca)>>8; a3 = (a3*ca)>>8; a4 = (a4*ca)>>8; \
			int a = a1+a2+a3+a4;			\
			\
			if(a==0) /* transparent */ \
				aDestPixels++; \
			else \
			{ \
				uint32_t r = (cr * (((((src1&0xFF0000)*a1)) + (((src2&0xFF0000)*a2)) + (((src3&0xFF0000)*a3)) + (((src4&0xFF0000)*a4)))>>8)) & 0xFF000000 ; \
				uint32_t g = ((((((src1&0x00FF00)*a1)) + (((src2&0x00FF00)*a2)) + (((src3&0x00FF00)*a3)) + (((src4&0x00FF00)*a4)))&0x00FF0000) * cg) & 0xFF000000; \
				uint32_t b = ((((((src1&0x0000FF)*a1)) + (((src2&0x0000FF)*a2)) + (((src3&0x0000FF)*a3)) + (((src4&0x0000FF)*a4)))&0x0000FF00) * cb) & 0x00FF0000; \
				if(a > 250) /* opaque */ \
					*aDestPixels++ = 0xFF000000 | (r>>8) | (g>>16) | (b>>16);\
				else /* blend */ \
				{\
					uint32_t destPixel = *aDestPixels; \
					uint32_t da = destPixel >> 24;\
					\
					uint32_t dr = (((destPixel&0xFF0000)*da)>>8) & 0xFF0000;\
					uint32_t dg = (((destPixel&0x00FF00)*da)>>8) & 0x00FF00;\
					uint32_t db = (((destPixel&0x0000FF)*da)>>8) & 0x0000FF;\
					\
					int finalAlpha = 256 - (((256 - a)*(256 - da))>>8);\
					r = ((r + (256-a)*dr)/finalAlpha) & 0xFF0000;\
					g = (((g>>8) + (256-a)*dg)/finalAlpha) & 0x00FF00;\
					b = (((b>>8) + (256-a)*db)/finalAlpha) & 0x0000FF;\
					\
					*aDestPixels++ = ((finalAlpha-1)<<24) | r | g | b;					\
				}\
			}	\
		}

		#include "BltRotatedHelper.inc"

		#undef WRITE_PIXEL
		#undef DEST_PIXEL_TYPE


	}
}

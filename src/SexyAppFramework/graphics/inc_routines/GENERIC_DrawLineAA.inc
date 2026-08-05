{
	PIXEL_TYPE* aDestPixels = &aBits[aY0*STRIDE + aX0];
	uint32_t anErrAcc = (uint32_t)(fabs(theStartX - aX0)*fabs(theStartY - aY0) * 0x10000);
	if (dx >= dy) // mostly horizontal
	{
		//uint32_t anErrAdj = (((uint32_t)dy << 16) / (uint32_t)dx);
		uint32_t anErrAdj = (uint32_t)(dyd / dxd * 0x10000);

		uchar aWeight = 255 - (uchar)(anErrAcc >> 8);
		//uchar aWeight2 = (aWeight ^ 0xFF); // unused

		int a = CALC_WEIGHT_A(aWeight);
		int oma = 256 - a;
		PIXEL_TYPE dest = *aDestPixels;
		BLEND_PIXEL(aDestPixels);
		while (--dx)
		{
			anErrAcc += anErrAdj;
			if (anErrAcc >= 0x10000)
			{
				anErrAcc &= 0xFFFF;
				aDestPixels += STRIDE;
				aY0++;
			}
			aDestPixels += aXinc;

			uchar aWeight = 255 - (uchar)(anErrAcc >> 8);
			uchar aWeight2 = (aWeight ^ 0xFF);

			dest = *aDestPixels;
			a = CALC_WEIGHT_A(aWeight);
			oma = 256 - a;

			BLEND_PIXEL(aDestPixels);
			if (aY0 < mHeight-1)
			{
				dest = *(aDestPixels+STRIDE);
				a = CALC_WEIGHT_A(aWeight2);
				oma = 256 - a;
				BLEND_PIXEL(aDestPixels+STRIDE);
			}
		}
	}
	else // mostly vertical
	{
		//uint32_t anErrAdj = (uint32_t)(((uint32_t)dx << 16) / (uint32_t)dy);
		uint32_t anErrAdj = (uint32_t)(dxd / dyd * 0x10000);

		uchar aWeight = 255 - (uchar)(anErrAcc >> 8);
		//uchar aWeight2 = (aWeight ^ 0xFF); // unused

		int a = CALC_WEIGHT_A(aWeight);
		int oma = 256 - a;
		PIXEL_TYPE dest = *aDestPixels;
		BLEND_PIXEL(aDestPixels);
		while (--dy)
		{
			anErrAcc += anErrAdj;
			if (anErrAcc >= 0x10000)
			{
				anErrAcc &= 0xFFFF;
				aDestPixels += aXinc;
				aX0 += aXinc;
			}
			aDestPixels += STRIDE;

			uchar aWeight = 255 - (uchar)(anErrAcc >> 8);
			uchar aWeight2 = (aWeight ^ 0xFF);

			dest = *aDestPixels;
			int a = CALC_WEIGHT_A(aWeight);
			int oma = 256 - a;

			BLEND_PIXEL(aDestPixels);
			if (aX0 < STRIDE-1)
			{
				a = CALC_WEIGHT_A(aWeight2);
				oma = 256 - a;
				dest = *(aDestPixels+aXinc);
				BLEND_PIXEL(aDestPixels+aXinc);
			}
		}
	}
	aDestPixels = &aBits[aY1*STRIDE + aX1];
	uchar aWeight = 255 - (uchar)(anErrAcc >> 8);
	// uchar aWeight2 = (aWeight ^ 0xFF); // unused

	int a = CALC_WEIGHT_A(aWeight);
	int oma = 256 - a;
	PIXEL_TYPE dest = *aDestPixels;
	BLEND_PIXEL(aDestPixels);
}

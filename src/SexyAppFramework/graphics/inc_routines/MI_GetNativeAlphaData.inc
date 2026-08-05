{
	uint32_t* anAlphaData = new uint32_t[mWidth*mHeight];

	if (theNumBits == 16)
	{
		for (int i = 0; i < mWidth*mHeight; i++)
		{
			uint32_t val = NEXT_SRC_COLOR;

			int anAlpha = val >> 24;

			int r = ((val >> 16) & 0xFF) * anAlpha >> 8;
			int g = ((val >> 8 ) & 0xFF) * anAlpha >> 8;
			int b = ((val      ) & 0xFF) * anAlpha >> 8;

			anAlphaData[i] =
				((((r * theRMask) >> 8) & theRMask) |
				(((g * theGMask) >> 8) & theGMask) |
				(((b * theBMask) >> 8) & theBMask) |
				(anAlpha << 24));
		}
	}
	else if (theNumBits == 32)
	{
		for (int i = 0; i < mWidth*mHeight; i++)
		{
			uint32_t val = NEXT_SRC_COLOR;

			int anAlpha = val >> 24;

			int r = (((val >> 16) & 0xFF) * anAlpha) >> 8;
			int g = (((val >> 8 ) & 0xFF) * anAlpha) >> 8;
			int b = (((val      ) & 0xFF) * anAlpha) >> 8;

			anAlphaData[i] =
				(((r * theRMask) >> 8) & theRMask) |
				(((g * theGMask) >> 8) & theGMask) |
				(((b * theBMask) >> 8) & theBMask) |
				(anAlpha << 24);
		}
	}

	mNativeAlphaData = anAlphaData;
}

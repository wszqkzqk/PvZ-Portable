if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 16)
{
	ushort* aDestPixelsRow = ((ushort*) mLockedSurfaceDesc.lpSurface) + (theY * mLockedSurfaceDesc.lPitch/2) + theX;
	uchar* aRLAlphaDataRow = aSrcRLAlphaData + (theSrcRect.mY * theImage->mWidth) + theSrcRect.mX;

	uint32_t aRMask = mLockedSurfaceDesc.ddpfPixelFormat.dwRBitMask;
	uint32_t aGMask = mLockedSurfaceDesc.ddpfPixelFormat.dwGBitMask;
	uint32_t aBMask = mLockedSurfaceDesc.ddpfPixelFormat.dwBBitMask;

	// unused
	//uint32_t aRRoundAdd = aRMask >> 1;
	//uint32_t aGRoundAdd = aGMask >> 1;
	//uint32_t aBRoundAdd = aBMask >> 1;

	if (theColor == Color::White)
	{
#ifdef OPTIMIZE_SOFTWARE_DRAWING
		if (aGMask == 0x7E0) // 5-6-5 optimizations
		{
			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				ushort* aDestPixels = aDestPixelsRow;
				uchar* aRLAlphaData = aRLAlphaDataRow;

				aSrcPixels = aSrcPixelsRow;

				for (int aSpanLeft = theSrcRect.mWidth; aSpanLeft > 0; )
				{
					uint32_t src = PEEK_SRC_COLOR;
					uchar rl = *aRLAlphaData;

					if (rl > aSpanLeft)
						rl = aSpanLeft;

					int oma = 256 - (src >> 24);

					if (oma == 1) // Fully opaque
					{
						for (int i = 0; i < rl; i++)
							*aDestPixels _PLUSPLUS = NEXT_SRC_COLOR;
					}
					else
					if (oma == 256) // Fully transparent
					{
						aDestPixels _PLUSEQUALS rl;
						aSrcPixels += rl;
					}
					else // Partially transparent
					{
						oma >>= 3;
						aSrcPixels++;
						uint32_t dest = (((*aDestPixels | (*aDestPixels << 16)) & 0x7E0F81F) * oma >> 5) & 0x7E0F81F;
						*(aDestPixels _PLUSPLUS) = src + (dest | (dest >> 16));

						for (int i = 1; i < rl; i++)
						{
							uint32_t src = NEXT_SRC_COLOR;
							int oma = (256 - (src >> 24)) >> 3;
							uint32_t dest = (((*aDestPixels | (*aDestPixels << 16)) & 0x7E0F81F) * oma >> 5) & 0x7E0F81F;
							*(aDestPixels _PLUSPLUS) = src + (dest | (dest >> 16));
						}
					}

					aRLAlphaData += rl;
					aSpanLeft -= rl;
				}

				aDestPixelsRow += mLockedSurfaceDesc.lPitch/2;
				aSrcPixelsRow += theImage->mWidth;
				aRLAlphaDataRow += theImage->mWidth;
			}
		}
		else if (aGMask == 0x3E0) // 5-5-5 optimizations
		{
			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				ushort* aDestPixels = aDestPixelsRow;
				uchar* aRLAlphaData = aRLAlphaDataRow;

				aSrcPixels = aSrcPixelsRow;

				for (int aSpanLeft = theSrcRect.mWidth; aSpanLeft > 0; )
				{
					uint32_t src = PEEK_SRC_COLOR;
					uchar rl = *aRLAlphaData;

					if (rl > aSpanLeft)
						rl = aSpanLeft;

					int oma = 256 - (src >> 24);

					if (oma == 1) // Fully opaque
					{
						for (int i = 0; i < rl; i++)
							*aDestPixels _PLUSPLUS = NEXT_SRC_COLOR;
					}
					else
					if (oma == 256) // Fully transparent
					{
						aDestPixels _PLUSEQUALS rl;
						aSrcPixels += rl;
					}
					else // Partially transparent
					{
						oma >>= 3;
						aSrcPixels++;
						uint32_t dest = (((*aDestPixels | (*aDestPixels << 16)) & 0x3E07C1F) * oma >> 5) & 0x3E07C1F;
						*(aDestPixels _PLUSPLUS) = src + (dest | (dest >> 16));

						for (int i = 1; i < rl; i++)
						{
							uint32_t src = NEXT_SRC_COLOR;
							int oma = (256 - (src >> 24)) >> 3;
							uint32_t dest = (((*aDestPixels | (*aDestPixels << 16)) & 0x3E07C1F) * oma >> 5) & 0x3E07C1F;
							*(aDestPixels _PLUSPLUS) = src + (dest | (dest >> 16));
						}
					}

					aRLAlphaData += rl;
					aSpanLeft -= rl;
				}

				aDestPixelsRow += mLockedSurfaceDesc.lPitch/2;
				aSrcPixelsRow += theImage->mWidth;
				aRLAlphaDataRow += theImage->mWidth;
			}
		}
		else
#endif
		{
			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				ushort* aDestPixels = aDestPixelsRow;
				uchar* aRLAlphaData = aRLAlphaDataRow;

				aSrcPixels = aSrcPixelsRow;

				for (int aSpanLeft = theSrcRect.mWidth; aSpanLeft > 0; )
				{
					uint32_t src = PEEK_SRC_COLOR;
					uchar rl = *aRLAlphaData;

					if (rl > aSpanLeft)
						rl = aSpanLeft;

					int oma = 256 - (src >> 24);

					if (oma == 1) // Fully opaque
					{
						for (int i = 0; i < rl; i++)
							*aDestPixels _PLUSPLUS = NEXT_SRC_COLOR;
					}
					else if (oma == 256) // Fully transparent
					{
						aDestPixels _PLUSEQUALS rl;
						aSrcPixels += rl;
					}
					else // Partially transparent
					{
						aSrcPixels++;
						uint32_t dest = *aDestPixels;
						*(aDestPixels _PLUSPLUS) = src +
							(((((dest & aRMask) * oma) ) >> 8) & aRMask) +
							(((((dest & aGMask) * oma) ) >> 8) & aGMask) +
							(((((dest & aBMask) * oma) ) >> 8) & aBMask);

						for (int i = 1; i < rl; i++)
						{
							uint32_t src = NEXT_SRC_COLOR;
							int oma = 256 - (src >> 24);

							uint32_t dest = *aDestPixels;
							*(aDestPixels _PLUSPLUS) = src +
								(((((dest & aRMask) * oma) ) >> 8) & aRMask) +
								(((((dest & aGMask) * oma) ) >> 8) & aGMask) +
								(((((dest & aBMask) * oma) ) >> 8) & aBMask);
						}
					}

					aRLAlphaData += rl;
					aSpanLeft -= rl;
				}

				aDestPixelsRow += mLockedSurfaceDesc.lPitch/2;
				aSrcPixelsRow += theImage->mWidth;
				aRLAlphaDataRow += theImage->mWidth;
			}
		}
	}
	else
	{
		int ca = theColor.mAlpha;
		int cr = (theColor.mRed * ca) / 255;
		int cg = (theColor.mGreen * ca) / 255;
		int cb = (theColor.mBlue * ca) / 255;

#ifdef OPTIMIZE_SOFTWARE_DRAWING
		bool performNormalBlit = true;
		if (cr == cg && cg == cb)
		{
			if (aGMask == 0x7E0)
			{
				performNormalBlit = false;
				cr >>= 3;

				for (int y = 0; y < theSrcRect.mHeight; y++)
				{
					ushort* aDestPixels = aDestPixelsRow;
					uchar* aRLAlphaData = aRLAlphaDataRow;

					aSrcPixels = aSrcPixelsRow;

					for (int aSpanLeft = theSrcRect.mWidth; aSpanLeft > 0; )
					{
						uint32_t src = PEEK_SRC_COLOR;
						uchar rl = *aRLAlphaData;

						if (rl > aSpanLeft)
							rl = aSpanLeft;

						int a = src >> 24;

						if ((a == 255) && (ca == 255)) // Fully opaque
						{
							for (int i = 0; i < rl; i++)
							{
								uint32_t src = NEXT_SRC_COLOR;
								uint32_t dest = (((( (src & 0xFFFF) | (src << 16)) & 0x7E0F81F) * cr) >> 5) & 0x7E0F81F;
								*(aDestPixels _PLUSPLUS) = dest | (dest >> 16);
							}
						}
						else if (a == 0) // Fully transparent
						{
							aDestPixels _PLUSEQUALS rl;
							aSrcPixels += rl;
						}
						else // Partially transparent
						{
							for (int i = 0; i < rl; i++)
							{
								uint32_t src = NEXT_SRC_COLOR;

								a = ((src >> 24) * ca) / 255;
								int oma = (256 - a) >> 3;

								uint32_t dest = (*aDestPixels | (*aDestPixels << 16)) & 0x7E0F81F;
								src &= 0xFFFF;
								src = ( (src & 0xFFFF) | (src << 16)) & 0x7E0F81F;
								dest = ((dest * oma + src * cr) >> 5) & 0x7E0F81F;
								*(aDestPixels _PLUSPLUS) = dest | (dest >> 16);
							}
						}

						aRLAlphaData += rl;
						aSpanLeft -= rl;
					}

					aDestPixelsRow += mLockedSurfaceDesc.lPitch/2;
					aSrcPixelsRow += theImage->mWidth;
					aRLAlphaDataRow += theImage->mWidth;
				}
			}
			else if (aGMask == 0x3E0)
			{
				performNormalBlit = false;
				cr >>= 3;

				for (int y = 0; y < theSrcRect.mHeight; y++)
				{
					ushort* aDestPixels = aDestPixelsRow;
					uchar* aRLAlphaData = aRLAlphaDataRow;

					aSrcPixels = aSrcPixelsRow;

					for (int aSpanLeft = theSrcRect.mWidth; aSpanLeft > 0; )
					{
						uint32_t src = PEEK_SRC_COLOR;
						uchar rl = *aRLAlphaData;

						if (rl > aSpanLeft)
							rl = aSpanLeft;

						int a = src >> 24;

						if ((a == 255) && (ca == 255)) // Fully opaque
						{
							for (int i = 0; i < rl; i++)
							{
								uint32_t src = NEXT_SRC_COLOR;
								uint32_t dest = (((( (src & 0xFFFF) | (src << 16)) & 0x3E07C1F) * cr) >> 5) & 0x3E07C1F;
								*(aDestPixels _PLUSPLUS) = dest | (dest >> 16);
							}
						}
						else if (a == 0) // Fully transparent
						{
							aDestPixels _PLUSEQUALS rl;
							aSrcPixels += rl;
						}
						else // Partially transparent
						{
							for (int i = 0; i < rl; i++)
							{
								uint32_t src = NEXT_SRC_COLOR;

								a = ((src >> 24) * ca) / 255;
								int oma = (256 - a) >> 3;

								uint32_t dest = (*aDestPixels | (*aDestPixels << 16)) & 0x3E07C1F;
								src &= 0xFFFF;
								src = ( (src & 0xFFFF) | (src << 16)) & 0x3E07C1F;
								dest = ((dest * oma + src * cr) >> 5) & 0x3E07C1F;
								*(aDestPixels _PLUSPLUS) = (dest | (dest >> 16));
							}
						}

						aRLAlphaData += rl;
						aSpanLeft -= rl;
					}

					aDestPixelsRow += mLockedSurfaceDesc.lPitch/2;
					aSrcPixelsRow += theImage->mWidth;
					aRLAlphaDataRow += theImage->mWidth;
				}
			}
		}
		if (performNormalBlit)
#endif
		{
			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				ushort* aDestPixels = aDestPixelsRow;
				uchar* aRLAlphaData = aRLAlphaDataRow;

				aSrcPixels = aSrcPixelsRow;

				for (int aSpanLeft = theSrcRect.mWidth; aSpanLeft > 0; )
				{
					uint32_t src = PEEK_SRC_COLOR;
					uchar rl = *aRLAlphaData;

					if (rl > aSpanLeft)
						rl = aSpanLeft;

					int a = src >> 24;

					if ((a == 255) && (ca == 255)) // Fully opaque
					{
						for (int i = 0; i < rl; i++)
						{
							uint32_t src = NEXT_SRC_COLOR;
							*(aDestPixels _PLUSPLUS) =
								(((((src & aRMask) * cr) ) >> 8) & aRMask) +
								(((((src & aGMask) * cg) ) >> 8) & aGMask) +
								(((((src & aBMask) * cb) ) >> 8) & aBMask);
						}
					}
					else if (a == 0) // Fully transparent
					{
						aDestPixels _PLUSEQUALS rl;
						aSrcPixels += rl;
					}
					else // Partially transparent
					{
						for (int i = 0; i < rl; i++)
						{
							uint32_t src = NEXT_SRC_COLOR;

							a = ((src >> 24) * ca) / 255;
							int oma = 256 - a;

							uint32_t dest = *aDestPixels;
							*(aDestPixels _PLUSPLUS) =
								(((((dest & aRMask) * oma) + ((src & aRMask) * cr) ) >> 8) & aRMask) +
								(((((dest & aGMask) * oma) + ((src & aGMask) * cg) ) >> 8) & aGMask) +
								(((((dest & aBMask) * oma) + ((src & aBMask) * cb) ) >> 8) & aBMask);
						}
					}

					aRLAlphaData += rl;
					aSpanLeft -= rl;
				}

				aDestPixelsRow += mLockedSurfaceDesc.lPitch/2;
				aSrcPixelsRow += theImage->mWidth;
				aRLAlphaDataRow += theImage->mWidth;
			}
		}
	}
}
else if (mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 32)
{
	uint32_t* aDestPixelsRow = ((uint32_t*) mLockedSurfaceDesc.lpSurface) + (theY * mLockedSurfaceDesc.lPitch/4) + theX;
	uchar* aRLAlphaDataRow = aSrcRLAlphaData + (theSrcRect.mY * theImage->mWidth) + theSrcRect.mX;

	uint32_t aRMask = mLockedSurfaceDesc.ddpfPixelFormat.dwRBitMask;
	uint32_t aGMask = mLockedSurfaceDesc.ddpfPixelFormat.dwGBitMask;
	uint32_t aBMask = mLockedSurfaceDesc.ddpfPixelFormat.dwBBitMask;

	// unused
	// uint32_t aRRoundAdd = aRMask >> 1;
	// uint32_t aGRoundAdd = aGMask >> 1;
	// uint32_t aBRoundAdd = aBMask >> 1;

	if (theColor == Color::White)
	{
		for (int y = 0; y < theSrcRect.mHeight; y++)
		{
			uint32_t* aDestPixels = aDestPixelsRow;
			uchar* aRLAlphaData = aRLAlphaDataRow;

			aSrcPixels = aSrcPixelsRow;

			for (int aSpanLeft = theSrcRect.mWidth; aSpanLeft > 0; )
			{
				uint32_t src = PEEK_SRC_COLOR;
				uchar rl = *aRLAlphaData;

				if (rl > aSpanLeft)
					rl = aSpanLeft;

				int oma = 256 - (src >> 24);

				if (oma == 1) // Fully opaque
				{
					for (int i = 0; i < rl; i++)
						*aDestPixels _PLUSPLUS = NEXT_SRC_COLOR;
				}
				else if (oma == 256) // Fully transparent
				{
					aDestPixels _PLUSEQUALS rl;
					aSrcPixels += rl;
				}
				else // Partially transparent
				{
#ifdef OPTIMIZE_SOFTWARE_DRAWING
					aSrcPixels++;

					int oma = 256 - (src >> 24);
					uint32_t dest = *aDestPixels;

					*(aDestPixels _PLUSPLUS) = src +
						((((dest & 0xFF00FF) * oma) >> 8) & 0xFF00FF) +
						((((dest & 0x00FF00) * oma) >> 8) & 0x00FF00);

					for (int i = 1; i < rl; i++)
					{
						uint32_t src = NEXT_SRC_COLOR;
						int oma = 256 - (src >> 24);
						uint32_t dest = *aDestPixels;

						*(aDestPixels _PLUSPLUS) = src +
							((((dest & 0xFF00FF) * oma) >> 8) & 0xFF00FF) +
							((((dest & 0x00FF00) * oma) >> 8) & 0x00FF00);
					}
#else
					aSrcPixels++;
					uint32_t dest = *aDestPixels;
					*(aDestPixels _PLUSPLUS) = src +
						(((((dest & aRMask) * oma)) >> 8) & aRMask) +
						(((((dest & aGMask) * oma)) >> 8) & aGMask) +
						(((((dest & aBMask) * oma)) >> 8) & aBMask);

					for (int i = 1; i < rl; i++)
					{
						uint32_t src = NEXT_SRC_COLOR;
						int oma = 256 - (src >> 24);

						uint32_t dest = *aDestPixels;
						*(aDestPixels _PLUSPLUS) = src +
							(((((dest & aRMask) * oma)) >> 8) & aRMask) +
							(((((dest & aGMask) * oma)) >> 8) & aGMask) +
							(((((dest & aBMask) * oma)) >> 8) & aBMask);
					}
#endif
				}

				aRLAlphaData += rl;
				aSpanLeft -= rl;
			}

			aDestPixelsRow += mLockedSurfaceDesc.lPitch/4;
			aSrcPixelsRow += theImage->mWidth;
			aRLAlphaDataRow += theImage->mWidth;
		}
	}
	else
	{
		int ca = theColor.mAlpha;
		int cr = (theColor.mRed * ca) / 255;
		int cg = (theColor.mGreen * ca) / 255;
		int cb = (theColor.mBlue * ca) / 255;

#ifdef OPTIMIZE_SOFTWARE_DRAWING
		bool performNormalBlit = true;
		if (cr == cg && cg == cb)
		{
			performNormalBlit = false;
			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				uint32_t* aDestPixels = aDestPixelsRow;
				uchar* aRLAlphaData = aRLAlphaDataRow;

				aSrcPixels = aSrcPixelsRow;

				for (int aSpanLeft = theSrcRect.mWidth; aSpanLeft > 0; )
				{
					uint32_t src = PEEK_SRC_COLOR;
					uchar rl = *aRLAlphaData;

					if (rl > aSpanLeft)
						rl = aSpanLeft;

					int a = src >> 24;

					if ((a == 255) && (ca == 255)) // Fully opaque
					{
						for (int i = 0; i < rl; i++)
						{
							uint32_t src = NEXT_SRC_COLOR;
							*(aDestPixels _PLUSPLUS) =
								((((src & 0xFF00FF) * cr) >> 8) & 0xFF00FF) +
								((((src & 0x00FF00) * cr) >> 8) & 0x00FF00);
						}
					}
					else if (a == 0) // Fully transparent
					{
						aDestPixels _PLUSEQUALS rl;
						aSrcPixels += rl;
					}
					else // Partially transparent
					{
						for (int i = 0; i < rl; i++)
						{
							uint32_t src = NEXT_SRC_COLOR;

							a = ((src >> 24) * ca) / 255;
							int oma = 256 - a;

							uint32_t dest = *aDestPixels;
							*(aDestPixels _PLUSPLUS) =
								((((dest & 0xFF00FF) * oma + (src & 0xFF00FF) * cr) >> 8) & 0xFF00FF) +
								((((dest & 0x00FF00) * oma + (src & 0x00FF00) * cr) >> 8) & 0x00FF00);
						}
					}

					aRLAlphaData += rl;
					aSpanLeft -= rl;
				}

				aDestPixelsRow += mLockedSurfaceDesc.lPitch/4;
				aSrcPixelsRow += theImage->mWidth;
				aRLAlphaDataRow += theImage->mWidth;
			}
		}
		if (performNormalBlit)
#endif
		{
			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				uint32_t* aDestPixels = aDestPixelsRow;
				uchar* aRLAlphaData = aRLAlphaDataRow;

				aSrcPixels = aSrcPixelsRow;

				for (int aSpanLeft = theSrcRect.mWidth; aSpanLeft > 0; )
				{
					uint32_t src = PEEK_SRC_COLOR;
					uchar rl = *aRLAlphaData;

					if (rl > aSpanLeft)
						rl = aSpanLeft;

					int a = src >> 24;

					if ((a == 255) && (ca == 255)) // Fully opaque
					{
						for (int i = 0; i < rl; i++)
						{
							uint32_t src = NEXT_SRC_COLOR;
							*(aDestPixels _PLUSPLUS) =
								(((((src & aRMask) * cr)) >> 8) & aRMask) +
								(((((src & aGMask) * cg)) >> 8) & aGMask) +
								(((((src & aBMask) * cb)) >> 8) & aBMask);
						}
					}
					else if (a == 0) // Fully transparent
					{
						aDestPixels _PLUSEQUALS rl;
						aSrcPixels += rl;
					}
					else // Partially transparent
					{
						for (int i = 0; i < rl; i++)
						{
							uint32_t src = NEXT_SRC_COLOR;

							a = ((src >> 24) * ca) / 255;
							int oma = 256 - a;

							uint32_t dest = *aDestPixels;
							*(aDestPixels _PLUSPLUS) =
								(((((dest & aRMask) * oma) + ((src & aRMask) * cr)) >> 8) & aRMask) +
								(((((dest & aGMask) * oma) + ((src & aGMask) * cg)) >> 8) & aGMask) +
								(((((dest & aBMask) * oma) + ((src & aBMask) * cb)) >> 8) & aBMask);
						}
					}

					aRLAlphaData += rl;
					aSpanLeft -= rl;
				}

				aDestPixelsRow += mLockedSurfaceDesc.lPitch/4;
				aSrcPixelsRow += theImage->mWidth;
				aRLAlphaDataRow += theImage->mWidth;
			}
		}
	}
}

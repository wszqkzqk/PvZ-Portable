{

	uchar* aWPtr = mRLAlphaData.get();

	if (mWidth==1)
	{
		memset(aWPtr,1,mHeight);
	}
	else
	{
		for (int aRow = 0; aRow < mHeight; aRow++)
		{
			int aRCount = 1;
			int aRLCount = 1;

			int anAVal = NEXT_SRC_COLOR >> 24;
			int aLastAClass = (anAVal == 0) ? 0 : (anAVal == 255) ? 1 : 2;

			while (aRCount < mWidth)
			{
				aRCount++;

				anAVal = NEXT_SRC_COLOR >> 24;
				int aThisAClass = (anAVal == 0) ? 0 : (anAVal == 255) ? 1 : 2;

				if ((aThisAClass != aLastAClass) || (aRCount == mWidth))
				{
					if (aThisAClass == aLastAClass)
						aRLCount++;

					for (int i = aRLCount; i > 0; i--)
					{
						if (i >= 255)
							*aWPtr++ = 255;
						else
							*aWPtr++ = i;
					}

					if ((aRCount == mWidth) && (aThisAClass != aLastAClass))
						*aWPtr++ = 1;

					aLastAClass = aThisAClass;
					aRLCount = 1;
				}
				else
				{
					aRLCount++;
				}
			}
		}
	}
}

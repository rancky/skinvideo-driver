		#define CFGLEN 64
		#define STRADDR 0x6000
		#define TABLELEN 24*1024

		typedef struct
		{
			unsigned short wFRInt;
			BYTE byCfgIdx;
		} FRData;	// Frame Rate Data

		typedef struct
		{
			unsigned short wWid;
			unsigned short wHei;
			unsigned char byNumFR;
			FRData aFRData[20];
		} ResData;	// Resolution Data

		unsigned short wStartAddr = 0;
		unsigned short wAddr = 0;
		unsigned short wCfgAddr = 0;
		unsigned short wHeaderLen = 0;
		unsigned short wTableLen = 0;
		unsigned char *pbyReadBuf;
		unsigned char byYUY2Idx = 0;
		unsigned char byNumRes = 0;
		unsigned char byNumFR = 0;
		unsigned char i = 0;
		unsigned char j = 0;
		ResData aResData[20];

		pbyReadBuf = (unsigned char *)ExAllocatePoolWithTag(NonPagedPool, TABLELEN, 'SF');
		memset(pbyReadBuf, 0xff, TABLELEN);

		// Read Section1 Address from STRADDR +++++
		//ReqToDriver(SF_READ, STRADDR, pbyReadBuf, 3);	// argument (Control, Address, Buffer, Length)
		// Read Section1 Address from STRADDR -----

		wStartAddr = (pbyReadBuf[1] << 8) | pbyReadBuf[2];
		
		// Read HeaderLen and TableLen from Section 1 +++++
		//ReqToDriver(SF_READ, wStartAddr, pbyReadBuf, 6);
		// Read HeaderLen and TableLen from Section 1 -----

		wHeaderLen = (pbyReadBuf[wAddr] << 8) | pbyReadBuf[wAddr + 1];
		wTableLen = (pbyReadBuf[wAddr + 4] << 8) | pbyReadBuf[wAddr + 5];

		// Read Table Data from STRADDR +++++
		//ReqToDriver(SF_READ, wStartAddr, pbyReadBuf, wTableLen);
		// Read Table Data from STRADDR -----

		wAddr += 7;
		byYUY2Idx = pbyReadBuf[wAddr];

		if (byYUY2Idx == 1)
			wAddr = (pbyReadBuf[wAddr + 7] << 8) | pbyReadBuf[wAddr + 8];
		else
			wAddr = (pbyReadBuf[wAddr + 9] << 8) | pbyReadBuf[wAddr + 10];;
			
		byNumRes = pbyReadBuf[wAddr];
		wAddr++;

		for (i = 0; i < byNumRes; i++)
		{
			byNumFR = pbyReadBuf[wAddr];
			wAddr++;
			
			for (j = 0; j < byNumFR; j++)
			{
				aResData[i].aFRData[j].wFRInt = (pbyReadBuf[wAddr]<<8) | (pbyReadBuf[wAddr+1]);
				wAddr += 2;
				
				aResData[i].aFRData[j].byCfgIdx = pbyReadBuf[wAddr];
				wAddr++;

				//TRACE("Res %d : FR Int = %x", i, aResData[i].aFRData[j].wFRInt);
			}
			
			wCfgAddr = wHeaderLen + (CFGLEN * (aResData[i].aFRData[0].byCfgIdx - 1));
			aResData[i].wWid = (pbyReadBuf[wCfgAddr+4]<<8) | (pbyReadBuf[wCfgAddr+5]);
			aResData[i].wHei = (pbyReadBuf[wCfgAddr+6]<<8) | (pbyReadBuf[wCfgAddr+7]);

			//TRACE("Res %d : Wid = %d, Hei = %d", i , aResData[i].wWid, aResData[i].wHei);
		}

		if(pbyReadBuf)
		{ 
			ExFreePoolWithTag(pbyReadBuf, 'FREE');
			pbyReadBuf = NULL;
		}

/*************************************************************************/ /*!
@Title          SIRFSOC common driver functions
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Dual MIT/GPLv2

The contents of this file are subject to the MIT license as set out below.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

Alternatively, the contents of this file may be used under the terms of
the GNU General Public License Version 2 ("GPL") in which case the provisions
of GPL are applicable instead of those above.

If you wish to allow use of your version of this file only under the terms of
GPL, and not to allow others to use your version of this file under the terms
of the MIT license, indicate your decision by deleting the provisions above
and replace them with the notice and other provisions required by GPL as set
out in the file called "GPL-COPYING" included in this distribution. If you do
not delete the provisions above, a recipient may use your version of this file
under the terms of either the MIT license or GPL.

This License is also included in this distribution in the file called
"MIT-COPYING".

EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/ /**************************************************************************/
/************************************************************************
* Imagination Technologies makes these files available under MIT or GPLv2.
* Portions of the file may also include code with the following notice
* Copyright (c) 2016, The Linux Foundation. All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* * Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of The Linux Foundation nor the names of its
* contributors may be used to endorse or promote products derived from
* this software without specific prior written permission.
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIEDWARRANTIES,
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OFMERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OFSUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; ORBUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
************************************************************************/
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/console.h>
#include <linux/fb.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/notifier.h>
#include "img_defs.h"
#include "servicesext.h"
#include "kerneldisplay.h"
#include "sirfsocfb.h"

#if defined(CONFIG_VDSSCOMP)
#include <video/vdsscomp.h>
#endif

#define SIRFSOCFB_COMMAND_COUNT		1

#define	SIRFSOCFB_VSYNC_SETTLE_COUNT	5

#define	SIRFSOCFB_MAX_NUM_DEVICES		FB_MAX
#if (SIRFSOCFB_MAX_NUM_DEVICES > FB_MAX)
#error "SIRFSOCFB_MAX_NUM_DEVICES must not be greater than FB_MAX"
#endif

static SIRFSOCFB_DEVINFO *gapsDevInfo[SIRFSOCFB_MAX_NUM_DEVICES];

static PFN_DC_GET_PVRJTABLE gpfnGetPVRJTable = NULL;

static inline unsigned long RoundUpToMultiple(unsigned long x, unsigned long y)
{
	unsigned long div = x / y;
	unsigned long rem = x % y;

	return (div + ((rem == 0) ? 0 : 1)) * y;
}

static unsigned long GCD(unsigned long x, unsigned long y)
{
	while (y != 0)
	{
		unsigned long r = x % y;
		x = y;
		y = r;
	}

	return x;
}

static unsigned long LCM(unsigned long x, unsigned long y)
{
	unsigned long gcd = GCD(x, y);

	return (gcd == 0) ? 0 : ((x / gcd) * y);
}

unsigned SIRFSOCFBMaxFBDevIDPlusOne(void)
{
	return SIRFSOCFB_MAX_NUM_DEVICES;
}

SIRFSOCFB_DEVINFO *SIRFSOCFBGetDevInfoPtr(unsigned uiFBDevID)
{
	WARN_ON(uiFBDevID >= SIRFSOCFBMaxFBDevIDPlusOne());

	if (uiFBDevID >= SIRFSOCFB_MAX_NUM_DEVICES)
	{
		return NULL;
	}

	return gapsDevInfo[uiFBDevID];
}

static inline void SIRFSOCFBSetDevInfoPtr(unsigned uiFBDevID, SIRFSOCFB_DEVINFO *psDevInfo)
{
	WARN_ON(uiFBDevID >= SIRFSOCFB_MAX_NUM_DEVICES);

	if (uiFBDevID < SIRFSOCFB_MAX_NUM_DEVICES)
	{
		gapsDevInfo[uiFBDevID] = psDevInfo;
	}
}

static inline SIRFSOCFB_BOOL SwapChainHasChanged(SIRFSOCFB_DEVINFO *psDevInfo, SIRFSOCFB_SWAPCHAIN *psSwapChain)
{
	return (psDevInfo->psSwapChain != psSwapChain) ||
		(psDevInfo->uiSwapChainID != psSwapChain->uiSwapChainID);
}

static inline SIRFSOCFB_BOOL DontWaitForVSync(SIRFSOCFB_DEVINFO *psDevInfo)
{
	SIRFSOCFB_BOOL bDontWait;

	bDontWait = SIRFSOCFBAtomicBoolRead(&psDevInfo->sBlanked) ||
			SIRFSOCFBAtomicBoolRead(&psDevInfo->sFlushCommands);

#if defined(CONFIG_HAS_EARLYSUSPEND)
	bDontWait = bDontWait || SIRFSOCFBAtomicBoolRead(&psDevInfo->sEarlySuspendFlag);
#endif
#if defined(SUPPORT_DRI_DRM)
	bDontWait = bDontWait || SIRFSOCFBAtomicBoolRead(&psDevInfo->sLeaveVT);
#endif
	return bDontWait;
}

static IMG_VOID SetDCState(IMG_HANDLE hDevice, IMG_UINT32 ui32State)
{
	SIRFSOCFB_DEVINFO *psDevInfo = (SIRFSOCFB_DEVINFO *)hDevice;

	switch (ui32State)
	{
		case DC_STATE_FLUSH_COMMANDS:
			SIRFSOCFBAtomicBoolSet(&psDevInfo->sFlushCommands, SIRFSOCFB_TRUE);
			break;
		case DC_STATE_NO_FLUSH_COMMANDS:
			SIRFSOCFBAtomicBoolSet(&psDevInfo->sFlushCommands, SIRFSOCFB_FALSE);
			break;
		default:
			break;
	}
}

static PVRSRV_ERROR OpenDCDevice(IMG_UINT32 uiPVRDevID,
                                 IMG_HANDLE *phDevice,
                                 PVRSRV_SYNC_DATA* psSystemBufferSyncData)
{
	SIRFSOCFB_DEVINFO *psDevInfo;
	SIRFSOCFB_ERROR eError;
	unsigned uiMaxFBDevIDPlusOne = SIRFSOCFBMaxFBDevIDPlusOne();
	unsigned i;

	for (i = 0; i < uiMaxFBDevIDPlusOne; i++)
	{
		psDevInfo = SIRFSOCFBGetDevInfoPtr(i);
		if (psDevInfo != NULL && psDevInfo->uiPVRDevID == uiPVRDevID)
		{
			break;
		}
	}
	if (i == uiMaxFBDevIDPlusOne)
	{
		DEBUG_PRINTK((KERN_WARNING DRIVER_PREFIX
			": %s: PVR Device %u not found\n", __FUNCTION__, uiPVRDevID));
		return PVRSRV_ERROR_INVALID_DEVICE;
	}


	psDevInfo->sSystemBuffer.psSyncData = psSystemBufferSyncData;

	eError = SIRFSOCFBUnblankDisplay(psDevInfo);
	if (eError != SIRFSOCFB_OK)
	{
		DEBUG_PRINTK((KERN_WARNING DRIVER_PREFIX
			": %s: Device %u: SIRFSOCFBUnblankDisplay failed (%d)\n", __FUNCTION__, psDevInfo->uiFBDevID, eError));
		return PVRSRV_ERROR_UNBLANK_DISPLAY_FAILED;
	}


	*phDevice = (IMG_HANDLE)psDevInfo;

	return PVRSRV_OK;
}

static PVRSRV_ERROR CloseDCDevice(IMG_HANDLE hDevice)
{
#if defined(SUPPORT_DRI_DRM)
	SIRFSOCFB_DEVINFO *psDevInfo = (SIRFSOCFB_DEVINFO *)hDevice;

	SIRFSOCFBAtomicBoolSet(&psDevInfo->sLeaveVT, SIRFSOCFB_FALSE);
	(void) SIRFSOCFBUnblankDisplay(psDevInfo);
#else
	UNREFERENCED_PARAMETER(hDevice);
#endif
	return PVRSRV_OK;
}

static PVRSRV_ERROR EnumDCFormats(IMG_HANDLE hDevice,
                                  IMG_UINT32 *pui32NumFormats,
                                  DISPLAY_FORMAT *psFormat)
{
	SIRFSOCFB_DEVINFO	*psDevInfo;

	if(!hDevice || !pui32NumFormats)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}

	psDevInfo = (SIRFSOCFB_DEVINFO*)hDevice;

	*pui32NumFormats = 1;

	if(psFormat)
	{
		psFormat[0] = psDevInfo->sDisplayFormat;
	}

	return PVRSRV_OK;
}

static PVRSRV_ERROR EnumDCDims(IMG_HANDLE hDevice,
                               DISPLAY_FORMAT *psFormat,
                               IMG_UINT32 *pui32NumDims,
                               DISPLAY_DIMS *psDim)
{
	SIRFSOCFB_DEVINFO	*psDevInfo;

	if(!hDevice || !psFormat || !pui32NumDims)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}

	psDevInfo = (SIRFSOCFB_DEVINFO*)hDevice;

	*pui32NumDims = 1;


	if(psDim)
	{
		psDim[0] = psDevInfo->sDisplayDim;
	}

	return PVRSRV_OK;
}


static PVRSRV_ERROR GetDCSystemBuffer(IMG_HANDLE hDevice, IMG_HANDLE *phBuffer)
{
	SIRFSOCFB_DEVINFO	*psDevInfo;

	if(!hDevice || !phBuffer)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}

	psDevInfo = (SIRFSOCFB_DEVINFO*)hDevice;

	*phBuffer = (IMG_HANDLE)&psDevInfo->sSystemBuffer;

	return PVRSRV_OK;
}


static PVRSRV_ERROR GetDCInfo(IMG_HANDLE hDevice, DISPLAY_INFO *psDCInfo)
{
	SIRFSOCFB_DEVINFO	*psDevInfo;

	if(!hDevice || !psDCInfo)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}

	psDevInfo = (SIRFSOCFB_DEVINFO*)hDevice;

	*psDCInfo = psDevInfo->sDisplayInfo;

	return PVRSRV_OK;
}

static PVRSRV_ERROR GetDCBufferAddr(IMG_HANDLE        hDevice,
                                    IMG_HANDLE        hBuffer,
                                    IMG_SYS_PHYADDR   **ppsSysAddr,
                                    IMG_UINT32        *pui32ByteSize,
                                    IMG_VOID          **ppvCpuVAddr,
                                    IMG_HANDLE        *phOSMapInfo,
                                    IMG_BOOL          *pbIsContiguous,
	                                IMG_UINT32		  *pui32TilingStride)
{
	SIRFSOCFB_DEVINFO	*psDevInfo;
	SIRFSOCFB_BUFFER *psSystemBuffer;

	UNREFERENCED_PARAMETER(pui32TilingStride);

	if(!hDevice)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}

	if(!hBuffer)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}

	if (!ppsSysAddr)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}

	if (!pui32ByteSize)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}

	psDevInfo = (SIRFSOCFB_DEVINFO*)hDevice;

	psSystemBuffer = (SIRFSOCFB_BUFFER *)hBuffer;

	*ppsSysAddr = &psSystemBuffer->sSysAddr;

	*pui32ByteSize = (IMG_UINT32)psDevInfo->sFBInfo.ulBufferSize;

	if (ppvCpuVAddr)
	{
		*ppvCpuVAddr = psSystemBuffer->sCPUVAddr;
	}

	if (phOSMapInfo)
	{
		*phOSMapInfo = (IMG_HANDLE)0;
	}

	if (pbIsContiguous)
	{
		*pbIsContiguous = IMG_TRUE;
	}

	return PVRSRV_OK;
}

static PVRSRV_ERROR CreateDCSwapChain(IMG_HANDLE hDevice,
                                      IMG_UINT32 ui32Flags,
                                      DISPLAY_SURF_ATTRIBUTES *psDstSurfAttrib,
                                      DISPLAY_SURF_ATTRIBUTES *psSrcSurfAttrib,
                                      IMG_UINT32 ui32BufferCount,
                                      PVRSRV_SYNC_DATA **ppsSyncData,
                                      IMG_UINT32 ui32OEMFlags,
                                      IMG_HANDLE *phSwapChain,
                                      IMG_UINT32 *pui32SwapChainID)
{
	SIRFSOCFB_DEVINFO	*psDevInfo;
	SIRFSOCFB_SWAPCHAIN *psSwapChain;
	SIRFSOCFB_BUFFER *psBuffer;
	IMG_UINT32 i;
	PVRSRV_ERROR eError;
	IMG_UINT32 ui32BuffersToSkip;

	UNREFERENCED_PARAMETER(ui32OEMFlags);


	if(!hDevice
	|| !psDstSurfAttrib
	|| !psSrcSurfAttrib
	|| !ppsSyncData
	|| !phSwapChain)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}

	psDevInfo = (SIRFSOCFB_DEVINFO*)hDevice;


	if (psDevInfo->sDisplayInfo.ui32MaxSwapChains == 0)
	{
		return PVRSRV_ERROR_NOT_SUPPORTED;
	}

	SIRFSOCFBCreateSwapChainLock(psDevInfo);


	if(psDevInfo->psSwapChain != NULL)
	{
		eError = PVRSRV_ERROR_FLIP_CHAIN_EXISTS;
		goto ExitUnLock;
	}


	if(ui32BufferCount > psDevInfo->sDisplayInfo.ui32MaxSwapChainBuffers)
	{
		eError = PVRSRV_ERROR_TOOMANYBUFFERS;
		goto ExitUnLock;
	}

	if ((psDevInfo->sFBInfo.ulRoundedBufferSize * (unsigned long)ui32BufferCount) > psDevInfo->sFBInfo.ulFBSize)
	{
		eError = PVRSRV_ERROR_TOOMANYBUFFERS;
		goto ExitUnLock;
	}


	ui32BuffersToSkip = psDevInfo->sDisplayInfo.ui32MaxSwapChainBuffers - ui32BufferCount;

    DEBUG_PRINTK((KERN_INFO DRIVER_PREFIX
			"Buffers to skip %u\n",
            ui32BuffersToSkip));


	if(psDstSurfAttrib->pixelformat != psDevInfo->sDisplayFormat.pixelformat
	|| psDstSurfAttrib->sDims.ui32ByteStride != psDevInfo->sDisplayDim.ui32ByteStride
	|| psDstSurfAttrib->sDims.ui32Width != psDevInfo->sDisplayDim.ui32Width
	|| psDstSurfAttrib->sDims.ui32Height != psDevInfo->sDisplayDim.ui32Height)
	{

		eError = PVRSRV_ERROR_INVALID_PARAMS;
		goto ExitUnLock;
	}

	if(psDstSurfAttrib->pixelformat != psSrcSurfAttrib->pixelformat
	|| psDstSurfAttrib->sDims.ui32ByteStride != psSrcSurfAttrib->sDims.ui32ByteStride
	|| psDstSurfAttrib->sDims.ui32Width != psSrcSurfAttrib->sDims.ui32Width
	|| psDstSurfAttrib->sDims.ui32Height != psSrcSurfAttrib->sDims.ui32Height)
	{

		eError = PVRSRV_ERROR_INVALID_PARAMS;
		goto ExitUnLock;
	}


	UNREFERENCED_PARAMETER(ui32Flags);

	psSwapChain = (SIRFSOCFB_SWAPCHAIN*)SIRFSOCFBAllocKernelMem(sizeof(SIRFSOCFB_SWAPCHAIN));
	if(!psSwapChain)
	{
		eError = PVRSRV_ERROR_OUT_OF_MEMORY;
		goto ExitUnLock;
	}

	psBuffer = (SIRFSOCFB_BUFFER*)SIRFSOCFBAllocKernelMem(sizeof(SIRFSOCFB_BUFFER) * ui32BufferCount);
	if(!psBuffer)
	{
		eError = PVRSRV_ERROR_OUT_OF_MEMORY;
		goto ErrorFreeSwapChain;
	}

	psSwapChain->ulBufferCount = (unsigned long)ui32BufferCount;
	psSwapChain->psBuffer = psBuffer;
	psSwapChain->bNotVSynced = SIRFSOCFB_TRUE;
	psSwapChain->uiFBDevID = psDevInfo->uiFBDevID;


	for(i=0; i<ui32BufferCount-1; i++)
	{
		psBuffer[i].psNext = &psBuffer[i+1];
	}

	psBuffer[i].psNext = &psBuffer[0];


	for(i=0; i<ui32BufferCount; i++)
	{
		IMG_UINT32 ui32SwapBuffer = i + ui32BuffersToSkip;
		IMG_UINT32 ui32BufferOffset = ui32SwapBuffer * (IMG_UINT32)psDevInfo->sFBInfo.ulRoundedBufferSize;

		psBuffer[i].psSyncData = ppsSyncData[i];

		psBuffer[i].sSysAddr.uiAddr = psDevInfo->sFBInfo.sSysAddr.uiAddr + ui32BufferOffset;
		psBuffer[i].sCPUVAddr = psDevInfo->sFBInfo.sCPUVAddr + ui32BufferOffset;
		psBuffer[i].ulYOffset = ui32BufferOffset / psDevInfo->sFBInfo.ulByteStride;
		psBuffer[i].psDevInfo = psDevInfo;

		SIRFSOCFBInitBufferForSwap(&psBuffer[i]);

        DEBUG_PRINTK((KERN_INFO DRIVER_PREFIX
			"Buffer %u has ulYOffset %lu\n",
            i, psBuffer[i].ulYOffset));

	}

	if (SIRFSOCFBCreateSwapQueue(psSwapChain) != SIRFSOCFB_OK)
	{
		printk(KERN_WARNING DRIVER_PREFIX ": %s: Device %u: Failed to create workqueue\n", __FUNCTION__, psDevInfo->uiFBDevID);
		eError = PVRSRV_ERROR_UNABLE_TO_INSTALL_ISR;
		goto ErrorFreeBuffers;
	}

	if (SIRFSOCFBEnableLFBEventNotification(psDevInfo)!= SIRFSOCFB_OK)
	{
		eError = PVRSRV_ERROR_UNABLE_TO_ENABLE_EVENT;
		printk(KERN_WARNING DRIVER_PREFIX ": %s: Device %u: Couldn't enable framebuffer event notification\n", __FUNCTION__, psDevInfo->uiFBDevID);
		goto ErrorDestroySwapQueue;
	}

	psDevInfo->uiSwapChainID++;
	if (psDevInfo->uiSwapChainID == 0)
	{
		psDevInfo->uiSwapChainID++;
	}

	psSwapChain->uiSwapChainID = psDevInfo->uiSwapChainID;

	psDevInfo->psSwapChain = psSwapChain;

	*pui32SwapChainID = psDevInfo->uiSwapChainID;

	*phSwapChain = (IMG_HANDLE)psSwapChain;

	eError = PVRSRV_OK;
	goto ExitUnLock;

ErrorDestroySwapQueue:
	SIRFSOCFBDestroySwapQueue(psSwapChain);
ErrorFreeBuffers:
	SIRFSOCFBFreeKernelMem(psBuffer);
ErrorFreeSwapChain:
	SIRFSOCFBFreeKernelMem(psSwapChain);
ExitUnLock:
	SIRFSOCFBCreateSwapChainUnLock(psDevInfo);
	return eError;
}

static PVRSRV_ERROR DestroyDCSwapChain(IMG_HANDLE hDevice,
	IMG_HANDLE hSwapChain)
{
	SIRFSOCFB_DEVINFO	*psDevInfo;
	SIRFSOCFB_SWAPCHAIN *psSwapChain;
	SIRFSOCFB_ERROR eError;


	if(!hDevice || !hSwapChain)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}

	psDevInfo = (SIRFSOCFB_DEVINFO*)hDevice;
	psSwapChain = (SIRFSOCFB_SWAPCHAIN*)hSwapChain;

	SIRFSOCFBCreateSwapChainLock(psDevInfo);

	if (SwapChainHasChanged(psDevInfo, psSwapChain))
	{
		printk(KERN_WARNING DRIVER_PREFIX
			": %s: Device %u: Swap chain mismatch\n", __FUNCTION__, psDevInfo->uiFBDevID);

		eError = PVRSRV_ERROR_INVALID_PARAMS;
		goto ExitUnLock;
	}


	SIRFSOCFBDestroySwapQueue(psSwapChain);

	eError = SIRFSOCFBDisableLFBEventNotification(psDevInfo);
	if (eError != SIRFSOCFB_OK)
	{
		printk(KERN_WARNING DRIVER_PREFIX ": %s: Device %u: Couldn't disable framebuffer event notification\n", __FUNCTION__, psDevInfo->uiFBDevID);
	}


	SIRFSOCFBFreeKernelMem(psSwapChain->psBuffer);
	SIRFSOCFBFreeKernelMem(psSwapChain);

	psDevInfo->psSwapChain = NULL;

	SIRFSOCFBFlip(psDevInfo, &psDevInfo->sSystemBuffer);
	(void) SIRFSOCFBCheckModeAndSync(psDevInfo);

	eError = PVRSRV_OK;

ExitUnLock:
	SIRFSOCFBCreateSwapChainUnLock(psDevInfo);

	return eError;
}

static PVRSRV_ERROR SetDCDstRect(IMG_HANDLE hDevice,
	IMG_HANDLE hSwapChain,
	IMG_RECT *psRect)
{
	UNREFERENCED_PARAMETER(hDevice);
	UNREFERENCED_PARAMETER(hSwapChain);
	UNREFERENCED_PARAMETER(psRect);



	return PVRSRV_ERROR_NOT_SUPPORTED;
}

static PVRSRV_ERROR SetDCSrcRect(IMG_HANDLE hDevice,
                                 IMG_HANDLE hSwapChain,
                                 IMG_RECT *psRect)
{
	UNREFERENCED_PARAMETER(hDevice);
	UNREFERENCED_PARAMETER(hSwapChain);
	UNREFERENCED_PARAMETER(psRect);



	return PVRSRV_ERROR_NOT_SUPPORTED;
}

static PVRSRV_ERROR SetDCDstColourKey(IMG_HANDLE hDevice,
                                      IMG_HANDLE hSwapChain,
                                      IMG_UINT32 ui32CKColour)
{
	UNREFERENCED_PARAMETER(hDevice);
	UNREFERENCED_PARAMETER(hSwapChain);
	UNREFERENCED_PARAMETER(ui32CKColour);



	return PVRSRV_ERROR_NOT_SUPPORTED;
}

static PVRSRV_ERROR SetDCSrcColourKey(IMG_HANDLE hDevice,
                                      IMG_HANDLE hSwapChain,
                                      IMG_UINT32 ui32CKColour)
{
	UNREFERENCED_PARAMETER(hDevice);
	UNREFERENCED_PARAMETER(hSwapChain);
	UNREFERENCED_PARAMETER(ui32CKColour);



	return PVRSRV_ERROR_NOT_SUPPORTED;
}

static PVRSRV_ERROR GetDCBuffers(IMG_HANDLE hDevice,
                                 IMG_HANDLE hSwapChain,
                                 IMG_UINT32 *pui32BufferCount,
                                 IMG_HANDLE *phBuffer)
{
	SIRFSOCFB_DEVINFO   *psDevInfo;
	SIRFSOCFB_SWAPCHAIN *psSwapChain;
	PVRSRV_ERROR eError;
	unsigned i;


	if(!hDevice
	|| !hSwapChain
	|| !pui32BufferCount
	|| !phBuffer)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}

	psDevInfo = (SIRFSOCFB_DEVINFO*)hDevice;
	psSwapChain = (SIRFSOCFB_SWAPCHAIN*)hSwapChain;

	SIRFSOCFBCreateSwapChainLock(psDevInfo);

	if (SwapChainHasChanged(psDevInfo, psSwapChain))
	{
		printk(KERN_WARNING DRIVER_PREFIX
			": %s: Device %u: Swap chain mismatch\n", __FUNCTION__, psDevInfo->uiFBDevID);

		eError = PVRSRV_ERROR_INVALID_PARAMS;
		goto Exit;
	}


	*pui32BufferCount = (IMG_UINT32)psSwapChain->ulBufferCount;


	for(i=0; i<psSwapChain->ulBufferCount; i++)
	{
		phBuffer[i] = (IMG_HANDLE)&psSwapChain->psBuffer[i];
	}

	eError = PVRSRV_OK;

Exit:
	SIRFSOCFBCreateSwapChainUnLock(psDevInfo);

	return eError;
}

static PVRSRV_ERROR SwapToDCBuffer(IMG_HANDLE hDevice,
                                   IMG_HANDLE hBuffer,
                                   IMG_UINT32 ui32SwapInterval,
                                   IMG_HANDLE hPrivateTag,
                                   IMG_UINT32 ui32ClipRectCount,
                                   IMG_RECT *psClipRect)
{
	UNREFERENCED_PARAMETER(hDevice);
	UNREFERENCED_PARAMETER(hBuffer);
	UNREFERENCED_PARAMETER(ui32SwapInterval);
	UNREFERENCED_PARAMETER(hPrivateTag);
	UNREFERENCED_PARAMETER(ui32ClipRectCount);
	UNREFERENCED_PARAMETER(psClipRect);



	return PVRSRV_OK;
}

static SIRFSOCFB_BOOL WaitForVSyncSettle(SIRFSOCFB_DEVINFO *psDevInfo)
{
		unsigned i;
		for(i = 0; i < SIRFSOCFB_VSYNC_SETTLE_COUNT; i++)
		{
			if (DontWaitForVSync(psDevInfo) || !SIRFSOCFBWaitForVSync(psDevInfo))
			{
				return SIRFSOCFB_FALSE;
			}
		}

		return SIRFSOCFB_TRUE;
}

void SIRFSOCFBSwapHandler(SIRFSOCFB_BUFFER *psBuffer)
{
	SIRFSOCFB_DEVINFO *psDevInfo = psBuffer->psDevInfo;
	SIRFSOCFB_SWAPCHAIN *psSwapChain = psDevInfo->psSwapChain;
	SIRFSOCFB_BOOL bPreviouslyNotVSynced;

#if defined(SUPPORT_DRI_DRM)
	if (!SIRFSOCFBAtomicBoolRead(&psDevInfo->sLeaveVT))
#endif
	{
		SIRFSOCFBFlip(psDevInfo, psBuffer);
#if defined(CONFIG_VDSSCOMP)
		vdsscomp_gralloc_queue(NULL,NULL,NULL);
#endif
	}

	bPreviouslyNotVSynced = psSwapChain->bNotVSynced;
	psSwapChain->bNotVSynced = SIRFSOCFB_TRUE;


	if (!DontWaitForVSync(psDevInfo))
	{
		SIRFSOCFB_UPDATE_MODE eMode = SIRFSOCFBGetUpdateMode(psDevInfo);
		int iBlankEvents = SIRFSOCFBAtomicIntRead(&psDevInfo->sBlankEvents);

		switch(eMode)
		{
			case SIRFSOCFB_UPDATE_MODE_AUTO:
				psSwapChain->bNotVSynced = SIRFSOCFB_FALSE;

				if (bPreviouslyNotVSynced || psSwapChain->iBlankEvents != iBlankEvents)
				{
					psSwapChain->iBlankEvents = iBlankEvents;
					psSwapChain->bNotVSynced = !WaitForVSyncSettle(psDevInfo);
				} else if (psBuffer->ulSwapInterval != 0)
				{
					psSwapChain->bNotVSynced = !SIRFSOCFBWaitForVSync(psDevInfo);
				}
				break;
#if defined(PVR_OMAPFB3_MANUAL_UPDATE_SYNC_IN_SWAP)
			case SIRFSOCFB_UPDATE_MODE_MANUAL:
				if (psBuffer->ulSwapInterval != 0)
				{
					(void) SIRFSOCFBManualSync(psDevInfo);
				}
				break;
#endif
			default:
				break;
		}
	}

	psDevInfo->sPVRJTable.pfnPVRSRVCmdComplete((IMG_HANDLE)psBuffer->hCmdComplete, IMG_TRUE);
}

static IMG_BOOL ProcessFlipV1(IMG_HANDLE hCmdCookie,
							SIRFSOCFB_DEVINFO *psDevInfo,
							SIRFSOCFB_SWAPCHAIN *psSwapChain,
							SIRFSOCFB_BUFFER *psBuffer,
							unsigned long ulSwapInterval)
{

	SIRFSOCFBCreateSwapChainLock(psDevInfo);

	if (SwapChainHasChanged(psDevInfo, psSwapChain))
	{
		DEBUG_PRINTK((KERN_WARNING DRIVER_PREFIX
			": %s: Device %u (PVR Device ID %u): The swap chain has been destroyed\n",
			__FUNCTION__, psDevInfo->uiFBDevID, psDevInfo->uiPVRDevID));
	}
	else
	{
		psBuffer->hCmdComplete = (SIRFSOCFB_HANDLE)hCmdCookie;
		psBuffer->ulSwapInterval = (unsigned long)ulSwapInterval;
		SIRFSOCFBQueueBufferForSwap(psSwapChain, psBuffer);
	}

	SIRFSOCFBCreateSwapChainUnLock(psDevInfo);

	return IMG_TRUE;
}

#if defined(CONFIG_VDSSCOMP)
static IMG_BOOL ProcessFlipV2(IMG_HANDLE hCmdCookie,
	SIRFSOCFB_DEVINFO *psDevInfo,
	PDC_MEM_INFO *ppsMemInfos,
	IMG_UINT32 ui32NumMemInfos,
	struct vdsscomp_setup_data *psDispData,
	IMG_UINT32 ui32DispDataLength)
{
	int i, layer_num;

	if (!psDispData)
	{
		printk(KERN_WARNING DRIVER_PREFIX
			": %s: Device %u: WARNING: psDispcData was NULL. "
			"The HWC probably has a bug. Silently ignoring.",
			__FUNCTION__, psDevInfo->uiFBDevID);
		gapsDevInfo[0]->sPVRJTable.pfnPVRSRVCmdComplete(hCmdCookie, IMG_TRUE);
		return IMG_TRUE;
	}

	if(ui32DispDataLength != sizeof(*psDispData))
	{
		printk(KERN_WARNING DRIVER_PREFIX
			   ": %s: Device %u: WARNING: Unexpected private data size, %u vs %u.",
			   __FUNCTION__, psDevInfo->uiFBDevID, ui32DispDataLength,
			   sizeof(*psDispData));
		return IMG_FALSE;
	}

	if (DontWaitForVSync(psDevInfo))
	{
		gapsDevInfo[0]->sPVRJTable.pfnPVRSRVCmdComplete(hCmdCookie, IMG_TRUE);
		return IMG_TRUE;
	}

        if (ui32NumMemInfos == 0 || psDispData->num_disps == 0 ||
		psDispData->disps[0].num_layers == 0)
        {
		printk(KERN_WARNING DRIVER_PREFIX
			": %s: Device %u: WARNING: must have at least one layer. ",
			__FUNCTION__, psDevInfo->uiFBDevID);
                return IMG_FALSE;
        }
#if 0
	printk(KERN_WARNING DRIVER_PREFIX
		"%s: INFO: disp_num %d, layer_num %d, meminfo_num %d.\n",
		__FUNCTION__, psDispData->num_disps, psDispData->disps[0].num_layers,
		ui32NumMemInfos);

	for (i = 0; i < psDispData->disps[0].num_layers; i++)
	{
		struct vdsscomp_layer_info *info = &psDispData->disps[0].layers[i];
		printk(KERN_WARNING DRIVER_PREFIX
			"layer %d: enabled %d, fmt %d, width %d, height %d\n",
			i, info->enabled, info->fmt, info->width, info->height);
		printk(KERN_WARNING DRIVER_PREFIX
			"src_rect(%d, %d, %d, %d), dst_rect(%d, %d, %d, %d)\n",
			info->src_rect.left, info->src_rect.right,
			info->src_rect.top, info->src_rect.bottom,
			info->dst_rect.left, info->dst_rect.right,
			info->dst_rect.top, info->dst_rect.bottom);
	}
#endif

	layer_num = psDispData->disps[0].num_layers + ((psDispData->num_disps > 1) ?
		psDispData->disps[1].num_layers : 0);

	if (layer_num != ui32NumMemInfos)
	{
		printk(KERN_WARNING DRIVER_PREFIX
			": %s: Device %u: WARNING: layer number %d doesn't match with meminfo num %d. ",
			__FUNCTION__, psDevInfo->uiFBDevID,
			layer_num, ui32NumMemInfos);
                return IMG_FALSE;
	}

	/* Maximum of 8 layer_infos. Meminfo array is dynamically sized */
	for (i = 0; i < ui32NumMemInfos; i++)
	{
		IMG_CPU_PHYADDR phyAddr;
		int index;

		if (i < psDispData->disps[0].num_layers)
		{
			index = psDispData->disps[0].phys_addr[i];
			psDevInfo->sPVRJTable.pfnPVRSRVDCMemInfoGetCpuPAddr(ppsMemInfos[index],
				0, &phyAddr);
			psDispData->disps[0].phys_addr[i] = phyAddr.uiAddr;
		}
		else if (psDispData->num_disps > 1)
		{
			int j;
			j = i - psDispData->disps[0].num_layers;
			index = psDispData->disps[1].phys_addr[j];
			psDevInfo->sPVRJTable.pfnPVRSRVDCMemInfoGetCpuPAddr(ppsMemInfos[index],
				0, &phyAddr);
			psDispData->disps[1].phys_addr[j] = phyAddr.uiAddr;
		}
		else
		{
			printk(KERN_WARNING DRIVER_PREFIX
				": %s: Device %u: WARNING: too many mem infos. ",
				__FUNCTION__, psDevInfo->uiFBDevID);
			break;
		}
	}
	vdsscomp_gralloc_queue(psDispData,
		(void *)psDevInfo->sPVRJTable.pfnPVRSRVCmdComplete, (void *)hCmdCookie);

	return IMG_TRUE;

}
#endif

static IMG_BOOL ProcessFlip(IMG_HANDLE  hCmdCookie,
                            IMG_UINT32  ui32DataSize,
                            IMG_VOID   *pvData)
{
	DISPLAYCLASS_FLIP_COMMAND *psFlipCmd;
	SIRFSOCFB_DEVINFO *psDevInfo;

	if(!hCmdCookie || !pvData)
	{
		return IMG_FALSE;
	}


	psFlipCmd = (DISPLAYCLASS_FLIP_COMMAND*)pvData;

	if (psFlipCmd == IMG_NULL)
	{
		return IMG_FALSE;
	}

	psDevInfo = (SIRFSOCFB_DEVINFO*)psFlipCmd->hExtDevice;
	if(psFlipCmd->hExtBuffer)
	{
		return ProcessFlipV1(hCmdCookie,
							 psDevInfo,
							 (SIRFSOCFB_SWAPCHAIN *)psFlipCmd->hExtSwapChain,
							 psFlipCmd->hExtBuffer,
							 psFlipCmd->ui32SwapInterval);
	}
	else
	{
#ifdef CONFIG_VDSSCOMP
		DISPLAYCLASS_FLIP_COMMAND2 *psFlipCmd2;
		psFlipCmd2 = (DISPLAYCLASS_FLIP_COMMAND2 *)pvData;
		return ProcessFlipV2(hCmdCookie,
			psDevInfo,
			psFlipCmd2->ppsMemInfos,
			psFlipCmd2->ui32NumMemInfos,
			psFlipCmd2->pvPrivData,
			psFlipCmd2->ui32PrivDataLength);
#else
		BUG();
#endif
	}
}


static SIRFSOCFB_ERROR SIRFSOCFBInitFBDev(SIRFSOCFB_DEVINFO *psDevInfo)
{
	struct fb_info *psLINFBInfo;
	struct module *psLINFBOwner;
	SIRFSOCFB_FBINFO *psPVRFBInfo = &psDevInfo->sFBInfo;
	SIRFSOCFB_ERROR eError = SIRFSOCFB_ERROR_GENERIC;
	unsigned long FBSize;
	unsigned long ulLCM;
	unsigned uiFBDevID = psDevInfo->uiFBDevID;

	console_lock();

	psLINFBInfo = registered_fb[uiFBDevID];
	if (psLINFBInfo == NULL)
	{
		eError = SIRFSOCFB_ERROR_INVALID_DEVICE;
		goto ErrorRelSem;
	}

	FBSize = (psLINFBInfo->screen_size) != 0 ?
					psLINFBInfo->screen_size :
					psLINFBInfo->fix.smem_len;


	if (FBSize == 0 || psLINFBInfo->fix.line_length == 0)
	{
		eError = SIRFSOCFB_ERROR_INVALID_DEVICE;
		goto ErrorRelSem;
	}

	psLINFBOwner = psLINFBInfo->fbops->owner;
	if (!try_module_get(psLINFBOwner))
	{
		printk(KERN_INFO DRIVER_PREFIX
			": %s: Device %u: Couldn't get framebuffer module\n", __FUNCTION__, uiFBDevID);

		goto ErrorRelSem;
	}

	if (psLINFBInfo->fbops->fb_open != NULL)
	{
		int res;

		res = psLINFBInfo->fbops->fb_open(psLINFBInfo, 0);
		if (res != 0)
		{
			printk(KERN_INFO DRIVER_PREFIX
				" %s: Device %u: Couldn't open framebuffer(%d)\n", __FUNCTION__, uiFBDevID, res);

			goto ErrorModPut;
		}
	}

	psDevInfo->psLINFBInfo = psLINFBInfo;

	ulLCM = LCM(psLINFBInfo->fix.line_length, SIRFSOCFB_PAGE_SIZE);

	DEBUG_PRINTK((KERN_INFO DRIVER_PREFIX
			": Device %u: Framebuffer physical address: 0x%lx\n",
			psDevInfo->uiFBDevID, psLINFBInfo->fix.smem_start));
	DEBUG_PRINTK((KERN_INFO DRIVER_PREFIX
			": Device %u: Framebuffer virtual address: 0x%lx\n",
			psDevInfo->uiFBDevID, (unsigned long)psLINFBInfo->screen_base));
	DEBUG_PRINTK((KERN_INFO DRIVER_PREFIX
			": Device %u: Framebuffer size: %lu\n",
			psDevInfo->uiFBDevID, FBSize));
	DEBUG_PRINTK((KERN_INFO DRIVER_PREFIX
			": Device %u: Framebuffer virtual width: %u\n",
			psDevInfo->uiFBDevID, psLINFBInfo->var.xres_virtual));
	DEBUG_PRINTK((KERN_INFO DRIVER_PREFIX
			": Device %u: Framebuffer virtual height: %u\n",
			psDevInfo->uiFBDevID, psLINFBInfo->var.yres_virtual));
	DEBUG_PRINTK((KERN_INFO DRIVER_PREFIX
			": Device %u: Framebuffer width: %u\n",
			psDevInfo->uiFBDevID, psLINFBInfo->var.xres));
	DEBUG_PRINTK((KERN_INFO DRIVER_PREFIX
			": Device %u: Framebuffer height: %u\n",
			psDevInfo->uiFBDevID, psLINFBInfo->var.yres));
	DEBUG_PRINTK((KERN_INFO DRIVER_PREFIX
			": Device %u: Framebuffer stride: %u\n",
			psDevInfo->uiFBDevID, psLINFBInfo->fix.line_length));
	DEBUG_PRINTK((KERN_INFO DRIVER_PREFIX
			": Device %u: LCM of stride and page size: %lu\n",
			psDevInfo->uiFBDevID, ulLCM));


	psPVRFBInfo->sSysAddr.uiAddr = psLINFBInfo->fix.smem_start;
	psPVRFBInfo->sCPUVAddr = psLINFBInfo->screen_base;

	psPVRFBInfo->ulWidth = psLINFBInfo->var.xres;
	psPVRFBInfo->ulHeight = psLINFBInfo->var.yres;
	psPVRFBInfo->ulByteStride =  psLINFBInfo->fix.line_length;
	psPVRFBInfo->ulFBSize = FBSize;
	psPVRFBInfo->ulBufferSize = psPVRFBInfo->ulHeight * psPVRFBInfo->ulByteStride;

	psPVRFBInfo->ulRoundedBufferSize = RoundUpToMultiple(psPVRFBInfo->ulBufferSize, ulLCM);

	DEBUG_PRINTK((KERN_INFO DRIVER_PREFIX
			": Device %u:  Multiple of buffer size %lu and LCM: %lu\n",
			psDevInfo->uiFBDevID, psPVRFBInfo->ulBufferSize, psPVRFBInfo->ulRoundedBufferSize));

	if(psLINFBInfo->var.bits_per_pixel == 16)
	{
		if((psLINFBInfo->var.red.length == 5) &&
			(psLINFBInfo->var.green.length == 6) &&
			(psLINFBInfo->var.blue.length == 5) &&
			(psLINFBInfo->var.red.offset == 11) &&
			(psLINFBInfo->var.green.offset == 5) &&
			(psLINFBInfo->var.blue.offset == 0) &&
			(psLINFBInfo->var.red.msb_right == 0))
		{
			psPVRFBInfo->ePixelFormat = PVRSRV_PIXEL_FORMAT_RGB565;
		}
		else
		{
			printk(KERN_INFO DRIVER_PREFIX ": %s: Device %u: Unknown FB format\n", __FUNCTION__, uiFBDevID);
		}
	}
	else if(psLINFBInfo->var.bits_per_pixel == 32)
	{
		if((psLINFBInfo->var.red.length == 8) &&
			(psLINFBInfo->var.green.length == 8) &&
			(psLINFBInfo->var.blue.length == 8) &&
			(psLINFBInfo->var.red.offset == 16) &&
			(psLINFBInfo->var.green.offset == 8) &&
			(psLINFBInfo->var.blue.offset == 0) &&
			(psLINFBInfo->var.red.msb_right == 0))
		{
			psPVRFBInfo->ePixelFormat = PVRSRV_PIXEL_FORMAT_ARGB8888;
		}
		else
		{
			printk(KERN_INFO DRIVER_PREFIX ": %s: Device %u: Unknown FB format\n", __FUNCTION__, uiFBDevID);
		}
	}
	else
	{
		printk(KERN_INFO DRIVER_PREFIX ": %s: Device %u: Unknown FB format\n", __FUNCTION__, uiFBDevID);
	}

	psDevInfo->sFBInfo.ulPhysicalWidthmm =
		((int)psLINFBInfo->var.width  > 0) ? psLINFBInfo->var.width  : 90;

	psDevInfo->sFBInfo.ulPhysicalHeightmm =
		((int)psLINFBInfo->var.height > 0) ? psLINFBInfo->var.height : 54;


	psDevInfo->sFBInfo.sSysAddr.uiAddr = psPVRFBInfo->sSysAddr.uiAddr;
	psDevInfo->sFBInfo.sCPUVAddr = psPVRFBInfo->sCPUVAddr;

	eError = SIRFSOCFB_OK;
	goto ErrorRelSem;

ErrorModPut:
	module_put(psLINFBOwner);
ErrorRelSem:
	console_unlock();

	return eError;
}

static void SIRFSOCFBDeInitFBDev(SIRFSOCFB_DEVINFO *psDevInfo)
{
	struct fb_info *psLINFBInfo = psDevInfo->psLINFBInfo;
	struct module *psLINFBOwner;

	console_lock();

	psLINFBOwner = psLINFBInfo->fbops->owner;

	if (psLINFBInfo->fbops->fb_release != NULL)
	{
		(void) psLINFBInfo->fbops->fb_release(psLINFBInfo, 0);
	}

	module_put(psLINFBOwner);

	console_unlock();
}

static SIRFSOCFB_DEVINFO *SIRFSOCFBInitDev(unsigned uiFBDevID)
{
	PFN_CMD_PROC	 	pfnCmdProcList[SIRFSOCFB_COMMAND_COUNT];
	IMG_UINT32		aui32SyncCountList[SIRFSOCFB_COMMAND_COUNT][2];
	SIRFSOCFB_DEVINFO		*psDevInfo = NULL;


	psDevInfo = (SIRFSOCFB_DEVINFO *)SIRFSOCFBAllocKernelMem(sizeof(SIRFSOCFB_DEVINFO));

	if(psDevInfo == NULL)
	{
		printk(KERN_ERR DRIVER_PREFIX
			": %s: Device %u: Couldn't allocate device information structure\n", __FUNCTION__, uiFBDevID);

		goto ErrorExit;
	}


	memset(psDevInfo, 0, sizeof(SIRFSOCFB_DEVINFO));

	psDevInfo->uiFBDevID = uiFBDevID;


	if(!(*gpfnGetPVRJTable)(&psDevInfo->sPVRJTable))
	{
		goto ErrorFreeDevInfo;
	}


	if(SIRFSOCFBInitFBDev(psDevInfo) != SIRFSOCFB_OK)
	{

		goto ErrorFreeDevInfo;
	}

	psDevInfo->sDisplayInfo.ui32MaxSwapChainBuffers = (IMG_UINT32)(psDevInfo->sFBInfo.ulFBSize / psDevInfo->sFBInfo.ulRoundedBufferSize);
	if (psDevInfo->sDisplayInfo.ui32MaxSwapChainBuffers != 0)
	{
		psDevInfo->sDisplayInfo.ui32MaxSwapChains = 1;
		psDevInfo->sDisplayInfo.ui32MaxSwapInterval = 1;
	}

	psDevInfo->sDisplayInfo.ui32PhysicalWidthmm = psDevInfo->sFBInfo.ulPhysicalWidthmm;
	psDevInfo->sDisplayInfo.ui32PhysicalHeightmm = psDevInfo->sFBInfo.ulPhysicalHeightmm;

	strncpy(psDevInfo->sDisplayInfo.szDisplayName, DISPLAY_DEVICE_NAME, MAX_DISPLAY_NAME_SIZE);

	psDevInfo->sDisplayFormat.pixelformat = psDevInfo->sFBInfo.ePixelFormat;
	psDevInfo->sDisplayDim.ui32Width      = (IMG_UINT32)psDevInfo->sFBInfo.ulWidth;
	psDevInfo->sDisplayDim.ui32Height     = (IMG_UINT32)psDevInfo->sFBInfo.ulHeight;
	psDevInfo->sDisplayDim.ui32ByteStride = (IMG_UINT32)psDevInfo->sFBInfo.ulByteStride;

	DEBUG_PRINTK((KERN_INFO DRIVER_PREFIX
		": Device %u: Maximum number of swap chain buffers: %u\n",
		psDevInfo->uiFBDevID, psDevInfo->sDisplayInfo.ui32MaxSwapChainBuffers));


	psDevInfo->sSystemBuffer.sSysAddr = psDevInfo->sFBInfo.sSysAddr;
	psDevInfo->sSystemBuffer.sCPUVAddr = psDevInfo->sFBInfo.sCPUVAddr;
	psDevInfo->sSystemBuffer.psDevInfo = psDevInfo;

	SIRFSOCFBInitBufferForSwap(&psDevInfo->sSystemBuffer);



	psDevInfo->sDCJTable.ui32TableSize = sizeof(PVRSRV_DC_SRV2DISP_KMJTABLE);
	psDevInfo->sDCJTable.pfnOpenDCDevice = OpenDCDevice;
	psDevInfo->sDCJTable.pfnCloseDCDevice = CloseDCDevice;
	psDevInfo->sDCJTable.pfnEnumDCFormats = EnumDCFormats;
	psDevInfo->sDCJTable.pfnEnumDCDims = EnumDCDims;
	psDevInfo->sDCJTable.pfnGetDCSystemBuffer = GetDCSystemBuffer;
	psDevInfo->sDCJTable.pfnGetDCInfo = GetDCInfo;
	psDevInfo->sDCJTable.pfnGetBufferAddr = GetDCBufferAddr;
	psDevInfo->sDCJTable.pfnCreateDCSwapChain = CreateDCSwapChain;
	psDevInfo->sDCJTable.pfnDestroyDCSwapChain = DestroyDCSwapChain;
	psDevInfo->sDCJTable.pfnSetDCDstRect = SetDCDstRect;
	psDevInfo->sDCJTable.pfnSetDCSrcRect = SetDCSrcRect;
	psDevInfo->sDCJTable.pfnSetDCDstColourKey = SetDCDstColourKey;
	psDevInfo->sDCJTable.pfnSetDCSrcColourKey = SetDCSrcColourKey;
	psDevInfo->sDCJTable.pfnGetDCBuffers = GetDCBuffers;
	psDevInfo->sDCJTable.pfnSwapToDCBuffer = SwapToDCBuffer;
	psDevInfo->sDCJTable.pfnSetDCState = SetDCState;


	if(psDevInfo->sPVRJTable.pfnPVRSRVRegisterDCDevice(
		&psDevInfo->sDCJTable,
		&psDevInfo->uiPVRDevID) != PVRSRV_OK)
	{
		printk(KERN_ERR DRIVER_PREFIX
			": %s: Device %u: PVR Services device registration failed\n", __FUNCTION__, uiFBDevID);

		goto ErrorDeInitFBDev;
	}
	DEBUG_PRINTK((KERN_INFO DRIVER_PREFIX
		": Device %u: PVR Device ID: %u\n",
		psDevInfo->uiFBDevID, psDevInfo->uiPVRDevID));


	pfnCmdProcList[DC_FLIP_COMMAND] = ProcessFlip;


	aui32SyncCountList[DC_FLIP_COMMAND][0] = 0;
	aui32SyncCountList[DC_FLIP_COMMAND][1] = 16;





	if (psDevInfo->sPVRJTable.pfnPVRSRVRegisterCmdProcList(psDevInfo->uiPVRDevID,
															&pfnCmdProcList[0],
															aui32SyncCountList,
															SIRFSOCFB_COMMAND_COUNT) != PVRSRV_OK)
	{
		printk(KERN_ERR DRIVER_PREFIX
			": %s: Device %u: Couldn't register command processing functions with PVR Services\n", __FUNCTION__, uiFBDevID);
		goto ErrorUnregisterDevice;
	}

	SIRFSOCFBCreateSwapChainLockInit(psDevInfo);

	SIRFSOCFBAtomicBoolInit(&psDevInfo->sBlanked, SIRFSOCFB_FALSE);
	SIRFSOCFBAtomicIntInit(&psDevInfo->sBlankEvents, 0);
	SIRFSOCFBAtomicBoolInit(&psDevInfo->sFlushCommands, SIRFSOCFB_FALSE);
#if defined(CONFIG_HAS_EARLYSUSPEND)
	SIRFSOCFBAtomicBoolInit(&psDevInfo->sEarlySuspendFlag, SIRFSOCFB_FALSE);
#endif
#if defined(SUPPORT_DRI_DRM)
	SIRFSOCFBAtomicBoolInit(&psDevInfo->sLeaveVT, SIRFSOCFB_FALSE);
#endif
	return psDevInfo;

ErrorUnregisterDevice:
	(void)psDevInfo->sPVRJTable.pfnPVRSRVRemoveDCDevice(psDevInfo->uiPVRDevID);
ErrorDeInitFBDev:
	SIRFSOCFBDeInitFBDev(psDevInfo);
ErrorFreeDevInfo:
	SIRFSOCFBFreeKernelMem(psDevInfo);
ErrorExit:
	return NULL;
}

SIRFSOCFB_ERROR SIRFSOCFBInit(void)
{
	unsigned uiMaxFBDevIDPlusOne = SIRFSOCFBMaxFBDevIDPlusOne();
	unsigned i;
	unsigned uiDevicesFound = 0;

	if(SIRFSOCFBGetLibFuncAddr ("PVRGetDisplayClassJTable", &gpfnGetPVRJTable) != SIRFSOCFB_OK)
	{
		return SIRFSOCFB_ERROR_INIT_FAILURE;
	}


	for(i = 0; i < uiMaxFBDevIDPlusOne; i++)
	{
		SIRFSOCFB_DEVINFO *psDevInfo = SIRFSOCFBInitDev(i);

		if (psDevInfo != NULL)
		{

			SIRFSOCFBSetDevInfoPtr(psDevInfo->uiFBDevID, psDevInfo);
			uiDevicesFound++;
			break;
		}
	}

	return (uiDevicesFound != 0) ? SIRFSOCFB_OK : SIRFSOCFB_ERROR_INIT_FAILURE;
}

static SIRFSOCFB_BOOL SIRFSOCFBDeInitDev(SIRFSOCFB_DEVINFO *psDevInfo)
{
	PVRSRV_DC_DISP2SRV_KMJTABLE *psPVRJTable = &psDevInfo->sPVRJTable;

	SIRFSOCFBCreateSwapChainLockDeInit(psDevInfo);

	SIRFSOCFBAtomicBoolDeInit(&psDevInfo->sBlanked);
	SIRFSOCFBAtomicIntDeInit(&psDevInfo->sBlankEvents);
	SIRFSOCFBAtomicBoolDeInit(&psDevInfo->sFlushCommands);
#if defined(CONFIG_HAS_EARLYSUSPEND)
	SIRFSOCFBAtomicBoolDeInit(&psDevInfo->sEarlySuspendFlag);
#endif
#if defined(SUPPORT_DRI_DRM)
	SIRFSOCFBAtomicBoolDeInit(&psDevInfo->sLeaveVT);
#endif
	psPVRJTable = &psDevInfo->sPVRJTable;

	if (psPVRJTable->pfnPVRSRVRemoveCmdProcList (psDevInfo->uiPVRDevID, SIRFSOCFB_COMMAND_COUNT) != PVRSRV_OK)
	{
		printk(KERN_ERR DRIVER_PREFIX
			": %s: Device %u: PVR Device %u: Couldn't unregister command processing functions\n", __FUNCTION__, psDevInfo->uiFBDevID, psDevInfo->uiPVRDevID);
		return SIRFSOCFB_FALSE;
	}


	if (psPVRJTable->pfnPVRSRVRemoveDCDevice(psDevInfo->uiPVRDevID) != PVRSRV_OK)
	{
		printk(KERN_ERR DRIVER_PREFIX
			": %s: Device %u: PVR Device %u: Couldn't remove device from PVR Services\n", __FUNCTION__, psDevInfo->uiFBDevID, psDevInfo->uiPVRDevID);
		return SIRFSOCFB_FALSE;
	}

	SIRFSOCFBDeInitFBDev(psDevInfo);

	SIRFSOCFBSetDevInfoPtr(psDevInfo->uiFBDevID, NULL);


	SIRFSOCFBFreeKernelMem(psDevInfo);

	return SIRFSOCFB_TRUE;
}

SIRFSOCFB_ERROR SIRFSOCFBDeInit(void)
{
	unsigned uiMaxFBDevIDPlusOne = SIRFSOCFBMaxFBDevIDPlusOne();
	unsigned i;
	SIRFSOCFB_BOOL bError = SIRFSOCFB_FALSE;

	for(i = 0; i < uiMaxFBDevIDPlusOne; i++)
	{
		SIRFSOCFB_DEVINFO *psDevInfo = SIRFSOCFBGetDevInfoPtr(i);

		if (psDevInfo != NULL)
		{
			bError |= !SIRFSOCFBDeInitDev(psDevInfo);
		}
	}

	return (bError) ? SIRFSOCFB_ERROR_INIT_FAILURE : SIRFSOCFB_OK;
}


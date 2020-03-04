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
* Portions of the file may also include code with the following notice:
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
#ifndef __SIRFSOCFB_H__
#define __SIRFSOCFB_H__

#include <linux/version.h>

#include <asm/atomic.h>

#include <linux/kernel.h>
#include <linux/console.h>
#include <linux/fb.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/notifier.h>
#include <linux/mutex.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define unref__ __attribute__ ((unused))

typedef void *       SIRFSOCFB_HANDLE;

typedef bool SIRFSOCFB_BOOL, *SIRFSOCFB_PBOOL;
#define	SIRFSOCFB_FALSE false
#define SIRFSOCFB_TRUE true

typedef	atomic_t	SIRFSOCFB_ATOMIC_BOOL;

typedef atomic_t	SIRFSOCFB_ATOMIC_INT;

typedef struct SIRFSOCFB_BUFFER_TAG
{
	struct SIRFSOCFB_BUFFER_TAG	*psNext;
	struct SIRFSOCFB_DEVINFO_TAG	*psDevInfo;

	struct work_struct sWork;


	unsigned long		     	ulYOffset;



	IMG_SYS_PHYADDR              	sSysAddr;
	IMG_CPU_VIRTADDR             	sCPUVAddr;
	PVRSRV_SYNC_DATA            	*psSyncData;

	SIRFSOCFB_HANDLE      		hCmdComplete;
	unsigned long    		ulSwapInterval;
} SIRFSOCFB_BUFFER;

typedef struct SIRFSOCFB_SWAPCHAIN_TAG
{

	unsigned int			uiSwapChainID;


	unsigned long       		ulBufferCount;


	SIRFSOCFB_BUFFER     		*psBuffer;


	struct workqueue_struct   	*psWorkQueue;


	SIRFSOCFB_BOOL			bNotVSynced;


	int				iBlankEvents;


	unsigned int            	uiFBDevID;
} SIRFSOCFB_SWAPCHAIN;

typedef struct SIRFSOCFB_FBINFO_TAG
{
	unsigned long       ulFBSize;
	unsigned long       ulBufferSize;
	unsigned long       ulRoundedBufferSize;
	unsigned long       ulWidth;
	unsigned long       ulHeight;
	unsigned long       ulByteStride;
	unsigned long       ulPhysicalWidthmm;
	unsigned long       ulPhysicalHeightmm;



	IMG_SYS_PHYADDR     sSysAddr;
	IMG_CPU_VIRTADDR    sCPUVAddr;


	PVRSRV_PIXEL_FORMAT ePixelFormat;
}SIRFSOCFB_FBINFO;

typedef struct SIRFSOCFB_DEVINFO_TAG
{

	unsigned int            uiFBDevID;


	unsigned int            uiPVRDevID;


	struct mutex		sCreateSwapChainMutex;


	SIRFSOCFB_BUFFER          sSystemBuffer;


	PVRSRV_DC_DISP2SRV_KMJTABLE	sPVRJTable;


	PVRSRV_DC_SRV2DISP_KMJTABLE	sDCJTable;


	SIRFSOCFB_FBINFO          sFBInfo;


	SIRFSOCFB_SWAPCHAIN      *psSwapChain;


	unsigned int		uiSwapChainID;


	SIRFSOCFB_ATOMIC_BOOL     sFlushCommands;


	struct fb_info         *psLINFBInfo;


	struct notifier_block   sLINNotifBlock;





	IMG_DEV_VIRTADDR	sDisplayDevVAddr;

	DISPLAY_INFO            sDisplayInfo;


	DISPLAY_FORMAT          sDisplayFormat;


	DISPLAY_DIMS            sDisplayDim;


	SIRFSOCFB_ATOMIC_BOOL	sBlanked;


	SIRFSOCFB_ATOMIC_INT	sBlankEvents;

#ifdef CONFIG_HAS_EARLYSUSPEND

	SIRFSOCFB_ATOMIC_BOOL	sEarlySuspendFlag;

	struct early_suspend    sEarlySuspend;
#endif

#if defined(SUPPORT_DRI_DRM)
	SIRFSOCFB_ATOMIC_BOOL     sLeaveVT;
#endif

}  SIRFSOCFB_DEVINFO;

#define	SIRFSOCFB_PAGE_SIZE 4096

#ifdef	DEBUG
#define	DEBUG_PRINTK(x) printk x
#else
#define	DEBUG_PRINTK(x)
#endif

#define DISPLAY_DEVICE_NAME "PowerVR SIRFSOC Linux Display Driver"
#define	DRVNAME	"sirfsocfb"
#define	DEVNAME	DRVNAME
#define	DRIVER_PREFIX DRVNAME

typedef enum _SIRFSOCFB_ERROR_
{
	SIRFSOCFB_OK                             =  0,
	SIRFSOCFB_ERROR_GENERIC                  =  1,
	SIRFSOCFB_ERROR_OUT_OF_MEMORY            =  2,
	SIRFSOCFB_ERROR_TOO_FEW_BUFFERS          =  3,
	SIRFSOCFB_ERROR_INVALID_PARAMS           =  4,
	SIRFSOCFB_ERROR_INIT_FAILURE             =  5,
	SIRFSOCFB_ERROR_CANT_REGISTER_CALLBACK   =  6,
	SIRFSOCFB_ERROR_INVALID_DEVICE           =  7,
	SIRFSOCFB_ERROR_DEVICE_REGISTER_FAILED   =  8,
	SIRFSOCFB_ERROR_SET_UPDATE_MODE_FAILED   =  9
} SIRFSOCFB_ERROR;

typedef enum _SIRFSOCFB_UPDATE_MODE_
{
	SIRFSOCFB_UPDATE_MODE_UNDEFINED			= 0,
	SIRFSOCFB_UPDATE_MODE_MANUAL			= 1,
	SIRFSOCFB_UPDATE_MODE_AUTO			= 2,
	SIRFSOCFB_UPDATE_MODE_DISABLED			= 3
} SIRFSOCFB_UPDATE_MODE;

#ifndef UNREFERENCED_PARAMETER
#define	UNREFERENCED_PARAMETER(param) (param) = (param)
#endif

SIRFSOCFB_ERROR SIRFSOCFBInit(void);
SIRFSOCFB_ERROR SIRFSOCFBDeInit(void);

SIRFSOCFB_DEVINFO *SIRFSOCFBGetDevInfoPtr(unsigned uiFBDevID);
unsigned SIRFSOCFBMaxFBDevIDPlusOne(void);
void *SIRFSOCFBAllocKernelMem(unsigned long ulSize);
void SIRFSOCFBFreeKernelMem(void *pvMem);
SIRFSOCFB_ERROR SIRFSOCFBGetLibFuncAddr(char *szFunctionName, PFN_DC_GET_PVRJTABLE *ppfnFuncTable);
SIRFSOCFB_ERROR SIRFSOCFBCreateSwapQueue (SIRFSOCFB_SWAPCHAIN *psSwapChain);
void SIRFSOCFBDestroySwapQueue(SIRFSOCFB_SWAPCHAIN *psSwapChain);
void SIRFSOCFBInitBufferForSwap(SIRFSOCFB_BUFFER *psBuffer);
void SIRFSOCFBSwapHandler(SIRFSOCFB_BUFFER *psBuffer);
void SIRFSOCFBQueueBufferForSwap(SIRFSOCFB_SWAPCHAIN *psSwapChain, SIRFSOCFB_BUFFER *psBuffer);
void SIRFSOCFBFlip(SIRFSOCFB_DEVINFO *psDevInfo, SIRFSOCFB_BUFFER *psBuffer);
SIRFSOCFB_UPDATE_MODE SIRFSOCFBGetUpdateMode(SIRFSOCFB_DEVINFO *psDevInfo);
SIRFSOCFB_BOOL SIRFSOCFBSetUpdateMode(SIRFSOCFB_DEVINFO *psDevInfo, SIRFSOCFB_UPDATE_MODE eMode);
SIRFSOCFB_BOOL SIRFSOCFBWaitForVSync(SIRFSOCFB_DEVINFO *psDevInfo);
SIRFSOCFB_BOOL SIRFSOCFBManualSync(SIRFSOCFB_DEVINFO *psDevInfo);
SIRFSOCFB_BOOL SIRFSOCFBCheckModeAndSync(SIRFSOCFB_DEVINFO *psDevInfo);
SIRFSOCFB_ERROR SIRFSOCFBUnblankDisplay(SIRFSOCFB_DEVINFO *psDevInfo);
SIRFSOCFB_ERROR SIRFSOCFBEnableLFBEventNotification(SIRFSOCFB_DEVINFO *psDevInfo);
SIRFSOCFB_ERROR SIRFSOCFBDisableLFBEventNotification(SIRFSOCFB_DEVINFO *psDevInfo);
void SIRFSOCFBCreateSwapChainLockInit(SIRFSOCFB_DEVINFO *psDevInfo);
void SIRFSOCFBCreateSwapChainLockDeInit(SIRFSOCFB_DEVINFO *psDevInfo);
void SIRFSOCFBCreateSwapChainLock(SIRFSOCFB_DEVINFO *psDevInfo);
void SIRFSOCFBCreateSwapChainUnLock(SIRFSOCFB_DEVINFO *psDevInfo);
void SIRFSOCFBAtomicBoolInit(SIRFSOCFB_ATOMIC_BOOL *psAtomic, SIRFSOCFB_BOOL bVal);
void SIRFSOCFBAtomicBoolDeInit(SIRFSOCFB_ATOMIC_BOOL *psAtomic);
void SIRFSOCFBAtomicBoolSet(SIRFSOCFB_ATOMIC_BOOL *psAtomic, SIRFSOCFB_BOOL bVal);
SIRFSOCFB_BOOL SIRFSOCFBAtomicBoolRead(SIRFSOCFB_ATOMIC_BOOL *psAtomic);
void SIRFSOCFBAtomicIntInit(SIRFSOCFB_ATOMIC_INT *psAtomic, int iVal);
void SIRFSOCFBAtomicIntDeInit(SIRFSOCFB_ATOMIC_INT *psAtomic);
void SIRFSOCFBAtomicIntSet(SIRFSOCFB_ATOMIC_INT *psAtomic, int iVal);
int SIRFSOCFBAtomicIntRead(SIRFSOCFB_ATOMIC_INT *psAtomic);
void SIRFSOCFBAtomicIntInc(SIRFSOCFB_ATOMIC_INT *psAtomic);

#endif


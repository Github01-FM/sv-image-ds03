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

#include <asm/atomic.h>

#if defined(SUPPORT_DRI_DRM)
#include <drm/drmP.h>
#else
#include <linux/module.h>
#endif

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/hardirq.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/fb.h>
#include <linux/console.h>
#include <linux/mutex.h>

#if defined(DEBUG)
#define	PVR_DEBUG DEBUG
#undef DEBUG
#endif
#if defined(DEBUG)
#undef DEBUG
#endif
#if defined(PVR_DEBUG)
#define	DEBUG PVR_DEBUG
#undef PVR_DEBUG
#endif

#include "img_defs.h"
#include "servicesext.h"
#include "kerneldisplay.h"
#include "sirfsocfb.h"
#include "pvrmodule.h"
#if defined(SUPPORT_DRI_DRM)
#include "pvr_drm.h"
#include "3rdparty_dc_drm_shared.h"
#endif

#if !defined(PVR_LINUX_USING_WORKQUEUES)
#error "PVR_LINUX_USING_WORKQUEUES must be defined"
#endif

MODULE_SUPPORTED_DEVICE(DEVNAME);


void *SIRFSOCFBAllocKernelMem(unsigned long ulSize)
{
	return kmalloc(ulSize, GFP_KERNEL);
}

void SIRFSOCFBFreeKernelMem(void *pvMem)
{
	kfree(pvMem);
}

void SIRFSOCFBCreateSwapChainLockInit(SIRFSOCFB_DEVINFO *psDevInfo)
{
	mutex_init(&psDevInfo->sCreateSwapChainMutex);
}

void SIRFSOCFBCreateSwapChainLockDeInit(SIRFSOCFB_DEVINFO *psDevInfo)
{
	mutex_destroy(&psDevInfo->sCreateSwapChainMutex);
}

void SIRFSOCFBCreateSwapChainLock(SIRFSOCFB_DEVINFO *psDevInfo)
{
	mutex_lock(&psDevInfo->sCreateSwapChainMutex);
}

void SIRFSOCFBCreateSwapChainUnLock(SIRFSOCFB_DEVINFO *psDevInfo)
{
	mutex_unlock(&psDevInfo->sCreateSwapChainMutex);
}

void SIRFSOCFBAtomicBoolInit(SIRFSOCFB_ATOMIC_BOOL *psAtomic, SIRFSOCFB_BOOL bVal)
{
	atomic_set(psAtomic, (int)bVal);
}

void SIRFSOCFBAtomicBoolDeInit(SIRFSOCFB_ATOMIC_BOOL *psAtomic)
{
}

void SIRFSOCFBAtomicBoolSet(SIRFSOCFB_ATOMIC_BOOL *psAtomic, SIRFSOCFB_BOOL bVal)
{
	atomic_set(psAtomic, (int)bVal);
}

SIRFSOCFB_BOOL SIRFSOCFBAtomicBoolRead(SIRFSOCFB_ATOMIC_BOOL *psAtomic)
{
	return (SIRFSOCFB_BOOL)atomic_read(psAtomic);
}

void SIRFSOCFBAtomicIntInit(SIRFSOCFB_ATOMIC_INT *psAtomic, int iVal)
{
	atomic_set(psAtomic, iVal);
}

void SIRFSOCFBAtomicIntDeInit(SIRFSOCFB_ATOMIC_INT *psAtomic)
{
}

void SIRFSOCFBAtomicIntSet(SIRFSOCFB_ATOMIC_INT *psAtomic, int iVal)
{
	atomic_set(psAtomic, iVal);
}

int SIRFSOCFBAtomicIntRead(SIRFSOCFB_ATOMIC_INT *psAtomic)
{
	return atomic_read(psAtomic);
}

void SIRFSOCFBAtomicIntInc(SIRFSOCFB_ATOMIC_INT *psAtomic)
{
	atomic_inc(psAtomic);
}

SIRFSOCFB_ERROR SIRFSOCFBGetLibFuncAddr (char *szFunctionName, PFN_DC_GET_PVRJTABLE *ppfnFuncTable)
{
	if(strcmp("PVRGetDisplayClassJTable", szFunctionName) != 0)
	{
		return (SIRFSOCFB_ERROR_INVALID_PARAMS);
	}


	*ppfnFuncTable = PVRGetDisplayClassJTable;

	return (SIRFSOCFB_OK);
}

void SIRFSOCFBQueueBufferForSwap(SIRFSOCFB_SWAPCHAIN *psSwapChain, SIRFSOCFB_BUFFER *psBuffer)
{
	int res = queue_work(psSwapChain->psWorkQueue, &psBuffer->sWork);

	if (res == 0)
	{
		printk(KERN_WARNING DRIVER_PREFIX ": %s: Device %u: Buffer already on work queue\n", __FUNCTION__, psSwapChain->uiFBDevID);
	}
}

static void WorkQueueHandler(struct work_struct *psWork)
{
	SIRFSOCFB_BUFFER *psBuffer = container_of(psWork, SIRFSOCFB_BUFFER, sWork);

	SIRFSOCFBSwapHandler(psBuffer);
}

SIRFSOCFB_ERROR SIRFSOCFBCreateSwapQueue(SIRFSOCFB_SWAPCHAIN *psSwapChain)
{

	psSwapChain->psWorkQueue = alloc_ordered_workqueue(DEVNAME, WQ_FREEZABLE | WQ_HIGHPRI);
	if (psSwapChain->psWorkQueue == NULL)
	{
		printk(KERN_WARNING DRIVER_PREFIX ": %s: Device %u: create_singlethreaded_workqueue failed\n", __FUNCTION__, psSwapChain->uiFBDevID);

		return (SIRFSOCFB_ERROR_INIT_FAILURE);
	}

	return (SIRFSOCFB_OK);
}

void SIRFSOCFBInitBufferForSwap(SIRFSOCFB_BUFFER *psBuffer)
{
	INIT_WORK(&psBuffer->sWork, WorkQueueHandler);
}

void SIRFSOCFBDestroySwapQueue(SIRFSOCFB_SWAPCHAIN *psSwapChain)
{
	destroy_workqueue(psSwapChain->psWorkQueue);
}

void SIRFSOCFBFlip(SIRFSOCFB_DEVINFO *psDevInfo, SIRFSOCFB_BUFFER *psBuffer)
{
	struct fb_var_screeninfo sFBVar;
	int res;
	unsigned long ulYResVirtual;

	console_lock();

	sFBVar = psDevInfo->psLINFBInfo->var;

	sFBVar.xoffset = 0;
	sFBVar.yoffset = psBuffer->ulYOffset;

	ulYResVirtual = psBuffer->ulYOffset + sFBVar.yres;

	if (sFBVar.xres_virtual != sFBVar.xres || sFBVar.yres_virtual < ulYResVirtual)
	{

		sFBVar.xres_virtual = sFBVar.xres;
		sFBVar.yres_virtual = ulYResVirtual;

		sFBVar.activate = FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;

		res = fb_set_var(psDevInfo->psLINFBInfo, &sFBVar);
		if (res != 0)
		{
			printk(KERN_INFO DRIVER_PREFIX ": %s: Device %u: fb_set_var failed (Y Offset: %lu, Error: %d)\n", __FUNCTION__, psDevInfo->uiFBDevID, psBuffer->ulYOffset, res);
		}
	}
	else
	{
		sFBVar.activate = FB_ACTIVATE_VBL;
		res = fb_pan_display(psDevInfo->psLINFBInfo, &sFBVar);
		if (res != 0)
		{
			printk(KERN_INFO DRIVER_PREFIX ": %s: Device %u: fb_pan_display failed (Y Offset: %lu, Error: %d)\n", __FUNCTION__, psDevInfo->uiFBDevID, psBuffer->ulYOffset, res);
		}
	}

	console_unlock();
}

SIRFSOCFB_UPDATE_MODE SIRFSOCFBGetUpdateMode(SIRFSOCFB_DEVINFO *psDevInfo)
{
	return SIRFSOCFB_UPDATE_MODE_UNDEFINED;
}

SIRFSOCFB_BOOL SIRFSOCFBSetUpdateMode(SIRFSOCFB_DEVINFO *psDevInfo, SIRFSOCFB_UPDATE_MODE eMode)
{
    return SIRFSOCFB_TRUE;
}

SIRFSOCFB_BOOL SIRFSOCFBWaitForVSync(SIRFSOCFB_DEVINFO *psDevInfo)
{
	/* TODO, need to implement the wait for vsync */
	return SIRFSOCFB_TRUE;
}

SIRFSOCFB_BOOL SIRFSOCFBManualSync(SIRFSOCFB_DEVINFO *psDevInfo)
{
	return SIRFSOCFB_TRUE;
}

SIRFSOCFB_BOOL SIRFSOCFBCheckModeAndSync(SIRFSOCFB_DEVINFO *psDevInfo)
{
	SIRFSOCFB_UPDATE_MODE eMode = SIRFSOCFBGetUpdateMode(psDevInfo);

	switch(eMode)
	{
		case SIRFSOCFB_UPDATE_MODE_AUTO:
		case SIRFSOCFB_UPDATE_MODE_MANUAL:
			return SIRFSOCFBManualSync(psDevInfo);
		default:
			break;
	}

	return SIRFSOCFB_TRUE;
}

static int SIRFSOCFBFrameBufferEvents(struct notifier_block *psNotif,
                             unsigned long event, void *data)
{
	SIRFSOCFB_DEVINFO *psDevInfo;
	struct fb_event *psFBEvent = (struct fb_event *)data;
	struct fb_info *psFBInfo = psFBEvent->info;
	SIRFSOCFB_BOOL bBlanked;


	if (event != FB_EVENT_BLANK)
	{
		return 0;
	}

	bBlanked = (*(IMG_INT *)psFBEvent->data != 0) ? SIRFSOCFB_TRUE: SIRFSOCFB_FALSE;

	psDevInfo = SIRFSOCFBGetDevInfoPtr(psFBInfo->node);

#if 0
	if (psDevInfo != NULL)
	{
		if (bBlanked)
		{
			DEBUG_PRINTK((KERN_INFO DRIVER_PREFIX ": %s: Device %u: Blank event received\n", __FUNCTION__, psDevInfo->uiFBDevID));
		}
		else
		{
			DEBUG_PRINTK((KERN_INFO DRIVER_PREFIX ": %s: Device %u: Unblank event received\n", __FUNCTION__, psDevInfo->uiFBDevID));
		}
	}
	else
	{
		DEBUG_PRINTK((KERN_INFO DRIVER_PREFIX ": %s: Device %u: Blank/Unblank event for unknown framebuffer\n", __FUNCTION__, psFBInfo->node));
	}
#endif

	if (psDevInfo != NULL)
	{
		SIRFSOCFBAtomicBoolSet(&psDevInfo->sBlanked, bBlanked);
		SIRFSOCFBAtomicIntInc(&psDevInfo->sBlankEvents);
	}

	return 0;
}

SIRFSOCFB_ERROR SIRFSOCFBUnblankDisplay(SIRFSOCFB_DEVINFO *psDevInfo)
{
	int res;

	console_lock();
	res = fb_blank(psDevInfo->psLINFBInfo, 0);
	console_unlock();
	if (res != 0 && res != -EINVAL)
	{
		printk(KERN_WARNING DRIVER_PREFIX
			": %s: Device %u: fb_blank failed (%d)\n", __FUNCTION__, psDevInfo->uiFBDevID, res);
		return (SIRFSOCFB_ERROR_GENERIC);
	}

	return (SIRFSOCFB_OK);
}

#ifdef CONFIG_HAS_EARLYSUSPEND

static void SIRFSOCFBBlankDisplay(SIRFSOCFB_DEVINFO *psDevInfo)
{
	console_lock();
	fb_blank(psDevInfo->psLINFBInfo, 1);
	console_unlock();
}

static void SIRFSOCFBEarlySuspendHandler(struct early_suspend *h)
{
	unsigned uiMaxFBDevIDPlusOne = SIRFSOCFBMaxFBDevIDPlusOne();
	unsigned i;

	for (i=0; i < uiMaxFBDevIDPlusOne; i++)
	{
		SIRFSOCFB_DEVINFO *psDevInfo = SIRFSOCFBGetDevInfoPtr(i);

		if (psDevInfo != NULL)
		{
			SIRFSOCFBAtomicBoolSet(&psDevInfo->sEarlySuspendFlag, SIRFSOCFB_TRUE);
			SIRFSOCFBBlankDisplay(psDevInfo);
		}
	}
}

static void SIRFSOCFBEarlyResumeHandler(struct early_suspend *h)
{
	unsigned uiMaxFBDevIDPlusOne = SIRFSOCFBMaxFBDevIDPlusOne();
	unsigned i;

	for (i=0; i < uiMaxFBDevIDPlusOne; i++)
	{
		SIRFSOCFB_DEVINFO *psDevInfo = SIRFSOCFBGetDevInfoPtr(i);

		if (psDevInfo != NULL)
		{
			SIRFSOCFBUnblankDisplay(psDevInfo);
			SIRFSOCFBAtomicBoolSet(&psDevInfo->sEarlySuspendFlag, SIRFSOCFB_FALSE);
		}
	}
}

#endif

SIRFSOCFB_ERROR SIRFSOCFBEnableLFBEventNotification(SIRFSOCFB_DEVINFO *psDevInfo)
{
	int                res;
	SIRFSOCFB_ERROR         eError;


	memset(&psDevInfo->sLINNotifBlock, 0, sizeof(psDevInfo->sLINNotifBlock));

	psDevInfo->sLINNotifBlock.notifier_call = SIRFSOCFBFrameBufferEvents;

	SIRFSOCFBAtomicBoolSet(&psDevInfo->sBlanked, SIRFSOCFB_FALSE);
	SIRFSOCFBAtomicIntSet(&psDevInfo->sBlankEvents, 0);

	res = fb_register_client(&psDevInfo->sLINNotifBlock);
	if (res != 0)
	{
		printk(KERN_WARNING DRIVER_PREFIX
			": %s: Device %u: fb_register_client failed (%d)\n", __FUNCTION__, psDevInfo->uiFBDevID, res);

		return (SIRFSOCFB_ERROR_GENERIC);
	}

	eError = SIRFSOCFBUnblankDisplay(psDevInfo);
	if (eError != SIRFSOCFB_OK)
	{
		printk(KERN_WARNING DRIVER_PREFIX
			": %s: Device %u: UnblankDisplay failed (%d)\n", __FUNCTION__, psDevInfo->uiFBDevID, eError);
		return eError;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	psDevInfo->sEarlySuspend.suspend = SIRFSOCFBEarlySuspendHandler;
	psDevInfo->sEarlySuspend.resume = SIRFSOCFBEarlyResumeHandler;
	psDevInfo->sEarlySuspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	register_early_suspend(&psDevInfo->sEarlySuspend);
#endif

	return (SIRFSOCFB_OK);
}

SIRFSOCFB_ERROR SIRFSOCFBDisableLFBEventNotification(SIRFSOCFB_DEVINFO *psDevInfo)
{
	int res;

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&psDevInfo->sEarlySuspend);
#endif


	res = fb_unregister_client(&psDevInfo->sLINNotifBlock);
	if (res != 0)
	{
		printk(KERN_WARNING DRIVER_PREFIX
			": %s: Device %u: fb_unregister_client failed (%d)\n", __FUNCTION__, psDevInfo->uiFBDevID, res);
		return (SIRFSOCFB_ERROR_GENERIC);
	}

	SIRFSOCFBAtomicBoolSet(&psDevInfo->sBlanked, SIRFSOCFB_FALSE);

	return (SIRFSOCFB_OK);
}

#if defined(SUPPORT_DRI_DRM) && defined(PVR_DISPLAY_CONTROLLER_DRM_IOCTL)
static SIRFSOCFB_DEVINFO *SIRFSOCFBPVRDevIDToDevInfo(unsigned uiPVRDevID)
{
	unsigned uiMaxFBDevIDPlusOne = SIRFSOCFBMaxFBDevIDPlusOne();
	unsigned i;

	for (i=0; i < uiMaxFBDevIDPlusOne; i++)
	{
		SIRFSOCFB_DEVINFO *psDevInfo = SIRFSOCFBGetDevInfoPtr(i);

		if (psDevInfo->uiPVRDevID == uiPVRDevID)
		{
			return psDevInfo;
		}
	}

	printk(KERN_WARNING DRIVER_PREFIX
		": %s: PVR Device %u: Couldn't find device\n", __FUNCTION__, uiPVRDevID);

	return NULL;
}

int PVR_DRM_MAKENAME(DISPLAY_CONTROLLER, _Ioctl)(struct drm_device unref__ *dev, void *arg, struct drm_file unref__ *pFile)
{
	uint32_t *puiArgs;
	uint32_t uiCmd;
	unsigned uiPVRDevID;
	int ret = 0;
	SIRFSOCFB_DEVINFO *psDevInfo;

	if (arg == NULL)
	{
		return -EFAULT;
	}

	puiArgs = (uint32_t *)arg;
	uiCmd = puiArgs[PVR_DRM_DISP_ARG_CMD];
	uiPVRDevID = puiArgs[PVR_DRM_DISP_ARG_DEV];

	psDevInfo = SIRFSOCFBPVRDevIDToDevInfo(uiPVRDevID);
	if (psDevInfo == NULL)
	{
		return -EINVAL;
	}


	switch (uiCmd)
	{
		case PVR_DRM_DISP_CMD_LEAVE_VT:
		case PVR_DRM_DISP_CMD_ENTER_VT:
		{
			SIRFSOCFB_BOOL bLeaveVT = (uiCmd == PVR_DRM_DISP_CMD_LEAVE_VT);
			DEBUG_PRINTK((KERN_WARNING DRIVER_PREFIX ": %s: PVR Device %u: %s\n",
				__FUNCTION__, uiPVRDevID,
				bLeaveVT ? "Leave VT" : "Enter VT"));

			SIRFSOCFBCreateSwapChainLock(psDevInfo);

			SIRFSOCFBAtomicBoolSet(&psDevInfo->sLeaveVT, bLeaveVT);
			if (psDevInfo->psSwapChain != NULL)
			{
				flush_workqueue(psDevInfo->psSwapChain->psWorkQueue);

				if (bLeaveVT)
				{
					SIRFSOCFBFlip(psDevInfo, &psDevInfo->sSystemBuffer);
					(void) SIRFSOCFBCheckModeAndSync(psDevInfo);
				}
			}

			SIRFSOCFBCreateSwapChainUnLock(psDevInfo);
			(void) SIRFSOCFBUnblankDisplay(psDevInfo);
			break;
		}
		case PVR_DRM_DISP_CMD_ON:
		case PVR_DRM_DISP_CMD_STANDBY:
		case PVR_DRM_DISP_CMD_SUSPEND:
		case PVR_DRM_DISP_CMD_OFF:
		{
			int iFBMode;
#if defined(DEBUG)
			{
				const char *pszMode;
				switch(uiCmd)
				{
					case PVR_DRM_DISP_CMD_ON:
						pszMode = "On";
						break;
					case PVR_DRM_DISP_CMD_STANDBY:
						pszMode = "Standby";
						break;
					case PVR_DRM_DISP_CMD_SUSPEND:
						pszMode = "Suspend";
						break;
					case PVR_DRM_DISP_CMD_OFF:
						pszMode = "Off";
						break;
					default:
						pszMode = "(Unknown Mode)";
						break;
				}
				printk (KERN_WARNING DRIVER_PREFIX ": %s: PVR Device %u: Display %s\n",
				__FUNCTION__, uiPVRDevID, pszMode);
			}
#endif
			switch(uiCmd)
			{
				case PVR_DRM_DISP_CMD_ON:
					iFBMode = FB_BLANK_UNBLANK;
					break;
				case PVR_DRM_DISP_CMD_STANDBY:
					iFBMode = FB_BLANK_HSYNC_SUSPEND;
					break;
				case PVR_DRM_DISP_CMD_SUSPEND:
					iFBMode = FB_BLANK_VSYNC_SUSPEND;
					break;
				case PVR_DRM_DISP_CMD_OFF:
					iFBMode = FB_BLANK_POWERDOWN;
					break;
				default:
					return -EINVAL;
			}

			SIRFSOCFBCreateSwapChainLock(psDevInfo);

			if (psDevInfo->psSwapChain != NULL)
			{
				flush_workqueue(psDevInfo->psSwapChain->psWorkQueue);
			}

			console_lock();
			ret = fb_blank(psDevInfo->psLINFBInfo, iFBMode);
			console_unlock();

			SIRFSOCFBCreateSwapChainUnLock(psDevInfo);

			break;
		}
		default:
		{
			ret = -EINVAL;
			break;
		}
	}

	return ret;
}
#endif

#if defined(SUPPORT_DRI_DRM)
int PVR_DRM_MAKENAME(DISPLAY_CONTROLLER, _Init)(struct drm_device unref__ *dev)
#else
static int __init SIRFSOCFB_Init(void)
#endif
{

	if(SIRFSOCFBInit() != SIRFSOCFB_OK)
	{
		printk(KERN_WARNING DRIVER_PREFIX ": %s: SIRFSOCFBInit failed\n", __FUNCTION__);
		return -ENODEV;
	}

	return 0;

}

#if defined(SUPPORT_DRI_DRM)
void PVR_DRM_MAKENAME(DISPLAY_CONTROLLER, _Cleanup)(struct drm_device unref__ *dev)
#else
static void __exit SIRFSOCFB_Cleanup(void)
#endif
{
	if(SIRFSOCFBDeInit() != SIRFSOCFB_OK)
	{
		printk(KERN_WARNING DRIVER_PREFIX ": %s: SIRFSOCFBDeInit failed\n", __FUNCTION__);
	}
}

#if !defined(SUPPORT_DRI_DRM)
late_initcall(SIRFSOCFB_Init);
module_exit(SIRFSOCFB_Cleanup);
#endif

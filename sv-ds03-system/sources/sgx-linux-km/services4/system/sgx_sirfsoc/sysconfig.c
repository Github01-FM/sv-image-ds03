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
#include "services_headers.h"
#include "kerneldisplay.h"
#include "oemfuncs.h"
#include "sgxinfo.h"
#include "sgxinfokm.h"
#include "syslocal.h"
#include "sysconfig.h"

SYS_DATA* gpsSysData = (SYS_DATA*)IMG_NULL;
SYS_DATA  gsSysData;

static SYS_SPECIFIC_DATA gsSysSpecificData;
SYS_SPECIFIC_DATA *gpsSysSpecificData;

static IMG_UINT32	gui32SGXDeviceID;
static SGX_DEVICE_MAP	gsSGXDeviceMap;
static PVRSRV_DEVICE_NODE *gpsSGXDevNode;

#define DEVICE_SGX_INTERRUPT 1

#if defined(NO_HARDWARE)
static IMG_CPU_VIRTADDR gsSGXRegsCPUVAddr;
#endif

#if defined(PVR_LINUX_DYNAMIC_SGX_RESOURCE_INFO)
extern struct platform_device *gpsPVRLDMDev;
#endif

IMG_UINT32 PVRSRV_BridgeDispatchKM(IMG_UINT32	Ioctl,
								   IMG_BYTE		*pInBuf,
								   IMG_UINT32	InBufLen,
								   IMG_BYTE		*pOutBuf,
								   IMG_UINT32	OutBufLen,
								   IMG_UINT32	*pdwBytesTransferred);

static INLINE PVRSRV_ERROR EnableSGXClocksWrap(SYS_DATA *psSysData)
{
	return EnableSGXClocks(psSysData);
}

static INLINE PVRSRV_ERROR EnableSystemClocksWrap(SYS_DATA *psSysData)
{
	PVRSRV_ERROR eError = EnableSystemClocks(psSysData);

#if !defined(SUPPORT_ACTIVE_POWER_MANAGEMENT)
	if(eError == PVRSRV_OK)
	{

		EnableSGXClocksWrap(psSysData);
	}
#endif

	return eError;
}

static PVRSRV_ERROR SysLocateDevices(SYS_DATA *psSysData)
{
#if defined(NO_HARDWARE)
	PVRSRV_ERROR eError;
	IMG_CPU_PHYADDR sCpuPAddr;
#else
#if defined(PVR_LINUX_DYNAMIC_SGX_RESOURCE_INFO)
	struct resource *dev_res;
	int dev_irq;
#endif
#endif

	PVR_UNREFERENCED_PARAMETER(psSysData);


	gsSGXDeviceMap.ui32Flags = 0x0;

#if defined(NO_HARDWARE)


	eError = OSBaseAllocContigMemory(SYS_SGX_REGS_SIZE,
									 &gsSGXRegsCPUVAddr,
									 &sCpuPAddr);
	if(eError != PVRSRV_OK)
	{
		return eError;
	}
	gsSGXDeviceMap.sRegsCpuPBase = sCpuPAddr;
	gsSGXDeviceMap.sRegsSysPBase = SysCpuPAddrToSysPAddr(gsSGXDeviceMap.sRegsCpuPBase);
	gsSGXDeviceMap.ui32RegsSize = SYS_SGX_REGS_SIZE;
#if defined(__linux__)

	gsSGXDeviceMap.pvRegsCpuVBase = gsSGXRegsCPUVAddr;
#else

	gsSGXDeviceMap.pvRegsCpuVBase = IMG_NULL;
#endif

	OSMemSet(gsSGXRegsCPUVAddr, 0, SYS_SGX_REGS_SIZE);

	/*
	 * device interrupt IRQ
	 * Note: no interrupts available on no hardware system
	 */
	gsSGXDeviceMap.ui32IRQ = 0;

#else
#if defined(PVR_LINUX_DYNAMIC_SGX_RESOURCE_INFO)
	/* get the resource and IRQ through platform resource API */
	dev_res = platform_get_resource(gpsPVRLDMDev, IORESOURCE_MEM, 0);
	if (dev_res == NULL)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: platform_get_resource failed", __FUNCTION__));
		return PVRSRV_ERROR_INVALID_DEVICE;
	}

	dev_irq = platform_get_irq(gpsPVRLDMDev, 0);
	if (dev_irq < 0)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: platform_get_irq failed (%d)", __FUNCTION__, -dev_irq));
		return PVRSRV_ERROR_INVALID_DEVICE;
	}

	gsSGXDeviceMap.sRegsSysPBase.uiAddr = dev_res->start;
	gsSGXDeviceMap.sRegsCpuPBase =
		SysSysPAddrToCpuPAddr(gsSGXDeviceMap.sRegsSysPBase);
	PVR_TRACE(("SGX register base: 0x%lx", (unsigned long)gsSGXDeviceMap.sRegsCpuPBase.uiAddr));

	gsSGXDeviceMap.ui32RegsSize = (unsigned int)(dev_res->end - dev_res->start);
	PVR_TRACE(("SGX register size: %d",gsSGXDeviceMap.ui32RegsSize));

	gsSGXDeviceMap.ui32IRQ = dev_irq;
	PVR_TRACE(("SGX IRQ: %d", gsSGXDeviceMap.ui32IRQ));
#else

	gsSGXDeviceMap.sRegsSysPBase.uiAddr = SYS_SGX_REGS_SYS_PHYS_BASE;
	gsSGXDeviceMap.sRegsCpuPBase = SysSysPAddrToCpuPAddr(gsSGXDeviceMap.sRegsSysPBase);
	gsSGXDeviceMap.ui32RegsSize = SYS_SGX_REGS_SIZE;

	gsSGXDeviceMap.ui32IRQ = SYS_SGX_IRQ;
#endif
#endif

#if defined(PDUMP)
	{

		static IMG_CHAR pszPDumpDevName[] = "SGXMEM";
		gsSGXDeviceMap.pszPDumpDevName = pszPDumpDevName;
	}
#endif




	return PVRSRV_OK;
}


IMG_CHAR *SysCreateVersionString(IMG_CPU_PHYADDR sRegRegion)
{
	static IMG_CHAR aszVersionString[100];
	SYS_DATA	*psSysData;
	IMG_UINT32	ui32SGXRevision;
	IMG_INT32	i32Count;
#if !defined(NO_HARDWARE)
	IMG_VOID	*pvRegsLinAddr;

	pvRegsLinAddr = OSMapPhysToLin(sRegRegion,
								   SYS_SGX_REGS_SIZE,
								   PVRSRV_HAP_UNCACHED|PVRSRV_HAP_KERNEL_ONLY,
								   IMG_NULL);
	if(!pvRegsLinAddr)
	{
		return IMG_NULL;
	}

	ui32SGXRevision = OSReadHWReg((IMG_PVOID)((IMG_PBYTE)pvRegsLinAddr),
								  EUR_CR_CORE_REVISION);
#else
	ui32SGXRevision = 0;
#endif

	SysAcquireData(&psSysData);

	i32Count = OSSNPrintf(aszVersionString, 100,
						  "SGX revision = %u.%u.%u",
						  (IMG_UINT)((ui32SGXRevision & EUR_CR_CORE_REVISION_MAJOR_MASK)
							>> EUR_CR_CORE_REVISION_MAJOR_SHIFT),
						  (IMG_UINT)((ui32SGXRevision & EUR_CR_CORE_REVISION_MINOR_MASK)
							>> EUR_CR_CORE_REVISION_MINOR_SHIFT),
						  (IMG_UINT)((ui32SGXRevision & EUR_CR_CORE_REVISION_MAINTENANCE_MASK)
							>> EUR_CR_CORE_REVISION_MAINTENANCE_SHIFT)
						 );

#if !defined(NO_HARDWARE)
	OSUnMapPhysToLin(pvRegsLinAddr,
					 SYS_SGX_REGS_SIZE,
					 PVRSRV_HAP_UNCACHED|PVRSRV_HAP_KERNEL_ONLY,
					 IMG_NULL);
#endif

	if(i32Count == -1)
	{
		return IMG_NULL;
	}

	return aszVersionString;
}


PVRSRV_ERROR SysInitialise(IMG_VOID)
{
	IMG_UINT32			i;
	PVRSRV_ERROR 		eError;
	PVRSRV_DEVICE_NODE	*psDeviceNode;
#if !defined(SGX_DYNAMIC_TIMING_INFO)
	SGX_TIMING_INFORMATION*	psTimingInfo;
#endif
	gpsSysData = &gsSysData;
	OSMemSet(gpsSysData, 0, sizeof(SYS_DATA));

	gpsSysSpecificData =  &gsSysSpecificData;
	OSMemSet(gpsSysSpecificData, 0, sizeof(SYS_SPECIFIC_DATA));

	gpsSysData->pvSysSpecificData = gpsSysSpecificData;

	eError = OSInitEnvData(&gpsSysData->pvEnvSpecificData);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"SysInitialise: Failed to setup env structure"));
		(IMG_VOID)SysDeinitialise(gpsSysData);
		gpsSysData = IMG_NULL;
		return eError;
	}
	SYS_SPECIFIC_DATA_SET(&gsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_ENVDATA);

	gpsSysData->ui32NumDevices = SYS_DEVICE_COUNT;


	for(i=0; i<SYS_DEVICE_COUNT; i++)
	{
		gpsSysData->sDeviceID[i].uiID = i;
		gpsSysData->sDeviceID[i].bInUse = IMG_FALSE;
	}

	gpsSysData->psDeviceNodeList = IMG_NULL;
	gpsSysData->psQueueList = IMG_NULL;

	eError = SysInitialiseCommon(gpsSysData);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"SysInitialise: Failed in SysInitialiseCommon"));
		(IMG_VOID)SysDeinitialise(gpsSysData);
		gpsSysData = IMG_NULL;
		return eError;
	}


#if !defined(SGX_DYNAMIC_TIMING_INFO)

	psTimingInfo = &gsSGXDeviceMap.sTimingInfo;
	psTimingInfo->ui32CoreClockSpeed = SYS_SGX_CLOCK_SPEED;
	psTimingInfo->ui32HWRecoveryFreq = SYS_SGX_HWRECOVERY_TIMEOUT_FREQ;
#if defined(SUPPORT_ACTIVE_POWER_MANAGEMENT)
	psTimingInfo->bEnableActivePM = IMG_TRUE;
#else
	psTimingInfo->bEnableActivePM = IMG_FALSE;
#endif
	psTimingInfo->ui32ActivePowManLatencyms = SYS_SGX_ACTIVE_POWER_LATENCY_MS;
	psTimingInfo->ui32uKernelFreq = SYS_SGX_PDS_TIMER_FREQ;
#endif


	eError = SysLocateDevices(gpsSysData);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"SysInitialise: Failed to locate devices"));
		(IMG_VOID)SysDeinitialise(gpsSysData);
		gpsSysData = IMG_NULL;
		return eError;
	}
	SYS_SPECIFIC_DATA_SET(&gsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_LOCATEDEV);


	eError = PVRSRVRegisterDevice(gpsSysData, SGXRegisterDevice,
								  DEVICE_SGX_INTERRUPT, &gui32SGXDeviceID);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"SysInitialise: Failed to register device!"));
		(IMG_VOID)SysDeinitialise(gpsSysData);
		gpsSysData = IMG_NULL;
		return eError;
	}
	SYS_SPECIFIC_DATA_SET(&gsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_REGDEV);





	psDeviceNode = gpsSysData->psDeviceNodeList;
	while(psDeviceNode)
	{

		switch(psDeviceNode->sDevId.eDeviceType)
		{
			case PVRSRV_DEVICE_TYPE_SGX:
			{
				DEVICE_MEMORY_INFO *psDevMemoryInfo;
				DEVICE_MEMORY_HEAP_INFO *psDeviceMemoryHeap;




				psDeviceNode->psLocalDevMemArena = IMG_NULL;


				psDevMemoryInfo = &psDeviceNode->sDevMemoryInfo;
				psDeviceMemoryHeap = psDevMemoryInfo->psDeviceMemoryHeap;


				for(i=0; i<psDevMemoryInfo->ui32HeapCount; i++)
				{
					psDeviceMemoryHeap[i].ui32Attribs |= PVRSRV_BACKINGSTORE_SYSMEM_NONCONTIG;
				}

				gpsSGXDevNode = psDeviceNode;
				gsSysSpecificData.psSGXDevNode = psDeviceNode;

				break;
			}
			default:
				PVR_DPF((PVR_DBG_ERROR,"SysInitialise: Failed to find SGX device node!"));
				return PVRSRV_ERROR_INIT_FAILURE;
		}


		psDeviceNode = psDeviceNode->psNext;
	}
	eError = EnableSystemClocksWrap(gpsSysData);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"SysInitialise: Failed to Enable system clocks (%d)", eError));
		(IMG_VOID)SysDeinitialise(gpsSysData);
		gpsSysData = IMG_NULL;
		return eError;
	}
	SYS_SPECIFIC_DATA_SET(&gsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_SYSCLOCKS);
#if defined(SUPPORT_ACTIVE_POWER_MANAGEMENT)
	eError = EnableSGXClocksWrap(gpsSysData);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"SysInitialise: Failed to Enable SGX clocks (%d)", eError));
		(IMG_VOID)SysDeinitialise(gpsSysData);
		gpsSysData = IMG_NULL;
		return eError;
	}
#endif

	eError = PVRSRVInitialiseDevice(gui32SGXDeviceID);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"SysInitialise: Failed to initialise device!"));
		(IMG_VOID)SysDeinitialise(gpsSysData);
		gpsSysData = IMG_NULL;
		return eError;
	}
	SYS_SPECIFIC_DATA_SET(&gsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_INITDEV);

#if defined(SUPPORT_ACTIVE_POWER_MANAGEMENT)

	DisableSGXClocks(gpsSysData);
#endif

	return PVRSRV_OK;
}


PVRSRV_ERROR SysFinalise(IMG_VOID)
{
	PVRSRV_ERROR eError = PVRSRV_OK;

#if defined(SUPPORT_ACTIVE_POWER_MANAGEMENT)
	eError = EnableSGXClocksWrap(gpsSysData);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"SysInitialise: Failed to Enable SGX clocks (%d)", eError));
		return eError;
	}
#endif

	eError = OSInstallMISR(gpsSysData);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"SysFinalise: Failed to install MISR"));
		return eError;
	}
	SYS_SPECIFIC_DATA_SET(&gsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_MISR);

#if defined(SYS_USING_INTERRUPTS)

	eError = OSInstallDeviceLISR(gpsSysData, gsSGXDeviceMap.ui32IRQ, "SGX ISR", gpsSGXDevNode);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"SysFinalise: Failed to install ISR"));
		return eError;
	}
	SYS_SPECIFIC_DATA_SET(&gsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_LISR);
#endif


	gpsSysData->pszVersionString = SysCreateVersionString(gsSGXDeviceMap.sRegsCpuPBase);
	if (!gpsSysData->pszVersionString)
	{
		PVR_DPF((PVR_DBG_ERROR,"SysFinalise: Failed to create a system version string"));
	}
	else
	{
		PVR_DPF((PVR_DBG_WARNING, "SysFinalise: Version string: %s", gpsSysData->pszVersionString));
	}

#if defined(SUPPORT_ACTIVE_POWER_MANAGEMENT)

	DisableSGXClocks(gpsSysData);
#endif

	gpsSysSpecificData->bSGXInitComplete = IMG_TRUE;

	return eError;
}


PVRSRV_ERROR SysDeinitialise (SYS_DATA *psSysData)
{
	PVRSRV_ERROR eError;

#if defined(SYS_USING_INTERRUPTS)
	if (SYS_SPECIFIC_DATA_TEST(gpsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_LISR))
	{
		eError = OSUninstallDeviceLISR(psSysData);
		if (eError != PVRSRV_OK)
		{
			PVR_DPF((PVR_DBG_ERROR,"SysDeinitialise: OSUninstallDeviceLISR failed"));
			return eError;
		}
	}
#endif

	if (SYS_SPECIFIC_DATA_TEST(gpsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_MISR))
	{
		eError = OSUninstallMISR(psSysData);
		if (eError != PVRSRV_OK)
		{
			PVR_DPF((PVR_DBG_ERROR,"SysDeinitialise: OSUninstallMISR failed"));
			return eError;
		}
	}

	if (SYS_SPECIFIC_DATA_TEST(gpsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_INITDEV))
	{
#if defined(SUPPORT_ACTIVE_POWER_MANAGEMENT)
		PVR_ASSERT(SYS_SPECIFIC_DATA_TEST(gpsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_SYSCLOCKS));

		eError = EnableSGXClocksWrap(gpsSysData);
		if (eError != PVRSRV_OK)
		{
			PVR_DPF((PVR_DBG_ERROR,"SysDeinitialise: EnableSGXClocks failed"));
			return eError;
		}
#endif


		eError = PVRSRVDeinitialiseDevice (gui32SGXDeviceID);
		if (eError != PVRSRV_OK)
		{
			PVR_DPF((PVR_DBG_ERROR,"SysDeinitialise: failed to de-init the device"));
			return eError;
		}
	}


	if (SYS_SPECIFIC_DATA_TEST(gpsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_SYSCLOCKS))
	{
		DisableSystemClocks(gpsSysData);
	}

	if (SYS_SPECIFIC_DATA_TEST(gpsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_ENVDATA))
	{
		eError = OSDeInitEnvData(gpsSysData->pvEnvSpecificData);
		if (eError != PVRSRV_OK)
		{
			PVR_DPF((PVR_DBG_ERROR,"SysDeinitialise: failed to de-init env structure"));
			return eError;
		}
	}

	if(gpsSysData->pvSOCTimerRegisterKM)
	{
		OSUnReservePhys(gpsSysData->pvSOCTimerRegisterKM,
						4,
						PVRSRV_HAP_MULTI_PROCESS|PVRSRV_HAP_UNCACHED,
						gpsSysData->hSOCTimerRegisterOSMemHandle);
	}

	SysDeinitialiseCommon(gpsSysData);

#if defined(NO_HARDWARE)
	if(SYS_SPECIFIC_DATA_TEST(gpsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_LOCATEDEV))
	{

		OSBaseFreeContigMemory(SYS_SGX_REGS_SIZE, gsSGXRegsCPUVAddr, gsSGXDeviceMap.sRegsCpuPBase);
	}
#endif


	gpsSysSpecificData->ui32SysSpecificData = 0;
	gpsSysSpecificData->bSGXInitComplete = IMG_FALSE;

	gpsSysData = IMG_NULL;

	return PVRSRV_OK;
}


PVRSRV_ERROR SysGetDeviceMemoryMap(PVRSRV_DEVICE_TYPE	eDeviceType,
								   IMG_VOID				**ppvDeviceMap)
{

	switch(eDeviceType)
	{
		case PVRSRV_DEVICE_TYPE_SGX:
		{

			*ppvDeviceMap = (IMG_VOID*)&gsSGXDeviceMap;

			break;
		}
		default:
		{
			PVR_DPF((PVR_DBG_ERROR,"SysGetDeviceMemoryMap: unsupported device type"));
		}
	}
	return PVRSRV_OK;
}


IMG_DEV_PHYADDR SysCpuPAddrToDevPAddr(PVRSRV_DEVICE_TYPE	eDeviceType,
									  IMG_CPU_PHYADDR		CpuPAddr)
{
	IMG_DEV_PHYADDR DevPAddr;

	PVR_UNREFERENCED_PARAMETER(eDeviceType);


	DevPAddr.uiAddr = CpuPAddr.uiAddr;

	return DevPAddr;
}

IMG_CPU_PHYADDR SysSysPAddrToCpuPAddr (IMG_SYS_PHYADDR sys_paddr)
{
	IMG_CPU_PHYADDR cpu_paddr;


	cpu_paddr.uiAddr = sys_paddr.uiAddr;
	return cpu_paddr;
}

IMG_SYS_PHYADDR SysCpuPAddrToSysPAddr (IMG_CPU_PHYADDR cpu_paddr)
{
	IMG_SYS_PHYADDR sys_paddr;


	sys_paddr.uiAddr = cpu_paddr.uiAddr;
	return sys_paddr;
}


IMG_DEV_PHYADDR SysSysPAddrToDevPAddr(PVRSRV_DEVICE_TYPE eDeviceType, IMG_SYS_PHYADDR SysPAddr)
{
	IMG_DEV_PHYADDR DevPAddr;

	PVR_UNREFERENCED_PARAMETER(eDeviceType);


	DevPAddr.uiAddr = SysPAddr.uiAddr;

	return DevPAddr;
}


IMG_SYS_PHYADDR SysDevPAddrToSysPAddr(PVRSRV_DEVICE_TYPE eDeviceType, IMG_DEV_PHYADDR DevPAddr)
{
	IMG_SYS_PHYADDR SysPAddr;

	PVR_UNREFERENCED_PARAMETER(eDeviceType);


	SysPAddr.uiAddr = DevPAddr.uiAddr;

	return SysPAddr;
}


IMG_VOID SysRegisterExternalDevice(PVRSRV_DEVICE_NODE *psDeviceNode)
{
	PVR_UNREFERENCED_PARAMETER(psDeviceNode);
}


IMG_VOID SysRemoveExternalDevice(PVRSRV_DEVICE_NODE *psDeviceNode)
{
	PVR_UNREFERENCED_PARAMETER(psDeviceNode);
}


IMG_UINT32 SysGetInterruptSource(SYS_DATA			*psSysData,
								 PVRSRV_DEVICE_NODE	*psDeviceNode)
{
	PVR_UNREFERENCED_PARAMETER(psSysData);
#if defined(NO_HARDWARE)

	return 0xFFFFFFFF;
#else

	return psDeviceNode->ui32SOCInterruptBit;
#endif
}


IMG_VOID SysClearInterrupts(SYS_DATA* psSysData, IMG_UINT32 ui32ClearBits)
{
	PVR_UNREFERENCED_PARAMETER(psSysData);
	PVR_UNREFERENCED_PARAMETER(ui32ClearBits);


	OSReadHWReg(((PVRSRV_SGXDEV_INFO *)gpsSGXDevNode->pvDevice)->pvRegsBaseKM,
										EUR_CR_EVENT_HOST_CLEAR);
}


PVRSRV_ERROR SysSystemPrePowerState(PVRSRV_SYS_POWER_STATE eNewPowerState)
{
	PVRSRV_ERROR eError = PVRSRV_OK;

	if (eNewPowerState == PVRSRV_SYS_POWER_STATE_D3)
	{
		PVR_TRACE(("SysSystemPrePowerState: Entering state D3"));

#if defined(SYS_USING_INTERRUPTS)
		if (SYS_SPECIFIC_DATA_TEST(&gsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_LISR))
		{
#if defined(SYS_CUSTOM_POWERLOCK_WRAP)
			IMG_BOOL bWrapped = WrapSystemPowerChange(&gsSysSpecificData);
#endif
			eError = OSUninstallDeviceLISR(gpsSysData);
#if defined(SYS_CUSTOM_POWERLOCK_WRAP)
			if (bWrapped)
			{
				UnwrapSystemPowerChange(&gsSysSpecificData);
			}
#endif
			if (eError != PVRSRV_OK)
			{
				PVR_DPF((PVR_DBG_ERROR,"SysSystemPrePowerState: OSUninstallDeviceLISR failed (%d)", eError));
				return eError;
			}
			SYS_SPECIFIC_DATA_SET(&gsSysSpecificData, SYS_SPECIFIC_DATA_PM_UNINSTALL_LISR);
			SYS_SPECIFIC_DATA_CLEAR(&gsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_LISR);
		}
#endif

		if (SYS_SPECIFIC_DATA_TEST(&gsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_SYSCLOCKS))
		{
			DisableSystemClocks(gpsSysData);

			SYS_SPECIFIC_DATA_SET(&gsSysSpecificData, SYS_SPECIFIC_DATA_PM_DISABLE_SYSCLOCKS);
			SYS_SPECIFIC_DATA_CLEAR(&gsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_SYSCLOCKS);
		}
	}

	return eError;
}


PVRSRV_ERROR SysSystemPostPowerState(PVRSRV_SYS_POWER_STATE eNewPowerState)
{
	PVRSRV_ERROR eError = PVRSRV_OK;

	if (eNewPowerState == PVRSRV_SYS_POWER_STATE_D0)
	{
		PVR_TRACE(("SysSystemPostPowerState: Entering state D0"));

		if (SYS_SPECIFIC_DATA_TEST(&gsSysSpecificData, SYS_SPECIFIC_DATA_PM_DISABLE_SYSCLOCKS))
		{
			eError = EnableSystemClocksWrap(gpsSysData);
			if (eError != PVRSRV_OK)
			{
				PVR_DPF((PVR_DBG_ERROR,"SysSystemPostPowerState: EnableSystemClocksWrap failed (%d)", eError));
				return eError;
			}
			SYS_SPECIFIC_DATA_SET(&gsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_SYSCLOCKS);
			SYS_SPECIFIC_DATA_CLEAR(&gsSysSpecificData, SYS_SPECIFIC_DATA_PM_DISABLE_SYSCLOCKS);
		}

#if defined(SYS_USING_INTERRUPTS)
		if (SYS_SPECIFIC_DATA_TEST(&gsSysSpecificData, SYS_SPECIFIC_DATA_PM_UNINSTALL_LISR))
		{
#if defined(SYS_CUSTOM_POWERLOCK_WRAP)
			IMG_BOOL bWrapped = WrapSystemPowerChange(&gsSysSpecificData);
#endif

#if defined(SUPPORT_ACTIVE_POWER_MANAGEMENT)
			eError = EnableSGXClocksWrap(gpsSysData);
			if (eError != PVRSRV_OK)
			{
				PVR_DPF((PVR_DBG_ERROR,"SysSystemPostPowerState: Failed to Enable SGX clocks (%d)", eError));
				return eError;
			}
#endif
			eError = OSInstallDeviceLISR(gpsSysData, gsSGXDeviceMap.ui32IRQ, "SGX ISR", gpsSGXDevNode);

#if defined(SUPPORT_ACTIVE_POWER_MANAGEMENT)
			DisableSGXClocks(gpsSysData);
#endif

#if defined(SYS_CUSTOM_POWERLOCK_WRAP)
			if (bWrapped)
			{
				UnwrapSystemPowerChange(&gsSysSpecificData);
			}
#endif
			if (eError != PVRSRV_OK)
			{
				PVR_DPF((PVR_DBG_ERROR,"SysSystemPostPowerState: OSInstallDeviceLISR failed to install ISR (%d)", eError));
				return eError;
			}
			SYS_SPECIFIC_DATA_SET(&gsSysSpecificData, SYS_SPECIFIC_DATA_ENABLE_LISR);
			SYS_SPECIFIC_DATA_CLEAR(&gsSysSpecificData, SYS_SPECIFIC_DATA_PM_UNINSTALL_LISR);
		}
#endif
	}
	return eError;
}


PVRSRV_ERROR SysDevicePrePowerState(IMG_UINT32				ui32DeviceIndex,
									PVRSRV_DEV_POWER_STATE	eNewPowerState,
									PVRSRV_DEV_POWER_STATE	eCurrentPowerState)
{
	PVR_UNREFERENCED_PARAMETER(eCurrentPowerState);

	if (ui32DeviceIndex != gui32SGXDeviceID)
	{
		return PVRSRV_OK;
	}

#if defined(SUPPORT_ACTIVE_POWER_MANAGEMENT)
	if (eNewPowerState == PVRSRV_DEV_POWER_STATE_OFF)
	{
		PVR_DPF((PVR_DBG_MESSAGE, "SysDevicePrePowerState: SGX Entering state D3"));
		DisableSGXClocks(gpsSysData);
	}
#else
	PVR_UNREFERENCED_PARAMETER(eNewPowerState );
#endif
	return PVRSRV_OK;
}


PVRSRV_ERROR SysDevicePostPowerState(IMG_UINT32				ui32DeviceIndex,
									 PVRSRV_DEV_POWER_STATE	eNewPowerState,
									 PVRSRV_DEV_POWER_STATE	eCurrentPowerState)
{
	PVRSRV_ERROR eError = PVRSRV_OK;

	PVR_UNREFERENCED_PARAMETER(eNewPowerState);

	if (ui32DeviceIndex != gui32SGXDeviceID)
	{
		return eError;
	}

#if defined(SUPPORT_ACTIVE_POWER_MANAGEMENT)
	if (eCurrentPowerState == PVRSRV_DEV_POWER_STATE_OFF)
	{
		PVR_DPF((PVR_DBG_MESSAGE, "SysDevicePostPowerState: SGX Leaving state D3"));
		eError = EnableSGXClocksWrap(gpsSysData);
	}
#else
	PVR_UNREFERENCED_PARAMETER(eCurrentPowerState);
#endif

	return eError;
}


PVRSRV_ERROR SysOEMFunction (	IMG_UINT32	ui32ID,
								IMG_VOID	*pvIn,
								IMG_UINT32	ulInSize,
								IMG_VOID	*pvOut,
								IMG_UINT32	ulOutSize)
{
	PVR_UNREFERENCED_PARAMETER(ui32ID);
	PVR_UNREFERENCED_PARAMETER(pvIn);
	PVR_UNREFERENCED_PARAMETER(ulInSize);
	PVR_UNREFERENCED_PARAMETER(pvOut);
	PVR_UNREFERENCED_PARAMETER(ulOutSize);

	if ((ui32ID == OEM_GET_EXT_FUNCS) &&
		(ulOutSize == sizeof(PVRSRV_DC_OEM_JTABLE)))
	{

		PVRSRV_DC_OEM_JTABLE *psOEMJTable = (PVRSRV_DC_OEM_JTABLE*) pvOut;
		psOEMJTable->pfnOEMBridgeDispatch = &PVRSRV_BridgeDispatchKM;
		return PVRSRV_OK;
	}

	return PVRSRV_ERROR_INVALID_PARAMS;
}

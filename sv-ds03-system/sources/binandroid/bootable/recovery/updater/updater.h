/****************************************************************************

*  Copyright (C) 2015 Cambridge Silicon Radio Limited

*

*  This software is based on third party software that has been modified by CSR.

*  The underlying software was licensed by the author under separate terms,

*  as provided in the original license statement for your information only.

*

*  Notwithstanding, this software, when distributed to you by CSR, is licensed

*  to you by CSR under the terms of the proprietary CSR license under which the

*  software was delivered to you, and not under the original author’s terms.

*

****************************************************************************/

/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _UPDATER_UPDATER_H_
#define _UPDATER_UPDATER_H_

#include <stdio.h>
#include "minzip/Zip.h"

typedef struct {
    FILE* cmd_pipe;
    ZipArchive* package_zip;
    int version;
} UpdaterInfo;

#endif

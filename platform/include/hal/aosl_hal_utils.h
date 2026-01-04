/***************************************************************************
 * Module:	utils hal definitions.
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/
#ifndef __AOSL_HAL_UTILS_H__
#define __AOSL_HAL_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief get device UUID
 * @param [out] buf buffer to store UUID string
 * @param [in] buf_sz size of buffer
 * @return 0 on success, < 0 on error
 */
int aosl_hal_get_uuid (char buf [], int buf_sz);

/**
 * @brief get OS version string
 * @param [out] buf buffer to store OS version string
 * @param [in] buf_sz size of buffer
 * @return 0 on success, < 0 on error
 */
int aosl_hal_os_version (char buf [], int buf_sz);

#ifdef __cplusplus
}
#endif

#endif
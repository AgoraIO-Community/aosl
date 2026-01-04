/***************************************************************************
 * Module:	AOSL regular file operations definition header file
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/

#ifndef __AOSL_FILE_H__
#define __AOSL_FILE_H__

#include <api/aosl_types.h>
#include <api/aosl_defs.h>
#include <hal/aosl_hal_file.h>

#ifdef __cplusplus
extern "C" {
#endif

extern __aosl_api__ int aosl_mkdir(const char *path);
extern __aosl_api__ int aosl_rmdir(const char *path);
extern __aosl_api__ int aosl_fexist(const char *path);
extern __aosl_api__ int aosl_fsize(const char *path);

extern __aosl_api__ int aosl_file_create(const char *filepath);
extern __aosl_api__ int aosl_file_delete(const char *filepath);
extern __aosl_api__ int aosl_file_rename(const char *old_name, const char *new_name);

extern __aosl_api__ aosl_fs_t aosl_fopen(const char *filepath, const char *mode);
extern __aosl_api__ int aosl_fclose(aosl_fs_t fs);
extern __aosl_api__ int aosl_fread(aosl_fs_t fs, void *buf, size_t size);
extern __aosl_api__ int aosl_fwrite(aosl_fs_t fs, const void *buf, size_t size);

#ifdef __cplusplus
}
#endif


#endif /* __AOSL_FILE_H__ */
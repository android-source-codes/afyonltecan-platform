/* Copyright (C) 2001-2006 Artifex Software, Inc.
   All Rights Reserved.
  
  This file is part of GNU ghostscript

  GNU ghostscript is free software; you can redistribute it and/or
  modify it under the terms of the version 2 of the GNU General Public
  License as published by the Free Software Foundation.

  GNU ghostscript is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with
  ghostscript; see the file COPYING. If not, write to the Free Software Foundation,
  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

/* $Id: ziodev2.c,v 1.8 2008/03/23 15:27:48 Arabidopsis Exp $ */
/* (Level 2) IODevice operators */
#include "string_.h"
#include "ghost.h"
#include "gp.h"
#include "oper.h"
#include "stream.h"
#include "gxiodev.h"
#include "dstack.h"		/* for systemdict */
#include "files.h"		/* for file_open_stream */
#include "iparam.h"
#include "iutil2.h"
#include "store.h"

/* ------ %null% ------ */

/* This represents the null output file. */
static iodev_proc_open_device(null_open);
const gx_io_device gs_iodev_null = {
    "%null%", "Special",
    {
	iodev_no_init, null_open, iodev_no_open_file,
	iodev_os_fopen, iodev_os_fclose,
	iodev_no_delete_file, iodev_no_rename_file, iodev_no_file_status,
	iodev_no_enumerate_files, NULL, NULL,
	iodev_no_get_params, iodev_no_put_params
    }
};

static int
null_open(gx_io_device * iodev, const char *access, stream ** ps,
	  gs_memory_t * mem)
{
    if (!streq1(access, 'w'))
	return_error(e_invalidfileaccess);
    return file_open_stream(gp_null_file_name,
			    strlen(gp_null_file_name),
			    access, 256 /* arbitrary */ , ps,
			    iodev, iodev->procs.fopen, mem);
}

/* ------ Operators ------ */

/* <iodevice> .getdevparams <mark> <name> <value> ... */
static int
zgetdevparams(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;
    gx_io_device *iodev;
    stack_param_list list;
    gs_param_list *const plist = (gs_param_list *) & list;
    int code;
    ref *pmark;

    check_read_type(*op, t_string);
    iodev = gs_findiodevice(op->value.bytes, r_size(op));
    if (iodev == 0)
	return_error(e_undefined);
    stack_param_list_write(&list, &o_stack, NULL, iimemory);
    if ((code = gs_getdevparams(iodev, plist)) < 0) {
	ref_stack_pop(&o_stack, list.count * 2);
	return code;
    }
    pmark = ref_stack_index(&o_stack, list.count * 2);
    make_mark(pmark);
    return 0;
}

/* <mark> <name> <value> ... <iodevice> .putdevparams */
static int
zputdevparams(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;
    gx_io_device *iodev;
    stack_param_list list;
    gs_param_list *const plist = (gs_param_list *) & list;
    int code;
    password system_params_password;

    check_read_type(*op, t_string);
    iodev = gs_findiodevice(op->value.bytes, r_size(op));
    if (iodev == 0)
	return_error(e_undefined);
    code = stack_param_list_read(&list, &o_stack, 1, NULL, false, iimemory);
    if (code < 0)
	return code;
    code = dict_read_password(&system_params_password, systemdict,
			      "SystemParamsPassword");
    if (code < 0)
	return code;
    code = param_check_password(plist, &system_params_password);
    if (code != 0) {
	iparam_list_release(&list);
	return_error(code < 0 ? code : e_invalidaccess);
    }
    code = gs_putdevparams(iodev, plist);
    iparam_list_release(&list);
    if (code < 0)
	return code;
    ref_stack_pop(&o_stack, list.count * 2 + 2);
    return 0;
}

/* ------ Initialization procedure ------ */

const op_def ziodev2_l2_op_defs[] =
{
    op_def_begin_level2(),
    {"1.getdevparams", zgetdevparams},
    {"2.putdevparams", zputdevparams},
    op_def_end(0)
};

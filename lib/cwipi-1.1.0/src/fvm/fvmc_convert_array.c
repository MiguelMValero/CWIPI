/*============================================================================
 * Functions related to the transformation of data arrays for import
 * or export of meshes and fields.
 *
 * All "reasonable" combinations of datatypes are handled here.
 * (templates would be useful here; we prefer duplicating code to
 * using macros, so that the code may be followed through a debugger).
 *============================================================================*/

/*
  This file is part of the "Finite Volume Mesh" library, intended to provide
  finite volume mesh and associated fields I/O and manipulation services.

  Copyright (C) 2011       ONERA

  Copyright (C) 2004-2006  EDF

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*----------------------------------------------------------------------------
 * Standard C library headers
 *----------------------------------------------------------------------------*/

#include <assert.h>

/*----------------------------------------------------------------------------
 * BFT library headers
 *----------------------------------------------------------------------------*/

#include <bftc_error.h>

/*----------------------------------------------------------------------------
 *  Local headers
 *----------------------------------------------------------------------------*/

#include "fvmc_config_defs.h"
#include "fvmc_defs.h"
#include "fvmc_writer.h"

/*----------------------------------------------------------------------------
 *  Header for the current file
 *----------------------------------------------------------------------------*/

#include "fvmc_convert_array.h"

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#if 0
} /* Fake brace to force Emacs auto-indentation back to column 0 */
#endif
#endif /* __cplusplus */

/*============================================================================
 * Local structure definitions
 *============================================================================*/

/*=============================================================================
 * Private function definitions
 *============================================================================*/

/*----------------------------------------------------------------------------
 * Convert an array representation of floats to floats, with possible
 * indirection, interlacing, de-interlacing, or change of data dimension.
 *
 * parameters:
 *   src_dim          <-- dimension of source data
 *   src_dim_shift    <-- source data dimension shift (start index)
 *   dest_dim         <-- destination data dimension (1 if non interlaced)
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        source dimension if non interlaced, times one per
 *                        parent list if multiple parent lists, with
 *                        x_parent_1, y_parent_1, ..., x_parent_2, ...) order
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_float_to_float(const int                     src_dim,
                              const int                     src_dim_shift,
                              const int                     dest_dim,
                              const fvmc_lnum_t              src_idx_start,
                              const fvmc_lnum_t              src_idx_end,
                              const fvmc_interlace_t         src_interlace,
                              const int                     n_parent_lists,
                              const fvmc_lnum_t              parent_num_shift[],
                              const fvmc_lnum_t              parent_num[],
                              const float            *const src_data[],
                              float                  *const dest_data)
{
  int  pl;
  size_t  i, k, l, min_dim;
  fvmc_lnum_t  j, parent_id;

  min_dim = (size_t)(FVMC_MIN((src_dim - src_dim_shift), dest_dim));

  /* If source data is interlaced */

  if (src_interlace == FVMC_INTERLACE) {

    if (n_parent_lists == 0) {
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = src_data[0][j*src_dim + l];
      }
    }
    else if (parent_num != NULL) {
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
             parent_id < parent_num_shift[pl] ;
             pl--);
        assert(pl > -1);
        parent_id -= parent_num_shift[pl];
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = src_data[pl][parent_id*src_dim + l];
      }
    }
    else { /* parent_num == NULL: implicit parent numbering */
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (parent_id = j, pl = n_parent_lists - 1 ;
             parent_id < parent_num_shift[pl] ;
             pl--);
        assert(pl > -1);
        parent_id -= parent_num_shift[pl];
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = src_data[pl][parent_id*src_dim + l];
      }
    }

  }

  /* If source data is not interlaced */

  else { /* if (src_interlace == FVMC_NO_INTERLACE) */

    if (n_parent_lists == 0) {
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k] = src_data[l][j];
      }
    }
    else if (parent_num != NULL) {
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
             parent_id < parent_num_shift[pl] ;
             pl--);
        assert(pl > -1);
        parent_id -= parent_num_shift[pl];
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = src_data[src_dim*pl + l][parent_id];
      }
    }
    else { /* parent_num == NULL: implicit parent numbering */
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (parent_id = j, pl = n_parent_lists - 1 ;
             parent_id < parent_num_shift[pl] ;
             pl--);
        assert(pl > -1);
        parent_id -= parent_num_shift[pl];
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = src_data[src_dim*pl + l][parent_id];
      }
    }

  }

  /* Complete with zeroes if necessary */

  if (min_dim < (size_t)dest_dim) {
    for (i = 0, j = src_idx_start ;
         j < src_idx_end ;
         i++, j++) {
      for (k = min_dim ; k < (size_t)dest_dim ; k++)
        dest_data[i*dest_dim + k] = 0.0;
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of floats to doubles, with possible
 * indirection, interlacing, de-interlacing, or change of data dimension.
 *
 * parameters:
 *   src_dim          <-- dimension of source data
 *   src_dim_shift    <-- source data dimension shift (start index)
 *   dest_dim         <-- destination data dimension (1 if non interlaced)
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        source dimension if non interlaced, times one per
 *                        parent list if multiple parent lists, with
 *                        x_parent_1, y_parent_1, ..., x_parent_2, ...) order
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_float_to_double(const int                     src_dim,
                               const int                     src_dim_shift,
                               const int                     dest_dim,
                               const fvmc_lnum_t              src_idx_start,
                               const fvmc_lnum_t              src_idx_end,
                               const fvmc_interlace_t         src_interlace,
                               const int                     n_parent_lists,
                               const fvmc_lnum_t              parent_num_shift[],
                               const fvmc_lnum_t              parent_num[],
                               const float            *const src_data[],
                               double                 *const dest_data)
{
  int  pl;
  size_t  i, k, l, min_dim;
  fvmc_lnum_t  j, parent_id;

  min_dim = (size_t)(FVMC_MIN((src_dim - src_dim_shift), dest_dim));

  /* If source data is interlaced */

  if (src_interlace == FVMC_INTERLACE) {

    if (n_parent_lists == 0) {
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = src_data[0][j*src_dim + l];
      }
    }
    else if (parent_num != NULL) {
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
             parent_id < parent_num_shift[pl] ;
             pl--);
        assert(pl > -1);
        parent_id -= parent_num_shift[pl];
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = src_data[pl][parent_id*src_dim + l];
      }
    }
    else { /* parent_num == NULL: implicit parent numbering */
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (parent_id = j, pl = n_parent_lists - 1 ;
             parent_id < parent_num_shift[pl] ;
             pl--);
        assert(pl > -1);
        parent_id -= parent_num_shift[pl];
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = src_data[pl][parent_id*src_dim + l];
      }
    }

  }

  /* If source data is not interlaced */

  else { /* if (src_interlace == FVMC_NO_INTERLACE) */

    if (n_parent_lists == 0) {
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k] = src_data[l][j];
      }
    }
    else if (parent_num != NULL) {
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
             parent_id < parent_num_shift[pl] ;
             pl--);
        assert(pl > -1);
        parent_id -= parent_num_shift[pl];
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = src_data[src_dim*pl + l][parent_id];
      }
    }
    else { /* parent_num == NULL: implicit parent numbering */
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (parent_id = j, pl = n_parent_lists - 1 ;
             parent_id < parent_num_shift[pl] ;
             pl--);
        assert(pl > -1);
        parent_id -= parent_num_shift[pl];
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = src_data[src_dim*pl + l][parent_id];
      }
    }

  }

  /* Complete with zeroes if necessary */

  if (min_dim < (size_t)dest_dim) {
    for (i = 0, j = src_idx_start ;
         j < src_idx_end ;
         i++, j++) {
      for (k = min_dim ; k < (size_t)dest_dim ; k++)
        dest_data[i*dest_dim + k] = 0.0;
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of doubles to floats, with possible
 * indirection, interlacing, de-interlacing, or change of data dimension.
 *
 * parameters:
 *   src_dim          <-- dimension of source data
 *   src_dim_shift    <-- source data dimension shift (start index)
 *   dest_dim         <-- destination data dimension (1 if non interlaced)
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        source dimension if non interlaced, times one per
 *                        parent list if multiple parent lists, with
 *                        x_parent_1, y_parent_1, ..., x_parent_2, ...) order
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_double_to_float(const int                     src_dim,
                               const int                     src_dim_shift,
                               const int                     dest_dim,
                               const fvmc_lnum_t              src_idx_start,
                               const fvmc_lnum_t              src_idx_end,
                               const fvmc_interlace_t         src_interlace,
                               const int                     n_parent_lists,
                               const fvmc_lnum_t              parent_num_shift[],
                               const fvmc_lnum_t              parent_num[],
                               const double           *const src_data[],
                               float                  *const dest_data)
{
  int  pl;
  size_t  i, k, l, min_dim;
  fvmc_lnum_t  j, parent_id;

  min_dim = (size_t)(FVMC_MIN((src_dim - src_dim_shift), dest_dim));

  /* If source data is interlaced */

  if (src_interlace == FVMC_INTERLACE) {

    if (n_parent_lists == 0) {
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = (float) src_data[0][j*src_dim + l];
      }
    }
    else if (parent_num != NULL) {
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
             parent_id < parent_num_shift[pl] ;
             pl--);
        assert(pl > -1);
        parent_id -= parent_num_shift[pl];
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = (float) src_data[pl][parent_id*src_dim + l];
      }
    }
    else { /* parent_num == NULL: implicit parent numbering */
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (parent_id = j, pl = n_parent_lists - 1 ;
             parent_id < parent_num_shift[pl] ;
             pl--);
        assert(pl > -1);
        parent_id -= parent_num_shift[pl];
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = (float) src_data[pl][parent_id*src_dim + l];
      }
    }

  }

  /* If source data is not interlaced */

  else { /* if (src_interlace == FVMC_NO_INTERLACE) */

    if (n_parent_lists == 0) {
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k] = (float) src_data[l][j];
      }
    }
    else if (parent_num != NULL) {
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
             parent_id < parent_num_shift[pl] ;
             pl--);
        assert(pl > -1);
        parent_id -= parent_num_shift[pl];
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = (float) src_data[src_dim*pl + l][parent_id];
      }
    }
    else { /* parent_num == NULL: implicit parent numbering */
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (parent_id = j, pl = n_parent_lists - 1 ;
             parent_id < parent_num_shift[pl] ;
             pl--);
        assert(pl > -1);
        parent_id -= parent_num_shift[pl];
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = (float) src_data[src_dim*pl + l][parent_id];
      }
    }

  }

  /* Complete with zeroes if necessary */

  if (min_dim < (size_t)dest_dim) {
    for (i = 0, j = src_idx_start ;
         j < src_idx_end ;
         i++, j++) {
      for (k = min_dim ; k < (size_t)dest_dim ; k++)
        dest_data[i*dest_dim + k] = 0.0;
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of doubles to doubles, with possible
 * indirection, interlacing, de-interlacing, or change of data dimension.
 *
 * parameters:
 *   src_dim          <-- dimension of source data
 *   src_dim_shift    <-- source data dimension shift (start index)
 *   dest_dim         <-- destination data dimension (1 if non interlaced)
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        source dimension if non interlaced, times one per
 *                        parent list if multiple parent lists, with
 *                        x_parent_1, y_parent_1, ..., x_parent_2, ...) order
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_double_to_double(const int                     src_dim,
                                const int                     src_dim_shift,
                                const int                     dest_dim,
                                const fvmc_lnum_t              src_idx_start,
                                const fvmc_lnum_t              src_idx_end,
                                const fvmc_interlace_t         src_interlace,
                                const int                     n_parent_lists,
                                const fvmc_lnum_t              parent_num_shift[],
                                const fvmc_lnum_t              parent_num[],
                                const double           *const src_data[],
                                double                 *const dest_data)
{
  int  pl;
  size_t  i, k, l, min_dim;
  fvmc_lnum_t  j, parent_id;

  min_dim = (size_t)(FVMC_MIN((src_dim - src_dim_shift), dest_dim));

  /* If source data is interlaced */

  if (src_interlace == FVMC_INTERLACE) {

    if (n_parent_lists == 0) {
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = src_data[0][j*src_dim + l];
      }
    }
    else if (parent_num != NULL) {
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
             parent_id < parent_num_shift[pl] ;
             pl--);
        assert(pl > -1);
        parent_id -= parent_num_shift[pl];
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = src_data[pl][parent_id*src_dim + l];
      }
    }
    else { /* parent_num == NULL: implicit parent numbering */
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (parent_id = j, pl = n_parent_lists - 1 ;
             parent_id < parent_num_shift[pl] ;
             pl--);
        assert(pl > -1);
        parent_id -= parent_num_shift[pl];
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = src_data[pl][parent_id*src_dim + l];
      }
    }

  }

  /* If source data is not interlaced */

  else { /* if (src_interlace == FVMC_NO_INTERLACE) */

    if (n_parent_lists == 0) {
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k] = src_data[l][j];
      }
    }
    else if (parent_num != NULL) {
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
             parent_id < parent_num_shift[pl] ;
             pl--);
        assert(pl > -1);
        parent_id -= parent_num_shift[pl];
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = src_data[src_dim*pl + l][parent_id];
      }
    }
    else { /* parent_num == NULL: implicit parent numbering */
      for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
        for (parent_id = j, pl = n_parent_lists - 1 ;
             parent_id < parent_num_shift[pl] ;
             pl--);
        assert(pl > -1);
        parent_id -= parent_num_shift[pl];
        for (k = 0, l = src_dim_shift ; k < min_dim ; k++, l++)
          dest_data[i*dest_dim + k]
            = src_data[src_dim*pl + l][parent_id];
      }
    }

  }

  /* Complete with zeroes if necessary */

  if (min_dim < (size_t)dest_dim) {
    for (i = 0, j = src_idx_start ;
         j < src_idx_end ;
         i++, j++) {
      for (k = min_dim ; k < (size_t)dest_dim ; k++)
        dest_data[i*dest_dim + k] = 0.0;
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of int32 to floats, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_int32_to_float(const fvmc_lnum_t              src_idx_start,
                              const fvmc_lnum_t              src_idx_end,
                              const int                     n_parent_lists,
                              const fvmc_lnum_t              parent_num_shift[],
                              const fvmc_lnum_t              parent_num[],
                              const int32_t          *const src_data[],
                              float                  *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = (float) src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = (float) src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = (float) src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to doubles, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_int32_to_double(const fvmc_lnum_t              src_idx_start,
                               const fvmc_lnum_t              src_idx_end,
                               const int                     n_parent_lists,
                               const fvmc_lnum_t              parent_num_shift[],
                               const fvmc_lnum_t              parent_num[],
                               const int32_t          *const src_data[],
                               double                 *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to ints, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_int32_to_int32(const fvmc_lnum_t              src_idx_start,
                              const fvmc_lnum_t              src_idx_end,
                              const int                     n_parent_lists,
                              const fvmc_lnum_t              parent_num_shift[],
                              const fvmc_lnum_t              parent_num[],
                              const int32_t          *const src_data[],
                              int32_t                *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to ints, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_int32_to_int64(const fvmc_lnum_t              src_idx_start,
                              const fvmc_lnum_t              src_idx_end,
                              const int                     n_parent_lists,
                              const fvmc_lnum_t              parent_num_shift[],
                              const fvmc_lnum_t              parent_num[],
                              const int32_t          *const src_data[],
                              int64_t                *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to ints, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_int32_to_uint32(const fvmc_lnum_t              src_idx_start,
                               const fvmc_lnum_t              src_idx_end,
                               const int                     n_parent_lists,
                               const fvmc_lnum_t              parent_num_shift[],
                               const fvmc_lnum_t              parent_num[],
                               const int32_t          *const src_data[],
                               uint32_t               *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to ints, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_int32_to_uint64(const fvmc_lnum_t              src_idx_start,
                               const fvmc_lnum_t              src_idx_end,
                               const int                     n_parent_lists,
                               const fvmc_lnum_t              parent_num_shift[],
                               const fvmc_lnum_t              parent_num[],
                               const int32_t          *const src_data[],
                               uint64_t               *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of int64 to floats, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_int64_to_float(const fvmc_lnum_t              src_idx_start,
                              const fvmc_lnum_t              src_idx_end,
                              const int                     n_parent_lists,
                              const fvmc_lnum_t              parent_num_shift[],
                              const fvmc_lnum_t              parent_num[],
                              const int64_t          *const src_data[],
                              float                  *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = (float) src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = (float) src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = (float) src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to doubles, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_int64_to_double(const fvmc_lnum_t              src_idx_start,
                               const fvmc_lnum_t              src_idx_end,
                               const int                     n_parent_lists,
                               const fvmc_lnum_t              parent_num_shift[],
                               const fvmc_lnum_t              parent_num[],
                               const int64_t          *const src_data[],
                               double                 *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = (double) src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = (double) src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = (double) src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to ints, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_int64_to_int32(const fvmc_lnum_t              src_idx_start,
                              const fvmc_lnum_t              src_idx_end,
                              const int                     n_parent_lists,
                              const fvmc_lnum_t              parent_num_shift[],
                              const fvmc_lnum_t              parent_num[],
                              const int64_t          *const src_data[],
                              int32_t                *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to ints, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_int64_to_int64(const fvmc_lnum_t              src_idx_start,
                              const fvmc_lnum_t              src_idx_end,
                              const int                     n_parent_lists,
                              const fvmc_lnum_t              parent_num_shift[],
                              const fvmc_lnum_t              parent_num[],
                              const int64_t          *const src_data[],
                              int64_t                *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to ints, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_int64_to_uint32(const fvmc_lnum_t              src_idx_start,
                               const fvmc_lnum_t              src_idx_end,
                               const int                     n_parent_lists,
                               const fvmc_lnum_t              parent_num_shift[],
                               const fvmc_lnum_t              parent_num[],
                               const int64_t          *const src_data[],
                               uint32_t               *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to ints, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_int64_to_uint64(const fvmc_lnum_t              src_idx_start,
                               const fvmc_lnum_t              src_idx_end,
                               const int                     n_parent_lists,
                               const fvmc_lnum_t              parent_num_shift[],
                               const fvmc_lnum_t              parent_num[],
                               const int64_t          *const src_data[],
                               uint64_t               *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to floats, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_uint32_to_float(const fvmc_lnum_t              src_idx_start,
                               const fvmc_lnum_t              src_idx_end,
                               const int                     n_parent_lists,
                               const fvmc_lnum_t              parent_num_shift[],
                               const fvmc_lnum_t              parent_num[],
                               const uint32_t         *const src_data[],
                               float                  *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = (float) src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = (float) src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = (float) src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to doubles, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_uint32_to_double(const fvmc_lnum_t              src_idx_start,
                                const fvmc_lnum_t              src_idx_end,
                                const int                     n_parent_lists,
                                const fvmc_lnum_t              parent_num_shift[],
                                const fvmc_lnum_t              parent_num[],
                                const uint32_t         *const src_data[],
                                double                 *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to ints, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_uint32_to_int32(const fvmc_lnum_t              src_idx_start,
                               const fvmc_lnum_t              src_idx_end,
                               const int                     n_parent_lists,
                               const fvmc_lnum_t              parent_num_shift[],
                               const fvmc_lnum_t              parent_num[],
                               const uint32_t         *const src_data[],
                               int32_t                *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to ints, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_uint32_to_int64(const fvmc_lnum_t              src_idx_start,
                               const fvmc_lnum_t              src_idx_end,
                               const int                     n_parent_lists,
                               const fvmc_lnum_t              parent_num_shift[],
                               const fvmc_lnum_t              parent_num[],
                               const uint32_t         *const src_data[],
                               int64_t                *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to ints, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_uint32_to_uint32(const fvmc_lnum_t              src_idx_start,
                                const fvmc_lnum_t              src_idx_end,
                                const int                     n_parent_lists,
                                const fvmc_lnum_t              parent_num_shift[],
                                const fvmc_lnum_t              parent_num[],
                                const uint32_t         *const src_data[],
                                uint32_t               *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to ints, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_uint32_to_uint64(const fvmc_lnum_t              src_idx_start,
                                const fvmc_lnum_t              src_idx_end,
                                const int                     n_parent_lists,
                                const fvmc_lnum_t              parent_num_shift[],
                                const fvmc_lnum_t              parent_num[],
                                const uint32_t         *const src_data[],
                                uint64_t               *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to floats, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_uint64_to_float(const fvmc_lnum_t              src_idx_start,
                               const fvmc_lnum_t              src_idx_end,
                               const int                     n_parent_lists,
                               const fvmc_lnum_t              parent_num_shift[],
                               const fvmc_lnum_t              parent_num[],
                               const uint64_t         *const src_data[],
                               float                  *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = (float) src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = (float) src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = (float) src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to doubles, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_uint64_to_double(const fvmc_lnum_t              src_idx_start,
                                const fvmc_lnum_t              src_idx_end,
                                const int                     n_parent_lists,
                                const fvmc_lnum_t              parent_num_shift[],
                                const fvmc_lnum_t              parent_num[],
                                const uint64_t         *const src_data[],
                                double                 *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = (double) src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = (double) src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = (double) src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to ints, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_uint64_to_int32(const fvmc_lnum_t              src_idx_start,
                               const fvmc_lnum_t              src_idx_end,
                               const int                     n_parent_lists,
                               const fvmc_lnum_t              parent_num_shift[],
                               const fvmc_lnum_t              parent_num[],
                               const uint64_t         *const src_data[],
                               int32_t                *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to ints, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_uint64_to_int64(const fvmc_lnum_t              src_idx_start,
                               const fvmc_lnum_t              src_idx_end,
                               const int                     n_parent_lists,
                               const fvmc_lnum_t              parent_num_shift[],
                               const fvmc_lnum_t              parent_num[],
                               const uint64_t         *const src_data[],
                               int64_t                *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to ints, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_uint64_to_uint32(const fvmc_lnum_t              src_idx_start,
                                const fvmc_lnum_t              src_idx_end,
                                const int                     n_parent_lists,
                                const fvmc_lnum_t              parent_num_shift[],
                                const fvmc_lnum_t              parent_num[],
                                const uint64_t         *const src_data[],
                                uint32_t               *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }

}

/*----------------------------------------------------------------------------
 * Convert an array representation of ints to ints, with possible
 * indirection.
 *
 * parameters:
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent list to common number index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        parent list if multiple parent lists)
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

static void
_convert_array_uint64_to_uint64(const fvmc_lnum_t              src_idx_start,
                                const fvmc_lnum_t              src_idx_end,
                                const int                     n_parent_lists,
                                const fvmc_lnum_t              parent_num_shift[],
                                const fvmc_lnum_t              parent_num[],
                                const uint64_t         *const src_data[],
                                uint64_t               *const dest_data)
{
  int  pl;
  size_t  i;
  fvmc_lnum_t  j, parent_id;

  if (n_parent_lists == 0) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++)
      dest_data[i] = src_data[0][j];
  }
  else if (parent_num != NULL) {
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = parent_num[j] - 1, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }
  else { /* parent_num == NULL: implicit parent numbering */
    for (i = 0, j = src_idx_start ; j < src_idx_end ; i++, j++) {
      for (parent_id = j, pl = n_parent_lists - 1 ;
           parent_id < parent_num_shift[pl] ;
           pl--);
      assert(pl > -1);
      parent_id -= parent_num_shift[pl];
      dest_data[i] = src_data[pl][parent_id];
    }
  }

}

/*=============================================================================
 * Public function definitions
 *============================================================================*/

/*----------------------------------------------------------------------------
 * Convert an array representation of one type to that of another type, with
 * possible indirection, interlacing, de-interlacing, or change of data
 * dimension (i.e. projection or filling extra dimension with zeroes).
 *
 * Floating point (real or double) source and destination arrays may be
 * multidimensional (interlaced or not), but with an integer type
 * for source or destination, only 1-D arrays are allowed (no use for
 * integer "vector fields" being currently required or apparent).
 *
 * Integer type destination arrays may be converted to floating point
 * (for output formats with no integer datatype, such as EnSight),
 * but floating point values may not be converted to integer values
 * (no use for this operation being currently apparent).
 *
 * parameters:
 *   src_dim          <-- dimension of source data
 *   src_dim_shift    <-- source data dimension shift (start index)
 *   dest_dim         <-- destination data dimension (1 if non interlaced)
 *   src_idx_start    <-- start index in source data
 *   src_idx_end      <-- past-the-end index in source data
 *   src_interlace    <-- indicates if source data is interlaced
 *   src_datatype     <-- source data type (float, double, or int)
 *   dest_datatype    <-- destination data type (float, double, or int)
 *   n_parent_lists   <-- number of parent lists (if parent_num != NULL)
 *   parent_num_shift <-- parent number to value array index shifts;
 *                        size: n_parent_lists
 *   parent_num       <-- if n_parent_lists > 0, parent entity numbers
 *   src_data         <-- array of source arrays (at least one, with one per
 *                        source dimension if non interlaced, times one per
 *                        parent list if multiple parent lists, with
 *                        x_parent_1, y_parent_1, ..., x_parent_2, ...) order
 *   dest_data        --> destination buffer
 *----------------------------------------------------------------------------*/

void
fvmc_convert_array(const int                     src_dim,
                  const int                     src_dim_shift,
                  const int                     dest_dim,
                  const fvmc_lnum_t              src_idx_start,
                  const fvmc_lnum_t              src_idx_end,
                  const fvmc_interlace_t         src_interlace,
                  const fvmc_datatype_t          src_datatype,
                  const fvmc_datatype_t          dest_datatype,
                  const int                     n_parent_lists,
                  const fvmc_lnum_t              parent_num_shift[],
                  const fvmc_lnum_t              parent_num[],
                  const void             *const src_data[],
                  void                   *const dest_data)
{
  assert(src_dim_shift <= src_dim);

  switch(src_datatype) {

  case FVMC_FLOAT:  /* float source datatype */

    switch(dest_datatype) {

    case FVMC_FLOAT:
      _convert_array_float_to_float(src_dim,
                                    src_dim_shift,
                                    dest_dim,
                                    src_idx_start,
                                    src_idx_end,
                                    src_interlace,
                                    n_parent_lists,
                                    parent_num_shift,
                                    parent_num,
                                    (const float **)src_data,
                                    (float *) dest_data);
      break;

    case FVMC_DOUBLE:
      _convert_array_float_to_double(src_dim,
                                     src_dim_shift,
                                     dest_dim,
                                     src_idx_start,
                                     src_idx_end,
                                     src_interlace,
                                     n_parent_lists,
                                     parent_num_shift,
                                     parent_num,
                                     (const float **)src_data,
                                     (double *) dest_data);
      break;

    case FVMC_INT16:
    case FVMC_UINT16:
    case FVMC_INT32:
    case FVMC_INT64:
    case FVMC_UINT32:
    case FVMC_UINT64:
      bftc_error(__FILE__, __LINE__, 0,
                _("fvmc_writer_convert_array() may not be used to convert "
                  "float to int32, int64, uint32, or uint64"));
      break;

    case FVMC_CHAR:
    case FVMC_UCHAR:
    case FVMC_DATATYPE_NULL:
      break;

    }
    break;

  case FVMC_DOUBLE:   /* double source datatype */

    switch(dest_datatype) {

    case FVMC_FLOAT:
      _convert_array_double_to_float(src_dim,
                                     src_dim_shift,
                                     dest_dim,
                                     src_idx_start,
                                     src_idx_end,
                                     src_interlace,
                                     n_parent_lists,
                                     parent_num_shift,
                                     parent_num,
                                     (const double **)src_data,
                                     (float *) dest_data);
      break;

    case FVMC_DOUBLE:
      _convert_array_double_to_double(src_dim,
                                      src_dim_shift,
                                      dest_dim,
                                      src_idx_start,
                                      src_idx_end,
                                      src_interlace,
                                      n_parent_lists,
                                      parent_num_shift,
                                      parent_num,
                                      (const double **)src_data,
                                      (double *) dest_data);
      break;

    case FVMC_INT16:
    case FVMC_UINT16:
    case FVMC_INT32:
    case FVMC_INT64:
    case FVMC_UINT32:
    case FVMC_UINT64:
      bftc_error(__FILE__, __LINE__, 0,
                _("fvmc_writer_convert_array() may not be used to convert "
                  "double to int32, int64, uint32, or uint64"));
      break;

    case FVMC_CHAR:
    case FVMC_UCHAR:
    case FVMC_DATATYPE_NULL:
      break;

    }
    break;

  case FVMC_INT32:  /* int32 source datatype */

    if (src_dim > 1 || dest_dim > 1)
      bftc_error(__FILE__, __LINE__, 0,
                _("fvmc_writer_convert_array() requires "
                  "src_dim = 1 and dest_dim = 1 \n"
                  "with integer source data (and not %d and %d)"),
                src_dim, dest_dim);

    switch(dest_datatype) {

    case FVMC_FLOAT:
      _convert_array_int32_to_float(src_idx_start,
                                    src_idx_end,
                                    n_parent_lists,
                                    parent_num_shift,
                                    parent_num,
                                    (const int32_t **)src_data,
                                    (float *) dest_data);
      break;

    case FVMC_DOUBLE:
      _convert_array_int32_to_double(src_idx_start,
                                     src_idx_end,
                                     n_parent_lists,
                                     parent_num_shift,
                                     parent_num,
                                     (const int32_t **)src_data,
                                     (double *) dest_data);
      break;

    case FVMC_INT32:
      _convert_array_int32_to_int32(src_idx_start,
                                    src_idx_end,
                                    n_parent_lists,
                                    parent_num_shift,
                                    parent_num,
                                    (const int32_t **) src_data,
                                    (int32_t *) dest_data);
      break;

    case FVMC_INT64:
      _convert_array_int32_to_int64(src_idx_start,
                                    src_idx_end,
                                    n_parent_lists,
                                    parent_num_shift,
                                    parent_num,
                                    (const int32_t **)src_data,
                                    (int64_t *) dest_data);
      break;

    case FVMC_UINT32:
      _convert_array_int32_to_uint32(src_idx_start,
                                     src_idx_end,
                                     n_parent_lists,
                                     parent_num_shift,
                                     parent_num,
                                     (const int32_t **)src_data,
                                     (uint32_t *) dest_data);
      break;

    case FVMC_UINT64:
      _convert_array_int32_to_uint64(src_idx_start,
                                     src_idx_end,
                                     n_parent_lists,
                                     parent_num_shift,
                                     parent_num,
                                     (const int32_t **)src_data,
                                     (uint64_t *) dest_data);
      break;

    case FVMC_CHAR:
    case FVMC_UCHAR:
    case FVMC_DATATYPE_NULL:
      break;
		case FVMC_INT16:
    case FVMC_UINT16:
      bftc_error(__FILE__, __LINE__, 0,
                _("fvmc_writer_convert_array() may not be used to convert "
                  "from int16 or uint16"));

    }
    break;

  case FVMC_INT64:  /* int64 source datatype */

    if (src_dim > 1 || dest_dim > 1)
      bftc_error(__FILE__, __LINE__, 0,
                _("fvmc_writer_convert_array() requires "
                  "src_dim = 1 and dest_dim = 1 \n"
                  "with integer source data (and not %d and %d)"),
                src_dim, dest_dim);

    switch(dest_datatype) {

    case FVMC_FLOAT:
      _convert_array_int64_to_float(src_idx_start,
                                    src_idx_end,
                                    n_parent_lists,
                                    parent_num_shift,
                                    parent_num,
                                    (const int64_t **)src_data,
                                    (float *) dest_data);
      break;

    case FVMC_DOUBLE:
      _convert_array_int64_to_double(src_idx_start,
                                     src_idx_end,
                                     n_parent_lists,
                                     parent_num_shift,
                                     parent_num,
                                     (const int64_t **)src_data,
                                     (double *) dest_data);
      break;

    case FVMC_INT32:
      _convert_array_int64_to_int32(src_idx_start,
                                    src_idx_end,
                                    n_parent_lists,
                                    parent_num_shift,
                                    parent_num,
                                    (const int64_t **)src_data,
                                    (int32_t *) dest_data);
      break;

    case FVMC_INT64:
      _convert_array_int64_to_int64(src_idx_start,
                                    src_idx_end,
                                    n_parent_lists,
                                    parent_num_shift,
                                    parent_num,
                                    (const int64_t **)src_data,
                                    (int64_t *) dest_data);
      break;

    case FVMC_UINT32:
      _convert_array_int64_to_uint32(src_idx_start,
                                     src_idx_end,
                                     n_parent_lists,
                                     parent_num_shift,
                                     parent_num,
                                     (const int64_t **)src_data,
                                     (uint32_t *) dest_data);
      break;

    case FVMC_UINT64:
      _convert_array_int64_to_uint64(src_idx_start,
                                     src_idx_end,
                                     n_parent_lists,
                                     parent_num_shift,
                                     parent_num,
                                     (const int64_t **)src_data,
                                     (uint64_t *)dest_data);
      break;

		case FVMC_INT16:
    case FVMC_UINT16:
      bftc_error(__FILE__, __LINE__, 0,
                _("fvmc_writer_convert_array() may not be used to convert "
                  "to int16 or uint16"));
      break;
	
    case FVMC_CHAR:
    case FVMC_UCHAR:
    case FVMC_DATATYPE_NULL:
      break;

    }
    break;

  case FVMC_UINT32:  /* unsigned int32 source datatype */

    if (src_dim > 1 || dest_dim > 1)
      bftc_error(__FILE__, __LINE__, 0,
                _("fvmc_writer_convert_array() requires "
                  "src_dim = 1 and dest_dim = 1 \n"
                  "with integer source data (and not %d and %d)"),
                src_dim, dest_dim);

    switch(dest_datatype) {

    case FVMC_FLOAT:
      _convert_array_uint32_to_float(src_idx_start,
                                     src_idx_end,
                                     n_parent_lists,
                                     parent_num_shift,
                                     parent_num,
                                     (const uint32_t **)src_data,
                                     (float *) dest_data);
      break;

    case FVMC_DOUBLE:
      _convert_array_uint32_to_double(src_idx_start,
                                      src_idx_end,
                                      n_parent_lists,
                                      parent_num_shift,
                                      parent_num,
                                      (const uint32_t **)src_data,
                                      (double *) dest_data);
      break;

    case FVMC_INT32:
      _convert_array_uint32_to_int32(src_idx_start,
                                     src_idx_end,
                                     n_parent_lists,
                                     parent_num_shift,
                                     parent_num,
                                     (const uint32_t **)src_data,
                                     (int32_t *) dest_data);
      break;

    case FVMC_INT64:
      _convert_array_uint32_to_int64(src_idx_start,
                                     src_idx_end,
                                     n_parent_lists,
                                     parent_num_shift,
                                     parent_num,
                                     (const uint32_t **)src_data,
                                     (int64_t *) dest_data);
      break;

    case FVMC_UINT32:
      _convert_array_uint32_to_uint32(src_idx_start,
                                      src_idx_end,
                                      n_parent_lists,
                                      parent_num_shift,
                                      parent_num,
                                      (const uint32_t **)src_data,
                                      (uint32_t *) dest_data);
      break;

    case FVMC_UINT64:
      _convert_array_uint32_to_uint64(src_idx_start,
                                      src_idx_end,
                                      n_parent_lists,
                                      parent_num_shift,
                                      parent_num,
                                      (const uint32_t **)src_data,
                                      (uint64_t *) dest_data);
      break;

    case FVMC_CHAR:
    case FVMC_UCHAR:
    case FVMC_DATATYPE_NULL:
      break;
		case FVMC_INT16:
    case FVMC_UINT16:
      bftc_error(__FILE__, __LINE__, 0,
                _("fvmc_writer_convert_array() may not be used to convert "
                  "to int16 or uint16"));

    }
    break;

  case FVMC_UINT64:  /* unsigned int64 source datatype */

    if (src_dim > 1 || dest_dim > 1)
      bftc_error(__FILE__, __LINE__, 0,
                _("fvmc_writer_convert_array() requires "
                  "src_dim = 1 and dest_dim = 1 \n"
                  "with integer source data (and not %d and %d)"),
                src_dim, dest_dim);

    switch(dest_datatype) {

    case FVMC_FLOAT:
      _convert_array_uint64_to_float(src_idx_start,
                                     src_idx_end,
                                     n_parent_lists,
                                     parent_num_shift,
                                     parent_num,
                                     (const uint64_t **)src_data,
                                     (float *) dest_data);
      break;

    case FVMC_DOUBLE:
      _convert_array_uint64_to_double(src_idx_start,
                                      src_idx_end,
                                      n_parent_lists,
                                      parent_num_shift,
                                      parent_num,
                                      (const uint64_t **)src_data,
                                      (double *) dest_data);
      break;

    case FVMC_INT32:
      _convert_array_uint64_to_int32(src_idx_start,
                                     src_idx_end,
                                     n_parent_lists,
                                     parent_num_shift,
                                     parent_num,
                                     (const uint64_t **)src_data,
                                     (int32_t *) dest_data);
      break;

    case FVMC_INT64:
      _convert_array_uint64_to_int64(src_idx_start,
                                     src_idx_end,
                                     n_parent_lists,
                                     parent_num_shift,
                                     parent_num,
                                     (const uint64_t **)src_data,
                                     (int64_t *) dest_data);
      break;

    case FVMC_UINT32:
      _convert_array_uint64_to_uint32(src_idx_start,
                                      src_idx_end,
                                      n_parent_lists,
                                      parent_num_shift,
                                      parent_num,
                                      (const uint64_t **)src_data,
                                      (uint32_t *) dest_data);
      break;

    case FVMC_UINT64:
      _convert_array_uint64_to_uint64(src_idx_start,
                                      src_idx_end,
                                      n_parent_lists,
                                      parent_num_shift,
                                      parent_num,
                                      (const uint64_t **)src_data,
                                      (uint64_t *) dest_data);
      break;

    case FVMC_CHAR:
    case FVMC_UCHAR:
    case FVMC_DATATYPE_NULL:
      break;
		case FVMC_INT16:
    case FVMC_UINT16:
      bftc_error(__FILE__, __LINE__, 0,
                _("fvmc_writer_convert_array() may not be used to convert "
                  "to int16 or uint16"));

    }
    break;

  case FVMC_CHAR:
  case FVMC_UCHAR:
  case FVMC_DATATYPE_NULL:
    break;
	case FVMC_INT16:
	case FVMC_UINT16:
		bftc_error(__FILE__, __LINE__, 0,
							_("fvmc_writer_convert_array() may not be used to convert "
								"from int16 or uint16"));
  }
}

/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

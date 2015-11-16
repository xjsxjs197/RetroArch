/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2015 - Daniel De Matteis
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "input_hid_driver.h"
#include "../general.h"
#include "../string_list_special.h"

static joypad_connection_t *slots;

static hid_driver_t *hid_drivers[] = {
#if defined(__APPLE__) && defined(IOS)
   &btstack_hid,
#endif
#if defined(__APPLE__) && defined(HAVE_IOHIDMANAGER)
   &iohidmanager_hid,
#endif
#ifdef HAVE_LIBUSB
   &libusb_hid,
#endif
#ifdef HW_RVL
   &wiiusb_hid,
#endif
   &null_hid,
   NULL,
};

/**
 * hid_driver_find_handle:
 * @idx                : index of driver to get handle to.
 *
 * Returns: handle to HID driver at index. Can be NULL
 * if nothing found.
 **/
const void *hid_driver_find_handle(int idx)
{
   const void *drv = hid_drivers[idx];
   if (!drv)
      return NULL;
   return drv;
}

/**
 * hid_driver_find_ident:
 * @idx                : index of driver to get handle to.
 *
 * Returns: Human-readable identifier of HID driver at index. Can be NULL
 * if nothing found.
 **/
const char *hid_driver_find_ident(int idx)
{
   const hid_driver_t *drv = hid_drivers[idx];
   if (!drv)
      return NULL;
   return drv->ident;
}

joypad_connection_t *hid_driver_find_slot(unsigned idx)
{
   joypad_connection_t *conn = &slots[idx];

   if (!conn)
      return NULL;
   return conn;
}

void hid_driver_free_slots(void)
{
   if (slots)
      free(slots);
   slots = NULL;
}

void hid_driver_destroy_slots(void)
{
   pad_connection_destroy(slots);
}

bool hid_driver_slot_has_interface(unsigned idx)
{
   return pad_connection_has_interface(slots, idx);
}

int hid_driver_slot_new(const char *name, uint16_t vid, uint16_t pid,
      void *data, send_control_t ptr)
{
   return pad_connection_pad_init(slots, name, vid, pid, data, ptr);
}

void hid_driver_slot_free(unsigned idx)
{
   pad_connection_pad_deinit(&slots[idx], idx);
}

int hid_driver_slot_connect(void)
{
   int pad = pad_connection_find_vacant_pad(slots);

   if (pad >= 0 && pad < MAX_USERS)
   {
      joypad_connection_t *s = (joypad_connection_t*)&slots[pad];

      if (s)
         s->connected = true;
   }

   return pad;
}

bool hid_driver_init_slots(unsigned max_users)
{
   slots = pad_connection_init(max_users);

   if (!slots)
      return false;

   return true;
}

/**
 * config_get_hid_driver_options:
 *
 * Get an enumerated list of all HID driver names, separated by '|'.
 *
 * Returns: string listing of all HID driver names, separated by '|'.
 **/
const char* config_get_hid_driver_options(void)
{
   return char_list_new_special(STRING_LIST_INPUT_HID_DRIVERS, NULL);
}

/**
 * input_hid_init_first:
 *
 * Finds first suitable HID driver and initializes.
 *
 * Returns: HID driver if found, otherwise NULL.
 **/
const hid_driver_t *input_hid_init_first(void)
{
   unsigned i;

   for (i = 0; hid_drivers[i]; i++)
   {
      driver_t *driver = driver_get_ptr();
      driver->hid_data = hid_drivers[i]->init();

      if (driver->hid_data)
      {
         RARCH_LOG("Found HID driver: \"%s\".\n",
               hid_drivers[i]->ident);
         return hid_drivers[i];
      }
   }

   return NULL;
}

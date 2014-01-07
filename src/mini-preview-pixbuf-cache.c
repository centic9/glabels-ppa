/*
 *  mini-preview-pixbuf-cache.c
 *  Copyright (C) 2007-2009  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of gLabels.
 *
 *  gLabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "mini-preview-pixbuf-cache.h"

#include <glib.h>

#include <libglabels.h>
#include "mini-preview-pixbuf.h"

#include "debug.h"

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static GHashTable *mini_preview_pixbuf_cache = NULL;

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/


/*****************************************************************************/
/* Create a new hash table to keep track of cached mini preview pixbufs.     */
/*****************************************************************************/
void
gl_mini_preview_pixbuf_cache_init (void)
{
        GList       *names = NULL;
        GList       *p;
        lglTemplate *template;

	gl_debug (DEBUG_PIXBUF_CACHE, "START");

	mini_preview_pixbuf_cache = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

        names = lgl_db_get_template_name_list_all (NULL, NULL, NULL);
        for ( p=names; p != NULL; p=p->next )
        {
                gl_debug (DEBUG_PIXBUF_CACHE, "name = \"%s\"", p->data);

                template = lgl_db_lookup_template_from_name (p->data);
                gl_mini_preview_pixbuf_cache_add_by_template (template);
                lgl_template_free (template);
        }
        lgl_db_free_template_name_list (names);

	gl_debug (DEBUG_PIXBUF_CACHE, "END pixbuf_cache=%p", mini_preview_pixbuf_cache);
}


/*****************************************************************************/
/* Add pixbuf to cache by template.                                          */
/*****************************************************************************/
void
gl_mini_preview_pixbuf_cache_add_by_template (lglTemplate *template)
{
        GdkPixbuf        *pixbuf;
        gchar            *name;

	gl_debug (DEBUG_PIXBUF_CACHE, "START");

        pixbuf = gl_mini_preview_pixbuf_new (template, 72, 72);

        name = g_strdup_printf ("%s %s", template->brand, template->part);
        g_hash_table_insert (mini_preview_pixbuf_cache, name, g_object_ref (pixbuf));

        g_object_unref (pixbuf);

	gl_debug (DEBUG_PIXBUF_CACHE, "END");
}


/*****************************************************************************/
/* Add pixbuf to cache by name.                                              */
/*****************************************************************************/
void
gl_mini_preview_pixbuf_cache_add_by_name (gchar      *name)
{
        lglTemplate *template;
        GdkPixbuf   *pixbuf;

	gl_debug (DEBUG_PIXBUF_CACHE, "START");

        template = lgl_db_lookup_template_from_name (name);
        pixbuf = gl_mini_preview_pixbuf_new (template, 72, 72);
        lgl_template_free (template);

        g_hash_table_insert (mini_preview_pixbuf_cache, g_strdup (name), pixbuf);

	gl_debug (DEBUG_PIXBUF_CACHE, "END");
}


/*****************************************************************************/
/* Delete pixbuf from cache by name.                                         */
/*****************************************************************************/
void
gl_mini_preview_pixbuf_cache_delete_by_name (gchar *name)
{
	gl_debug (DEBUG_PIXBUF_CACHE, "START");

        g_hash_table_remove (mini_preview_pixbuf_cache, name);

	gl_debug (DEBUG_PIXBUF_CACHE, "END");
}


/*****************************************************************************/
/* Get pixbuf.                                                               */
/*****************************************************************************/
GdkPixbuf *
gl_mini_preview_pixbuf_cache_get_pixbuf (gchar      *name)
{
	GdkPixbuf   *pixbuf;

	gl_debug (DEBUG_PIXBUF_CACHE, "START pixbuf_cache=%p", mini_preview_pixbuf_cache);

	pixbuf = g_hash_table_lookup (mini_preview_pixbuf_cache, name);

        if (!pixbuf)
        {
                gl_mini_preview_pixbuf_cache_add_by_name (name);
                pixbuf = g_hash_table_lookup (mini_preview_pixbuf_cache, name);
        }

	gl_debug (DEBUG_PIXBUF_CACHE, "END");

	return g_object_ref (pixbuf);
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */

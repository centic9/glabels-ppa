/*
 *  mini-preview-pixbuf-cache.h
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

#ifndef __MINI_PREVIEW_PIXBUF_CACHE_H__
#define __MINI_PREVIEW_PIXBUF_CACHE_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libglabels/libglabels.h>

G_BEGIN_DECLS

void        gl_mini_preview_pixbuf_cache_init            (void);

void        gl_mini_preview_pixbuf_cache_add_by_name     (gchar       *name);
void        gl_mini_preview_pixbuf_cache_add_by_template (lglTemplate *template);

void        gl_mini_preview_pixbuf_cache_delete_by_name  (gchar       *name);

GdkPixbuf  *gl_mini_preview_pixbuf_cache_get_pixbuf      (gchar       *name);


G_END_DECLS

#endif /*__MINI_PREVIEW_PIXBUF_CACHE_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */

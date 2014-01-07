/*
 *  font-util.h
 *  Copyright (C) 2009  Jim Evins <evins@snaught.com>.
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

#ifndef __FONT_UTIL_H__
#define __FONT_UTIL_H__

#include <glib.h>

G_BEGIN_DECLS

const GList *gl_font_util_get_all_families          (void);
const GList *gl_font_util_get_proportional_families (void);
const GList *gl_font_util_get_fixed_width_families  (void);

gchar       *gl_font_util_validate_family           (const gchar *family);
gboolean     gl_font_util_is_family_installed       (const gchar *family);

G_END_DECLS

#endif /* __FONT_UTIL_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */

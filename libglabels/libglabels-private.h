/*
 *  libglabels-private.h
 *  Copyright (C) 2004-2009  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of libglabels.
 *
 *  libglabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libglabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libglabels.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LIBGLABELS_PRIVATE_H__
#define __LIBGLABELS_PRIVATE_H__

#include <glib.h>

#include "lgl-str.h"
#include "lgl-template.h"

#undef  G_LOG_DOMAIN
#define G_LOG_DOMAIN "LibGlabels"

#define UTF8_EQUAL(s1,s2) (!lgl_str_utf8_casecmp (s1, s2))
#define ASCII_EQUAL(s1,s2) (!g_ascii_strcasecmp (s1, s2))

void _lgl_db_register_template_internal (const lglTemplate   *template);


#endif /* __LIBGLABELS_PRIVATE_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */

/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_mini_preview.h:  mini-preview widget module header file
 *
 *  Copyright (C) 2001-2007  Jim Evins <evins@snaught.com>.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifndef __WDGT_MINI_PREVIEW_H__
#define __WDGT_MINI_PREVIEW_H__

#include <gtk/gtk.h>
#include "label.h"

G_BEGIN_DECLS

#define GL_TYPE_WDGT_MINI_PREVIEW (gl_wdgt_mini_preview_get_type ())
#define GL_WDGT_MINI_PREVIEW(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_WDGT_MINI_PREVIEW, glWdgtMiniPreview ))
#define GL_WDGT_MINI_PREVIEW_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_WDGT_MINI_PREVIEW, glWdgtMiniPreviewClass))
#define GL_IS_WDGT_MINI_PREVIEW(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_WDGT_MINI_PREVIEW))
#define GL_IS_WDGT_MINI_PREVIEW_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_WDGT_MINI_PREVIEW))

typedef struct _glWdgtMiniPreview        glWdgtMiniPreview;
typedef struct _glWdgtMiniPreviewPrivate glWdgtMiniPreviewPrivate;
typedef struct _glWdgtMiniPreviewClass   glWdgtMiniPreviewClass;

struct _glWdgtMiniPreview {
	GtkDrawingArea            parent_widget;

	glWdgtMiniPreviewPrivate *priv;
};

struct _glWdgtMiniPreviewClass {
	GtkDrawingAreaClass       parent_class;

	void (*clicked) (glWdgtMiniPreview *preview,
			 gint index,
			 gpointer user_data);

	void (*pressed) (glWdgtMiniPreview *preview,
			 gint index1,
			 gint index2,
			 gpointer user_data);
};

GType      gl_wdgt_mini_preview_get_type          (void) G_GNUC_CONST;

GtkWidget *gl_wdgt_mini_preview_new               (gint               height,
						   gint               width);

void       gl_wdgt_mini_preview_set_label_by_name (glWdgtMiniPreview *preview,
						   const gchar       *name);

void       gl_wdgt_mini_preview_set_template      (glWdgtMiniPreview *preview,
						   const lglTemplate *template);

void       gl_wdgt_mini_preview_highlight_range   (glWdgtMiniPreview *preview,
						   gint               first_label,
						   gint               last_label);

G_END_DECLS

#endif

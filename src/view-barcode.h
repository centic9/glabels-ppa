/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_barcode.h:  GLabels label barcode object view
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

#ifndef __VIEW_BARCODE_H__
#define __VIEW_BARCODE_H__

#include "view-object.h"
#include "label-barcode.h"

G_BEGIN_DECLS


#define GL_TYPE_VIEW_BARCODE            (gl_view_barcode_get_type ())
#define GL_VIEW_BARCODE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_VIEW_BARCODE, glViewBarcode))
#define GL_VIEW_BARCODE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_VIEW_BARCODE, glViewBarcodeClass))
#define GL_IS_VIEW_BARCODE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_VIEW_BARCODE))
#define GL_IS_VIEW_BARCODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_VIEW_BARCODE))

typedef struct _glViewBarcode          glViewBarcode;
typedef struct _glViewBarcodeClass     glViewBarcodeClass;

typedef struct _glViewBarcodePrivate   glViewBarcodePrivate;

struct _glViewBarcode {
	glViewObject              parent_object;

	glViewBarcodePrivate     *priv;
};

struct _glViewBarcodeClass {
	glViewObjectClass         parent_class;
};


GType          gl_view_barcode_get_type (void) G_GNUC_CONST;

glViewObject  *gl_view_barcode_new      (glLabelBarcode *object,
                                         glView         *view);


/* cursor for creating barcode objects */
GdkCursor *gl_view_barcode_get_create_cursor (void);

/* Object creation handlers. */
void       gl_view_barcode_create_button_press_event   (glView *view,
                                                        gdouble x,
                                                        gdouble y);

void       gl_view_barcode_create_motion_event         (glView *view,
                                                        gdouble x,
                                                        gdouble y);

void       gl_view_barcode_create_button_release_event (glView *view,
                                                        gdouble x,
                                                        gdouble y);


G_END_DECLS

#endif /* __VIEW_BARCODE_H__ */

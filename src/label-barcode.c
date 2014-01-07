/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label_barcode.c:  GLabels label text object
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

#include "label-barcode.h"

#include <glib/gi18n.h>
#include <glib/gmem.h>
#include <glib/gstrfuncs.h>
#include <glib/gmessages.h>
#include <cairo.h>
#include <pango/pangocairo.h>

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define FONT_SCALE (72.0/96.0)

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glLabelBarcodePrivate {
	glTextNode     *text_node;
	gchar          *id;
	glColorNode    *color_node;
	gboolean        text_flag;
	gboolean        checksum_flag;
	guint           format_digits;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void  gl_label_barcode_finalize      (GObject             *object);

static void  copy                           (glLabelObject       *dst_object,
					     glLabelObject       *src_object);

static void  get_size                       (glLabelObject       *object,
					     gdouble             *w,
					     gdouble             *h);

static void  set_line_color                 (glLabelObject       *object,
					     glColorNode         *line_color);

static glColorNode *get_line_color          (glLabelObject       *object);

static void    draw_object                (glLabelObject     *object,
                                           cairo_t           *cr,
                                           gboolean           screen_flag,
                                           glMergeRecord     *record);



/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glLabelBarcode, gl_label_barcode, GL_TYPE_LABEL_OBJECT);

static void
gl_label_barcode_class_init (glLabelBarcodeClass *class)
{
	GObjectClass       *object_class       = G_OBJECT_CLASS (class);
	glLabelObjectClass *label_object_class = GL_LABEL_OBJECT_CLASS (class);

	gl_label_barcode_parent_class = g_type_class_peek_parent (class);

	label_object_class->copy           = copy;
	label_object_class->get_size       = get_size;
	label_object_class->set_line_color = set_line_color;
	label_object_class->get_line_color = get_line_color;
        label_object_class->draw_object    = draw_object;
        label_object_class->draw_shadow    = NULL;

	object_class->finalize = gl_label_barcode_finalize;
}

static void
gl_label_barcode_init (glLabelBarcode *lbc)
{
	lbc->priv = g_new0 (glLabelBarcodePrivate, 1);
	lbc->priv->color_node = gl_color_node_new_default ();
	lbc->priv->text_node  = gl_text_node_new_from_text ("");
}

static void
gl_label_barcode_finalize (GObject *object)
{
	glLabelBarcode *lbc = GL_LABEL_BARCODE (object);

	g_return_if_fail (object && GL_IS_LABEL_BARCODE (object));

	gl_text_node_free (&lbc->priv->text_node);
	g_free (lbc->priv->id);
	gl_color_node_free (&(lbc->priv->color_node));
	g_free (lbc->priv);

	G_OBJECT_CLASS (gl_label_barcode_parent_class)->finalize (object);
}

/*****************************************************************************/
/* NEW label "barcode" object.                                               */
/*****************************************************************************/
GObject *
gl_label_barcode_new (glLabel *label)
{
	glLabelBarcode *lbc;

	lbc = g_object_new (gl_label_barcode_get_type(), NULL);

	gl_label_object_set_parent (GL_LABEL_OBJECT(lbc), label);

	return G_OBJECT (lbc);
}

/*****************************************************************************/
/* Copy object contents.                                                     */
/*****************************************************************************/
static void
copy (glLabelObject *dst_object,
      glLabelObject *src_object)
{
	glLabelBarcode      *lbc     = (glLabelBarcode *)src_object;
	glLabelBarcode      *new_lbc = (glLabelBarcode *)dst_object;
	glTextNode          *text_node;
	gchar               *id;
	gboolean             text_flag;
	gboolean             checksum_flag;
	glColorNode         *color_node;
	guint                format_digits;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (lbc && GL_IS_LABEL_BARCODE (lbc));
	g_return_if_fail (new_lbc && GL_IS_LABEL_BARCODE (new_lbc));

	text_node = gl_label_barcode_get_data (lbc);
	gl_label_barcode_get_props (lbc, &id, &text_flag, &checksum_flag, &format_digits);
	color_node = get_line_color (src_object);

	gl_label_barcode_set_data (new_lbc, text_node);
	gl_label_barcode_set_props (new_lbc, id, text_flag, checksum_flag, format_digits);
	set_line_color (dst_object, color_node);

	gl_color_node_free (&color_node);
	gl_text_node_free (&text_node);
	g_free (id);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set object params.                                                        */
/*****************************************************************************/
void
gl_label_barcode_set_data (glLabelBarcode *lbc,
			   glTextNode     *text_node)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (lbc && GL_IS_LABEL_BARCODE (lbc));

	if (!gl_text_node_equal (lbc->priv->text_node, text_node)) {

		gl_text_node_free (&lbc->priv->text_node);
		lbc->priv->text_node = gl_text_node_dup (text_node);

		gl_label_object_emit_changed (GL_LABEL_OBJECT(lbc));

	}

	gl_debug (DEBUG_LABEL, "END");
}

void
gl_label_barcode_set_props (glLabelBarcode *lbc,
			    gchar          *id,
			    gboolean        text_flag,
			    gboolean        checksum_flag,
			    guint           format_digits)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (lbc && GL_IS_LABEL_BARCODE (lbc));

	if ( ((lbc->priv->id == NULL) && (id != NULL))
	     || (g_strcasecmp (lbc->priv->id, id) != 0)
	     || (lbc->priv->text_flag != text_flag)
	     || (lbc->priv->checksum_flag != checksum_flag)
	     || (lbc->priv->format_digits != format_digits)) {

		lbc->priv->id               = g_strdup (id);
		lbc->priv->text_flag        = text_flag;
		lbc->priv->checksum_flag    = checksum_flag;
		lbc->priv->format_digits    = format_digits;

		gl_label_object_emit_changed (GL_LABEL_OBJECT(lbc));

	}

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Get object params.                                                        */
/*****************************************************************************/
glTextNode *
gl_label_barcode_get_data (glLabelBarcode *lbc)
{
	g_return_val_if_fail (lbc && GL_IS_LABEL_BARCODE (lbc), NULL);

	return gl_text_node_dup (lbc->priv->text_node);
}

void
gl_label_barcode_get_props (glLabelBarcode *lbc,
			    gchar          **id,
			    gboolean       *text_flag,
			    gboolean       *checksum_flag,
			    guint          *format_digits)
{
	g_return_if_fail (lbc && GL_IS_LABEL_BARCODE (lbc));

	*id               = g_strdup (lbc->priv->id);
	*text_flag        = lbc->priv->text_flag;
	*checksum_flag    = lbc->priv->checksum_flag;
	*format_digits    = lbc->priv->format_digits;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Get object size method.                                         */
/*---------------------------------------------------------------------------*/
static void
get_size (glLabelObject *object,
	  gdouble       *w,
	  gdouble       *h)
{
	glLabelBarcode      *lbc = (glLabelBarcode *)object;
	gchar               *data;
	gdouble              w_parent, h_parent;
	glBarcode           *gbc;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (lbc && GL_IS_LABEL_BARCODE (lbc));

	gl_label_object_get_raw_size (object, &w_parent, &h_parent);

	if (lbc->priv->text_node->field_flag) {
		data = gl_barcode_default_digits (lbc->priv->id,
						  lbc->priv->format_digits);
	} else {
		data = gl_text_node_expand (lbc->priv->text_node, NULL);
	}

	gbc = gl_barcode_new (lbc->priv->id,
			      lbc->priv->text_flag,
			      lbc->priv->checksum_flag,
			      w_parent,
			      h_parent,
			      data);
	g_free (data);

	if ( gbc == NULL ) {
		/* Try again with default digits. */
		data = gl_barcode_default_digits (lbc->priv->id,
						  lbc->priv->format_digits);
		gbc = gl_barcode_new (lbc->priv->id,
				      lbc->priv->text_flag,
				      lbc->priv->checksum_flag,
				      w_parent,
				      h_parent,
				      data);
                g_free (data);
	}

        if ( gbc != NULL )
        {
                *w = gbc->width;
                *h = gbc->height;
        }
        else
        {
                /* If we still can't render, just set a default size. */
                *w = 144;
                *h = 72;
        }

	gl_barcode_free (&gbc);

	gl_debug (DEBUG_LABEL, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set line color method.                                          */
/*---------------------------------------------------------------------------*/
static void
set_line_color (glLabelObject *object,
		glColorNode   *line_color_node)
{
	glLabelBarcode *lbarcode = (glLabelBarcode *)object;

	g_return_if_fail (lbarcode && GL_IS_LABEL_BARCODE (lbarcode));

	if ( !gl_color_node_equal(lbarcode->priv->color_node, line_color_node) ) {
		
		gl_color_node_free (&(lbarcode->priv->color_node));
		lbarcode->priv->color_node = gl_color_node_dup (line_color_node);
		gl_label_object_emit_changed (GL_LABEL_OBJECT(lbarcode));
	}
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Get line color method.                                          */
/*---------------------------------------------------------------------------*/
static glColorNode*
get_line_color (glLabelObject *object)
{
	glLabelBarcode *lbarcode = (glLabelBarcode *)object;

	g_return_val_if_fail (lbarcode && GL_IS_LABEL_BARCODE (lbarcode), NULL);

	return gl_color_node_dup (lbarcode->priv->color_node);
}

/*****************************************************************************/
/* Draw object method.                                                       */
/*****************************************************************************/
static void
draw_object (glLabelObject *object,
             cairo_t       *cr,
             gboolean       screen_flag,
             glMergeRecord *record)
{
        gdouble             x0, y0;
        cairo_matrix_t      matrix;
	glBarcode          *gbc;
	glBarcodeLine      *line;
	glBarcodeChar      *bchar;
	GList              *li;
	gdouble             y_offset;
        PangoLayout        *layout;
        PangoFontDescription *desc;
	gchar              *text, *cstring;
	glTextNode         *text_node;
	gchar              *id;
	gboolean            text_flag;
	gboolean            checksum_flag;
	guint               color;
	glColorNode        *color_node;
	guint               format_digits;
	gdouble             w, h;

	gl_debug (DEBUG_LABEL, "START");

        gl_label_object_get_position (object, &x0, &y0);
        gl_label_object_get_matrix (object, &matrix);

	text_node = gl_label_barcode_get_data (GL_LABEL_BARCODE (object));
	gl_label_barcode_get_props (GL_LABEL_BARCODE (object),
				    &id, &text_flag, &checksum_flag, &format_digits);
					
	color_node = gl_label_object_get_line_color (object);
	color = gl_color_node_expand (color_node, record);
        if (color_node->field_flag && screen_flag)
        {
                color = GL_COLOR_MERGE_DEFAULT;
        }
	gl_color_node_free (&color_node);
	
	gl_label_object_get_size (object, &w, &h);

	text_node = gl_label_barcode_get_data(GL_LABEL_BARCODE(object));
        text = gl_text_node_expand (text_node, record);
	if (text_node->field_flag && screen_flag) {
		text = gl_barcode_default_digits (id, format_digits);
	}

	gbc = gl_barcode_new (id, text_flag, checksum_flag, w, h, text);

        cairo_set_source_rgba (cr, GL_COLOR_RGBA_ARGS (color));

	if (gbc == NULL) {

                layout = pango_cairo_create_layout (cr);

                desc = pango_font_description_new ();
                pango_font_description_set_family (desc, GL_BARCODE_FONT_FAMILY);
                pango_font_description_set_size   (desc, 12 * PANGO_SCALE * FONT_SCALE);
                pango_layout_set_font_description (layout, desc);
                pango_font_description_free       (desc);

                if (text == NULL || *text == '\0')
                {
                        pango_layout_set_text (layout, _("Barcode data empty"), -1);
                }
                else
                {
                        pango_layout_set_text (layout, _("Invalid barcode data"), -1);
                }

                cairo_move_to (cr, 0, 0);
                pango_cairo_show_layout (cr, layout);

                g_object_unref (layout);

	} else {

		for (li = gbc->lines; li != NULL; li = li->next) {
			line = (glBarcodeLine *) li->data;

			cairo_move_to (cr, line->x, line->y);
			cairo_line_to (cr, line->x, line->y + line->length);
			cairo_set_line_width (cr, line->width);
			cairo_stroke (cr);
		}

		for (li = gbc->chars; li != NULL; li = li->next) {
			bchar = (glBarcodeChar *) li->data;

                        layout = pango_cairo_create_layout (cr);

                        desc = pango_font_description_new ();
                        pango_font_description_set_family (desc, GL_BARCODE_FONT_FAMILY);
                        pango_font_description_set_size   (desc, bchar->fsize * PANGO_SCALE * FONT_SCALE);
                        pango_layout_set_font_description (layout, desc);
                        pango_font_description_free       (desc);

			cstring = g_strdup_printf ("%c", bchar->c);
                        pango_layout_set_text (layout, cstring, -1);
			g_free (cstring);

                        y_offset = 0.2 * bchar->fsize;

			cairo_move_to (cr, bchar->x, bchar->y-y_offset);
                        pango_cairo_show_layout (cr, layout);

                        g_object_unref (layout);

		}

		gl_barcode_free (&gbc);

	}

	g_free (text);
	gl_text_node_free (&text_node);
	g_free (id);

	gl_debug (DEBUG_LABEL, "END");
}


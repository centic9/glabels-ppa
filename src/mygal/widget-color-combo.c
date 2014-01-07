/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * widget-color-combo.c - A color selector combo box
 * Copyright 2000, 2001, Ximian, Inc.
 *
 * Authors:
 *   Miguel de Icaza (miguel@kernel.org)
 *   Dom Lachowicz (dominicl@seas.upenn.edu)
 *
 * Reworked and split up into a separate ColorPalette object:
 *   Michael Levy (mlevy@genoscope.cns.fr)
 *
 * And later revised and polished by:
 *   Almer S. Tigelaar (almer@gnome.org)
 *
 * Modified for gLabels by:
 *   Jim Evins <evins@snaught.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License, version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <config.h>

#include <gtk/gtkentry.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkimage.h>
#include "e-util.h"
#include "e-colors.h"
#include "widget-color-combo.h"

enum {
	CHANGED,
	LAST_SIGNAL
};

static guint color_combo_signals [LAST_SIGNAL] = { 0, };

#define PARENT_TYPE MYGAL_COMBO_BOX_TYPE
static GObjectClass *color_combo_parent_class;

#define make_color(CC,COL) (((COL) != NULL) ? (COL) : ((CC) ? ((CC)->default_color) : NULL))
#define RGBA_TO_UINT(r,g,b,a)	((((guint)(r))<<24)|(((guint)(g))<<16)|(((guint)(b))<<8)|(guint)(a))
#define GDK_TO_UINT(c)	RGBA_TO_UINT(((c).red>>8), ((c).green>>8), ((c).blue>>8), 0xff)

#define PREVIEW_SIZE 20

static void
color_combo_set_color_internal (ColorCombo *cc, GdkColor *color)
{
	guint color_y, color_height;
	guint height, width;
	GdkPixbuf *pixbuf;
	GdkPixbuf *color_pixbuf;
	GdkColor *new_color;
	GdkColor *outline_color;

	new_color = make_color (cc,color);
	/* If the new and the default are NULL draw an outline */
	outline_color = (new_color) ? new_color : &e_dark_gray;

	pixbuf = gtk_image_get_pixbuf (GTK_IMAGE (cc->preview_image));

	if (!pixbuf)
		return;

	width = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);

	if (cc->preview_is_icon) {
		color_y = height - 4;
		color_height = 4;
	}
	else {
		color_y = 0;
		color_height = height;
	}

	color_pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
				       TRUE, 8,
				       width,
				       color_height);
	gdk_pixbuf_fill (color_pixbuf, GDK_TO_UINT (*outline_color));
	gdk_pixbuf_copy_area (color_pixbuf, 0, 0, width, color_height,
			      pixbuf, 0, color_y);

	if (new_color != NULL)
		gdk_pixbuf_fill (color_pixbuf, GDK_TO_UINT (*new_color));
	else
		gdk_pixbuf_fill (color_pixbuf, 0xffffff00);
	gdk_pixbuf_copy_area (color_pixbuf, 0, 0, width - 2, color_height -2,
			      pixbuf, 1, color_y + 1);

	g_object_unref (color_pixbuf);

	gtk_widget_queue_draw (GTK_WIDGET (cc));
}

static void
color_combo_class_init (GObjectClass *object_class)
{
	color_combo_parent_class = g_type_class_ref (PARENT_TYPE);

	color_combo_signals [CHANGED] =
		g_signal_new ("color_changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (ColorComboClass, color_changed),
			      NULL, NULL,
			      e_marshal_NONE__POINTER_BOOLEAN_BOOLEAN_BOOLEAN,
			      G_TYPE_NONE, 4, G_TYPE_POINTER,
			      G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN);
}

E_MAKE_TYPE (color_combo,
	     "ColorCombo",
	     ColorCombo,
	     color_combo_class_init,
	     NULL,
	     PARENT_TYPE)

/*
 * Fires signal "color_changed" with the current color as its param
 */
static void
emit_color_changed (ColorCombo *cc, GdkColor *color,
		    gboolean is_custom, gboolean by_user, gboolean is_default)
{
  	g_signal_emit (cc,
		       color_combo_signals [CHANGED], 0,
		       color, is_custom, by_user, is_default);
	mygal_combo_box_popup_hide (MYGAL_COMBO_BOX (cc));
}

static void
cb_palette_color_changed (ColorPalette *P, GdkColor *color,
		 gboolean custom, gboolean by_user, gboolean is_default,
		 ColorCombo *cc)
{
	color_combo_set_color_internal (cc, color);
	emit_color_changed (cc, color, custom, by_user, is_default);
}

static void
preview_clicked (GtkWidget *button, ColorCombo *cc)
{
	gboolean is_default;
	GdkColor *color = color_palette_get_current_color (cc->palette, &is_default);
	emit_color_changed (cc, color, FALSE, TRUE, is_default);
	if (color)
		gdk_color_free (color);
}

static void
cb_cust_color_clicked (GtkWidget *widget, ColorCombo *cc)
{
	mygal_combo_box_popup_hide (MYGAL_COMBO_BOX (cc));
}

/*
 * Creates the color table
 */
static void
color_table_setup (ColorCombo *cc,
		   char const *no_color_label, ColorGroup *color_group)
{
	g_return_if_fail (cc != NULL);

	/* Tell the palette that we will be changing it's custom colors */
	cc->palette =
		COLOR_PALETTE (color_palette_new (no_color_label,
						  cc->default_color,
						  color_group));

	{
		GtkWidget *picker = color_palette_get_color_picker (cc->palette);
		g_signal_connect (picker, "clicked",
				  G_CALLBACK (cb_cust_color_clicked), cc);
	}

	g_signal_connect (cc->palette, "color_changed",
			  G_CALLBACK (cb_palette_color_changed), cc);

	gtk_widget_show_all (GTK_WIDGET (cc->palette));

	return;
}

void
color_combo_box_set_preview_relief (ColorCombo *cc, GtkReliefStyle relief)
{
	g_return_if_fail (cc != NULL);
	g_return_if_fail (IS_COLOR_COMBO (cc));

	gtk_button_set_relief (GTK_BUTTON (cc->preview_button), relief);
}

/*
 * Where the actual construction goes on
 */
static void
color_combo_construct (ColorCombo *cc, GdkPixbuf *icon,
		       char const *no_color_label,
		       ColorGroup *color_group)
{
	GdkColor *color;
	GdkPixbuf *pixbuf = NULL;

	g_return_if_fail (cc != NULL);
	g_return_if_fail (IS_COLOR_COMBO (cc));

	/*
	 * Our button with the gtk_image preview
	 */
	cc->preview_button = gtk_button_new ();
	cc->preview_is_icon = FALSE;
	
	if (icon)
		/* use icon only if size > 4*4 */
		if ((gdk_pixbuf_get_width (icon) > 4) && 
		    (gdk_pixbuf_get_height (icon) > 4))
		{
			cc->preview_is_icon = TRUE;
			pixbuf = gdk_pixbuf_copy (icon);
		}
	
	if (pixbuf == NULL)
		pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
					 TRUE, 8, 
					 PREVIEW_SIZE, 
					 PREVIEW_SIZE);

	cc->preview_image = gtk_image_new_from_pixbuf (pixbuf);
	g_object_unref (pixbuf);

	gtk_button_set_relief (GTK_BUTTON (cc->preview_button), GTK_RELIEF_NONE);
	gtk_widget_show (cc->preview_image);
	
	gtk_container_add (GTK_CONTAINER (cc->preview_button), cc->preview_image);
	g_signal_connect (cc->preview_button, "clicked",
			  G_CALLBACK (preview_clicked), cc);

	color_table_setup (cc, no_color_label, color_group);

	gtk_widget_show_all (cc->preview_button);

	mygal_combo_box_construct (MYGAL_COMBO_BOX (cc),
				   cc->preview_button,
				   GTK_WIDGET (cc->palette));

	mygal_combo_box_set_tearable (MYGAL_COMBO_BOX (cc), FALSE);

	color = color_palette_get_current_color (cc->palette, NULL);
	color_combo_set_color_internal (cc, color);
	if (color) gdk_color_free (color);
}

/* color_combo_get_color:
 *
 * Return current color, result must be freed with gdk_color_free !
 */
GdkColor *
color_combo_get_color (ColorCombo *cc, gboolean *is_default)
{
	return color_palette_get_current_color (cc->palette, is_default);
}

/**
 * color_combo_set_color
 * @cc     The combo
 * @color  The color
 *
 * Set the color of the combo to the given color. Causes the color_changed
 * signal to be emitted.
 */
void
color_combo_set_color (ColorCombo *cc, GdkColor *color)
{
	/* This will change the color on the palette than it will invoke
	 * cb_palette_color_changed which will call emit_color_changed and
	 * set_color_internal which will change the color on our preview and
	 * will let the users of the combo know that the current color has
	 * changed
	 */
	if (color != NULL)
		gdk_rgb_find_color (gtk_widget_get_colormap (GTK_WIDGET (cc)), color);
	color_palette_set_current_color (cc->palette, color);
}

/**
 * color_combo_set_color_to_default
 * @cc  The combo
 *
 * Set the color of the combo to the default color. Causes the color_changed
 * signal to be emitted.
 */
void
color_combo_set_color_to_default (ColorCombo *cc)
{
	color_palette_set_color_to_default (cc->palette);
}

/**
 * color_combo_new :
 * icon : optionally NULL.
 * , const char *no_color_label,
 * Default constructor. Pass an optional icon and an optional label for the
 * no/auto color button.
 */
GtkWidget *
color_combo_new (GdkPixbuf *icon, char const *no_color_label,
		 GdkColor *default_color,
		 ColorGroup *color_group)
{
	ColorCombo *cc;

	cc = g_object_new (COLOR_COMBO_TYPE, NULL);

	if ( default_color )
	{
		cc->default_color = gdk_color_copy (default_color);
	}

	color_combo_construct (cc, icon, no_color_label, color_group);

	return GTK_WIDGET (cc);
}

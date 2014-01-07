/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_print_copies.c:  custom print copies widget module
 *
 *  Copyright (C) 2001  Jim Evins <evins@snaught.com>.
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

#include <config.h>

#include "wdgt-print-copies.h"

#include <glib/gi18n.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtkradiobutton.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkvbox.h>

#include "hig.h"
#include "wdgt-mini-preview.h"
#include "marshal.h"

#include "debug.h"

#define WDGT_MINI_PREVIEW_HEIGHT 175
#define WDGT_MINI_PREVIEW_WIDTH  150

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_print_copies_finalize      (GObject * object);

static void gl_wdgt_print_copies_construct     (glWdgtPrintCopies * copies,
						glLabel * label);

static void sheets_radio_cb                    (GtkToggleButton * togglebutton,
						gpointer user_data);
static void first_spin_cb                      (GtkSpinButton * spinbutton,
						gpointer user_data);
static void last_spin_cb                       (GtkSpinButton * spinbutton,
						gpointer user_data);

static void
preview_pressed (glWdgtMiniPreview *mini_preview,
		 gint first, gint last, gpointer user_data);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glWdgtPrintCopies, gl_wdgt_print_copies, GTK_TYPE_HBOX);

static void
gl_wdgt_print_copies_class_init (glWdgtPrintCopiesClass * class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	gl_wdgt_print_copies_parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_wdgt_print_copies_finalize;
}

static void
gl_wdgt_print_copies_init (glWdgtPrintCopies * copies)
{
	copies->labels_per_sheet = 0;

	copies->mini_preview = NULL;

	copies->sheets_radio = NULL;
	copies->sheets_spin = NULL;

	copies->labels_radio = NULL;
	copies->first_spin = NULL;
	copies->last_spin = NULL;
}

static void
gl_wdgt_print_copies_finalize (GObject * object)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WDGT_PRINT_COPIES (object));

	G_OBJECT_CLASS (gl_wdgt_print_copies_parent_class)->finalize (object);
}

GtkWidget *
gl_wdgt_print_copies_new (glLabel * label)
{
	glWdgtPrintCopies *copies;

	copies = g_object_new (gl_wdgt_print_copies_get_type (), NULL);

	gl_wdgt_print_copies_construct (copies, label);

	return GTK_WIDGET (copies);
}

/*--------------------------------------------------------------------------*/
/* Construct composite widget.                                              */
/*--------------------------------------------------------------------------*/
static void
gl_wdgt_print_copies_construct (glWdgtPrintCopies *copies,
				glLabel           *label)
{
	const lglTemplateFrame    *frame;
	GtkWidget                 *whbox, *wvbox, *whbox1;
	GSList                    *radio_group = NULL;
	GtkObject                 *adjust;

	whbox = GTK_WIDGET (copies);

        frame = (lglTemplateFrame *)label->template->frames->data;

	copies->labels_per_sheet = lgl_template_frame_get_n_labels (frame);

	/* mini_preview canvas */
	copies->mini_preview = gl_wdgt_mini_preview_new (WDGT_MINI_PREVIEW_HEIGHT,
						    WDGT_MINI_PREVIEW_WIDTH);
	gl_wdgt_mini_preview_set_template (GL_WDGT_MINI_PREVIEW(copies->mini_preview),
					   label->template);
	gtk_box_pack_start (GTK_BOX(whbox), copies->mini_preview, FALSE, FALSE, 0);

	wvbox = gtk_vbox_new (FALSE, GL_HIG_PAD1);
	gtk_box_pack_start (GTK_BOX(whbox), wvbox, FALSE, FALSE, 0);

	/* Sheet controls */
	whbox1 = gtk_hbox_new (FALSE, GL_HIG_PAD2);
	gtk_box_pack_start (GTK_BOX(wvbox), whbox1, FALSE, FALSE, 0);
	copies->sheets_radio =
	    gtk_radio_button_new_with_label (radio_group, _("Sheets:"));
	gtk_box_pack_start (GTK_BOX(whbox1), copies->sheets_radio, FALSE, FALSE, 0);
	adjust = gtk_adjustment_new (1, 1.0, 100.0, 1.0, 10.0, 0.0);
	copies->sheets_spin = gtk_spin_button_new (GTK_ADJUSTMENT (adjust),
						   1.0, 0);
	gtk_box_pack_start (GTK_BOX(whbox1), copies->sheets_spin, FALSE, FALSE, 0);
	gl_wdgt_mini_preview_highlight_range (GL_WDGT_MINI_PREVIEW(copies->mini_preview),
					      1, copies->labels_per_sheet);

	/* Blank line */
	gtk_box_pack_start (GTK_BOX(wvbox), gtk_label_new (""), FALSE, FALSE, 0);

	/* Label controls */
	whbox1 = gtk_hbox_new (FALSE, GL_HIG_PAD2);
	gtk_box_pack_start (GTK_BOX(wvbox), whbox1, FALSE, FALSE, 0);
	radio_group =
	    gtk_radio_button_get_group (GTK_RADIO_BUTTON (copies->sheets_radio));
	copies->labels_radio =
	    gtk_radio_button_new_with_label (radio_group, _("Labels"));
	gtk_box_pack_start (GTK_BOX(whbox1), copies->labels_radio, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(whbox1), gtk_label_new (_("from:")), FALSE, FALSE, 0);
	adjust = gtk_adjustment_new (1, 1.0, copies->labels_per_sheet, 1.0, 10.0, 0.0);
	copies->first_spin = gtk_spin_button_new (GTK_ADJUSTMENT (adjust),
						  1.0, 0);
	gtk_box_pack_start (GTK_BOX(whbox1), copies->first_spin, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(whbox1), gtk_label_new (_("to:")), FALSE, FALSE, 0);
	adjust = gtk_adjustment_new (copies->labels_per_sheet, 1.0, copies->labels_per_sheet, 1.0, 10.0, 0.0);
	copies->last_spin = gtk_spin_button_new (GTK_ADJUSTMENT (adjust),
						 1.0, 0);
	gtk_box_pack_start (GTK_BOX(whbox1), copies->last_spin, FALSE, FALSE, 0);
	gtk_widget_set_sensitive (copies->first_spin, FALSE);
	gtk_widget_set_sensitive (copies->last_spin, FALSE);

	/* Connect signals to controls */
	g_signal_connect (G_OBJECT (copies->mini_preview), "pressed",
			  G_CALLBACK (preview_pressed), copies);
	g_signal_connect (G_OBJECT (copies->sheets_radio), "toggled",
			  G_CALLBACK (sheets_radio_cb), copies);
	g_signal_connect (G_OBJECT (copies->first_spin), "changed",
			  G_CALLBACK (first_spin_cb), copies);
	g_signal_connect (G_OBJECT (copies->last_spin), "changed",
			  G_CALLBACK (last_spin_cb), copies);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Enable/Disable appropriate controls due to radio button toggle.*/
/*--------------------------------------------------------------------------*/
static void
sheets_radio_cb (GtkToggleButton * togglebutton,
		 gpointer user_data)
{
	glWdgtPrintCopies *copies = GL_WDGT_PRINT_COPIES (user_data);
	gint first, last;

	if (gtk_toggle_button_get_active (togglebutton)) {

		gtk_widget_set_sensitive (copies->sheets_spin, TRUE);
		gtk_widget_set_sensitive (copies->first_spin, FALSE);
		gtk_widget_set_sensitive (copies->last_spin, FALSE);

		gl_wdgt_mini_preview_highlight_range (GL_WDGT_MINI_PREVIEW(copies->mini_preview),
					1, copies->labels_per_sheet);

	} else {

		gtk_widget_set_sensitive (copies->sheets_spin, FALSE);
		gtk_widget_set_sensitive (copies->first_spin, TRUE);
		gtk_widget_set_sensitive (copies->last_spin, TRUE);

		first =
		    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
						      (copies->first_spin));
		last =
		    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
						      (copies->last_spin));
		gl_wdgt_mini_preview_highlight_range (GL_WDGT_MINI_PREVIEW(copies->mini_preview),
						 first, last);

	}
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change of first spin button               */
/*--------------------------------------------------------------------------*/
static void
first_spin_cb (GtkSpinButton * spinbutton,
	       gpointer user_data)
{
	glWdgtPrintCopies *copies = GL_WDGT_PRINT_COPIES (user_data);
	gint first, last;

	first =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (copies->first_spin));
	last =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (copies->last_spin));

	gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (copies->last_spin))->lower = first;

	gl_wdgt_mini_preview_highlight_range (GL_WDGT_MINI_PREVIEW(copies->mini_preview),
                                              first, last);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change of last spin button                */
/*--------------------------------------------------------------------------*/
static void
last_spin_cb (GtkSpinButton * spinbutton,
	      gpointer user_data)
{
	glWdgtPrintCopies *copies = GL_WDGT_PRINT_COPIES (user_data);
	gint first, last;

	first =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (copies->first_spin));
	last =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (copies->last_spin));

	gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (copies->first_spin))->upper = last;

	gl_wdgt_mini_preview_highlight_range (GL_WDGT_MINI_PREVIEW(copies->mini_preview),
                                              first, last);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Canvas event handler, select first and last items.             */
/*--------------------------------------------------------------------------*/
static void
preview_pressed (glWdgtMiniPreview *mini_preview,
		 gint first,
		 gint last,
		 gpointer user_data)
{
	glWdgtPrintCopies *copies = GL_WDGT_PRINT_COPIES (user_data);

	gl_wdgt_print_copies_set_range (copies, 1, first, last);
}

/****************************************************************************/
/* query selected range of labels within sheet or number of sheets.         */
/****************************************************************************/
void
gl_wdgt_print_copies_get_range (glWdgtPrintCopies * copies,
				gint * n_sheets,
				gint * first_label,
				gint * last_label)
{
	gboolean sheets_active;

	sheets_active =
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
					  (copies->sheets_radio));

	if (sheets_active) {
		*n_sheets =
		    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
						      (copies->sheets_spin));
		*first_label = 1;
		*last_label = copies->labels_per_sheet;
	} else {
		*n_sheets = 1;
		*first_label =
		    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
						      (copies->first_spin));
		*last_label =
		    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
						      (copies->last_spin));
	}
}

/****************************************************************************/
/* set range of labels within sheet or number of sheets                     */
/****************************************************************************/
void
gl_wdgt_print_copies_set_range (glWdgtPrintCopies * copies,
				gint n_sheets,
				gint first_label,
				gint last_label)
{
	gint old_first_label;

	old_first_label =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (copies->first_spin));

	if (first_label > old_first_label) {
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (copies->last_spin),
					   last_label);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (copies->first_spin),
					   first_label);
	} else {
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (copies->first_spin),
					   first_label);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (copies->last_spin),
					   last_label);
	}
	if ((first_label == 1) && (last_label == copies->labels_per_sheet)) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (copies->sheets_radio), TRUE);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON
					   (copies->sheets_spin), n_sheets);
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (copies->labels_radio), TRUE);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON
					   (copies->sheets_spin), 1.0);
	}
}

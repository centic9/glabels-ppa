/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/**
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  object-editor.c:  object properties editor module
 *
 *  Copyright (C) 2003  Jim Evins <evins@snaught.com>.
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

#include "object-editor.h"

#include <glib/gi18n.h>
#include <glade/glade-xml.h>
#include <gtk/gtklabel.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkcombobox.h>
#include <gtk/gtktogglebutton.h>

#include <math.h>

#include "prefs.h"
#include "mygal/widget-color-combo.h"
#include "color.h"
#include "wdgt-chain-button.h"
#include "marshal.h"
#include "util.h"

#include "object-editor-private.h"

#include "debug.h"

/*===========================================*/
/* Private macros                            */
/*===========================================*/

/*===========================================*/
/* Private data types                        */
/*===========================================*/

typedef void (*ChangedSignal) (GObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

gint gl_object_editor_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_object_editor_finalize           (GObject              *object);

static void gl_object_notebook_construct_valist (glObjectEditor       *editor,
                                                 glLabel              *label,
						 glObjectEditorOption  first_option,
						 va_list               args);

static void prefs_changed_cb                    (glObjectEditor       *editor);

static void label_changed_cb                    (glLabel              *label,
                                                 glObjectEditor       *editor);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glObjectEditor, gl_object_editor, GTK_TYPE_VBOX);

static void
gl_object_editor_class_init (glObjectEditorClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	gl_debug (DEBUG_EDITOR, "START");
	
  	gl_object_editor_parent_class = g_type_class_peek_parent (class);

  	object_class->finalize = gl_object_editor_finalize;  	

	gl_object_editor_signals[CHANGED] =
	    g_signal_new ("changed",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glObjectEditorClass, changed),
			  NULL, NULL,
			  gl_marshal_VOID__VOID,
			  G_TYPE_NONE, 0);

	gl_object_editor_signals[SIZE_CHANGED] =
	    g_signal_new ("size_changed",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glObjectEditorClass, size_changed),
			  NULL, NULL,
			  gl_marshal_VOID__VOID,
			  G_TYPE_NONE, 0);

	gl_debug (DEBUG_EDITOR, "END");
}

static void
gl_object_editor_init (glObjectEditor *editor)
{
	gl_debug (DEBUG_EDITOR, "START");
	
	editor->priv = g_new0 (glObjectEditorPrivate, 1);

	editor->priv->gui = glade_xml_new (GLABELS_GLADE_DIR "object-editor.glade",
					   "editor_vbox",
					   NULL);

	if (!editor->priv->gui) {
		g_critical ("Could not open object-editor.glade. gLabels may not be installed correctly!");
		return;
	}

	editor->priv->editor_vbox = glade_xml_get_widget (editor->priv->gui,
							  "editor_vbox");
	gtk_box_pack_start (GTK_BOX(editor),
			    editor->priv->editor_vbox,
			    FALSE, FALSE, 0);

	editor->priv->title_image = glade_xml_get_widget (editor->priv->gui,
							  "title_image");
	editor->priv->title_label = glade_xml_get_widget (editor->priv->gui,
							  "title_label");
	editor->priv->notebook    = glade_xml_get_widget (editor->priv->gui,
							  "notebook");

	gtk_widget_show_all (GTK_WIDGET(editor));

	/* Hide all notebook pages to start with. */
	gtk_widget_hide_all (editor->priv->notebook);
	gtk_widget_set_no_show_all (editor->priv->notebook, TRUE);

	gl_debug (DEBUG_EDITOR, "END");
}

static void 
gl_object_editor_finalize (GObject *object)
{
	glObjectEditor* editor = GL_OBJECT_EDITOR (object);;
	
	gl_debug (DEBUG_EDITOR, "START");
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_OBJECT_EDITOR (editor));
	g_return_if_fail (editor->priv != NULL);

	g_signal_handlers_disconnect_by_func (G_OBJECT(gl_prefs),
					      prefs_changed_cb, editor);
	g_signal_handlers_disconnect_by_func (G_OBJECT(editor->priv->label),
					      label_changed_cb, editor);

        g_object_unref (editor->priv->gui);
	g_free (editor->priv);

	G_OBJECT_CLASS (gl_object_editor_parent_class)->finalize (object);

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* NEW object editor.                                                      */
/*****************************************************************************/
GtkWidget*
gl_object_editor_new (gchar                *image,
		      gchar                *title,
                      glLabel              *label,
		      glObjectEditorOption  first_option, ...)
{
	glObjectEditor *editor;
	va_list         args;

	gl_debug (DEBUG_EDITOR, "START");

	editor = GL_OBJECT_EDITOR (g_object_new (GL_TYPE_OBJECT_EDITOR, NULL));

	if (image) {
		gtk_image_set_from_stock (GTK_IMAGE(editor->priv->title_image),
					  image,
					  GTK_ICON_SIZE_LARGE_TOOLBAR);
	}

	if (title) {
		gchar *s;

		s = g_strdup_printf ("<span weight=\"bold\">%s</span>",
				     title);
		gtk_label_set_text (GTK_LABEL(editor->priv->title_label), s);
		g_free (s);

		gtk_label_set_use_markup (GTK_LABEL(editor->priv->title_label), TRUE);
					  
	}

	gtk_notebook_set_homogeneous_tabs (GTK_NOTEBOOK(editor->priv->notebook), TRUE);

	va_start (args, first_option);
	gl_object_notebook_construct_valist (editor, label, first_option, args);
	va_end (args);

	gl_debug (DEBUG_EDITOR, "END");

	return GTK_WIDGET(editor);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct notebook.                                            */
/*--------------------------------------------------------------------------*/
static void
gl_object_notebook_construct_valist (glObjectEditor       *editor,
                                     glLabel              *label,
				     glObjectEditorOption  first_option,
				     va_list               args)
{
	glObjectEditorOption option;
	gint pages = 0;

	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->label = label;

	option = first_option;

	for ( option=first_option; option; option=va_arg (args, glObjectEditorOption) ) {

		switch (option) {

		case GL_OBJECT_EDITOR_EMPTY:
			gtk_widget_set_sensitive (editor->priv->title_image, FALSE);
			gtk_widget_set_sensitive (editor->priv->title_label, FALSE);
			break;

		case GL_OBJECT_EDITOR_POSITION_PAGE:
			gl_object_editor_prepare_position_page (editor);
			pages++;
			break;

		case GL_OBJECT_EDITOR_SIZE_PAGE:
		case GL_OBJECT_EDITOR_SIZE_IMAGE_PAGE:
			gl_object_editor_prepare_size_page (editor, option);
			pages++;
			break;

		case GL_OBJECT_EDITOR_SIZE_LINE_PAGE:
			gl_object_editor_prepare_lsize_page (editor);
			pages++;
			break;

		case GL_OBJECT_EDITOR_FILL_PAGE:
			gl_object_editor_prepare_fill_page (editor);
			pages++;
			break;

		case GL_OBJECT_EDITOR_LINE_PAGE:
			gl_object_editor_prepare_line_page (editor);
			pages++;
			break;

		case GL_OBJECT_EDITOR_IMAGE_PAGE:
			gl_object_editor_prepare_image_page (editor);
			pages++;
			break;

		case GL_OBJECT_EDITOR_TEXT_PAGE:
			gl_object_editor_prepare_text_page (editor);
			pages++;
			break;

		case GL_OBJECT_EDITOR_EDIT_PAGE:
			gl_object_editor_prepare_edit_page (editor);
			pages++;
			break;

		case GL_OBJECT_EDITOR_BC_PAGE:
			gl_object_editor_prepare_bc_page (editor);
			pages++;
			break;

		case GL_OBJECT_EDITOR_DATA_PAGE:
			gl_object_editor_prepare_data_page (editor);
			pages++;
			break;

		case GL_OBJECT_EDITOR_SHADOW_PAGE:
			gl_object_editor_prepare_shadow_page (editor);
			pages++;
			break;

		default:
			g_message ("option = %d", option);
			g_assert_not_reached ();
		}
		
	}
	if (pages) {
		gtk_widget_show (editor->priv->notebook);
	}

	g_signal_connect_swapped (G_OBJECT (gl_prefs), "changed",
				  G_CALLBACK (prefs_changed_cb),
				  editor);
        if (label)
        {
                label_changed_cb (label, editor);
                g_signal_connect (G_OBJECT (label), "size_changed",
                                  G_CALLBACK (label_changed_cb),
                                  editor);
                g_signal_connect (G_OBJECT (label), "merge_changed",
                                  G_CALLBACK (label_changed_cb),
                                  editor);
        }

	gl_debug (DEBUG_EDITOR, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE. Widget changed callback.  Emit our "changed" signal.            */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_changed_cb (glObjectEditor *editor)
{
        if (editor->priv->stop_signals) return;

	gl_debug (DEBUG_EDITOR, "START");

	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (editor), gl_object_editor_signals[CHANGED], 0);

	gl_debug (DEBUG_EDITOR, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE. Widget size changed callback.  Emit our "size-changed" signal.  */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_size_changed_cb (glObjectEditor *editor)
{
        if (editor->priv->stop_signals) return;

	gl_debug (DEBUG_EDITOR, "START");

	/* Emit our "size_changed" signal */
	g_signal_emit (G_OBJECT (editor), gl_object_editor_signals[SIZE_CHANGED], 0);

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Set possible key names from merge object.                                 */
/*****************************************************************************/
void
gl_object_editor_set_key_names (glObjectEditor      *editor,
				glMerge             *merge)
{
        GList     *keys;
	GtkWidget *combo;
	gboolean   fixed_flag;
	gboolean   state;
 
        gl_debug (DEBUG_EDITOR, "START");

	if (editor->priv->edit_key_label) {
		gtk_widget_set_sensitive (editor->priv->edit_key_label, merge != NULL);
	}
 
	if (editor->priv->edit_key_combo) {
		gtk_widget_set_sensitive (editor->priv->edit_key_combo, merge != NULL);
	}
 
	if (editor->priv->text_auto_shrink_check) {
		gtk_widget_set_sensitive (editor->priv->text_auto_shrink_check,
					  merge != NULL);
	}
 
	if (editor->priv->text_page_vbox) {
		gtk_widget_set_sensitive (editor->priv->text_color_key_radio, merge != NULL);
		if (merge == NULL) {
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON(editor->priv->text_color_radio), TRUE);
			gtk_widget_set_sensitive (editor->priv->text_color_combo, TRUE);
			gtk_widget_set_sensitive (editor->priv->text_color_key_combo, FALSE);
		} else {
			state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(editor->priv->text_color_key_radio));
			gtk_widget_set_sensitive (editor->priv->text_color_combo, !state);
			gtk_widget_set_sensitive (editor->priv->text_color_key_combo, state);
						  
		}
	}

	if (editor->priv->edit_insert_field_button) {
		gtk_widget_set_sensitive (editor->priv->edit_insert_field_button,
					  merge != NULL);
	}

	if (editor->priv->img_key_combo) {
		gtk_widget_set_sensitive (editor->priv->img_key_combo, merge != NULL);
	}
 
	if (editor->priv->img_key_radio) {
		gtk_widget_set_sensitive (editor->priv->img_key_radio, merge != NULL);
		if (merge == NULL) {
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON(editor->priv->img_file_radio), TRUE);
		}
	}

	if (editor->priv->data_key_combo) {
		gtk_widget_set_sensitive (editor->priv->data_key_combo, merge != NULL);
	}
 
	if (editor->priv->data_key_radio) {
		gtk_widget_set_sensitive (editor->priv->data_key_radio, merge != NULL);
		if (merge == NULL) {
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON(editor->priv->data_literal_radio), TRUE);
		}
	}
	
	fixed_flag = editor->priv->data_format_fixed_flag;
	if (editor->priv->data_format_label) {
		gtk_widget_set_sensitive (editor->priv->data_format_label,
					  (merge != NULL));
	}
	if (editor->priv->data_ex_label) {
		gtk_widget_set_sensitive (editor->priv->data_ex_label,
					  (merge != NULL));
	}
	if (editor->priv->data_digits_label) {
		gtk_widget_set_sensitive (editor->priv->data_digits_label,
					  (merge != NULL) && !fixed_flag);
	}
	if (editor->priv->data_digits_spin) {
		gtk_widget_set_sensitive (editor->priv->data_digits_spin,
					  (merge != NULL) && !fixed_flag);
	}

	if (editor->priv->fill_page_vbox) {
		gtk_widget_set_sensitive (editor->priv->fill_key_radio, merge != NULL);
		if (merge == NULL) {
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON(editor->priv->fill_color_radio), TRUE);
			gtk_widget_set_sensitive (editor->priv->fill_color_combo, TRUE);
			gtk_widget_set_sensitive (editor->priv->fill_key_combo, FALSE);
		} else {
			state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(editor->priv->fill_key_radio));
			gtk_widget_set_sensitive (editor->priv->fill_color_combo, !state);
			gtk_widget_set_sensitive (editor->priv->fill_key_combo, state);
						  
		}
	}

	if (editor->priv->line_page_vbox) {
		gtk_widget_set_sensitive (editor->priv->line_key_radio, merge != NULL);
		if (merge == NULL) {
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON(editor->priv->line_color_radio), TRUE);
			gtk_widget_set_sensitive (editor->priv->line_color_combo, TRUE);
			gtk_widget_set_sensitive (editor->priv->line_key_combo, FALSE);
		} else {
			state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(editor->priv->line_key_radio));
			gtk_widget_set_sensitive (editor->priv->line_color_combo, !state);
			gtk_widget_set_sensitive (editor->priv->line_key_combo, state);
						  
		}
	}

	if (editor->priv->bc_page_vbox) {
		gtk_widget_set_sensitive (editor->priv->bc_key_radio, merge != NULL);
		if (merge == NULL) {
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON(editor->priv->bc_color_radio), TRUE);
			gtk_widget_set_sensitive (editor->priv->bc_color_combo, TRUE);
			gtk_widget_set_sensitive (editor->priv->bc_key_combo, FALSE);
		} else {
			state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(editor->priv->bc_key_radio));
			gtk_widget_set_sensitive (editor->priv->bc_color_combo, !state);
			gtk_widget_set_sensitive (editor->priv->bc_key_combo, state);
						  
		}
	}

	if (editor->priv->shadow_page_vbox) {
		gtk_widget_set_sensitive (editor->priv->shadow_key_radio, merge != NULL);
		if (merge == NULL) {
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON(editor->priv->shadow_color_radio), TRUE);
			gtk_widget_set_sensitive (editor->priv->shadow_color_combo, TRUE);
			gtk_widget_set_sensitive (editor->priv->shadow_key_combo, FALSE);
		} else {
			state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(editor->priv->shadow_key_radio));
			gtk_widget_set_sensitive (editor->priv->shadow_color_combo, !state);
			gtk_widget_set_sensitive (editor->priv->shadow_key_combo, state);
						  
		}
	}

        keys = gl_merge_get_key_list (merge);
        if ( keys == NULL ) {
                keys = g_list_append (keys, g_strdup (""));
	}

	combo = editor->priv->img_key_combo;
	if (combo) {
		gl_util_combo_box_set_strings (GTK_COMBO_BOX (combo), keys);
	}

	combo = editor->priv->edit_key_combo;
	if (combo) {
		gl_util_combo_box_set_strings (GTK_COMBO_BOX (combo), keys);
	}

	combo = editor->priv->data_key_combo;
	if (combo) {
		gl_util_combo_box_set_strings (GTK_COMBO_BOX (combo), keys);
	}
		
	combo = editor->priv->fill_key_combo;
	if (combo) {
		gl_util_combo_box_set_strings (GTK_COMBO_BOX (combo), keys);
	}

	combo = editor->priv->text_color_key_combo;
	if (combo) {
		gl_util_combo_box_set_strings (GTK_COMBO_BOX (combo), keys);
	}

	combo = editor->priv->line_key_combo;
	if (combo) {
		gl_util_combo_box_set_strings (GTK_COMBO_BOX (combo), keys);
	}
		
	combo = editor->priv->bc_key_combo;
	if (combo) {
		gl_util_combo_box_set_strings (GTK_COMBO_BOX (combo), keys);
	}
		
	combo = editor->priv->shadow_key_combo;
	if (combo) {
		gl_util_combo_box_set_strings (GTK_COMBO_BOX (combo), keys);
	}

	gl_merge_free_key_list (&keys);
 
        gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Construct color combo "Custom widget".                                    */
/*****************************************************************************/
GtkWidget *
gl_object_editor_construct_color_combo (gchar *name,
					gchar *string1,
					gchar *string2,
					gint   int1,
					gint   int2)
{
	GtkWidget  *color_combo;
	ColorGroup *cg;
	gchar      *cg_name;
	GdkColor   *gdk_color;
	gchar      *no_color;

	switch (int1) {

	case 3:
		cg_name  = "shadow_color_group";
                gdk_color = gl_color_to_gdk_color (GL_COLOR_SHADOW_DEFAULT);
		no_color = _("Default");
		break;

	case 2:
		cg_name  = "text_color_group";
                gdk_color = gl_color_to_gdk_color (gl_prefs->default_text_color);
		no_color = _("Default");
		break;

	case 1:
		cg_name  = "line_color_group";
                gdk_color = NULL;
		no_color = _("No line");
		break;

	case 0:
	default:
		cg_name  = "fill_color_group";
                gdk_color = NULL;
		no_color = _("No fill");
		break;

	}

	cg = color_group_fetch (cg_name, NULL);
	color_combo = color_combo_new (NULL, no_color, gdk_color, cg);
        g_free (gdk_color);

	color_combo_box_set_preview_relief (COLOR_COMBO(color_combo), GTK_RELIEF_NORMAL);

	return color_combo;
}

/*****************************************************************************/
/* Construct chain button "Custom widget".                                   */
/*****************************************************************************/
GtkWidget *
gl_object_editor_construct_chain_button (gchar *name,
					 gchar *string1,
					 gchar *string2,
					 gint   int1,
					 gint   int2)
{
	GtkWidget  *chain_button;

	chain_button = gl_wdgt_chain_button_new (GL_WDGT_CHAIN_RIGHT);
	gl_wdgt_chain_button_set_active (GL_WDGT_CHAIN_BUTTON(chain_button), TRUE);

	return chain_button;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE. Prefs changed callback.  Update units related items.            */
/*--------------------------------------------------------------------------*/
static void
prefs_changed_cb (glObjectEditor *editor)
{

	gl_debug (DEBUG_EDITOR, "START");

	if (editor->priv->lsize_r_spin) {
		lsize_prefs_changed_cb (editor);
	}
		
	if (editor->priv->size_w_spin) {
		size_prefs_changed_cb (editor);
	}
		
	if (editor->priv->pos_x_spin) {
		position_prefs_changed_cb (editor);
	}

	if (editor->priv->shadow_x_spin) {
		shadow_prefs_changed_cb (editor);
	}

	gl_debug (DEBUG_EDITOR, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label "changed" callback.                                        */
/*---------------------------------------------------------------------------*/
static void
label_changed_cb (glLabel        *label,
                  glObjectEditor *editor)
{
	gdouble   label_width, label_height;
	glMerge	 *merge;

	gl_debug (DEBUG_EDITOR, "START");

	gl_label_get_size (label, &label_width, &label_height);
	gl_object_editor_set_max_position (GL_OBJECT_EDITOR (editor),
					   label_width, label_height);
	gl_object_editor_set_max_size (GL_OBJECT_EDITOR (editor),
				       label_width, label_height);
	gl_object_editor_set_max_lsize (GL_OBJECT_EDITOR (editor),
				       label_width, label_height);
	gl_object_editor_set_max_shadow_offset (GL_OBJECT_EDITOR (editor),
						label_width, label_height);
	
	merge = gl_label_get_merge (label);
	gl_object_editor_set_key_names (editor, merge);

	gl_debug (DEBUG_EDITOR, "END");
}


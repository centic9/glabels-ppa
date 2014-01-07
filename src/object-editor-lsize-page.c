/*
 *  object-editor-lsize-page.c
 *  Copyright (C) 2003-2009  Jim Evins <evins@snaught.com>.
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

#include "object-editor.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <math.h>

#include "prefs.h"
#include "builder-util.h"
#include "units-util.h"

#include "object-editor-private.h"

#include "debug.h"


/*===========================================*/
/* Private macros                            */
/*===========================================*/

#define LENGTH(x,y) sqrt( (x)*(x) + (y)*(y) )
#define ANGLE(x,y)  ( (180.0/G_PI)*atan2( -(y), (x) ) )
#define COMP_X(l,a) ( (l) * cos( (G_PI/180.0)*(a) ) )
#define COMP_Y(l,a) ( -(l) * sin( (G_PI/180.0)*(a) ) )
                                                                                

/*===========================================*/
/* Private data types                        */
/*===========================================*/


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare line size page.                                        */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_lsize_page (glObjectEditor       *editor)
{
        lglUnits      units;
	const gchar  *units_string;
	gdouble       climb_rate;
	gint          digits;

	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
        gl_builder_util_get_widgets (editor->priv->builder,
                                     "lsize_page_vbox",     &editor->priv->lsize_page_vbox,
                                     "lsize_r_spin",        &editor->priv->lsize_r_spin,
                                     "lsize_theta_spin",    &editor->priv->lsize_theta_spin,
                                     "lsize_r_units_label", &editor->priv->lsize_r_units_label,
                                     NULL);

	/* Get configuration information */
        units = gl_prefs_model_get_units (gl_prefs);
	units_string = lgl_units_get_name (units);
	editor->priv->units_per_point = lgl_units_get_units_per_point (units);
	climb_rate = gl_units_util_get_step_size (units);
	digits = gl_units_util_get_precision (units);

	/* Modify widgets based on configuration */
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(editor->priv->lsize_r_spin), digits);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(editor->priv->lsize_r_spin),
					climb_rate, 0);
	gtk_label_set_text (GTK_LABEL(editor->priv->lsize_r_units_label), units_string);

	/* Un-hide */
	gtk_widget_show_all (editor->priv->lsize_page_vbox);

	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->lsize_r_spin),
				  "value-changed",
				  G_CALLBACK (gl_object_editor_size_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->lsize_theta_spin),
				  "value-changed",
				  G_CALLBACK (gl_object_editor_size_changed_cb),
				  G_OBJECT (editor));

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Set line size.                                                            */
/*****************************************************************************/
void
gl_object_editor_set_lsize (glObjectEditor      *editor,
			    gdouble              dx,
			    gdouble              dy)
{
	gdouble r, theta;

	gl_debug (DEBUG_EDITOR, "START");


        g_signal_handlers_block_by_func (G_OBJECT (editor->priv->lsize_r_spin),
                                         gl_object_editor_size_changed_cb, editor);
        g_signal_handlers_block_by_func (G_OBJECT (editor->priv->lsize_theta_spin),
                                         gl_object_editor_size_changed_cb, editor);


	/* save a copy in internal units */
	editor->priv->dx = dx;
	editor->priv->dy = dy;

	/* convert internal units to displayed units */
	gl_debug (DEBUG_EDITOR, "internal dx,dy = %g, %g", dx, dy);
	dx *= editor->priv->units_per_point;
	dy *= editor->priv->units_per_point;
	gl_debug (DEBUG_EDITOR, "display dx,dy = %g, %g", dx, dy);

	r     = LENGTH (dx, dy);
	theta = ANGLE (dx, dy);

	/* Set widget values */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->lsize_r_spin), r);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->lsize_theta_spin),
				   theta);


        g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->lsize_r_spin),
                                           gl_object_editor_size_changed_cb, editor);
        g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->lsize_theta_spin),
                                           gl_object_editor_size_changed_cb, editor);


	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Set maximum line size.                                                    */
/*****************************************************************************/
void
gl_object_editor_set_max_lsize (glObjectEditor      *editor,
				gdouble              dx_max,
				gdouble              dy_max)
{
	gdouble tmp;

	gl_debug (DEBUG_EDITOR, "START");


        g_signal_handlers_block_by_func (G_OBJECT (editor->priv->lsize_r_spin),
                                         gl_object_editor_size_changed_cb, editor);
        g_signal_handlers_block_by_func (G_OBJECT (editor->priv->lsize_theta_spin),
                                         gl_object_editor_size_changed_cb, editor);


        /* save a copy in internal units */
        editor->priv->dx_max = dx_max;
        editor->priv->dy_max = dy_max;

        /* convert internal units to displayed units */
        gl_debug (DEBUG_EDITOR, "internal dx_max,dy_max = %g, %g", dx_max, dy_max);
        dx_max *= editor->priv->units_per_point;
        dy_max *= editor->priv->units_per_point;
        gl_debug (DEBUG_EDITOR, "display dx_max,dy_max = %g, %g", dx_max, dy_max);

        /* Set widget values */
        tmp = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->lsize_r_spin));
        gtk_spin_button_set_range (GTK_SPIN_BUTTON (editor->priv->lsize_r_spin),
                                   0.0, 2.0*LENGTH (dx_max, dy_max));
        gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->lsize_r_spin), tmp);


        g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->lsize_r_spin),
                                           gl_object_editor_size_changed_cb, editor);
        g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->lsize_theta_spin),
                                           gl_object_editor_size_changed_cb, editor);


	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Query line size.                                                          */
/*****************************************************************************/
void
gl_object_editor_get_lsize (glObjectEditor      *editor,
			    gdouble             *dx,
			    gdouble             *dy)
{
	gdouble r, theta;

	gl_debug (DEBUG_EDITOR, "START");

	/* Get values from widgets */
	r = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->lsize_r_spin));
	theta = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->lsize_theta_spin));

	/* convert everything back to our internal units (points) */
	r /= editor->priv->units_per_point;

	*dx = COMP_X (r, theta);
	*dy = COMP_Y (r, theta);

	/* save a copy in internal units */
	editor->priv->dx = *dx;
	editor->priv->dy = *dy;

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* PRIVATE. Prefs changed callback.  Update units related items.            */
/*****************************************************************************/
void
lsize_prefs_changed_cb (glObjectEditor *editor)
{
        lglUnits      units;
	const gchar  *units_string;
	gdouble       climb_rate;
	gint          digits;

	gl_debug (DEBUG_EDITOR, "START");


        g_signal_handlers_block_by_func (G_OBJECT (editor->priv->lsize_r_spin),
                                         gl_object_editor_size_changed_cb, editor);
        g_signal_handlers_block_by_func (G_OBJECT (editor->priv->lsize_theta_spin),
                                         gl_object_editor_size_changed_cb, editor);


        /* Get new configuration information */
        units = gl_prefs_model_get_units (gl_prefs);
        units_string = lgl_units_get_name (units);
        editor->priv->units_per_point = lgl_units_get_units_per_point (units);
        climb_rate = gl_units_util_get_step_size (units);
        digits = gl_units_util_get_precision (units);

	/* Update characteristics of r_spin */
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(editor->priv->lsize_r_spin), digits);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(editor->priv->lsize_r_spin), climb_rate, 0);

	/* Update r_units_label */
	gtk_label_set_text (GTK_LABEL(editor->priv->lsize_r_units_label),
			    units_string);

	/* Update values of r_spin/theta_spin */
	gl_object_editor_set_lsize (editor,
				    editor->priv->dx,
				    editor->priv->dy);
	gl_object_editor_set_max_lsize (editor,
					editor->priv->dx_max,
					editor->priv->dy_max);


        g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->lsize_r_spin),
                                           gl_object_editor_size_changed_cb, editor);
        g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->lsize_theta_spin),
                                           gl_object_editor_size_changed_cb, editor);


	gl_debug (DEBUG_EDITOR, "END");
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */

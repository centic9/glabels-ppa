/*
 *  color-combo-button.c
 *  Copyright (C) 2008-2009  Jim Evins <evins@snaught.com>.
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

#include "color-combo-button.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "color-combo-menu.h"
#include "color-swatch.h"
#include "color.h"
#include "marshal.h"

#define IMAGE_W 24
#define IMAGE_H 24

#define SWATCH_H 5


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

/** GL_COLOR_COMBO_BUTTON Private fields */
struct _glColorComboButtonPrivate {

        guint       color;
        gboolean    is_default_flag;

        guint       default_color;

        GtkWidget  *button;
        GtkWidget  *button_vbox;
        GtkWidget  *swatch;
        GtkWidget  *dropdown_button;

        GtkWidget  *menu;
};

enum {
        COLOR_CHANGED,
        LAST_SIGNAL
};


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static guint signals[LAST_SIGNAL] = {0};


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void
gl_color_combo_button_finalize (GObject *object);

static void
button_clicked_cb (glColorComboButton *this);

static gboolean
dropdown_button_press_event_cb (GtkWidget          *widget,
                                GdkEventButton     *event,
                                glColorComboButton *this);

static void
menu_color_changed_cb (glColorComboMenu            *object,
                       guint                        color,
                       gboolean                     is_default,
                       glColorComboButton          *this);

static void
menu_selection_done_cb (GtkMenuShell               *object,
                        glColorComboButton         *this);


/*****************************************************************************/
/* Object infrastructure.                                                    */
/*****************************************************************************/
G_DEFINE_TYPE (glColorComboButton, gl_color_combo_button, GTK_TYPE_HBOX)


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_color_combo_button_class_init (glColorComboButtonClass *class)
{
        GObjectClass            *gobject_class = (GObjectClass *) class;

        gl_color_combo_button_parent_class = g_type_class_peek_parent (class);

        gobject_class->finalize = gl_color_combo_button_finalize;

        signals[COLOR_CHANGED] =
                g_signal_new ("color_changed",
                              G_OBJECT_CLASS_TYPE (gobject_class),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (glColorComboButtonClass, color_changed),
                              NULL, NULL,
                              gl_marshal_VOID__UINT_BOOLEAN,
                              G_TYPE_NONE,
                              2, G_TYPE_POINTER, G_TYPE_BOOLEAN);

}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_color_combo_button_init (glColorComboButton *this)
{
        GtkWidget *arrow;

        gtk_box_set_spacing (GTK_BOX (this), 0);

        this->priv = g_new0 (glColorComboButtonPrivate, 1);

        this->priv->button_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

        this->priv->button = gtk_toggle_button_new ();
        gtk_container_add (GTK_CONTAINER (this->priv->button), this->priv->button_vbox);
        gtk_button_set_focus_on_click (GTK_BUTTON (this->priv->button), FALSE);
        g_signal_connect_swapped (this->priv->button, "clicked",
                          G_CALLBACK(button_clicked_cb), this);

        gtk_box_pack_start (GTK_BOX (this), this->priv->button, FALSE, FALSE, 0);

        this->priv->dropdown_button = gtk_toggle_button_new ();
        arrow = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_IN);
        gtk_container_add (GTK_CONTAINER (this->priv->dropdown_button), arrow);
        gtk_button_set_relief (GTK_BUTTON (this->priv->dropdown_button), GTK_RELIEF_NONE);
        gtk_button_set_focus_on_click (GTK_BUTTON (this->priv->dropdown_button), FALSE);
        g_signal_connect (this->priv->dropdown_button, "button_press_event",
                          G_CALLBACK(dropdown_button_press_event_cb), this);

        gtk_box_pack_start (GTK_BOX (this), this->priv->dropdown_button, FALSE, FALSE, 0);
}


/*****************************************************************************/
/* Finalize Method.                                                          */
/*****************************************************************************/
static void
gl_color_combo_button_finalize (GObject *object)
{
        glColorComboButton    *this;

        g_return_if_fail (object && IS_GL_COLOR_COMBO_BUTTON (object));
        this = GL_COLOR_COMBO_BUTTON (object);

        g_object_ref_sink (this->priv->menu);
        g_free (this->priv);

        G_OBJECT_CLASS (gl_color_combo_button_parent_class)->finalize (object);
}


/*****************************************************************************/
/** New Object Generator.                                                    */
/*****************************************************************************/
GtkWidget *
gl_color_combo_button_new (const gchar  *icon_name,
                           const gchar  *default_label,
                           guint         default_color,
                           guint         color)
{
        glColorComboButton *this;
        GtkWidget          *wimage;
        GdkPixbuf          *pixbuf1, *pixbuf2;

        this = g_object_new (TYPE_GL_COLOR_COMBO_BUTTON, NULL);

        if (!default_label)
        {
                default_label = _("Default Color");
        }

        this->priv->default_color = default_color;
        this->priv->color = color;

        if (icon_name)
        {
                pixbuf1 = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
                                                    icon_name, IMAGE_W, 0, NULL);
                pixbuf2 = gdk_pixbuf_new_subpixbuf (pixbuf1, 0, 0, IMAGE_W, IMAGE_H-SWATCH_H);

                wimage = gtk_image_new_from_pixbuf (pixbuf2);
                g_object_unref (G_OBJECT (pixbuf1));
                g_object_unref (G_OBJECT (pixbuf2));
                gtk_box_pack_start (GTK_BOX (this->priv->button_vbox), wimage, FALSE, FALSE, 0);

                this->priv->swatch = gl_color_swatch_new (IMAGE_W, SWATCH_H, color);
        }
        else
        {
                this->priv->swatch = gl_color_swatch_new (IMAGE_W, IMAGE_H, color);
        }
        gtk_box_pack_start (GTK_BOX (this->priv->button_vbox), this->priv->swatch, FALSE, FALSE, 0);

        this->priv->menu = gl_color_combo_menu_new (default_label, color);
        gtk_widget_show_all (this->priv->menu);

        g_signal_connect (this->priv->menu, "color_changed",
                          G_CALLBACK (menu_color_changed_cb), this);
        g_signal_connect (this->priv->menu, "selection_done",
                          G_CALLBACK (menu_selection_done_cb), this);

        return GTK_WIDGET (this);
}


/*****************************************************************************/
/* Set color.                                                                */
/*****************************************************************************/
void
gl_color_combo_button_set_color (glColorComboButton  *this,
                                 guint                color)
{
        this->priv->color = color;

        gl_color_swatch_set_color (GL_COLOR_SWATCH (this->priv->swatch), color);
}


/*****************************************************************************/
/* Set to default color.                                                     */
/*****************************************************************************/
void
gl_color_combo_button_set_to_default (glColorComboButton  *this)
{
        gl_color_combo_button_set_color (this, this->priv->default_color);
}

/*****************************************************************************/
/* Get color.                                                                */
/*****************************************************************************/
guint
gl_color_combo_button_get_color (glColorComboButton  *this,
                                 gboolean            *is_default)
{
        if (is_default)
        {
                *is_default = this->priv->is_default_flag;
        }

        return this->priv->color;
}


/*****************************************************************************/
/** Set relief style.                                                        */
/*****************************************************************************/
void
gl_color_combo_button_set_relief( glColorComboButton  *this,
                                  GtkReliefStyle       relief )
{
        gtk_button_set_relief (GTK_BUTTON (this->priv->button), relief);
}


/*****************************************************************************/
/* Color button "clicked" callback.                                          */
/*****************************************************************************/
static void
button_clicked_cb( glColorComboButton *this )
{
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (this->priv->button),
                                      FALSE);

        g_signal_emit (this, signals[COLOR_CHANGED], 0,
                       this->priv->color,
                       this->priv->is_default_flag);
}


/*****************************************************************************/
/* Menu positioning function.                                                */
/*****************************************************************************/
static void
menu_position_function (GtkMenu            *menu,
                        gint               *x,
                        gint               *y,
                        gboolean           *push_in,
                        glColorComboButton *this)
{
        GdkScreen          *screen;
        gint                w_screen, h_screen;
        GdkWindow          *window;
        gint                x_window, y_window;
        GtkAllocation       allocation;
        gint                x_this, y_this, h_this;
        GtkRequisition      menu_requisition;
        gint                h_menu, w_menu;

        /*
         * Screen size
         */
        screen = gtk_widget_get_screen (GTK_WIDGET (this));
        w_screen = gdk_screen_get_width (screen);
        h_screen = gdk_screen_get_height (screen);

        /*
         * Absolute position of "this" window on screen.
         */
        window = gtk_widget_get_window (GTK_WIDGET (this));
        gdk_window_get_origin (window, &x_window, &y_window);

        /*
         *  Position and size of "this" inside window
         */
        gtk_widget_get_allocation (GTK_WIDGET (this), &allocation);
        x_this = allocation.x;
        y_this = allocation.y;
        h_this = allocation.height;

        /*
         * Size of menu.
         */
        gtk_widget_get_preferred_size (this->priv->menu, NULL, &menu_requisition);
        h_menu = menu_requisition.height;
        w_menu = menu_requisition.width;

        /*
         * Default position anchored to lower left corner of "this".
         */
        *x = x_window + x_this;
        *y = y_window + y_this + h_this;
                
        /*
         * Adjust vertical position if menu if extends past bottom of screen.
         */
        if ( (*y + h_menu) > h_screen )
        {
                *y = y_window + y_this - h_menu;

                if ( *y < 0 )
                {
                        *y = h_screen - h_menu;
                }
        }

        /*
         * Adjust horizontal position if menu if extends past edge of screen.
         */
        if ( (*x + w_menu) > w_screen )
        {
                *x = w_screen - w_menu;
        }


        *push_in = TRUE;
}


/*****************************************************************************/
/* Dropdown button "button_press_event" callback.                            */
/*****************************************************************************/
static gboolean
dropdown_button_press_event_cb (GtkWidget          *widget,
                                GdkEventButton     *event,
                                glColorComboButton *this)
{
        switch (event->button)
        {

        case 1:
                g_signal_handlers_block_by_func (G_OBJECT (this->priv->button),
                                                 button_clicked_cb, this);
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (this->priv->button),
                                              TRUE);
                g_signal_handlers_unblock_by_func (G_OBJECT (this->priv->button),
                                                   button_clicked_cb, this);
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (this->priv->dropdown_button),
                                              TRUE);

                gtk_menu_popup (GTK_MENU (this->priv->menu),
                                NULL, NULL,
                                (GtkMenuPositionFunc)menu_position_function, this,
                                event->button, event->time);
                break;

        default:
                break;

        }

        return FALSE;
}


/*****************************************************************************/
/* Menu "color changed" callback.                                            */
/*****************************************************************************/
static void
menu_color_changed_cb (glColorComboMenu   *object,
                       guint               color,
                       gboolean            is_default,
                       glColorComboButton *this)
{
        if (is_default)
        {
                this->priv->color = this->priv->default_color;
        }
        else
        {
                this->priv->color = color;
        }
        this->priv->is_default_flag = is_default;

        gl_color_swatch_set_color (GL_COLOR_SWATCH (this->priv->swatch),
                                   this->priv->color);

        g_signal_emit (this, signals[COLOR_CHANGED], 0,
                       this->priv->color,
                       this->priv->is_default_flag);
}


/*****************************************************************************/
/* Menu "color changed" callback.                                            */
/*****************************************************************************/
static void
menu_selection_done_cb (GtkMenuShell       *object,
                        glColorComboButton *this)
{
        g_signal_handlers_block_by_func (G_OBJECT (this->priv->button),
                                         button_clicked_cb, this);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (this->priv->button),
                                      FALSE);
        g_signal_handlers_unblock_by_func (G_OBJECT (this->priv->button),
                                           button_clicked_cb, this);

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (this->priv->dropdown_button),
                                      FALSE);
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */

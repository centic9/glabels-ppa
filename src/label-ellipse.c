/*
 *  label-ellipse.c
 *  Copyright (C) 2001-2009  Jim Evins <evins@snaught.com>.
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

#include "label-ellipse.h"

#include <glib/gi18n.h>
#include <glib.h>
#include <math.h>

#include "cairo-ellipse-path.h"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glLabelEllipsePrivate {
	gdouble          line_width;
	glColorNode     *line_color_node;
	glColorNode     *fill_color_node;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void    gl_label_ellipse_finalize  (GObject           *object);

static void    copy                       (glLabelObject     *dst_object,
                                           glLabelObject     *src_object);

static void    set_fill_color             (glLabelObject     *object,
                                           glColorNode       *fill_color_node,
                                           gboolean           checkpoint);

static void    set_line_color             (glLabelObject     *object,
                                           glColorNode       *line_color_node,
                                           gboolean           checkpoint);

static void    set_line_width             (glLabelObject     *object,
                                           gdouble            line_width,
                                           gboolean           checkpoint);

static glColorNode*   get_fill_color      (glLabelObject     *object);

static glColorNode*   get_line_color      (glLabelObject     *object);

static gdouble get_line_width             (glLabelObject     *object);

static void    draw_object                (glLabelObject     *object,
                                           cairo_t           *cr,
                                           gboolean           screen_flag,
                                           glMergeRecord     *record);

static void    draw_shadow                (glLabelObject     *object,
                                           cairo_t           *cr,
                                           gboolean           screen_flag,
                                           glMergeRecord     *record);

static gboolean object_at                 (glLabelObject     *object,
                                           cairo_t           *cr,
                                           gdouble            x_pixels,
                                           gdouble            y_pixels);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glLabelEllipse, gl_label_ellipse, GL_TYPE_LABEL_OBJECT)


static void
gl_label_ellipse_class_init (glLabelEllipseClass *class)
{
	GObjectClass       *object_class       = G_OBJECT_CLASS (class);
	glLabelObjectClass *label_object_class = GL_LABEL_OBJECT_CLASS (class);

	gl_label_ellipse_parent_class = g_type_class_peek_parent (class);

	label_object_class->copy           = copy;
	label_object_class->set_fill_color = set_fill_color;
	label_object_class->set_line_color = set_line_color;
	label_object_class->set_line_width = set_line_width;
	label_object_class->get_fill_color = get_fill_color;
	label_object_class->get_line_color = get_line_color;
	label_object_class->get_line_width = get_line_width;
        label_object_class->draw_object    = draw_object;
        label_object_class->draw_shadow    = draw_shadow;
        label_object_class->object_at      = object_at;

	object_class->finalize = gl_label_ellipse_finalize;
}


static void
gl_label_ellipse_init (glLabelEllipse *lellipse)
{
	lellipse->priv = g_new0 (glLabelEllipsePrivate, 1);
}


static void
gl_label_ellipse_finalize (GObject *object)
{
	glLabelEllipse *lellipse = GL_LABEL_ELLIPSE (object);

	g_return_if_fail (object && GL_IS_LABEL_ELLIPSE (object));

	gl_color_node_free (&(lellipse->priv->line_color_node));
	gl_color_node_free (&(lellipse->priv->fill_color_node));
	g_free (lellipse->priv);

	G_OBJECT_CLASS (gl_label_ellipse_parent_class)->finalize (object);
}


/*****************************************************************************/
/* NEW label "ellipse" object.                                               */
/*****************************************************************************/
GObject *
gl_label_ellipse_new (glLabel *label,
                      gboolean checkpoint)
{
	glLabelEllipse      *lellipse;
	glColorNode         *fill_color_node;
	glColorNode         *line_color_node;

	lellipse = g_object_new (gl_label_ellipse_get_type(), NULL);

        if (label != NULL)
        {
                if ( checkpoint )
                {
                        gl_label_checkpoint (label, _("Create ellipse object"));
                }

                fill_color_node = gl_color_node_new_default ();
                line_color_node = gl_color_node_new_default ();

                line_color_node->color = gl_label_get_default_line_color(label);
                fill_color_node->color = gl_label_get_default_fill_color(label);

                lellipse->priv->line_width      = gl_label_get_default_line_width(label);
                lellipse->priv->line_color_node = line_color_node;
                lellipse->priv->fill_color_node = fill_color_node;

                gl_label_add_object (label, GL_LABEL_OBJECT (lellipse));
                gl_label_object_set_parent (GL_LABEL_OBJECT (lellipse), label);
        }

	return G_OBJECT (lellipse);
}


/*****************************************************************************/
/* Copy object contents.                                                     */
/*****************************************************************************/
static void
copy (glLabelObject *dst_object,
      glLabelObject *src_object)
{
	glLabelEllipse *lellipse     = (glLabelEllipse *)src_object;
	glLabelEllipse *new_lellipse = (glLabelEllipse *)dst_object;
	gdouble         line_width;
	glColorNode    *line_color_node;
	glColorNode    *fill_color_node;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (lellipse && GL_IS_LABEL_ELLIPSE (lellipse));
	g_return_if_fail (new_lellipse && GL_IS_LABEL_ELLIPSE (new_lellipse));

	line_width = get_line_width (src_object);
	line_color_node = get_line_color (src_object);
	fill_color_node = get_fill_color (src_object);

	set_line_width (dst_object, line_width, FALSE);
	set_line_color (dst_object, line_color_node, FALSE);
	set_fill_color (dst_object, fill_color_node, FALSE);

	gl_color_node_free (&line_color_node);
	gl_color_node_free (&fill_color_node);
	
	gl_debug (DEBUG_LABEL, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set fill color method.                                          */
/*---------------------------------------------------------------------------*/
static void
set_fill_color (glLabelObject *object,
		glColorNode   *fill_color_node,
                gboolean       checkpoint)
{
	glLabelEllipse *lellipse = (glLabelEllipse *)object;
        glLabel        *label;

	g_return_if_fail (lellipse && GL_IS_LABEL_ELLIPSE (lellipse));

	if (!gl_color_node_equal (lellipse->priv->fill_color_node, fill_color_node))
        {
                if ( checkpoint )
                {
                        label = gl_label_object_get_parent (GL_LABEL_OBJECT (lellipse));
                        gl_label_checkpoint (label, _("Fill color"));
                }

		gl_color_node_free (&(lellipse->priv->fill_color_node));
		lellipse->priv->fill_color_node = gl_color_node_dup (fill_color_node);

		gl_label_object_emit_changed (GL_LABEL_OBJECT(lellipse));
	}	
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set line color method.                                          */
/*---------------------------------------------------------------------------*/
static void
set_line_color (glLabelObject *object,
		glColorNode   *line_color_node,
                gboolean       checkpoint)
{
	glLabelEllipse *lellipse = (glLabelEllipse *)object;
        glLabel        *label;

	g_return_if_fail (lellipse && GL_IS_LABEL_ELLIPSE (lellipse));

	if ( !gl_color_node_equal (lellipse->priv->line_color_node, line_color_node) )
        {
                if ( checkpoint )
                {
                        label = gl_label_object_get_parent (GL_LABEL_OBJECT (lellipse));
                        gl_label_checkpoint (label, _("Line color"));
                }

		gl_color_node_free (&(lellipse->priv->line_color_node));
		lellipse->priv->line_color_node = gl_color_node_dup (line_color_node);
		
		gl_label_object_emit_changed (GL_LABEL_OBJECT(lellipse));
	}
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set line width method.                                          */
/*---------------------------------------------------------------------------*/
static void
set_line_width (glLabelObject *object,
		gdouble        line_width,
                gboolean       checkpoint)
{
	glLabelEllipse *lellipse = (glLabelEllipse *)object;
        glLabel        *label;

	g_return_if_fail (lellipse && GL_IS_LABEL_ELLIPSE (lellipse));

	if ( lellipse->priv->line_width != line_width )
        {
                if ( checkpoint )
                {
                        label = gl_label_object_get_parent (GL_LABEL_OBJECT (lellipse));
                        gl_label_checkpoint (label, _("Line width"));
                }

		lellipse->priv->line_width = line_width;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(lellipse));
	}
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Get fill color method.                                          */
/*---------------------------------------------------------------------------*/
static gdouble
get_line_width (glLabelObject *object)
{
	glLabelEllipse *lellipse = (glLabelEllipse *)object;

	g_return_val_if_fail (lellipse && GL_IS_LABEL_ELLIPSE (lellipse), 0.0);

	return lellipse->priv->line_width;
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Get line color method.                                          */
/*---------------------------------------------------------------------------*/
static glColorNode*
get_line_color (glLabelObject *object)
{
	glLabelEllipse *lellipse = (glLabelEllipse *)object;

	g_return_val_if_fail (lellipse && GL_IS_LABEL_ELLIPSE (lellipse), 0);

	return gl_color_node_dup (lellipse->priv->line_color_node);
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Get line width method.                                          */
/*---------------------------------------------------------------------------*/
static glColorNode*
get_fill_color (glLabelObject *object)
{
	glLabelEllipse *lellipse = (glLabelEllipse *)object;

	g_return_val_if_fail (lellipse && GL_IS_LABEL_ELLIPSE (lellipse), 0);

	return gl_color_node_dup (lellipse->priv->fill_color_node);
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
        gdouble        w, h;
	gdouble        line_width;
	glColorNode   *line_color_node;
	glColorNode   *fill_color_node;
        guint          line_color;
        guint          fill_color;

	gl_debug (DEBUG_LABEL, "START");

        gl_label_object_get_size (object, &w, &h);

	line_width = gl_label_object_get_line_width (object);
	
	line_color_node = gl_label_object_get_line_color (object);
	fill_color_node = gl_label_object_get_fill_color (object);
        line_color = gl_color_node_expand (line_color_node, record);
        fill_color = gl_color_node_expand (fill_color_node, record);
        if (line_color_node->field_flag && screen_flag)
        {
                line_color = GL_COLOR_MERGE_DEFAULT;
        }
        if (fill_color_node->field_flag && screen_flag)
        {
                fill_color = GL_COLOR_FILL_MERGE_DEFAULT;
        }


        gl_cairo_ellipse_path (cr, w/2, h/2);

	/* Paint fill color */
        cairo_set_source_rgba (cr, GL_COLOR_RGBA_ARGS (fill_color));
	cairo_fill_preserve (cr);

	/* Draw outline */
        cairo_set_source_rgba (cr, GL_COLOR_RGBA_ARGS (line_color));
        cairo_set_line_width (cr, line_width);
        cairo_stroke (cr);

	gl_color_node_free (&line_color_node);
	gl_color_node_free (&fill_color_node);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Draw shadow method.                                                       */
/*****************************************************************************/
static void
draw_shadow (glLabelObject *object,
             cairo_t       *cr,
             gboolean       screen_flag,
             glMergeRecord *record)
{
        gdouble        w, h;
	gdouble        line_width;
	glColorNode   *line_color_node;
	glColorNode   *fill_color_node;
        guint          line_color;
        guint          fill_color;
	glColorNode   *shadow_color_node;
        guint          shadow_color;
	gdouble        shadow_opacity;

	gl_debug (DEBUG_LABEL, "START");

        cairo_save (cr);

        gl_label_object_get_size (object, &w, &h);

	line_width = gl_label_object_get_line_width (object);
	
	line_color_node = gl_label_object_get_line_color (object);
	fill_color_node = gl_label_object_get_fill_color (object);
        line_color = gl_color_node_expand (line_color_node, record);
        fill_color = gl_color_node_expand (fill_color_node, record);
        if (line_color_node->field_flag && screen_flag)
        {
                line_color = GL_COLOR_MERGE_DEFAULT;
        }
        if (fill_color_node->field_flag && screen_flag)
        {
                fill_color = GL_COLOR_FILL_MERGE_DEFAULT;
        }

	shadow_color_node = gl_label_object_get_shadow_color (object);
        shadow_color = gl_color_node_expand (shadow_color_node, record);
	if (shadow_color_node->field_flag && screen_flag)
	{
		shadow_color = GL_COLOR_SHADOW_MERGE_DEFAULT;
	}
	shadow_opacity = gl_label_object_get_shadow_opacity (object);
	shadow_color = gl_color_set_opacity (shadow_color, shadow_opacity);
	

        cairo_set_source_rgba (cr, GL_COLOR_RGBA_ARGS (shadow_color));

        if ( GL_COLOR_F_ALPHA (fill_color) )
        {
                if ( GL_COLOR_F_ALPHA (line_color) )
                {
                        /* Has FILL and OUTLINE: adjust size to account for line width. */
                        cairo_translate (cr, -line_width/2, -line_width/2);
                        gl_cairo_ellipse_path (cr, (w+line_width)/2, (h+line_width)/2);

                }
                else
                {
                        /* Has FILL but no OUTLINE. */
                        gl_cairo_ellipse_path (cr, w/2, h/2);
                }

                /* Draw shadow */
                cairo_fill (cr);
        }
        else
        {
                if ( GL_COLOR_F_ALPHA (line_color) )
                {
                        /* Has only OUTLINE. */
                        gl_cairo_ellipse_path (cr, w/2, h/2);

                        /* Draw shadow of outline */
                        cairo_set_line_width (cr, line_width);
                        cairo_stroke (cr);
                }
        }


	gl_color_node_free (&line_color_node);
	gl_color_node_free (&fill_color_node);
	gl_color_node_free (&shadow_color_node);

        cairo_restore (cr);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Is object at coordinates?                                                 */
/*****************************************************************************/
static gboolean
object_at (glLabelObject *object,
           cairo_t       *cr,
           gdouble        x,
           gdouble        y)
{
        gdouble           w, h;
        gdouble           line_width;

        gl_label_object_get_size (object, &w, &h);

        cairo_new_path (cr);
        gl_cairo_ellipse_path (cr, w/2, h/2);

        if (cairo_in_fill (cr, x, y))
        {
                return TRUE;
        }

        line_width = gl_label_object_get_line_width (object);
        cairo_set_line_width (cr, line_width);
        if (cairo_in_stroke (cr, x, y))
        {
                return TRUE;
        }

        return FALSE;
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */

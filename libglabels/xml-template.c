/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (LIBGLABELS) Template library for GLABELS
 *
 *  xml-template.c:  template xml module
 *
 *  Copyright (C) 2001-2006  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of the LIBGLABELS library.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 *  MA 02111-1307, USA
 */
#include <config.h>

#include "xml-template.h"

#include <glib/gi18n.h>
#include <glib/gmessages.h>
#include <string.h>
#include <libintl.h>

#include "libglabels-private.h"

#include "db.h"
#include "xml.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/
static void  xml_parse_meta_node            (xmlNodePtr              label_node,
					     lglTemplate            *template);
static void  xml_parse_label_rectangle_node (xmlNodePtr              label_node,
					     lglTemplate            *template);
static void  xml_parse_label_round_node     (xmlNodePtr              label_node,
					     lglTemplate            *template);
static void  xml_parse_label_cd_node        (xmlNodePtr              label_node,
					     lglTemplate            *template);
static void  xml_parse_layout_node          (xmlNodePtr              layout_node,
					     lglTemplateFrame       *frame);
static void  xml_parse_markup_margin_node   (xmlNodePtr              markup_node,
					     lglTemplateFrame       *frame);
static void  xml_parse_markup_line_node     (xmlNodePtr              markup_node,
					     lglTemplateFrame       *frame);
static void  xml_parse_markup_circle_node   (xmlNodePtr              markup_node,
					     lglTemplateFrame       *frame);
static void  xml_parse_markup_rect_node     (xmlNodePtr              markup_node,
					     lglTemplateFrame       *frame);
static void  xml_parse_alias_node           (xmlNodePtr              alias_node,
					     lglTemplate            *template);

static void  xml_create_meta_node           (const gchar                  *category,
					     xmlNodePtr                    root,
					     const xmlNsPtr                ns);
static void  xml_create_label_node          (const lglTemplateFrame       *frame,
					     xmlNodePtr                    root,
					     const xmlNsPtr                ns);
static void  xml_create_layout_node         (const lglTemplateLayout      *layout,
					     xmlNodePtr                    root,
					     const xmlNsPtr                ns);
static void  xml_create_markup_margin_node  (const lglTemplateMarkup      *margin,
					     xmlNodePtr                    root,
					     const xmlNsPtr                ns);
static void  xml_create_markup_line_node    (const lglTemplateMarkup      *line,
					     xmlNodePtr                    root,
					     const xmlNsPtr                ns);
static void  xml_create_markup_circle_node  (const lglTemplateMarkup      *circle,
					     xmlNodePtr                    root,
					     const xmlNsPtr                ns);
static void  xml_create_markup_rect_node    (const lglTemplateMarkup      *circle,
					     xmlNodePtr                    root,
					     const xmlNsPtr                ns);
static void  xml_create_alias_node          (const lglTemplateAlias       *alias,
					     xmlNodePtr                    root,
					     const xmlNsPtr                ns);


/**
 * lgl_xml_template_read_templates_from_file:
 * @utf8_filename:       Filename of papers file (name encoded as UTF-8)
 *
 * Read glabels templates from template file.
 *
 * Returns: a list of #lglTemplate structures.
 *
 */
GList *
lgl_xml_template_read_templates_from_file (const gchar *utf8_filename)
{
	gchar      *filename;
	xmlDocPtr   templates_doc;
	GList      *templates = NULL;

	LIBXML_TEST_VERSION;

	filename = g_filename_from_utf8 (utf8_filename, -1, NULL, NULL, NULL);
	if (!filename) {
		g_message ("Utf8 filename conversion error");
		return NULL;
	}

	templates_doc = xmlParseFile (filename);
	if (!templates_doc) {
		g_message ("\"%s\" is not a glabels template file (not XML)",
		      filename);
		return templates;
	}

	templates = lgl_xml_template_parse_templates_doc (templates_doc);

	g_free (filename);
	xmlFreeDoc (templates_doc);

	return templates;
}


/**
 * lgl_xml_template_parse_templates_doc:
 * @templates_doc:  libxml #xmlDocPtr tree, representing template file.
 *
 * Read glabels templates from a libxml #xmlDocPtr tree.
 *
 * Returns: a list of #lglTemplate structures.
 *
 */
GList *
lgl_xml_template_parse_templates_doc (const xmlDocPtr templates_doc)
{
	
	GList       *templates = NULL;
	xmlNodePtr   root, node;
	lglTemplate *template;

	LIBXML_TEST_VERSION;

	root = xmlDocGetRootElement (templates_doc);
	if (!root || !root->name) {
		g_message ("\"%s\" is not a glabels template file (no root node)",
			   templates_doc->URL);
		return templates;
	}
	if (!lgl_xml_is_node (root, "Glabels-templates")) {
		g_message ("\"%s\" is not a glabels template file (wrong root node)",
		      templates_doc->URL);
		return templates;
	}

	for (node = root->xmlChildrenNode; node != NULL; node = node->next) {

		if (lgl_xml_is_node (node, "Template")) {
			template = lgl_xml_template_parse_template_node (node);
			templates = g_list_append (templates, template);
		} else {
			if ( !xmlNodeIsText(node) ) {
				if (!lgl_xml_is_node (node,"comment")) {
					g_message ("bad node =  \"%s\"",node->name);
				}
			}
		}
	}

	return templates;
}


/**
 * lgl_xml_template_parse_template_node:
 * @template_node:  libxml #xmlNodePtr template node from a #xmlDocPtr tree.
 *
 * Read a single glabels template from a libxml #xmlNodePtr node.
 *
 * Returns: a pointer to a newly created #lglTemplate structure.
 *
 */
lglTemplate *
lgl_xml_template_parse_template_node (const xmlNodePtr template_node)
{
	gchar                 *brand;
        gchar                 *part;
	gchar                 *name;
	gchar                 *description;
	gchar                 *paper_id;
	gdouble                page_width, page_height;
	lglPaper               *paper = NULL;
	lglTemplate           *template;
	xmlNodePtr             node;
        gchar                **v;

	brand = lgl_xml_get_prop_string (template_node, "brand", NULL);
	part  = lgl_xml_get_prop_string (template_node, "part", NULL);
        if (!brand || !part)
        {
                name = lgl_xml_get_prop_string (template_node, "name", NULL);
                if (name)
                {
                        v = g_strsplit (name, " ", 2);
                        brand = g_strdup (v[0]);
                        part  = g_strchug (g_strdup (v[1]));
                        g_free (name);
                        g_strfreev (v);
                        
                }
                else
                {
			g_message (_("Missing name or brand/part attributes."));
                }
        }

	description = lgl_xml_get_prop_i18n_string (template_node, "description", NULL);
	paper_id = lgl_xml_get_prop_string (template_node, "size", NULL);

	if (lgl_db_is_paper_id_other (paper_id)) {

		page_width = lgl_xml_get_prop_length (template_node, "width", 0);
		page_height = lgl_xml_get_prop_length (template_node, "height", 0);

	} else {
		paper = lgl_db_lookup_paper_from_id (paper_id);
		if (paper == NULL) {
			/* This should always be an id, but just in case a name
			   slips by! */
			g_message (_("Unknown page size id \"%s\", trying as name"),
				   paper_id);
			paper = lgl_db_lookup_paper_from_name (paper_id);
			g_free (paper_id);
			paper_id = g_strdup (paper->id);
		}
		if (paper != NULL) {
			page_width  = paper->width;
			page_height = paper->height;
		} else {
			page_width  = 612;
			page_height = 792;
			g_message (_("Unknown page size id or name \"%s\""),
				   paper_id);
		}
		lgl_paper_free (paper);
		paper = NULL;
	}

	template = lgl_template_new (brand, part, description,
                                     paper_id, page_width, page_height);

	for (node = template_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (lgl_xml_is_node (node, "Meta")) {
			xml_parse_meta_node (node, template);
		} else if (lgl_xml_is_node (node, "Label-rectangle")) {
			xml_parse_label_rectangle_node (node, template);
		} else if (lgl_xml_is_node (node, "Label-round")) {
			xml_parse_label_round_node (node, template);
		} else if (lgl_xml_is_node (node, "Label-cd")) {
			xml_parse_label_cd_node (node, template);
		} else if (lgl_xml_is_node (node, "Alias")) {
			xml_parse_alias_node (node, template);
		} else {
			if (!xmlNodeIsText (node)) {
				if (!lgl_xml_is_node (node,"comment")) {
					g_message ("bad node =  \"%s\"",node->name);
				}
			}
		}
	}

	g_free (brand);
	g_free (part);
	g_free (description);
	g_free (paper_id);

        /*
         * Create a default full-page frame, if a known frame type was not found.
         */
        if ( template->frames == NULL )
        {
                lglTemplateFrame    *frame;

                frame = lgl_template_frame_rect_new ("0", page_width, page_height, 0, 0, 0);
                lgl_template_frame_add_layout (frame, lgl_template_layout_new (1, 1, 0, 0, 0, 0));
                lgl_template_add_frame (template, frame);
        }

	return template;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Meta Node.                                 */
/*--------------------------------------------------------------------------*/
static void
xml_parse_meta_node (xmlNodePtr   meta_node,
		     lglTemplate *template)
{
	gchar               *category;

	category = lgl_xml_get_prop_string (meta_node, "category", NULL);

	if (category != NULL)
	{
		lgl_template_add_category (template, category);
		g_free (category);
	}
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label-rectangle Node.                      */
/*--------------------------------------------------------------------------*/
static void
xml_parse_label_rectangle_node (xmlNodePtr   label_node,
				lglTemplate *template)
{
	gchar               *id;
	gchar               *tmp;
	gdouble              x_waste, y_waste;
	gdouble              w, h, r;
	lglTemplateFrame    *frame;
	xmlNodePtr           node;

	id      = lgl_xml_get_prop_string (label_node, "id", NULL);

	if ((tmp = lgl_xml_get_prop_string (label_node, "waste", NULL))) {
		/* Handle single "waste" property. */
		x_waste = y_waste = lgl_xml_get_prop_length (label_node, "waste", 0);
		g_free (tmp);
	} else {
		x_waste = lgl_xml_get_prop_length (label_node, "x_waste", 0);
		y_waste = lgl_xml_get_prop_length (label_node, "y_waste", 0);
	}

	w       = lgl_xml_get_prop_length (label_node, "width", 0);
	h       = lgl_xml_get_prop_length (label_node, "height", 0);
	r       = lgl_xml_get_prop_length (label_node, "round", 0);

	frame = lgl_template_frame_rect_new ((gchar *)id, w, h, r, x_waste, y_waste);
	lgl_template_add_frame (template, frame);

	for (node = label_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (lgl_xml_is_node (node, "Layout")) {
			xml_parse_layout_node (node, frame);
		} else if (lgl_xml_is_node (node, "Markup-margin")) {
			xml_parse_markup_margin_node (node, frame);
		} else if (lgl_xml_is_node (node, "Markup-line")) {
			xml_parse_markup_line_node (node, frame);
		} else if (lgl_xml_is_node (node, "Markup-circle")) {
			xml_parse_markup_circle_node (node, frame);
		} else if (lgl_xml_is_node (node, "Markup-rect")) {
			xml_parse_markup_rect_node (node, frame);
		} else if (!xmlNodeIsText (node)) {
			if (!lgl_xml_is_node (node, "comment")) {
				g_message ("bad node =  \"%s\"",node->name);
			}
		}
	}

	g_free (id);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label-round Node.                          */
/*--------------------------------------------------------------------------*/
static void
xml_parse_label_round_node (xmlNodePtr   label_node,
			    lglTemplate *template)
{
	gchar               *id;
	gdouble              waste;
	gdouble              r;
	lglTemplateFrame    *frame;
	xmlNodePtr           node;

	id    = lgl_xml_get_prop_string (label_node, "id", NULL);
	waste = lgl_xml_get_prop_length (label_node, "waste", 0);
	r     = lgl_xml_get_prop_length (label_node, "radius", 0);

	frame = lgl_template_frame_round_new ((gchar *)id, r, waste);
	lgl_template_add_frame (template, frame);

	for (node = label_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (lgl_xml_is_node (node, "Layout")) {
			xml_parse_layout_node (node, frame);
		} else if (lgl_xml_is_node (node, "Markup-margin")) {
			xml_parse_markup_margin_node (node, frame);
		} else if (lgl_xml_is_node (node, "Markup-line")) {
			xml_parse_markup_line_node (node, frame);
		} else if (lgl_xml_is_node (node, "Markup-circle")) {
			xml_parse_markup_circle_node (node, frame);
		} else if (lgl_xml_is_node (node, "Markup-rect")) {
			xml_parse_markup_rect_node (node, frame);
		} else if (!xmlNodeIsText (node)) {
			if (!lgl_xml_is_node (node, "comment")) {
				g_message ("bad node =  \"%s\"",node->name);
			}
		}
	}

	g_free (id);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label-cd Node.                             */
/*--------------------------------------------------------------------------*/
static void
xml_parse_label_cd_node (xmlNodePtr   label_node,
			 lglTemplate *template)
{
	gchar               *id;
	gdouble              waste;
	gdouble              r1, r2, w, h;
	lglTemplateFrame    *frame;
	xmlNodePtr           node;

	id    = lgl_xml_get_prop_string (label_node, "id", NULL);
	waste = lgl_xml_get_prop_length (label_node, "waste", 0);
	r1    = lgl_xml_get_prop_length (label_node, "radius", 0);
	r2    = lgl_xml_get_prop_length (label_node, "hole", 0);
	w     = lgl_xml_get_prop_length (label_node, "width", 0);
	h     = lgl_xml_get_prop_length (label_node, "height", 0);

	frame = lgl_template_frame_cd_new ((gchar *)id, r1, r2, w, h, waste);
	lgl_template_add_frame (template, frame);

	for (node = label_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (lgl_xml_is_node (node, "Layout")) {
			xml_parse_layout_node (node, frame);
		} else if (lgl_xml_is_node (node, "Markup-margin")) {
			xml_parse_markup_margin_node (node, frame);
		} else if (lgl_xml_is_node (node, "Markup-line")) {
			xml_parse_markup_line_node (node, frame);
		} else if (lgl_xml_is_node (node, "Markup-circle")) {
			xml_parse_markup_circle_node (node, frame);
		} else if (lgl_xml_is_node (node, "Markup-rect")) {
			xml_parse_markup_rect_node (node, frame);
		} else if (!xmlNodeIsText (node)) {
			if (!lgl_xml_is_node (node, "comment")) {
				g_message ("bad node =  \"%s\"",node->name);
			}
		}
	}

	g_free (id);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label->Layout Node.                        */
/*--------------------------------------------------------------------------*/
static void
xml_parse_layout_node (xmlNodePtr          layout_node,
		       lglTemplateFrame   *frame)
{
	gint        nx, ny;
	gdouble     x0, y0, dx, dy;
	xmlNodePtr  node;

	nx = lgl_xml_get_prop_int (layout_node, "nx", 1);
	ny = lgl_xml_get_prop_int (layout_node, "ny", 1);

	x0 = lgl_xml_get_prop_length (layout_node, "x0", 0);
	y0 = lgl_xml_get_prop_length (layout_node, "y0", 0);

	dx = lgl_xml_get_prop_length (layout_node, "dx", 0);
	dy = lgl_xml_get_prop_length (layout_node, "dy", 0);

	lgl_template_frame_add_layout (frame, lgl_template_layout_new (nx, ny, x0, y0, dx, dy));

	for (node = layout_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (!xmlNodeIsText (node)) {
			if (!lgl_xml_is_node (node, "comment")) {
				g_message ("bad node =  \"%s\"",node->name);
			}
		}
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label->Markup-margin Node.                 */
/*--------------------------------------------------------------------------*/
static void
xml_parse_markup_margin_node (xmlNodePtr          markup_node,
			      lglTemplateFrame   *frame)
{
	gdouble     size;
	xmlNodePtr  node;

	size = lgl_xml_get_prop_length (markup_node, "size", 0);

	lgl_template_frame_add_markup (frame, lgl_template_markup_margin_new (size));

	for (node = markup_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (!xmlNodeIsText (node)) {
			if (!lgl_xml_is_node (node, "comment")) {
				g_message ("bad node =  \"%s\"",node->name);
			}
		}
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label->Markup-line Node.                   */
/*--------------------------------------------------------------------------*/
static void
xml_parse_markup_line_node (xmlNodePtr          markup_node,
			    lglTemplateFrame   *frame)
{
	gdouble     x1, y1, x2, y2;
	xmlNodePtr  node;

	x1 = lgl_xml_get_prop_length (markup_node, "x1", 0);
	y1 = lgl_xml_get_prop_length (markup_node, "y1", 0);
	x2 = lgl_xml_get_prop_length (markup_node, "x2", 0);
	y2 = lgl_xml_get_prop_length (markup_node, "y2", 0);

	lgl_template_frame_add_markup (frame, lgl_template_markup_line_new (x1, y1, x2, y2));

	for (node = markup_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (!xmlNodeIsText (node)) {
			if (!lgl_xml_is_node (node, "comment")) {
				g_message ("bad node =  \"%s\"",node->name);
			}
		}
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label->Markup-circle Node.                 */
/*--------------------------------------------------------------------------*/
static void
xml_parse_markup_circle_node (xmlNodePtr          markup_node,
			      lglTemplateFrame   *frame)
{
	gdouble     x0, y0, r;
	xmlNodePtr  node;

	x0 = lgl_xml_get_prop_length (markup_node, "x0", 0);
	y0 = lgl_xml_get_prop_length (markup_node, "y0", 0);
	r  = lgl_xml_get_prop_length (markup_node, "radius", 0);

	lgl_template_frame_add_markup (frame, lgl_template_markup_circle_new (x0, y0, r));

	for (node = markup_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (!xmlNodeIsText (node)) {
			if (!lgl_xml_is_node (node, "comment")) {
				g_message ("bad node =  \"%s\"",node->name);
			}
		}
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label->Markup-rect Node.                   */
/*--------------------------------------------------------------------------*/
static void
xml_parse_markup_rect_node (xmlNodePtr          markup_node,
			    lglTemplateFrame   *frame)
{
	gdouble     x1, y1, w, h, r;
	xmlNodePtr  node;

	x1 = lgl_xml_get_prop_length (markup_node, "x1", 0);
	y1 = lgl_xml_get_prop_length (markup_node, "y1", 0);
	w  = lgl_xml_get_prop_length (markup_node, "w", 0);
	h  = lgl_xml_get_prop_length (markup_node, "h", 0);
	r  = lgl_xml_get_prop_length (markup_node, "r", 0);

	lgl_template_frame_add_markup (frame, lgl_template_markup_rect_new (x1, y1, w, h, r));

	for (node = markup_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (!xmlNodeIsText (node)) {
			if (!lgl_xml_is_node (node, "comment")) {
				g_message ("bad node =  \"%s\"",node->name);
			}
		}
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Alias Node.                                */
/*--------------------------------------------------------------------------*/
static void
xml_parse_alias_node (xmlNodePtr   alias_node,
		      lglTemplate *template)
{
	gchar             *brand;
	gchar             *part;
	gchar             *name;
        gchar            **v;

	brand = lgl_xml_get_prop_string (alias_node, "brand", NULL);
	part  = lgl_xml_get_prop_string (alias_node, "part", NULL);
        if (!brand || !part)
        {
                name = lgl_xml_get_prop_string (alias_node, "name", NULL);
                if (name)
                {
			g_message (_("Missing required \"brand\" or \"part\" attribute, trying deprecated name."));
                        v = g_strsplit (name, " ", 2);
                        brand = g_strdup (v[0]);
                        part  = g_strdup (v[1]);
                        g_free (name);
                        g_strfreev (v);
                        
                }
                else
                {
			g_message (_("Name attribute also missing."));
                }
        }

	lgl_template_add_alias (template, lgl_template_alias_new (brand, part));

	g_free (brand);
	g_free (part);
}

/**
 * lgl_xml_template_write_templates_to_file:
 * @templates:      List of #lglTemplate structures
 * @utf8_filename:  Filename of templates file (name encoded as UTF-8)
 *
 * Write a list of #lglTemplate structures to a glabels XML template file.
 *
 * Returns: the number of bytes written or -1 in case of failure
 *
 */
gint
lgl_xml_template_write_templates_to_file (GList       *templates,
                                          const gchar *utf8_filename)
{
	xmlDocPtr    doc;
	xmlNsPtr     ns;
	gint         bytes_written;
	GList       *p;
	lglTemplate *template;
	gchar       *filename;

	doc = xmlNewDoc ((xmlChar *)"1.0");
	doc->xmlRootNode = xmlNewDocNode (doc, NULL, (xmlChar *)"Glabels-templates", NULL);

	ns = xmlNewNs (doc->xmlRootNode, (xmlChar *)LGL_XML_NAME_SPACE, NULL);
	xmlSetNs (doc->xmlRootNode, ns);

	for (p=templates; p!=NULL; p=p->next) {
		template = (lglTemplate *)p->data;
		lgl_xml_template_create_template_node (template, doc->xmlRootNode, ns);
	}

	filename = g_filename_from_utf8 (utf8_filename, -1, NULL, NULL, NULL);
	if (!filename)
        {
		g_message (_("Utf8 conversion error."));
                return -1;
        }
	else
        {
		xmlSetDocCompressMode (doc, 0);
		bytes_written = xmlSaveFormatFile (filename, doc, TRUE);
		xmlFreeDoc (doc);
		g_free (filename);
                return bytes_written;
	}

}


/**
 * lgl_xml_template_write_template_to_file:
 * @template:       #lglTemplate structure to be written
 * @utf8_filename:  Filename of templates file (name encoded as UTF-8)
 *
 * Write a single #lglTemplate structures to a glabels XML template file.
 *
 * Returns: the number of bytes written or -1 in case of failure
 *
 */
gint
lgl_xml_template_write_template_to_file (const lglTemplate  *template,
                                         const gchar        *utf8_filename)
{
	GList     *templates = NULL;
	gint       bytes_written;

	templates = g_list_append (templates, (gpointer)template);

	bytes_written = lgl_xml_template_write_templates_to_file (templates, utf8_filename);

	g_list_free (templates);

        return bytes_written;
}


/**
 * lgl_xml_template_create_template_node:
 * @template:       #lglTemplate structure to be written
 * @root:           parent node to receive new child node
 * @ns:             a libxml #xmlNsPtr
 *
 * Add a single #lglTemplate child node to given #xmlNodePtr.
 *
 */
void
lgl_xml_template_create_template_node (const lglTemplate *template,
                                       xmlNodePtr         root,
                                       const xmlNsPtr     ns)
{
	xmlNodePtr          node;
	GList              *p;
	lglTemplateAlias   *alias;
	lglTemplateFrame   *frame;

	node = xmlNewChild (root, ns, (xmlChar *)"Template", NULL);

	lgl_xml_set_prop_string (node, "brand", template->brand);
	lgl_xml_set_prop_string (node, "part", template->part);

	lgl_xml_set_prop_string (node, "size", template->paper_id);
	if (xmlStrEqual ((xmlChar *)template->paper_id, (xmlChar *)"Other"))
        {

		lgl_xml_set_prop_length (node, "width", template->page_width);
		lgl_xml_set_prop_length (node, "height", template->page_height);

	}

	lgl_xml_set_prop_string (node, "description", template->description);

	for ( p=template->aliases; p != NULL; p=p->next ) {
                alias = (lglTemplateAlias *)p->data;
		if ( !(xmlStrEqual ((xmlChar *)template->brand, (xmlChar *)alias->brand) &&
                       xmlStrEqual ((xmlChar *)template->part, (xmlChar *)alias->part)) )
                {
			xml_create_alias_node ( alias, node, ns );
		}
	}
	for ( p=template->category_ids; p != NULL; p=p->next )
        {
                xml_create_meta_node ( p->data, node, ns );
	}
	for ( p=template->frames; p != NULL; p=p->next )
        {
		frame = (lglTemplateFrame *)p->data;
		xml_create_label_node (frame, node, ns);
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Meta Node.                                   */
/*--------------------------------------------------------------------------*/
static void
xml_create_meta_node (const gchar      *category,
		      xmlNodePtr        root,
		      const xmlNsPtr    ns)
{
	xmlNodePtr node;

	node = xmlNewChild (root, ns, (xmlChar *)"Meta", NULL);
	lgl_xml_set_prop_string (node, "category", category);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Label Node.                                  */
/*--------------------------------------------------------------------------*/
static void
xml_create_label_node (const lglTemplateFrame  *frame,
		       xmlNodePtr               root,
		       const xmlNsPtr           ns)
{
	xmlNodePtr        node;
	GList            *p;
	lglTemplateMarkup *markup;
	lglTemplateLayout *layout;

	switch (frame->shape) {

	case LGL_TEMPLATE_FRAME_SHAPE_RECT:
		node = xmlNewChild(root, ns, (xmlChar *)"Label-rectangle", NULL);
		lgl_xml_set_prop_string (node, "id",      frame->all.id);
		lgl_xml_set_prop_length (node, "width",   frame->rect.w);
		lgl_xml_set_prop_length (node, "height",  frame->rect.h);
		lgl_xml_set_prop_length (node, "round",   frame->rect.r);
		lgl_xml_set_prop_length (node, "x_waste", frame->rect.x_waste);
		lgl_xml_set_prop_length (node, "y_waste", frame->rect.y_waste);
		break;

	case LGL_TEMPLATE_FRAME_SHAPE_ROUND:
		node = xmlNewChild(root, ns, (xmlChar *)"Label-round", NULL);
		lgl_xml_set_prop_string (node, "id",      frame->all.id);
		lgl_xml_set_prop_length (node, "radius",  frame->round.r);
		lgl_xml_set_prop_length (node, "waste",   frame->round.waste);
		break;

	case LGL_TEMPLATE_FRAME_SHAPE_CD:
		node = xmlNewChild(root, ns, (xmlChar *)"Label-cd", NULL);
		lgl_xml_set_prop_string (node, "id",     frame->all.id);
		lgl_xml_set_prop_length (node, "radius", frame->cd.r1);
		lgl_xml_set_prop_length (node, "hole",   frame->cd.r2);
		if (frame->cd.w != 0.0) {
			lgl_xml_set_prop_length (node, "width",  frame->cd.w);
		}
		if (frame->cd.h != 0.0) {
			lgl_xml_set_prop_length (node, "height", frame->cd.h);
		}
		lgl_xml_set_prop_length (node, "waste",  frame->cd.waste);
		break;

	default:
		g_message ("Unknown label style");
		return;
		break;

	}

	for ( p=frame->all.markups; p != NULL; p=p->next ) {
		markup = (lglTemplateMarkup *)p->data;
		switch (markup->type) {
		case LGL_TEMPLATE_MARKUP_MARGIN:
			xml_create_markup_margin_node (markup, node, ns);
			break;
		case LGL_TEMPLATE_MARKUP_LINE:
			xml_create_markup_line_node (markup, node, ns);
			break;
		case LGL_TEMPLATE_MARKUP_CIRCLE:
			xml_create_markup_circle_node (markup, node, ns);
			break;
		case LGL_TEMPLATE_MARKUP_RECT:
			xml_create_markup_rect_node (markup, node, ns);
			break;
		default:
			g_message ("Unknown markup type");
			break;
		}
	}

	for ( p=frame->all.layouts; p != NULL; p=p->next ) {
		layout = (lglTemplateLayout *)p->data;
		xml_create_layout_node (layout, node, ns);
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Label->Layout Node.                          */
/*--------------------------------------------------------------------------*/
static void
xml_create_layout_node (const lglTemplateLayout *layout,
			xmlNodePtr               root,
			const xmlNsPtr           ns)
{
	xmlNodePtr  node;

	node = xmlNewChild(root, ns, (xmlChar *)"Layout", NULL);
	lgl_xml_set_prop_int (node, "nx", layout->nx);
	lgl_xml_set_prop_int (node, "ny", layout->ny);
	lgl_xml_set_prop_length (node, "x0", layout->x0);
	lgl_xml_set_prop_length (node, "y0", layout->y0);
	lgl_xml_set_prop_length (node, "dx", layout->dx);
	lgl_xml_set_prop_length (node, "dy", layout->dy);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Label->Markup-margin Node.                   */
/*--------------------------------------------------------------------------*/
static void
xml_create_markup_margin_node (const lglTemplateMarkup  *markup,
			       xmlNodePtr                root,
			       const xmlNsPtr            ns)
{
	xmlNodePtr  node;

	node = xmlNewChild(root, ns, (xmlChar *)"Markup-margin", NULL);

	lgl_xml_set_prop_length (node, "size", markup->margin.size);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Label->Markup-line Node.                     */
/*--------------------------------------------------------------------------*/
static void
xml_create_markup_line_node (const lglTemplateMarkup *markup,
			     xmlNodePtr               root,
			     const xmlNsPtr           ns)
{
	xmlNodePtr  node;

	node = xmlNewChild(root, ns, (xmlChar *)"Markup-line", NULL);

	lgl_xml_set_prop_length (node, "x1", markup->line.x1);
	lgl_xml_set_prop_length (node, "y1", markup->line.y1);
	lgl_xml_set_prop_length (node, "x2", markup->line.x2);
	lgl_xml_set_prop_length (node, "y2", markup->line.y2);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Label->Markup-circle Node.                   */
/*--------------------------------------------------------------------------*/
static void
xml_create_markup_circle_node (const lglTemplateMarkup *markup,
			       xmlNodePtr               root,
			       const xmlNsPtr           ns)
{
	xmlNodePtr  node;

	node = xmlNewChild(root, ns, (xmlChar *)"Markup-circle", NULL);

	lgl_xml_set_prop_length (node, "x0",     markup->circle.x0);
	lgl_xml_set_prop_length (node, "y0",     markup->circle.y0);
	lgl_xml_set_prop_length (node, "radius", markup->circle.r);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Label->Markup-rect Node.                     */
/*--------------------------------------------------------------------------*/
static void
xml_create_markup_rect_node (const lglTemplateMarkup *markup,
			     xmlNodePtr               root,
			     const xmlNsPtr           ns)
{
	xmlNodePtr  node;

	node = xmlNewChild(root, ns, (xmlChar *)"Markup-rect", NULL);

	lgl_xml_set_prop_length (node, "x1", markup->rect.x1);
	lgl_xml_set_prop_length (node, "y1", markup->rect.y1);
	lgl_xml_set_prop_length (node, "w",  markup->rect.w);
	lgl_xml_set_prop_length (node, "h",  markup->rect.h);
	lgl_xml_set_prop_length (node, "r",  markup->rect.r);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Alias Node.                                  */
/*--------------------------------------------------------------------------*/
static void
xml_create_alias_node (const lglTemplateAlias *alias,
		       xmlNodePtr              root,
		       const xmlNsPtr          ns)
{
	xmlNodePtr node;

	node = xmlNewChild (root, ns, (xmlChar *)"Alias", NULL);

	lgl_xml_set_prop_string (node, "brand", alias->brand);
	lgl_xml_set_prop_string (node, "part",  alias->part);

}


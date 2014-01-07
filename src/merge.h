/*
 *  merge.h
 *  Copyright (C) 2002-2009  Jim Evins <evins@snaught.com>.
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

#ifndef __MERGE_H__
#define __MERGE_H__

#include <glib-object.h>

G_BEGIN_DECLS

typedef enum {
	GL_MERGE_SRC_IS_FIXED,
	GL_MERGE_SRC_IS_FILE
} glMergeSrcType;

typedef struct {
	gchar *key;
	gchar *value;
} glMergeField;

typedef struct {
	gboolean select_flag;
	GList    *field_list;  /* List of glMergeFields */
} glMergeRecord;


#define GL_TYPE_MERGE              (gl_merge_get_type ())
#define GL_MERGE(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_MERGE, glMerge))
#define GL_MERGE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_MERGE, glMergeClass))
#define GL_IS_MERGE(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_MERGE))
#define GL_IS_MERGE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_MERGE))
#define GL_MERGE_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), GL_TYPE_MERGE, glMergeClass))


typedef struct _glMerge          glMerge;
typedef struct _glMergeClass     glMergeClass;

typedef struct _glMergePrivate   glMergePrivate;


struct _glMerge {
	GObject          object;

	glMergePrivate  *priv;
};

struct _glMergeClass {
	GObjectClass     parent_class;

        GList         *(*get_key_list)    (const glMerge *merge);

        gchar         *(*get_primary_key) (const glMerge *merge);

	void           (*open)            (glMerge       *merge);

	void           (*close)           (glMerge       *merge);

	glMergeRecord *(*get_record)      (glMerge       *merge);

	void           (*copy)            (glMerge       *dst_merge,
					   const glMerge *src_merge);
};


void              gl_merge_register_backend    (GType              type,
						gchar             *name,
						gchar             *description,
						glMergeSrcType     src_type,
						const gchar       *first_arg_name,
						...);

GList            *gl_merge_get_descriptions    (void);

void              gl_merge_free_descriptions   (GList **descriptions);

gchar            *gl_merge_description_to_name (gchar *description);

GType             gl_merge_get_type            (void) G_GNUC_CONST;

glMerge          *gl_merge_new                 (const gchar         *name);

glMerge          *gl_merge_dup                 (const glMerge       *orig);

gchar            *gl_merge_get_name            (const glMerge       *merge);

gchar            *gl_merge_get_description     (const glMerge       *merge);

glMergeSrcType    gl_merge_get_src_type        (const glMerge       *merge);

void              gl_merge_set_src             (glMerge             *merge,
						const gchar         *src);

gchar            *gl_merge_get_src             (const glMerge       *merge);

GList            *gl_merge_get_key_list        (const glMerge       *merge);

void              gl_merge_free_key_list       (GList              **keys);

gchar            *gl_merge_get_primary_key     (const glMerge       *merge);

gchar            *gl_merge_eval_key            (const glMergeRecord *record,
                                                const gchar         *key);

const GList      *gl_merge_get_record_list     (const glMerge       *merge);

gint              gl_merge_get_record_count    (const glMerge       *merge);

G_END_DECLS

#endif



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */

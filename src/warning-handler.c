/*
 *  warning-handler.c
 *  Copyright (C) 2005-2009  Jim Evins <evins@snaught.com>.
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

#include "warning-handler.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <stdlib.h>

static void warning_handler (const gchar    *log_domain,
                             GLogLevelFlags  log_level,
                             const gchar    *message,
                             gpointer        user_data);


/***************************************************************************/
/* Initialize error handler.                                               */
/***************************************************************************/
void
gl_warning_handler_init (void)
{
        g_log_set_handler ("LibGlabels",
                           G_LOG_LEVEL_WARNING,
                           warning_handler,
                           "libglabels");

        g_log_set_handler (G_LOG_DOMAIN,
                           G_LOG_LEVEL_WARNING,
                           warning_handler,
                           "glabels");
}


/*-------------------------------------------------------------------------*/
/* PRIVATE.  Actual error handler.                                         */
/*-------------------------------------------------------------------------*/
static void
warning_handler (const gchar    *log_domain,
                 GLogLevelFlags  log_level,
                 const gchar    *message,
                 gpointer        user_data)
{
        GtkWidget *dialog;

        dialog = gtk_message_dialog_new (NULL,
                                         GTK_DIALOG_MODAL,
                                         GTK_MESSAGE_WARNING,
                                         GTK_BUTTONS_CLOSE,
                                         _("gLabels Error!"));
        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                                  "%s", message);

        gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_widget_destroy (GTK_WIDGET (dialog));
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */

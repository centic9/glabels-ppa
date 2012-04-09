
#ifndef __gl_marshal_MARSHAL_H__
#define __gl_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:VOID (marshal.list:1) */
#define gl_marshal_VOID__VOID	g_cclosure_marshal_VOID__VOID

/* VOID:INT (marshal.list:2) */
#define gl_marshal_VOID__INT	g_cclosure_marshal_VOID__INT

/* VOID:INT,INT (marshal.list:3) */
extern void gl_marshal_VOID__INT_INT (GClosure     *closure,
                                      GValue       *return_value,
                                      guint         n_param_values,
                                      const GValue *param_values,
                                      gpointer      invocation_hint,
                                      gpointer      marshal_data);

/* VOID:INT,UINT (marshal.list:4) */
extern void gl_marshal_VOID__INT_UINT (GClosure     *closure,
                                       GValue       *return_value,
                                       guint         n_param_values,
                                       const GValue *param_values,
                                       gpointer      invocation_hint,
                                       gpointer      marshal_data);

/* VOID:DOUBLE (marshal.list:5) */
#define gl_marshal_VOID__DOUBLE	g_cclosure_marshal_VOID__DOUBLE

/* VOID:DOUBLE,DOUBLE (marshal.list:6) */
extern void gl_marshal_VOID__DOUBLE_DOUBLE (GClosure     *closure,
                                            GValue       *return_value,
                                            guint         n_param_values,
                                            const GValue *param_values,
                                            gpointer      invocation_hint,
                                            gpointer      marshal_data);

/* VOID:OBJECT (marshal.list:7) */
#define gl_marshal_VOID__OBJECT	g_cclosure_marshal_VOID__OBJECT

/* VOID:STRING (marshal.list:8) */
#define gl_marshal_VOID__STRING	g_cclosure_marshal_VOID__STRING

/* VOID:UINT,BOOLEAN (marshal.list:9) */
extern void gl_marshal_VOID__UINT_BOOLEAN (GClosure     *closure,
                                           GValue       *return_value,
                                           guint         n_param_values,
                                           const GValue *param_values,
                                           gpointer      invocation_hint,
                                           gpointer      marshal_data);

G_END_DECLS

#endif /* __gl_marshal_MARSHAL_H__ */


/*
 * Copyright © 2018 Benjamin Otte
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Benjamin Otte <otte@gnome.org>
 */

#include "config.h"

#include "gtklistitemprivate.h"

#include "gtkbinlayout.h"
#include "gtkintl.h"
#include "gtkwidget.h"

/**
 * SECTION:gtklistitem
 * @title: GtkListItem
 * @short_description: Object used to represent items of a ListModel
 * @see_also: #GtkListView, #GListModel
 *
 * #GtkListItem is the object that list-handling containers such
 * as #GtkListView use to represent items in a #GListModel. They are
 * managed by the container and cannot be created by application code.
 *
 * #GtkListItems need to be populated by application code. This is done by
 * calling gtk_list_item_set_child().
 *
 * #GtkListItems exist in 2 stages:
 *
 * 1. The unbound stage where the listitem is not currently connected to
 *    an item in the list. In that case, the #GtkListItem:item property is
 *    set to %NULL.
 *
 * 2. The bound stage where the listitem references an item from the list.
 *    The #GtkListItem:item property is not %NULL.
 */

struct _GtkListItem
{
  GtkWidget parent_instance;

  GObject *item;
  GtkWidget *child;
};

struct _GtkListItemClass
{
  GtkWidgetClass parent_class;
};

enum
{
  PROP_0,
  PROP_CHILD,
  PROP_ITEM,

  N_PROPS
};

G_DEFINE_TYPE (GtkListItem, gtk_list_item, GTK_TYPE_WIDGET)

static GParamSpec *properties[N_PROPS] = { NULL, };

static void
gtk_list_item_dispose (GObject *object)
{
  GtkListItem *self = GTK_LIST_ITEM (object);

  g_assert (self->item == NULL);
  g_clear_pointer (&self->child, gtk_widget_unparent);

  G_OBJECT_CLASS (gtk_list_item_parent_class)->dispose (object);
}

static void
gtk_list_item_get_property (GObject    *object,
                            guint       property_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  GtkListItem *self = GTK_LIST_ITEM (object);

  switch (property_id)
    {
    case PROP_CHILD:
      g_value_set_object (value, self->child);
      break;

    case PROP_ITEM:
      g_value_set_object (value, self->item);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gtk_list_item_set_property (GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  GtkListItem *self = GTK_LIST_ITEM (object);

  switch (property_id)
    {
    case PROP_CHILD:
      gtk_list_item_set_child (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gtk_list_item_class_init (GtkListItemClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->dispose = gtk_list_item_dispose;
  gobject_class->get_property = gtk_list_item_get_property;
  gobject_class->set_property = gtk_list_item_set_property;

  /**
   * GtkListItem:child:
   *
   * Widget used for display
   */
  properties[PROP_CHILD] =
    g_param_spec_object ("child",
                         P_("Child"),
                         P_("Widget used for display"),
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  /**
   * GtkListItem:item:
   *
   * Displayed item
   */
  properties[PROP_ITEM] =
    g_param_spec_object ("item",
                         P_("Item"),
                         P_("Displayed item"),
                         G_TYPE_OBJECT,
                         G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, properties);

  /* This gets overwritten by gtk_list_item_new() but better safe than sorry */
  gtk_widget_class_set_css_name (widget_class, I_("row"));
  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
}

static void
gtk_list_item_init (GtkListItem *self)
{
}

GtkWidget *
gtk_list_item_new (const char *css_name)
{
  g_return_val_if_fail (css_name != NULL, NULL);

  return g_object_new (GTK_TYPE_LIST_ITEM,
                       "css-name", css_name,
                       NULL);
}

/**
 * gtk_list_item_get_item:
 * @self: a #GtkListItem
 *
 * Gets the item that is currently displayed or model that @self is
 * currently bound to or %NULL if @self is unbound.
 *
 * Returns: (nullable) (transfer none) (type GObject): The model in use
 **/
gpointer
gtk_list_item_get_item (GtkListItem *self)
{
  g_return_val_if_fail (GTK_IS_LIST_ITEM (self), NULL);

  return self->item;
}

/**
 * gtk_list_item_get_child:
 * @self: a #GtkListItem
 *
 * Gets the child previously set via gtk_list_item_set_child() or
 * %NULL if none was set.
 *
 * Returns: (transfer none) (nullable): The child
 **/
GtkWidget *
gtk_list_item_get_child (GtkListItem *self)
{
  g_return_val_if_fail (GTK_IS_LIST_ITEM (self), NULL);

  return self->child;
}

/**
 * gtk_list_item_set_child:
 * @self: a #GtkListItem
 * @child: (nullable): The list item's child or %NULL to unset
 *
 * Sets the child to be used for this listitem.
 *
 * This function is typically called by applications when
 * setting up a listitem so that the widget can be reused when
 * binding it multiple times.
 **/
void
gtk_list_item_set_child (GtkListItem *self,
                         GtkWidget   *child)
{
  g_return_if_fail (GTK_IS_LIST_ITEM (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (self->child == child)
    return;

  g_clear_pointer (&self->child, gtk_widget_unparent);

  if (child)
    {
      gtk_widget_insert_after (child, GTK_WIDGET (self), NULL);
      self->child = child;
    }

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ITEM]);
}

void
gtk_list_item_bind (GtkListItem *self,
                    gpointer     item)
{
  g_return_if_fail (GTK_IS_LIST_ITEM (self));
  g_return_if_fail (G_IS_OBJECT (item));
  /* Must unbind before rebinding */
  g_return_if_fail (self->item == NULL);

  self->item = g_object_ref (item);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ITEM]);
}

void
gtk_list_item_unbind (GtkListItem *self)
{
  g_return_if_fail (GTK_IS_LIST_ITEM (self));
  /* Must be bound */
  g_return_if_fail (self->item != NULL);

  g_clear_object (&self->item);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ITEM]);
}

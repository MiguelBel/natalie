C.include '<gtk/gtk.h>'

C.top_eval <<-END
  void gtk3_signal_callback(GtkWidget *widget, gpointer data) {
    NatObject *callback = (NatObject*)data;
    nat_send(callback->block->env, callback, "call", 0, NULL, NULL);
  }
END

module Gtk3
  WINDOW_TOPLEVEL = 0

  ORIENTATION_HORIZONTAL = 0
  ORIENTATION_VERTICAL = 1

  ALIGN_START = 1
  ALIGN_END = 2

  class Widget
    def set_halign(align)
      Gtk3::widget_set_halign(self, align)
    end

    def set_valign(align)
      Gtk3::widget_set_valign(self, align)
    end

    def signal_connect(signal, callback)
      Gtk3.g_signal_connect(self, signal, callback)
    end
  end

  class Window < Widget
    def self.new(type)
      Gtk3.window_new(type)
    end

    def set_title(title)
      Gtk3.window_set_title(self, title)
    end

    def container_add(container)
      Gtk3.container_add(self, container)
    end

    def set_border_width(width)
      Gtk3.container_set_border_width(self, width)
    end
  end

  class Box < Widget
    def self.new(orientation, spacing)
      Gtk3.box_new(orientation, spacing)
    end

    def pack_start(child, expand, fill, padding)
      Gtk3.box_pack_start(self, child, expand, fill, padding)
    end
  end

  class Image < Widget
    def self.new_from_file(filename)
      Gtk3.image_new_from_file(filename)
    end
  end

  class Label < Widget
    def self.new(text)
      Gtk3.label_new(text)
    end

    def set_markup(markup)
      Gtk3.label_set_markup(self, markup)
    end
  end

  class Button < Widget
    def self.new_with_label(label)
      Gtk3::button_new_with_label(label)
    end
  end

  class << self
    def init
      C.eval <<-END
        int main_argc = 0;
        gtk_init(&main_argc, NULL);
      END
    end

    C.define_method :window_new, <<-END
      NAT_ASSERT_ARGC(1);
      int type;
      nat_arg_spread(env, argc, args, "i", &type);
      GtkWidget *gtk_window = gtk_window_new(type);
      NatObject *Window = nat_const_get(env, self, "Window", true);
      NatObject *window_wrapper = nat_alloc(env, Window, NAT_VALUE_OTHER);
      NatObject *ptr = nat_void_ptr(env, gtk_window);
      nat_ivar_set(env, window_wrapper, "@_ptr", ptr);
      return window_wrapper;
    END

    C.define_method :widget_show_all, <<-END
      NAT_ASSERT_ARGC(1);
      GtkWidget *gtk_window;
      nat_arg_spread(env, argc, args, "v", &gtk_window);
      gtk_widget_show_all(gtk_window);
      return NAT_NIL;
    END

    C.define_method :window_set_title, <<-END
      NAT_ASSERT_ARGC(2);
      GtkWidget *gtk_window;
      char *title;
      nat_arg_spread(env, argc, args, "vs", &gtk_window, &title);
      gtk_window_set_title(GTK_WINDOW(gtk_window), title);
      return NAT_NIL;
    END

    C.define_method :box_new, <<-END
      NAT_ASSERT_ARGC(2);
      int orientation, spacing;
      nat_arg_spread(env, argc, args, "ii", &orientation, &spacing);
      GtkWidget *gtk_box = gtk_box_new(orientation, spacing);
      NatObject *Box = nat_const_get(env, self, "Box", true);
      NatObject *box_wrapper = nat_alloc(env, Box, NAT_VALUE_OTHER);
      NatObject *ptr = nat_void_ptr(env, gtk_box);
      nat_ivar_set(env, box_wrapper, "@_ptr", ptr);
      return box_wrapper;
    END

    C.define_method :container_add, <<-END
      NAT_ASSERT_ARGC(2);
      GtkWidget *gtk_window, *gtk_container;
      nat_arg_spread(env, argc, args, "vv", &gtk_window, &gtk_container);
      gtk_container_add(GTK_CONTAINER(gtk_window), gtk_container);
      return NAT_NIL;
    END

    C.define_method :image_new_from_file, <<-END
      NAT_ASSERT_ARGC(1);
      char *filename;
      nat_arg_spread(env, argc, args, "s", &filename);
      GtkWidget *gtk_image = gtk_image_new_from_file(filename);
      NatObject *Image = nat_const_get(env, self, "Image", true);
      NatObject *image_wrapper = nat_alloc(env, Image, NAT_VALUE_OTHER);
      NatObject *ptr = nat_void_ptr(env, gtk_image);
      nat_ivar_set(env, image_wrapper, "@_ptr", ptr);
      return image_wrapper;
    END

    C.define_method :box_pack_start, <<-END
      NAT_ASSERT_ARGC(5);
      GtkWidget *gtk_box, *gtk_child;
      bool expand, fill;
      int padding;
      nat_arg_spread(env, argc, args, "vvbbi", &gtk_box, &gtk_child, &expand, &fill, &padding);
      gtk_box_pack_start(GTK_BOX(gtk_box), gtk_child, expand, fill, padding);
      return NAT_NIL;
    END

    C.define_method :widget_set_halign, <<-END
      NAT_ASSERT_ARGC(2);
      GtkWidget *gtk_widget;
      int align;
      nat_arg_spread(env, argc, args, "vi", &gtk_widget, &align);
      gtk_widget_set_halign(gtk_widget, align);
      return NAT_NIL;
    END

    C.define_method :widget_set_valign, <<-END
      NAT_ASSERT_ARGC(2);
      GtkWidget *gtk_widget;
      int align;
      nat_arg_spread(env, argc, args, "vi", &gtk_widget, &align);
      gtk_widget_set_valign(gtk_widget, align);
      return NAT_NIL;
    END

    C.define_method :label_new, <<-END
      NAT_ASSERT_ARGC(1);
      char *text;
      nat_arg_spread(env, argc, args, "s", &text);
      GtkWidget *gtk_label = gtk_label_new(text);
      NatObject *Label = nat_const_get(env, self, "Label", true);
      NatObject *label_wrapper = nat_alloc(env, Label, NAT_VALUE_OTHER);
      NatObject *ptr = nat_void_ptr(env, gtk_label);
      nat_ivar_set(env, label_wrapper, "@_ptr", ptr);
      return label_wrapper;
    END

    C.define_method :label_set_markup, <<-END
      NAT_ASSERT_ARGC(2);
      GtkWidget *gtk_label;
      char *markup;
      nat_arg_spread(env, argc, args, "vs", &gtk_label, &markup);
      gtk_label_set_markup(GTK_LABEL(gtk_label), markup);
      return NAT_NIL;
    END

    C.define_method :button_new_with_label, <<-END
      NAT_ASSERT_ARGC(1);
      char *label;
      nat_arg_spread(env, argc, args, "s", &label);
      GtkWidget *gtk_button = gtk_button_new_with_label(label);
      NatObject *Button = nat_const_get(env, self, "Button", true);
      NatObject *button_wrapper = nat_alloc(env, Button, NAT_VALUE_OTHER);
      NatObject *ptr = nat_void_ptr(env, gtk_button);
      nat_ivar_set(env, button_wrapper, "@_ptr", ptr);
      return button_wrapper;
    END

    C.define_method :container_set_border_width, <<-END
      NAT_ASSERT_ARGC(2);
      GtkWidget *container;
      int width;
      nat_arg_spread(env, argc, args, "vi", &container, &width);
      gtk_container_set_border_width(GTK_CONTAINER(container), width);
      return NAT_NIL;
    END

    C.define_method :g_signal_connect, <<-END
      NAT_ASSERT_ARGC(3);
      GObject *instance;
      char *signal;
      NatObject *callback;
      nat_arg_spread(env, argc, args, "vso", &instance, &signal, &callback);
      g_signal_connect(instance, signal, G_CALLBACK(gtk3_signal_callback), callback);
      return NAT_NIL;
    END

    def main
      C.eval <<-END
        gtk_main();
      END
    end

    def main_quit
      C.eval <<-END
        gtk_main_quit();
      END
    end
  end
end

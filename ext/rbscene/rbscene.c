#include "ruby.h"
#include "raylib.h"

static VALUE engine_run(VALUE self, VALUE objects)
{
    Check_Type(objects, T_ARRAY);

    InitWindow(800, 600, "Ruby + raylib");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        long len = RARRAY_LEN(objects);
        for (int i = 0; i < len; i++)
        {
            VALUE obj = rb_ary_entry(objects, i);

            if (rb_respond_to(obj, rb_intern("update")))
                rb_funcall(obj, rb_intern("update"), 0);

            if (rb_respond_to(obj, rb_intern("draw")))
                rb_funcall(obj, rb_intern("draw"), 0);
        }

        EndDrawing();
    }

    CloseWindow();
    return Qnil;
}

static VALUE draw_rectangle(VALUE self, VALUE x, VALUE y, VALUE w, VALUE h)
{
    DrawRectangle(NUM2INT(x), NUM2INT(y), NUM2INT(w), NUM2INT(h), RED);
    return Qnil;
}

void Init_rbscene(void)
{
    VALUE rbscene_module = rb_define_module("RBScene");
    VALUE engine_class = rb_define_class_under(rbscene_module, "Engine", rb_cObject);
    rb_define_singleton_method(engine_class, "run", engine_run, 1);

    VALUE bindings_module = rb_define_module_under(rbscene_module, "Bindings");
    rb_define_module_function(bindings_module, "draw_rectangle", draw_rectangle, 4);
}

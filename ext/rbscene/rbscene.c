#include "ruby.h"
#include "raylib.h"

typedef struct
{
    Texture2D texture;
} RBTexture;

typedef struct
{
    Music music;
} RBMusic;

typedef struct
{
    Sound sound;
} RBSound;

static void texture_free(void *ptr)
{
    RBTexture *tex = (RBTexture*)ptr;
    UnloadTexture(tex->texture);
    ruby_xfree(ptr);
}

static const rb_data_type_t texture_type =
{
    "RBScene::Texture",
    { 0, texture_free, 0 },
    0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};

static void music_free(void *ptr)
{
    RBMusic *music = (RBMusic*)ptr;
    UnloadMusicStream(music->music);
    ruby_xfree(ptr);
}

static const rb_data_type_t music_type =
{
    "RBScene::Music",
    { 0, music_free, 0 },
    0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};

static void sound_free(void *ptr)
{
    RBSound *sound = (RBSound*)ptr;
    UnloadSound(sound->sound);
    ruby_xfree(ptr);
}

static const rb_data_type_t sound_type =
{
    "RBScene::Sound",
    { 0, sound_free, 0 },
    0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE keycode_to_symbol(int keycode)
{
    switch (keycode)
    {
        case KEY_SPACE: return ID2SYM(rb_intern("space"));
        case KEY_ENTER: return ID2SYM(rb_intern("enter"));
        case KEY_Z:     return ID2SYM(rb_intern("z"));
        case KEY_X:     return ID2SYM(rb_intern("x"));
        case KEY_UP:    return ID2SYM(rb_intern("up"));
        case KEY_DOWN:    return ID2SYM(rb_intern("down"));
        case KEY_LEFT:    return ID2SYM(rb_intern("left"));
        case KEY_RIGHT:    return ID2SYM(rb_intern("right"));
        default: return Qnil;
    }
}

// global engine variables
static Camera2D cam = { .zoom = 2 };

static VALUE engine_run(VALUE self, VALUE objects)
{
    Check_Type(objects, T_ARRAY);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // handle every keycode
        for (int key = KEY_NULL + 1; key <= KEY_KB_MENU; key++)
        {
            if (IsKeyPressed(key)) {
                VALUE key_sym = keycode_to_symbol(key);
                if (key_sym == Qnil) continue;

                for (int i = 0; i < RARRAY_LEN(objects); i++) {
                    VALUE obj = rb_ary_entry(objects, i);

                    if (rb_respond_to(obj, rb_intern("handle_event"))) {
                        rb_funcall(obj, rb_intern("handle_event"), 2,
                                ID2SYM(rb_intern("keypress")), key_sym);
                    }
                }
            }
            else if (IsKeyDown(key))
            {
                VALUE key_sym = keycode_to_symbol(key);
                if (key_sym == Qnil) continue;

                for (int i = 0; i < RARRAY_LEN(objects); i++) {
                    VALUE obj = rb_ary_entry(objects, i);

                    if (rb_respond_to(obj, rb_intern("handle_event"))) {
                        rb_funcall(obj, rb_intern("handle_event"), 2,
                                ID2SYM(rb_intern("keydown")), key_sym);
                    }
                }
            }
        }

        long len = RARRAY_LEN(objects);
        for (int i = 0; i < len; i++)
        {
            VALUE obj = rb_ary_entry(objects, i);

            if (rb_respond_to(obj, rb_intern("update")))
                rb_funcall(obj, rb_intern("update"), 0);

            // slow, need to update all objects at once
            BeginMode2D(cam);

            if (rb_respond_to(obj, rb_intern("draw")))
                rb_funcall(obj, rb_intern("draw"), 0);

            EndMode2D();
        }

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return Qnil;
}

static VALUE texture_load(VALUE self, VALUE filename)
{
    Check_Type(filename, T_STRING);

    RBTexture *tex;
    VALUE obj = TypedData_Make_Struct(self, RBTexture, &texture_type, tex);

    tex->texture = LoadTexture(StringValueCStr(filename));
    return obj;
}

static VALUE texture_draw(VALUE self, VALUE x, VALUE y)
{
    Check_Type(x, T_FIXNUM);
    Check_Type(y, T_FIXNUM);

    RBTexture *tex;
    TypedData_Get_Struct(self, RBTexture, &texture_type, tex);

    DrawTexture(tex->texture, NUM2INT(x), NUM2INT(y), WHITE);
    return Qnil;
}

static VALUE music_load(VALUE self, VALUE filename)
{
    Check_Type(filename, T_STRING);

    RBMusic *music;
    VALUE obj = TypedData_Make_Struct(self, RBMusic, &music_type, music);

    music->music = LoadMusicStream(StringValueCStr(filename));
    return obj;
}

static VALUE music_play(VALUE self)
{
    RBMusic *music;
    TypedData_Get_Struct(self, RBMusic, &music_type, music);
    PlayMusicStream(music->music);
    return Qnil;
}

// absolutely should not exist
static VALUE music_update(VALUE self)
{
    RBMusic *music;
    TypedData_Get_Struct(self, RBMusic, &music_type, music);
    UpdateMusicStream(music->music);
    return Qnil;
}

static VALUE sound_load(VALUE self, VALUE filename)
{
    Check_Type(filename, T_STRING);

    RBSound *sound;
    VALUE obj = TypedData_Make_Struct(self, RBSound, &sound_type, sound);

    sound->sound = LoadSound(StringValueCStr(filename));
    return obj;
}

static VALUE sound_play(VALUE self)
{
    RBSound *sound;
    TypedData_Get_Struct(self, RBSound, &sound_type, sound);
    PlaySound(sound->sound);
    return Qnil;
}

void Init_rbscene(void)
{
    // init bindings
    VALUE rbscene_module = rb_define_module("RBScene");
    VALUE engine_class = rb_define_class_under(rbscene_module, "Engine", rb_cObject);
    rb_define_singleton_method(engine_class, "run", engine_run, 1);

    VALUE texture_class = rb_define_class_under(rbscene_module, "Texture", rb_cObject);
    rb_define_singleton_method(texture_class, "load", texture_load, 1);
    rb_define_method(texture_class, "draw", texture_draw, 2);

    VALUE music_class = rb_define_class_under(rbscene_module, "Music", rb_cObject);
    rb_define_singleton_method(music_class, "load", music_load, 1);
    rb_define_method(music_class, "play", music_play, 0);
    rb_define_method(music_class, "update", music_update, 0);

    VALUE sound_class = rb_define_class_under(rbscene_module, "Sound", rb_cObject);
    rb_define_singleton_method(sound_class, "load", sound_load, 1);
    rb_define_method(sound_class, "play", sound_play, 0);

    // init raylib
    InitWindow(320, 288, "Game");
    InitAudioDevice();
    SetTargetFPS(60);
}

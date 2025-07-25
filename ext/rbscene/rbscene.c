#include "ruby.h"
#include "raylib.h"

// these are set on init
static VALUE rbscene_module = Qnil;
static VALUE game_object_class = Qnil;

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

// global engine variables
static Camera2D cam = { .zoom = 2 };

static VALUE engine_run(VALUE self)
{
    while (!WindowShouldClose())
    {
        // fetch the current scene's objects
        VALUE scene = rb_iv_get(rbscene_module, "@current_scene");
        VALUE objects = rb_iv_get(scene, "@objects");
        long len = RARRAY_LEN(objects);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // update loop
        for (int i = 0; i < len; i++)
        {
            VALUE obj_val = rb_ary_entry(objects, i);
            if (rb_obj_is_kind_of(obj_val, game_object_class))
            {
                rb_funcall(obj_val, rb_intern("update"), 0);
            }
            else
            {
                VALUE obj_class = rb_obj_class(obj_val);
                VALUE class_name = rb_class_name(obj_class);
                rb_raise(rb_eTypeError, "Attempting to update a %s, which is not a GameObject", StringValueCStr(class_name));
            }
        }

        // draw loop
        for (int i = 0; i < len; i++)
        {
            VALUE obj_val = rb_ary_entry(objects, i);
            if (rb_obj_is_kind_of(obj_val, game_object_class))
            {
                // slow, need to draw all objects at once
                BeginMode2D(cam);

                VALUE sprite_val = rb_iv_get(obj_val, "@sprite");
                VALUE x_val = rb_iv_get(obj_val, "@x");
                VALUE y_val = rb_iv_get(obj_val, "@y");
                VALUE width_val = rb_iv_get(obj_val, "@width");
                VALUE height_val = rb_iv_get(obj_val, "@height");
                VALUE angle_val = rb_iv_get(obj_val, "@angle");
                RBTexture *tex;
                TypedData_Get_Struct(sprite_val, RBTexture, &texture_type, tex);

                float x = NUM2DBL(x_val);
                float y = NUM2DBL(y_val);
                float width = NUM2DBL(width_val);
                float height = NUM2DBL(height_val);
                float angle = NUM2DBL(angle_val);

                Rectangle src = {.x = 0, .y = 0, .width = width, .height = height};
                Rectangle dst = {.x = x, .y = y, .width = width, .height = height};
                Vector2 origin = {.x = 0, .y = 0};
                DrawTexturePro(tex->texture, src, dst, origin, angle, WHITE);

                EndMode2D();
            }
            else
            {
                VALUE obj_class = rb_obj_class(obj_val);
                VALUE class_name = rb_class_name(obj_class);
                rb_raise(rb_eTypeError, "Attempting to draw a %s, which is not a GameObject", StringValueCStr(class_name));
            }
        }

        EndDrawing();
    }

    // should not need to clean up loaded textures and audio, when Ruby closes they should be GC'd
    CloseAudioDevice();
    CloseWindow();
    return Qnil;
}

static VALUE texture_load(VALUE self, VALUE filename)
{
    Check_Type(filename, T_STRING);
    
    VALUE cache_val = rb_iv_get(self, "@cache");
    VALUE texture_val = rb_hash_lookup(cache_val, filename);

    if (texture_val == Qnil)
    {
        // cache doesn't have an entry at this key, make a new one
        RBTexture *tex;
        texture_val = TypedData_Make_Struct(self, RBTexture, &texture_type, tex);
        tex->texture = LoadTexture(StringValueCStr(filename));
        rb_hash_aset(cache_val, filename, texture_val);
    }

    // if cache already has this entry, just return it
    return texture_val;
}

static VALUE texture_width(VALUE self)
{
    RBTexture *tex;
    TypedData_Get_Struct(self, RBTexture, &texture_type, tex);
    return DBL2NUM(tex->texture.width);
}

static VALUE texture_height(VALUE self)
{
    RBTexture *tex;
    TypedData_Get_Struct(self, RBTexture, &texture_type, tex);
    return DBL2NUM(tex->texture.height);
}

static VALUE music_load(VALUE self, VALUE filename)
{
    Check_Type(filename, T_STRING);

    RBMusic *music;
    VALUE obj_val = TypedData_Make_Struct(self, RBMusic, &music_type, music);

    music->music = LoadMusicStream(StringValueCStr(filename));
    return obj_val;
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
    VALUE obj_val = TypedData_Make_Struct(self, RBSound, &sound_type, sound);

    sound->sound = LoadSound(StringValueCStr(filename));
    return obj_val;
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
    rbscene_module = rb_define_module("RBScene");
    VALUE engine_class = rb_define_class_under(rbscene_module, "Engine", rb_cObject);
    rb_define_singleton_method(engine_class, "run", engine_run, 0);

    VALUE texture_class = rb_define_class_under(rbscene_module, "Texture", rb_cObject);
    rb_iv_set(texture_class, "@cache", rb_hash_new());
    rb_define_singleton_method(texture_class, "load", texture_load, 1);
    rb_define_method(texture_class, "width", texture_width, 0);
    rb_define_method(texture_class, "height", texture_height, 0);

    VALUE music_class = rb_define_class_under(rbscene_module, "Music", rb_cObject);
    rb_define_singleton_method(music_class, "load", music_load, 1);
    rb_define_method(music_class, "play", music_play, 0);
    rb_define_method(music_class, "update", music_update, 0);

    VALUE sound_class = rb_define_class_under(rbscene_module, "Sound", rb_cObject);
    rb_define_singleton_method(sound_class, "load", sound_load, 1);
    rb_define_method(sound_class, "play", sound_play, 0);

    // reference existing Ruby classes
    game_object_class = rb_const_get(rbscene_module, rb_intern("GameObject"));

    // init raylib
    InitWindow(320, 288, "Game");
    InitAudioDevice();
    SetTargetFPS(60);
}

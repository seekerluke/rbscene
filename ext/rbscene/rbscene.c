#include "ruby.h"
#include "raylib.h"

// global refs to modules and classes, usually for type checks
static VALUE rbscene_module = Qnil;
static VALUE engine_class = Qnil;
static VALUE game_object_class = Qnil;
static VALUE render_props_class = Qnil;
static VALUE rect_class = Qnil;
static VALUE scene_class = Qnil;
static VALUE texture_class = Qnil;
static VALUE sound_class = Qnil;
static VALUE music_class = Qnil;
static VALUE input_class = Qnil;
static VALUE debug_class = Qnil;

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

// used to keep track of game object properties used for rendering
// faster than just calling rb_iv_get on each of them individually
typedef struct
{
    float x, y;
    float width, height;
    float angle;
    Rectangle frame;
    bool hflip, vflip;
    float origin_x, origin_y;
} RBRenderProps;

typedef struct
{
    int window_width;
    int window_height;
    const char* window_title;
} RBEngineConfig;

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

static const rb_data_type_t render_props_type =
{
    "RBScene::RenderObject",
    { 0, RUBY_DEFAULT_FREE, 0 },
    0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};

static const rb_data_type_t rect_type =
{
    "RBScene::Rect",
    { 0, RUBY_DEFAULT_FREE, 0 },
    0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};

// global engine variables
static Camera2D cam = { .zoom = 2 };
static RBMusic *current_music = NULL;

static int symbol_to_keycode(VALUE sym)
{
    Check_Type(sym, T_SYMBOL);
    const char *name = rb_id2name(SYM2ID(sym));

    if (strcmp(name, "space") == 0) return KEY_SPACE;
    if (strcmp(name, "enter") == 0) return KEY_ENTER;
    if (strcmp(name, "escape") == 0) return KEY_ESCAPE;
    if (strcmp(name, "left") == 0) return KEY_LEFT;
    if (strcmp(name, "right") == 0) return KEY_RIGHT;
    if (strcmp(name, "up") == 0) return KEY_UP;
    if (strcmp(name, "down") == 0) return KEY_DOWN;

    // a-z keys
    if (strlen(name) == 1 && name[0] >= 'a' && name[0] <= 'z') {
        return KEY_A + (name[0] - 'a');
    }

    rb_raise(rb_eArgError, "Unrecognized key symbol: :%s", name);
    return -1; // unreachable, but needed for compilation
}

static int inputs_inner_foreach_callback(VALUE key, VALUE value, VALUE arg)
{
    // inner entry = {right: [true, false, false], gp_right: [false, false, false]}
    Check_Type(key, T_SYMBOL);

    // asserts for internal values are fine, change to rb_raise if this ever becomes user facing code
    assert(value == Qtrue || value == Qfalse);

    // create a new array to store the key state results in
    VALUE keystate_array = rb_ary_new();
    int code = symbol_to_keycode(key);

    // [key_down, key_pressed, key_released]
    VALUE result = IsKeyDown(code) ? Qtrue : Qfalse;
    rb_ary_push(keystate_array, result);
    result = IsKeyPressed(code) ? Qtrue : Qfalse;
    rb_ary_push(keystate_array, result);
    result = IsKeyReleased(code) ? Qtrue : Qfalse;
    rb_ary_push(keystate_array, result);

    rb_hash_aset(arg, key, keystate_array);
    return ST_CONTINUE;
}

static int inputs_foreach_callback(VALUE key, VALUE value, VALUE arg)
{
    // entry looks like this: "right": {right: true, gp_right: false}
    // key is a string, key inside the hash is a symbol, value is a bool
    if (TYPE(key) != T_STRING && TYPE(key) != T_SYMBOL)
    {
        rb_raise(rb_eTypeError, "Expected String or Symbol");
    }

    Check_Type(value, T_HASH);

    rb_hash_foreach(value, inputs_inner_foreach_callback, value);

    return ST_CONTINUE;
}

static RBEngineConfig get_engine_config() {
    VALUE config_val = rb_iv_get(engine_class, "@config");

    VALUE window_title_val = rb_iv_get(config_val, "@window_title");
    Check_Type(window_title_val, T_STRING);

    VALUE window_size_val = rb_iv_get(config_val, "@window_size");
    Check_Type(window_size_val, T_ARRAY);

    VALUE window_width_val = rb_ary_entry(window_size_val, 0);
    VALUE window_height_val = rb_ary_entry(window_size_val, 1);

    if (!rb_obj_is_kind_of(window_width_val, rb_cNumeric))
        rb_raise(rb_eTypeError, "Window width is not a Numeric");

    if (!rb_obj_is_kind_of(window_height_val, rb_cNumeric))
        rb_raise(rb_eTypeError, "Window height is not a Numeric");

    RBEngineConfig config;
    config.window_title = StringValueCStr(window_title_val);
    config.window_width = NUM2UINT(window_width_val);
    config.window_height = NUM2UINT(window_height_val);

    return config;
}

static VALUE engine_init(VALUE self)
{
    RBEngineConfig config = get_engine_config();

    InitWindow(config.window_width, config.window_height, config.window_title);
    InitAudioDevice();
    SetTargetFPS(60);

    return Qnil;
}

static VALUE engine_update(VALUE self)
{
    RBEngineConfig config = get_engine_config();

    SetWindowTitle(config.window_title);
    SetWindowSize(config.window_width, config.window_height);

    return Qnil;
}

static VALUE engine_run(VALUE self)
{
    while (!WindowShouldClose())
    {
        // fetch the current scene's objects
        VALUE scene = rb_iv_get(engine_class, "@current_scene");

        if (!rb_obj_is_kind_of(scene, scene_class)) {
            rb_raise(rb_eTypeError, "Internal error: No active scene. There might be an issue with your config.rb, or your scene switching logic.");
        }

        VALUE objects = rb_iv_get(scene, "@objects");
        Check_Type(objects, T_ARRAY);

        // handle inputs
        VALUE inputs = rb_iv_get(input_class, "@inputs");
        Check_Type(inputs, T_HASH);
        rb_hash_foreach(inputs, inputs_foreach_callback, Qnil);

        if (current_music) UpdateMusicStream(current_music->music);

        // update loop
        // do not optimise by making RARRAY_LEN local, objects can be destroyed during update
        for (int i = 0; i < RARRAY_LEN(objects); i++)
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

        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(cam);

        // draw loop
        for (int i = 0; i < RARRAY_LEN(objects); i++)
        {
            VALUE obj_val = rb_ary_entry(objects, i);
            if (rb_obj_is_kind_of(obj_val, game_object_class))
            {                
                VALUE texture_val = rb_iv_get(obj_val, "@texture");
                RBTexture *tex;
                TypedData_Get_Struct(texture_val, RBTexture, &texture_type, tex);

                VALUE render_props_val = rb_iv_get(obj_val, "@render_props");
                assert(rb_obj_is_kind_of(render_props_val, render_props_class));

                RBRenderProps *props;
                TypedData_Get_Struct(render_props_val, RBRenderProps, &render_props_type, props);

                Rectangle src = props->frame;
                src.width = props->hflip ? -src.width : src.width;
                src.height = props->vflip ? -src.height : src.height;

                Rectangle dst = {
                    .x = props->x,
                    .y = props->y,
                    .width = props->width,
                    .height = props->height
                };
                Vector2 origin = {.x = props->origin_x, .y = props->origin_y};
                DrawTexturePro(tex->texture, src, dst, origin, props->angle, WHITE);
            }
            else
            {
                VALUE obj_class = rb_obj_class(obj_val);
                VALUE class_name = rb_class_name(obj_class);
                rb_raise(rb_eTypeError, "Attempting to draw a %s, which is not a GameObject", StringValueCStr(class_name));
            }
        }

        // debug drawing
        VALUE debug_rects_val = rb_iv_get(debug_class, "@rects");
        Check_Type(debug_rects_val, T_ARRAY);

        for (int i = 0; i < RARRAY_LEN(debug_rects_val); i++)
        {
            VALUE obj_val = rb_ary_entry(debug_rects_val, i);
            if (rb_obj_is_kind_of(obj_val, rect_class))
            {
                Rectangle *rect;
                TypedData_Get_Struct(obj_val, Rectangle, &rect_type, rect);
                DrawRectangleLinesEx(*rect, 1, RED);
            }
            else
            {
                VALUE obj_class = rb_obj_class(obj_val);
                VALUE class_name = rb_class_name(obj_class);
                rb_raise(rb_eTypeError, "Attempting to debug draw a %s, which is not a Rect", StringValueCStr(class_name));
            }
        }

        EndMode2D();
        EndDrawing();
    }

    // should not need to clean up loaded textures and audio, when Ruby closes they should be GC'd
    CloseAudioDevice();
    CloseWindow();
    return Qnil;
}

static VALUE assets_load_texture(VALUE self, VALUE filename)
{
    Check_Type(filename, T_STRING);
    
    VALUE cache_val = rb_iv_get(self, "@textures");
    Check_Type(cache_val, T_HASH);

    VALUE texture_val = rb_hash_lookup(cache_val, filename);
    assert(rb_obj_is_kind_of(texture_val, texture_class));

    if (texture_val == Qnil)
    {
        // cache doesn't have an entry at this key, make a new one
        RBTexture *tex;
        texture_val = TypedData_Make_Struct(texture_class, RBTexture, &texture_type, tex);
        tex->texture = LoadTexture(StringValueCStr(filename));
        // TODO: should raise error if texture loading failed
        rb_hash_aset(cache_val, filename, texture_val);
    }

    // if cache already has this entry, just return it
    return texture_val;
}

static VALUE assets_load_sound(VALUE self, VALUE filename)
{
    Check_Type(filename, T_STRING);

    VALUE cache_val = rb_iv_get(self, "@sounds");
    Check_Type(cache_val, T_HASH);

    VALUE sound_val = rb_hash_lookup(cache_val, filename);
    assert(rb_obj_is_kind_of(sound_val, sound_class));

    if (sound_val == Qnil)
    {
        // cache doesn't have an entry at this key, make a new one
        RBSound *sound;
        sound_val = TypedData_Make_Struct(sound_class, RBSound, &sound_type, sound);
        sound->sound = LoadSound(StringValueCStr(filename));
        // TODO: should raise error if sound loading failed
        rb_hash_aset(cache_val, filename, sound_val);
    }

    // if cache already has this entry, just return it
    return sound_val;
}

static VALUE assets_load_music(VALUE self, VALUE filename)
{
    Check_Type(filename, T_STRING);

    VALUE cache_val = rb_iv_get(self, "@music");
    Check_Type(cache_val, T_HASH);

    VALUE music_val = rb_hash_lookup(cache_val, filename);
    assert(rb_obj_is_kind_of(music_val, music_class));

    if (music_val == Qnil)
    {
        // cache doesn't have an entry at this key, make a new one
        RBMusic *music;
        music_val = TypedData_Make_Struct(music_class, RBMusic, &music_type, music);
        music->music = LoadMusicStream(StringValueCStr(filename));
        // TODO: should raise error if music loading failed
        rb_hash_aset(cache_val, filename, music_val);
    }

    // if cache already has this entry, just return it
    return music_val;
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

static VALUE music_play(VALUE self)
{
    RBMusic *music;
    TypedData_Get_Struct(self, RBMusic, &music_type, music);
    PlayMusicStream(music->music);
    current_music = music;
    return Qnil;
}

static VALUE music_stop(VALUE self)
{
    if (current_music)
    {
        StopMusicStream(current_music->music);
        current_music = NULL;
    }
    return Qnil;
}

static VALUE sound_play(VALUE self)
{
    RBSound *sound;
    TypedData_Get_Struct(self, RBSound, &sound_type, sound);
    PlaySound(sound->sound);
    return Qnil;
}

// creates default render props with internally on game object, called on init
// width and height defaults to the passed texture's width and height, everything else is zero
static VALUE game_object_make_render_props(VALUE self, VALUE texture)
{
    assert(rb_obj_is_kind_of(texture, texture_class));

    RBRenderProps *robj;
    VALUE robj_val = TypedData_Make_Struct(render_props_class, RBRenderProps, &render_props_type, robj);

    RBTexture *tex;
    TypedData_Get_Struct(texture, RBTexture, &texture_type, tex);

    robj->x = 0;
    robj->y = 0;
    robj->width = tex->texture.width;
    robj->height = tex->texture.height;
    robj->angle = 0;
    robj->frame = (Rectangle){
        .x = 0,
        .y = 0,
        .width = tex->texture.width,
        .height = tex->texture.height,
    };
    robj->origin_x = 0;
    robj->origin_y = 0;

    return robj_val;
}

static VALUE render_props_x_getter(VALUE self)
{
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    return DBL2NUM(props->x);
}

static VALUE render_props_y_getter(VALUE self)
{
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    return DBL2NUM(props->y);
}

static VALUE render_props_width_getter(VALUE self)
{
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    return DBL2NUM(props->width);
}

static VALUE render_props_height_getter(VALUE self)
{
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    return DBL2NUM(props->height);
}

static VALUE render_props_angle_getter(VALUE self)
{
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    return DBL2NUM(props->angle);
}

static VALUE render_props_frame_getter(VALUE self)
{
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);

    Rectangle *rect;
    VALUE rect_val = TypedData_Make_Struct(rect_class, Rectangle, &rect_type, rect);
    *rect = props->frame;
    return rect_val;
}

static VALUE render_props_hflip_getter(VALUE self)
{
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    return props->hflip ? Qtrue : Qfalse;
}

static VALUE render_props_vflip_getter(VALUE self)
{
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    return props->vflip ? Qtrue : Qfalse;
}

static VALUE render_props_origin_x_getter(VALUE self)
{
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    return DBL2NUM(props->origin_x);
}

static VALUE render_props_origin_y_getter(VALUE self)
{
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    return DBL2NUM(props->origin_y);
}

static VALUE render_props_x_setter(VALUE self, VALUE val)
{
    if (!rb_obj_is_kind_of(val, rb_cNumeric)) rb_raise(rb_eTypeError, "x is not a Numeric");
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    props->x = NUM2DBL(val);
    return self;
}

static VALUE render_props_y_setter(VALUE self, VALUE val)
{
    if (!rb_obj_is_kind_of(val, rb_cNumeric)) rb_raise(rb_eTypeError, "y is not a Numeric");
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    props->y = NUM2DBL(val);
    return self;
}

static VALUE render_props_width_setter(VALUE self, VALUE val)
{
    if (!rb_obj_is_kind_of(val, rb_cNumeric)) rb_raise(rb_eTypeError, "width is not a Numeric");
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    props->width = NUM2DBL(val);
    return self;
}

static VALUE render_props_height_setter(VALUE self, VALUE val)
{
    if (!rb_obj_is_kind_of(val, rb_cNumeric)) rb_raise(rb_eTypeError, "height is not a Numeric");
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    props->height = NUM2DBL(val);
    return self;
}

static VALUE render_props_angle_setter(VALUE self, VALUE val)
{
    if (!rb_obj_is_kind_of(val, rb_cNumeric)) rb_raise(rb_eTypeError, "angle is not a Numeric");
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    props->angle = NUM2DBL(val);
    return self;
}

static VALUE render_props_frame_setter(VALUE self, VALUE val)
{
    if (!rb_obj_is_kind_of(val, rect_class)) rb_raise(rb_eTypeError, "frame is not a Rect");

    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);

    Rectangle *rect;
    TypedData_Get_Struct(val, Rectangle, &rect_type, rect);

    props->frame = *rect;
    return self;
}

static VALUE render_props_hflip_setter(VALUE self, VALUE val)
{
    if (val != Qtrue && val != Qfalse) rb_raise(rb_eTypeError, "hflip is not a Boolean");
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    props->hflip = val == Qtrue;
    return self;
}

static VALUE render_props_vflip_setter(VALUE self, VALUE val)
{
    if (val != Qtrue && val != Qfalse) rb_raise(rb_eTypeError, "vflip is not a Boolean");
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    props->vflip = val == Qtrue;
    return self;
}

static VALUE render_props_origin_x_setter(VALUE self, VALUE val)
{
    if (!rb_obj_is_kind_of(val, rb_cNumeric)) rb_raise(rb_eTypeError, "x origin is not a Numeric");
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    props->origin_x = NUM2DBL(val);
    return self;
}

static VALUE render_props_origin_y_setter(VALUE self, VALUE val)
{
    if (!rb_obj_is_kind_of(val, rb_cNumeric)) rb_raise(rb_eTypeError, "y origin is not a Numeric");
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    props->origin_y = NUM2DBL(val);
    return self;
}

static VALUE rect_alloc(VALUE self)
{
    Rectangle *rect;
    VALUE rect_val = TypedData_Make_Struct(self, Rectangle, &rect_type, rect);
    rect->x = 0;
    rect->y = 0;
    rect->width = 0;
    rect->height = 0;
    return rect_val;
}

static VALUE rect_initialize(VALUE self, VALUE x, VALUE y, VALUE width, VALUE height)
{
    if (!rb_obj_is_kind_of(x, rb_cNumeric)) rb_raise(rb_eTypeError, "x is not a Numeric");
    if (!rb_obj_is_kind_of(y, rb_cNumeric)) rb_raise(rb_eTypeError, "y is not a Numeric");
    if (!rb_obj_is_kind_of(width, rb_cNumeric)) rb_raise(rb_eTypeError, "w is not a Numeric");
    if (!rb_obj_is_kind_of(height, rb_cNumeric)) rb_raise(rb_eTypeError, "h is not a Numeric");

    Rectangle *rect;
    TypedData_Get_Struct(self, Rectangle, &rect_type, rect);
    rect->x = NUM2DBL(x);
    rect->y = NUM2DBL(y);
    rect->width = NUM2DBL(width);
    rect->height = NUM2DBL(height);

    return self;
}

static VALUE rect_x_getter(VALUE self)
{
    Rectangle *rect;
    TypedData_Get_Struct(self, Rectangle, &rect_type, rect);
    return DBL2NUM(rect->x);
}

static VALUE rect_x_setter(VALUE self, VALUE val)
{
    if (!rb_obj_is_kind_of(val, rb_cNumeric)) rb_raise(rb_eTypeError, "x is not a Numeric");
    Rectangle *rect;
    TypedData_Get_Struct(self, Rectangle, &rect_type, rect);
    rect->x = NUM2DBL(val);
    return Qnil;
}

static VALUE rect_y_getter(VALUE self)
{
    Rectangle *rect;
    TypedData_Get_Struct(self, Rectangle, &rect_type, rect);
    return DBL2NUM(rect->y);
}

static VALUE rect_y_setter(VALUE self, VALUE val)
{
    if (!rb_obj_is_kind_of(val, rb_cNumeric)) rb_raise(rb_eTypeError, "y is not a Numeric");
    Rectangle *rect;
    TypedData_Get_Struct(self, Rectangle, &rect_type, rect);
    rect->y = NUM2DBL(val);
    return Qnil;
}

static VALUE rect_w_getter(VALUE self)
{
    Rectangle *rect;
    TypedData_Get_Struct(self, Rectangle, &rect_type, rect);
    return DBL2NUM(rect->width);
}

static VALUE rect_w_setter(VALUE self, VALUE val)
{
    if (!rb_obj_is_kind_of(val, rb_cNumeric)) rb_raise(rb_eTypeError, "w is not a Numeric");
    Rectangle *rect;
    TypedData_Get_Struct(self, Rectangle, &rect_type, rect);
    rect->width = NUM2DBL(val);
    return Qnil;
}

static VALUE rect_h_getter(VALUE self)
{
    Rectangle *rect;
    TypedData_Get_Struct(self, Rectangle, &rect_type, rect);
    return DBL2NUM(rect->height);
}

static VALUE rect_h_setter(VALUE self, VALUE val)
{
    if (!rb_obj_is_kind_of(val, rb_cNumeric)) rb_raise(rb_eTypeError, "h is not a Numeric");
    Rectangle *rect;
    TypedData_Get_Struct(self, Rectangle, &rect_type, rect);
    rect->height = NUM2DBL(val);
    return Qnil;
}

void Init_rbscene(void)
{
    // init bindings
    rbscene_module = rb_define_module("RBScene");
    engine_class = rb_define_class_under(rbscene_module, "Engine", rb_cObject);
    rb_define_singleton_method(engine_class, "init", engine_init, 0);
    rb_define_singleton_method(engine_class, "run", engine_run, 0);
    rb_define_singleton_method(engine_class, "update", engine_update, 0);

    VALUE assets_class = rb_define_class_under(rbscene_module, "Assets", rb_cObject);
    rb_define_singleton_method(assets_class, "load_texture", assets_load_texture, 1);
    rb_define_singleton_method(assets_class, "load_sound", assets_load_sound, 1);
    rb_define_singleton_method(assets_class, "load_music", assets_load_music, 1);

    texture_class = rb_define_class_under(rbscene_module, "Texture", rb_cObject);
    rb_define_method(texture_class, "width", texture_width, 0);
    rb_define_method(texture_class, "height", texture_height, 0);

    music_class = rb_define_class_under(rbscene_module, "Music", rb_cObject);
    rb_define_singleton_method(music_class, "stop", music_stop, 0);
    rb_define_method(music_class, "play", music_play, 0);

    sound_class = rb_define_class_under(rbscene_module, "Sound", rb_cObject);
    rb_define_method(sound_class, "play", sound_play, 0);

    render_props_class = rb_define_class_under(rbscene_module, "RenderProps", rb_cObject);
    rb_define_method(render_props_class, "x", render_props_x_getter, 0);
    rb_define_method(render_props_class, "y", render_props_y_getter, 0);
    rb_define_method(render_props_class, "width", render_props_width_getter, 0);
    rb_define_method(render_props_class, "height", render_props_height_getter, 0);
    rb_define_method(render_props_class, "angle", render_props_angle_getter, 0);
    rb_define_method(render_props_class, "frame", render_props_frame_getter, 0);
    rb_define_method(render_props_class, "hflip", render_props_hflip_getter, 0);
    rb_define_method(render_props_class, "vflip", render_props_vflip_getter, 0);
    rb_define_method(render_props_class, "origin_x", render_props_origin_x_getter, 0);
    rb_define_method(render_props_class, "origin_y", render_props_origin_y_getter, 0);
    rb_define_method(render_props_class, "x=", render_props_x_setter, 1);
    rb_define_method(render_props_class, "y=", render_props_y_setter, 1);
    rb_define_method(render_props_class, "width=", render_props_width_setter, 1);
    rb_define_method(render_props_class, "height=", render_props_height_setter, 1);
    rb_define_method(render_props_class, "angle=", render_props_angle_setter, 1);
    rb_define_method(render_props_class, "frame=", render_props_frame_setter, 1);
    rb_define_method(render_props_class, "hflip=", render_props_hflip_setter, 1);
    rb_define_method(render_props_class, "vflip=", render_props_vflip_setter, 1);
    rb_define_method(render_props_class, "origin_x=", render_props_origin_x_setter, 1);
    rb_define_method(render_props_class, "origin_y=", render_props_origin_y_setter, 1);

    rect_class = rb_define_class_under(rbscene_module, "Rect", rb_cObject);
    rb_define_alloc_func(rect_class, rect_alloc);
    rb_define_method(rect_class, "initialize", rect_initialize, 4);
    rb_define_method(rect_class, "x", rect_x_getter, 0);
    rb_define_method(rect_class, "y", rect_y_getter, 0);
    rb_define_method(rect_class, "w", rect_w_getter, 0);
    rb_define_method(rect_class, "h", rect_h_getter, 0);
    rb_define_method(rect_class, "x=", rect_x_setter, 1);
    rb_define_method(rect_class, "y=", rect_y_setter, 1);
    rb_define_method(rect_class, "w=", rect_w_setter, 1);
    rb_define_method(rect_class, "h=", rect_h_setter, 1);

    // reference existing Ruby classes
    game_object_class = rb_const_get(rbscene_module, rb_intern("GameObject"));
    rb_define_method(game_object_class, "make_render_props", game_object_make_render_props, 1);
    rb_funcall(game_object_class, rb_intern("private"), 1, ID2SYM(rb_intern("make_render_props"))); // declare make_render_props private

    scene_class = rb_const_get(rbscene_module, rb_intern("Scene"));
    input_class = rb_const_get(rbscene_module, rb_intern("Input"));
    debug_class = rb_const_get(rbscene_module, rb_intern("Debug"));
}

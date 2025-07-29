#include "ruby.h"
#include "raylib.h"

// global refs to modules and classes, usually for type checks
static VALUE rbscene_module = Qnil;
static VALUE game_object_class = Qnil;
static VALUE render_props_class = Qnil;
static VALUE rect_class = Qnil;
static VALUE scene_class = Qnil;
static VALUE texture_class = Qnil;
static VALUE input_class = Qnil;

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
} RBRenderProps;

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

static VALUE engine_run(VALUE self)
{
    while (!WindowShouldClose())
    {
        // fetch the current scene's objects
        VALUE scene = rb_iv_get(rbscene_module, "@current_scene");
        assert(rb_obj_is_kind_of(scene, scene_class));

        VALUE objects = rb_iv_get(scene, "@objects");
        Check_Type(objects, T_ARRAY);
        
        long len = RARRAY_LEN(objects);

        // handle inputs
        VALUE inputs = rb_iv_get(input_class, "@inputs");
        Check_Type(inputs, T_HASH);
        rb_hash_foreach(inputs, inputs_foreach_callback, Qnil);

        if (current_music) UpdateMusicStream(current_music->music);

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

        BeginMode2D(cam);

        // draw loop
        for (int i = 0; i < len; i++)
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
                Vector2 origin = {.x = 0, .y = 0};
                DrawTexturePro(tex->texture, src, dst, origin, props->angle, WHITE);
            }
            else
            {
                VALUE obj_class = rb_obj_class(obj_val);
                VALUE class_name = rb_class_name(obj_class);
                rb_raise(rb_eTypeError, "Attempting to draw a %s, which is not a GameObject", StringValueCStr(class_name));
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

static VALUE texture_load(VALUE self, VALUE filename)
{
    Check_Type(filename, T_STRING);
    
    VALUE cache_val = rb_iv_get(self, "@cache");
    Check_Type(cache_val, T_HASH);

    VALUE texture_val = rb_hash_lookup(cache_val, filename);
    assert(rb_obj_is_kind_of(texture_val, texture_class));

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
    VALUE obj_val = TypedData_Make_Struct(self, RBMusic, &music_type, current_music);
    current_music->music = LoadMusicStream(StringValueCStr(filename)); // TODO: cache this
    return obj_val;
}

static VALUE music_play(VALUE self)
{
    if (current_music) PlayMusicStream(current_music->music);
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
    return TypedData_Wrap_Struct(rect_class, &rect_type, &props->frame);
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

static VALUE render_props_x_setter(VALUE self, VALUE val)
{
    assert(rb_obj_is_kind_of(val, rb_cNumeric));
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    props->x = NUM2DBL(val);
    return self;
}

static VALUE render_props_y_setter(VALUE self, VALUE val)
{
    assert(rb_obj_is_kind_of(val, rb_cNumeric));
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    props->y = NUM2DBL(val);
    return self;
}

static VALUE render_props_width_setter(VALUE self, VALUE val)
{
    assert(rb_obj_is_kind_of(val, rb_cNumeric));
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    props->width = NUM2DBL(val);
    return self;
}

static VALUE render_props_height_setter(VALUE self, VALUE val)
{
    assert(rb_obj_is_kind_of(val, rb_cNumeric));
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    props->height = NUM2DBL(val);
    return self;
}

static VALUE render_props_angle_setter(VALUE self, VALUE val)
{
    assert(rb_obj_is_kind_of(val, rb_cNumeric));
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    props->angle = NUM2DBL(val);
    return self;
}

static VALUE render_props_frame_setter(VALUE self, VALUE val)
{
    assert(rb_obj_is_kind_of(val, rect_class));

    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);

    Rectangle *rect;
    TypedData_Get_Struct(val, Rectangle, &rect_type, rect);

    props->frame = *rect;
    return self;
}

static VALUE render_props_hflip_setter(VALUE self, VALUE val)
{
    assert(val == Qtrue || val == Qfalse);
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    props->hflip = val == Qtrue;
    return self;
}

static VALUE render_props_vflip_setter(VALUE self, VALUE val)
{
    assert(val == Qtrue || val == Qfalse);
    RBRenderProps *props;
    TypedData_Get_Struct(self, RBRenderProps, &render_props_type, props);
    props->vflip = val == Qtrue;
    return self;
}

static VALUE rect_alloc(VALUE self)
{
    Rectangle *rect;
    return TypedData_Make_Struct(self, Rectangle, &rect_type, rect);
}

static VALUE rect_initialize(VALUE self, VALUE x, VALUE y, VALUE width, VALUE height)
{
    assert(rb_obj_is_kind_of(x, rb_cNumeric));
    assert(rb_obj_is_kind_of(y, rb_cNumeric));
    assert(rb_obj_is_kind_of(width, rb_cNumeric));
    assert(rb_obj_is_kind_of(height, rb_cNumeric));

    Rectangle *rect;
    TypedData_Get_Struct(self, Rectangle, &rect_type, rect);
    rect->x = NUM2DBL(x);
    rect->y = NUM2DBL(y);
    rect->width = NUM2DBL(width);
    rect->height = NUM2DBL(height);

    rb_iv_set(self, "@x", x);
    rb_iv_set(self, "@y", y);
    rb_iv_set(self, "@w", width);
    rb_iv_set(self, "@h", height);

    return self;
}

void Init_rbscene(void)
{
    // init bindings
    rbscene_module = rb_define_module("RBScene");
    VALUE engine_class = rb_define_class_under(rbscene_module, "Engine", rb_cObject);
    rb_define_singleton_method(engine_class, "run", engine_run, 0);

    texture_class = rb_define_class_under(rbscene_module, "Texture", rb_cObject);
    rb_iv_set(texture_class, "@cache", rb_hash_new());
    rb_define_singleton_method(texture_class, "load", texture_load, 1);
    rb_define_method(texture_class, "width", texture_width, 0);
    rb_define_method(texture_class, "height", texture_height, 0);

    VALUE music_class = rb_define_class_under(rbscene_module, "Music", rb_cObject);
    rb_define_singleton_method(music_class, "load", music_load, 1);
    rb_define_singleton_method(music_class, "play", music_play, 0);

    VALUE sound_class = rb_define_class_under(rbscene_module, "Sound", rb_cObject);
    rb_define_singleton_method(sound_class, "load", sound_load, 1);
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
    rb_define_method(render_props_class, "x=", render_props_x_setter, 1);
    rb_define_method(render_props_class, "y=", render_props_y_setter, 1);
    rb_define_method(render_props_class, "width=", render_props_width_setter, 1);
    rb_define_method(render_props_class, "height=", render_props_height_setter, 1);
    rb_define_method(render_props_class, "angle=", render_props_angle_setter, 1);
    rb_define_method(render_props_class, "frame=", render_props_frame_setter, 1);
    rb_define_method(render_props_class, "hflip=", render_props_hflip_setter, 1);
    rb_define_method(render_props_class, "vflip=", render_props_vflip_setter, 1);

    rect_class = rb_define_class_under(rbscene_module, "Rect", rb_cObject);
    rb_define_alloc_func(rect_class, rect_alloc);
    rb_define_method(rect_class, "initialize", rect_initialize, 4);
    // more defs in rect.rb

    // reference existing Ruby classes
    game_object_class = rb_const_get(rbscene_module, rb_intern("GameObject"));
    rb_define_method(game_object_class, "make_render_props", game_object_make_render_props, 1);
    rb_funcall(game_object_class, rb_intern("private"), 1, ID2SYM(rb_intern("make_render_props"))); // declare make_render_props private

    scene_class = rb_const_get(rbscene_module, rb_intern("Scene"));
    input_class = rb_const_get(rbscene_module, rb_intern("Input"));

    // init raylib
    InitWindow(320, 288, "Game");
    InitAudioDevice();
    SetTargetFPS(60);
}

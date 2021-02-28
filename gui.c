/*  =========================================================================
    A particle editor made with raylib (https://github.com/raysan5/raylib) 
    and raygui (https://github.com/raysan5/raygui) to be used with my very 
    own particle library.
    WARNING: This is a very early alpha version so take care when using.
    
    TIP: use CTR+C/CTR+V outside the gui area to copy/paste emitters
    
    Inspired by:
     * pixijs editor (https://github.com/pixijs/pixi-particles-editor) 
     * libpartikel (https://github.com/dbriemann/libpartikel)
    =========================================================================
    LICENSE: zlib
    Copyright (C) 2021 Vlad Adrian (@Demizdor - https://github.com/Demizdor)
    =========================================================================
*/

#include "global.h"

#define RAYGUI_SUPPORT_ICONS
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#define GUI_PROPERTY_LIST_IMPLEMENTATION
#include "dm_property_list.h"

#define GUI_BUTTON_SIZE 30
#define GUI_WINDOW_SIZE 250

// ---------------------------------------------------------------------------------------
// GUI global variables
// ---------------------------------------------------------------------------------------
#define EMITTER_TYPE_NAMES "Point;Rect;Circle;Ring"
#define EMITTER_BLEND_MODES "Alpha;Additive;Multiply;AddColors; SubColors"

#define EASING_NAMES "Linear;SineIn;SineOut;SineInOut;CircIn;CircOut;CircInOut;CubicIn;CubicOut;CubicInOut; \
    QuadIn;QuadOut;QuadInOut;ExpoIn;ExpoOut;ExpoInOut;BackIn;BackOut;BackInOut;BounceIn;BounceOut;BounceInOut; \
    ElasticIn;ElasticOut;ElasticInOut"

const Easing Easings[] = {
    &EaseLinearNone, &EaseSineIn, &EaseSineOut, &EaseSineInOut,
    &EaseCircIn, &EaseCircOut, &EaseCircInOut,
    &EaseCubicIn, &EaseCubicOut, &EaseCubicInOut, 
    &EaseQuadIn, &EaseQuadOut, &EaseQuadInOut,
    &EaseExpoIn, &EaseExpoOut, &EaseExpoInOut,
    &EaseBackIn, &EaseBackOut, &EaseBackInOut,
    &EaseBounceIn, &EaseBounceOut, &EaseBounceInOut,
    &EaseElasticIn, &EaseElasticOut, &EaseElasticInOut
};
// ---------------------------------------------------------------------------------------


bool CanMoveEmitter() {
    return CheckCollisionPointRec(GetMousePosition(), (Rectangle){0.0f, 0.0f, GetScreenWidth()-GUI_WINDOW_SIZE-GUI_BUTTON_SIZE, GetScreenHeight()});
}

void DrawGridSystem() 
{
    int color = GuiGetStyle(DEFAULT, LINE_COLOR); // get the default line color
    GuiSetStyle(DEFAULT, LINE_COLOR, ColorToInt(Editor.options.gridcolor)); // set our own color
    // make the grid tile infinitely based on the camera offset
    Rectangle grid = {fmodf(Editor.camera.offset.x, 48.0f)-96.0f, fmodf(Editor.camera.offset.y, 48.0f)-96.0f, GetScreenWidth()+144.0f, GetScreenHeight()+144.0f};
    GuiGrid(grid, 48.0f, 4);
    GuiSetStyle(DEFAULT, LINE_COLOR, color); // reset line color to default
}

static void EmitterWindow(Rectangle bounds)
{
    // emitter property enum in sync with the property array below it
    enum { 
        EMISSION_EMITTER_PROP = 0,
        PULSES_EMITTER_PROP,
        
        FLAGS_SECTION_EMITTER_PROP,
        FLAGS_DISABLED_EMITTER_PROP,
        FLAGS_PAUSED_EMITTER_PROP,
        FLAGS_SPAWN_INSIDE_EMITTER_PROP,
        FLAGS_REVERSE_DRAW_EMITTER_PROP,
        FLAGS_LOCAL_SPACE_EMITTER_PROP,
        FLAGS_DRAW_TRIANGLES_EMITTER_PROP,
        FLAGS_DRAW_OUTLINE_EMITTER_PROP,
        FLAGS_DIRECTIONAL_ROTATION_EMITTER_PROP,
        FLAGS_LOOP_EMITTER_PROP,
        FLAGS_MULTITEXTURE_EMITTER_PROP,
        
        LIFE_EMITTER_PROP,
        EASING_EMITTER_PROP,
        DELAY_EMITTER_PROP,
        
        BLEND_MODES_EMITTER_PROP,
        
        SIZE_SECTION_EMITTER_PROP,
        SIZE_MIN_EMITTER_PROP,
        SIZE_MAX_EMITTER_PROP,
        
        SCALE_SECTION_EMITTER_PROP,
        SCALE_START_EMITTER_PROP,
        SCALE_END_EMITTER_PROP,
        
        ROTATION_SECTION_EMITTER_PROP,
        ROTATION_START_EMITTER_PROP,
        ROTATION_END_EMITTER_PROP,
        
        ANGLE_SECTION_EMITTER_PROP,
        ANGLE_MIN_EMITTER_PROP,
        ANGLE_MAX_EMITTER_PROP,
        
        SPEED_SECTION_EMITTER_PROP,
        SPEED_MIN_EMITTER_PROP,
        SPEED_MAX_EMITTER_PROP,
        
        AGE_SECTION_EMITTER_PROP,
        AGE_MIN_EMITTER_PROP,
        AGE_MAX_EMITTER_PROP,
        
        OFFSET_SECTION_EMITTER_PROP,
        OFFSET_MIN_EMITTER_PROP,
        OFFSET_MAX_EMITTER_PROP,
        
        ACC_SECTION_EMITTER_PROP,
        ACC_START_EMITTER_PROP,
        ACC_END_EMITTER_PROP,
        
        TACC_SECTION_EMITTER_PROP,
        TACC_START_EMITTER_PROP,
        TACC_END_EMITTER_PROP,
        
        TYPE_EMITTER_PROP,
        // the last 2 properties are not needed
    };
    
    // Global emitter property array
    static GuiDMProperty prop[] = 
    {
        PINTPTR_RANGE("Emission", 0, NULL, 1, 1, MAX_PARTICLES-1),
        PINTPTR_RANGE("Pulses", 0, NULL, 1, 0, 128),
            
        PSECTION("Flags", 0, 9),
        PBOOLPTR("disabled", 0, NULL),
        PBOOLPTR("paused", 0, NULL),
        PBOOLPTR("spawn inside", 0, NULL),
        PBOOLPTR("reverse draw", 0, NULL),
        PBOOLPTR("local space", 0, NULL),
        PBOOLPTR("triangles", 0, NULL),
        PBOOLPTR("outline", 0, NULL),
        PBOOLPTR("directional rot", 0, NULL),
        PBOOLPTR("loop", 0, NULL),
        PBOOLPTR("mutitextures", 0, NULL),
            
        PFLOATPTR_RANGE("Life", 0, NULL, 1.0, 2, 0.0f, 600.0f),
        PSELECT("Easing", 0, (char*)EASING_NAMES, 0),
        
        PFLOATPTR_RANGE("Delay", 0, NULL, 1.0, 3, 0.0f, 60.0f),
        
        PSELECT("Blend Mode", 0, (char*)EMITTER_BLEND_MODES, 0),
        
        PSECTION("Size", 0, 2),
        PFLOATPTR_RANGE("min", 0, NULL,  0.1f, 2, 0.0f, 10.0f),
        PFLOATPTR_RANGE("max", 0, NULL,  0.1f, 2, 0.0f, 10.0f),
        
        PSECTION("Scale", 0, 2),
        PFLOATPTR_RANGE("start", 0, NULL,  0.1f, 2, 0.0f, 10.0f),
        PFLOATPTR_RANGE("end", 0, NULL,  0.1f, 2, 0.0f, 10.0f),
        
        PSECTION("Rotation", 0, 2),
        PFLOATPTR_RANGE("start", 0, NULL,  0.1f, 2, 0.0f, 1000.0f),
        PFLOATPTR_RANGE("end", 0, NULL,  0.1f, 2, 0.0f, 1000.0f),
        
        PSECTION("Angle", 0, 2),
        PFLOATPTR_RANGE("min", 0, NULL,  0.1f, 2, -360.0f, 360.0f),
        PFLOATPTR_RANGE("max", 0, NULL,  0.1f, 2, -360.0f, 360.0f),
        
        PSECTION("Speed", 0, 2),
        PFLOATPTR_RANGE("min", 0, NULL,  1.0f, 2, 0.0f, 2000.0f),
        PFLOATPTR_RANGE("max", 0, NULL,  1.0f, 2, 0.0f, 2000.0f),
        
        PSECTION("Age", 0, 2),
        PFLOATPTR_RANGE("min", 0, NULL,  1.0f, 2, 0.0f, 100.0f),
        PFLOATPTR_RANGE("max", 0, NULL,  1.0f, 2, 0.0f, 100.0f),
        
        PSECTION("Offset", 0, 2),
        PFLOATPTR_RANGE("min", 0, NULL,  1.0f, 0, -1000.0f, 1000.0f),
        PFLOATPTR_RANGE("max", 0, NULL,  1.0f, 0, -1000.0f, 1000.0f),
        
        PSECTION("Acc", 0, 2),
        PFLOATPTR_RANGE("start", 0, NULL,  1.0f, 2, -1000.0f, 1000.0f),
        PFLOATPTR_RANGE("end", 0, NULL,  1.0f, 2, -1000.0f, 1000.0f),
        
        PSECTION("TAcc", 0, 2),
        PFLOATPTR_RANGE("start", 0, NULL,  1.0f, 2, -1000.0f, 1000.0f),
        PFLOATPTR_RANGE("end", 0, NULL,  1.0f, 2, -1000.0f, 1000.0f),
        
        PSELECT("Type", 0, (char*)EMITTER_TYPE_NAMES, 0),
        PINT("[1]", GUI_PFLAG_DISABLED, 0),
        PINT("[2]", GUI_PFLAG_DISABLED, 0),
    };
    
    Emitter* e = Editor.emitters[Editor.active_emitter]; // get the active emitter
    
    // get index of the easing used by the active emitter
    // using a map will be better but meh, too much work
    int easing_idx = 0;
    for(int i=0; i<SIZEOF(Easings); ++i) {
        if(Easings[i] == e->config.easing) { easing_idx = i; break; }
    }
    
    // populate a array will all the flags used by the active emitter
    bool flags[] = {
            FLAG_CHECK(e->flags, EMITTER_FLAG_DISABLED),
            FLAG_CHECK(e->flags, EMITTER_FLAG_PAUSED),
            FLAG_CHECK(e->flags, EMITTER_FLAG_SPAWN_INSIDE),
            FLAG_CHECK(e->flags, EMITTER_FLAG_REVERSE_DRAW_ORDER),
            FLAG_CHECK(e->flags, EMITTER_FLAG_WORLD_SPACE),
            FLAG_CHECK(e->flags, EMITTER_FLAG_DRAW_TRIANGLES),
            FLAG_CHECK(e->flags, EMITTER_FLAG_DRAW_OUTLINE),
            FLAG_CHECK(e->flags, EMITTER_FLAG_DIRECTIONAL_ROTATION),
            FLAG_CHECK(e->flags, EMITTER_FLAG_LOOP),
            FLAG_CHECK(e->flags, EMITTER_FLAG_MULTITEXTURE),
    };
    
    // get initial pulses for this emitter (if this changes we need to reset the particles)
    int pulses = e->config.pulses;
    
    // overwrite the property array values with the actual values for this emitter
    PSET_INTPTR(prop, EMISSION_EMITTER_PROP, &e->config.emission);
    PSET_INTPTR(prop, PULSES_EMITTER_PROP, &e->config.pulses);
    // set all the flags 
    for(int i=0; i<SIZEOF(flags); ++i) { 
        PSET_BOOLPTR(prop, FLAGS_DISABLED_EMITTER_PROP+i, &flags[i]);
    }
    PSET_FLOATPTR(prop, LIFE_EMITTER_PROP, &e->life);
    PSET_SELECT_ACTIVE(prop, EASING_EMITTER_PROP, easing_idx);
    PSET_FLOATPTR(prop, DELAY_EMITTER_PROP, &e->delay);
    PSET_SELECT_ACTIVE(prop, BLEND_MODES_EMITTER_PROP, e->mode);
    PSET_FLOATPTR(prop, SIZE_MIN_EMITTER_PROP, &e->config.size.min);
    PSET_FLOATPTR(prop, SIZE_MAX_EMITTER_PROP, &e->config.size.max);
    PSET_FLOATPTR(prop, SCALE_START_EMITTER_PROP, &e->config.scale.start);
    PSET_FLOATPTR(prop, SCALE_END_EMITTER_PROP, &e->config.scale.end);
    PSET_FLOATPTR(prop, ROTATION_START_EMITTER_PROP, &e->config.rotation.start);
    PSET_FLOATPTR(prop, ROTATION_END_EMITTER_PROP, &e->config.rotation.end);
    PSET_FLOATPTR(prop, ANGLE_MIN_EMITTER_PROP, &e->config.angle.min);
    PSET_FLOATPTR(prop, ANGLE_MAX_EMITTER_PROP, &e->config.angle.max);
    PSET_FLOATPTR(prop, SPEED_MIN_EMITTER_PROP, &e->config.speed.min);
    PSET_FLOATPTR(prop, SPEED_MAX_EMITTER_PROP, &e->config.speed.max);    
    PSET_FLOATPTR(prop, AGE_MIN_EMITTER_PROP, &e->config.age.min);
    PSET_FLOATPTR(prop, AGE_MAX_EMITTER_PROP, &e->config.age.max);
    PSET_FLOATPTR(prop, OFFSET_MIN_EMITTER_PROP, &e->config.offset.min);
    PSET_FLOATPTR(prop, OFFSET_MAX_EMITTER_PROP, &e->config.offset.max);
    PSET_FLOATPTR(prop, ACC_START_EMITTER_PROP, &e->config.acc.start);
    PSET_FLOATPTR(prop, ACC_END_EMITTER_PROP, &e->config.acc.end);
    PSET_FLOATPTR(prop, TACC_START_EMITTER_PROP, &e->config.tacc.start);
    PSET_FLOATPTR(prop, TACC_END_EMITTER_PROP, &e->config.tacc.end);
    PSET_SELECT_ACTIVE(prop, TYPE_EMITTER_PROP, e->config.container.type);
    
    
    // change the container properties based on the emitter type
    int type = e->config.container.type;
    if(type == EMITTER_RECT) 
    {
        prop[SIZEOF(prop)-2] = PFLOATPTR_RANGE("Width", 0, &e->config.container.opt1, 1.0f, 0, 0.0f, 4096.0f);
        prop[SIZEOF(prop)-1] = PFLOATPTR_RANGE("Height", 0, &e->config.container.opt2, 1.0f, 0, 0.0f, 4096.0f);
    } 
    else if(type == EMITTER_CIRCLE)
    {
        prop[SIZEOF(prop)-2] = PFLOATPTR_RANGE("Radius", 0, &e->config.container.opt1, 1.0f, 0, 0.0f, 4096.0f);
        prop[SIZEOF(prop)-1] = PINT("[2]", GUI_PFLAG_DISABLED, 0);
    }
    else if(type == EMITTER_RING)
    {
        prop[SIZEOF(prop)-2] = PFLOATPTR_RANGE("Min Radius", 0, &e->config.container.opt1, 1.0f, 0, 0.0f, 4096.0f);
        prop[SIZEOF(prop)-1] = PFLOATPTR_RANGE("Max Radius", 0, &e->config.container.opt2, 1.0f, 0, 0.0f, 4096.0f);
    }
    else
    {
        prop[SIZEOF(prop)-2] = PINT("[1]", GUI_PFLAG_DISABLED, 0);
        prop[SIZEOF(prop)-1] = PINT("[2]", GUI_PFLAG_DISABLED, 0);
    }
    
    
    // draw the properties
    static int focused = 0;
    static int scroll = 0;
    GuiDMPropertyList(bounds, prop, SIZEOF(prop), &focused, &scroll);
    
    // get changed properties
    easing_idx = PGET_SELECT_ACTIVE(prop, EASING_EMITTER_PROP);
    e->config.easing = Easings[easing_idx];
    e->mode = PGET_SELECT_ACTIVE(prop, BLEND_MODES_EMITTER_PROP);
    e->config.container.type = PGET_SELECT_ACTIVE(prop, TYPE_EMITTER_PROP);
    // set flags
    for(int i=0; i<SIZEOF(flags); ++i) {
        bool check = FLAG_CHECK(e->flags, 1<<i);
        if(flags[i] != check) FLAG_TOGGLE(e->flags, 1<<i);
    }
    // reset particles if the pulses property has changed
    if(pulses != e->config.pulses) {
        memset(e->particles.data, 0, MAX_PARTICLES*sizeof(Particle));
        e->particles.count = 0;
    }
}

static void ColorWindow(Rectangle bounds) 
{
    GuiPanel(bounds);
    
    Emitter* e = Editor.emitters[Editor.active_emitter]; // get the active emitter
    
    if(Editor.active_color >= e->config.gradient.count) Editor.active_color = 0;
    
    // setup color changer
    GuiDMProperty prop[] = {
        PCOLOR("Color", 0, 0, 0, 0, 0),
    };
    PSET_COLOR(prop, 0, e->config.gradient.colors[Editor.active_color]);
    
    // draw color changer
    Rectangle item = (Rectangle){bounds.x+2, bounds.y+2, bounds.width-4, 180};
    static int focus = 0;
    static int scroll = 0;
    GuiDMPropertyList(item, prop, SIZEOF(prop), &focus, &scroll);
    e->config.gradient.colors[Editor.active_color] = PGET_COLOR(prop, 0);
    
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
    
    // draw available colors
    Rectangle cr = (Rectangle){item.x, item.y+200, 30, 30};
    for(int i=0; i < e->config.gradient.count; ++i) {
        if(GuiLabelButton(cr, TextFormat("# %i", i))) Editor.active_color = i; // set this color as the active color on click
        GuiDrawRectangle(cr, 2, Editor.active_color != i ? BLACK : RED, e->config.gradient.colors[i]);
        if(cr.x + 2*35 >= item.x + GUI_WINDOW_SIZE) { 
            cr.x = item.x; cr.y += 35;
        } else cr.x += 35;
    }
    
    // draw the Add/Remove color buttons
    cr.width = GUI_WINDOW_SIZE/2 - 4;
    if(cr.x != item.x) cr.y += 35;
    cr.x = item.x;
    if(GuiButton(cr, "Add")) {
        if(e->config.gradient.count < MAX_COLORS) {
            e->config.gradient.colors[e->config.gradient.count] = GenerateRandomColor(0.5f, 0.78f);
            Editor.active_color = e->config.gradient.count;
            e->config.gradient.count += 1;
        }
    }
    cr.x += cr.width + 2;
    if(GuiButton(cr, "Remove")) {
       if(e->config.gradient.count > 1) e->config.gradient.count -= 1;
       if(Editor.active_color >= e->config.gradient.count) Editor.active_color = e->config.gradient.count - 1;
    }
    
    // draw color gradient
    cr.y += 35;
    int cw = (GUI_WINDOW_SIZE - 4)/e->config.gradient.count;
    for(int i=0; i < e->config.gradient.count; ++i) {
        Rectangle cb = {item.x+cw*i, cr.y, cw, GetScreenHeight()-45-cr.y-3};
        DrawRectangleRec(cb, e->config.gradient.colors[i]);
    }
    
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
}

static void ForcesWindow(Rectangle bounds) 
{
    GuiPanel(bounds);
    Emitter* e = Editor.emitters[Editor.active_emitter]; // get the active emitter
    
    Rectangle item = {bounds.x+2, bounds.y+2, bounds.width-4, 0};
    // populate the forces property list
    if(e->config.forces.count != 0) 
    {
        item.height = e->config.forces.count*98;
        int max_height = bounds.height - 82;
        if(item.height > max_height) item.height = max_height;
        GuiDMProperty prop[MAX_FORCES*3];
        
        for(int i=0, p=0; i< e->config.forces.count; ++i, p+=3) {
            prop[p] = PSECTION((char* )TextFormat("Force %i", i+1), 0, 2);
            prop[p+1] = PFLOATPTR_RANGE("direction", 0, &e->config.forces.data[i].direction, 1.0f, 2, -360.0f, 360.0f);
            prop[p+2] = PFLOATPTR_RANGE("strength", 0, &e->config.forces.data[i].strength, 1.0f, 2, -1000.0f, 1000.0f);
        }
        
        // draw forces list
        static int focus = 0;
        static int scroll = 0;
        GuiDMPropertyList(item, prop, e->config.forces.count*3, &focus, &scroll);
        
    }
    
    // draw the Add/Remove buttons
    item.y += item.height + 10;
    item.height = 30;
    if(GuiButton(item, "Add Force")) {
        if(e->config.forces.count < MAX_FORCES) e->config.forces.count += 1;
    }
    item.y += 35;
    if(GuiButton(item, "Remove Force")) {
        if(e->config.forces.count > 0) e->config.forces.count -= 1;
    }
}

static void OptionsWindow(Rectangle bounds)
{
    GuiPanel(bounds);
    Rectangle item = (Rectangle){bounds.x+2, bounds.y+2, bounds.width-2, GetScreenHeight()-99};
    static GuiDMProperty prop[] = {
        PBOOLPTR("Grid", 0, &Editor.options.show_grid),
        PBOOLPTR("Placeholder", 0, &Editor.options.show_placeholder),
        PBOOLPTR("Debug", 0, &Editor.options.show_debug),
        
        PCOLOR("Background", 0, 0, 0, 0, 0),
        PCOLOR("Foreground", 0, 0, 0, 0, 0),
        PCOLOR("Grid Color", 0, 0, 0, 0, 0),
        PCOLOR("Debug", 0, 0, 0, 0, 0)
    };
    
    PSET_COLOR(prop, 3, Editor.options.bg);
    PSET_COLOR(prop, 4, Editor.options.fg);
    PSET_COLOR(prop, 5, Editor.options.gridcolor);
    PSET_COLOR(prop, 6, Editor.options.debug);
    
    static int focus = 0;
    static int scroll = 0;
    GuiDMPropertyList(item, prop, SIZEOF(prop), &focus, &scroll);
    
    Editor.options.bg = PGET_COLOR(prop, 3);
    Editor.options.fg = PGET_COLOR(prop, 4);
    Editor.options.gridcolor = PGET_COLOR(prop, 5);
    Editor.options.debug = PGET_COLOR(prop, 6);
    
    item.y += item.height + 5;
    item.x += 10;
    item.width -= 20;
    item.height = 40;
    if(GuiButton(item, "Reset")) SetDefaultOptions();
}

static void TextureWindow(Rectangle bounds)
{
    GuiPanel(bounds);
    Emitter* e = Editor.emitters[Editor.active_emitter]; // get the active emitter
    
    Rectangle item = {bounds.x+2, bounds.y+2, bounds.width-4, bounds.width-4};
    if(e->config.atlas.texture.id < 1) 
    {
        GuiDrawText("NO TEXTURE", item, GUI_TEXT_ALIGN_CENTER, GRAY);
        GuiDrawRectangle(item, 2, GRAY, BLANK);
    } 
    else 
    {
        Rectangle src = {0.0f, 0.0f, e->config.atlas.texture.width, e->config.atlas.texture.height};
        GuiDrawRectangle(item, 0, BLANK, BLACK);
        DrawTexturePro(e->config.atlas.texture, src, item, (Vector2){0,0}, 0.0f, WHITE);
        
        item.y += bounds.width+6;
        item.height = 30;
        if(GuiButton(item, "Clear Texture")) 
        {
            bool unload = true;
            // only unload if no other emitter has the texture in use 
            for(int i=0; i<Editor.emitter_count; ++i) {
                if(i != Editor.active_emitter && Editor.emitters[i]->config.atlas.texture.id == e->config.atlas.texture.id) {
                    unload = false; 
                    break;
                }
            }
            
            // check for clipboard texture
            if(Editor.clipboard != NULL && e->config.atlas.texture.id == Editor.clipboard->config.atlas.texture.id) {
                unload = false; 
            }
            
            if(unload) UnloadTexture(e->config.atlas.texture);
            
            e->config.atlas.texture = (Texture){0};
            e->config.atlas.vframes = e->config.atlas.hframes = 0;
        }
        
        // draw texture properties
        item.y += 40;
        item.height = 160;
        GuiDMProperty prop[] = {
            PSECTION("Frames", 0, 2),
            PINTPTR_RANGE("horizontal", 0, &e->config.atlas.hframes, 1, 0, 256),
            PINTPTR_RANGE("vertical", 0, &e->config.atlas.vframes, 1, 0, 256),
            PINTPTR_RANGE("loop", 0, &e->config.atlas.loop, 1, 1, 256),
        };
        
        static int focus = 0;
        static int scroll = 0;
        GuiDMPropertyList(item, prop, SIZEOF(prop), &focus, &scroll);
    }
}

// Draw the right side window
static void DrawWindow(Rectangle bounds)
{
    // draw the curently active window
    static void (*window[])(Rectangle bounds) = { &EmitterWindow, &ColorWindow, &TextureWindow, &ForcesWindow, &OptionsWindow};
    (*window[Editor.active_window])(bounds);
    
    // draw the tabs below it
    Editor.active_window = GuiToggleGroup((Rectangle){bounds.x, GetScreenHeight()-40, 30, 30}, "#96#;#27#;#12#;#147#;#140#", Editor.active_window);
    int pressed = GuiToggleGroup((Rectangle){bounds.x+bounds.width-60, GetScreenHeight()-40, 58, 30}, "#6#Save", -1);
    if(pressed == 0) { 
        // Save particle system to file
        static int i = 1;
        const char* file = TextFormat("%s_%00i.dps", Editor.name, i);
        if(SaveEmitters(file)) 
        {
            TraceLog(LOG_INFO , TextFormat("Saved emitters as `%s`", file));
            ++i;
        } else {
            TraceLog(LOG_WARNING, TextFormat("Failed to save emitters as `%s`", file));
        }
    }
}

// Draw the editor buttons at the right of the screen
void EditorButtons(Rectangle bounds)
{
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
    
    // Draw all the available emitters
    for(int i=0; i<Editor.emitter_count; ++i)
    {
        int color = GuiGetStyle(LABEL, TEXT_COLOR_NORMAL);
        if(i == Editor.active_emitter) GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(Editor.options.debug));
        else GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(Editor.options.fg));
        if(GuiLabelButton(bounds, TextFormat("%02i", Editor.emitter_id[i]+1))) {
            Editor.active_emitter = i;
        }
        GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, color);
        bounds.y += GUI_BUTTON_SIZE + 1;
    }
    
    // ADD EMITTER BUTTON
    Vector2 mouse = GetMousePosition();
    if(CheckCollisionPointRec(mouse, bounds)) GuiSetTooltip("Add new emitter");
    if(GuiLabelButton(bounds, "#08#")) EditorAddEmitter((Vector2){(GetScreenWidth()-GUI_WINDOW_SIZE)/2, GetScreenHeight()/2});
    
    // REMOVE EMITTER BUTTON
    bounds.y += GUI_BUTTON_SIZE + 1;
    if(CheckCollisionPointRec(mouse, bounds)) GuiSetTooltip("Remove last emitter");
    if(GuiLabelButton(bounds, "#09#")) EditorRemoveEmitter();
    
    // MOVE UP BUTTON
    bounds.y += GUI_BUTTON_SIZE + 1;
    if(CheckCollisionPointRec(mouse, bounds)) GuiSetTooltip("Move Up");
    if(GuiLabelButton(bounds, "#117#")) EditorMoveUpEmitter();
    
    // MOVE DOWN BUTTON
    bounds.y += GUI_BUTTON_SIZE + 1;
    if(CheckCollisionPointRec(mouse, bounds)) GuiSetTooltip("Move Down");
    if(GuiLabelButton(bounds, "#116#")) EditorMoveDownEmitter();
    
    // CLOSE WINDOW BUTTON
    bounds.y += GUI_BUTTON_SIZE + 1;
    if(CheckCollisionPointRec(mouse, bounds)) GuiSetTooltip("Close Window");
    if(GuiLabelButton(bounds, "#113#")) Editor.active_emitter = -1;
    
    // SYNC BUTTON
    bounds.y += GUI_BUTTON_SIZE + 1;
    if(CheckCollisionPointRec(mouse, bounds)) GuiSetTooltip("Synchronize emitters");
    if(GuiLabelButton(bounds, "#139#")) EditorSyncEmitters();
    
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
}

// Draw the name input at the top of the screen
static inline void EditorNameInput(Rectangle bounds) 
{
    // Draw text input only when mouse is hovered
    if(CheckCollisionPointRec(GetMousePosition(), bounds)) GuiTextBox(bounds, (char*)&Editor.name, MAX_NAME_LEN-1, true);
    else GuiDrawText((char*)&Editor.name, bounds, GUI_TEXT_ALIGN_CENTER, Editor.options.fg);
    
    // We can't have any spaces that will mess up the parsing when saving/loading so scan the name and replace them with a `_`
    for(int i=0; i<MAX_NAME_LEN; ++i) {
        if(Editor.name[i] == ' ') Editor.name[i] = '_';
        else if(Editor.name[i] == '\0') break;
    }    
}

#define FORMAT_MEASUREMENT(IN, OUT, U, S) do { \
    if(IN > S*S) { OUT = IN/(S*S); U = "M"; } \
    else if(IN > S) { OUT = IN/S; U = "K"; } \
    else { OUT = IN; U = ""; } \
} while(0)

void DrawGUI()
{
    // Calculate bounds for the window on the right side
    Rectangle bounds = {GetScreenWidth()-GUI_WINDOW_SIZE-GUI_BUTTON_SIZE, 2, GUI_BUTTON_SIZE, GUI_BUTTON_SIZE};
    if(Editor.active_emitter == -1) bounds.x += GUI_WINDOW_SIZE;
    
    // Draw FPS
    const char* fmt = Editor.statistics.shown ? "#120#FPS %i" : "#119#FPS %i";
    if(GuiLabelButton((Rectangle){10.0f, 8.0f, 50.0f, 20.0f}, TextFormat(fmt, GetFPS())) ) Editor.statistics.shown = !Editor.statistics.shown;
    
    // Draw Statistics
    if(Editor.statistics.shown)
    {
        double pixels = 0.0;
        char* pu = "";
        FORMAT_MEASUREMENT(Editor.statistics.pixels, pixels, pu, 1000.0);
        
        int used_mem = 0;
        char* umu = "";
        for(int i=0; i<Editor.emitter_count; ++i) {
            used_mem += Editor.emitters[i]->particles.count*sizeof(Particle) +
                Editor.emitters[i]->config.gradient.count*sizeof(Color) + Editor.emitters[i]->config.forces.count*sizeof(Force);
        }
        FORMAT_MEASUREMENT(used_mem, used_mem, umu, 1024);
        
        int total_mem = Editor.statistics.total_mem;
        char* tmu = "";
        FORMAT_MEASUREMENT(total_mem, total_mem, tmu, 1024);
        
        DrawText(TextFormat("updated %d\ndrawn %d\npixels %.2f%s\nused_mem %d%s\ntotal_mem %d%s", Editor.statistics.updated, Editor.statistics.drawn, 
            pixels, pu, used_mem, umu, total_mem, tmu), 10, 35, 10, Editor.options.fg);
    }
    
    // Draw the name input at the top of the screen
    EditorNameInput((Rectangle){74.0f, 2.0f, (Editor.active_emitter == -1) ? GetScreenWidth()-GUI_BUTTON_SIZE-70 : GetScreenWidth()-GUI_WINDOW_SIZE-GUI_BUTTON_SIZE-70, GUI_BUTTON_SIZE});
    
    // Draw the buttons on the right side
    EditorButtons(bounds);
    
    if(Editor.active_emitter == -1 || Editor.emitter_count == 0 ) return;
    
    // Draw the window on the right side
    bounds = (Rectangle){GetScreenWidth()-GUI_WINDOW_SIZE, -1, GUI_WINDOW_SIZE+1, GetScreenHeight()-45 };
    DrawWindow(bounds);
    
    // Show the tooltips (we do it here so that the tootips will show on top of everything)
    Vector2 mouse = GetMousePosition();
    GuiDrawTooltip((Rectangle){mouse.x-10.0f, mouse.y-10.0f, 100.0f, 30.0f});
    GuiClearTooltip();
}

static inline int WasTextureExported(int tidx, int id)
{
    for(int i=0; i<id; ++i) {
        if(Editor.emitters[i]->config.atlas.texture.id == tidx) return i;
    }
    return id;
}

int SaveEmitters(const char* file)
{
    if(Editor.emitter_count == 0) return 0;
    
    FILE* fp = fopen(file, "wb");
    if(fp == NULL) return 0;
    
    //write help 
    fprintf(fp, "#\n"
        "# Demizdor's Particle System file (v" EDITOR_VER ")\n"
        "#\n"
        "# Emitter properties:\n"
        "#\tn name_of_particle_system\n"
        "#\tp placeholder_texture\n"
        "#\n"
        "#\tg ID gradient_count g0 g1 ... gn\n"
        "#\tf ID forces_count f0_direction f0_strength .. fn_direction fn_strength\n"
        "#\ta ID hframes vframes loop texture\n"
        "#\tc ID emission pulses size_min size_max angle_min angle_max offset_min offset_max age_min age_max speed_min speed_max scale_start scale_end acc_start acc_end tacc_start tacc_end rot_start rot_end\n"
        "#\te ID pos_x pos_y max_particles life delay blend_mode easing flags type opt1 opt2\n"
        "#\n");
    
    // write placeholder and name of the particle system
    fprintf(fp, "\nn %s\n", Editor.name);
    if(Editor.placeholder.id > 0) {
        ExportImage(GetTextureData(Editor.placeholder), TextFormat("%s/%s_ph.png", GetDirectoryPath(file), Editor.name));
        fprintf(fp, "p %s_ph.png\n", Editor.name);
    }

    
    
    for(int i=0; i<Editor.emitter_count; ++i) 
    {
        Emitter* e = Editor.emitters[i];
        
        // write gradients
        fprintf(fp, "\n\ng  %02i %02i", i, e->config.gradient.count);
        for(int g=0; g<e->config.gradient.count; ++g) {
            fprintf(fp, " %08X", ColorToInt(e->config.gradient.colors[g]));
        }
        fprintf(fp, "\n");
        
        // write forces
        if(e->config.forces.count > 0) {
            fprintf(fp, "f  %02i %02i", i, e->config.forces.count);
            for(int f=0; f<e->config.forces.count; ++f) {
                fprintf(fp, " %.2f %.2f", e->config.forces.data[f].direction, e->config.forces.data[f].strength);
            }
            fprintf(fp, "\n");
        }
        
        // write atlas
        if(e->config.atlas.texture.id > 0) 
        {
            int id = WasTextureExported(e->config.atlas.texture.id, i);
            if(id == i) ExportImage(GetTextureData(e->config.atlas.texture), TextFormat("%s/%s_%02i.png", GetDirectoryPath(file), Editor.name, i));
            fprintf(fp, "a  %02i %3i %3i %3i %s_%02i.png\n", i, e->config.atlas.hframes, e->config.atlas.vframes, e->config.atlas.loop, Editor.name, id);
        }
        
        // find the easing id
        int easing_idx = 0;
        for(int k=0; k<SIZEOF(Easings); ++k) {
            if(Easings[k] == e->config.easing) {
                easing_idx = k;
                break;
            }
        }
        // write configuration
        fprintf(fp, "c  %02i %04i %2i %f %f %.1f %.1f %.0f %.0f %f %f %f %f %f %f %.0f %.0f %.0f %.0f %.1f %.1f\n", i,
            e->config.emission, e->config.pulses, e->config.size.min, e->config.size.max, e->config.angle.min, e->config.angle.max,
            e->config.offset.min, e->config.offset.max, e->config.age.min, e->config.age.max, e->config.speed.min, e->config.speed.max, e->config.scale.start, e->config.scale.end,
            e->config.acc.start, e->config.acc.end, e->config.tacc.start, e->config.tacc.end, e->config.rotation.start, e->config.rotation.end);
        
        // write emitter
        // e ID pos_x pos_y max_particles life delay blend_mode easing flags type opt1 opt2
        fprintf(fp, "e  %02i %.0f %.0f %04i %f %f %02i %04i %i %02i %4.0f %4.0f\n", i, e->position.x, e->position.y, e->particles.max, e->life, e->delay, e->mode, easing_idx, e->flags, 
            e->config.container.type, e->config.container.opt1, e->config.container.opt2);
    }
    
    fclose(fp);
    
    return 1;
}

int LoadEmitters(const char* file)
{
    FILE* fp = fopen(file, "rb");
    if(fp == NULL) return 0;
    
    enum {MAX_BUFFER_SIZE = 512};
    
    char buffer[MAX_BUFFER_SIZE] = {0};
    fgets(buffer, MAX_BUFFER_SIZE, fp);
    if(buffer[0] != '#') { fclose(fp); return 0; }
    
    // reset emitters
    DeallocateEmitters();
    Editor.emitter_count = 0;
    Editor.active_emitter = -1;
    // reset emitter ids
    for(int i=0; i<MAX_EMITTERS; ++i) {
        Editor.emitter_id[i] = i;
    }
    
    while(!feof(fp)) 
    {
        if(buffer[0] == 'g') 
        {
            // parse color gradients
            int id = MAX_EMITTERS, count = 0, idx = 0, bytes = 0;
            sscanf(buffer, "g  %02d %02d%n", &id, &count, &bytes);
            if(id < MAX_EMITTERS && count <= MAX_COLORS) 
            {
                if(Editor.emitters[id] == NULL) {
                    // allocate memory for emitter
                    Editor.emitters[id] = calloc(1, sizeof(Emitter));
                    if(Editor.emitters[id] == NULL) TraceLog(LOG_FATAL, "EMITTER: Failed to allocate memory");
                    Editor.statistics.total_mem += sizeof(Emitter);
                }
                
                if(Editor.emitters[id]->config.gradient.colors == NULL) {
                    Editor.emitters[id]->config.gradient.colors = calloc(MAX_COLORS, sizeof(Color));
                    Editor.statistics.total_mem += MAX_COLORS*sizeof(Color);
                }
                
                if(Editor.emitters[id]->config.gradient.colors != NULL)
                {
                    idx += bytes;
                    for(int c=0; c<count; ++c) {
                        int color = 0;
                        if(sscanf(&buffer[idx], " %X%n", &color, &bytes) == EOF) {
                            count = c + 1;
                            break;
                        }
                        idx += bytes;
                        Editor.emitters[id]->config.gradient.colors[c] = GetColor(color);
                    }
                    Editor.emitters[id]->config.gradient.count = count;
                } else Editor.emitters[id]->config.gradient.count = 0;
            }
        }
        else if(buffer[0] == 'f')
        {
            // parse forces
            int id = MAX_EMITTERS, count = 0, idx = 0, bytes = 0;;
            sscanf(buffer, "f  %02d %02d%n", &id, &count, &bytes);
            if(id < MAX_EMITTERS && count <= MAX_FORCES) 
            {
                if(Editor.emitters[id] == NULL) {
                    // allocate memory for emitter
                    Editor.emitters[id] = calloc(1, sizeof(Emitter));
                    if(Editor.emitters[id] == NULL) TraceLog(LOG_FATAL, "EMITTER: Failed to allocate memory");
                    Editor.statistics.total_mem += sizeof(Emitter);
                }
                
                if(Editor.emitters[id]->config.forces.data == NULL) {
                    Editor.emitters[id]->config.forces.data = calloc(MAX_FORCES, sizeof(Force));
                    Editor.statistics.total_mem += MAX_FORCES*sizeof(Force);
                }
                
                if(Editor.emitters[id]->config.forces.data != NULL) {
                    idx += bytes;
                    for(int f=0; f<count; ++f) {
                        if(sscanf(&buffer[idx], " %f %f%n", &Editor.emitters[id]->config.forces.data[f].direction, 
                            &Editor.emitters[id]->config.forces.data[f].strength, &bytes) == EOF)
                        {
                            count = f + 1;
                            break;
                        }
                        idx += bytes;
                    }
                    Editor.emitters[id]->config.forces.count = count;
                } else Editor.emitters[id]->config.forces.count = 0;
            }
        }
        else if(buffer[0] == 'a') 
        {
            // parse atlas
            int id = MAX_EMITTERS, idx=0, bytes=0;
            sscanf(buffer, "a  %02d%n", &id, &bytes);
            if(id < MAX_EMITTERS) 
            {
                if(Editor.emitters[id] == NULL) {
                    // allocate memory for emitter
                    Editor.emitters[id] = calloc(1, sizeof(Emitter));
                    if(Editor.emitters[id] == NULL) TraceLog(LOG_FATAL, "EMITTER: Failed to allocate memory");
                    Editor.statistics.total_mem += sizeof(Emitter);
                }
                
                idx += bytes;
                char texture[128] = {0};
                Emitter* e = Editor.emitters[id];
                sscanf(&buffer[idx], " %3d %3d %3d %127[^\r\n]s\n", &e->config.atlas.hframes, &e->config.atlas.vframes, &e->config.atlas.loop, (char*)&texture);
                int has_texture = strncmp("NONE", texture, 4);
                if(has_texture != 0) {
                    Texture t = LoadTexture(TextFormat("%s/%s", GetDirectoryPath(file), texture));
                    if(t.id > 0) {
                        if(Editor.clipboard == NULL || (Editor.clipboard != NULL && Editor.clipboard->config.atlas.texture.id != e->config.atlas.texture.id)) 
                            UnloadTexture(e->config.atlas.texture);
                        e->config.atlas.texture = t;
                    }
                }
            }
        }
        else if(buffer[0] == 'c') 
        {
            // parse configuration
            int id = MAX_EMITTERS, idx=0, bytes=0;
            sscanf(buffer, "c  %02d%n", &id, &bytes);
            if(id < MAX_EMITTERS) 
            {
                if(Editor.emitters[id] == NULL) {
                    // allocate memory for emitter
                    Editor.emitters[id] = calloc(1, sizeof(Emitter));
                    if(Editor.emitters[id] == NULL) TraceLog(LOG_FATAL, "EMITTER: Failed to allocate memory");
                    Editor.statistics.total_mem += sizeof(Emitter);
                }
                
                idx += bytes;
                EmitterConfig* cfg = &Editor.emitters[id]->config;
                sscanf(&buffer[idx], " %04d %2d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", 
                    &cfg->emission, &cfg->pulses, &cfg->size.min, &cfg->size.max, &cfg->angle.min, &cfg->angle.max, &cfg->offset.min, &cfg->offset.max, 
                    &cfg->age.min, &cfg->age.max, &cfg->speed.min, &cfg->speed.max, &cfg->scale.start, &cfg->scale.end, &cfg->acc.start, &cfg->acc.end, 
                    &cfg->tacc.start, &cfg->tacc.end, &cfg->rotation.start, &cfg->rotation.end);
            }
        }
        else if(buffer[0] == 'e')
        {
            // parse emitter
            int id = MAX_EMITTERS, idx = 0, bytes = 0;
            sscanf(buffer, "e  %02d%n", &id, &bytes);
            if(id < MAX_EMITTERS) 
            {
                if(Editor.emitters[id] == NULL) 
                {
                    // allocate memory for emitter
                    Editor.emitters[id] = calloc(1, sizeof(Emitter));
                    if(Editor.emitters[id] == NULL) TraceLog(LOG_FATAL, "EMITTER: Failed to allocate memory");
                    Editor.statistics.total_mem += sizeof(Emitter);
                }
                
                if(Editor.emitters[id]->config.forces.data == NULL) {
                    Editor.emitters[id]->config.forces.data = calloc(MAX_FORCES, sizeof(Force));
                    Editor.statistics.total_mem += MAX_FORCES*sizeof(Force);
                }
                
                Emitter* e = Editor.emitters[id];
                int eidx = 0;
                idx += bytes;
                sscanf(&buffer[idx], " %f %f %04d %f %f %02d %04d %d %02d %f %f", &e->position.x, &e->position.y, &e->particles.max, &e->life, &e->delay, (int*)&e->mode, &eidx, &e->flags, 
                    (int*)&e->config.container.type, &e->config.container.opt1, &e->config.container.opt2);
                e->particles.max = MAX_PARTICLES;
                // make sure we allocate memory for the particles
                if(e->particles.data == NULL) {
                    e->particles.data = calloc(MAX_PARTICLES, sizeof(Particle));
                    Editor.statistics.total_mem += MAX_PARTICLES*sizeof(Particle);
                }
                if(eidx < SIZEOF(Easings)) e->config.easing = Easings[eidx];
                Editor.emitter_count += 1;
            }
        } 
        else if(buffer[0] == 'n') 
        {
            sscanf(buffer, "n  %31[^\r\n]s", (char*)&Editor.name);
        }
        else if(buffer[0] == 'p')
        {
            char placeholder_path[32] = {0};
            sscanf(buffer, "p  %31[^\r\n]s", (char*)&placeholder_path);
            
            // try to load placeholder
            if(strlen((char*)&placeholder_path) > 0) 
            {
                Texture t = LoadTexture(TextFormat("%s/%s", GetDirectoryPath(file), placeholder_path));
                if(t.id > 0) {
                    UnloadTexture(Editor.placeholder);
                    Editor.placeholder = t;
                    Editor.options.show_placeholder = true;
                }
            }
        } 
        
        fgets(buffer, MAX_BUFFER_SIZE, fp);
    }
    fclose(fp);
    
    return 1;
}

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

#include <stdio.h>
#include <string.h>

#define LIB_RAY_PARTICLES_IMPL
#include "particles.h"
#include "global.h"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 540


static void UpdateEditor();
static void DrawEditor();

inline void SetDefaultOptions() 
{
    // set default editor colors
    Editor.options.bg = RAYWHITE;
    Editor.options.fg = BLACK;
    Editor.options.gridcolor = LIGHTGRAY;
    Editor.options.debug = RED;
    
    Editor.options.show_debug = Editor.options.show_grid = true;
}

void InitializeEditor() 
{
    //initialize raylib
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Particle Editor (v" EDITOR_VER ")");
    SetWindowMinSize(800, 450);
    SetTargetFPS(60);

    // initialize the global editor variable
    Editor = (struct SEditor){0};
    Editor.camera.zoom = 1.0f;
    Editor.active_emitter = -1;  // set no emitter active
    
    // setup default options
    SetDefaultOptions();
    
    // load options from storage
    SetTraceLogLevel(LOG_NONE);
    int saved = LoadStorageValue(7);
    if(saved)
    {
        Editor.options.show_grid = LoadStorageValue(0);
        Editor.options.show_placeholder = LoadStorageValue(1);
        Editor.options.show_debug = LoadStorageValue(2);
        Editor.options.bg = GetColor(LoadStorageValue(3));
        Editor.options.fg = GetColor(LoadStorageValue(4));
        Editor.options.gridcolor = GetColor(LoadStorageValue(5));
        Editor.options.debug = GetColor(LoadStorageValue(6));
    }
    SetTraceLogLevel(LOG_INFO);
    
    // set default ids
    for(int i=0; i<MAX_EMITTERS; ++i) {
        Editor.emitter_id[i] = i;
    }
    
}

void DeallocateEmitters() 
{
    // deallocate memory
    for(int i=0; i<Editor.emitter_count; ++i) 
    {
        free(Editor.emitters[i]->particles.data);
        free(Editor.emitters[i]->config.gradient.colors);
        free(Editor.emitters[i]->config.forces.data);
        free(Editor.emitters[i]);
        Editor.statistics.total_mem -= sizeof(Emitter) + MAX_PARTICLES*sizeof(Particle) + MAX_COLORS*sizeof(Color) + MAX_FORCES*sizeof(Force);
        Editor.emitters[i] = NULL;
    }
}

void FinalizeEditor() 
{
    // unload textures
    UnloadTexture(Editor.placeholder);
    if(Editor.clipboard != NULL) UnloadTexture(Editor.clipboard->config.atlas.texture);
    
    // deallocate emitters and clipboard
    DeallocateEmitters();
    if(Editor.clipboard != NULL) free(Editor.clipboard);
    
    // save options
    SetTraceLogLevel(LOG_NONE);
    SaveStorageValue(0, Editor.options.show_grid);
    SaveStorageValue(1, Editor.options.show_placeholder);
    SaveStorageValue(2, Editor.options.show_debug);
    SaveStorageValue(3, ColorToInt(Editor.options.bg));
    SaveStorageValue(4, ColorToInt(Editor.options.fg));
    SaveStorageValue(5, ColorToInt(Editor.options.gridcolor));
    SaveStorageValue(6, ColorToInt(Editor.options.debug));
    SaveStorageValue(7, 1); // when loading signals that the options were saved
    SetTraceLogLevel(LOG_INFO);
    
    // raylib finalize
    CloseWindow(); 
}

void UpdateEditor() 
{
    // ---------------------------------------------------------------------------------------
    // Move camera with right mouse button and move the active emitter with the left
    // ---------------------------------------------------------------------------------------
    if(IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
        Editor.mouse = GetMousePosition();
        Editor.offset = Editor.camera.offset;
    } 
    else if(IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
    {
        Editor.camera.offset = Vector2Subtract(Editor.offset, Vector2Subtract(Editor.mouse, GetMousePosition()));
    }
    else if(Editor.active_emitter != -1 && CanMoveEmitter())
    {
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Editor.mouse = GetMousePosition();
            Editor.offset = Editor.emitters[Editor.active_emitter]->position;
        } else if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Editor.emitters[Editor.active_emitter]->position = Vector2Subtract(Editor.offset, Vector2Subtract(Editor.mouse, GetMousePosition()));
        }
    }
    
    // ---------------------------------------------------------------------------------------
    // Handle Copy/Pasting of emitters
    // ---------------------------------------------------------------------------------------
    if(Editor.active_emitter != -1 && CanMoveEmitter() && (IsKeyDown(KEY_RIGHT_CONTROL) || IsKeyDown(KEY_LEFT_CONTROL)) )
    {
        if(IsKeyPressed(KEY_C)) 
        {
            if(Editor.clipboard != NULL) 
            {
                // erase old clipboard content
                memset(Editor.clipboard->particles.data, 0, MAX_PARTICLES*sizeof(Particle));
                memset(Editor.clipboard->config.gradient.colors, 0, MAX_COLORS*sizeof(Color));
                memset(Editor.clipboard->config.forces.data, 0, MAX_FORCES*sizeof(Force));
                
                // unload texture only if not in use
                bool unload = true;
                for(int i=0; i<Editor.emitter_count; ++i) 
                {
                    if( Editor.emitters[i]->config.atlas.texture.id == Editor.clipboard->config.atlas.texture.id) {
                        unload = false; 
                        break;
                    }
                }
                
                if(unload) UnloadTexture(Editor.clipboard->config.atlas.texture);
            }
            else 
            {
                // allocate clipboard memory 
                Editor.clipboard = calloc(1, sizeof*Editor.clipboard);
                if(Editor.clipboard == NULL) TraceLog(LOG_FATAL, "CLIPBOARD: Failed to allocate memory");
                
                Editor.clipboard->particles.data = calloc(MAX_PARTICLES, sizeof(Particle));
                if(Editor.clipboard->particles.data == NULL) TraceLog(LOG_FATAL, "CLIPBOARD: Failed to allocate memory");
                
                Editor.clipboard->config.gradient.colors = calloc(MAX_COLORS, sizeof(Color));
                if(Editor.clipboard->config.gradient.colors == NULL) TraceLog(LOG_FATAL, "CLIPBOARD: Failed to allocate memory");
                
                Editor.clipboard->config.forces.data = calloc(MAX_FORCES, sizeof(Force));
                if(Editor.clipboard->config.forces.data == NULL) TraceLog(LOG_FATAL, "CLIPBOARD: Failed to allocate memory");
            }
            
            // save clipboard pointers
            Particle* particles = Editor.clipboard->particles.data;
            Color* colors = Editor.clipboard->config.gradient.colors;
            Force* forces = Editor.clipboard->config.forces.data;
            
            // overwrite clipboard
            *Editor.clipboard = *Editor.emitters[Editor.active_emitter];
            memcpy(forces, Editor.emitters[Editor.active_emitter]->config.forces.data, MAX_FORCES*sizeof(Force));
            memcpy(colors, Editor.emitters[Editor.active_emitter]->config.gradient.colors, MAX_COLORS*sizeof(Color));
            
            // restore the actual clipboard pointers
            Editor.clipboard->particles.data = particles;
            Editor.clipboard->config.gradient.colors = colors;
            Editor.clipboard->config.forces.data = forces;
            
            Editor.clipboard->emit_timer = Editor.clipboard->spawn_timer = 0.0f;
            Editor.clipboard->particles.count = 1;
        }
        else if(IsKeyPressed(KEY_V)) 
        {
            if(Editor.clipboard != NULL) 
            {
                // erase emitter content
                memset(Editor.emitters[Editor.active_emitter]->particles.data, 0, MAX_PARTICLES*sizeof(Particle));
                memset(Editor.emitters[Editor.active_emitter]->config.gradient.colors, 0, MAX_COLORS*sizeof(Color));
                memset(Editor.emitters[Editor.active_emitter]->config.forces.data, 0, MAX_FORCES*sizeof(Force));
                
                // unload texture only if not in use
                bool unload = true;
                for(int i=0; i<Editor.emitter_count; ++i) 
                {
                    if(i != Editor.active_emitter && Editor.emitters[i]->config.atlas.texture.id == Editor.emitters[Editor.active_emitter]->config.atlas.texture.id) {
                        unload = false; 
                        break;
                    }
                }
                
                // also check the clipboard texture
                if(Editor.emitters[Editor.active_emitter]->config.atlas.texture.id == Editor.clipboard->config.atlas.texture.id) {
                    unload = false; 
                }
                
                if(unload) UnloadTexture(Editor.emitters[Editor.active_emitter]->config.atlas.texture);
                
                // backup pointers
                Particle* particles = Editor.emitters[Editor.active_emitter]->particles.data;
                Color* colors = Editor.emitters[Editor.active_emitter]->config.gradient.colors;
                Force* forces = Editor.emitters[Editor.active_emitter]->config.forces.data;
                
                // overwrite emitter with the clipboard data
                *Editor.emitters[Editor.active_emitter] = *Editor.clipboard;
                memcpy(forces, Editor.clipboard->config.forces.data, MAX_FORCES*sizeof(Force));
                memcpy(colors, Editor.clipboard->config.gradient.colors, MAX_COLORS*sizeof(Color));
                
                // restore emitter pointers
                Editor.emitters[Editor.active_emitter]->particles.data = particles;
                Editor.emitters[Editor.active_emitter]->config.gradient.colors = colors;
                Editor.emitters[Editor.active_emitter]->config.forces.data = forces;
            }
        }
    }
    
    // ---------------------------------------------------------------------------------------
    // Handle dropped files
    // ---------------------------------------------------------------------------------------
    if(IsFileDropped())
    {
        char** file = NULL;
        int count = 0;
        file = GetDroppedFiles(&count);
        if(IsFileExtension(file[0], ".png")) 
        {
            // user dropped a texture
            if(Editor.active_emitter != -1) 
            {
                Texture t = LoadTexture(file[0]);
                if(t.id > 0) 
                {
                    bool unload = true;
                    // only unload if no other emitter has the texture in use 
                    for(int i=0; i<Editor.emitter_count; ++i) 
                    {
                        if(i != Editor.active_emitter && Editor.emitters[i]->config.atlas.texture.id == Editor.emitters[Editor.active_emitter]->config.atlas.texture.id) {
                            unload = false; 
                            break;
                        }
                    }
                    
                    // check for clipboard texture
                    if(Editor.clipboard != NULL && Editor.emitters[Editor.active_emitter]->config.atlas.texture.id == Editor.clipboard->config.atlas.texture.id) {
                        unload = false; 
                    }
                    
                    if(unload) UnloadTexture(Editor.emitters[Editor.active_emitter]->config.atlas.texture);
                    
                    Editor.emitters[Editor.active_emitter]->config.atlas.texture = t;
                    Editor.emitters[Editor.active_emitter]->config.atlas.hframes = Editor.emitters[Editor.active_emitter]->config.atlas.vframes = 0;
                    Editor.active_window = 2; // switch to texture window
                }
            } 
            else 
            {
                // dropped placeholder
                Texture t = LoadTexture(file[0]);
                if(t.id > 0) { 
                    UnloadTexture(Editor.placeholder);
                    Editor.placeholder = t;
                }
            }
        }
        else if(IsFileExtension(file[0], ".dps")) {
            if(!LoadEmitters(file[0])) TraceLog(LOG_WARNING, "Failed to load emitters");
        }
        ClearDroppedFiles();
    }

    
    // ---------------------------------------------------------------------------------------
    // Update emitters
    // ---------------------------------------------------------------------------------------
    Editor.statistics.updated = 0;
    for(int i=0; i<Editor.emitter_count; ++i) 
    {
        Editor.statistics.updated += EmitterUpdate(Editor.emitters[i]);
    }
    // ---------------------------------------------------------------------------------------
}

void RunEditor() 
{
    while(!WindowShouldClose()) 
    {
        UpdateEditor();
        
        BeginDrawing();
            ClearBackground(Editor.options.bg);
            
            DrawEditor();
            DrawGUI();
            
        EndDrawing();
    }
}

int main(int argc, char **argv)
{
    InitializeEditor();
    RunEditor();
    FinalizeEditor();
    
	return 0;
}

void DrawEditor() 
{
    // draw the grid below everything
    if(Editor.options.show_grid) DrawGridSystem();
    
    Editor.statistics.drawn = 0;
    Editor.statistics.pixels = 0;

    BeginMode2D(Editor.camera);
        // draw placeholder
        if(Editor.options.show_placeholder) 
            DrawTexture(Editor.placeholder, (GetScreenWidth()-Editor.placeholder.width)/2, (GetScreenHeight()-Editor.placeholder.height)/2, WHITE);
        
        EmitterExtraParams params = {0};
        params.screen = &((Rectangle){-Editor.camera.offset.x, -Editor.camera.offset.y, GetScreenWidth(), GetScreenHeight()});
        
        // draw emitters
        for(int i=0; i<Editor.emitter_count; ++i)
        {
            EmitterDraw(Editor.emitters[i], &params);
            
            if(Editor.options.show_debug && Editor.active_emitter == i) 
            {
                // draw debug info for active emitter
                Vector2 pos = Editor.emitters[i]->position;
                DrawCircleLines(pos.x, pos.y, 4.0f, Editor.options.debug);
                
                // draw the timers
                if(Editor.emitters[i]->life > 0.0f) {
                    if(Editor.emitters[i]->delay > 0.0f && Editor.emitters[i]->emit_timer < Editor.emitters[i]->delay) {
                        DrawRectangleLines(pos.x - 4,  pos.y + 6, 32, 4, Editor.options.debug);
                        DrawRectangle(pos.x - 4, pos.y + 7, 32.0f*Editor.emitters[i]->emit_timer/Editor.emitters[i]->delay, 3, Editor.options.debug);
                        DrawText(TextFormat("s %.2f", Editor.emitters[i]->emit_timer), pos.x - 4, pos.y + 12, 10, Editor.options.debug);
                    }
                    else if(Editor.emitters[i]->delay > 0.0f && Editor.emitters[i]->emit_timer > Editor.emitters[i]->delay + Editor.emitters[i]->life && 
                        Editor.emitters[i]->emit_timer < 2*Editor.emitters[i]->delay + Editor.emitters[i]->life)
                    {
                        float time = Editor.emitters[i]->emit_timer - Editor.emitters[i]->delay - Editor.emitters[i]->life;
                        DrawRectangleLines(pos.x - 4,  pos.y + 6, 32, 4, Editor.options.debug);
                        DrawRectangle(pos.x - 4, pos.y + 7, 32.0f*time/Editor.emitters[i]->delay, 2, Editor.options.debug);
                        DrawText(TextFormat("e %.2f", time), pos.x - 4, pos.y + 12, 10, Editor.options.debug);
                    }
                }
                
                if(Editor.emitters[i]->config.container.type == EMITTER_CIRCLE)
                    DrawCircleLines(pos.x, pos.y, Editor.emitters[i]->config.container.opt1, Editor.options.debug);
                else if(Editor.emitters[i]->config.container.type == EMITTER_RING)
                    DrawRingLines(pos, Editor.emitters[i]->config.container.opt1, Editor.emitters[i]->config.container.opt2, 0.0f, 360.0f, 0, Editor.options.debug);
                else if(Editor.emitters[i]->config.container.type == EMITTER_RECT) 
                {
                        const float width = Editor.emitters[i]->config.container.opt1;
                        const float height = Editor.emitters[i]->config.container.opt2;
                        DrawRectangleLines(pos.x-width/2, pos.y-height/2, width, height, Editor.options.debug);
                }
            }
        }

        Editor.statistics.drawn = params.drawn;
        Editor.statistics.pixels = params.pixels;
    EndMode2D();
}

// Generates a nice color with a random hue
inline Color GenerateRandomColor(float s, float v)
{
    const float Phi = 0.618033988749895; // golden ratio conjugate
    float h = GetRandomValue(0, 360);
    h = fmodf((h + h*Phi), 360.0f);
    return ColorFromHSV(h, s, v);
}

static inline void SetDefaultEmitterConfig(Emitter* e) 
{
    e->config.emission = 10;
    e->config.pulses = 0;
    e->config.size.min = 1.0f;
    e->config.size.max = 2.0f;
    e->config.scale.start = e->config.scale.end = 1.0f;
    e->config.age.min = e->config.age.max = 4.0f;
    e->config.speed.min = e->config.speed.max = 80.0f;
    e->config.angle.max = 360.0f;
    e->config.easing = &EaseLinearNone;
    e->delay = 0.0f;
    e->life = 4.0f;
}

void EditorAddEmitter(Vector2 loc) 
{
    if(Editor.emitter_count < MAX_EMITTERS)
    {
        int pos = Editor.emitter_count;
        
        // allocate memory for emitter
        if(Editor.emitters[pos] == NULL) {
            Editor.emitters[pos] = calloc(1, sizeof(Emitter));
            if(Editor.emitters[pos] == NULL) TraceLog(LOG_FATAL, "EMITTER: Failed to allocate memory");
            Editor.statistics.total_mem += sizeof(Emitter);
        }
        
        *Editor.emitters[pos] = (Emitter){0};
        // set default configuration for the new emitter
        SetDefaultEmitterConfig(Editor.emitters[pos]);
        
        // allocate memory for emitter particles
        if(Editor.emitters[pos]->particles.data == NULL) {
            Editor.emitters[pos]->particles.data = calloc(MAX_PARTICLES, sizeof(Particle));
            if(Editor.emitters[pos]->particles.data == NULL) TraceLog(LOG_FATAL, "PARTICLES: Failed to allocate memory");
            Editor.emitters[pos]->particles.max = MAX_PARTICLES;
            Editor.statistics.total_mem += MAX_PARTICLES*sizeof(Particle);
        }
        Editor.emitters[pos]->particles.count = 0;
        
        // allocate memory for the colors
        if(Editor.emitters[pos]->config.gradient.colors == NULL) {
            Editor.emitters[pos]->config.gradient.colors = calloc(MAX_COLORS, sizeof(Color));
            if(Editor.emitters[pos]->config.gradient.colors == NULL) TraceLog(LOG_FATAL, "COLORS: Failed to allocate memory");
            Editor.statistics.total_mem += MAX_COLORS*sizeof(Color);
        }
        Editor.emitters[pos]->config.gradient.count = 1; // at least one color should always be set
        Editor.emitters[pos]->config.gradient.colors[0] = GenerateRandomColor(0.4f, 0.89f);
        
        // allocate memory for the forces
        if(Editor.emitters[pos]->config.forces.data == NULL) {
            Editor.emitters[pos]->config.forces.data = calloc(MAX_FORCES, sizeof(Force));
            if(Editor.emitters[pos]->config.forces.data == NULL) TraceLog(LOG_FATAL, "FORCES: Failed to allocate memory");
            Editor.statistics.total_mem += MAX_FORCES*sizeof(Force);
        }
        Editor.emitters[pos]->config.forces.count = 0;
        
        Editor.emitters[pos]->position = loc;
        
        Editor.emitter_count += 1;
    }
}

void EditorRemoveEmitter()
{
    if(Editor.emitter_count > 0)
    {
        int pos = Editor.emitter_count - 1;
        if(Editor.active_emitter == pos) Editor.active_emitter = pos - 1;
        
        free(Editor.emitters[pos]->particles.data);
        free(Editor.emitters[pos]->config.gradient.colors);
        free(Editor.emitters[pos]->config.forces.data);
        
        if(Editor.clipboard == NULL || (Editor.clipboard != NULL && Editor.clipboard->config.atlas.texture.id != Editor.emitters[pos]->config.atlas.texture.id))
            UnloadTexture(Editor.emitters[pos]->config.atlas.texture);
        
        free(Editor.emitters[pos]);
        Editor.emitters[pos] = NULL;
        
        Editor.emitter_count -= 1;
        Editor.statistics.total_mem -= sizeof(Emitter) + MAX_PARTICLES*sizeof(Particle) + MAX_COLORS*sizeof(Color) + MAX_FORCES*sizeof(Force);
    }
}

void EditorMoveUpEmitter()
{
    if(Editor.active_emitter > 0) 
    {
        // swap current active emitter with the one above it
        Emitter* tmp = Editor.emitters[Editor.active_emitter];
        Editor.emitters[Editor.active_emitter] = Editor.emitters[Editor.active_emitter - 1];
        Editor.emitters[Editor.active_emitter - 1] = tmp;
        
        //do the same with the ids
        int tid = Editor.emitter_id[Editor.active_emitter];
        Editor.emitter_id[Editor.active_emitter] = Editor.emitter_id[Editor.active_emitter - 1];
        Editor.emitter_id[Editor.active_emitter - 1] = tid;
        Editor.active_emitter -= 1;
    }
}

void EditorMoveDownEmitter()
{
    if(Editor.active_emitter != -1 && Editor.active_emitter + 1 < Editor.emitter_count) {
        // swap current active emitter with the one below it
        Emitter* tmp = Editor.emitters[Editor.active_emitter];
        Editor.emitters[Editor.active_emitter] = Editor.emitters[Editor.active_emitter + 1];
        Editor.emitters[Editor.active_emitter + 1] = tmp;
        
        //do the same with the ids
        int tid = Editor.emitter_id[Editor.active_emitter];
        Editor.emitter_id[Editor.active_emitter] = Editor.emitter_id[Editor.active_emitter + 1];
        Editor.emitter_id[Editor.active_emitter + 1] = tid;
        
        Editor.active_emitter += 1;
    }
}

void EditorSyncEmitters()
{
    // reset all emitter particles so they are in sync
    for(int i=0; i<Editor.emitter_count; ++i) {
        memset(Editor.emitters[i]->particles.data, 0, MAX_PARTICLES*sizeof(Particle));
        Editor.emitters[i]->particles.count = 0;
        Editor.emitters[i]->spawn_timer = 0.0f;
        Editor.emitters[i]->emit_timer = 0.0f;
    }
}



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

#pragma once

#include "particles.h"

#define EDITOR_VER "1.04 - ALPHA"

#define MAX_EMITTERS 8
#define MAX_PARTICLES 2000
#define MAX_COLORS 16
#define MAX_FORCES 4
#define MAX_NAME_LEN 32
#ifndef SIZEOF
#define SIZEOF(A) (sizeof(A)/sizeof(A[0]))
#endif


struct SEditor 
{
    Emitter* emitters[MAX_EMITTERS];
    int emitter_id[MAX_EMITTERS];
    int emitter_count;
    
    int active_emitter;
    int active_color;
    int active_window;
    
    Camera2D camera;
    Vector2 offset, mouse;      // used when moving
    
    Texture placeholder;
    
    Emitter* clipboard;
    
    struct SOptions 
    {
        bool show_placeholder;
        bool show_grid;
        bool show_debug;
        Color gridcolor;
        Color debug;
        Color fg;
        Color bg;
    } options;
    
    struct SStatistics {
        bool shown;
        int drawn;      // total number of particles drawn on the screen each frame
        int updated;    // total number of particles updated per frame
        unsigned long long  pixels;
        int total_mem;
    } statistics;
    
    char name[MAX_NAME_LEN];
} Editor;

// ---------------------------------------------------------------------------------------
// Global functions
// ---------------------------------------------------------------------------------------
Color GenerateRandomColor(float s, float v);
void SetDefaultOptions();
void DrawGUI();
void DrawGridSystem();
bool CanMoveEmitter();
void DeallocateEmitters();
int SaveEmitters(const char* file);
int LoadEmitters(const char* file);
void EditorAddEmitter(Vector2 loc);
void EditorRemoveEmitter(void);
void EditorMoveUpEmitter(void);
void EditorMoveDownEmitter(void);
void EditorSyncEmitters(void);
// ---------------------------------------------------------------------------------------

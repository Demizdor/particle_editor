/*  =========================================================================
    A particle library for raylib (https://github.com/raysan5/raylib) to be 
    used as a header only library.
    WARNING: This is a very early alpha version so take care when using.

    Make sure to #define LIB_RAY_PARTICLES_IMPL in exactly one source file to
    include the implementation.
    
    So far i haven't decided on a stable API so only 2 functions are exposed
    =========================================================================
    LICENSE: zlib
    Copyright (C) 2021 Vlad Adrian (@Demizdor - https://github.com/Demizdor)
    =========================================================================
*/


#pragma once

#include <raylib.h>
#include <raymath.h>
#include <easings.h>
#include <stdlib.h>

#include <math.h>

#define FLAG_SET(n, f) ((n) |= (f))
#define FLAG_CLEAR(n, f) ((n) &= ~(f))
#define FLAG_TOGGLE(n, f) ((n) ^= (f))
#define FLAG_CHECK(n, f) ((n) & (f))

typedef struct {
    Vector2 origin;             // Position of the particle when spawned
    Vector2 direction;          // Direction vector normalized
    Vector2 position;           // Current position
    float size;                 // Initial size
    float speed;                // Initial speed
    float time;                 // Curent particle age
    float life;                 // Total life
    float angle;                // Starting angle (information is stored in the directional vector but needed by EMITTER_FLAG_DIRECTIONAL_ROTATION)
    int tidx;                   // Index in a multitexture (used only when EMITTER_FLAG_MULTITEXTURE is set)
    
    // FIXME: the struct is getting way too big (3*8 + 6*4 = 48 bytes)
    // think of a way to make it smaller while still retaining all the fields
    // maybe calculate the direction each frame (since we already have the angle) 
    // and just use a random seed when spawned and calculate (mostly) everything from that seed, something like below
    // struct { Vector2 origin, position; float time; int seed; } Particle; // (2*8+2*4 = 24 bytes, much better)
} Particle;

typedef float (*Easing)(float, float, float, float);

typedef enum {
    EMITTER_POINT = 0,
    EMITTER_RECT,
    EMITTER_CIRCLE,
    EMITTER_RING,
    //TODO: add an EMITTER_ELIPSE and maybe EMITTER_TRI
} EmitterType;

typedef enum {
    EMITTER_FLAG_DISABLED = 1 << 0,                 // Is the emitter disabled (default is false)
    EMITTER_FLAG_PAUSED = 1 << 1,                   // Is the emitter currently paused? (default is false)
    EMITTER_FLAG_SPAWN_INSIDE = 1 << 2,             // Spawn particles inside or outside container (default outside) (used for everything except EMITTER_POINT)
    EMITTER_FLAG_REVERSE_DRAW_ORDER = 1 << 3,       // Draw particles new -> old or old -> new(default)
    EMITTER_FLAG_WORLD_SPACE = 1 << 4,              // Particle has origin in local space or world space(default)
    EMITTER_FLAG_DRAW_TRIANGLES = 1 << 5,           // When no texture is present draw triangles or squares(default)
    EMITTER_FLAG_DRAW_OUTLINE = 1 << 6,             // When no texture is present draw outlines instead of filled shapes(default)
    EMITTER_FLAG_DIRECTIONAL_ROTATION = 1 << 7,     // Rotate texture in the direction of movement or not (default)
    EMITTER_FLAG_LOOP = 1 << 8,                     // Stop emitting when dead(default) or loop from start
    EMITTER_FLAG_MULTITEXTURE = 1 << 9,             // Is the main texture a multitexture or not (default)
} EmitterFlags;

typedef struct {
    float direction;        // Direction of the force in degrees
    float strength;         // Strength (positive or negative)
} Force;

typedef struct {
    int emission;           // Maximum number of particles to spawn, when this is reached no particles will spawn until some die
    int pulses;
    
    struct { 
        float min, max; 
    }   size,               // A random size will be selected when spawned
        angle,              // Restrict particles spawn only between these angles
        age,                // A random age will be selected for each particle
        offset,             // A random offset applied to the particle origin
        speed;              // A random speed will be selected for each particle
    
    struct {
        float start, end;
    }   scale,              // Multiplier applied to the size of the particle
        acc,                // Linear acceleration
        tacc,               // Tangential acceleration
        rotation;           
        
    struct {
        Force* data;        // Array holding all the forces affecting the emitter
        int count;          // Number of forces active on the emitter
    } forces;
        
    struct {
        Texture2D texture;      // Main texture
        int hframes, vframes;   // How many horizontal/vertical frames there are (or multitextures when EMITTER_FLAG_MULTITEXTURE is set)
        int loop;               // How many times to loop the animation (1 -> n)
    } atlas;
    
    struct {
        Color* colors;
        int count;
    } gradient;             // Color gradient that will be applied to all particles over time
    
    Easing easing;          // Easing used when updating particles each frame
    
    struct {
        EmitterType type;   // Type of the emitter container
        float opt1;         // First option (depending on the container type)
        float opt2;         // Second option (depending on the container type)
    } container;
    
} EmitterConfig;


typedef struct {
    Vector2 position;
    
    EmitterConfig config;   // Main configuration for the emitter
    
    struct {
        Particle* data;     // Array of particles for this emitter (shouldn't be NULL)
        int count;          // Number of particles that are alive
        int max;            // Max capacity of the array
    } particles;
    
    float life;             // Life of the emitter in seconds
    float delay;            // How long to wait until the emitter emits particles after it is dead (only if EMITTER_FLAG_LOOP is set)
    BlendMode mode;         // Emitter blend mode (currently only the raylib blend modes are supported)
    int flags;
    
    // PRIVATE MEMBERS - SHOULDN'T  BE CHANGED BY THE USER
    float spawn_timer;      // Time since last spawned particle
    float emit_timer;       // Time since emitting particles
    
} Emitter;

// Extra params used when drawing the emitter
typedef struct {
    Rectangle* screen;              // If not null particles will drawn only if inside this screen area (used to cull particles)
    int drawn;                      // Number of particles drawned each frame
    unsigned long long pixels;      // Number of pixels drawn each frame
} EmitterExtraParams;


// Update emitter `e`. should be called before `EmitterDraw()`
extern int EmitterUpdate(Emitter* e);
// Draw emitter `e` using some extra params. Should be called after `EmitterUpdate()`
extern void EmitterDraw(Emitter* e, EmitterExtraParams* params);
// Get a random float between 0.0 and 1.0
extern float GetRandomFloat();
// Get a random float between min and max
extern float GetRandomFloatBetween(float min, float max);




// define this in exactly one source file to include the implementation
// #define LIB_RAY_PARTICLES_IMPL

#ifdef LIB_RAY_PARTICLES_IMPL

// Get a random float between 0.0 and 1.0
inline float GetRandomFloat() {
    return (float)GetRandomValue(0, RAND_MAX)/RAND_MAX;
}

// Get a random float between min and max
inline float GetRandomFloatBetween(float min, float max) {
    return min + (max - min)*GetRandomFloat();
}

// Rotates point `p` `a` degrees around origin `o`
static inline Vector2 RotatePointOnCircle(Vector2 o, Vector2 p, float a) {
    const float ra = a*DEG2RAD;
    const float s = sin(ra);
    const float c = cos(ra);
    return (Vector2){o.x + (p.x-o.x)*c - (p.y-o.y)*s, o.y + (p.x-o.x)*s + (p.y-o.y)*c };
}

static inline void ParticleUpdate(Emitter* e, Particle* p) {
    float dt = GetFrameTime();
    if(dt == 0.0f) dt = 0.0016f;
    
    const Easing easing = e->config.easing;
    
    // calculate speed and acceleration
    const float speed = (p->speed + easing(p->time, e->config.acc.start, e->config.acc.end - e->config.acc.start, p->life))*dt;
    Vector2 npos = Vector2Add(p->position, Vector2Scale(p->direction, speed));
    
    // calculate tangential acceleration
    const float tacc = easing(p->time, e->config.tacc.start, e->config.tacc.end - e->config.tacc.start, p->life)*dt;
    if(tacc != 0.0f) {
        Vector2 n = Vector2Subtract(npos, p->origin);
        float angle = 90.0f;
        if(tacc < 0.0f) {
            n = Vector2Subtract(p->origin, npos);
            angle = -90.0f;
        }
        
        n = Vector2Normalize(n);
        //#define MIN_RADIUS 20.0f
        //Vector2 t = Vector2Add(npos, Vector2Scale(n, tacc*(Vector2Distance(npos, p->origin)/MIN_RADIUS) ));
        // FIXME: `tacc` needs to depend on the distance to origin (larger distance -> bigger effect)
        Vector2 t = Vector2Add(npos, Vector2Scale(n, tacc));
        p->position = RotatePointOnCircle(npos, t, angle);
    }
    else p->position = npos;
    
    // TODO: just adding the forces together..hmmm, is this correct?!
    Vector2 forces = {0.0f, 0.0f};
    for(int i=0; i<e->config.forces.count; ++i) {
        Force* f = &e->config.forces.data[i];
        const float angle = f->direction*DEG2RAD;
        Vector2 direction = Vector2Normalize((Vector2){cosf(angle), sinf(angle)});
        forces = Vector2Add(forces, Vector2Scale(direction, f->strength*dt));
    }
    
    p->position = Vector2Add(p->position, forces);
    
    p->time += dt;
}

// Keep angle between 0-360
static inline float NormalizeAngle(float angle) {
    angle = fmodf(angle, 360.0);
	if (angle < 0.0f) angle += 360.0;
    return angle;
}

// Get quadrant from angle (0-7 quadrants each 45 degrees)
static inline int GetQuadrant(float angle) {
	return (int)fmodf(NormalizeAngle(angle)/45.0f, 8.0);
}

static Particle ParticleGenerate(Emitter* e) {
    Particle p;
    
    // Get offset and angle
    const Vector2 offset = {GetRandomFloatBetween(e->config.offset.min, e->config.offset.max), 
        GetRandomFloatBetween(e->config.offset.min, e->config.offset.max)};
    p.angle = GetRandomFloatBetween(e->config.angle.min, e->config.angle.max);
    const float angle = p.angle*DEG2RAD;
    
    // Generate a random multitexture index if EMITTER_FLAG_MULTITEXTURE is set
    if(e->config.atlas.hframes*e->config.atlas.vframes > 1) p.tidx = GetRandomValue(0, e->config.atlas.hframes*e->config.atlas.vframes-1);
    
    switch(e->config.container.type) 
    {
        case EMITTER_POINT:
            p.origin = offset;
            p.direction = Vector2Normalize((Vector2){cosf(angle), sinf(angle)});
        break;
        
        case EMITTER_RECT: {
            const float width = e->config.container.opt1;
            const float height = e->config.container.opt2;
            
            if(!FLAG_CHECK(e->flags, EMITTER_FLAG_SPAWN_INSIDE)) {
                // spawn outside the rectangle (is there a better way to do this?!?)
                const float a = angle*RAD2DEG;
                const int q = GetQuadrant(a); // divide the rectangle in 8(0-7) quadrants 45degrees each
                const float pc = fmodf(NormalizeAngle(a), 45.0f) / 45.0f; // calculate percent of the side to fill
                // calculate x/y coordinates for each quadrant
                const float qw[] = { width/2, (width/2)*(1.0-pc), (-width/2)*pc, -width/2, -width/2, (-width/2)*(1.0-pc), (width/2)*pc, width/2};
                const float qh[] = { (height/2)*pc, height/2, height/2, (height/2)*(1.0-pc), (-height/2)*pc, -height/2, -height/2, (-height/2)*(1.0-pc)};
                // set coordinates depending on quadrant
                p.origin.x = qw[q];
                p.origin.y = qh[q];
            } else {
                // randomly spawn inside the rectangle
                p.origin.x = GetRandomValue(-width/2, width/2);
                p.origin.y = GetRandomValue(-height/2, height/2);
            }
            p.direction = Vector2Normalize((Vector2){width*cosf(angle), height*sinf(angle)});
            p.origin = Vector2Add(p.origin, offset);
        }
        break;
        
        case EMITTER_CIRCLE: 
        {
            const float r = e->config.container.opt1;
            p.direction = Vector2Normalize((Vector2){r*cosf(angle), r*sinf(angle)});
            if(!FLAG_CHECK(e->flags, EMITTER_FLAG_SPAWN_INSIDE)) p.origin = Vector2Scale(p.direction, r); // spawn outside the circle
            else p.origin = Vector2Scale(p.direction, GetRandomFloatBetween(0.0f, r)); // spawn inside circle
            p.origin = Vector2Add(p.origin, offset);
        }
        break;
        
        case EMITTER_RING:
        {
            float ra = e->config.container.opt1;
            float rb = e->config.container.opt2;
            
            if(ra > rb) { // swap if needed
                const float tmp = ra;
                ra = rb;
                rb = tmp;
            }
            
            p.direction = Vector2Normalize((Vector2){rb*cosf(angle), rb*sinf(angle)});
            if(!FLAG_CHECK(e->flags, EMITTER_FLAG_SPAWN_INSIDE)) {
                const float rnd = GetRandomFloat(); // get a random float to see where should we spawn
                if(rnd >= 0.5f)
                    p.origin = Vector2Scale(p.direction, rb); // spawn outside outer ring
                else { 
                    p.origin = Vector2Scale(p.direction, ra); // spawn inside inner ring
                    p.direction = Vector2Negate(p.direction); // inverse direction
                    p.angle *= -1.0f;
                }
            } else {
                p.origin = Vector2Scale(p.direction, GetRandomFloatBetween(ra, rb)); // spawn inbetween rings
            }
             p.origin = Vector2Add(p.origin, offset);
        }
        break;
    }
    
    if(!FLAG_CHECK(e->flags, EMITTER_FLAG_WORLD_SPACE)) {
        p.origin.x += e->position.x;
        p.origin.y += e->position.y;
    }
    p.position = p.origin;
    
    // Calculate initial particle size and speed
    p.size = GetRandomFloatBetween(e->config.size.min, e->config.size.max);
    p.speed = GetRandomFloatBetween(e->config.speed.min, e->config.speed.max);
    
    // Calculate particle life
    p.life = GetRandomFloatBetween(e->config.age.min, e->config.age.max); 
    p.time = 0.0f;
    
    return p;
}

int EmitterUpdate(Emitter* e) {
    if(FLAG_CHECK(e->flags, EMITTER_FLAG_DISABLED) || FLAG_CHECK(e->flags, EMITTER_FLAG_PAUSED)) 
        return 0; // don't update when paused or disabled
    
    // Emit particles
    if(e->particles.count < e->config.emission && e->life != 0.0f && e->emit_timer > e->delay && e->emit_timer < e->delay + e->life) 
    {
        const float duration = e->life;
        const float dt = GetFrameTime();
        
        // FIXME: hmmm... this is wrong!!! not all the particles are emitted.
        float tick = dt;
        int rate = (float)e->config.emission/duration*dt;
        
        if(e->config.pulses != 0) {
            rate = e->config.emission/e->config.pulses; // rate per pulse
            tick = duration/e->config.pulses; // time of each pulse
        }
        
        if(e->particles.count == 0) tick = e->spawn_timer;
        
        if(rate + e->particles.count > e->config.emission) rate = e->config.emission - e->particles.count;
        if(e->spawn_timer >= tick) 
        {
            e->spawn_timer -= tick;
            for(int i=0, r=0; i<e->particles.max; ++i) {
                if(e->particles.data[i].life == 0.0f) {
                    e->particles.data[i] = ParticleGenerate(e);
                    e->particles.count += 1;
                    if(++r >= rate) break;
                }
            }
        }
        e->spawn_timer += dt;
    }
    
    // Update particles
    int updated = 0;
    for(int i=0; i<e->particles.max && e->particles.count > 0; ++i) {
        if(e->particles.data[i].life != 0.0f) {
            if(e->particles.data[i].time >= e->particles.data[i].life) {
                // remove particle
                e->particles.count -= 1;
                if(e->particles.count < 0 ) e->particles.count = 0;
                e->particles.data[i].life = 0.0f;
            } else {
                ParticleUpdate(e, &e->particles.data[i]);
                if(i > 0 && e->particles.data[i-1].life > e->particles.data[i].life) 
                {
                    // overtime swapping values like this will autosort the particle array (over time)
                    Particle tmp = e->particles.data[i];
                    e->particles.data[i] = e->particles.data[i-1];
                    e->particles.data[i-1] = tmp;
                }
                ++updated;
            }
        }
    }
    
    // Handle emitting in a loop
    if(FLAG_CHECK(e->flags, EMITTER_FLAG_LOOP)) 
    {
        if(e->emit_timer > 2*e->delay + e->life) { e->emit_timer = e->delay; }
        else { e->emit_timer += GetFrameTime(); }
    }
    else 
    {
        if(e->emit_timer < 2*e->delay + e->life) 
            e->emit_timer += GetFrameTime();
    }
    
    return updated;
}

static inline Color MixColors(Emitter* e, Color a, Color b, float st, float et) {
    int cr = e->config.easing(st, a.r, b.r-a.r, et);
    cr = Clamp(cr, 0, 255);
    int cg = e->config.easing(st, a.g, b.g-a.g, et);
    cg = Clamp(cg, 0, 255);
    int cb = e->config.easing(st, a.b, b.b-a.b, et);
    cb = Clamp(cb, 0, 255);
    
    Color color = {cr, cg, cb, b.a};
    if(b.a != a.a) 
    {
        int ca = e->config.easing(st, a.a, b.a-a.a, et);
        color.a = Clamp(ca, 0, 255);
    }
    
	return color;
}

static inline Color Interpolate(Emitter* e, Particle* p) { 
    if(e->config.gradient.colors == NULL || e->config.gradient.count == 0) return RED;
    if(e->config.gradient.count == 1) return e->config.gradient.colors[0]; // no need to interpolate since there's only one color
    
    const int max = e->config.gradient.count - 1;
    const float u = p->life/max;
    int idx = 0;
    idx = floorf(p->time*max/p->life);
    
    float st = fmodf(p->time, u);
    if(st+GetFrameTime() > u) st = u;

    return MixColors(e, e->config.gradient.colors[idx], e->config.gradient.colors[idx+1], st, u);
}

void EmitterDraw(Emitter* e, EmitterExtraParams* params) 
{
    // NOTE: this code has been (somewhat) optimised but still slow :(
    if(!FLAG_CHECK(e->flags, EMITTER_FLAG_DISABLED) && e->particles.count > 0) // don't draw when disabled
    {
        BeginBlendMode(e->mode);
        int start = 0, end = e->particles.max, step = 1;
        if(FLAG_CHECK(e->flags, EMITTER_FLAG_REVERSE_DRAW_ORDER)) 
        { 
            // drawn particles in reverse order
            start = e->particles.max - 1;
            end = -1;
            step = -1;
        }
        
        if(e->config.atlas.texture.id != 0) 
        {
            // DRAW TEXTURED PARTICLES
            for(int i=start; i!=end; i+=step)
            {
                Particle* p = &e->particles.data[i];
                if(p->life != 0.0f)
                {
                    float size = p->size*e->config.easing(p->time, e->config.scale.start, e->config.scale.end - e->config.scale.start, p->life);
                    
                    float rotst = e->config.rotation.start;
                    if(FLAG_CHECK(e->flags, EMITTER_FLAG_DIRECTIONAL_ROTATION))  rotst += p->angle;
                    float rotation = e->config.easing(p->time, rotst, e->config.rotation.end - e->config.scale.start, p->life);
                    
                    Vector2 center = p->position;
                    if(FLAG_CHECK(e->flags, EMITTER_FLAG_WORLD_SPACE)) center = Vector2Add(center, e->position);
                    
                    float w = (float)e->config.atlas.texture.width*size;
                    float h = (float)e->config.atlas.texture.height*size;
                    if(e->config.atlas.hframes*e->config.atlas.vframes > 1) {
                        w = (float)e->config.atlas.texture.width/e->config.atlas.hframes*size;
                        h = (float)e->config.atlas.texture.height/e->config.atlas.vframes*size;
                    }
                    Vector2 vertex[4] = { (Vector2){center.x-w/2, center.y-h/2}, 
                            (Vector2){center.x+w/2, center.y-h/2},
                            (Vector2){center.x+w/2, center.y+h/2}, 
                            (Vector2){center.x-w/2, center.y+h/2}, 
                    };
                        
                    if(rotation != 0.0f) { 
                        // rotate the vertices of the quad that holds the texture
                        vertex[0] = RotatePointOnCircle(center, vertex[0], rotation);
                        vertex[1] = RotatePointOnCircle(center, vertex[1], rotation);
                        vertex[2] = RotatePointOnCircle(center, vertex[2], rotation);
                        vertex[3] = RotatePointOnCircle(center, vertex[3], rotation);
                    }
                    
                    int inside = true;
                    if(params->screen != NULL) 
                    {
                        // check rotated points to see if at least one is inside the screen area
                        inside = CheckCollisionPointRec(vertex[0], *params->screen) | CheckCollisionPointRec(vertex[1], *params->screen) |
                            CheckCollisionPointRec(vertex[2], *params->screen) | CheckCollisionPointRec(vertex[3], *params->screen);
                    }
                    
                    if(inside) 
                    {
                        // Get current color by interpolating
                        Color color = Interpolate(e, p);
                        
                        if(e->config.atlas.hframes*e->config.atlas.vframes <= 1)
                        {
                            // DRAW STATIC TEXTURE
                            DrawTexturePro(e->config.atlas.texture, (Rectangle){0.0f, 0.0f, e->config.atlas.texture.width, e->config.atlas.texture.height}, 
                                (Rectangle){vertex[0].x, vertex[0].y, w, h}, (Vector2){0.0f, 0.0f}, rotation, color);
                        }
                        else 
                        {
                            // DRAW ANIMATED TEXTURE OR MULTITEXTURED PARTICLES
                            int frame = p->tidx; // set multitexture index
                            if(!FLAG_CHECK(e->flags, EMITTER_FLAG_MULTITEXTURE)) {
                                // This is a animated texture so get the current frame of animation
                                frame = e->config.easing(p->time, 0, e->config.atlas.vframes*e->config.atlas.hframes*e->config.atlas.loop-1, p->life);
                                frame = Clamp(frame, 0.0f, e->config.atlas.vframes*e->config.atlas.hframes*e->config.atlas.loop-1);    
                            }
                            
                            Rectangle src = {0.0f, 0.0f, 
                                (float)e->config.atlas.texture.width/e->config.atlas.hframes, 
                                (float)e->config.atlas.texture.height/e->config.atlas.vframes
                            };
                            
                            src.x = (frame%e->config.atlas.hframes)*src.width;
                            src.y = ((int)floorf(frame/e->config.atlas.hframes)%e->config.atlas.vframes)*src.height;
                            DrawTexturePro(e->config.atlas.texture, src, (Rectangle){vertex[0].x, vertex[0].y, w, h}, (Vector2){0.0f, 0.0f}, rotation, color);
                        }
                        params->pixels += w*h;
                        params->drawn++;
                    }
                    /* draw bounding box
                    DrawLineEx(vertex[0], vertex[1], 2.0f, RED);
                    DrawLineEx(vertex[1], vertex[2], 2.0f, RED);
                    DrawLineEx(vertex[2], vertex[3], 2.0f, RED);
                    DrawLineEx(vertex[3], vertex[0], 2.0f, RED);
                     */
                }
            }
        } 
        else 
        {
            // DRAW UNTEXTURED PARTICLES
            for(int i=start; i!=end; i+=step)
            {
                Particle* p = &e->particles.data[i];
                
                if(p->life != 0.0f)
                {
                    float size = p->size*e->config.easing(p->time, e->config.scale.start, e->config.scale.end - e->config.scale.start, p->life);
                    float rotation = e->config.easing(p->time, e->config.rotation.start, e->config.rotation.end - e->config.rotation.start, p->life);
                    Color color = Interpolate(e, p);
                    
                    Vector2 center = p->position;
                    if(FLAG_CHECK(e->flags, EMITTER_FLAG_WORLD_SPACE)) center = Vector2Add(center, e->position);
                    
                    if(!FLAG_CHECK(e->flags, EMITTER_FLAG_DRAW_TRIANGLES)) 
                    {
                        // DRAW SQUARES
                        Vector2 point[5] = { (Vector2){center.x-size/2, center.y-size/2}, 
                            (Vector2){center.x+size/2, center.y-size/2}, 
                            (Vector2){center.x+size/2, center.y+size/2}, 
                            (Vector2){center.x-size/2, center.y+size/2}, 
                            (Vector2){center.x-size/2, center.y-size/2}
                        };
                        
                        if(rotation != 0.0f) { 
                            // rotate the points of the rectangle
                            point[0] = RotatePointOnCircle(center, point[0], rotation);
                            point[1] = RotatePointOnCircle(center, point[1], rotation);
                            point[2] = RotatePointOnCircle(center, point[2], rotation);
                            point[3] = RotatePointOnCircle(center, point[3], rotation);
                            point[4] = point[0]; // the rect has only 4 points, the 5th point is just used by DrawLineStrip()
                        }
                        
                        int inside = true;
                        
                        if(params->screen != NULL) {
                            // check rotated points to see if at least one is inside the screen area
                            inside = CheckCollisionPointRec(point[0], *params->screen) | CheckCollisionPointRec(point[1], *params->screen) |
                                CheckCollisionPointRec(point[2], *params->screen) | CheckCollisionPointRec(point[3], *params->screen);
                        }
                        
                        if(inside) 
                        {
                            if(!FLAG_CHECK(e->flags, EMITTER_FLAG_DRAW_OUTLINE)) {
                                DrawRectanglePro((Rectangle){point[0].x, point[0].y, size, size}, (Vector2){0.0f, 0.0f}, rotation, color);
                                params->pixels += size*size;
                            }
                            else {
                                DrawLineStrip((Vector2*)&point, 5, color);
                                params->pixels += 2*(size+size);
                            }
                            params->drawn++;
                        }
                    } 
                    else 
                    {
                        // DRAW TRIANGLES
                        Vector2 point[4] = {(Vector2){center.x, center.y-size/2}, 
                            (Vector2){center.x+size/2, center.y+size/2}, 
                            (Vector2){center.x-size/2, center.y+size/2},
                            (Vector2){center.x, center.y-size/2},
                        };
                        
                        if(rotation != 0.0f) { 
                            // rotate the points of the rectangle
                            point[0] = RotatePointOnCircle(center, point[0], rotation);
                            point[1] = RotatePointOnCircle(center, point[1], rotation);
                            point[2] = RotatePointOnCircle(center, point[2], rotation);
                            point[3] = point[0]; // the triangle has only 3 points, the 4th point is just used by DrawLineStrip()
                        }
                        
                        int inside = true;
                        if(params->screen != NULL) {
                            // check rotated points to see if at least one is inside the screen area
                            inside = CheckCollisionPointRec(point[0], *params->screen) | CheckCollisionPointRec(point[1], *params->screen) |
                                CheckCollisionPointRec(point[2], *params->screen);
                        }
                        
                        if(inside) {
                            if(!FLAG_CHECK(e->flags, EMITTER_FLAG_DRAW_OUTLINE)) {
                                DrawTriangle(point[0], point[2], point[1], color);
                                params->pixels += (size*size*sqrtf(3))/4;
                            }
                            else {
                                DrawLineStrip((Vector2*)&point, 4, color);
                                params->pixels += 3*size;
                            }
                            
                            params->drawn++;
                        }
                        
                    }
                }
            }
        }
        EndBlendMode();
    }
}


#endif  // LIB_RAY_PARTICLES_IMPL
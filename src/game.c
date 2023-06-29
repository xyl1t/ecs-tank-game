#include <malloc/_malloc.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SGL_IMPLEMENTATION
#include "common.h"
#include "game.h"
#include "sgl.h"
#include "util.h"
#if __has_include("SDL2/SDL.h")
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

void draw_bullet(uint32_t id);
void draw_player(uint32_t id);
void draw_obstacle(uint32_t id);
void draw_bg(uint32_t id);

static World* world;
// static uint32_t* bg_pixels;
// static sglBuffer* bg_buffer;

GAME_MAIN(game_main) {
    if (op == OP_LOAD) {
        world = w;
        init_ecs();
    } else if (op == OP_INIT) {
        world = w;
        init_game();
    } else if (op == OP_INJECT) {
        world = w;
        inject_code_on_reload();
    } else if (op == OP_STEP) {
        step();
    }
    return 0;
}

void init_ecs(void) {
    world->player_components = calloc(MAX_ENTITIES, sizeof(Player));
    world->position_components = calloc(MAX_ENTITIES, sizeof(Position));
    world->velocity_components = calloc(MAX_ENTITIES, sizeof(Velocity));
    world->acceleration_components = calloc(MAX_ENTITIES, sizeof(Acceleration));
    world->direction_components = calloc(MAX_ENTITIES, sizeof(Direction));
    world->circle_collider_components =
        calloc(MAX_ENTITIES, sizeof(CircleCollider));
    world->capsule_collider_components =
        calloc(MAX_ENTITIES, sizeof(CapsuleCollider));
    world->projectile_components = calloc(MAX_ENTITIES, sizeof(Projectile));
    world->time_to_live_components = calloc(MAX_ENTITIES, sizeof(TimeToLive));
    world->appearance_components = calloc(MAX_ENTITIES, sizeof(Appearance));

    world->component_masks = calloc(MAX_ENTITIES, sizeof(uint32_t));

    world->ids = calloc(MAX_ENTITIES, sizeof(uint32_t));
    world->next_id = 1; // NOTE: 0 could have a special meaning, similar to NULL
    world->entity_states = calloc(MAX_ENTITIES, sizeof(uint32_t));
    // world->queried_ids = calloc(MAX_ENTITIES, sizeof(uint32_t));
    // world->query_num_results = 0;
    for (size_t i = 0; i < sizeof(world->queries) / sizeof(world->queries[0]);
         i++) {
        printf("i: %zu\n", i);
        world->queries[i].ids = calloc(MAX_ENTITIES, sizeof(uint32_t));
    }

    //    bg_pixels = (uint32_t*)malloc(CANVAS_WIDTH * CANVAS_HEIGHT *
    //    sizeof(bg_pixels));
    // bg_buffer = sglCreateBuffer(bg_pixels, CANVAS_WIDTH, CANVAS_HEIGHT,
    // SGL_PIXELFORMAT_ABGR32);

    static size_t mem = 0;
    mem += MAX_ENTITIES * sizeof(Player);
    mem += MAX_ENTITIES * sizeof(Position);
    mem += MAX_ENTITIES * sizeof(Velocity);
    mem += MAX_ENTITIES * sizeof(Acceleration);
    mem += MAX_ENTITIES * sizeof(Direction);
    mem += MAX_ENTITIES * sizeof(CircleCollider);
    mem += MAX_ENTITIES * sizeof(CapsuleCollider);
    mem += MAX_ENTITIES * sizeof(Projectile);
    mem += MAX_ENTITIES * sizeof(TimeToLive);
    mem += MAX_ENTITIES * sizeof(Appearance);
    mem += MAX_ENTITIES * sizeof(uint32_t);
    mem += MAX_ENTITIES * sizeof(uint32_t);
    mem += MAX_ENTITIES * sizeof(uint32_t);
    for (size_t i = 0; i < sizeof(world->queries) / sizeof(world->queries[0]);
         i++) {
        mem += MAX_ENTITIES * sizeof(uint32_t);
    }
    // mem += CANVAS_WIDTH * CANVAS_HEIGHT * sizeof(bg_pixels);
    printf("Mem allocated: " BLU "%zu\n" RESET, mem);
    world->font = sglCreateFont("../res/small-font.png", 5, 7, true);
}

void reset_world(void) {
    world->next_id = 1;
    memset(world->entity_states, 0, MAX_ENTITIES * sizeof(uint32_t));
    memset(world->component_masks, 0, MAX_ENTITIES * sizeof(uint32_t));

    memset(world->player_components, 0, MAX_ENTITIES * sizeof(Player));
    memset(world->position_components, 0, MAX_ENTITIES * sizeof(Position));
    memset(world->velocity_components, 0, MAX_ENTITIES * sizeof(Velocity));
    memset(
        world->acceleration_components, 0, MAX_ENTITIES * sizeof(Acceleration));
    memset(world->direction_components, 0, MAX_ENTITIES * sizeof(Direction));
    memset(world->circle_collider_components, 0,
        MAX_ENTITIES * sizeof(CircleCollider));
    memset(world->capsule_collider_components, 0,
        MAX_ENTITIES * sizeof(CapsuleCollider));
    memset(world->projectile_components, 0, MAX_ENTITIES * sizeof(Projectile));
    memset(
        world->time_to_live_components, 0, MAX_ENTITIES * sizeof(TimeToLive));
    memset(world->appearance_components, 0, MAX_ENTITIES * sizeof(Appearance));
}

void init_game(void) {
    reset_world();

    // players
    for (size_t i = 0; i < 2; i++) {
        delete_entity(world->player_ids[i]);
        uint32_t id = create_entity();
        world->player_ids[i] = id;

        add_component(id, PLAYER_COMPONENT);
        add_component(id, POSITION_COMPONENT);
        add_component(id, VELOCITY_COMPONENT);
        add_component(id, ACCELERATION_COMPONENT);
        add_component(id, DIRECTION_COMPONENT);
        add_component(id, CIRCLE_COLLIDER_COMPONENT);
        add_component(id, APPEARANCE_COMPONENT);

        world->player_components[id].health = 100.f;
        world->position_components[id].x = CANVAS_WIDTH * (1 + i) / 3.f;
        world->position_components[id].y = CANVAS_HEIGHT / 2.f;
        world->circle_collider_components[id].radius = 6;
        world->circle_collider_components[id].mass = 10;
        world->appearance_components[id].draw = draw_player;
        world->appearance_components[id].layer = 100;
    }

    world->player_components[world->player_ids[0]].color = 0x00ff00ff;
    world->player_components[world->player_ids[1]].color = 0xff0000ff;

    world->player_components[world->player_ids[1]].angle = M_PI;

    srand(time(NULL));

    // barriers
	for (size_t i = 0; i < 8; i++) {
		uint32_t capsule_id = create_entity();
		add_component(capsule_id, CAPSULE_COLLIDER_COMPONENT);
		add_component(capsule_id, APPEARANCE_COMPONENT);
		world->appearance_components[capsule_id].layer = 109;
		world->appearance_components[capsule_id].draw = draw_obstacle;
		float sx = randf() * CANVAS_WIDTH;
		float sy = randf() * CANVAS_HEIGHT;
		world->capsule_collider_components[capsule_id].sx = sx;
		world->capsule_collider_components[capsule_id].sy = sy;
		world->capsule_collider_components[capsule_id].ex = sx + randf() * 100 - 50;
		world->capsule_collider_components[capsule_id].ey = sy + randf() * 100 - 50;
		world->capsule_collider_components[capsule_id].radius = randf() * 2 + 2;
	}

    // background
    // uint32_t bg = create_entity();
    // add_component(bg, APPEARANCE_COMPONENT);
    // world->appearance_components[bg].layer = 0;
    // world->appearance_components[bg].draw = draw_bg;

    // float bg_unit = (CANVAS_WIDTH + CANVAS_HEIGHT)/2.f/12;
    // for (int i = 0; i < 128; i++) {
    // 	uint8_t r = 0x37 + ((randf()+1)/2) * 30;
    // 	uint8_t g = 0x5c + ((randf()+1)/2) * 30;
    // 	uint8_t b = 0x20 + ((randf()+1)/2) * 30;
    // 	uint32_t c = (r << 24) + (g << 16) + (b << 8) + 0xff;
    // 	sglFillCircle(bg_buffer, c, CANVAS_WIDTH*randf(), CANVAS_HEIGHT*randf(),
    // bg_unit*(randf()*2+1));
    // }
}

void inject_code_on_reload(void) {}

void handleInput(void) {
    if (world->m.left) {
    } else {
    }

    // world->capsule_collider_components[3].ex = world->m.x;
    // world->capsule_collider_components[3].ey = world->m.y;

    world->player_components[world->player_ids[0]].acc = 0;
    world->player_components[world->player_ids[1]].acc = 0;

    int direction1 = 1;

    if (world->keyboard[SDL_SCANCODE_W]) {
        world->player_components[world->player_ids[0]].acc = 300;
    }
    if (world->keyboard[SDL_SCANCODE_S]) {
        world->player_components[world->player_ids[0]].acc = -300;
        direction1 = -1;
    }
    if (world->keyboard[SDL_SCANCODE_A]) {
        world->player_components[world->player_ids[0]].angle +=
            -3.5 * direction1 * WORLD_DT;
        world->player_components[world->player_ids[0]].driven_distance +=
            30 * WORLD_DT;
    }
    if (world->keyboard[SDL_SCANCODE_D]) {
        world->player_components[world->player_ids[0]].angle +=
            3.5 * direction1 * WORLD_DT;
        world->player_components[world->player_ids[0]].driven_distance +=
            30 * WORLD_DT;
    }
    if (world->keyboard[SDL_SCANCODE_F] &&
        world->time_since_start -
                world->player_components[world->player_ids[0]].timeout >
            0.5) {
        world->player_components[world->player_ids[0]].timeout =
            world->time_since_start;
        const uint32_t projectile_id = create_entity();
        add_component(projectile_id, POSITION_COMPONENT);
        add_component(projectile_id, VELOCITY_COMPONENT);
        add_component(projectile_id, ACCELERATION_COMPONENT);
        add_component(projectile_id, DIRECTION_COMPONENT);
        add_component(projectile_id, CIRCLE_COLLIDER_COMPONENT);
        add_component(projectile_id, PROJECTILE_COMPONENT);
        add_component(projectile_id, APPEARANCE_COMPONENT);

        world->projectile_components[projectile_id].entity_source =
            world->player_ids[0];
        world->projectile_components[projectile_id].color = 0x007700ff;

        float px = world->position_components[world->player_ids[0]].x;
        float py = world->position_components[world->player_ids[0]].y;
        float pangle = world->player_components[world->player_ids[0]].angle;

        world->position_components[projectile_id].x = px + cosf(pangle) * 6;
        world->position_components[projectile_id].y = py + sinf(pangle) * 6;

        world->direction_components[projectile_id].angle = pangle;

        world->velocity_components[projectile_id].vel = 500;

        world->circle_collider_components[projectile_id].radius = 3;
        world->circle_collider_components[projectile_id].mass = 2;

        world->appearance_components[projectile_id].draw = draw_bullet;
        world->appearance_components[projectile_id].layer = 110;
    }

    int direction2 = 1;

    if (world->keyboard[SDL_SCANCODE_UP]) {
        world->player_components[world->player_ids[1]].acc = 300;
    }
    if (world->keyboard[SDL_SCANCODE_DOWN]) {
        world->player_components[world->player_ids[1]].acc = -300;
        direction2 = -1;
    }
    if (world->keyboard[SDL_SCANCODE_LEFT]) {
        world->player_components[world->player_ids[1]].angle +=
            -3.5 * direction2 * WORLD_DT;
        world->player_components[world->player_ids[1]].driven_distance +=
            30 * WORLD_DT;
    }
    if (world->keyboard[SDL_SCANCODE_RIGHT]) {
        world->player_components[world->player_ids[1]].angle +=
            3.5 * direction2 * WORLD_DT;
        world->player_components[world->player_ids[1]].driven_distance +=
            30 * WORLD_DT;
    }
    if (world->keyboard[SDL_SCANCODE_PERIOD] &&
        world->time_since_start -
                world->player_components[world->player_ids[1]].timeout >
            0.5) {
        world->player_components[world->player_ids[1]].timeout =
            world->time_since_start;
        const uint32_t projectile_id = create_entity();
        add_component(projectile_id, POSITION_COMPONENT);
        add_component(projectile_id, VELOCITY_COMPONENT);
        add_component(projectile_id, ACCELERATION_COMPONENT);
        add_component(projectile_id, DIRECTION_COMPONENT);
        add_component(projectile_id, CIRCLE_COLLIDER_COMPONENT);
        add_component(projectile_id, PROJECTILE_COMPONENT);
        add_component(projectile_id, APPEARANCE_COMPONENT);

        world->projectile_components[projectile_id].entity_source =
            world->player_ids[1];
        world->projectile_components[projectile_id].color = 0x990000ff;

        float px = world->position_components[world->player_ids[1]].x;
        float py = world->position_components[world->player_ids[1]].y;
        float pangle = world->player_components[world->player_ids[1]].angle;

        world->position_components[projectile_id].x = px + cosf(pangle) * 6;
        world->position_components[projectile_id].y = py + sinf(pangle) * 6;

        world->direction_components[projectile_id].angle = pangle;

        world->velocity_components[projectile_id].vel = 500;

        world->circle_collider_components[projectile_id].radius = 3;
        world->circle_collider_components[projectile_id].mass = 2;

        world->appearance_components[projectile_id].draw = draw_bullet;
        world->appearance_components[projectile_id].layer = 110;
    }
}

void step(void) {
    handleInput();
    draw_system();
    physics_system();
    time_to_live_system();
}

uint32_t create_entity(void) {
    // TODO: maybe realloc when full
    uint32_t newEntity = 0;
    do {
        newEntity = world->next_id++;
        world->next_id = world->next_id % MAX_ENTITIES;
        if (world->next_id == 0)
            world->next_id++;
    } while (ENTITY_ALIVE == world->entity_states[newEntity]);

    world->entity_states[newEntity] = ENTITY_ALIVE;
    return newEntity;
}

void delete_entity(uint32_t id) {
    world->component_masks[id] = 0;
    world->entity_states[id] = ENTITY_DEAD;
}

void add_component(uint32_t id, uint32_t component) {
    if (world->entity_states[id] == ENTITY_DEAD) {
        printf("warning: adding component to dead entity!\n");
    }
    world->component_masks[id] |= component;
}

void remove_component(uint32_t id, uint32_t component) {
    if (world->entity_states[id] == ENTITY_DEAD) {
        printf("warning: adding component to dead entity!\n");
    }
    world->component_masks[id] ^= component;
}

void query_entities(QueryResult* qr, uint32_t query_component_mask) {
    qr->num_results = 0;
    for (size_t id = 0; id < MAX_ENTITIES; id++) {
        if (world->entity_states[id] == ENTITY_ALIVE &&
            (world->component_masks[id] & query_component_mask) ==
                query_component_mask) {
            qr->ids[(qr->num_results)++] = id;
        }
    }
}

bool has_component(uint32_t id, uint32_t component) {
    return world->component_masks[id] & component;
}

//== SYSTEMS ===============================================================//

// void input_system() {}

void draw_bg(uint32_t id) {
    // drawBackground
    // float bg_unit = (CANVAS_WIDTH + CANVAS_HEIGHT)/2.f/12;
    // for (int i = 0; i < 24; i++) {
    // 	uint8_t r = 0x37 + randf() * 20;
    // 	uint8_t g = 0x5c + randf() * 20;
    // 	uint8_t b = 0x20 + randf() * 20;
    // 	uint32_t c = (r << 24) + (g << 16) + (b << 8) + 0xff;
    // 	sglFillCircle(world->buffer, c, bg_unit*randf()*12, bg_unit*randf()*12,
    // bg_unit*randf()*12);
    // }
}

void draw_obstacle(uint32_t id) {
    float sx = world->capsule_collider_components[id].sx;
    float sy = world->capsule_collider_components[id].sy;
    float ex = world->capsule_collider_components[id].ex;
    float ey = world->capsule_collider_components[id].ey;
    float r = world->capsule_collider_components[id].radius;

    // float angle = atan2f(ey, ex);

    sglFillCircle(world->buffer, 0xffffffff, sx, sy, r);
    sglFillCircle(world->buffer, 0xffffffff, ex, ey, r);

    float rsx = -sy;
    float rsy = sx;
    float rex = -ey;
    float rey = ex;

    float dx = (rex - rsx);
    float dy = (rey - rsy);
    float len = sqrtf(dx * dx + dy * dy);
    dx /= len;
    dy /= len;

    for (float step = 0; step < r - 1; step += 0.1) {
        sglDrawLine(world->buffer, 0xffffffff, roundf(sx + dx * step),
            round(sy + dy * step), roundf(ex + dx * step),
            roundf(ey + dy * step));
    }
    for (float step = 0; -step < r - 1; step -= 0.1) {
        sglDrawLine(world->buffer, 0xffffffff, roundf(sx + dx * step),
            round(sy + dy * step), roundf(ex + dx * step),
            roundf(ey + dy * step));
    }
}

void draw_player(uint32_t id) {
    float x = world->position_components[id].x;
    float y = world->position_components[id].y;
    float angle = world->player_components[id].angle;
    uint32_t color = world->player_components[id].color;

    float w = 10;
    float h = 8;

    float tl_x = 0 - w / 2;
    float tl_y = 0 - h / 2;
    float tr_x = w - w / 2;
    float tr_y = 0 - h / 2;
    float br_x = w - w / 2;
    float br_y = h - h / 2;
    float bl_x = 0 - w / 2;
    float bl_y = h - h / 2;

    rotate_point(&tl_x, &tl_y, tl_x, tl_y, angle);
    rotate_point(&tr_x, &tr_y, tr_x, tr_y, angle);
    rotate_point(&br_x, &br_y, br_x, br_y, angle);
    rotate_point(&bl_x, &bl_y, bl_x, bl_y, angle);

    sglFillTriangle(world->buffer, 0x000000ff, tl_x + x, tl_y + y, bl_x + x,
        bl_y + y, tr_x + x, tr_y + y);
    sglFillTriangle(world->buffer, 0x000000ff, tr_x + x, tr_y + y, bl_x + x,
        bl_y + y, br_x + x, br_y + y);

    sglDrawLine(world->buffer, color, tl_x + x, tl_y + y, tr_x + x, tr_y + y);
    sglDrawLine(world->buffer, color, tr_x + x, tr_y + y, br_x + x, br_y + y);
    sglDrawLine(world->buffer, color, br_x + x, br_y + y, bl_x + x, bl_y + y);
    sglDrawLine(world->buffer, color, bl_x + x, bl_y + y, tl_x + x, tl_y + y);

    float barrel_sx = 0;
    float barrel_sy = 0;
    float barrel_ex = 8;
    float barrel_ey = 0;

    rotate_point(&barrel_sx, &barrel_sy, barrel_sx, barrel_sy, angle);
    rotate_point(&barrel_ex, &barrel_ey, barrel_ex, barrel_ey, angle);

    sglDrawLine(world->buffer, color, barrel_sx + x, barrel_sy + y,
        barrel_ex + x, barrel_ey + y);

    float h_bar_len = 12;
    float current_h = 12 * world->player_components[id].health / 100;
    sglDrawLine(world->buffer, 0x004400ff, x - h_bar_len / 2, y - 10,
        x + h_bar_len - h_bar_len / 2, y - 10);
    sglDrawLine(world->buffer, 0x00ff00ff, x - h_bar_len / 2, y - 10,
        x + current_h - h_bar_len / 2, y - 10);
}

void draw_bullet(uint32_t id) {
    float x = world->position_components[id].x;
    float y = world->position_components[id].y;
    float r = world->circle_collider_components[id].radius;
    uint32_t c = world->projectile_components[id].color;

    sglFillCircle(world->buffer, c, x, y, r - 1);
}

void draw_explosion(uint32_t id) {
    float x = world->position_components[id].x;
    float y = world->position_components[id].y;
    float ttl = world->time_to_live_components[id].time;
    sglFillCircle(world->buffer, 0xffbb22ff, x, y, ttl * 20);
}

void draw_track(uint32_t id) {
    float x = world->position_components[id].x;
    float y = world->position_components[id].y;
    float angle = world->direction_components[id].angle;
    float ttl = world->time_to_live_components[id].time;

    // left track
    float left_x = -4;
    float left_y = 0;
    float right_x = -2;
    float right_y = 0;
    rotate_point(&left_x, &left_y, left_x, left_y, angle);
    rotate_point(&right_x, &right_y, right_x, right_y, angle);

    sglDrawLine(world->buffer, 0xffffff00 + (int)(ttl / 5.f * 0x20), left_x + x,
        left_y + y, right_x + x, right_y + y);

    // right track
    left_x = +4;
    left_y = 0;
    right_x = +2;
    right_y = 0;
    rotate_point(&left_x, &left_y, left_x, left_y, angle);
    rotate_point(&right_x, &right_y, right_x, right_y, angle);

    sglDrawLine(world->buffer, 0xffffff00 + (int)(ttl / 5.f * 0x20), left_x + x,
        left_y + y, right_x + x, right_y + y);

    // sglFillCircle(world->buffer, 0xffbb22ff, x, y, ttl * 20);
}

int compare_layers(const void* a, const void* b) {
    int id1 = *(const uint32_t*)a;
    int id2 = *(const uint32_t*)b;

    uint16_t layer1 = world->appearance_components[id1].layer;
    uint16_t layer2 = world->appearance_components[id2].layer;

    return layer1 - layer2;
}

void draw_system(void) {
    // clear pixel buffer
    sglClear(world->buffer);

    // memcpy(world->buffer->pixels, bg_pixels, CANVAS_WIDTH * CANVAS_HEIGHT *
    // sizeof(uint32_t));

    QueryResult* qr = &(world->queries[0]);

    query_entities(qr, APPEARANCE_COMPONENT);
    // sort layers
    qsort(qr->ids, qr->num_results, sizeof(uint32_t), compare_layers);
    for (size_t i = 0; i < qr->num_results; i++) {
        const uint32_t id = qr->ids[i];
        world->appearance_components[id].draw(id);
    }

    if (world->debug_mode) {
        sglClear(world->debug_buffer);

        query_entities(qr, POSITION_COMPONENT | VELOCITY_COMPONENT |
                               CIRCLE_COLLIDER_COMPONENT | DIRECTION_COMPONENT);
        for (size_t i = 0; i < qr->num_results; i++) {
            const uint32_t id = qr->ids[i];
            float x = world->position_components[id].x;
            float y = world->position_components[id].y;
            float r = world->circle_collider_components[id].radius;
            float v = world->velocity_components[id].vel;
            float a = world->direction_components[id].angle;
            sglDrawCircle(world->debug_buffer, 0xffffffaa, x * SCALE_FACTOR,
                y * SCALE_FACTOR, r * SCALE_FACTOR);
            sglDrawLine(world->debug_buffer, 0x00ffff77, x * SCALE_FACTOR,
                y * SCALE_FACTOR, (x * SCALE_FACTOR + cosf(a) * v),
                (y * SCALE_FACTOR + sinf(a) * v));
            if (has_component(id, PLAYER_COMPONENT)) {
                float pv = world->player_components[id].vel;
                float pa = world->player_components[id].angle;
                sglDrawLine(world->debug_buffer, 0xff00ff77, x * SCALE_FACTOR,
                    y * SCALE_FACTOR, (x * SCALE_FACTOR + cosf(pa) * pv),
                    (y * SCALE_FACTOR + sinf(pa) * pv));
            }
        }

        query_entities(qr, CAPSULE_COLLIDER_COMPONENT);
        for (size_t i = 0; i < qr->num_results; i++) {
            const uint32_t id = qr->ids[i];
            float sx = world->capsule_collider_components[id].sx * SCALE_FACTOR;
            float sy = world->capsule_collider_components[id].sy * SCALE_FACTOR;
            float ex = world->capsule_collider_components[id].ex * SCALE_FACTOR;
            float ey = world->capsule_collider_components[id].ey * SCALE_FACTOR;
            float r =
                world->capsule_collider_components[id].radius * SCALE_FACTOR;

            float rsx = -sy;
            float rsy = sx;
            float rex = -ey;
            float rey = ex;

            float dx = (rex - rsx);
            float dy = (rey - rsy);
            float len = sqrtf(dx * dx + dy * dy);
            dx /= len;
            dy /= len;

            sglDrawCircle(world->debug_buffer, 0xff00ffff, sx, sy, r);
            sglDrawCircle(world->debug_buffer, 0xff00ffff, ex, ey, r);
            sglDrawLine(world->debug_buffer, 0xff00ffff, sx + dx * r,
                sy + dy * r, ex + dx * r, ey + dy * r);
            sglDrawLine(world->debug_buffer, 0xff00ffff, sx + dx * -r,
                sy + dy * -r, ex + dx * -r, ey + dy * -r);
        }

        float win_w = WINDOW_WIDTH - 16;
        float scale = win_w / (float)MAX_ENTITIES;

        float span = roundf(MAX_ENTITIES / (float)win_w);
        for (float i = 0; i < MAX_ENTITIES - span; i += span) {
            float avg[9] = {0};
            for (size_t j = 0; j < span; j++) {
                uint32_t id = (uint32_t)roundf(i + j);
                for (size_t comp = 0; comp < 9; comp++) {
                    if (world->entity_states[id]) {
                        avg[comp] += has_component(id, 1 << (comp + 1));
                    }
                }
            }
            for (size_t comp = 0; comp < 9; comp++) {
                avg[comp] /= span;
            }

            for (size_t comp = 0; comp < 9; comp++) {
                if (avg[comp] >= 0.1) {
                    sglDrawPixel(world->debug_buffer, 0, avg[comp] * 255, 0,
                        0xff, i * scale + 8, WINDOW_HEIGHT - 12 + comp);
                } else {
                    sglDrawPixel(world->debug_buffer, 0, 0,
                        (1 - avg[comp]) * 255, 0x44, i * scale + 8,
                        WINDOW_HEIGHT - 12 + comp);
                }
            }
        }

        sglDrawLine(world->debug_buffer, 0xaaaaaaff,
            world->next_id * ((float)win_w / MAX_ENTITIES) + 8,
            WINDOW_HEIGHT - 24,
            world->next_id * ((float)win_w / MAX_ENTITIES) + 8, WINDOW_HEIGHT);
    }

    sglDrawText(
        world->buffer, "Press [H] For Help", 0x0077ffff, 4, 4, world->font);
}

struct {
    uint32_t first;
    uint32_t second;
} colliding_pairs[1024 * 16]; // pool of colliding circles
uint32_t fake_ball_ids[1024 * 16];

// TODO: set tick rate to a fixed value (1/60)
void physics_system(void) {
    QueryResult* qr = &(world->queries[0]);
    query_entities(qr, PLAYER_COMPONENT);

	for (size_t ph_step = 0; ph_step < PHYSICS_SUB_STEP; ph_step++) {
		float sim_dt = WORLD_DT/PHYSICS_SUB_STEP;

		for (size_t i = 0; i < qr->num_results; i++) {
			const uint32_t id = qr->ids[i];

			float s = sinf(world->player_components[id].angle);
			float c = cosf(world->player_components[id].angle);
			float r = world->circle_collider_components[id].radius;

			// printf("acc: %f\n", world->acceleration_components[id].acc);
			// printf("vel: %f\n", world->velocity_components[id].vel);

			world->player_components[id].vel +=
				world->player_components[id].acc * sim_dt;

			world->position_components[id].x +=
				c * world->player_components[id].vel * sim_dt;
			world->position_components[id].y +=
				s * world->player_components[id].vel * sim_dt;

			world->player_components[id].driven_distance +=
				fabs(world->player_components[id].vel * sim_dt);

			for (float* f = &world->player_components[id].old_driven_distance;
			*f < world->player_components[id].driven_distance; *f += 5) {
				const uint32_t track_id = create_entity();
				add_component(track_id, POSITION_COMPONENT);
				add_component(track_id, DIRECTION_COMPONENT);
				add_component(track_id, TIME_TO_LIVE_COMPONENT);
				add_component(track_id, APPEARANCE_COMPONENT);

				world->position_components[track_id].x =
					world->position_components[id].x;
				world->position_components[track_id].y =
					world->position_components[id].y;
				world->direction_components[track_id].angle =
					world->player_components[id].angle + M_PI / 2;
				world->time_to_live_components[track_id].time = 5;
				world->appearance_components[track_id].draw = draw_track;
				world->appearance_components[track_id].layer = 90;
			}

			world->player_components[id].vel +=
				world->player_components[id].vel * -4 * sim_dt;

			if (world->position_components[id].x + r >= CANVAS_WIDTH) {
				world->position_components[id].x = CANVAS_WIDTH - r;
			}
			if (world->position_components[id].y + r >= CANVAS_HEIGHT) {
				world->position_components[id].y = CANVAS_HEIGHT - r;
			}
			if (world->position_components[id].x - r < 0) {
				world->position_components[id].x = r;
			}
			if (world->position_components[id].y - r < 0) {
				world->position_components[id].y = r;
			}
		}

		query_entities(qr, POSITION_COMPONENT | VELOCITY_COMPONENT |
				 ACCELERATION_COMPONENT | DIRECTION_COMPONENT);
		for (size_t i = 0; i < qr->num_results; i++) {
			const uint32_t id = qr->ids[i];

			float s = sinf(world->direction_components[id].angle);
			float c = cosf(world->direction_components[id].angle);

			// printf("acc: %f\n", world->acceleration_components[id].acc);
			// printf("vel: %f\n", world->velocity_components[id].vel);

			world->velocity_components[id].vel +=
				world->acceleration_components[id].acc * sim_dt;

			world->position_components[id].x +=
				c * world->velocity_components[id].vel * sim_dt;
			world->position_components[id].y +=
				s * world->velocity_components[id].vel * sim_dt;

			if (has_component(id, PLAYER_COMPONENT)) {
				world->player_components[id].driven_distance +=
					fabs(world->velocity_components[id].vel * sim_dt);
			}

			float mass = 1;
			if (has_component(id, CIRCLE_COLLIDER_COMPONENT)) {
				mass = world->circle_collider_components[id].mass;
			}

			world->velocity_components[id].vel +=
				world->velocity_components[id].vel * -1.1 * mass * sim_dt;
		}

		query_entities(qr, PROJECTILE_COMPONENT | VELOCITY_COMPONENT);
		// printf("PROJECTILE_COMPONENT | VELOCITY_COMPONENT\t %d\n",
		// qr.num_results);
		for (size_t i = 0; i < qr->num_results; i++) {
			const uint32_t id = qr->ids[i];
			const float vel = world->velocity_components[id].vel;
			if (vel < 30) {
				delete_entity(id);

				// add explosion
				const uint32_t explosion_id = create_entity();
				add_component(explosion_id, POSITION_COMPONENT);
				add_component(explosion_id, TIME_TO_LIVE_COMPONENT);
				add_component(explosion_id, APPEARANCE_COMPONENT);
				world->appearance_components[explosion_id].draw = draw_explosion;
				world->appearance_components[explosion_id].layer = 111;

				world->time_to_live_components[explosion_id].time = 0.3;
				world->position_components[explosion_id].x =
					world->position_components[id].x;
				world->position_components[explosion_id].y =
					world->position_components[id].y;
			}
		}

		QueryResult* qr_capsule = &(world->queries[1]);

		query_entities(qr_capsule, CAPSULE_COLLIDER_COMPONENT);

		query_entities(qr,
				 POSITION_COMPONENT | VELOCITY_COMPONENT | CIRCLE_COLLIDER_COMPONENT);

		size_t colliding_pairs_count = 0;
		size_t fake_ball_count = 0;
		for (size_t i = 0; i < qr->num_results; i++) {
			const uint32_t id1 = qr->ids[i];
			float x1 = world->position_components[id1].x;
			float y1 = world->position_components[id1].y;
			float r1 = world->circle_collider_components[id1].radius;
			float m1 = world->circle_collider_components[id1].mass;

			// printf("id1: %d, x %f, y %f\n",id1, x1, y1);

			for (size_t j = 0; j < qr->num_results; j++) {
				const uint32_t id2 = qr->ids[j];
				if (id1 == id2)
					break;

				float x2 = world->position_components[id2].x;
				float y2 = world->position_components[id2].y;
				float r2 = world->circle_collider_components[id2].radius;

				if (entities_overlap(x1, y1, r1, x2, y2, r2)) {
					colliding_pairs[colliding_pairs_count].first = id1;
					colliding_pairs[colliding_pairs_count].second = id2;
					colliding_pairs_count++;

					float distance =
						sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
					float overlap = (distance - r1 - r2) / 2.f;

					x1 -= overlap * (x1 - x2) / distance;
					y1 -= overlap * (y1 - y2) / distance;

					x2 += overlap * (x1 - x2) / distance;
					y2 += overlap * (y1 - y2) / distance;

					world->position_components[id1].x = x1;
					world->position_components[id1].y = y1;
					world->position_components[id2].x = x2;
					world->position_components[id2].y = y2;
				}
			}

			for (size_t j = 0; j < qr_capsule->num_results; j++) {
				const uint32_t id2 = qr_capsule->ids[j];

				float sx = world->capsule_collider_components[id2].sx;
				float sy = world->capsule_collider_components[id2].sy;
				float ex = world->capsule_collider_components[id2].ex;
				float ey = world->capsule_collider_components[id2].ey;
				float r2 = world->capsule_collider_components[id2].radius;

				float fLineX1 = ex - sx;
				float fLineY1 = ey - sy;

				float fLineX2 = x1 - sx;
				float fLineY2 = y1 - sy;

				float fEdgeLength = fLineX1 * fLineX1 + fLineY1 * fLineY1;

				// This is nifty - It uses the DP of the line segment vs the line to
				// the object, to work out how much of the segment is in the
				// "shadow" of the object vector. The min and max clamp this to lie
				// between 0 and the line segment length, which is then normalised.
				// We can use this to calculate the closest point on the line
				// segment
				float t = fmaxf(0, fminf(fEdgeLength,
							 (fLineX1 * fLineX2 + fLineY1 * fLineY2))) /
					fEdgeLength;

				// Which we do here
				float fClosestPointX = sx + t * fLineX1;
				float fClosestPointY = sy + t * fLineY1;

				// And once we know the closest point, we can check if the ball has
				// collided with the segment in the same way we check if two balls
				// have collided
				float fDistance =
					sqrtf((x1 - fClosestPointX) * (x1 - fClosestPointX) +
		   (y1 - fClosestPointY) * (y1 - fClosestPointY));

				if (fDistance <= (r1 + r2)) {
					uint32_t fake_ball_id = create_entity();
					add_component(fake_ball_id, POSITION_COMPONENT);
					add_component(fake_ball_id, CIRCLE_COLLIDER_COMPONENT);
					add_component(fake_ball_id, VELOCITY_COMPONENT);
					add_component(fake_ball_id, DIRECTION_COMPONENT);
					world->circle_collider_components[fake_ball_id].radius = r2;
					world->circle_collider_components[fake_ball_id].mass = m1;
					world->position_components[fake_ball_id].x = fClosestPointX;
					world->position_components[fake_ball_id].y = fClosestPointY;
					world->velocity_components[fake_ball_id].vel =
						world->velocity_components[id1].vel;
					world->direction_components[fake_ball_id].angle =
						world->direction_components[id1].angle + M_PI;

					fake_ball_ids[fake_ball_count++] = fake_ball_id;

					colliding_pairs[colliding_pairs_count].first = id1;
					colliding_pairs[colliding_pairs_count].second = fake_ball_id;
					colliding_pairs_count++;

					float fOverlap = 1.0f * (fDistance - r1 - r2);

					world->position_components[id1].x -=
						fOverlap * (x1 - fClosestPointX) / fDistance;
					world->position_components[id1].y -=
						fOverlap * (y1 - fClosestPointY) / fDistance;
				}
			}
		}

		for (size_t i = 0; i < colliding_pairs_count; i++) {
			uint32_t id1 = colliding_pairs[i].first;
			uint32_t id2 = colliding_pairs[i].second;

			if ((has_component(id1, PLAYER_COMPONENT) &&
				has_component(id2, PROJECTILE_COMPONENT)) ||
				(has_component(id2, PLAYER_COMPONENT) &&
				has_component(id1, PROJECTILE_COMPONENT))) {

				uint32_t projectile_id = id1;
				uint32_t player_id = id2;

				if (has_component(id2, PROJECTILE_COMPONENT)) {
					projectile_id = id2;
					player_id = id1;
				}

				if (world->projectile_components[projectile_id].entity_source !=
					player_id) {
					// delete bullet if hit anyone other than source
					delete_entity(projectile_id);

					world->player_components[player_id].health -= 20;

					if (world->player_components[player_id].health <= 0) {
						init_game();
						// world->player_components[player_id].health = 100;
						// world->position_components[player_id].x =
						//     CANVAS_WIDTH / 2.f;
						// world->position_components[player_id].y =
						//     CANVAS_HEIGHT / 2.f;
					}

					// add explosion
					const uint32_t explosion_id = create_entity();
					add_component(explosion_id, POSITION_COMPONENT);
					add_component(explosion_id, TIME_TO_LIVE_COMPONENT);
					add_component(explosion_id, APPEARANCE_COMPONENT);

					world->appearance_components[explosion_id].draw =
						draw_explosion;
					world->appearance_components[explosion_id].layer = 111;
					world->time_to_live_components[explosion_id].time = 0.3;
					world->position_components[explosion_id].x =
						world->position_components[projectile_id].x +
						cosf(world->direction_components[projectile_id].angle) * 3;
					world->position_components[explosion_id].y =
						world->position_components[projectile_id].y +
						sinf(world->direction_components[projectile_id].angle) * 3;
				}
			}

			float px1 = world->position_components[id1].x;
			float py1 = world->position_components[id1].y;
			float px2 = world->position_components[id2].x;
			float py2 = world->position_components[id2].y;

			float vx1 = world->velocity_components[id1].vel *
				cosf(world->direction_components[id1].angle);
			float vy1 = world->velocity_components[id1].vel *
				sinf(world->direction_components[id1].angle);
			float vx2 = world->velocity_components[id2].vel *
				cosf(world->direction_components[id2].angle);
			float vy2 = world->velocity_components[id2].vel *
				sinf(world->direction_components[id2].angle);

			float dist =
				sqrtf((px1 - px2) * (px1 - px2) + (py1 - py2) * (py1 - py2));

			float nx = (px2 - px1) / dist;
			float ny = (py2 - py1) / dist;

			float m1 = world->circle_collider_components[id1].mass;
			float m2 = world->circle_collider_components[id2].mass;

			float kx = (vx1 - vx2);
			float ky = (vy1 - vy2);
			float p = 2.0 * (nx * kx + ny * ky) / (m1 + m2);

			vx1 = vx1 - p * m2 * nx;
			vy1 = vy1 - p * m2 * ny;
			vx2 = vx2 + p * m1 * nx;
			vy2 = vy2 + p * m1 * ny;

			world->direction_components[id1].angle = atan2f(vy1, vx1);
			world->velocity_components[id1].vel =
				sqrtf((vx1) * (vx1) + (vy1) * (vy1));
			world->direction_components[id2].angle = atan2f(vy2, vx2);
			world->velocity_components[id2].vel =
				sqrtf((vx2) * (vx2) + (vy2) * (vy2));
		}

		for (size_t i = 0; i < fake_ball_count; i++) {
			delete_entity(fake_ball_ids[i]);
		}
	}
}

void time_to_live_system(void) {
    QueryResult* qr = &(world->queries[0]);
    query_entities(qr, TIME_TO_LIVE_COMPONENT);
    for (size_t i = 0; i < qr->num_results; i++) {
        const uint32_t id = qr->ids[i];

        world->time_to_live_components[id].time -= WORLD_DT;

        if (world->time_to_live_components[id].time <= 0) {
            delete_entity(id);
        }
    }
}
